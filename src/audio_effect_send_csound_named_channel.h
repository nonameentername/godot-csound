#ifndef AUDIO_EFFECT_SEND_CSOUND_NAMED_CHANNEL_H
#define AUDIO_EFFECT_SEND_CSOUND_NAMED_CHANNEL_H

#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_server.hpp>

namespace godot {

class AudioEffectSetCsoundNamedChannel;

class AudioEffectSetCsoundNamedChannelInstance : public AudioEffectInstance {
    GDCLASS(AudioEffectSetCsoundNamedChannelInstance, AudioEffectInstance);
    friend class AudioEffectCaptureCsound;
    Ref<AudioEffectSetCsoundNamedChannel> base;

private:
    bool has_data = false;

public:
    virtual void _process(const void *src_buffer, AudioFrame *dst_buffer, int32_t frame_count) override;
    virtual bool _process_silence() const override;

protected:
    static void _bind_methods();
};

class AudioEffectSetCsoundNamedChannel : public AudioEffect {
    GDCLASS(AudioEffectSetCsoundNamedChannel, AudioEffect)
    friend class AudioEffectCaptureCsoundInstance;

    String csound_name;
    String channel_left;
    String channel_right;
    bool forward_audio = true;

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

    void set_forward_audio(bool p_forward_audio);
    bool get_forward_audio();
};
} // namespace godot

#endif