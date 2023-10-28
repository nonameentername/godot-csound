#include "audio_stream_csound_named_channel.h"
#include "audio_stream_player_csound.h"
#include "csound_engine.h"
#include "godot_cpp/classes/resource.hpp"

using namespace godot;

Ref<AudioStreamPlayback> AudioStreamCsoundNamedChannel::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsound> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsoundNamedChannel>(this);
    return talking_tree;
}

int AudioStreamCsoundNamedChannel::gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        CsoundGodot *csound_godot = csound_engine->get(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->get_named_channel_sample(p_buffer, p_rate, p_frames, left, right);
        }
    }
    return p_frames;
}

void AudioStreamCsoundNamedChannel::set_left(String p_left) {
    left = p_left;
}

String AudioStreamCsoundNamedChannel::get_left() {
    return left;
}

void AudioStreamCsoundNamedChannel::set_right(String p_right) {
    right = p_right;
}

String AudioStreamCsoundNamedChannel::get_right() {
    return right;
}

void AudioStreamCsoundNamedChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_left", "channel"), &AudioStreamCsoundNamedChannel::set_left);
    ClassDB::bind_method(D_METHOD("get_left"), &AudioStreamCsoundNamedChannel::get_left);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_left"), "set_left",
                          "get_left");
    ClassDB::bind_method(D_METHOD("set_right", "channel"), &AudioStreamCsoundNamedChannel::set_right);
    ClassDB::bind_method(D_METHOD("get_right"), &AudioStreamCsoundNamedChannel::get_right);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_right"), "set_right",
                          "get_right");
}
