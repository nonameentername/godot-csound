#ifndef AUDIO_EFFECT_SEND_CSOUND_CHANNEL_H
#define AUDIO_EFFECT_SEND_CSOUND_CHANNEL_H

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_server.hpp>

namespace godot {

class AudioEffectSetCsoundChannel;

class AudioEffectSetCsoundChannelInstance : public AudioEffectInstance {
    GDCLASS(AudioEffectSetCsoundChannelInstance, AudioEffectInstance);
    friend class AudioEffectCaptureCsound;
    Ref<AudioEffectSetCsoundChannel> base;

private:
    bool has_data = false;

public:
    virtual void _process(const void *src_buffer, AudioFrame *dst_buffer, int32_t frame_count) override;
    virtual bool _process_silence() const override;

protected:
    static void _bind_methods();
};

class AudioEffectSetCsoundChannel : public AudioEffect {
    GDCLASS(AudioEffectSetCsoundChannel, AudioEffect)
    friend class AudioEffectCaptureCsoundInstance;

    String csound_name;
    int channel_left = 0;
    int channel_right = 1;
    bool forward_audio = true;

protected:
    static void _bind_methods();

public:
    AudioEffectSetCsoundChannel();
    ~AudioEffectSetCsoundChannel();

    virtual Ref<AudioEffectInstance> _instantiate() override;

    void set_csound_name(const String &name);
    const String &get_csound_name() const;

    void set_channel_left(int p_channel_left);
    int get_channel_left();

    void set_channel_right(int p_channel_right);
    int get_channel_right();

    void set_forward_audio(bool p_forward_audio);
    bool get_forward_audio();

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

    void csound_layout_changed();
};
} // namespace godot

#endif
