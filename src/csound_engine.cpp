#include "csound_engine.h"

using namespace godot;

CsoundEngine *CsoundEngine::singleton = NULL;

CsoundEngine::CsoundEngine() {
    set_process_internal(true);
    singleton = this;
    call_deferred("initialize");
}

CsoundEngine::~CsoundEngine() {
    singleton = NULL;
}

CsoundEngine *CsoundEngine::get_singleton() {
    return singleton;
}

void CsoundEngine::initialize() {
    Object::cast_to<SceneTree>(Engine::get_singleton()->get_main_loop())->get_root()->add_child(this);
}

void CsoundEngine::_notification(int p_what) {
    switch (p_what) {
    case NOTIFICATION_INTERNAL_PROCESS: {
        process(get_process_delta_time());
    }
    }
}

void CsoundEngine::process(double delta) {
}

CsoundGodot *CsoundEngine::create(String name) {
    if (!csound_instances.has(name)) {
        CsoundGodot *csound = memnew(CsoundGodot);
        csound_instances.insert(name, csound);
        call_deferred("add_child", csound);

        return csound;
    } else {
        return csound_instances.get(name);
    }
}

CsoundGodot *CsoundEngine::get(String name) {
    if (csound_instances.has(name)) {
        return csound_instances.get(name);
    }

    return NULL;
}

bool CsoundEngine::has(String name) {
    return csound_instances.has(name);
}

void CsoundEngine::erase(String name) {
    if (csound_instances.has(name)) {
        CsoundGodot *csound = csound_instances.get(name);
        csound_instances.erase(name);
        csound->queue_free();
    }
}

void CsoundEngine::rename(String prev_name, String name) {
    if (csound_instances.has(prev_name)) {
        CsoundGodot *csound = csound_instances.get(prev_name);
        csound_instances.erase(prev_name);
        csound_instances.insert(name, csound);
    }
}

void CsoundEngine::add(const String &name, CsoundGodot *csound) {
    csound_instances.insert(name, csound);
}

void CsoundEngine::remove(const String &name) {
    if (csound_instances.has(name)) {
        csound_instances.erase(name);
    }
}

void CsoundEngine::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &CsoundEngine::initialize);
    ClassDB::bind_method(D_METHOD("process", "delta"), &CsoundEngine::process);
    ClassDB::bind_method(D_METHOD("get", "name"), &CsoundEngine::get);
    ClassDB::bind_method(D_METHOD("erase", "name"), &CsoundEngine::erase);
}
