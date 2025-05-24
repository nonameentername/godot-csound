#include "audio_stream_csound_channel.h"
#include "audio_stream_player_csound_channel.h"
#include "csound_instance.h"
#include "csound_server.h"
#include "godot_cpp/core/property_info.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

using namespace godot;

AudioStreamCsoundChannel::AudioStreamCsoundChannel() {
    CsoundServer::get_singleton()->connect("csound_layout_changed", Callable(this, "csound_layout_changed"));
    channel_left = 0;
    channel_right = 1;
}

AudioStreamCsoundChannel::~AudioStreamCsoundChannel() {
}

void AudioStreamCsoundChannel::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsoundChannel::get_csound_name() const {
    static const String default_name = "Main";
    if (csound_name.length() == 0) {
        return default_name;
    }

    for (int i = 0; i < CsoundServer::get_singleton()->get_csound_count(); i++) {
        if (CsoundServer::get_singleton()->get_csound_name(i) == csound_name) {
            return csound_name;
        }
    }

    return default_name;
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
    CsoundInstance *csound_instance = CsoundServer::get_singleton()->get_csound(get_csound_name());
    if (csound_instance != NULL && csound_instance->is_active()) {
        return csound_instance->get_channel_sample(p_buffer, p_rate, p_frames, channel_left, channel_right);
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        p_buffer[frame].left = 0;
        p_buffer[frame].right = 0;
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

bool AudioStreamCsoundChannel::_set(const StringName &p_name, const Variant &p_value) {
    if ((String)p_name == "csound_name") {
        set_csound_name(p_value);
        return true;
    }
    return false;
}

bool AudioStreamCsoundChannel::_get(const StringName &p_name, Variant &r_ret) const {
    if ((String)p_name == "csound_name") {
        r_ret = get_csound_name();
        return true;
    }
    return false;
}

void AudioStreamCsoundChannel::csound_layout_changed() {
    notify_property_list_changed();
}

void AudioStreamCsoundChannel::_get_property_list(List<PropertyInfo> *p_list) const {
    String options = CsoundServer::get_singleton()->get_csound_name_options();
    p_list->push_back(PropertyInfo(Variant::STRING_NAME, "csound_name", PROPERTY_HINT_ENUM, options));
}

void AudioStreamCsoundChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsoundChannel::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsoundChannel::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsoundChannel::get_csound_name);

    ClassDB::bind_method(D_METHOD("set_channel_left", "channel"), &AudioStreamCsoundChannel::set_channel_left);
    ClassDB::bind_method(D_METHOD("get_channel_left"), &AudioStreamCsoundChannel::get_channel_left);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::INT, "channel_left"), "set_channel_left",
                          "get_channel_left");

    ClassDB::bind_method(D_METHOD("set_channel_right", "channel"), &AudioStreamCsoundChannel::set_channel_right);
    ClassDB::bind_method(D_METHOD("get_channel_right"), &AudioStreamCsoundChannel::get_channel_right);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::INT, "channel_right"), "set_channel_right",
                          "get_channel_right");

    ClassDB::bind_method(D_METHOD("csound_layout_changed"), &AudioStreamCsoundChannel::csound_layout_changed);
}
