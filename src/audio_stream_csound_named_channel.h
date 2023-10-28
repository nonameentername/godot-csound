#ifndef AUDIOSTREAMCSOUNDNAMEDCHANNEL_H
#define AUDIOSTREAMCSOUNDNAMEDCHANNEL_H

#include "csound.hpp"

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/godot.hpp>

#include "audio_stream_csound.h"
#include "csound_godot.h"

namespace godot {

class AudioStreamCsoundNamedChannel : public AudioStreamCsound {
    GDCLASS(AudioStreamCsoundNamedChannel, AudioStreamCsound)

private:
    friend class AudioStreamPlaybackCsound;
    float pos;
    int mix_rate;
    bool stereo;
    int hz;
    String csound_name;
    String left;
    String right;

public:
    void set_left(String p_left);
    String get_left();
    void set_right(String p_right);
    String get_right();
    int gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames) override;
    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
