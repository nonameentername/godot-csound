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
    PackedByteArray in_array = data;
    PackedByteArray out_array;
    for (int i = 0; i < in_array.size(); i++) {
        out_array.append(in_array[i]);
    }
    array_data = out_array;
}

PackedByteArray SoundFontFileReader::get_data() {
    PackedByteArray out_array;
    for (int i = 0; i < array_data.size(); i++) {
        out_array.append(array_data[i]);
    }
    return out_array;
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
