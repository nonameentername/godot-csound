@tool
extends EditorPlugin

var midi_plugin
var soundfont_plugin
var csound_plugin


func _enter_tree():
	midi_plugin = preload("import_midi_plugin.gd").new()
	add_import_plugin(midi_plugin)

	soundfont_plugin = preload("import_soundfont_plugin.gd").new()
	add_import_plugin(soundfont_plugin)

	csound_plugin = preload("res://editor_csound_instances.tscn").instantiate()
	csound_plugin.editor_interface = get_editor_interface()
	csound_plugin.undo_redo = get_undo_redo()

	add_control_to_bottom_panel(csound_plugin, "Csound")

	#get_editor_interface().edit_node(csound_plugin) #keep this around


func _exit_tree():
	remove_import_plugin(midi_plugin)
	#midi_plugin.free()

	remove_import_plugin(soundfont_plugin)
	#soundfont_plugin.free()

	remove_control_from_bottom_panel(csound_plugin)
	if csound_plugin:
		csound_plugin.queue_free()


func _notification(what):
	pass
