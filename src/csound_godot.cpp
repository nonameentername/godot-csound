#include "csound_godot.h"
#include "csound_files.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/classes/audio_stream_mp3.hpp"
//#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/classes/project_settings.hpp"
#include "godot_cpp/classes/time.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "csound_godot_opcodes.h"
#include <cstdio>
#include <cstdlib>
#ifdef MINGW
#include "win_fmemopen.h"
#else
#include <sys/mman.h>
#endif

using namespace godot;

CsoundGodot::CsoundGodot() {
    csound = NULL;
    initialized = false;
    active = false;
    channels_cleared = false;

    finished = false;
    csound = new Csound();

    configure_csound();

    mutex.instantiate();
    semaphore.instantiate();

    mix_rate = AudioServer::get_singleton()->get_mix_rate();

    midi_buffer = csoundCreateCircularBuffer(csound->GetCsound(), 2048, sizeof(int));

    for (int i = 0; i < BUFFER_FRAME_SIZE; i++) {
        temp_buffer[i] = 0;
    }
}

void CsoundGodot::configure_csound() {
    // csound->CreateMessageBuffer(0);
    csound->SetDebug(false);
    csound->SetHostAudioIO();

    if (ProjectSettings::get_singleton()->get_setting("audio/csound/use_resource_files", true)) {
        csoundSetOpenFileCallback(csound->GetCsound(), open_file);
        csoundSetOpenSoundFileCallback(csound->GetCsound(), open_sound_file);
    }

    csound->SetMessageCallback(set_message);

    csound->SetHostMIDIIO();
    csound->SetExternalMidiInOpenCallback(open_midi_device);
    csound->SetExternalMidiWriteCallback(write_midi_data);
    csound->SetExternalMidiReadCallback(read_midi_data);
    csound->SetHostData((void *)this);

    csnd::DictionarySetValue::load_plugin((csnd::Csound *)csound->GetCsound());
}

CsoundGodot::~CsoundGodot() {
    csoundDestroyCircularBuffer(csound->GetCsound(), midi_buffer);

    if (csound != NULL) {
        delete csound;
        csound = NULL;
    }
}

void CsoundGodot::start() {
    if (csound != NULL) {
        //int processor_count = OS::get_singleton()->get_processor_count();
        // csound->SetOption(vformat("-j%d", processor_count).ascii());
        csound->SetOption("-n");
        csound->SetOption("-d");

        if (ProjectSettings::get_singleton()->get_setting("audio/csound/hide_csound_logs", true)) {
            csound->SetOption("-m16");
        }

        bool csound_error = false;

        String default_csd = R"(
            <CsoundSynthesizer>
            <CsOptions>
            -+rtmidi=NULL -M0 -n
            </CsOptions>
            <CsInstruments>

            sr = 44100
            ksmps = 32
            nchnls = 2
            0dbfs = 1

            </CsInstruments>
            <CsScore>
            f0 z

            </CsScore>
            </CsoundSynthesizer>
        )";

        if (script.is_null()) {
            godot::UtilityFunctions::push_warning("csound script not specified. Using default csd");
            int error = csound->CompileCSD(default_csd.ascii(), 1);
            if (error != 0) {
                godot::UtilityFunctions::push_error("Could not compile default csound script");
                csound_error = true;
            }
        } else if (script.is_valid()) {
            int error = csound->Compile(script->get_path().get_file().ascii());
            if (error != 0) {
                godot::UtilityFunctions::push_error("Could not compile csound script: ", script->get_path().get_file());
                csound_error = true;
            }
        }

        if (!csound_error) {
            //int p_frames = 512;

            csound->Start();

            //int frame_size = p_frames + csound->GetKsmps();

            output_left_channel.buffer = csoundCreateCircularBuffer(csound->GetCsound(), CIRCULAR_BUFFER_SIZE, sizeof(MYFLT));
            output_right_channel.buffer = csoundCreateCircularBuffer(csound->GetCsound(), CIRCULAR_BUFFER_SIZE, sizeof(MYFLT));

            // TODO: check input and output channels separately
            if (output_channels.size() != csound->GetChannels(0)) {
                input_channels.resize(csound->GetChannels(0));
                output_channels.resize(csound->GetChannels(0));

                for (int j = 0; j < csound->GetChannels(0); j++) {
                    input_channels.write[j] = csoundCreateCircularBuffer(csound->GetCsound(), CIRCULAR_BUFFER_SIZE, sizeof(MYFLT));
                    output_channels.write[j].buffer = csoundCreateCircularBuffer(csound->GetCsound(), CIRCULAR_BUFFER_SIZE, sizeof(MYFLT));
                }
            }

            if (ksmps_temp_buffer.size() != csound->GetKsmps()) {
                ksmps_temp_buffer.resize(csound->GetKsmps());
                ksmps_temp_buffer.fill(0);
            }

            if (ksmps_left_buffer.size() != csound->GetKsmps()) {
                ksmps_left_buffer.resize(csound->GetKsmps());
                ksmps_left_buffer.fill(0);
            }

            if (ksmps_right_buffer.size() != csound->GetKsmps()) {
                ksmps_right_buffer.resize(csound->GetKsmps());
                ksmps_right_buffer.fill(0);
            }

            initialized = true;
            start_thread();

            emit_signal("csound_ready", csound_name);
        }
    }
}

void CsoundGodot::stop() {
    initialized = false;
    stop_thread();

    if (csound != NULL) {
        // csound->Stop();
        cleanup_channels();
    }
}

void CsoundGodot::finish() {
    stop();
    finished = true;
}

void CsoundGodot::reset() {
    initialized = false;
    stop_thread();

    if (csound != NULL) {
        cleanup_channels();
        csound->Reset();
        configure_csound();
    }
}

void CsoundGodot::cleanup_channels() {
    csoundDestroyCircularBuffer(csound->GetCsound(), output_left_channel.buffer);
    csoundDestroyCircularBuffer(csound->GetCsound(), output_right_channel.buffer);

    for (int channel = 0; channel < input_channels.size(); channel++) {
        csoundDestroyCircularBuffer(csound->GetCsound(), input_channels[channel]);
    }

    for (int channel = 0; channel < output_channels.size(); channel++) {
        csoundDestroyCircularBuffer(csound->GetCsound(), output_channels[channel].buffer);
    }

    for (KeyValue<String, void *> &E : input_named_channels_buffer) {
        csoundDestroyCircularBuffer(csound->GetCsound(), input_named_channels_buffer[E.key]);
    }

    for (int channel = 0; channel < output_named_channels.size(); channel++) {
        csoundDestroyCircularBuffer(csound->GetCsound(), output_named_channels[channel].buffer);
    }

    input_channels.clear();
    output_channels.clear();
    input_named_channels_buffer.clear();
    output_named_channels.clear();
    named_channels.clear();
}

void CsoundGodot::set_soundfont(Ref<SoundFontFileReader> p_soundfont) {
    soundfont = p_soundfont;

    /*
    if (ResourceLoader::get_singleton()->exists(node_path)) {
        Variant resource = ResourceLoader::get_singleton()->load(node_path);
        Ref<SoundFontFileReader> soundfont = resource;
        if (soundfont != NULL) {
        }
    }
    */
}

Ref<SoundFontFileReader> CsoundGodot::get_soundfont() {
    return soundfont;
}

void CsoundGodot::set_midi_file(Ref<MidiFileReader> p_midi_file) {
    midi_file = p_midi_file;
}

Ref<MidiFileReader> CsoundGodot::get_midi_file() {
    return midi_file;
}

void CsoundGodot::add_named_channel(String name) {
    if (!input_named_channels_buffer.has(name)) {
        input_named_channels_buffer.insert(name, csoundCreateCircularBuffer(csound->GetCsound(), CIRCULAR_BUFFER_SIZE, sizeof(MYFLT)));
    }

    if (!named_channels.has(name)) {
        int index = output_named_channels.size();
        output_named_channels.resize(index + 1);
        output_named_channels.write[index].buffer =
            csoundCreateCircularBuffer(csound->GetCsound(), CIRCULAR_BUFFER_SIZE, sizeof(MYFLT));
        output_named_channels.write[index].name = name;
        named_channels.insert(name, index);
    }
}

void CsoundGodot::update_named_channels(int p_frames) {
    if (!initialized) {
        return;
    }

    //int frame_size = p_frames + csound->GetKsmps();

    controlChannelInfo_t *tmp;
    int num_channels = csound->ListChannels(tmp);

    for (int channel = 0; channel < num_channels; channel++) {
        char *name = tmp[channel].name;
        int type = tmp[channel].type;
        if ((type & CSOUND_CHANNEL_TYPE_MASK) == CSOUND_AUDIO_CHANNEL &&
            (type & (CSOUND_INPUT_CHANNEL | CSOUND_OUTPUT_CHANNEL))) {
            add_named_channel(name);
        }
    }

    // TODO: does deleteChannelList delete leaks memory?
    csound->DeleteChannelList(tmp);
}

int CsoundGodot::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    if (finished) {
        return 0;
    }

    lock();

    int read = p_frames;

    if (Time::get_singleton()) {
        last_mix_time = Time::get_singleton()->get_ticks_usec();
    }

    if (!initialized || output_left_channel.buffer == NULL || output_right_channel.buffer == NULL) {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = 0;
            p_buffer[frame].right = 0;
        }
    } else {
        csoundReadCircularBuffer(csound->GetCsound(), output_left_channel.buffer, temp_buffer, p_frames);
        for (int i = 0; i < read; i++) {
            p_buffer[i].left = temp_buffer[i];
        }

        csoundReadCircularBuffer(csound->GetCsound(), output_right_channel.buffer, temp_buffer, p_frames);
        for (int i = 0; i < read; i++) {
            p_buffer[i].right = temp_buffer[i];
        }
    }

    unlock();

    semaphore->post();

    return p_frames;
}

void CsoundGodot::set_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    bool has_left_channel = left >= 0 && left < input_channels.size();
    bool has_right_channel = right >= 0 && right < input_channels.size();

    if (!has_left_channel && !has_right_channel && !active) {
        return;
    }

    lock();

    if (has_left_channel) {
        csoundWriteCircularBuffer(csound->GetCsound(), input_channels.write[left], p_buffer, p_frames);
    }

    if (has_right_channel) {
        csoundWriteCircularBuffer(csound->GetCsound(), input_channels.write[right], p_buffer, p_frames);
    }

    unlock();
}

int CsoundGodot::get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    bool has_left_channel = left >= 0 && left < output_channels.size();
    bool has_right_channel = right >= 0 && right < output_channels.size();

    if (finished) {
        return 0;
    }

    lock();
    if (has_left_channel && active) {
        csoundReadCircularBuffer(csound->GetCsound(), output_channels[left].buffer, temp_buffer, p_frames);
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = temp_buffer[frame];
        }
    } else {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = 0;
        }
    }
    if (has_right_channel && active) {
        csoundReadCircularBuffer(csound->GetCsound(), output_channels[right].buffer, temp_buffer, p_frames);
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].right = temp_buffer[frame];
        }
    } else {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].right = 0;
        }
    }
    unlock();

    return p_frames;
}

void CsoundGodot::set_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left,
                                           String right) {
    bool has_left_channel = input_named_channels_buffer.has(left);
    bool has_right_channel = input_named_channels_buffer.has(right);

    if (!active) {
        return;
    }

    lock();

    if (!has_left_channel) {
        add_named_channel(left);
        has_left_channel = true;
    }

    if (!has_right_channel) {
        add_named_channel(right);
        has_right_channel = true;
    }

    if (has_left_channel) {
        csoundWriteCircularBuffer(csound->GetCsound(), input_named_channels_buffer[left], p_buffer, p_frames);
    }

    if (has_right_channel) {
        csoundWriteCircularBuffer(csound->GetCsound(), input_named_channels_buffer[right], p_buffer, p_frames);
    }

    unlock();
}

int CsoundGodot::get_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right) {
    if (finished) {
        return 0;
    }

    bool has_left_channel = named_channels.has(left);
    bool has_right_channel = named_channels.has(right);

    lock();

    if (!has_left_channel) {
        add_named_channel(left);
    }

    if (!has_right_channel) {
        add_named_channel(right);
    }

    if (has_left_channel && active) {
        csoundReadCircularBuffer(csound->GetCsound(), output_named_channels[named_channels[left]].buffer, temp_buffer,
                                 p_frames);
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = temp_buffer[frame];
        }
    } else {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = 0;
        }
    }
    if (has_right_channel && active) {
        csoundReadCircularBuffer(csound->GetCsound(), output_named_channels[named_channels[right]].buffer, temp_buffer,
                                 p_frames);
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].right = temp_buffer[frame];
        }
    } else {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].right = 0;
        }
    }
    unlock();

    return p_frames;
}

void CsoundGodot::program_select(int chan, int bank_num, int preset_num) {
}

void CsoundGodot::note_on(int chan, int key, int vel) {
    if (!initialized) {
        return;
    }

    MidiEvent event;

    if (vel > 0) {
        event.message = MIDIMessage::MIDI_MESSAGE_NOTE_ON;
    } else {
        event.message = MIDIMessage::MIDI_MESSAGE_NOTE_OFF;
    }
    event.channel = chan;
    event.note = key;
    event.velocity = vel;

    csoundWriteCircularBuffer(csound->GetCsound(), midi_buffer, &event, 4);

    // float instrnum = chan + chan / 100.0 + key / 100000.0;
    // String note_on = vformat("i%f 0 -1 %d %d %d", instrnum, chan, key, vel);
    // csound->EventString(note_on.ascii().get_data(), 0);
}

void CsoundGodot::note_off(int chan, int key) {
    if (!initialized) {
        return;
    }

    MidiEvent event;
    event.message = MIDIMessage::MIDI_MESSAGE_NOTE_OFF;
    event.channel = chan;
    event.note = key;
    event.velocity = 0;

    csoundWriteCircularBuffer(csound->GetCsound(), midi_buffer, &event, 4);

    // float instrnum = chan + chan / 100.0 + key / 100000.0;
    // String note_off = vformat("i-%f 0 %d 0 %d", chan, instrnum, key);
    // csound->EventString(note_off.ascii().get_data(), 0);
}

void CsoundGodot::event_string(String message) {
    if (!initialized) {
        return;
    }

    csound->EventString(message.ascii());
}

double CsoundGodot::evaluate_code(String message) {
    if (!initialized) {
        return 0;
    }

    return csound->EvalCode(message.ascii());
}

void CsoundGodot::compile_csd(String orchestra) {
    if (!initialized) {
        return;
    }

    csound->CompileCSD(orchestra.ascii(), 1);
}

void CsoundGodot::compile_orchestra(String orchestra) {
    if (!initialized) {
        return;
    }

    csound->CompileOrc(orchestra.ascii(), 1);
}

void CsoundGodot::send_control_channel(String channel, float value) {
    if (!initialized) {
        return;
    }

    csound->SetControlChannel(channel.ascii().get_data(), value);
}

float CsoundGodot::get_control_channel(String channel) {
    if (!initialized) {
        return 0;
    }

    return csound->GetControlChannel(channel.ascii().get_data());
}

void CsoundGodot::pitch_bend(int chan, int val) {
}

void CsoundGodot::play_midi(Ref<MidiFileReader> p_midi_file) {
    Ref<MidiFileReader> local_midi_file;

    if (p_midi_file != NULL) {
        if (ResourceLoader::get_singleton()->exists(p_midi_file->get_path())) {
            Variant resource = ResourceLoader::get_singleton()->load(p_midi_file->get_path());
            local_midi_file = resource;
        }
    } else {
        local_midi_file = midi_file;
    }

    if (local_midi_file != NULL) {
        PackedByteArray byte_array = local_midi_file->get_data();
        if (byte_array.size() > 0) {
        }
    }
}

// void CsoundGodot::process(double delta) {
void CsoundGodot::thread_func() {
    int p_frames = 512;

    while (!exit_thread) {
        if (!initialized) {
            continue;
        }

        last_mix_frames = p_frames;

            lock();

            // TODO: enable after fixing memory leak
            // update_named_channels(p_frames);

            int to_fill = p_frames;

            MYFLT *spin = csound->GetSpin();
            const MYFLT *spout = csound->GetSpout();
            MYFLT scale = csound->Get0dBFS();

            float volume = godot::UtilityFunctions::db_to_linear(volume_db);

            if (CsoundServer::get_singleton()->get_solo_mode()) {
                if (!solo) {
                    volume = 0.0;
                }
            } else {
                if (mute) {
                    volume = 0.0;
                }
            }

            Vector<float> channel_peak;
            channel_peak.resize(output_channels.size());
            for (int i = 0; i < output_channels.size(); i++) {
                channel_peak.write[i] = 0;
            }
            Vector<float> named_channel_peak;
            named_channel_peak.resize(output_named_channels.size());
            for (int i = 0; i < output_named_channels.size(); i++) {
                named_channel_peak.write[i] = 0;
            }

            while (to_fill > 0) {
                for (int channel = 0; channel < csound->GetChannels(0); channel++) {
                    csoundReadCircularBuffer(csound->GetCsound(), input_channels[channel], ksmps_temp_buffer.ptrw(),
                                             csound->GetKsmps());
                    for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                        ksmps_temp_buffer.write[frame] = ksmps_temp_buffer[frame] * scale;
                        spin[frame * csound->GetChannels(0) + channel] = ksmps_temp_buffer[frame];
                    }
                    if (bypass) {
                        csoundWriteCircularBuffer(csound->GetCsound(), output_channels.write[channel].buffer,
                                                  ksmps_temp_buffer.ptr(), csound->GetKsmps());
                    }
                }

                for (KeyValue<String, void *> &E : input_named_channels_buffer) {
                    csoundReadCircularBuffer(csound->GetCsound(), input_named_channels_buffer[E.key],
                                             ksmps_temp_buffer.ptrw(), csound->GetKsmps());
                    for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                        ksmps_temp_buffer.write[frame] = ksmps_temp_buffer[frame] * scale;
                    }
                    csound->SetAudioChannel(E.key.utf8().get_data(), ksmps_temp_buffer.ptrw());
                    if (bypass) {
                        csoundWriteCircularBuffer(csound->GetCsound(),
                                                  output_named_channels.write[named_channels[E.key]].buffer,
                                                  ksmps_temp_buffer.ptr(), csound->GetKsmps());
                    }
                }

                int result = csound->PerformKsmps();
                if (result == 1) {
                    finished = true;
                }

                int buffer_index = 0;
                for (int i = 0; i < csound->GetKsmps() * csound->GetChannels(0); i = i + csound->GetChannels(0)) {
                    if (bypass) {
                        ksmps_left_buffer.ptrw()[buffer_index] = spin[i] * scale;
                        ksmps_right_buffer.ptrw()[buffer_index] = spin[i + 1] * scale;
                    } else {
                        ksmps_left_buffer.ptrw()[buffer_index] = spout[i] / scale * volume;
                        ksmps_right_buffer.ptrw()[buffer_index] = spout[i + 1] / scale * volume;
                    }

                    to_fill = to_fill - 1;
                    buffer_index++;
                }

                csoundWriteCircularBuffer(csound->GetCsound(), output_left_channel.buffer,
                                          ksmps_left_buffer.ptr(), csound->GetKsmps());

                csoundWriteCircularBuffer(csound->GetCsound(), output_right_channel.buffer,
                                          ksmps_right_buffer.ptr(), csound->GetKsmps());

                if (!bypass) {
                    for (int channel = 0; channel < csound->GetChannels(0); channel++) {
                        for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                            int index = (frame * csound->GetChannels(0)) + channel;
                            float value = spout[index] / scale * volume;
                            float p = ABS(value);
                            if (p > channel_peak[channel]) {
                                channel_peak.write[channel] = p;
                            }
                            ksmps_temp_buffer.write[frame] = value;
                        }
                        csoundWriteCircularBuffer(csound->GetCsound(), output_channels.write[channel].buffer,
                                                  ksmps_temp_buffer.ptr(), csound->GetKsmps());
                    }
                }

                if (!bypass) {
                    for (int channel = 0; channel < output_named_channels.size(); channel++) {
                        csound->GetAudioChannel(output_named_channels[channel].name.utf8().get_data(),
                                                ksmps_temp_buffer.ptrw());
                        for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                            float value = ksmps_temp_buffer[frame] / scale * volume;
                            float p = ABS(value);
                            if (p > named_channel_peak[channel]) {
                                named_channel_peak.write[channel] = p;
                            }
                            ksmps_temp_buffer.write[frame] = value;
                        }
                        csoundWriteCircularBuffer(csound->GetCsound(), output_named_channels.write[channel].buffer,
                                                  ksmps_temp_buffer.ptr(), csound->GetKsmps());

                        // clear channel after reading
                        for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                            ksmps_temp_buffer.write[frame] = 0;
                        }
                        csound->SetAudioChannel(output_named_channels[channel].name.utf8().get_data(),
                                                ksmps_temp_buffer.ptrw());
                    }
                }
            }

            for (int i = 0; i < output_channels.size(); i++) {
                output_channels.write[i].peak_volume =
                    godot::UtilityFunctions::linear_to_db(channel_peak[i] + AUDIO_PEAK_OFFSET);

                if (channel_peak[i] > 0) {
                    output_channels.write[i].active = true;
                } else {
                    output_channels.write[i].active = false;
                }
            }

            for (int i = 0; i < output_named_channels.size(); i++) {
                output_named_channels.write[i].peak_volume =
                    godot::UtilityFunctions::linear_to_db(named_channel_peak[i] + AUDIO_PEAK_OFFSET);

                if (named_channel_peak[i] > 0) {
                    output_named_channels.write[i].active = true;
                } else {
                    output_named_channels.write[i].active = false;
                }
            }



        // TODO: can I remove this now that it's running on a thread?
        /*
        if (!active && !channels_cleared) {
            for (int channel = 0; channel < input_channels.size(); channel++) {
                for (int frame = 0; frame < last_mix_frames; frame++) {
                    // input_channels.write[channel].write[frame] = 0;
                }
            }

            for (int channel = 0; channel < output_channels.size(); channel++) {
                output_channels.write[channel].active = false;
                output_channels.write[channel].peak_volume = AUDIO_MIN_PEAK_DB;

                for (int frame = 0; frame < last_mix_frames; frame++) {
                    // output_channels.write[channel].buffer.write[frame] = 0;
                }
            }

            for (KeyValue<String, void *> &E : input_named_channels_buffer) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    String key = E.key;
                    // input_named_channels_buffer.get(E.key).write[frame] = 0;
                }
            }

            for (int channel = 0; channel < output_named_channels.size(); channel++) {
                output_named_channels.write[channel].active = false;
                output_named_channels.write[channel].peak_volume = AUDIO_MIN_PEAK_DB;

                for (int frame = 0; frame < last_mix_frames; frame++) {
                    // output_named_channels.write[channel].buffer.write[frame] = 0;
                }
            }

            channels_cleared = true;
        }
        */

        unlock();

        semaphore->wait();
    }
}

Error CsoundGodot::start_thread() {
    if (thread.is_null()) {
        thread.instantiate();
        exit_thread = false;
        thread->start(callable_mp(this, &CsoundGodot::thread_func), Thread::PRIORITY_HIGH);
    }
    return (Error)OK;
}

void CsoundGodot::stop_thread() {
    if (thread.is_valid()) {
        exit_thread = true;
        semaphore->post();
        thread->wait_to_finish();
        thread.unref();
    }
}

void CsoundGodot::lock() {
    if (thread.is_null() || mutex.is_null()) {
        return;
    }
    mutex->lock();
}

void CsoundGodot::unlock() {
    if (thread.is_null() || mutex.is_null()) {
        return;
    }
    mutex->unlock();
}

void CsoundGodot::initialize() {
    start();
}

int CsoundGodot::open_midi_device(CSOUND *csound, void **userData, const char *dev) {
    for (int i = 0; i < CsoundServer::get_singleton()->get_csound_count(); i++) {
        CsoundGodot *csound_godot = CsoundServer::get_singleton()->get_csound_by_index(i);
        if (csound_godot->csound != NULL && csound_godot->csound->GetCsound() == csound) {
            *userData = (void *)csound_godot;
            return 0;
        }
    }

    return -1;
}

int CsoundGodot::write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes) {

    return 0;
}

int CsoundGodot::read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    CsoundGodot *csound_godot = (CsoundGodot *)userData;

    if (!csound_godot->initialized) {
        return 0;
    }

    //int bytesLeft = nbytes;
    int bytesRead = 0;

    MidiEvent midi_event;

    int32_t read = csoundReadCircularBuffer(csound, csound_godot->midi_buffer, &midi_event, 4);

    while (read > 0 && bytesRead < nbytes) {
        //int channel = midi_event.channel;
        int byte1 = midi_event.message << 4 | midi_event.channel % 16;
        int byte2 = 0x80 | midi_event.channel / 16;
        int byte3 = midi_event.note;
        int byte4 = midi_event.velocity;

        mbuf[bytesRead++] = byte1;
        mbuf[bytesRead++] = byte2;
        mbuf[bytesRead++] = byte3;
        mbuf[bytesRead++] = byte4;

        read = csoundReadCircularBuffer(csound, csound_godot->midi_buffer, &midi_event, 4);
    }

    return bytesRead;
}

void CsoundGodot::set_message(CSOUND *, int attr, const char *format, va_list valist) {
    std::string message;
    va_list valist_copy;
    va_copy(valist_copy, valist);
    size_t len = vsnprintf(0, 0, format, valist_copy);
    message.resize(len + 1);
    vsnprintf(&message[0], len + 1, format, valist);
    message.resize(len);
    godot::UtilityFunctions::print(message.c_str());
    va_end(valist_copy);
}

FILE *CsoundGodot::open_file(CSOUND *csound, const char *filename, const char *mode) {
    String node_path = filename;
    if (ResourceLoader::get_singleton()->exists(node_path)) {
        Variant resource = ResourceLoader::get_singleton()->load(node_path);
        Ref<CsoundFileReader> csound_file = resource;
        if (csound_file != NULL) {
            FILE *fp = fmemopen(NULL, csound_file->get_array_size() + 1, "w+");
            char *data = csound_file->get_array_data();
            for (int i = 0; i < csound_file->get_array_size(); i++) {
                fprintf(fp, "%c", data[i]);
            }
            fflush(fp);
            rewind(fp);
            return fp;
        }

        Ref<SoundFontFileReader> soundfont_file = resource;
        if (soundfont_file != NULL) {
            FILE *fp = fmemopen(NULL, soundfont_file->get_array_size(), "w+");
            char *data = soundfont_file->get_array_data();
            for (int i = 0; i < soundfont_file->get_array_size(); i++) {
                fprintf(fp, "%c", data[i]);
            }
            fflush(fp);
            rewind(fp);
            return fp;
        }

        Ref<AudioStreamMP3> mp3_file = resource;
        if (mp3_file != NULL) {
            FILE *fp = fmemopen(NULL, mp3_file->get_data().size(), "w+");
            char *data = (char *)mp3_file->get_data().ptr();
            for (int i = 0; i < mp3_file->get_data().size(); i++) {
                fprintf(fp, "%c", data[i]);
            }
            fflush(fp);
            rewind(fp);
            return fp;
        }
    }

    return NULL;
}

void *CsoundGodot::open_sound_file(CSOUND *csound, const char *filename, int mode, void *userdata) {
    String node_path = filename;
    SFLIB_INFO *sfinfo = (SFLIB_INFO *)userdata;

    if (ResourceLoader::get_singleton()->exists(node_path)) {
        Variant resource = ResourceLoader::get_singleton()->load(node_path);

        Ref<AudioStreamMP3> mp3_file = resource;

        if (mp3_file != NULL) {
            int size = mp3_file->get_data().size();
            PackedByteArray byte_array = mp3_file->get_data();
            char *txt = (char *)calloc(size, sizeof(char));

            for (int i = 0; i < size; i++) {
                txt[i] = byte_array.ptr()[i];
            }

            MemoryFile *file = new MemoryFile{txt, (sf_count_t)size, 0};
            SF_VIRTUAL_IO *vio = new SF_VIRTUAL_IO;
            vio->get_filelen = vio_get_filelen;
            vio->seek = vio_seek;
            vio->read = vio_read;
            vio->write = vio_write;
            vio->tell = vio_tell;

            SNDFILE *handle;
            SF_INFO *info = new SF_INFO;

            if (mode == SFM_WRITE) {
                info->samplerate = sfinfo->samplerate;
                info->channels = sfinfo->channels;
                info->format = sfinfo->format;
            }
            handle = sf_open_virtual(vio, mode, info, file);

            //int error =
            sf_error(handle);

            if (mode == SFM_READ) {
                sfinfo->samplerate = info->samplerate;
                sfinfo->channels = info->channels;
                sfinfo->format = info->format;
                sfinfo->frames = info->frames;
            }

            return handle;
        }
    }

    return NULL;
}

void CsoundGodot::set_csound_name(const String &name) {
    csound_name = name;
}

const String &CsoundGodot::get_csound_name() {
    return csound_name;
}

void CsoundGodot::set_csound_script(Ref<CsoundFileReader> p_script) {
    reset();
    script = p_script;
    start();
}

Ref<CsoundFileReader> CsoundGodot::get_csound_script() {
    return script;
}

int CsoundGodot::get_channel_count() {
    if (csound != NULL) {
        return csound->GetChannels(0);
    } else {
        return 2;
    }
}

int CsoundGodot::get_named_channel_count() {
    if (csound != NULL) {
        return output_named_channels.size();
    } else {
        return 0;
    }
}

String CsoundGodot::get_named_channel_name(int p_channel) {
    ERR_FAIL_INDEX_V(p_channel, output_named_channels.size(), "");

    return output_named_channels[p_channel].name;
}

double CsoundGodot::get_time_since_last_mix() {
    return (Time::get_singleton()->get_ticks_usec() - last_mix_time) / 1000000.0;
}

double CsoundGodot::get_time_to_next_mix() {
    double total = get_time_since_last_mix();
    double mix_buffer = last_mix_frames / AudioServer::get_singleton()->get_mix_rate();
    return mix_buffer - total;
}

sf_count_t CsoundGodot::vio_get_filelen(void *user_data) {
    MemoryFile *file = (MemoryFile *)user_data;
    return file->length;
}

sf_count_t CsoundGodot::vio_seek(sf_count_t offset, int whence, void *user_data) {
    MemoryFile *file = (MemoryFile *)user_data;
    sf_count_t newpos = 0;
    switch (whence) {
    case SEEK_SET:
        newpos = offset;
        break;
    case SEEK_CUR:
        newpos = file->curpos + offset;
        break;
    case SEEK_END:
        newpos = file->length - offset;
        break;
    }
    if ((newpos >= 0) && (newpos < file->length)) {
        file->curpos = newpos;
    }
    return file->curpos;
}

sf_count_t CsoundGodot::vio_read(void *ptr, sf_count_t count, void *user_data) {
    MemoryFile *file = (MemoryFile *)user_data;
    if (count + file->curpos > file->length) {
        count = file->length - file->curpos;
    }
    if (count > 0) {
        memcpy(ptr, file->buffer + file->curpos, count);
    }
    file->curpos += count;
    return count;
}

sf_count_t CsoundGodot::vio_write(const void *ptr, sf_count_t count, void *user_data) {
    return 0;
}

sf_count_t CsoundGodot::vio_tell(void *user_data) {
    MemoryFile *file = (MemoryFile *)user_data;
    return file->curpos;
}

void CsoundGodot::set_active(bool p_active) {
    active = p_active;
    channels_cleared = false;
}

bool CsoundGodot::is_active() {
    return active;
}

void CsoundGodot::set_value(double key, double value) {
    csound_data[key] = value;
}

const double CsoundGodot::get_value(double key) {
    return csound_data[key];
}

void CsoundGodot::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &CsoundGodot::initialize);
    ClassDB::bind_method(D_METHOD("program_select"), &CsoundGodot::program_select);
    ClassDB::bind_method(D_METHOD("finish"), &CsoundGodot::finish);

    ClassDB::bind_method(D_METHOD("note_on"), &CsoundGodot::note_on);
    ClassDB::bind_method(D_METHOD("note_off"), &CsoundGodot::note_off);

    ClassDB::bind_method(D_METHOD("event_string", "message"), &CsoundGodot::event_string);
    ClassDB::bind_method(D_METHOD("evaluate_code", "message"), &CsoundGodot::evaluate_code);
    ClassDB::bind_method(D_METHOD("compile_csd", "orchestra"), &CsoundGodot::compile_csd);
    ClassDB::bind_method(D_METHOD("compile_orchestra", "orchestra"), &CsoundGodot::compile_orchestra);

    ClassDB::bind_method(D_METHOD("send_control_channel", "value"), &CsoundGodot::send_control_channel);
    ClassDB::bind_method(D_METHOD("get_control_channel"), &CsoundGodot::get_control_channel);

    ClassDB::bind_method(D_METHOD("pitch_bend"), &CsoundGodot::pitch_bend);
    ClassDB::bind_method(D_METHOD("play_midi"), &CsoundGodot::play_midi);
    ClassDB::bind_method(D_METHOD("set_soundfont", "soundfont"), &CsoundGodot::set_soundfont);
    ClassDB::bind_method(D_METHOD("get_soundfont"), &CsoundGodot::get_soundfont);
    ClassDB::add_property(
        "CsoundGodot", PropertyInfo(Variant::OBJECT, "soundfont", PROPERTY_HINT_RESOURCE_TYPE, "SoundFontFileReader"),
        "set_soundfont", "get_soundfont");
    ClassDB::bind_method(D_METHOD("set_midi_file", "midi_file"), &CsoundGodot::set_midi_file);
    ClassDB::bind_method(D_METHOD("get_midi_file"), &CsoundGodot::get_midi_file);
    ClassDB::add_property("CsoundGodot",
                          PropertyInfo(Variant::OBJECT, "midi_file", PROPERTY_HINT_RESOURCE_TYPE, "MidiFileReader"),
                          "set_midi_file", "get_midi_file");
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &CsoundGodot::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &CsoundGodot::get_csound_name);

    ClassDB::bind_method(D_METHOD("set_csound_script", "script"), &CsoundGodot::set_csound_script);
    ClassDB::bind_method(D_METHOD("get_csound_script"), &CsoundGodot::get_csound_script);

    ClassDB::add_property("CsoundGodot", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");

    ADD_SIGNAL(MethodInfo("csound_ready", PropertyInfo(Variant::STRING, "csound_name")));
}
