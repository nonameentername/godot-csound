#ifndef AUDIOSTREAMCSOUNDNAMEDCHANNEL_H
#define AUDIOSTREAMCSOUNDNAMEDCHANNEL_H

#include "csound.hpp"

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/godot.hpp>

#include "csound_godot.h"

namespace godot {

class AudioStreamCsoundNamedChannel : public AudioStream {
    GDCLASS(AudioStreamCsoundNamedChannel, AudioStream)

private:
    friend class AudioStreamPlaybackCsoundNamedChannel;
    String csound_name;
    String channel_left;
    String channel_right;

public:
    virtual String get_stream_name() const;
    virtual int process_sample(AudioFrame *p_buffer, float p_rate, int p_frames);
    virtual float get_length() const;
    AudioStreamCsoundNamedChannel();
    ~AudioStreamCsoundNamedChannel();
    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;
    void set_csound_name(const String &name);
    const String &get_csound_name();
    void set_channel_left(String p_channel_left);
    String get_channel_left();
    void set_channel_right(String p_channel_right);
    String get_channel_right();

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
