#include "csound_layout.h"
#include "csound_instrument.h"

using namespace godot;

bool CsoundLayout::_set(const StringName &p_name, const Variant &p_value) {
    String s = p_name;
    if (s.begins_with("csound/")) {
        int index = s.get_slice("/", 1).to_int();
        if (csounds.size() <= index) {
            csounds.resize(index + 1);
        }

        Csound &csound = csounds.write[index];

        String what = s.get_slice("/", 2);

        if (what == "name") {
            csound.name = p_value;
        } else if (what == "solo") {
            csound.solo = p_value;
        } else if (what == "mute") {
            csound.mute = p_value;
        } else if (what == "bypass") {
            csound.bypass = p_value;
        } else if (what == "volume_db") {
            csound.volume_db = p_value;
        } else if (what == "tab") {
            csound.tab = p_value;
        } else if (what == "script") {
            csound.script = p_value;

        } else if (what == "instrument") {
            int which = s.get_slice("/", 3).to_int();
            if (csound.instruments.size() <= which) {
                csound.instruments.resize(which + 1);
            }
            csound.instruments.write[which] = p_value;
        } else {
            return false;
        }

        return true;
    }

    return false;
}

bool CsoundLayout::_get(const StringName &p_name, Variant &r_ret) const {
    String s = p_name;
    if (s.begins_with("csound/")) {
        int index = s.get_slice("/", 1).to_int();
        if (index < 0 || index >= csounds.size()) {
            return false;
        }

        const Csound &csound = csounds[index];

        String what = s.get_slice("/", 2);

        if (what == "name") {
            r_ret = csound.name;
        } else if (what == "solo") {
            r_ret = csound.solo;
        } else if (what == "mute") {
            r_ret = csound.mute;
        } else if (what == "bypass") {
            r_ret = csound.bypass;
        } else if (what == "volume_db") {
            r_ret = csound.volume_db;
        } else if (what == "tab") {
            r_ret = csound.tab;
        } else if (what == "script") {
            r_ret = csound.script;
        } else if (what == "instrument") {
            int which = s.get_slice("/", 3).to_int();
            if (which < 0 || which >= csound.instruments.size()) {
                return false;
            }
            r_ret = csound.instruments[which];
        } else {
            return false;
        }

        return true;
    }

    return false;
}

void CsoundLayout::_get_property_list(List<PropertyInfo> *p_list) const {
    for (int i = 0; i < csounds.size(); i++) {
        p_list->push_back(PropertyInfo(Variant::STRING, "csound/" + itos(i) + "/name", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        p_list->push_back(PropertyInfo(Variant::BOOL, "csound/" + itos(i) + "/solo", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        p_list->push_back(PropertyInfo(Variant::BOOL, "csound/" + itos(i) + "/mute", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        p_list->push_back(PropertyInfo(Variant::BOOL, "csound/" + itos(i) + "/bypass", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        p_list->push_back(PropertyInfo(Variant::FLOAT, "csound/" + itos(i) + "/volume_db", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        p_list->push_back(PropertyInfo(Variant::INT, "csound/" + itos(i) + "/tab", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        p_list->push_back(PropertyInfo(Variant::OBJECT, "csound/" + itos(i) + "/script", PROPERTY_HINT_NONE, "",
                                       PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));

        for (int j = 0; j < csounds[i].instruments.size(); j++) {
            p_list->push_back(PropertyInfo(Variant::OBJECT, "csound/" + itos(i) + "/instrument/" + itos(j),
                                           PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL));
        }
    }
}

CsoundLayout::CsoundLayout() {
    csounds.resize(1);
    csounds.write[0].name = "Main";
}

void CsoundLayout::_bind_methods() {
}
