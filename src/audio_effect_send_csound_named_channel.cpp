#include "audio_effect_send_csound_named_channel.h"
#include "csound_server.h"
#include "godot_cpp/classes/audio_server.hpp"

using namespace godot;

void AudioEffectSetCsoundNamedChannel::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioEffectSetCsoundNamedChannel::get_csound_name() {
    return csound_name;
}

void AudioEffectSetCsoundNamedChannel::set_channel_left(String p_channel_left) {
    channel_left = p_channel_left;
}

String AudioEffectSetCsoundNamedChannel::get_channel_left() {
    return channel_left;
}

void AudioEffectSetCsoundNamedChannel::set_channel_right(String p_channel_right) {
    channel_right = p_channel_right;
}

String AudioEffectSetCsoundNamedChannel::get_channel_right() {
    return channel_right;
}

void AudioEffectSetCsoundNamedChannel::set_forward_audio(bool p_forward_audio) {
    forward_audio = p_forward_audio;
}

bool AudioEffectSetCsoundNamedChannel::get_forward_audio() {
    return forward_audio;
}

void AudioEffectSetCsoundNamedChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioEffectSetCsoundNamedChannel::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioEffectSetCsoundNamedChannel::get_csound_name);
    ClassDB::add_property("AudioEffectSetCsoundNamedChannel", PropertyInfo(Variant::STRING, "csound_name"),
                          "set_csound_name", "get_csound_name");
    ClassDB::bind_method(D_METHOD("set_channel_left", "channel"), &AudioEffectSetCsoundNamedChannel::set_channel_left);
    ClassDB::bind_method(D_METHOD("get_channel_left"), &AudioEffectSetCsoundNamedChannel::get_channel_left);
    ClassDB::add_property("AudioEffectSetCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_left"),
                          "set_channel_left", "get_channel_left");
    ClassDB::bind_method(D_METHOD("set_channel_right", "channel"),
                         &AudioEffectSetCsoundNamedChannel::set_channel_right);
    ClassDB::bind_method(D_METHOD("get_channel_right"), &AudioEffectSetCsoundNamedChannel::get_channel_right);
    ClassDB::add_property("AudioEffectSetCsoundNamedChannel", PropertyInfo(Variant::STRING, "channel_right"),
                          "set_channel_right", "get_channel_right");
    ClassDB::bind_method(D_METHOD("set_forward_audio", "forward_audio"),
                         &AudioEffectSetCsoundNamedChannel::set_forward_audio);
    ClassDB::bind_method(D_METHOD("get_forward_audio"), &AudioEffectSetCsoundNamedChannel::get_forward_audio);
    ClassDB::add_property("AudioEffectSetCsoundNamedChannel", PropertyInfo(Variant::BOOL, "forward_audio"),
                          "set_forward_audio", "get_forward_audio");
}

Ref<AudioEffectInstance> AudioEffectSetCsoundNamedChannel::_instantiate() {
    Ref<AudioEffectSetCsoundNamedChannelInstance> ins;
    ins.instantiate();
    ins->base = Ref<AudioEffectSetCsoundNamedChannel>(this);

    return ins;
}

void AudioEffectSetCsoundNamedChannelInstance::_process(const void *p_src_frames, AudioFrame *p_dst_frames,
                                                        int p_frame_count) {
    AudioFrame *src_frames = (AudioFrame *)p_src_frames;

    for (int i = 0; i < p_frame_count; i++) {
        if (base->forward_audio) {
            p_dst_frames[i] = src_frames[i];
        } else {
            p_dst_frames[i].left = 0;
            p_dst_frames[i].right = 0;
        }
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

bool AudioEffectSetCsoundNamedChannelInstance::_process_silence() const {
    return true;
}

void AudioEffectSetCsoundNamedChannelInstance::_bind_methods() {
}
