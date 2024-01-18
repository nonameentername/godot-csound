@tool
class_name CsoundDataSaver
extends ResourceFormatSaver


func _get_recognized_extensions(res):
	if res.has_method("get_extension"):
		return PackedStringArray([res.get_extension()])
	else:
		return PackedStringArray()


func _recognize(res):
	return res.has_method("get_extension")


func _save(resource, path, flags):
	var f = FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		return FileAccess.get_open_error()

	f.store_buffer(resource.get_data())
	f.close()
