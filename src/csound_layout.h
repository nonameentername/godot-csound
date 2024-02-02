#ifndef CSOUNDLAYOUT_H
#define CSOUNDLAYOUT_H

#include "csound_file_reader.h"
#include "csound_instrument.h"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>

namespace godot {

class CsoundLayout : public Resource {
    GDCLASS(CsoundLayout, Resource);
    friend class CsoundServer;

    struct Csound {
        String name;
        bool solo = false;
        bool mute = false;
        bool bypass = false;
        float volume_db = 0.0f;
        int tab = 0;

        Ref<CsoundFileReader> script;
        Vector<Ref<CsoundInstrument>> instruments;

        Csound() {
        }
    };

    Vector<Csound> csounds;

protected:
    static void _bind_methods();

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

public:
    CsoundLayout();
};

} // namespace godot

#endif
