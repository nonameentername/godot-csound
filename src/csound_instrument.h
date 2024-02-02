#ifndef CSOUNDINSTRUMENT_H
#define CSOUNDINSTRUMENT_H

#include "godot_cpp/classes/resource.hpp"

namespace godot {

class CsoundInstrument : public Resource {
    GDCLASS(CsoundInstrument, Resource);

private:
    String name;

protected:
    static void _bind_methods();

    bool _set(const StringName &p_name, const Variant &p_value);
    bool _get(const StringName &p_name, Variant &r_ret) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

public:
    CsoundInstrument();

    void set_name(const String &name);
    const String &get_name() const;
};
} // namespace godot

#endif
