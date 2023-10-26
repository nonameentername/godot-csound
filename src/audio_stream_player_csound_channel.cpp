#include "audio_stream_player_csound_channel.h"
#include "audio_stream_csound_channel.h"

using namespace godot;

AudioStreamPlaybackCsoundChannel::AudioStreamPlaybackCsoundChannel() : active(false) {
}

AudioStreamPlaybackCsoundChannel::~AudioStreamPlaybackCsoundChannel() {
}

void AudioStreamPlaybackCsoundChannel::_stop() {
    active = false;
    base->reset();
}

void AudioStreamPlaybackCsoundChannel::_start(double p_from_pos) {
    active = true;
    mixed = 0.0;
}

void AudioStreamPlaybackCsoundChannel::_seek(double p_time) {
    if (p_time < 0) {
        p_time = 0;
    }
    base->set_position(uint64_t(p_time * base->mix_rate) << MIX_FRAC_BITS);
}

int AudioStreamPlaybackCsoundChannel::_mix(AudioFrame *p_buffer, double p_rate, int p_frames) {
    ERR_FAIL_COND_V(!active, 0);
    if (!active) {
        return 0;
    }

    int frames = base->gen_tone(p_buffer, p_rate, p_frames);
    float mix_rate = base->mix_rate;
    mixed += frames / mix_rate;
    return frames;
}

int AudioStreamPlaybackCsoundChannel::_get_loop_count() const {
    return 10;
}

double AudioStreamPlaybackCsoundChannel::_get_playback_position() const {
    return 0;
}

float AudioStreamPlaybackCsoundChannel::_get_length() const {
    return 0.0;
}

bool AudioStreamPlaybackCsoundChannel::_is_playing() const {
    return active;
}

void AudioStreamPlaybackCsoundChannel::_bind_methods() {
}
