#include "audio_stream_csound_named_channel.h"
#include "audio_stream_player_csound_named_channel.h"
#include "csound_engine.h"
#include "godot_cpp/classes/resource.hpp"

using namespace godot;

AudioStreamCsoundNamedChannel::AudioStreamCsoundNamedChannel() {
}

AudioStreamCsoundNamedChannel::~AudioStreamCsoundNamedChannel() {
}

void AudioStreamCsoundNamedChannel::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsoundNamedChannel::get_csound_name() {
    return csound_name;
}

String AudioStreamCsoundNamedChannel::get_stream_name() const {
    return "CsoundNamedChannel";
}

float AudioStreamCsoundNamedChannel::get_length() const {
    return 0;
}

Ref<AudioStreamPlayback> AudioStreamCsoundNamedChannel::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsoundNamedChannel> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsoundNamedChannel>(this);
    return talking_tree;
}

int AudioStreamCsoundNamedChannel::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        CsoundGodot *csound_godot = csound_engine->get(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->get_named_channel_sample(p_buffer, p_rate, p_frames, channel_left, channel_right);
        }
    }
    return p_frames;
}

void AudioStreamCsoundNamedChannel::set_channel_left(String p_channel_left) {
    channel_left = p_channel_left;
}

String AudioStreamCsoundNamedChannel::get_channel_left() {
    return channel_left;
}

void AudioStreamCsoundNamedChannel::set_channel_right(String p_channel_right) {
    channel_right = p_channel_right;
}

String AudioStreamCsoundNamedChannel::get_channel_right() {
    return channel_right;
}

void AudioStreamCsoundNamedChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsoundNamedChannel::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsoundNamedChannel::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsoundNamedChannel::get_csound_name);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "csound_name"),
                          "set_csound_name", "get_csound_name");
    ClassDB::add_property_group(get_class_static(), "Channels", "channel_");
    ClassDB::bind_method(D_METHOD("set_channel_left", "channel"), &AudioStreamCsoundNamedChannel::set_channel_left);
    ClassDB::bind_method(D_METHOD("get_channel_left"), &AudioStreamCsoundNamedChannel::get_channel_left);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_left"),
                          "set_channel_left", "get_channel_left");
    ClassDB::bind_method(D_METHOD("set_channel_right", "channel"), &AudioStreamCsoundNamedChannel::set_channel_right);
    ClassDB::bind_method(D_METHOD("get_channel_right"), &AudioStreamCsoundNamedChannel::get_channel_right);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_right"),
                          "set_channel_right", "get_channel_right");
}
