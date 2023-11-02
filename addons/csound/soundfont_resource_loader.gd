@tool
class_name SoundFontDataLoader
extends ResourceFormatLoader


func _get_recognized_extensions():
	return PackedStringArray(["sf2str"])


func _get_resource_type(path):
	var ext = path.get_extension().to_lower()
	if ext == "sf2str":
		return "SoundFontFileReader"
	return ""


func _handles_type(typename):
	return typename == "SoundFontFileReader"


func _load(path, original_path, use_sub_threads, cache_mode):
	var f = FileAccess.open(path, FileAccess.READ)
	if f == null:
		return FileAccess.get_open_error()

	var res = SoundFontFileReader.new()
	var data = f.get_buffer(f.get_length())
	res.set_data(data)
	f.close()

	return res
