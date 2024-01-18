@tool
extends EditorImportPlugin


func _get_importer_name():
	return "csound"


func _get_visible_name():
	return "Csound"


func _get_recognized_extensions():
	return ["orc", "sco", "csd", "udo", "inc"]


func _get_save_extension():
	return "csoundstr"


func _get_resource_type():
	return "CsoundFileReader"


func _get_option_visibility(path, option, options):
	return true


func _get_preset_count():
	return 1


func _get_preset_name(preset):
	return "Default"


func _get_import_options(path, preset_index):
	return []


func _get_import_order():
	return 0


func _get_priority():
	return 1


func _import(source_file, save_path, options, r_platform_variants, r_gen_files):
	var file = FileAccess.open(source_file, FileAccess.READ)
	if file == null:
		return FileAccess.get_open_error()
	var data = file.get_buffer(file.get_length())
	var csound_file = CsoundFileReader.new()
	csound_file.set_data(data)
	file.close()

	return ResourceSaver.save(csound_file, "%s.%s" % [save_path, _get_save_extension()])
