#include "csound_godot.h"

using namespace godot;

CsoundGodot::CsoundGodot() {
    buffer = new float[44100 * 2];

    csound = new Csound();
    csound->Compile("test.csd");

    csound->CreateMessageBuffer(0);
    csound->SetDebug(true);

    csound->SetHostImplementedAudioIO(1, 0);
    // csound->SetHostImplementedMIDIIO(true);
    // csound->SetExternalMidiWriteCallback(write_midi_data);
    // csound->SetExternalMidiReadCallback(read_midi_data);

    csound->Start();

    spout = csound->GetSpout();
    spin = csound->GetSpin();
    scale = csound->Get0dBFS();

    set_process_internal(true);
}

CsoundGodot::~CsoundGodot() {
    delete csound;
}

void CsoundGodot::set_soundfont(String node_path) {
    if (ResourceLoader::get_singleton()->exists(node_path)) {
        Variant resource = ResourceLoader::get_singleton()->load(node_path);
        Ref<SoundFontFileReader> soundfont = resource;
        if (soundfont != NULL) {
        }
    }
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

int CsoundGodot::gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) {
    int64_t to_fill = p_frames;

    int mix_rate = 44100;

    if (to_fill > mix_rate) {
        to_fill = mix_rate;
    }

    int p_index = 0;

    while (csound->PerformKsmps() == 0 && to_fill > 0) {
        for (int i = 0; i < csound->GetKsmps() * csound->GetNchnls(); i = i + csound->GetNchnls()) {
            p_buffer[p_index].left = spout[i] / scale;
            p_buffer[p_index].right = spout[i + 1] / scale;

            p_index = p_index + 1;
            to_fill = to_fill - 1;
        }
    }

    /*
    int index = 0;
    while (to_fill > 0) {
        p_buffer[p_index].left = buffer[index];
        p_buffer[p_index].right = buffer[index + 1];

        index = index + 2;
        p_index = p_index + 1;
        to_fill = to_fill - 1;
    }
    */

    return p_frames;
}

void CsoundGodot::program_select(int chan, int bank_num, int preset_num) {
}

void CsoundGodot::note_on(int chan, int key, int vel) {
    char buffer[100];
    sprintf(buffer, "i1 0 -1 %d %d", key, vel);
    csound->InputMessage(buffer);
}

void CsoundGodot::note_off(int chan, int key) {
    char buffer[100];
    sprintf(buffer, "i-1 0 0 %d", key);
    csound->InputMessage(buffer);
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
    controlChannelInfo_t *tmp;
    int num_channels = csound->ListChannels(tmp);

    for (int i = 0; i < num_channels; i++) {
        godot::UtilityFunctions::print("name = ", tmp[i].name, " type = ", tmp[i].type);
    }
}

int CsoundGodot::write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes) {
    return 0;
}

int CsoundGodot::read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    return 0;
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
}
