#include "soundfont_file_reader.h"

using namespace godot;

SoundFontFileReader::SoundFontFileReader() {
}

SoundFontFileReader::~SoundFontFileReader() {
}

void SoundFontFileReader::_init() {
}

void SoundFontFileReader::clear_data() {
}

void SoundFontFileReader::set_data(PackedByteArray data) {
    array_data = data;
}

PackedByteArray SoundFontFileReader::get_data() {
    return array_data;
}

char *SoundFontFileReader::get_array_data() {
    return (char *)array_data.ptr();
}

long SoundFontFileReader::get_array_size() {
    return array_data.size();
}

String SoundFontFileReader::get_extension() {
    return "sf2str";
}

void SoundFontFileReader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_data"), &SoundFontFileReader::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &SoundFontFileReader::get_data);
    ClassDB::bind_method(D_METHOD("get_extension"), &SoundFontFileReader::get_extension);
    ClassDB::add_property("SoundFontFileReader", PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data",
                          "get_data");
}
