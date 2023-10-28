#include "audio_stream_csound_channel.h"
#include "audio_stream_player_csound.h"
#include "csound_engine.h"
#include "godot_cpp/classes/resource.hpp"

using namespace godot;

AudioStreamCsoundChannel::AudioStreamCsoundChannel() {
    left = 0;
    right = 1;
}

Ref<AudioStreamPlayback> AudioStreamCsoundChannel::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsound> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsoundChannel>(this);
    return talking_tree;
}

int AudioStreamCsoundChannel::gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        CsoundGodot *csound_godot = csound_engine->get(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->get_channel_sample(p_buffer, p_rate, p_frames, left, right);
        }
    }
    return p_frames;
}

void AudioStreamCsoundChannel::set_left(int p_left) {
    left = p_left;
}

int AudioStreamCsoundChannel::get_left() {
    return left;
}

void AudioStreamCsoundChannel::set_right(int p_right) {
    right = p_right;
}

int AudioStreamCsoundChannel::get_right() {
    return right;
}

void AudioStreamCsoundChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_left", "channel"), &AudioStreamCsoundChannel::set_left);
    ClassDB::bind_method(D_METHOD("get_left"), &AudioStreamCsoundChannel::get_left);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::INT, "channel_left"), "set_left",
                          "get_left");
    ClassDB::bind_method(D_METHOD("set_right", "channel"), &AudioStreamCsoundChannel::set_right);
    ClassDB::bind_method(D_METHOD("get_right"), &AudioStreamCsoundChannel::get_right);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::INT, "channel_right"), "set_right",
                          "get_right");
}
