#include "audio_stream_player_csound.h"
#include "audio_stream_csound.h"

using namespace godot;

AudioStreamPlaybackCsound::AudioStreamPlaybackCsound() : active(false) {
}

AudioStreamPlaybackCsound::~AudioStreamPlaybackCsound() {
    _stop();
}

void AudioStreamPlaybackCsound::_stop() {
    active = false;
    base->set_active(active);
}

void AudioStreamPlaybackCsound::_start(double p_from_pos) {
    active = true;
    base->set_active(active);
}

void AudioStreamPlaybackCsound::_seek(double p_time) {
    if (p_time < 0) {
        p_time = 0;
    }
}

int AudioStreamPlaybackCsound::_mix(AudioFrame *p_buffer, float p_rate, int p_frames) {
    ERR_FAIL_COND_V(!active, 0);
    if (!active) {
        return 0;
    }

    return base->process_sample(p_buffer, p_rate, p_frames);
}

void AudioStreamPlaybackCsound::_tag_used_streams() {
}

int AudioStreamPlaybackCsound::_get_loop_count() const {
    return 10;
}

double AudioStreamPlaybackCsound::_get_playback_position() const {
    return 0;
}

float AudioStreamPlaybackCsound::_get_length() const {
    return 0.0;
}

bool AudioStreamPlaybackCsound::_is_playing() const {
    return active;
}

void AudioStreamPlaybackCsound::_bind_methods() {
}
