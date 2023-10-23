#ifndef AUDIOSTREAMCSOUND_H
#define AUDIOSTREAMCSOUND_H

#include "csound.hpp"

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/godot.hpp>

#include "csound_godot.h"
#include "midi_file_reader.h"
#include "soundfont_file_reader.h"

namespace godot {

class AudioStreamCsound : public AudioStream {
    GDCLASS(AudioStreamCsound, AudioStream)

private:
    friend class AudioStreamPlaybackCsound;
    float pos;
    int mix_rate;
    bool stereo;
    int hz;
    Ref<SoundFontFileReader> soundfont;
    Ref<MidiFileReader> midi_file;
    String csound_name;

public:
    void reset();
    void set_position(uint64_t pos);
    virtual String get_stream_name() const;
    int gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames);
    virtual float get_length() const {
        return 0;
    } // if supported, otherwise return 0
    AudioStreamCsound();
    ~AudioStreamCsound();
    virtual Ref<AudioStreamPlayback> _instantiate_playback() const override;
    void set_soundfont(Ref<SoundFontFileReader> p_soundfont);
    Ref<SoundFontFileReader> get_soundfont();
    void set_midi_file(Ref<MidiFileReader> p_midi_file);
    Ref<MidiFileReader> get_midi_file();
    void set_csound_name(const String &name);
    String get_csound_name();

protected:
    static void _bind_methods();
};
} // namespace godot

#endif
