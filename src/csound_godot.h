#ifndef CSOUNDGODOT_H
#define CSOUNDGODOT_H

#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/thread.hpp"
#include <queue>
#define USE_LIBSNDFILE 1
#include <csound.hpp>
#include <csPerfThread.hpp>
#include <csound_files.h>
#include <soundfile.h>

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

#include "csound_file_reader.h"
#include "csound_instrument.h"
#include "midi_file_reader.h"
#include "soundfont_file_reader.h"
#include "sysdep.h"

static const float AUDIO_PEAK_OFFSET = 0.0000000001f;
static const float AUDIO_MIN_PEAK_DB = -200.0f;

namespace godot {

class CsoundGodot : public Object {
    GDCLASS(CsoundGodot, Object);
    friend class CsoundServer;

public:
    struct MidiEvent {
        int message;
        int channel;
        int note;
        int velocity;
        MidiEvent() {
        }
    };

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
    int tab;
    bool initialized;
    Ref<CsoundFileReader> script;
    std::queue<MidiEvent> midi_queue;
    Vector<Ref<CsoundInstrument>> instruments;

    struct Channel {
        String name;
        bool used = false;
        bool active = false;
        float peak_volume = AUDIO_MIN_PEAK_DB;
        Vector<MYFLT> buffer;
        Channel() {
        }
    };

    Vector<Vector<MYFLT>> input_channels;
    Vector<Channel> output_channels;

    HashMap<String, Vector<MYFLT>> input_named_channels_buffer;
    Vector<MYFLT> temp_buffer;

    Vector<Channel> output_named_channels;
    HashMap<String, int> named_channels;

    void initialize_channels(int p_frames);

    struct MemoryFile {
        char *buffer;
        sf_count_t length;
        sf_count_t curpos;
    };

    static sf_count_t vio_get_filelen(void *user_data);
    static sf_count_t vio_seek(sf_count_t offset, int whence, void *user_data);
    static sf_count_t vio_read(void *ptr, sf_count_t count, void *user_data);
    static sf_count_t vio_write(const void *ptr, sf_count_t count, void *user_data);
    static sf_count_t vio_tell(void *user_data);

protected:
    static void _bind_methods();

public:
    CsoundGodot();
    ~CsoundGodot();

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

    void input_message(String message);
    void compile_orchestra(String orchestra);

    void send_control_channel(String channel, float value);
    float get_control_channel(String channel);

    // val value (0-16383 with 8192 being center)
    void pitch_bend(int chan, int val);
    void play_midi(Ref<MidiFileReader> p_midi_file);

    int process_sample(AudioFrame *p_buffer, float p_rate, int p_frames);

    void set_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right);
    int get_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, int left, int right);

    void set_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right);
    int get_named_channel_sample(AudioFrame *p_buffer, float p_rate, int p_frames, String left, String right);

    static int open_midi_device(CSOUND *csound, void **userData, const char *dev);
    static int write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes);
    static int read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes);
    static void set_message(CSOUND *, int attr, const char *format, va_list valist);

    static FILE *open_file(CSOUND *csound, const char *filename, const char *mode);
    static void *open_sound_file(CSOUND *csound, const char *pathname, int mode, void *userdata);

    void set_csound_name(const String &name);
    const String &get_csound_name();

    void set_csound_script(Ref<CsoundFileReader> p_script);
    Ref<CsoundFileReader> get_csound_script();

    int get_channel_count();
    int get_named_channel_count();
    String get_named_channel_name(int p_channel);

    double get_time_since_last_mix();
    double get_time_to_next_mix();

    static void csound_message_callback(CSOUND *csound, int attr, const char *format, va_list args);

    void thread_func();
    void initialize();
};
} // namespace godot

#endif
