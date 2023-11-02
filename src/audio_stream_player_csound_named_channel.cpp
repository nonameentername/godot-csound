#include "audio_stream_player_csound_named_channel.h"
#include "audio_stream_csound_named_channel.h"

using namespace godot;

AudioStreamPlaybackCsoundNamedChannel::AudioStreamPlaybackCsoundNamedChannel() : active(false) {
}

AudioStreamPlaybackCsoundNamedChannel::~AudioStreamPlaybackCsoundNamedChannel() {
}

void AudioStreamPlaybackCsoundNamedChannel::_stop() {
    active = false;
}

void AudioStreamPlaybackCsoundNamedChannel::_start(double p_from_pos) {
    active = true;
}

void AudioStreamPlaybackCsoundNamedChannel::_seek(double p_time) {
    if (p_time < 0) {
        p_time = 0;
    }
}

int AudioStreamPlaybackCsoundNamedChannel::_mix(AudioFrame *p_buffer, double p_rate, int p_frames) {
    ERR_FAIL_COND_V(!active, 0);
    if (!active) {
        return 0;
    }

    return base->process_sample(p_buffer, p_rate, p_frames);
}

int AudioStreamPlaybackCsoundNamedChannel::_get_loop_count() const {
    return 10;
}

double AudioStreamPlaybackCsoundNamedChannel::_get_playback_position() const {
    return 0;
}

float AudioStreamPlaybackCsoundNamedChannel::_get_length() const {
    return 0.0;
}

bool AudioStreamPlaybackCsoundNamedChannel::_is_playing() const {
    return active;
}

void AudioStreamPlaybackCsoundNamedChannel::_bind_methods() {
}
