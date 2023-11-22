#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "audio_effect_capture_csound.h"
#include "audio_stream_csound.h"
#include "audio_stream_csound_channel.h"
#include "audio_stream_csound_named_channel.h"
#include "audio_stream_mytone.h"
#include "audio_stream_player_csound.h"
#include "audio_stream_player_csound_channel.h"
#include "audio_stream_player_csound_named_channel.h"
#include "audio_stream_player_mytone.h"
#include "csound_godot.h"
#include "csound_layout.h"
#include "csound_server.h"
#include "editor_audio_meter_notches_csound.h"
#include "godot_cpp/classes/editor_plugin.hpp"
#include "midi_file_reader.h"
#include "soundfont_file_reader.h"

using namespace godot;

static CsoundServer *csound_module;

void initialize_csoundgodot_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        ClassDB::register_class<CsoundGodot>();
        ClassDB::register_class<CsoundServer>();
        ClassDB::register_class<CsoundLayout>();
        csound_module = memnew(CsoundServer);
        Engine::get_singleton()->register_singleton("CsoundServer", CsoundServer::get_singleton());

        ClassDB::register_class<MidiFileReader>();
        ClassDB::register_class<SoundFontFileReader>();
        ClassDB::register_class<AudioStreamCsound>();
        ClassDB::register_class<AudioStreamPlaybackCsound>();
        ClassDB::register_class<AudioStreamMyTone>();
        ClassDB::register_class<AudioStreamPlaybackMyTone>();
        ClassDB::register_class<AudioStreamCsoundChannel>();
        ClassDB::register_class<AudioStreamPlaybackCsoundChannel>();
        ClassDB::register_class<AudioStreamCsoundNamedChannel>();
        ClassDB::register_class<AudioStreamPlaybackCsoundNamedChannel>();
        ClassDB::register_class<AudioEffectCaptureCsound>();
        ClassDB::register_class<AudioEffectCaptureCsoundInstance>();
        ClassDB::register_class<EditorAudioMeterNotchesCsound>();
    }
}

void uninitialize_csoundgodot_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        Engine::get_singleton()->unregister_singleton("Csound");
        memdelete(csound_module);
    }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT csoundgodot_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                    const GDExtensionClassLibraryPtr p_library,
                                                    GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_csoundgodot_module);
    init_obj.register_terminator(uninitialize_csoundgodot_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_EDITOR);

    return init_obj.init();
}
}
