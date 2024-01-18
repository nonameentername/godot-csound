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
    array_data = data;
}

PackedByteArray MidiFileReader::get_data() {
    return array_data;
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
