#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "csound_layout.h"
#include "csound_server.h"
#include "godot_cpp/classes/engine.hpp"
#include "godot_cpp/classes/input_event_midi.hpp"
#include "godot_cpp/classes/os.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/callable_method_pointer.hpp"
#include "version_generated.gen.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/val.h>
#endif

using namespace godot;

#ifdef __EMSCRIPTEN__
using namespace emscripten;
#endif

CsoundServer *CsoundServer::singleton = NULL;

CsoundServer::CsoundServer() {
    initialized = false;
    layout_loaded = false;
    edited = false;
    singleton = this;
    exit_thread = false;
    call_deferred("initialize");
}

CsoundServer::~CsoundServer() {
    singleton = NULL;
}

bool CsoundServer::get_solo_mode() {
    return solo_mode;
}

void CsoundServer::set_edited(bool p_edited) {
    edited = p_edited;
}

bool CsoundServer::get_edited() {
    return edited;
}

CsoundServer *CsoundServer::get_singleton() {
    return singleton;
}

void CsoundServer::add_property(String name, String default_value, GDExtensionVariantType extension_type,
                                PropertyHint hint) {
    if (godot::Engine::get_singleton()->is_editor_hint() && !ProjectSettings::get_singleton()->has_setting(name)) {
        ProjectSettings::get_singleton()->set_setting(name, default_value);
        Dictionary property_info;
        property_info["name"] = name;
        property_info["type"] = extension_type;
        property_info["hint"] = hint;
        property_info["hint_string"] = "";
        ProjectSettings::get_singleton()->add_property_info(property_info);
        ProjectSettings::get_singleton()->set_initial_value(name, default_value);
        Error error = ProjectSettings::get_singleton()->save();
        ERR_FAIL_COND_MSG(error != OK, "Could not save project settings");
    }
}

void CsoundServer::initialize() {
    add_property("audio/csound/default_csound_layout", "res://default_csound_layout.tres",
                 GDEXTENSION_VARIANT_TYPE_STRING, PROPERTY_HINT_FILE);
    add_property("audio/csound/use_resource_files", "true", GDEXTENSION_VARIANT_TYPE_BOOL, PROPERTY_HINT_NONE);
    add_property("audio/csound/hide_csound_logs", "true", GDEXTENSION_VARIANT_TYPE_BOOL, PROPERTY_HINT_NONE);

    if (!load_default_csound_layout()) {
        set_csound_count(1);
    }

    set_edited(false);
    initialized = true;
}

void CsoundServer::thread_func() {
    int msdelay = 1000;
    while (!exit_thread) {
        if (!initialized) {
            continue;
        }

        lock();

        bool use_solo = false;
        for (int i = 0; i < csound_instances.size(); i++) {
            if (csound_instances[i]->solo == true) {
                use_solo = true;
            }
        }

        if (use_solo != solo_mode) {
            solo_mode = use_solo;
        }

        unlock();
        OS::get_singleton()->delay_usec(msdelay * 1000);
    }
}

void CsoundServer::set_csound_count(int p_count) {
    ERR_FAIL_COND(p_count < 1);
    ERR_FAIL_INDEX(p_count, 256);

    edited = true;

    int cb = csound_instances.size();

    if (p_count < csound_instances.size()) {
        for (int i = p_count; i < csound_instances.size(); i++) {
            csound_map.erase(csound_instances[i]->csound_name);
            memdelete(csound_instances[i]);
        }
    }

    csound_instances.resize(p_count);

    for (int i = cb; i < csound_instances.size(); i++) {
        String attempt = "New Csound";
        int attempts = 1;
        while (true) {
            bool name_free = true;
            for (int j = 0; j < i; j++) {
                if (csound_instances[j]->csound_name == attempt) {
                    name_free = false;
                    break;
                }
            }

            if (!name_free) {
                attempts++;
                attempt = "New Csound " + itos(attempts);
            } else {
                break;
            }
        }

        if (i == 0) {
            attempt = "Main";
        }

        csound_instances.write[i] = memnew(CsoundInstance);
        csound_instances[i]->csound_name = attempt;
        csound_instances[i]->solo = false;
        csound_instances[i]->mute = false;
        csound_instances[i]->bypass = false;
        csound_instances[i]->volume_db = 0;
        csound_instances[i]->tab = 0;
        csound_instances[i]->script = Ref<CsoundFileReader>();

        csound_map[attempt] = csound_instances[i];
    }

    emit_signal("csound_layout_changed");
}

int CsoundServer::get_csound_count() const {
    if (!initialized) {
        return 0;
    }
    return csound_instances.size();
}

void CsoundServer::remove_csound(int p_index) {
    ERR_FAIL_INDEX(p_index, csound_instances.size());
    ERR_FAIL_COND(p_index == 0);

    edited = true;

    csound_instances[p_index]->stop();
    csound_map.erase(csound_instances[p_index]->csound_name);
    memdelete(csound_instances[p_index]);
    csound_instances.remove_at(p_index);

    emit_signal("csound_layout_changed");
}

void CsoundServer::add_csound(int p_at_pos) {
    edited = true;

    if (p_at_pos >= csound_instances.size()) {
        p_at_pos = -1;
    } else if (p_at_pos == 0) {
        if (csound_instances.size() > 1) {
            p_at_pos = 1;
        } else {
            p_at_pos = -1;
        }
    }

    String attempt = "New Csound";
    int attempts = 1;
    while (true) {
        bool name_free = true;
        for (int j = 0; j < csound_instances.size(); j++) {
            if (csound_instances[j]->csound_name == attempt) {
                name_free = false;
                break;
            }
        }

        if (!name_free) {
            attempts++;
            attempt = "New Csound " + itos(attempts);
        } else {
            break;
        }
    }

    CsoundInstance *csound_instance = memnew(CsoundInstance);
    csound_instance->csound_name = attempt;
    csound_instance->solo = false;
    csound_instance->mute = false;
    csound_instance->bypass = false;
    csound_instance->volume_db = 0;
    csound_instance->tab = 0;
    csound_instance->script = Ref<CsoundFileReader>();

    csound_map[attempt] = csound_instance;

    if (p_at_pos == -1) {
        csound_instances.push_back(csound_instance);
    } else {
        csound_instances.insert(p_at_pos, csound_instance);
    }

    emit_signal("csound_layout_changed");
}

void CsoundServer::move_csound(int p_csound, int p_to_pos) {
    ERR_FAIL_COND(p_csound < 1 || p_csound >= csound_instances.size());
    ERR_FAIL_COND(p_to_pos != -1 && (p_to_pos < 1 || p_to_pos > csound_instances.size()));

    edited = true;

    if (p_csound == p_to_pos) {
        return;
    }

    CsoundInstance *csound_instance = csound_instances[p_csound];
    csound_instances.remove_at(p_csound);

    if (p_to_pos == -1) {
        csound_instances.push_back(csound_instance);
    } else if (p_to_pos < p_csound) {
        csound_instances.insert(p_to_pos, csound_instance);
    } else {
        csound_instances.insert(p_to_pos - 1, csound_instance);
    }

    emit_signal("csound_layout_changed");
}

void CsoundServer::set_csound_name(int p_csound, const String &p_name) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());
    if (p_csound == 0 && p_name != "Main") {
        return; // csound 0 is always main
    }

    edited = true;

    if (csound_instances[p_csound]->csound_name == p_name) {
        return;
    }

    String attempt = p_name;
    int attempts = 1;

    while (true) {
        bool name_free = true;
        for (int i = 0; i < csound_instances.size(); i++) {
            if (csound_instances[i]->csound_name == attempt) {
                name_free = false;
                break;
            }
        }

        if (name_free) {
            break;
        }

        attempts++;
        attempt = p_name + String(" ") + itos(attempts);
    }
    csound_map.erase(csound_instances[p_csound]->csound_name);
    csound_instances[p_csound]->csound_name = attempt;
    csound_map[attempt] = csound_instances[p_csound];

    emit_signal("csound_layout_changed");
}

String CsoundServer::get_csound_name(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), String());
    return csound_instances[p_csound]->csound_name;
}

int CsoundServer::get_csound_index(const StringName &p_csound_name) const {
    for (int i = 0; i < csound_instances.size(); ++i) {
        if (csound_instances[i]->csound_name == p_csound_name) {
            return i;
        }
    }
    return -1;
}

String CsoundServer::get_csound_name_options() const {
    String options;
    for (int i = 0; i < get_csound_count(); i++) {
        if (i > 0) {
            options += ",";
        }
        String name = get_csound_name(i);
        options += name;
    }
    return options;
}

int CsoundServer::get_csound_channel_count(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    return csound_instances[p_csound]->get_channel_count();
}

int CsoundServer::get_csound_named_channel_count(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    return csound_instances[p_csound]->get_named_channel_count();
}

String CsoundServer::get_csound_named_channel_name(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), "");
    return csound_instances[p_csound]->get_named_channel_name(p_channel);
}

void CsoundServer::set_csound_volume_db(int p_csound, float p_volume_db) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->volume_db = p_volume_db;
}

float CsoundServer::get_csound_volume_db(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    return csound_instances[p_csound]->volume_db;
}

void CsoundServer::set_csound_tab(int p_csound, float p_tab) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->tab = p_tab;
}

int CsoundServer::get_csound_tab(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    return csound_instances[p_csound]->tab;
}

void CsoundServer::set_csound_solo(int p_csound, bool p_enable) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->solo = p_enable;
}

bool CsoundServer::is_csound_solo(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), false);

    return csound_instances[p_csound]->solo;
}

void CsoundServer::set_csound_mute(int p_csound, bool p_enable) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->mute = p_enable;
}

bool CsoundServer::is_csound_mute(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), false);

    return csound_instances[p_csound]->mute;
}

void CsoundServer::set_csound_bypass(int p_csound, bool p_enable) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->bypass = p_enable;
}

bool CsoundServer::is_csound_bypassing(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), false);

    return csound_instances[p_csound]->bypass;
}

void CsoundServer::set_csound_script(int p_csound, Ref<CsoundFileReader> p_script) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->set_csound_script(p_script);
}

Ref<CsoundFileReader> CsoundServer::get_csound_script(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), NULL);

    return csound_instances[p_csound]->script;
}

void CsoundServer::add_csound_instrument(int p_csound, const Ref<CsoundInstrument> &p_instrument, int p_at_pos) {
    ERR_FAIL_COND(p_instrument.is_null());
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    if (p_at_pos >= csound_instances[p_csound]->instruments.size() || p_at_pos < 0) {
        csound_instances[p_csound]->instruments.push_back(p_instrument);
    } else {
        csound_instances[p_csound]->instruments.insert(p_at_pos, p_instrument);
    }

    //_update_csound_instruments(p_csound);

    emit_signal("csound_layout_changed");
}

void CsoundServer::remove_csound_instrument(int p_csound, int p_instrument) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());

    edited = true;

    csound_instances[p_csound]->instruments.remove_at(p_instrument);
    //_update_csound_instruments(p_csound);

    emit_signal("csound_layout_changed");
}

int CsoundServer::get_csound_instrument_count(int p_csound) {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);

    return csound_instances[p_csound]->instruments.size();
}

Ref<CsoundInstrument> CsoundServer::get_csound_instrument(int p_csound, int p_instrument) {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), Ref<CsoundInstrument>());
    ERR_FAIL_INDEX_V(p_instrument, csound_instances[p_csound]->instruments.size(), Ref<CsoundInstrument>());

    return csound_instances[p_csound]->instruments[p_instrument];
}

void CsoundServer::swap_csound_instruments(int p_csound, int p_instrument, int p_by_instrument) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());
    ERR_FAIL_INDEX(p_instrument, csound_instances[p_csound]->instruments.size());
    ERR_FAIL_INDEX(p_by_instrument, csound_instances[p_csound]->instruments.size());

    edited = true;

    SWAP(csound_instances.write[p_csound]->instruments.write[p_instrument],
         csound_instances.write[p_csound]->instruments.write[p_by_instrument]);
    //_update_csound_instruments(p_csound);
    emit_signal("csound_layout_changed");
}

float CsoundServer::get_csound_channel_peak_volume_db(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    ERR_FAIL_INDEX_V(p_channel, csound_instances[p_csound]->output_channels.size(), 0);

    return csound_instances[p_csound]->output_channels[p_channel].peak_volume;
}

float CsoundServer::get_csound_named_channel_peak_volume_db(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    ERR_FAIL_INDEX_V(p_channel, csound_instances[p_csound]->output_named_channels.size(), 0);

    return csound_instances[p_csound]->output_named_channels[p_channel].peak_volume;
}

bool CsoundServer::is_csound_channel_active(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), false);
    if (p_channel >= csound_instances[p_csound]->output_channels.size()) {
        return false;
    }

    ERR_FAIL_INDEX_V(p_channel, csound_instances[p_csound]->output_channels.size(), false);

    return csound_instances[p_csound]->output_channels[p_channel].active;
}

bool CsoundServer::is_csound_named_channel_active(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), false);
    ERR_FAIL_INDEX_V(p_channel, csound_instances[p_csound]->output_named_channels.size(), false);

    return csound_instances[p_csound]->output_named_channels[p_channel].active;
}

bool CsoundServer::load_default_csound_layout() {
    if (layout_loaded) {
        return true;
    }

    String layout_path =
        ProjectSettings::get_singleton()->get_setting("audio/csound/default_csound_layout");

    if (layout_path.is_empty() || layout_path.get_file() == "<null>") {
        layout_path = "res://default_csound_layout.tres";
    }

    if (ResourceLoader::get_singleton()->exists(layout_path)) {
        Ref<CsoundLayout> default_layout = ResourceLoader::get_singleton()->load(layout_path);
        if (default_layout.is_valid()) {
            set_csound_layout(default_layout);
            emit_signal("csound_layout_changed");
            return true;
        }
    }
    return false;
}

void CsoundServer::set_csound_layout(const Ref<CsoundLayout> &p_csound_layout) {
    ERR_FAIL_COND(p_csound_layout.is_null() || p_csound_layout->csounds.size() == 0);

    int prev_size = csound_instances.size();
    for (int i = prev_size; i < csound_instances.size(); i++) {
        csound_instances[i]->stop();
        memdelete(csound_instances[i]);
    }
    csound_instances.resize(p_csound_layout->csounds.size());
    csound_map.clear();
    for (int i = 0; i < p_csound_layout->csounds.size(); i++) {
        CsoundInstance *csound;
        if (i >= prev_size) {
            csound = memnew(CsoundInstance);
        } else {
            csound = csound_instances[i];
            csound->reset();
        }

        if (i == 0) {
            csound->csound_name = "Main";
        } else {
            csound->csound_name = p_csound_layout->csounds[i].name;
        }

        csound->solo = p_csound_layout->csounds[i].solo;
        csound->mute = p_csound_layout->csounds[i].mute;
        csound->bypass = p_csound_layout->csounds[i].bypass;
        csound->volume_db = p_csound_layout->csounds[i].volume_db;
        csound->tab = p_csound_layout->csounds[i].tab;
        csound->script = p_csound_layout->csounds[i].script;
        csound->instruments.resize(p_csound_layout->csounds[i].instruments.size());
        for (int j = 0; j < csound->instruments.size(); j++) {
            csound->instruments.write[j] = p_csound_layout->csounds[i].instruments[j];
        }

        csound_map[csound->csound_name] = csound;
        csound_instances.write[i] = csound;

        csound->call_deferred("initialize");
        if (!csound->is_connected("csound_ready", Callable(this, "on_csound_ready"))) {
            csound->connect("csound_ready", Callable(this, "on_csound_ready"));
        }
    }
    edited = false;
    layout_loaded = true;
}

void CsoundServer::on_csound_ready(String csound_name) {
    emit_signal("csound_ready", csound_name);
}

Ref<CsoundLayout> CsoundServer::generate_csound_layout() const {
    Ref<CsoundLayout> state;
    state.instantiate();

    state->csounds.resize(csound_instances.size());

    for (int i = 0; i < csound_instances.size(); i++) {
        state->csounds.write[i].name = csound_instances[i]->csound_name;
        state->csounds.write[i].mute = csound_instances[i]->mute;
        state->csounds.write[i].solo = csound_instances[i]->solo;
        state->csounds.write[i].bypass = csound_instances[i]->bypass;
        state->csounds.write[i].volume_db = csound_instances[i]->volume_db;
        state->csounds.write[i].tab = csound_instances[i]->tab;
        state->csounds.write[i].script = csound_instances[i]->script;
        state->csounds.write[i].instruments.resize(csound_instances[i]->instruments.size());
        for (int j = 0; j < csound_instances[i]->instruments.size(); j++) {
            state->csounds.write[i].instruments.write[j] = csound_instances[i]->instruments[j];
        }
    }

    return state;
}

Error CsoundServer::start() {
    thread_exited = false;
    thread.instantiate();
    mutex.instantiate();
    thread->start(callable_mp(this, &CsoundServer::thread_func), Thread::PRIORITY_NORMAL);
    return OK;
}

void CsoundServer::lock() {
    if (thread.is_null() || mutex.is_null()) {
        return;
    }
    mutex->lock();
}

void CsoundServer::unlock() {
    if (thread.is_null() || mutex.is_null()) {
        return;
    }
    mutex->unlock();
}

void CsoundServer::finish() {
    exit_thread = true;
    thread->wait_to_finish();
}

CsoundInstance *CsoundServer::get_csound(const String &p_name) {
    if (csound_map.has(p_name)) {
        return csound_map.get(p_name);
    }

    return NULL;
}

CsoundInstance *CsoundServer::get_csound_by_index(int p_index) {
    return csound_instances.get(p_index);
}

CsoundInstance *CsoundServer::get_csound_(const Variant &p_variant) {
    if (p_variant.get_type() == Variant::STRING) {
        String str = p_variant;
        return csound_map.get(str);
    }

    if (p_variant.get_type() == Variant::INT) {
        int index = p_variant.operator int();
        return csound_instances.get(index);
    }

    return NULL;
}

extern "C" {
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_KEEPALIVE
#endif
    void dynCall_midi_note_on(int channel, int note, int velocity) {
        Ref<InputEventMIDI> event;
        event.instantiate();
        event->set_device(0);
        event->set_channel(0);

        event->set_message(MIDIMessage::MIDI_MESSAGE_NOTE_ON);
        event->set_pitch(note);
        event->set_velocity(velocity);

        Input::get_singleton()->parse_input_event(event);

        printf("note_on: note = %d velocity = %d\n", note, velocity);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_KEEPALIVE
#endif
    void dynCall_midi_note_off(int channel, int note) {
        Ref<InputEventMIDI> event;
        event.instantiate();
        event->set_device(0);
        event->set_channel(0);

        event->set_message(MIDIMessage::MIDI_MESSAGE_NOTE_OFF);
        event->set_pitch(note);
        event->set_velocity(0);

        Input::get_singleton()->parse_input_event(event);

        printf("note_off: note = %d\n", note);
    }
}

#ifdef __EMSCRIPTEN__
EM_JS(void, open_midi_input, (), {

function onMIDIMessage (message) {
    var channel = message.data[0] & 0xf;
    var command = message.data[0] >> 4;
    var note = message.data[1];
    var velocity = (message.data.length > 2) ? message.data[2] : 0;
    switch (command) {
        case 9: // noteOn
            if (velocity > 0) {
                Module.dynCall_midi_note_on(channel, note, velocity);
            } else {
                Module.dynCall_midi_note_off(channel, note);
            }
            break;
        case 8: // noteOff
			Module.dynCall_midi_note_off(channel, note);
            break;
    }
}

function failure () {
    alert('No access to your midi devices.')
}

function success (midi) {
    var inputs = midi.inputs.values();
    for (var input = inputs.next(); input && !input.done; input = inputs.next()) {
        input.value.onmidimessage = onMIDIMessage;
    }
}

if (navigator.requestMIDIAccess) {
    navigator.requestMIDIAccess().then(success, failure);
} else {
    alert('no midi support, you cannot play');
}

});

#endif

void CsoundServer::open_web_midi_inputs() {
#ifdef __EMSCRIPTEN__
    open_midi_input();
#endif
}

String CsoundServer::get_version() {
    return GODOT_CSOUND_VERSION;
}

String CsoundServer::get_build() {
    return GODOT_CSOUND_BUILD;
}

void CsoundServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &CsoundServer::initialize);

    ClassDB::bind_method(D_METHOD("get_version"), &CsoundServer::get_version);
    ClassDB::bind_method(D_METHOD("get_build"), &CsoundServer::get_build);

    ClassDB::bind_method(D_METHOD("set_edited", "edited"), &CsoundServer::set_edited);
    ClassDB::bind_method(D_METHOD("get_edited"), &CsoundServer::get_edited);

    ClassDB::bind_method(D_METHOD("set_csound_count", "amount"), &CsoundServer::set_csound_count);
    ClassDB::bind_method(D_METHOD("get_csound_count"), &CsoundServer::get_csound_count);

    ClassDB::bind_method(D_METHOD("remove_csound", "index"), &CsoundServer::remove_csound);
    ClassDB::bind_method(D_METHOD("add_csound", "at_position"), &CsoundServer::add_csound, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("move_csound", "index", "to_index"), &CsoundServer::move_csound);

    ClassDB::bind_method(D_METHOD("set_csound_name", "csound_idx", "name"), &CsoundServer::set_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_name", "csound_idx"), &CsoundServer::get_csound_name);
    ClassDB::bind_method(D_METHOD("get_csound_index", "csound_name"), &CsoundServer::get_csound_index);

    ClassDB::bind_method(D_METHOD("get_csound_name_options"), &CsoundServer::get_csound_name_options);

    ClassDB::bind_method(D_METHOD("get_csound_channel_count", "csound_idx"), &CsoundServer::get_csound_channel_count);
    ClassDB::bind_method(D_METHOD("get_csound_named_channel_count", "csound_idx"),
                         &CsoundServer::get_csound_named_channel_count);
    ClassDB::bind_method(D_METHOD("get_csound_named_channel_name", "csound_idx", "channel"),
                         &CsoundServer::get_csound_named_channel_name);

    ClassDB::bind_method(D_METHOD("set_csound_volume_db", "csound_idx", "volume_db"),
                         &CsoundServer::set_csound_volume_db);
    ClassDB::bind_method(D_METHOD("get_csound_volume_db", "csound_idx"), &CsoundServer::get_csound_volume_db);

    ClassDB::bind_method(D_METHOD("set_csound_tab", "csound_idx", "tab"), &CsoundServer::set_csound_tab);
    ClassDB::bind_method(D_METHOD("get_csound_tab", "csound_idx"), &CsoundServer::get_csound_tab);

    ClassDB::bind_method(D_METHOD("set_csound_solo", "csound_idx", "enable"), &CsoundServer::set_csound_solo);
    ClassDB::bind_method(D_METHOD("is_csound_solo", "csound_idx"), &CsoundServer::is_csound_solo);

    ClassDB::bind_method(D_METHOD("set_csound_mute", "csound_idx", "enable"), &CsoundServer::set_csound_mute);
    ClassDB::bind_method(D_METHOD("is_csound_mute", "csound_idx"), &CsoundServer::is_csound_mute);

    ClassDB::bind_method(D_METHOD("set_csound_bypass", "csound_idx", "enable"), &CsoundServer::set_csound_bypass);
    ClassDB::bind_method(D_METHOD("is_csound_bypassing", "csound_idx"), &CsoundServer::is_csound_bypassing);

    ClassDB::bind_method(D_METHOD("set_csound_script", "csound_idx", "script"), &CsoundServer::set_csound_script);
    ClassDB::bind_method(D_METHOD("get_csound_script", "csound_idx"), &CsoundServer::get_csound_script);

    ClassDB::bind_method(D_METHOD("add_csound_instrument", "csound_idx", "instrument", "at_position"),
                         &CsoundServer::add_csound_instrument, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("remove_csound_instrument", "csound_idx", "instrument_idx"),
                         &CsoundServer::remove_csound_instrument);
    ClassDB::bind_method(D_METHOD("get_csound_instrument_count", "csound_idx"),
                         &CsoundServer::get_csound_instrument_count);
    ClassDB::bind_method(D_METHOD("get_csound_instrument", "csound_idx", "instrument_idx"),
                         &CsoundServer::get_csound_instrument);
    ClassDB::bind_method(D_METHOD("swap_csound_instruments", "csound_idx", "instrument_idx", "by_instrument_idx"),
                         &CsoundServer::swap_csound_instruments);

    ClassDB::bind_method(D_METHOD("get_csound_channel_peak_volume_db", "csound_idx", "channel"),
                         &CsoundServer::get_csound_channel_peak_volume_db);
    ClassDB::bind_method(D_METHOD("get_csound_named_channel_peak_volume_db", "csound_idx", "channel"),
                         &CsoundServer::get_csound_named_channel_peak_volume_db);

    ClassDB::bind_method(D_METHOD("is_csound_channel_active", "csound_idx", "channel"),
                         &CsoundServer::is_csound_channel_active);
    ClassDB::bind_method(D_METHOD("is_csound_named_channel_active", "csound_idx", "channel"),
                         &CsoundServer::is_csound_named_channel_active);

    ClassDB::bind_method(D_METHOD("lock"), &CsoundServer::lock);
    ClassDB::bind_method(D_METHOD("unlock"), &CsoundServer::unlock);

    ClassDB::bind_method(D_METHOD("set_csound_layout", "csound_layout"), &CsoundServer::set_csound_layout);
    ClassDB::bind_method(D_METHOD("generate_csound_layout"), &CsoundServer::generate_csound_layout);

    ClassDB::bind_method(D_METHOD("get_csound", "csound_name"), &CsoundServer::get_csound);

    ClassDB::bind_method(D_METHOD("open_web_midi_inputs"), &CsoundServer::open_web_midi_inputs);

    ClassDB::bind_method(D_METHOD("on_csound_ready", "csound_name"), &CsoundServer::on_csound_ready);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "csound_count"), "set_csound_count", "get_csound_count");

    ADD_SIGNAL(MethodInfo("csound_layout_changed"));
    ADD_SIGNAL(MethodInfo("csound_ready", PropertyInfo(Variant::STRING, "csound_name")));
}
