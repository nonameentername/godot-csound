#ifndef MIDIFILEREADER_H
#define MIDIFILEREADER_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class MidiFileReader : public Resource {
    GDCLASS(MidiFileReader, Resource)

private:
    String file;

    PackedByteArray array_data;
    void clear_data();

public:
    MidiFileReader();
    ~MidiFileReader();

    void _init();

    void set_data(PackedByteArray data);
    PackedByteArray get_data();
    String get_extension();

    static void _bind_methods();
};

} // namespace godot

#endif
