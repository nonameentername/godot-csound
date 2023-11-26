#ifndef CSOUNDSERVER_H
#define CSOUNDSERVER_H

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
#include "csound_layout.h"

namespace godot {

class CsoundServer : public Node {
    GDCLASS(CsoundServer, Node);

private:
    bool solo_mode;
    bool edited;
    int sfont_id;
    Ref<SoundFontFileReader> soundfont;
    Ref<MidiFileReader> midi_file;

    Vector<CsoundGodot *> csound_instances;
    HashMap<String, CsoundGodot *> csound_map;

protected:
    static void _bind_methods();
    static CsoundServer *singleton;

public:
    CsoundServer();
    ~CsoundServer();

    bool get_solo_mode();

    void set_edited(bool p_edited);
    bool get_edited();

    static CsoundServer *get_singleton();
    void initialize();
    void process(double delta);
    void _notification(int p_what);

    void set_csound_count(int p_count);
    int get_csound_count() const;

    void remove_csound(int p_index);
    void add_csound(int p_at_pos = -1);

    void move_csound(int p_csound, int p_to_pos);

    void set_csound_name(int p_csound, const String &p_name);
    String get_csound_name(int p_csound) const;
    int get_csound_index(const StringName &p_csound_name) const;

    String get_csound_name_options() const;

    int get_csound_channel_count(int p_csound) const;
    int get_csound_named_channel_count(int p_csound) const;
    String get_csound_named_channel_name(int p_csound, int p_channel) const;

    void set_csound_volume_db(int p_csound, float p_volume_db);
    float get_csound_volume_db(int p_csound) const;

    void set_csound_tab(int p_csound, float p_tab);
    int get_csound_tab(int p_csound) const;

    void set_csound_solo(int p_csound, bool p_enable);
    bool is_csound_solo(int p_csound) const;

    void set_csound_mute(int p_csound, bool p_enable);
    bool is_csound_mute(int p_csound) const;

    void set_csound_bypass(int p_csound, bool p_enable);
    bool is_csound_bypassing(int p_csound) const;

    /*
    void add_csound_instrument(int p_csound, const Ref<AudioEffect> &p_instrument, int p_at_pos = -1);
    void remove_csound_instrument(int p_csound, int p_instrument);

    int get_csound_instrument_count(int p_csound);

    void swap_csound_instruments(int p_bus, int p_instrument, int p_by_instrument);
    */

    float get_csound_channel_peak_volume_db(int p_csound, int p_channel) const;
    float get_csound_named_channel_peak_volume_db(int p_csound, int p_channel) const;

    bool is_csound_channel_active(int p_csound, int p_channel) const;
    bool is_csound_named_channel_active(int p_csound, int p_channel) const;

    bool load_default_csound_layout();
    void set_csound_layout(const Ref<CsoundLayout> &p_csound_layout);
    Ref<CsoundLayout> generate_csound_layout() const;

    void lock();
    void unlock();

    CsoundGodot *get_csound(const String &p_name);
    CsoundGodot *get_csound_(const Variant &p_name);
};
} // namespace godot

#endif
