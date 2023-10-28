#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "audio_stream_csound.h"
#include "audio_stream_csound_channel.h"
#include "audio_stream_csound_named_channel.h"
#include "audio_stream_player_csound.h"
#include "csound_engine.h"
#include "csound_godot.h"
#include "midi_file_reader.h"
#include "soundfont_file_reader.h"

using namespace godot;

static CsoundEngine *csound_module;

void initialize_gdmidiplayer_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (CsoundEngine::get_singleton() != NULL) {
        return;
    }

    ClassDB::register_class<CsoundGodot>();
    ClassDB::register_class<CsoundEngine>();
    csound_module = memnew(CsoundEngine);
    Engine::get_singleton()->register_singleton("Csound", CsoundEngine::get_singleton());

    ClassDB::register_class<MidiFileReader>();
    ClassDB::register_class<SoundFontFileReader>();
    ClassDB::register_class<AudioStreamPlaybackCsound>();
    ClassDB::register_class<AudioStreamCsound>();
    ClassDB::register_class<AudioStreamCsoundChannel>();
    ClassDB::register_class<AudioStreamCsoundNamedChannel>();
}

void uninitialize_gdmidiplayer_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (CsoundEngine::get_singleton() == NULL) {
        return;
    }

    Engine::get_singleton()->unregister_singleton("Csound");
    memdelete(csound_module);
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT gdmidiplayer_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                     const GDExtensionClassLibraryPtr p_library,
                                                     GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_gdmidiplayer_module);
    init_obj.register_terminator(uninitialize_gdmidiplayer_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
