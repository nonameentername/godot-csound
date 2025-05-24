#ifndef AUDIOSTREAMCSOUND_H
#define AUDIOSTREAMCSOUND_H

#include "csound_instance.h"

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
    bool active;

    CsoundInstance *get_csound_instance();
    void csound_layout_changed();
    void csound_ready(String csound_name);

public:
    virtual String get_stream_name() const;
    virtual float get_length() const;

    int process_sample(AudioFrame *p_buffer, float p_rate, int p_frames);

    AudioStreamCsound();
    ~AudioStreamCsound();

    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;
    void set_csound_name(const String &name);
    const String &get_csound_name() const;

    void set_active(bool active);
    bool is_active();

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
