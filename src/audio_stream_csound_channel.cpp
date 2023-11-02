#include "audio_stream_csound_channel.h"
#include "audio_stream_player_csound_channel.h"
#include "csound_engine.h"
#include "godot_cpp/classes/resource.hpp"

using namespace godot;

AudioStreamCsoundChannel::AudioStreamCsoundChannel() {
    channel_left = 0;
    channel_right = 1;
}

AudioStreamCsoundChannel::~AudioStreamCsoundChannel() {
}

void AudioStreamCsoundChannel::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsoundChannel::get_csound_name() {
    return csound_name;
}

String AudioStreamCsoundChannel::get_stream_name() const {
    return "CsoundChannel";
}

float AudioStreamCsoundChannel::get_length() const {
    return 0;
}

Ref<AudioStreamPlayback> AudioStreamCsoundChannel::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsoundChannel> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsoundChannel>(this);
    return talking_tree;
}

int AudioStreamCsoundChannel::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        CsoundGodot *csound_godot = csound_engine->get(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->get_channel_sample(p_buffer, p_rate, p_frames, channel_left, channel_right);
        }
    }
    return p_frames;
}

void AudioStreamCsoundChannel::set_channel_left(int p_channel_left) {
    channel_left = p_channel_left;
}

int AudioStreamCsoundChannel::get_channel_left() {
    return channel_left;
}

void AudioStreamCsoundChannel::set_channel_right(int p_channel_right) {
    channel_right = p_channel_right;
}

int AudioStreamCsoundChannel::get_channel_right() {
    return channel_right;
}

void AudioStreamCsoundChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsoundChannel::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsoundChannel::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsoundChannel::get_csound_name);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");
    ClassDB::add_property_group(get_class_static(), "Channels", "channel_");
    ClassDB::bind_method(D_METHOD("set_channel_left", "channel"), &AudioStreamCsoundChannel::set_channel_left);
    ClassDB::bind_method(D_METHOD("get_channel_left"), &AudioStreamCsoundChannel::get_channel_left);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::INT, "channel_left"), "set_channel_left",
                          "get_channel_left");
    ClassDB::bind_method(D_METHOD("set_channel_right", "channel"), &AudioStreamCsoundChannel::set_channel_right);
    ClassDB::bind_method(D_METHOD("get_channel_right"), &AudioStreamCsoundChannel::get_channel_right);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::INT, "channel_right"), "set_channel_right",
                          "get_channel_right");
}
