#ifndef AUDIOSTREAMCSOUNDCHANNEL_H
#define AUDIOSTREAMCSOUNDCHANNEL_H

#include "csound.hpp"

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/godot.hpp>

#include "csound_godot.h"

namespace godot {

class AudioStreamCsoundChannel : public AudioStream {
    GDCLASS(AudioStreamCsoundChannel, AudioStream)

private:
    friend class AudioStreamPlaybackCsoundChannel;
    float pos;
    int mix_rate;
    bool stereo;
    int hz;
    String csound_name;

public:
    void reset();
    void set_position(uint64_t pos);
    virtual String get_stream_name() const;
    int gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames);
    virtual float get_length() const {
        return 0;
    } // if supported, otherwise return 0
    AudioStreamCsoundChannel();
    ~AudioStreamCsoundChannel();
    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;
    void set_csound_name(const String &name);
    const String &get_csound_name();

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
