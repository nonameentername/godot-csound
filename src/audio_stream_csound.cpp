#include "audio_stream_csound.h"
#include "audio_stream_player_csound.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_stream.hpp"

using namespace godot;

AudioStreamCsound::AudioStreamCsound() {
    CsoundServer::get_singleton()->connect("csound_layout_changed", Callable(this, "csound_layout_changed"));
    CsoundServer::get_singleton()->connect("csound_ready", Callable(this, "csound_ready"));
    active = false;
}

AudioStreamCsound::~AudioStreamCsound() {
}

Ref<AudioStreamPlayback> AudioStreamCsound::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsound> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsound>(this);

    return talking_tree;
}

void AudioStreamCsound::set_active(bool p_active) {
    active = p_active;

    CsoundInstance *csound_instance = get_csound_instance();
    if (csound_instance != NULL) {
        csound_instance->set_active(active);
    }
}

bool AudioStreamCsound::is_active() {
    CsoundInstance *csound_instance = get_csound_instance();
    if (csound_instance != NULL) {
        return csound_instance->is_active();
    }
    return false;
}

void AudioStreamCsound::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsound::get_csound_name() const {
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

String AudioStreamCsound::get_stream_name() const {
    return "Csound";
}

float AudioStreamCsound::get_length() const {
    return 0;
}

bool AudioStreamCsound::_set(const StringName &p_name, const Variant &p_value) {
    if ((String)p_name == "csound_name") {
        set_csound_name(p_value);
        return true;
    }
    return false;
}

bool AudioStreamCsound::_get(const StringName &p_name, Variant &r_ret) const {
    if ((String)p_name == "csound_name") {
        r_ret = get_csound_name();
        return true;
    }
    return false;
}

void AudioStreamCsound::csound_layout_changed() {
    notify_property_list_changed();
}

void AudioStreamCsound::csound_ready(String p_csound_name) {
    if (get_csound_name() == p_csound_name) {
        CsoundInstance *csound_instance = get_csound_instance();
        if (csound_instance != NULL) {
            csound_instance->set_active(active);
        }
    }
}

CsoundInstance *AudioStreamCsound::get_csound_instance() {
    CsoundServer *csound_server = (CsoundServer *)Engine::get_singleton()->get_singleton("CsoundServer");
    if (csound_server != NULL) {
        CsoundInstance *csound_instance = csound_server->get_csound(get_csound_name());
        return csound_instance;
    }
    return NULL;
}

int AudioStreamCsound::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundInstance *csound_instance = get_csound_instance();
    if (csound_instance != NULL) {
        return csound_instance->process_sample(p_buffer, p_rate, p_frames);
    }

    for (int frame = 0; frame < p_frames; frame += 1) {
        p_buffer[frame].left = 0;
        p_buffer[frame].right = 0;
    }

    return p_frames;
}

void AudioStreamCsound::_get_property_list(List<PropertyInfo> *p_list) const {
    String options = CsoundServer::get_singleton()->get_csound_name_options();
    p_list->push_back(PropertyInfo(Variant::STRING_NAME, "csound_name", PROPERTY_HINT_ENUM, options));
}

void AudioStreamCsound::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsound::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsound::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsound::get_csound_name);
    ClassDB::bind_method(D_METHOD("csound_layout_changed"), &AudioStreamCsound::csound_layout_changed);
    ClassDB::bind_method(D_METHOD("csound_ready", "csound_name"), &AudioStreamCsound::csound_ready);
}
