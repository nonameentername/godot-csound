#include "audio_stream_player_mytone.h"
#include "audio_stream_mytone.h"

using namespace godot;

AudioStreamPlaybackMyTone::AudioStreamPlaybackMyTone() : active(false) {
}

AudioStreamPlaybackMyTone::~AudioStreamPlaybackMyTone() {
}

void AudioStreamPlaybackMyTone::_stop() {
    active = false;
    base->reset();
}

void AudioStreamPlaybackMyTone::_start(double p_from_pos) {
    active = true;
    mixed = 0.0;
}

void AudioStreamPlaybackMyTone::_seek(double p_time) {
    if (p_time < 0) {
        p_time = 0;
    }
    // base->set_position(uint64_t(p_time * base->mix_rate) << MIX_FRAC_BITS);
}

int AudioStreamPlaybackMyTone::_mix(AudioFrame *p_buffer, float p_rate, int p_frames) {
    ERR_FAIL_COND_V(!active, 0);
    if (!active) {
        return 0;
    }
    base->gen_tone(p_buffer, p_rate, p_frames);
    float mix_rate = base->mix_rate;
    mixed += p_frames / mix_rate;
    return p_frames;
}

int AudioStreamPlaybackMyTone::_get_loop_count() const {
    return 10;
}

double AudioStreamPlaybackMyTone::_get_playback_position() const {
    return 0;
}

float AudioStreamPlaybackMyTone::_get_length() const {
    return 0.0;
}

bool AudioStreamPlaybackMyTone::_is_playing() const {
    return active;
}

void AudioStreamPlaybackMyTone::_bind_methods() {
}
