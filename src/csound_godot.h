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

namespace godot {

class CsoundGodot : public Node {
    GDCLASS(CsoundGodot, Node);

private:
    int sfont_id;
    Ref<SoundFontFileReader> soundfont;
    Ref<MidiFileReader> midi_file;
    Csound *csound;
    MYFLT *spin;
    MYFLT *spout;
    MYFLT scale;
    int ksmps_index;
    bool finished;
    String csound_name;
    Vector<Vector<MYFLT>> channel_buffers;
    Vector<int> write_buffer_index;
    Vector<int> read_buffer_index;
    HashMap<String, Vector<MYFLT>> named_channel_buffers;
    HashMap<String, int> write_named_buffer_index;
    HashMap<String, int> read_named_buffer_index;

    void next_frame(Vector<int> &buffer_index, int channel, int p_frames);
    void next_frame(HashMap<String, int> &buffer_index, String channel, int p_frames);

protected:
    static void _bind_methods();

public:
    CsoundGodot();
    ~CsoundGodot();
    virtual void _ready() override;
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
    int gen_tone(AudioFrame *p_buffer, float p_rate, int p_frames);
    int get_sample(AudioFrame *p_buffer, float p_rate, int p_frames);
    void process(double delta);
    static int write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes);
    static int read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes);
    void _notification(int p_what);
    void set_csound_name(const String &name);
    const String &get_csound_name();
};
} // namespace godot

#endif
