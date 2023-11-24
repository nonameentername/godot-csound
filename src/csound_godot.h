#ifndef CSOUNDGODOT_H
#define CSOUNDGODOT_H

#include <csound.hpp>

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "midi_file_reader.h"
#include "soundfont_file_reader.h"
#include "sysdep.h"

static const float AUDIO_PEAK_OFFSET = 0.0000000001f;
static const float AUDIO_MIN_PEAK_DB = -200.0f;

namespace godot {

class CsoundGodot : public Node {
    GDCLASS(CsoundGodot, Node);
    friend class CsoundServer;

private:
    uint64_t last_mix_time;
    int last_mix_frames;
    bool active;

    int sfont_id;
    Ref<SoundFontFileReader> soundfont;
    Ref<MidiFileReader> midi_file;
    Csound *csound;
    bool finished;
    String csound_name;
    bool solo;
    bool mute;
    bool bypass;
    float volume_db;
    bool initialized;

    struct Channel {
        bool used = false;
        bool active = false;
        float peak_volume = AUDIO_MIN_PEAK_DB;
        Vector<MYFLT> buffer;
        Channel() {
        }
    };

    Vector<Vector<MYFLT>> input_channels;
    Vector<Channel> output_channels;
    HashMap<String, Vector<MYFLT>> named_channel_input_buffers;
    HashMap<String, Vector<MYFLT>> named_channel_output_buffers;

    void initialize_channels(int p_frames);

protected:
    static void _bind_methods();

public:
    CsoundGodot();
    ~CsoundGodot();

    virtual void _ready() override;

    void start();
    void stop();
    void reset();

    void set_soundfont(Ref<SoundFontFileReader> p_soundfont);
    Ref<SoundFontFileReader> get_soundfont();

    void set_midi_file(Ref<MidiFileReader> p_midi_file);
    Ref<MidiFileReader> get_midi_file();

    void program_select(int chan, int bank_num, int preset_num);
    void note_on(int chan, int key, int vel);
    void note_off(int chan, int key);
    void send_control_channel(String channel, float value);
    // val value (0-16383 with 8192 being center)
    void pitch_bend(int chan, int val);
    void play_midi(Ref<MidiFileReader> p_midi_file);

    int process_sample(AudioFrame *p_buffer, float p_rate, int p_frames);

    void set_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right);
    int get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right);

    void set_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right);
    int get_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right);

    void process(double delta);

    static int write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes);
    static int read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes);
    void _notification(int p_what);
    void set_csound_name(const String &name);
    const String &get_csound_name();

    int get_channel_count();

    double get_time_since_last_mix();
    double get_time_to_next_mix();

    static void csound_message_callback(CSOUND *csound, int attr, const char *format, va_list args);
};
} // namespace godot

#endif
