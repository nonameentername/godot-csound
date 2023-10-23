#ifndef CSOUNDGODOT_H
#define CSOUNDGODOT_H

#include <csound.hpp>

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
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
    float *buffer;
    Csound *csound;
    MYFLT *spin;
    MYFLT *spout;
    MYFLT scale;

protected:
    static void _bind_methods();

public:
    CsoundGodot();
    ~CsoundGodot();
    void set_soundfont(String node_path);
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
    void process(double delta);
    static int write_midi_data(CSOUND *csound, void *userData, const unsigned char *mbuf, int nbytes);
    static int read_midi_data(CSOUND *csound, void *userData, unsigned char *mbuf, int nbytes);
    void _notification(int p_what);
};
} // namespace godot

#endif
