#include "audio_stream_mytone.h"
#include "audio_stream_player_mytone.h"
#include "godot_cpp/classes/audio_server.hpp"

using namespace godot;

AudioStreamMyTone::AudioStreamMyTone() : hz(639) {
    pos = 0;
}

Ref<AudioStreamPlayback> AudioStreamMyTone::_instantiate_playback() const {
    Ref<AudioStreamPlaybackMyTone> talking_tree;
    talking_tree.instantiate();
    talking_tree->base = Ref<AudioStreamMyTone>(this);
    return talking_tree;
}

String AudioStreamMyTone::get_stream_name() const {
    return "MyTone";
}

void AudioStreamMyTone::reset() {
    set_position(0);
}

void AudioStreamMyTone::set_position(uint64_t p) {
    pos = p;
}

void AudioStreamMyTone::gen_tone(AudioFrame *p_buffer, double p_rate, int p_frames) {
    AudioServer *audio_server = AudioServer::get_singleton();
    if (audio_server != NULL) {
        mix_rate = AudioServer::get_singleton()->get_mix_rate();
    } else {
        mix_rate = 44100;
    }

    for (int i = 0; i < p_frames; i++) {
        float inc = 1.0 / (float(mix_rate) / hz);
        pos += inc;
        if (pos > 1.0) {
            pos -= 1.0;
        }
        p_buffer[i].left = sin(2.0 * Math_PI * pos);
        p_buffer[i].right = sin(2.0 * Math_PI * pos);
    }
}

void AudioStreamMyTone::set_hz(float p_hz) {
    hz = p_hz;
}

float AudioStreamMyTone::get_hz() {
    return hz;
}

float AudioStreamMyTone::get_length() const {
    return 0;
}

void AudioStreamMyTone::_bind_methods() {
    ClassDB::bind_method(D_METHOD("reset"), &AudioStreamMyTone::reset);
    ClassDB::bind_method(D_METHOD("get_stream_name"), &AudioStreamMyTone::get_stream_name);
    ClassDB::bind_method(D_METHOD("get_hz"), &AudioStreamMyTone::get_hz);
    ClassDB::bind_method(D_METHOD("set_hz", "hz"), &AudioStreamMyTone::set_hz);

    ClassDB::add_property("AudioStreamMyTone",
                          PropertyInfo(Variant::FLOAT, "hz", PROPERTY_HINT_RANGE, "10,20000,suffix:Hz"), "set_hz",
                          "get_hz");
}
