#include "csound_gdextension.h"

using namespace godot;

CsoundEngine *CsoundEngine::singleton = NULL;

CsoundEngine::CsoundEngine() {
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

    singleton = this;

    call_deferred("initialize");
}

CsoundEngine::~CsoundEngine() {
    delete csound;
    singleton = NULL;
}

CsoundEngine *CsoundEngine::get_singleton() {
    return singleton;
}

void CsoundEngine::set_soundfont(String node_path) {
    if (ResourceLoader::get_singleton()->exists(node_path)) {
        Variant resource = ResourceLoader::get_singleton()->load(node_path);
        Ref<SoundFontFileReader> soundfont = resource;
        if (soundfont != NULL) {
        }
    }
}

Ref<SoundFontFileReader> CsoundEngine::get_soundfont() {
    return soundfont;
}

void CsoundEngine::set_midi_file(Ref<MidiFileReader> p_midi_file) {
    midi_file = p_midi_file;
}

Ref<MidiFileReader> CsoundEngine::get_midi_file() {
    return midi_file;
}

int CsoundEngine::gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) {
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

void CsoundEngine::program_select(int chan, int bank_num, int preset_num) {
}

void CsoundEngine::note_on(int chan, int key, int vel) {
    char buffer[100];
    sprintf(buffer, "i1 0 -1 %d %d", key, vel);
    csound->InputMessage(buffer);
}

void CsoundEngine::note_off(int chan, int key) {
    char buffer[100];
    sprintf(buffer, "i-1 0 0 %d", key);
    csound->InputMessage(buffer);
}

void CsoundEngine::send_control_channel(String channel, float value) {
    csound->SetControlChannel(channel.ascii().get_data(), value);
}

void CsoundEngine::pitch_bend(int chan, int val) {
}

void CsoundEngine::play_midi(Ref<MidiFileReader> p_midi_file) {
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

void CsoundEngine::initialize() {
    Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop())->get_root()->add_child(this);
}

void CsoundEngine::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_INTERNAL_PROCESS: {
        process(get_process_delta_time());
    }
    }
}

void CsoundEngine::process(double delta) {
}

int CsoundEngine::write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes) {
    return 0;
}

int CsoundEngine::read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes) {
    return 0;
}

void CsoundEngine::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &CsoundEngine::initialize);
    ClassDB::bind_method(D_METHOD("process", "delta"), &CsoundEngine::process);
    ClassDB::bind_method(D_METHOD("program_select"), &CsoundEngine::program_select);
    ClassDB::bind_method(D_METHOD("note_on"), &CsoundEngine::note_on);
    ClassDB::bind_method(D_METHOD("note_off"), &CsoundEngine::note_off);
    ClassDB::bind_method(D_METHOD("send_control_channel"), &CsoundEngine::send_control_channel);
    ClassDB::bind_method(D_METHOD("pitch_bend"), &CsoundEngine::pitch_bend);
    ClassDB::bind_method(D_METHOD("play_midi"), &CsoundEngine::play_midi, DEFVAL(NULL));
    ClassDB::bind_method(D_METHOD("set_soundfont", "soundfont"), &CsoundEngine::set_soundfont);
    ClassDB::bind_method(D_METHOD("get_soundfont"), &CsoundEngine::get_soundfont);
    ClassDB::add_property(
        "CsoundEngine", PropertyInfo(Variant::OBJECT, "soundfont", PROPERTY_HINT_RESOURCE_TYPE, "SoundFontFileReader"),
        "set_soundfont", "get_soundfont");
    ClassDB::bind_method(D_METHOD("set_midi_file", "midi_file"), &CsoundEngine::set_midi_file);
    ClassDB::bind_method(D_METHOD("get_midi_file"), &CsoundEngine::get_midi_file);
    ClassDB::add_property("CsoundEngine",
                          PropertyInfo(Variant::OBJECT, "midi_file", PROPERTY_HINT_RESOURCE_TYPE, "MidiFileReader"),
                          "set_midi_file", "get_midi_file");
}
