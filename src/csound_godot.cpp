#include "csound_godot.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

CsoundGodot::CsoundGodot() {
    csound = NULL;
    initialized = false;
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
    csound->Compile("test.csd");

    csound->CreateMessageBuffer(0);
    csound->SetDebug(true);

    csound->SetHostImplementedAudioIO(1, 0);

    // csound->SetHostImplementedMIDIIO(true);
    // csound->SetExternalMidiWriteCallback(write_midi_data);
    // csound->SetExternalMidiReadCallback(read_midi_data);

    csound->Start();

    set_process_internal(true);
    initialized = true;
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

    if (channels.size() != csound->GetNchnls()) {
        channels.resize(csound->GetNchnls());

        for (int j = 0; j < csound->GetNchnls(); j++) {
            channels.write[j].buffer.resize(p_frames);
        }
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
        return p_frames;
    }

    int p_index = 0;
    int to_fill = p_frames;

    MYFLT *spin = csound->GetSpin();
    MYFLT *spout = csound->GetSpout();
    MYFLT scale = csound->Get0dBFS();

    initialize_channels(p_frames);

    int write_buffer_index = 0;

    while (to_fill > 0) {
        for (KeyValue<String, Vector<MYFLT>> &E : named_channel_input_buffers) {
            Vector<MYFLT> temp_buffer;
            temp_buffer.resize(csound->GetKsmps());
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                temp_buffer.write[frame] = named_channel_input_buffers.get(E.key)[write_buffer_index + frame];
            }
            csound->SetAudioChannel(E.key.utf8().get_data(), temp_buffer.ptrw());
        }

        int result = csound->PerformKsmps();
        if (result == 1) {
            finished = true;
        }

        for (int i = 0; i < csound->GetKsmps() * csound->GetNchnls(); i = i + csound->GetNchnls()) {
            p_buffer[p_index].left = spout[i] / scale;
            p_buffer[p_index].right = spout[i + 1] / scale;

            p_index = p_index + 1;
            to_fill = to_fill - 1;
        }

        for (int index = 0; index < csound->GetNchnls(); index++) {
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                if (write_buffer_index + frame < channels[index].buffer.size()) {
                    channels.write[index].buffer.write[write_buffer_index + frame] =
                        csound->GetSpoutSample(frame, index) / scale;
                }
            }
        }

        for (KeyValue<String, Vector<MYFLT>> &E : named_channel_output_buffers) {
            MYFLT temp_buffer[csound->GetKsmps()];
            csound->GetAudioChannel(E.key.utf8().get_data(), temp_buffer);
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                named_channel_output_buffers.getptr(E.key)->write[write_buffer_index + frame] = temp_buffer[frame];
            }
        }

        write_buffer_index += csound->GetKsmps();
    }

    return p_frames;
}

int CsoundGodot::get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    for (int frame = 0; frame < p_frames; frame += 1) {
        if (left < channels.size() && frame < channels[left].buffer.size()) {
            p_buffer[frame].left = channels[left].buffer[frame];
        } else {
            p_buffer[frame].left = 0;
        }
        if (right < channels.size() && frame < channels[right].buffer.size()) {
            p_buffer[frame].right = channels[right].buffer[frame];
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
