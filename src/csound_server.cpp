#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "csound_layout.h"
#include "csound_server.h"
#include "godot_cpp/core/error_macros.hpp"

using namespace godot;

CsoundServer *CsoundServer::singleton = NULL;

CsoundServer::CsoundServer() {
    edited = false;
    set_process_internal(true);
    singleton = this;
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

void CsoundServer::initialize() {
    Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop())->get_root()->add_child(this);

    String name = "audio/csound/default_csound_layout";
    String default_filename = "res://default_csound_layout.tres";
    String layout_path = ProjectSettings::get_singleton()->get_setting_with_override(name);

    if (layout_path.is_empty()) {
        ProjectSettings::get_singleton()->set_setting(name, default_filename);
        Dictionary property_info;
        property_info["name"] = name;
        property_info["type"] = GDEXTENSION_VARIANT_TYPE_STRING;
        property_info["hint"] = PROPERTY_HINT_FILE;
        property_info["hint_string"] = "";
        ProjectSettings::get_singleton()->add_property_info(property_info);
        ProjectSettings::get_singleton()->set_initial_value(name, default_filename);
        Error error = ProjectSettings::get_singleton()->save();
        ERR_FAIL_COND_MSG(error != OK, "Could not save project settings");
    }

    if (!load_default_csound_layout()) {
        set_csound_count(1);
    }

    set_edited(false);
}

void CsoundServer::process(double delta) {
    bool use_solo = false;
    for (int i = 0; i < csound_instances.size(); i++) {
        if (csound_instances[i]->solo == true) {
            use_solo = true;
        }
    }

    if (use_solo != solo_mode) {
        solo_mode = use_solo;
    }
}

void CsoundServer::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_INTERNAL_PROCESS: {
        process(get_process_delta_time());
    }
    }
}

void CsoundServer::set_csound_count(int p_count) {
    ERR_FAIL_COND(p_count < 1);
    ERR_FAIL_INDEX(p_count, 256);

    edited = true;

    lock();
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

        csound_instances.write[i] = memnew(CsoundGodot);
        csound_instances[i]->csound_name = attempt;
        csound_instances[i]->solo = false;
        csound_instances[i]->mute = false;
        csound_instances[i]->bypass = false;
        csound_instances[i]->volume_db = 0;

        csound_map[attempt] = csound_instances[i];
        call_deferred("add_child", csound_instances[i]);
    }

    unlock();

    emit_signal("csound_layout_changed");
}

int CsoundServer::get_csound_count() const {
    return csound_instances.size();
}

void CsoundServer::remove_csound(int p_index) {
    ERR_FAIL_INDEX(p_index, csound_instances.size());
    ERR_FAIL_COND(p_index == 0);

    edited = true;

    lock();
    csound_map.erase(csound_instances[p_index]->csound_name);
    memdelete(csound_instances[p_index]);
    csound_instances.remove_at(p_index);
    unlock();

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

    CsoundGodot *csound_godot = memnew(CsoundGodot);
    csound_godot->csound_name = attempt;
    csound_godot->solo = false;
    csound_godot->mute = false;
    csound_godot->bypass = false;
    csound_godot->volume_db = 0;

    csound_map[attempt] = csound_godot;
    call_deferred("add_child", csound_godot);

    if (p_at_pos == -1) {
        csound_instances.push_back(csound_godot);
    } else {
        csound_instances.insert(p_at_pos, csound_godot);
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

    CsoundGodot *csound_godot = csound_instances[p_csound];
    csound_instances.remove_at(p_csound);

    if (p_to_pos == -1) {
        csound_instances.push_back(csound_godot);
    } else if (p_to_pos < p_csound) {
        csound_instances.insert(p_to_pos, csound_godot);
    } else {
        csound_instances.insert(p_to_pos - 1, csound_godot);
    }

    emit_signal("csound_layout_changed");
}

void CsoundServer::set_csound_name(int p_csound, const String &p_name) {
    ERR_FAIL_INDEX(p_csound, csound_instances.size());
    if (p_csound == 0 && p_name != "Main") {
        return; // csound 0 is always master
    }

    edited = true;

    lock();

    if (csound_instances[p_csound]->csound_name == p_name) {
        unlock();
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
    unlock();

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

int CsoundServer::get_csound_channel_count(int p_csound) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    return csound_instances[p_csound]->get_channel_count();
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

/*
    instruments
*/

float CsoundServer::get_csound_peak_volume_db(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), 0);
    ERR_FAIL_INDEX_V(p_channel, csound_instances[p_csound]->output_channels.size(), 0);

    return csound_instances[p_csound]->output_channels[p_channel].peak_volume;
}

bool CsoundServer::is_csound_channel_active(int p_csound, int p_channel) const {
    ERR_FAIL_INDEX_V(p_csound, csound_instances.size(), false);
    ERR_FAIL_INDEX_V(p_channel, csound_instances[p_csound]->output_channels.size(), false);

    return csound_instances[p_csound]->output_channels[p_channel].active;
}

bool CsoundServer::load_default_csound_layout() {
    String layout_path =
        ProjectSettings::get_singleton()->get_setting_with_override("audio/csound/default_csound_layout");

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

    lock();
    for (int i = 0; i < csound_instances.size(); i++) {
        memdelete(csound_instances[i]);
    }
    csound_instances.resize(p_csound_layout->csounds.size());
    csound_map.clear();
    for (int i = 0; i < p_csound_layout->csounds.size(); i++) {
        CsoundGodot *csound = memnew(CsoundGodot);
        if (i == 0) {
            csound->csound_name = "Main";
        } else {
            csound->csound_name = p_csound_layout->csounds[i].name;
        }

        csound->solo = p_csound_layout->csounds[i].solo;
        csound->mute = p_csound_layout->csounds[i].mute;
        csound->bypass = p_csound_layout->csounds[i].bypass;
        csound->volume_db = p_csound_layout->csounds[i].volume_db;

        csound_map[csound->csound_name] = csound;
        csound_instances.write[i] = csound;
        call_deferred("add_child", csound_instances[i]);
    }
    edited = false;
    unlock();
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
    }

    return state;
}

void CsoundServer::lock() {
}

void CsoundServer::unlock() {
}

CsoundGodot *CsoundServer::get_csound(const String &p_name) {
    if (csound_map.has(p_name)) {
        return csound_map.get(p_name);
    }

    return NULL;
}

CsoundGodot *CsoundServer::get_csound_(const Variant &p_variant) {
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

void CsoundServer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &CsoundServer::initialize);
    ClassDB::bind_method(D_METHOD("process", "delta"), &CsoundServer::process);

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

    ClassDB::bind_method(D_METHOD("get_csound_channel_count", "csound_idx"), &CsoundServer::get_csound_channel_count);

    ClassDB::bind_method(D_METHOD("set_csound_volume_db", "csound_idx", "volume_db"),
                         &CsoundServer::set_csound_volume_db);
    ClassDB::bind_method(D_METHOD("get_csound_volume_db", "csound_idx"), &CsoundServer::get_csound_volume_db);

    ClassDB::bind_method(D_METHOD("set_csound_solo", "csound_idx", "enable"), &CsoundServer::set_csound_solo);
    ClassDB::bind_method(D_METHOD("is_csound_solo", "csound_idx"), &CsoundServer::is_csound_solo);

    ClassDB::bind_method(D_METHOD("set_csound_mute", "csound_idx", "enable"), &CsoundServer::set_csound_mute);
    ClassDB::bind_method(D_METHOD("is_csound_mute", "csound_idx"), &CsoundServer::is_csound_mute);

    ClassDB::bind_method(D_METHOD("set_csound_bypass", "csound_idx", "enable"), &CsoundServer::set_csound_bypass);
    ClassDB::bind_method(D_METHOD("is_csound_bypassing", "csound_idx"), &CsoundServer::is_csound_bypassing);

    ClassDB::bind_method(D_METHOD("get_csound_peak_volume_db", "csound_idx", "channel"),
                         &CsoundServer::get_csound_peak_volume_db);

    ClassDB::bind_method(D_METHOD("is_csound_channel_active", "csound_idx", "channel"),
                         &CsoundServer::is_csound_channel_active);

    ClassDB::bind_method(D_METHOD("lock"), &CsoundServer::lock);
    ClassDB::bind_method(D_METHOD("unlock"), &CsoundServer::unlock);

    ClassDB::bind_method(D_METHOD("set_csound_layout", "csound_layout"), &CsoundServer::set_csound_layout);
    ClassDB::bind_method(D_METHOD("generate_csound_layout"), &CsoundServer::generate_csound_layout);

    ClassDB::bind_method(D_METHOD("get_csound", "csound_name"), &CsoundServer::get_csound);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "csound_count"), "set_csound_count", "get_csound_count");

    ADD_SIGNAL(MethodInfo("csound_layout_changed"));
}
