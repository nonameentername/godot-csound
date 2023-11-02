#ifndef AUDIOSTREAMMYTONE_H
#define AUDIOSTREAMMYTONE_H

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/godot.hpp>

namespace godot {

class AudioStreamMyTone : public AudioStream {
    GDCLASS(AudioStreamMyTone, AudioStream)

private:
    friend class AudioStreamPlaybackMyTone;
    float pos;
    int mix_rate;
    float hz;

public:
    void reset();
    void set_position(uint64_t pos);
    virtual String get_stream_name() const;
    void gen_tone(AudioFrame *p_buffer, double p_rate, int p_frames);
    void set_hz(float p_hz);
    float get_hz();
    virtual float get_length() const;
    AudioStreamMyTone();
    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
