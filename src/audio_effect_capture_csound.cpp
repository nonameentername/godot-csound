#include "audio_effect_capture_csound.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_server.hpp"

using namespace godot;

void AudioEffectCaptureCsound::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioEffectCaptureCsound::get_csound_name() {
    return csound_name;
}

void AudioEffectCaptureCsound::set_channel_left(String p_channel_left) {
    channel_left = p_channel_left;
}

String AudioEffectCaptureCsound::get_channel_left() {
    return channel_left;
}

void AudioEffectCaptureCsound::set_channel_right(String p_channel_right) {
    channel_right = p_channel_right;
}

String AudioEffectCaptureCsound::get_channel_right() {
    return channel_right;
}

void AudioEffectCaptureCsound::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioEffectCaptureCsound::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioEffectCaptureCsound::get_csound_name);
    ClassDB::add_property("AudioEffectCaptureCsound", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");
    ClassDB::bind_method(D_METHOD("set_channel_left", "channel"), &AudioEffectCaptureCsound::set_channel_left);
    ClassDB::bind_method(D_METHOD("get_channel_left"), &AudioEffectCaptureCsound::get_channel_left);
    ClassDB::add_property("AudioEffectCaptureCsound", PropertyInfo(Variant::STRING, "channel_left"), "set_channel_left",
                          "get_channel_left");
    ClassDB::bind_method(D_METHOD("set_channel_right", "channel"), &AudioEffectCaptureCsound::set_channel_right);
    ClassDB::bind_method(D_METHOD("get_channel_right"), &AudioEffectCaptureCsound::get_channel_right);
    ClassDB::add_property("AudioEffectCaptureCsound", PropertyInfo(Variant::STRING, "channel_right"),
                          "set_channel_right", "get_channel_right");
}

Ref<AudioEffectInstance> AudioEffectCaptureCsound::_instantiate() {
    Ref<AudioEffectCaptureCsoundInstance> ins;
    ins.instantiate();
    ins->base = Ref<AudioEffectCaptureCsound>(this);

    return ins;
}

void AudioEffectCaptureCsoundInstance::_process(const void *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) {
    AudioFrame *src_frames = (AudioFrame *)p_src_frames;

    for (int i = 0; i < p_frame_count; i++) {
        p_dst_frames[i] = src_frames[i];
        if (src_frames[i].left > 0 || src_frames[i].right > 0) {
            has_data = true;
        }
    }

    if (!has_data) {
        return;
    }

    CsoundServer *csound_server = (CsoundServer *)Engine::get_singleton()->get_singleton("CsoundServer");
    if (csound_server != NULL) {
        CsoundGodot *csound_godot = csound_server->get_csound(base->csound_name);
        if (csound_godot != NULL) {
            int p_rate = 1;
            csound_godot->set_named_channel_sample(src_frames, p_rate, p_frame_count, base->channel_left,
                                                   base->channel_right);
        }
    }
}

bool AudioEffectCaptureCsoundInstance::_process_silence() const {
    return true;
}

void AudioEffectCaptureCsoundInstance::_bind_methods() {
}
