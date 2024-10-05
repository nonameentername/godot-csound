#include "audio_stream_csound.h"
#include "audio_stream_player_csound.h"
#include "csound_server.h"

using namespace godot;

AudioStreamCsound::AudioStreamCsound() {
    CsoundServer::get_singleton()->connect("csound_layout_changed", Callable(this, "csound_layout_changed"));
}

AudioStreamCsound::~AudioStreamCsound() {
}

Ref<AudioStreamPlayback> AudioStreamCsound::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsound> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsound>(this);

    return talking_tree;
}

void AudioStreamCsound::set_active(bool active) {
    CsoundGodot *csound_godot = get_csound_godot();
    if (csound_godot != NULL) {
        csound_godot->set_active(active);
    }
}


bool AudioStreamCsound::is_active() {
    CsoundGodot *csound_godot = get_csound_godot();
    if (csound_godot != NULL) {
        return csound_godot->is_active();
    }
    return false;
}

void AudioStreamCsound::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsound::get_csound_name() const {
    for (int i = 0; i < CsoundServer::get_singleton()->get_csound_count(); i++) {
        if (CsoundServer::get_singleton()->get_csound_name(i) == csound_name) {
            return csound_name;
        }
    }

    static const String default_name = "Main";
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

CsoundGodot *AudioStreamCsound::get_csound_godot() {
    CsoundServer *csound_server = (CsoundServer *)Engine::get_singleton()->get_singleton("CsoundServer");
    if (csound_server != NULL) {
        CsoundGodot *csound_godot = csound_server->get_csound(get_csound_name());
        return csound_godot;
    }
    return NULL;
}

int AudioStreamCsound::process_sample(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundGodot *csound_godot = get_csound_godot();
    if (csound_godot != NULL) {
        return csound_godot->process_sample(p_buffer, p_rate, p_frames);
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
}
