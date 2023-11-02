#ifndef AUDIOSTREAMCSOUND_H
#define AUDIOSTREAMCSOUND_H

#include "csound.hpp"

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {

class AudioStreamCsound : public AudioStream {
    GDCLASS(AudioStreamCsound, AudioStream)

private:
    friend class AudioStreamPlaybackCsound;
    String csound_name;

public:
    virtual String get_stream_name() const;
    int process_sample(AudioFrame *p_buffer, float p_rate, int p_frames);
    virtual float get_length() const;
    AudioStreamCsound();
    ~AudioStreamCsound();
    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;
    void set_csound_name(const String &name);
    const String &get_csound_name();

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
