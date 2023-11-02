#include "csound_godot.h"
#include "csound_engine.h"
#include "godot_cpp/classes/audio_server.hpp"
#include "godot_cpp/variant/variant.hpp"

using namespace godot;

CsoundGodot::CsoundGodot() {
    csound = NULL;
}

CsoundGodot::~CsoundGodot() {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        csound_engine->remove(csound_name);
    }

    if (csound != NULL) {
        delete csound;
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

    CsoundEngine *csound_engine = Object::cast_to<CsoundEngine>(Engine::get_singleton()->get_singleton("Csound"));
    if (csound_engine != NULL) {
        csound_engine->add(csound_name, this);
    }

    set_process_internal(true);
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
    if (channel_buffers.size() != csound->GetNchnls()) {
        for (int channel = 0; channel < csound->GetNchnls(); channel++) {
            Vector<MYFLT> channel_buffer;
            channel_buffer.resize(p_frames);
            channel_buffers.append(channel_buffer);
        }
    }

    controlChannelInfo_t *tmp;
    int num_channels = csound->ListChannels(tmp);

    for (int channel = 0; channel < num_channels; channel++) {
        char *name = tmp[channel].name;
        int type = tmp[channel].type;
        if ((type & CSOUND_CHANNEL_TYPE_MASK) == CSOUND_AUDIO_CHANNEL &&
            (type & (CSOUND_INPUT_CHANNEL | CSOUND_OUTPUT_CHANNEL))) {
            if (!named_channel_buffers.has(name)) {
                Vector<MYFLT> channel_buffer;
                channel_buffer.resize(p_frames);
                named_channel_buffers.insert(name, channel_buffer);
                // godot::UtilityFunctions::print("name = ", tmp[channel].name, " type = ", tmp[channel].type);
            }
        }
    }
}

int CsoundGodot::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    int p_index = 0;
    int to_fill = p_frames;

    MYFLT *spin = csound->GetSpin();
    MYFLT *spout = csound->GetSpout();
    MYFLT scale = csound->Get0dBFS();

    initialize_channels(p_frames);

    int write_buffer_index = 0;

    while (to_fill > 0) {
        for (KeyValue<String, Vector<MYFLT>> &E : named_channel_buffers) {
            MYFLT temp_buffer[csound->GetKsmps()];
            Vector<MYFLT> channel_buffer = named_channel_buffers.get(E.key);
            const MYFLT *buffer = channel_buffer.ptr();
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                if (buffer) {
                    temp_buffer[frame] = buffer[write_buffer_index + frame];
                }
            }
            csound->SetAudioChannel(E.key.utf8().get_data(), temp_buffer);
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

        for (int channel = 0; channel < csound->GetNchnls(); channel++) {
            Vector<MYFLT> *channel_buffer = channel_buffers.ptrw();
            MYFLT *buffer = channel_buffer->ptrw();
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                buffer[write_buffer_index + frame] = csound->GetSpoutSample(frame, channel) / scale;
            }
        }

        for (KeyValue<String, Vector<MYFLT>> &E : named_channel_buffers) {
            MYFLT temp_buffer[csound->GetKsmps()];
            csound->GetAudioChannel(E.key.utf8().get_data(), temp_buffer);
            Vector<MYFLT> *channel_buffer = named_channel_buffers.getptr(E.key);
            MYFLT *buffer = channel_buffer->ptrw();
            for (int frame = 0; frame < csound->GetKsmps(); frame++) {
                buffer[write_buffer_index + frame] = temp_buffer[frame];
            }
        }

        write_buffer_index += csound->GetKsmps();
    }

    return p_frames;
}

int CsoundGodot::get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right) {
    if (left > channel_buffers.size() || right > channel_buffers.size()) {
        return p_frames;
    }

    const MYFLT *left_buffer = channel_buffers.get(left).ptr();
    const MYFLT *right_buffer = channel_buffers.get(right).ptr();

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

void CsoundGodot::set_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left,
                                           String right) {
    if (!named_channel_buffers.has(left) || !named_channel_buffers.has(right)) {
        return;
    }

    MYFLT *left_buffer = named_channel_buffers.getptr(left)->ptrw();
    MYFLT *right_buffer = named_channel_buffers.getptr(right)->ptrw();

    for (int frame = 0; frame < p_frames; frame += 1) {
        if (left_buffer) {
            left_buffer[frame] = p_buffer[frame].left;
        } else {
            left_buffer[frame] = 0;
        }
        if (right_buffer) {
            right_buffer[frame] = p_buffer[frame].right;
        } else {
            right_buffer[frame] = 0;
        }
    }
}

int CsoundGodot::get_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right) {
    if (!named_channel_buffers.has(left) || !named_channel_buffers.has(right)) {
        return p_frames;
    }

    const MYFLT *left_buffer = named_channel_buffers.get(left).ptr();
    const MYFLT *right_buffer = named_channel_buffers.get(right).ptr();

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
    float instrnum = chan + chan / 100.0 + key / 100000.0;
    String note_on = vformat("i%f 0 -1 %d %d", instrnum, key, vel);
    csound->InputMessage(note_on.ascii().get_data());
}

void CsoundGodot::note_off(int chan, int key) {
    float instrnum = chan + chan / 100.0 + key / 100000.0;
    String note_off = vformat("i-%f 0 0 %d", instrnum, key);
    csound->InputMessage(note_off.ascii().get_data());
}

void CsoundGodot::send_control_channel(String channel, float value) {
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
}

int CsoundGodot::write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes) {
    return 0;
}

int CsoundGodot::read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    return 0;
}

void CsoundGodot::set_csound_name(const String &name) {
    csound_name = name;
    /*
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        if (csound_engine->has(csound_name) && !csound_engine->has(name)) {
            csound_engine->rename(csound_name, name);
        }
    }
    */
}

const String &CsoundGodot::get_csound_name() {
    return csound_name;
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
