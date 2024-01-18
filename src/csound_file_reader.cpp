#include "csound_file_reader.h"

using namespace godot;

CsoundFileReader::CsoundFileReader() {
}

CsoundFileReader::~CsoundFileReader() {
}

void CsoundFileReader::_init() {
}

void CsoundFileReader::clear_data() {
}

void CsoundFileReader::set_data(PackedByteArray data) {
    array_data = data;
}

PackedByteArray CsoundFileReader::get_data() {
    return array_data;
}

String CsoundFileReader::get_extension() {
    return "csoundstr";
}

char *CsoundFileReader::get_array_data() {
    return (char *)array_data.ptr();
}

long CsoundFileReader::get_array_size() {
    return array_data.size();
}

void CsoundFileReader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_data"), &CsoundFileReader::set_data);
    ClassDB::bind_method(D_METHOD("get_data"), &CsoundFileReader::get_data);
    ClassDB::bind_method(D_METHOD("get_extension"), &CsoundFileReader::get_extension);
    ClassDB::add_property("CsoundFileReader", PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data", "get_data");
}
