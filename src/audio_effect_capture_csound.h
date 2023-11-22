#ifndef AUDIO_EFFECT_CAPTURE_CSOUND_H
#define AUDIO_EFFECT_CAPTURE_CSOUND_H

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_server.hpp>

namespace godot {

class AudioEffectCaptureCsound;

class AudioEffectCaptureCsoundInstance : public AudioEffectInstance {
    GDCLASS(AudioEffectCaptureCsoundInstance, AudioEffectInstance);
    friend class AudioEffectCaptureCsound;
    Ref<AudioEffectCaptureCsound> base;

private:
    bool has_data = false;

public:
    virtual void _process(const void *src_buffer, AudioFrame *dst_buffer, int32_t frame_count) override;
    virtual bool _process_silence() const override;

protected:
    static void _bind_methods();
};

class AudioEffectCaptureCsound : public AudioEffect {
    GDCLASS(AudioEffectCaptureCsound, AudioEffect)
    friend class AudioEffectCaptureCsoundInstance;

    String csound_name;
    String channel_left;
    String channel_right;

protected:
    static void _bind_methods();

public:
    virtual Ref<AudioEffectInstance> _instantiate() override;

    void set_csound_name(const String &name);
    const String &get_csound_name();
    void set_channel_left(String p_channel_left);
    String get_channel_left();
    void set_channel_right(String p_channel_right);
    String get_channel_right();
};
} // namespace godot

#endif
