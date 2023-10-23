#ifndef CSOUNDENGINE_H
#define CSOUNDENGINE_H

#include <csound.hpp>

#include <godot_cpp/classes/audio_frame.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "csound_godot.h"

namespace godot {

class CsoundEngine : public Node {
    GDCLASS(CsoundEngine, Node);

private:
    int sfont_id;
    Ref<SoundFontFileReader> soundfont;
    Ref<MidiFileReader> midi_file;
    HashMap<String, CsoundGodot *> csound_instances;

protected:
    static void _bind_methods();
    static CsoundEngine *singleton;

public:
    CsoundEngine();
    ~CsoundEngine();
    static CsoundEngine *get_singleton();
    void initialize();
    void process(double delta);
    void _notification(int p_what);
    CsoundGodot *create(String name);
    CsoundGodot *get(String name);
    void erase(String name);
    bool has(String name);
    void rename(String prev_name, String name);
};
} // namespace godot

#endif
