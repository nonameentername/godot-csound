#include "csound_godot.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/classes/time.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

CsoundGodot::CsoundGodot() {
    csound = NULL;
    initialized = false;
    active = false;
}

CsoundGodot::~CsoundGodot() {
    if (csound != NULL) {
        delete csound;
        csound = NULL;
    }
}

void CsoundGodot::_ready() {
    finished = false;
    csound = new Csound();

    csound->CreateMessageBuffer(0);
    csound->SetDebug(true);
    csound->SetHostImplementedAudioIO(1, 0);

    // csound->SetHostImplementedMIDIIO(true);
    // csound->SetExternalMidiWriteCallback(write_midi_data);
    // csound->SetExternalMidiReadCallback(read_midi_data);

    set_process_internal(true);

    start();
}

void CsoundGodot::start() {
    csound->Compile("test.csd");
    csound->Start();

    if (output_channels.size() != csound->GetNchnls()) {
        input_channels.resize(csound->GetNchnls());
        output_channels.resize(csound->GetNchnls());

        int p_frames = 512;
        for (int j = 0; j < csound->GetNchnls(); j++) {
            input_channels.write[j].resize(p_frames);
            output_channels.write[j].buffer.resize(p_frames);

            for (int frame = 0; frame < p_frames; frame++) {
                input_channels.write[j].write[frame] = 0;
                output_channels.write[j].buffer.write[frame] = 0;
            }
        }
    }

    initialized = true;
}

void CsoundGodot::stop() {
    if (csound != NULL) {
        csound->Stop();
    }
}

void CsoundGodot::reset() {
    if (csound != NULL) {
        csound->Reset();
    }

    input_channels.clear();
    output_channels.clear();
    named_channel_input_buffers.clear();
    named_channel_output_buffers.clear();
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

void CsoundGodot::initialize_channels(int p_frames) {
    if (!initialized) {
        return;
    }

    controlChannelInfo_t *tmp;
    int num_channels = csound->ListChannels(tmp);

    for (int channel = 0; channel < num_channels; channel++) {
        char *name = tmp[channel].name;
        int type = tmp[channel].type;
        if ((type & CSOUND_CHANNEL_TYPE_MASK) == CSOUND_AUDIO_CHANNEL &&
            (type & (CSOUND_INPUT_CHANNEL | CSOUND_OUTPUT_CHANNEL))) {
            if (!named_channel_input_buffers.has(name)) {
                Vector<MYFLT> channel_buffer;
                channel_buffer.resize(p_frames);
                named_channel_input_buffers.insert(name, channel_buffer);
            }
            if (!named_channel_output_buffers.has(name)) {
                Vector<MYFLT> channel_buffer;
                channel_buffer.resize(p_frames);
                named_channel_output_buffers.insert(name, channel_buffer);
            }
        }
    }
}

int CsoundGodot::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    if (!initialized) {
        for (int frame = 0; frame < p_frames; frame++) {
            p_buffer[frame].left = 0;
            p_buffer[frame].right = 0;
        }
        return p_frames;
    }

    active = true;

    if (Time::get_singleton()) {
        last_mix_time = Time::get_singleton()->get_ticks_usec();
    }

    last_mix_frames = p_frames;

    int buffer_index = 0;
    int to_fill = p_frames;

    MYFLT *spin = csound->GetSpin();
    MYFLT *spout = csound->GetSpout();
    MYFLT scale = csound->Get0dBFS();

    initialize_channels(p_frames);

    int ksmps_buffer_index = 0;

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

    float peak[output_channels.size()];
    for (int i = 0; i < output_channels.size(); i++) {
        peak[i] = 0;
    }

    while (to_fill > 0) {
        for (int frame = 0; frame < csound->GetKsmps(); frame++) {
            for (int channel = 0; channel < csound->GetNchnls(); channel++) {
                spin[frame * csound->GetNchnls() + channel] =
                    input_channels[channel][ksmps_buffer_index + frame] * scale;
            }
        }

        for (KeyValue<String, Vector<MYFLT>> &E : named_channel_input_buffers) {
            Vector<MYFLT> temp_buffer;
            temp_buffer.resize(csound->GetKsmps());
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                temp_buffer.write[frame] = named_channel_input_buffers.get(E.key)[ksmps_buffer_index + frame] * scale;
            }
            csound->SetAudioChannel(E.key.utf8().get_data(), temp_buffer.ptrw());
        }

        int result = csound->PerformKsmps();
        if (result == 1) {
            finished = true;
        }

        for (int i = 0; i < csound->GetKsmps() * csound->GetNchnls(); i = i + csound->GetNchnls()) {
            if (bypass) {
                p_buffer[buffer_index].left = spin[i] * scale;
                p_buffer[buffer_index].right = spin[i + 1] * scale;
            } else {
                p_buffer[buffer_index].left = spout[i] / scale * volume;
                p_buffer[buffer_index].right = spout[i + 1] / scale * volume;
            }

            buffer_index = buffer_index + 1;
            to_fill = to_fill - 1;
        }

        if (bypass) {
            for (int channel = 0; channel < csound->GetNchnls(); channel++) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    output_channels.write[channel].buffer.write[ksmps_buffer_index + frame] =
                        input_channels[channel][ksmps_buffer_index + frame];
                }
            }
        } else {
            for (int channel = 0; channel < csound->GetNchnls(); channel++) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    float value = csound->GetSpoutSample(frame, channel) / scale * volume;
                    float p = ABS(value);
                    if (p > peak[channel]) {
                        peak[channel] = p;
                    }
                    output_channels.write[channel].buffer.write[ksmps_buffer_index + frame] = value;
                }
            }
        }

        if (bypass) {
            for (KeyValue<String, Vector<MYFLT>> &E : named_channel_output_buffers) {
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    named_channel_output_buffers.getptr(E.key)->write[ksmps_buffer_index + frame] = 0;
                }
            }
        } else {
            for (KeyValue<String, Vector<MYFLT>> &E : named_channel_output_buffers) {
                MYFLT temp_buffer[csound->GetKsmps()];
                csound->GetAudioChannel(E.key.utf8().get_data(), temp_buffer);
                for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                    named_channel_output_buffers.getptr(E.key)->write[ksmps_buffer_index + frame] =
                        temp_buffer[frame] / scale * volume;
                }
            }
        }

        ksmps_buffer_index += csound->GetKsmps();
    }

    for (int i = 0; i < output_channels.size(); i++) {
        output_channels.write[i].peak_volume = godot::UtilityFunctions::linear_to_db(peak[i] + AUDIO_PEAK_OFFSET);

        if (peak[i] > 0) {
            output_channels.write[i].active = true;
        } else {
            output_channels.write[i].active = false;
        }
    }

    return p_frames;
}

void CsoundGodot::set_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    if (left < 0 || left >= input_channels.size() || right < 0 || right >= input_channels.size()) {
        return;
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        input_channels.write[left].write[frame] = p_buffer[frame].left;
        input_channels.write[right].write[frame] = p_buffer[frame].right;
    }
}

int CsoundGodot::get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    for (int frame = 0; frame < p_frames; frame += 1) {
        if (left >= 0 && left < output_channels.size() && frame < output_channels[left].buffer.size()) {
            p_buffer[frame].left = output_channels[left].buffer[frame];
        } else {
            p_buffer[frame].left = 0;
        }
        if (right >= 0 && right < output_channels.size() && frame < output_channels[right].buffer.size()) {
            p_buffer[frame].right = output_channels[right].buffer[frame];
        } else {
            p_buffer[frame].right = 0;
        }
    }

    return p_frames;
}

void CsoundGodot::set_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left,
                                           String right) {
    if (!named_channel_input_buffers.has(left) || !named_channel_input_buffers.has(right)) {
        return;
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        named_channel_input_buffers.getptr(left)->write[frame] = p_buffer[frame].left;
        named_channel_input_buffers.getptr(right)->write[frame] = p_buffer[frame].right;
    }
}

int CsoundGodot::get_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right) {
    if (!named_channel_output_buffers.has(left) || !named_channel_output_buffers.has(right)) {
        for (int frame = 0; frame < p_frames; frame += 1) {
            p_buffer[frame].left = 0;
            p_buffer[frame].right = 0;
        }

        return p_frames;
    }

    const MYFLT *left_buffer = named_channel_output_buffers.get(left).ptr();
    const MYFLT *right_buffer = named_channel_output_buffers.get(right).ptr();

    for (int frame = 0; frame < p_frames; frame += 1) {
        if (left_buffer) {
            p_buffer[frame].left = left_buffer[frame];
        } else {
            p_buffer[frame].left = 0;
        }
        if (right_buffer) {
            p_buffer[frame].right = right_buffer[frame];
        } else {
            p_buffer[frame].right = 0;
        }
    }

    return p_frames;
}

void CsoundGodot::program_select(int chan, int bank_num, int preset_num) {
}

void CsoundGodot::note_on(int chan, int key, int vel) {
    if (!initialized) {
        return;
    }

    float instrnum = chan + chan / 100.0 + key / 100000.0;
    String note_on = vformat("i%f 0 -1 %d %d", instrnum, key, vel);
    csound->InputMessage(note_on.ascii().get_data());
}

void CsoundGodot::note_off(int chan, int key) {
    if (!initialized) {
        return;
    }

    float instrnum = chan + chan / 100.0 + key / 100000.0;
    String note_off = vformat("i-%f 0 0 %d", instrnum, key);
    csound->InputMessage(note_off.ascii().get_data());
}

void CsoundGodot::send_control_channel(String channel, float value) {
    if (!initialized) {
        return;
    }

    csound->SetControlChannel(channel.ascii().get_data(), value);
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

void CsoundGodot::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_INTERNAL_PROCESS: {
        process(get_process_delta_time());
    }
    }
}

void CsoundGodot::process(double delta) {
    if (!initialized) {
        return;
    }

    if (active) {
        double time_to_future_mix = 2 * get_time_to_next_mix();

        if (time_to_future_mix < 0) {
            active = false;

            for (int channel = 0; channel < output_channels.size(); channel++) {
                output_channels.write[channel].active = false;
                output_channels.write[channel].peak_volume = AUDIO_MIN_PEAK_DB;
            }
        }
    }

    for (int i = 0; i < csound->GetMessageCnt(); i++) {
        godot::UtilityFunctions::printraw(csound->GetFirstMessage());
        csound->PopFirstMessage();
    }
}

int CsoundGodot::write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes) {
    return 0;
}

int CsoundGodot::read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    return 0;
}

void CsoundGodot::set_csound_name(const String &name) {
    csound_name = name;
}

const String &CsoundGodot::get_csound_name() {
    return csound_name;
}

int CsoundGodot::get_channel_count() {
    if (csound != NULL) {
        return csound->GetNchnls();
    } else {
        return 2;
    }
}

double CsoundGodot::get_time_since_last_mix() {
    return (Time::get_singleton()->get_ticks_usec() - last_mix_time) / 1000000.0;
}

double CsoundGodot::get_time_to_next_mix() {
    double total = get_time_since_last_mix();
    double mix_buffer = last_mix_frames / AudioServer::get_singleton()->get_mix_rate();
    return mix_buffer - total;
}

void CsoundGodot::_bind_methods() {
    ClassDB::bind_method(D_METHOD("process", "delta"), &CsoundGodot::process);
    ClassDB::bind_method(D_METHOD("program_select"), &CsoundGodot::program_select);
    ClassDB::bind_method(D_METHOD("note_on"), &CsoundGodot::note_on);
    ClassDB::bind_method(D_METHOD("note_off"), &CsoundGodot::note_off);
    ClassDB::bind_method(D_METHOD("send_control_channel"), &CsoundGodot::send_control_channel);
    ClassDB::bind_method(D_METHOD("pitch_bend"), &CsoundGodot::pitch_bend);
    ClassDB::bind_method(D_METHOD("play_midi"), &CsoundGodot::play_midi, DEFVAL(NULL));
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
    ClassDB::add_property("CsoundGodot", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");
}
