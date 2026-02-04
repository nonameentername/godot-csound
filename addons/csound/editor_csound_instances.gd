@tool
extends VBoxContainer
class_name EditorCsoundInstances

var editor_interface: EditorInterface
var undo_redo: EditorUndoRedoManager
var edited_path: String
var save_timer: Timer
var csound_hbox: HBoxContainer
var editor_csound_packed_scene: PackedScene
var file_label: Label
var file_dialog: EditorFileDialog
var new_layout: bool
var renaming_csound: bool
var drop_end: EditorCsoundDrop


func _init():
	renaming_csound = false

	file_dialog = EditorFileDialog.new()

	var extensions = ResourceLoader.get_recognized_extensions_for_type("CsoundLayout")
	for extension in extensions:
		file_dialog.add_filter("*.%s" % extension, "Csound Layout")

	add_child(file_dialog)
	file_dialog.connect("file_selected", _file_dialog_callback)

	CsoundServer.connect("csound_layout_changed", _update_csound)


func _ready():
	var layout = ProjectSettings.get_setting_with_override("audio/csound/default_csound_layout")
	if layout:
		edited_path = layout
	else:
		edited_path = "res://default_csound_layout.tres"
	editor_csound_packed_scene = preload("res://addons/csound/editor_csound_instance.tscn")

	save_timer = $SaveTimer
	csound_hbox = $CsoundScroll/CsoundHBox
	file_label = $TopBoxContainer/FileLabel

	_update_csound()


func _process(_delta):
	pass


func _notification(what):
	match what:
		NOTIFICATION_ENTER_TREE:
			_update_theme()

		NOTIFICATION_THEME_CHANGED:
			_update_theme()

		NOTIFICATION_DRAG_END:
			if drop_end:
				csound_hbox.remove_child(drop_end)
				drop_end.queue_free()
				drop_end = null

		NOTIFICATION_PROCESS:
			var edited = CsoundServer.get_edited()
			CsoundServer.set_edited(false)

			if edited:
				save_timer.start()


func _update_theme():
	var stylebox: StyleBoxEmpty = get_theme_stylebox("panel", "Tree")
	$CsoundScroll.add_theme_stylebox_override("panel", stylebox)

	for control in $CsoundScroll/CsoundHBox.get_children():
		control.editor_interface = editor_interface


func _add_csound():
	undo_redo.create_action("Add Csound")
	undo_redo.add_do_method(CsoundServer, "set_csound_count", CsoundServer.get_csound_count() + 1)
	undo_redo.add_undo_method(CsoundServer, "set_csound_count", CsoundServer.get_csound_count())
	undo_redo.add_do_method(self, "_update_csound")
	undo_redo.add_undo_method(self, "_update_csound")
	undo_redo.commit_action()


func _update_csound():
	if renaming_csound:
		return

	for i in range(csound_hbox.get_child_count(), 0, -1):
		i = i - 1
		var editor_csound_instance: Control = csound_hbox.get_child(i)
		if editor_csound_instance:
			csound_hbox.remove_child(editor_csound_instance)
			editor_csound_instance.queue_free()

	if drop_end:
		csound_hbox.remove_child(drop_end)
		drop_end.queue_free()
		drop_end = null

	for i in range(0, CsoundServer.get_csound_count()):
		var editor_csound_instance: EditorCsoundInstance = editor_csound_packed_scene.instantiate()
		editor_csound_instance.editor_interface = editor_interface
		var is_main: bool = i == 0
		editor_csound_instance.editor_csound_instances = self
		editor_csound_instance.is_main = is_main
		editor_csound_instance.undo_redo = undo_redo
		csound_hbox.add_child(editor_csound_instance)

		editor_csound_instance.connect(
			"delete_request", _delete_csound.bind(editor_csound_instance), CONNECT_DEFERRED
		)
		editor_csound_instance.connect("duplicate_request", _duplicate_csound, CONNECT_DEFERRED)
		editor_csound_instance.connect(
			"vol_reset_request", _reset_csound_volume.bind(editor_csound_instance), CONNECT_DEFERRED
		)
		editor_csound_instance.connect("drop_end_request", _request_drop_end, CONNECT_DEFERRED)
		editor_csound_instance.connect("dropped", _drop_at_index, CONNECT_DEFERRED)


func _update_csound_instance(index):
	if index >= csound_hbox.get_child_count():
		return

	csound_hbox.get_child(index).update_csound()


func _delete_csound(editor_csound_instance: EditorCsoundInstance):
	var index: int = editor_csound_instance.get_index()
	if index == 0:
		push_warning("Main Csound can't be deleted!")
		return

	undo_redo.create_action("Delete Csound")
	undo_redo.add_do_method(CsoundServer, "remove_csound", index)
	undo_redo.add_undo_method(CsoundServer, "add_csound", index)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_name", index, CsoundServer.get_csound_name(index)
	)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_volume_db", index, CsoundServer.get_csound_volume_db(index)
	)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_solo", index, CsoundServer.is_csound_solo(index)
	)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_mute", index, CsoundServer.is_csound_mute(index)
	)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_bypass", index, CsoundServer.is_csound_bypassing(index)
	)

	undo_redo.add_do_method(self, "_update_csound")
	undo_redo.add_undo_method(self, "_update_csound")
	undo_redo.commit_action()


func _duplicate_csound(which: int):
	var add_at_pos: int = which + 1
	undo_redo.create_action("Duplicate Csound")
	undo_redo.add_do_method(CsoundServer, "add_csound", add_at_pos)
	undo_redo.add_do_method(
		CsoundServer, "set_csound_name", add_at_pos, CsoundServer.get_csound_name(which) + " Copy"
	)
	undo_redo.add_do_method(
		CsoundServer, "set_csound_volume_db", add_at_pos, CsoundServer.get_csound_volume_db(which)
	)
	undo_redo.add_do_method(
		CsoundServer, "set_csound_solo", add_at_pos, CsoundServer.is_csound_solo(which)
	)
	undo_redo.add_do_method(
		CsoundServer, "set_csound_mute", add_at_pos, CsoundServer.is_csound_mute(which)
	)
	undo_redo.add_do_method(
		CsoundServer, "set_csound_bypass", add_at_pos, CsoundServer.is_csound_bypassing(which)
	)
	undo_redo.add_do_method(
		CsoundServer, "set_csound_script", add_at_pos, CsoundServer.get_csound_script(which)
	)
	for i in range(0, CsoundServer.get_csound_instrument_count(which)):
		undo_redo.add_do_method(
			CsoundServer,
			"add_csound_instrument",
			add_at_pos,
			CsoundServer.get_csound_instrument(which, i)
		)
	undo_redo.add_undo_method(CsoundServer, "remove_csound", add_at_pos)
	undo_redo.add_do_method(self, "_update_csound")
	undo_redo.add_undo_method(self, "_update_csound")
	undo_redo.commit_action()


func _reset_csound_volume(editor_csound_instance: EditorCsoundInstance):
	var index: int = editor_csound_instance.get_index()
	undo_redo.create_action("Reset Csound Volume")
	undo_redo.add_do_method(CsoundServer, "set_csound_volume_db", index, 0)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_volume_db", index, CsoundServer.get_csound_volume_db(index)
	)
	undo_redo.add_do_method(self, "_update_csound")
	undo_redo.add_undo_method(self, "_update_csound")
	undo_redo.commit_action()


func _request_drop_end():
	if not drop_end and csound_hbox.get_child_count():
		drop_end = EditorCsoundDrop.new()
		csound_hbox.add_child(drop_end)
		drop_end.custom_minimum_size = csound_hbox.get_child(0).size
		drop_end.connect("dropped", _drop_at_index, CONNECT_DEFERRED)


func _drop_at_index(csound, index):
	undo_redo.create_action("Move Csound")

	undo_redo.add_do_method(CsoundServer, "move_csound", csound, index)
	var real_csound: int = csound if index > csound else csound + 1
	var real_index: int = index - 1 if index > csound else index
	undo_redo.add_undo_method(CsoundServer, "move_csound", real_index, real_csound)

	undo_redo.add_do_method(self, "_update_csound")
	undo_redo.add_undo_method(self, "_update_csound")
	undo_redo.commit_action()


func _server_save():
	var status = CsoundServer.generate_csound_layout()
	ResourceSaver.save(status, edited_path)


func _save_as_layout():
	file_dialog.file_mode = EditorFileDialog.FILE_MODE_SAVE_FILE
	file_dialog.title = "Save Csound Layout As..."
	file_dialog.current_path = edited_path
	file_dialog.popup_centered_clamped(Vector2(1050, 700) * scale, 0.8)
	new_layout = false


func _new_layout():
	file_dialog.file_mode = EditorFileDialog.FILE_MODE_SAVE_FILE
	file_dialog.title = "Location for New Layout..."
	file_dialog.current_path = edited_path
	file_dialog.popup_centered_clamped(Vector2(1050, 700) * scale, 0.8)
	new_layout = false


func _select_layout():
	if editor_interface:
		editor_interface.get_file_system_dock().navigate_to_path(edited_path)


func _load_layout():
	file_dialog.file_mode = EditorFileDialog.FILE_MODE_OPEN_FILE
	file_dialog.title = "Open Csound Layout"
	file_dialog.current_path = edited_path
	file_dialog.popup_centered_clamped(Vector2(1050, 700) * scale, 0.8)
	new_layout = false


func _load_default_layout():
	var layout_path = ProjectSettings.get_setting_with_override(
		"audio/csound/default_csound_layout"
	)
	var state = ResourceLoader.load(layout_path, "", ResourceLoader.CACHE_MODE_IGNORE)
	if not state:
		push_warning("There is no '%s' file." % layout_path)
		return

	edited_path = layout_path
	file_label.text = "Layout: %s" % edited_path.get_file()
	CsoundServer.set_csound_layout(state)
	_update_csound()
	call_deferred("_select_layout")


func _file_dialog_callback(filename: String):
	if file_dialog.file_mode == EditorFileDialog.FILE_MODE_OPEN_FILE:
		var state = ResourceLoader.load(filename, "", ResourceLoader.CACHE_MODE_IGNORE)
		if not state:
			push_warning("Invalid file, not a csound layout.")
			return

		edited_path = filename
		file_label.text = "Layout: %s" % edited_path.get_file()
		CsoundServer.set_csound_layout(state)
		_update_csound()
		call_deferred("_select_layout")

	elif file_dialog.file_mode == EditorFileDialog.FILE_MODE_SAVE_FILE:
		if new_layout:
			var csound_layout: CsoundLayout = CsoundLayout.new()
			CsoundServer.set_csound_layout(csound_layout)

		var error = ResourceSaver.save(CsoundServer.generate_csound_layout(), filename)
		if error != OK:
			push_warning("Error saving file: %s" % filename)
			return

		edited_path = filename
		file_label.text = "Layout: %s" % edited_path.get_file()
		_update_csound()
		call_deferred("_select_layout")


func _set_renaming_csound(renaming: bool):
	renaming_csound = renaming


func resource_saved(resource: Resource):
	for editor_csound_instance in csound_hbox.get_children():
		if editor_csound_instance is EditorCsoundInstance:
			editor_csound_instance.resource_saved(resource)
