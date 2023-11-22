#include "audio_stream_csound.h"
#include "audio_stream_player_csound.h"
#include "csound_server.h"

using namespace godot;

AudioStreamCsound::AudioStreamCsound() {
}

AudioStreamCsound::~AudioStreamCsound() {
}

Ref<AudioStreamPlayback> AudioStreamCsound::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsound> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsound>(this);

    return talking_tree;
}

void AudioStreamCsound::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsound::get_csound_name() {
    return csound_name;
}

String AudioStreamCsound::get_stream_name() const {
    return "Csound";
}

float AudioStreamCsound::get_length() const {
    return 0;
}

int AudioStreamCsound::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundServer *csound_server = (CsoundServer *)Engine::get_singleton()->get_singleton("CsoundServer");
    if (csound_server != NULL) {
        CsoundGodot *csound_godot = csound_server->get_csound(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->process_sample(p_buffer, p_rate, p_frames);
        }
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        p_buffer[frame].left = 0;
        p_buffer[frame].right = 0;
    }

    return p_frames;
}

void AudioStreamCsound::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsound::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsound::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsound::get_csound_name);
    ClassDB::add_property("AudioStreamCsound", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");
}
