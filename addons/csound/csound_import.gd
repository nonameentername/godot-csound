@tool
extends EditorPlugin

var midi_plugin
var soundfont_plugin
var csound_import_plugin
var csound_plugin: EditorCsoundInstances


func _enter_tree():
	midi_plugin = preload("import_midi_plugin.gd").new()
	add_import_plugin(midi_plugin)

	soundfont_plugin = preload("import_soundfont_plugin.gd").new()
	add_import_plugin(soundfont_plugin)

	csound_import_plugin = preload("import_csound_plugin.gd").new()
	add_import_plugin(csound_import_plugin)

	csound_plugin = preload("res://editor_csound_instances.tscn").instantiate()
	csound_plugin.editor_interface = get_editor_interface()
	csound_plugin.undo_redo = get_undo_redo()

	add_control_to_bottom_panel(csound_plugin, "Csound")

	resource_saved.connect(_resource_saved)

	#get_editor_interface().edit_node(csound_plugin) #keep this around


func _resource_saved(resource: Resource):
	if resource is CsoundFileReader:
		csound_plugin.resource_saved(resource)


func _exit_tree():
	remove_import_plugin(midi_plugin)

	remove_import_plugin(soundfont_plugin)

	remove_import_plugin(csound_import_plugin)

	remove_control_from_bottom_panel(csound_plugin)
	if csound_plugin:
		csound_plugin.queue_free()


func _notification(what):
	pass
