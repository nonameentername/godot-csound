#ifndef CSOUNDFILEREADER_H
#define CSOUNDFILEREADER_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class CsoundFileReader : public Resource {
    GDCLASS(CsoundFileReader, Resource)

private:
    String file;

    PackedByteArray array_data;
    void clear_data();

public:
    CsoundFileReader();
    ~CsoundFileReader();

    void _init();

    void set_data(PackedByteArray data);
    PackedByteArray get_data();
    String get_extension();

    char *get_array_data();
    long get_array_size();

    static void _bind_methods();
};

} // namespace godot

#endif
