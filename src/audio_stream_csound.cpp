#include "audio_stream_csound.h"
#include "audio_stream_player_csound.h"
#include "csound_engine.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/resource.hpp"

using namespace godot;

AudioStreamCsound::AudioStreamCsound() : mix_rate(44100), stereo(false), hz(639) {
}

AudioStreamCsound::~AudioStreamCsound() {
}

Ref<AudioStreamPlayback> AudioStreamCsound::_instantiate_playback() const {
    Ref<AudioStreamPlaybackCsound> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamCsound>(this);
    return talking_tree;
}

void AudioStreamCsound::set_csound_name(const String &name) {
    csound_name = name;
}

const String &AudioStreamCsound::get_csound_name() {
    return csound_name;
}

String AudioStreamCsound::get_stream_name() const {
    return "Csound";
}

void AudioStreamCsound::reset() {
    set_position(0);
}

void AudioStreamCsound::set_position(uint64_t p) {
    pos = p;
}

int AudioStreamCsound::gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) {
    CsoundEngine *csound_engine = (CsoundEngine *)Engine::get_singleton()->get_singleton("Csound");
    if (csound_engine != NULL) {
        CsoundGodot *csound_godot = csound_engine->get(csound_name);
        if (csound_godot != NULL) {
            return csound_godot->gen_tone(p_buffer, p_rate, p_frames);
        }
    }
    return p_frames;
}

void AudioStreamCsound::_bind_methods() {
    ClassDB::bind_method(D_METHOD("reset"), &AudioStreamCsound::reset);
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamCsound::get_stream_name);
    ClassDB::bind_method(D_METHOD("set_csound_name", "name"), &AudioStreamCsound::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name"), &AudioStreamCsound::get_csound_name);
    ClassDB::add_property("AudioStreamCsound", PropertyInfo(Variant::STRING, "csound_name"), "set_csound_name",
                          "get_csound_name");
}
