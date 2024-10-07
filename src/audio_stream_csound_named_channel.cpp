#include "audio_stream_csound_named_channel.h"
#include "audio_stream_player_csound_named_channel.h"
#include "csound_server.h"

using namespace godot;

AudioStreamCsoundNamedChannel::AudioStreamCsoundNamedChannel() {
    CsoundServer::get_singleton()->connect("csound_layout_changed", Callable(this, "csound_layout_changed"));
}

AudioStreamCsoundNamedChannel::~AudioStreamCsoundNamedChannel() {
}

void AudioStreamCsoundNamedChannel::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsoundNamedChannel::get_csound_name() const {
    for (int i = 0; i < CsoundServer::get_singleton()->get_csound_count(); i++) {
        if (CsoundServer::get_singleton()->get_csound_name(i) == csound_name) {
            return csound_name;
        }
    }

    static const String default_name = "Main";
    return default_name;
}

String AudioStreamCsoundNamedChannel::get_stream_name() const {
    return "CsoundNamedChannel";
}

float AudioStreamCsoundNamedChannel::get_length() const {
    return 0;
}

Ref<AudioStreamPlayback> AudioStreamCsoundNamedChannel::_instantiate_playback() const {
    CsoundGodot *csound_godot = CsoundServer::get_singleton()->get_csound(get_csound_name());
    if (csound_godot == NULL || !csound_godot->is_active()) {
        godot::UtilityFunctions::push_warning("Csound is not active. AudioStreamCsound should be started before AudioStreamCsoundNamedChannel.");
    }

    Ref<AudioStreamPlaybackCsoundNamedChannel> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsoundNamedChannel>(this);
    return talking_tree;
}

int AudioStreamCsoundNamedChannel::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundGodot *csound_godot = CsoundServer::get_singleton()->get_csound(get_csound_name());
    if (csound_godot != NULL) {
        return csound_godot->get_named_channel_sample(p_buffer, p_rate, p_frames, channel_left, channel_right);
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        p_buffer[frame].left = 0;
        p_buffer[frame].right = 0;
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

bool AudioStreamCsoundNamedChannel::_set(const StringName &p_name, const Variant &p_value) {
    if ((String)p_name == "csound_name") {
        set_csound_name(p_value);
        return true;
    }
    return false;
}

bool AudioStreamCsoundNamedChannel::_get(const StringName &p_name, Variant &r_ret) const {
    if ((String)p_name == "csound_name") {
        r_ret = get_csound_name();
        return true;
    }
    return false;
}

void AudioStreamCsoundNamedChannel::csound_layout_changed() {
    notify_property_list_changed();
}

void AudioStreamCsoundNamedChannel::_get_property_list(List<PropertyInfo> *p_list) const {
    String options = CsoundServer::get_singleton()->get_csound_name_options();
    p_list->push_back(PropertyInfo(Variant::STRING_NAME, "csound_name", PROPERTY_HINT_ENUM, options));
}

void AudioStreamCsoundNamedChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsoundNamedChannel::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsoundNamedChannel::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsoundNamedChannel::get_csound_name);

    ClassDB::add_property_group(get_class_static(), "Channels", "channel_");
    ClassDB::bind_method(D_METHOD("set_channel_left", "channel"), &AudioStreamCsoundNamedChannel::set_channel_left);
    ClassDB::bind_method(D_METHOD("get_channel_left"), &AudioStreamCsoundNamedChannel::get_channel_left);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_left"),
                          "set_channel_left", "get_channel_left");
    ClassDB::bind_method(D_METHOD("set_channel_right", "channel"), &AudioStreamCsoundNamedChannel::set_channel_right);
    ClassDB::bind_method(D_METHOD("get_channel_right"), &AudioStreamCsoundNamedChannel::get_channel_right);
    ClassDB::add_property("AudioStreamCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_right"),
                          "set_channel_right", "get_channel_right");

    ClassDB::bind_method(D_METHOD("csound_layout_changed"), &AudioStreamCsoundNamedChannel::csound_layout_changed);
}
