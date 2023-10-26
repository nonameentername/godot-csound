#include "audio_stream_csound_channel.h"
#include "audio_stream_player_csound_channel.h"
#include "csound_engine.h"
#include "godot_cpp/classes/resource.hpp"

using namespace godot;

AudioStreamCsoundChannel::AudioStreamCsoundChannel() : mix_rate(44100), stereo(false), hz(639) {
}

AudioStreamCsoundChannel::~AudioStreamCsoundChannel() {
}

Ref<AudioStreamPlayback> AudioStreamCsoundChannel::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsoundChannel> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsoundChannel>(this);
    return talking_tree;
}

void AudioStreamCsoundChannel::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsoundChannel::get_csound_name() {
    return csound_name;
}

String AudioStreamCsoundChannel::get_stream_name() const {
    return "Csound";
}

void AudioStreamCsoundChannel::reset() {
    set_position(0);
}

void AudioStreamCsoundChannel::set_position(uint64_t p) {
    pos = p;
}

int AudioStreamCsoundChannel::gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        CsoundGodot *csound_godot = csound_engine->get(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->get_sample(p_buffer, p_rate, p_frames);
        }
    }
    return p_frames;
}

void AudioStreamCsoundChannel::_bind_methods() {
    ClassDB::bind_method(D_METHOD("reset"), &AudioStreamCsoundChannel::reset);
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsoundChannel::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsoundChannel::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsoundChannel::get_csound_name);
    ClassDB::add_property("AudioStreamCsoundChannel", PropertyInfo(Variant::STRING, "csound name"), "set_csound_name",
                          "get_csound_name");
}
