#include "csound_instrument.h"

using namespace godot;

CsoundInstrument::CsoundInstrument() {
    name = "NewInstrument";
}

void CsoundInstrument::set_name(const String &p_name) {
    name = p_name;
}

const String &CsoundInstrument::get_name() const {
    return name;
}

bool CsoundInstrument::_set(const StringName &p_name, const Variant &p_value) {
    String s = p_name;
    String value = p_value;

    if (s == "name") {
        name = p_value;
        emit_changed();
        return true;
    }
    return false;
}

bool CsoundInstrument::_get(const StringName &p_name, Variant &r_ret) const {
    String s = p_name;

    if (s == "name") {
        r_ret = name;
        return true;
    }
    return false;
}

void CsoundInstrument::_get_property_list(List<PropertyInfo> *p_list) const {
    p_list->push_back(PropertyInfo(Variant::STRING, "name", PROPERTY_HINT_NONE, ""));
}

void CsoundInstrument::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_name", "name"), &CsoundInstrument::set_name);
    ClassDB::bind_method(D_METHOD("get_name"), &CsoundInstrument::get_name);
}
