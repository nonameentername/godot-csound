#ifndef SOUNDFONTFILEREADER_H
#define SOUNDFONTFILEREADER_H

#include <csound.h>

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class SoundFontFileReader : public Resource {
    GDCLASS(SoundFontFileReader, Resource)

private:
    String file;

    int position;
    PackedByteArray array_data;
    void clear_data();

public:
    SoundFontFileReader();
    ~SoundFontFileReader();

    void _init();

    void set_data(PackedByteArray data);
    PackedByteArray get_data();
    char *get_array_data();
    long get_array_size();
    String get_extension();

    static void _bind_methods();
};

} // namespace godot

#endif
