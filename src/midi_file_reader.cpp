#include "midi_file_reader.h"

using namespace godot;

MidiFileReader::MidiFileReader() {
}

MidiFileReader::~MidiFileReader() {
}

void MidiFileReader::_init() {
}

void MidiFileReader::clear_data() {
}

void MidiFileReader::set_data(PackedByteArray data) {
    PackedByteArray in_array = data;
    PackedByteArray out_array;
    for (int i = 0; i < in_array.size(); i++) {
        out_array.append(in_array[i]);
    }
    array_data = out_array;
}

PackedByteArray MidiFileReader::get_data() {
    PackedByteArray out_array;
    for (int i = 0; i < array_data.size(); i++) {
        out_array.append(array_data[i]);
    }
    return out_array;
}

String MidiFileReader::get_extension() {
    return "midstr";
}

void MidiFileReader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_data"), &MidiFileReader::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &MidiFileReader::get_data);
    ClassDB::bind_method(D_METHOD("get_extension"), &MidiFileReader::get_extension);
    ClassDB::add_property("MidiFileReader", PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data", "get_data");
}
