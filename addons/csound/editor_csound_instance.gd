@tool
extends PanelContainer
class_name EditorCsoundInstance

var editor_interface: EditorInterface
var dark_theme = false
var audio_meter: EditorAudioMeterNotchesCsound
var audio_meter2: EditorAudioMeterNotchesCsound
var audio_value_preview_box: Panel
var audio_value_preview_label: Label
var solo: Button
var mute: Button
var bypass: Button
var options: MenuButton
var tree: Tree
var add_intrument: TreeItem
var slider: VSlider
var preview_timer: Timer
var initialized = false
var updating_csound: bool
var csound_name: LineEdit
var is_main: bool
var enabled_vu: Texture2D
var disabled_vu: Texture2D
var active_bus_texture: Texture2D
var hovering_drop: bool
var editor_csound_instances: EditorCsoundInstances
var undo_redo: EditorUndoRedoManager
var channel_container: HBoxContainer
var named_channel_container: HBoxContainer
var tab_container: TabContainer
var csound_file_label: LineEdit
var csound_file_hbox: HBoxContainer
var resource_picker: CsoundResourcePicker
var delete_instrument_popup: PopupMenu


class Channel:
	var prev_active: bool
	var peak: float = 0
	var vu: TextureProgressBar


var channels: Array[Channel]
var named_channels: Array[Channel]


func _init():
	add_user_signal("duplicate_request")
	add_user_signal("delete_request")
	add_user_signal("vol_reset_request")
	add_user_signal("drop_end_request")
	add_user_signal("dropped")

	updating_csound = false
	hovering_drop = false


func _ready():
	csound_name = $VBoxContainer/LineEdit
	tab_container = $VBoxContainer/HBoxContainer2/TabContainer
	channel_container = $VBoxContainer/HBoxContainer2/TabContainer/Channels
	named_channel_container = $VBoxContainer/HBoxContainer2/TabContainer/Named
	slider = $VBoxContainer/HBoxContainer2/TabContainer/Channels/Slider
	audio_value_preview_box = $VBoxContainer/HBoxContainer2/TabContainer/Channels/Slider/AudioValuePreview
	audio_value_preview_label = $VBoxContainer/HBoxContainer2/TabContainer/Channels/Slider/AudioValuePreview/AudioPreviewHBox/AudioPreviewLabel
	solo = $VBoxContainer/HBoxContainer/LeftHBox/solo
	mute = $VBoxContainer/HBoxContainer/LeftHBox/mute
	bypass = $VBoxContainer/HBoxContainer/LeftHBox/bypass
	options = $VBoxContainer/HBoxContainer/RightHBox/options
	tree = $VBoxContainer/Tree
	csound_file_label = $VBoxContainer/HBoxContainer3/LeftHBox/Label
	csound_file_hbox = $VBoxContainer/HBoxContainer3/RightHBox
	preview_timer = $Timer

	initialized = true

	focus_mode = Control.FOCUS_CLICK

	options.shortcut_context = self
	var popup = options.get_popup()

	popup.clear()

	var input_event: InputEventKey = InputEventKey.new()
	input_event.ctrl_pressed = true
	input_event.command_or_control_autoremap = true
	input_event.keycode = KEY_D
	var shortcut: Shortcut = Shortcut.new()
	shortcut.events = [input_event]
	shortcut.resource_name = "Duplicate Csound"

	popup.add_shortcut(shortcut, 1)

	input_event = InputEventKey.new()
	input_event.keycode = KEY_DELETE
	shortcut = Shortcut.new()
	shortcut.events = [input_event]
	shortcut.resource_name = "Delete Csound"
	popup.add_shortcut(shortcut)
	popup.set_item_disabled(1, is_main)

	shortcut = Shortcut.new()
	shortcut.resource_name = "Reset volume"
	popup.add_shortcut(shortcut)

	popup.connect("index_pressed", _csound_popup_pressed)

	tree.hide_root = true
	tree.hide_folding = true
	tree.allow_rmb_select = true
	tree.focus_mode = FOCUS_CLICK
	tree.allow_reselect = true

	resource_picker = CsoundResourcePicker.new(csound_file_label)
	resource_picker.base_type = "CsoundFileReader"
	resource_picker.resource_selected.connect(_resource_selected)
	resource_picker.resource_changed.connect(_resource_changed)

	csound_file_hbox.add_child(resource_picker)

	shortcut = Shortcut.new()
	shortcut.resource_name = "Delete Instrument"
	delete_instrument_popup = PopupMenu.new()
	delete_instrument_popup.add_shortcut(shortcut)
	delete_instrument_popup.connect("index_pressed", _csound_delete_instrument)
	add_child(delete_instrument_popup)

	update_csound()
	_update_theme()


func _create_channel_progress_bar():
	var channel_progress_bar: TextureProgressBar = TextureProgressBar.new()
	channel_progress_bar.fill_mode = TextureProgressBar.FILL_BOTTOM_TO_TOP
	channel_progress_bar.min_value = -80
	channel_progress_bar.max_value = 24
	channel_progress_bar.step = 0.1

	channel_progress_bar.texture_under = enabled_vu
	channel_progress_bar.tint_under = Color(0.75, 0.75, 0.75)
	channel_progress_bar.texture_progress = enabled_vu

	return channel_progress_bar


func _create_channel():
	var channel: Channel = Channel.new()
	channel.peak = -200
	channel.prev_active = false

	return channel


func _update_visiable_channels():
	for channel in channel_container.get_children():
		if is_instance_of(channel, TextureProgressBar):
			channel_container.remove_child(channel)

	channels.clear()
	channels.resize(CsoundServer.get_csound_channel_count(get_index()))

	for i in range(0, CsoundServer.get_csound_channel_count(get_index())):
		var channel: Channel = _create_channel()
		var channel_progress_bar: TextureProgressBar = _create_channel_progress_bar()
		channel.vu = channel_progress_bar
		channels[i] = channel
		channel_container.add_child(channel_progress_bar)

	channel_container.move_child(audio_meter, channel_container.get_child_count())

	for channel in named_channel_container.get_children():
		if is_instance_of(channel, TextureProgressBar):
			named_channel_container.remove_child(channel)

	named_channels.clear()
	named_channels.resize(CsoundServer.get_csound_named_channel_count(get_index()))

	for i in range(0, CsoundServer.get_csound_named_channel_count(get_index())):
		var channel: Channel = _create_channel()
		var channel_progress_bar: TextureProgressBar = _create_channel_progress_bar()
		channel_progress_bar.tooltip_text = CsoundServer.get_csound_named_channel_name(
			get_index(), i
		)
		channel.vu = channel_progress_bar
		named_channels[i] = channel
		named_channel_container.add_child(channel_progress_bar)

	named_channel_container.move_child(audio_meter2, named_channel_container.get_child_count())


func _process(_delta):
	if (
		channels.size() != CsoundServer.get_csound_channel_count(get_index())
		or named_channels.size() != CsoundServer.get_csound_named_channel_count(get_index())
	):
		_update_visiable_channels()

	for i in range(0, channels.size()):
		var real_peak = -100

		if CsoundServer.is_csound_channel_active(get_index(), i):
			real_peak = max(
				real_peak, CsoundServer.get_csound_channel_peak_volume_db(get_index(), i)
			)

		_update_channel_vu(channels[i], real_peak)

	for i in range(0, named_channels.size()):
		var real_peak = -100

		if CsoundServer.is_csound_named_channel_active(get_index(), i):
			real_peak = max(
				real_peak, CsoundServer.get_csound_named_channel_peak_volume_db(get_index(), i)
			)

		_update_channel_vu(named_channels[i], real_peak)


func _update_channel_vu(channel, real_peak):
	if real_peak > channel.peak:
		channel.peak = real_peak
	else:
		channel.peak -= get_process_delta_time() * 60.0
	channel.vu.value = channel.peak

	if _scaled_db_to_normalized_volume(channel.peak) > 0:
		channel.vu.texture_over = null
	else:
		channel.vu.texture_over = disabled_vu


func _notification(what):
	match what:
		NOTIFICATION_THEME_CHANGED:
			if not initialized:
				return
			_update_theme()

		NOTIFICATION_DRAW:
			if is_main:
				draw_style_box(get_theme_stylebox("disabled", "Button"), Rect2(Vector2(), size))
			elif has_focus():
				draw_style_box(get_theme_stylebox("focus", "Button"), Rect2(Vector2(), size))
			else:
				draw_style_box(get_theme_stylebox("panel", "TabContainer"), Rect2(Vector2(), size))

			if get_index() != 0 && hovering_drop:
				var accent: Color = get_theme_color("accent_color", "Editor")
				accent.a *= 0.7
				draw_rect(Rect2(Vector2(), size), accent, false)

		NOTIFICATION_PROCESS:
			pass

		NOTIFICATION_MOUSE_EXIT:
			if hovering_drop:
				hovering_drop = false
				queue_redraw()

		NOTIFICATION_DRAG_END:
			if hovering_drop:
				hovering_drop = false
				queue_redraw()


func _hide_value_preview():
	audio_value_preview_box.hide()


func _show_value(value):
	var db: float
	if Input.is_key_pressed(KEY_CTRL):
		db = round(_normalized_volume_to_scaled_db(value))
	else:
		db = _normalized_volume_to_scaled_db(value)

	var text

	if is_zero_approx(snapped(db, 0.1)):
		text = " 0.0 dB"
	else:
		text = "%.1f dB" % db

	slider.tooltip_text = text
	audio_value_preview_label.text = text
	var slider_size: Vector2 = slider.size
	var slider_position: Vector2 = slider.global_position
	var vert_padding: float = 10.0
	var box_position: Vector2 = Vector2(
		slider_size.x, (slider_size.y - vert_padding) * (1.0 - slider.value) - vert_padding
	)
	audio_value_preview_box.position = slider_position + box_position
	audio_value_preview_box.size = audio_value_preview_label.size

	if slider.has_focus() and !audio_value_preview_box.visible:
		audio_value_preview_box.show()
	preview_timer.start()


func _tab_changed(tab: int):
	if updating_csound:
		return

	CsoundServer.set_csound_tab(get_index(), tab)
	_update_slider()


func _value_changed(normalized):
	if updating_csound:
		return

	updating_csound = true

	var db: float = _normalized_volume_to_scaled_db(normalized)
	if Input.is_key_pressed(KEY_CTRL):
		slider.value = _scaled_db_to_normalized_volume(round(db))

	undo_redo.create_action("Change Csound Volume", UndoRedo.MERGE_ENDS)
	undo_redo.add_do_method(CsoundServer, "set_csound_volume_db", get_index(), db)
	undo_redo.add_undo_method(
		CsoundServer,
		"set_csound_volume_db",
		get_index(),
		CsoundServer.get_csound_volume_db(get_index())
	)
	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.commit_action()

	updating_csound = false


func _normalized_volume_to_scaled_db(normalized):
	if normalized > 0.6:
		return 22.22 * normalized - 16.2
	elif normalized < 0.05:
		return 830.72 * normalized - 80.0
	else:
		return 45.0 * pow(normalized - 1.0, 3)


func _scaled_db_to_normalized_volume(db):
	if db > -2.88:
		return (db + 16.2) / 22.22
	elif db < -38.602:
		return (db + 80.0) / 830.72
	else:
		if db < 0.0:
			var positive_x: float = pow(abs(db) / 45.0, 1.0 / 3.0) + 1.0
			var translation: Vector2 = Vector2(1.0, 0.0) - Vector2(positive_x, abs(db))
			var reflected_position: Vector2 = Vector2(1.0, 0.0) + translation
			return reflected_position.x
		else:
			return pow(db / 45.0, 1.0 / 3.0) + 1.0


func _update_theme():
	if not editor_interface:
		return

	var base_color: Color = EditorInterface.get_editor_settings().get_setting(
		"interface/theme/base_color"
	)

	dark_theme = base_color.v < .5

	var solo_color = Color(1.0, 0.89, 0.22) if dark_theme else Color(1.0, 0.92, 0.44)
	solo.add_theme_color_override("icon_pressed_color", solo_color)
	solo.icon = get_theme_icon("AudioBusSolo", "EditorIcons")

	var mute_color = Color(1.0, 0.16, 0.16) if dark_theme else Color(1.0, 0.44, 0.44)
	mute.add_theme_color_override("icon_pressed_color", mute_color)
	mute.icon = get_theme_icon("AudioBusMute", "EditorIcons")

	var bypass_color = Color(0.13, 0.8, 1.0) if dark_theme else Color(0.44, 0.87, 1.0)
	bypass.add_theme_color_override("icon_pressed_color", bypass_color)
	bypass.icon = get_theme_icon("AudioBusBypass", "EditorIcons")

	options.icon = get_theme_icon("GuiTabMenuHl", "EditorIcons")

	audio_value_preview_label.add_theme_color_override(
		"font_color", get_theme_color("font_color", "TooltipLabel")
	)
	audio_value_preview_label.add_theme_color_override(
		"font_shadow_color", get_theme_color("font_shadow_color", "TooltipLabel")
	)
	audio_value_preview_box.add_theme_stylebox_override(
		"panel", get_theme_stylebox("panel", "TooltipPanel")
	)

	tree.custom_minimum_size = Vector2(0, 80) * EditorInterface.get_editor_scale()
	tree.clear()

	var root: TreeItem = tree.create_item()

	tree.button_clicked.connect(_tree_button_clicked)
	tree.custom_item_clicked.connect(_tree_custom_item_clicked)

	for i in range(0, CsoundServer.get_csound_instrument_count(get_index())):
		var instrument = CsoundServer.get_csound_instrument(get_index(), i)
		var tree_item: TreeItem = tree.create_item(root)
		tree_item.set_cell_mode(0, TreeItem.CELL_MODE_CUSTOM)
		tree_item.set_text(0, instrument.get_name())
		tree_item.set_editable(0, true)
		tree_item.set_selectable(0, true)
		tree_item.set_metadata(0, i)
		#var remove_button = get_theme_icon("Edit", "EditorIcons")
		#tree_item.add_button(0, remove_button)

	add_intrument = tree.create_item(root)
	add_intrument.set_cell_mode(0, TreeItem.CELL_MODE_CUSTOM)
	add_intrument.set_editable(0, false)
	add_intrument.set_selectable(0, false)
	add_intrument.set_text(0, "Add Instrument")
	add_intrument.visible = false

	var add_button = get_theme_icon("Add", "EditorIcons")
	add_intrument.add_button(0, add_button)

	enabled_vu = get_theme_icon("BusVuActive", "EditorIcons")
	disabled_vu = get_theme_icon("BusVuFrozen", "EditorIcons")

	if not is_instance_valid(audio_meter):
		audio_meter = EditorAudioMeterNotchesCsound.new()
		channel_container.add_child(audio_meter)

	if not is_instance_valid(audio_meter2):
		audio_meter2 = EditorAudioMeterNotchesCsound.new()
		named_channel_container.add_child(audio_meter2)


func _tree_button_clicked(item: TreeItem, _column: int, _id: int, _mouse_button_index: int):
	if item == add_intrument:
		print("adding an instrument")

		updating_csound = true

		var instrument = CsoundInstrument.new()
		instrument.set_name("NewInstrument")
		undo_redo.create_action("Add Csound Instrument")
		undo_redo.add_do_method(CsoundServer, "add_csound_instrument", get_index(), instrument)
		undo_redo.add_undo_method(
			CsoundServer,
			"remove_csound_instrument",
			get_index(),
			CsoundServer.get_csound_instrument_count(get_index())
		)
		undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
		undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
		undo_redo.commit_action()

		updating_csound = false
	else:
		print("gone clicking")


func _tree_custom_item_clicked(mouse_button_index: int):
	if mouse_button_index == MOUSE_BUTTON_LEFT:
		var selected: TreeItem = tree.get_selected()
		var index = selected.get_metadata(0)
		var instrument: CsoundInstrument = CsoundServer.get_csound_instrument(get_index(), index)
		if not instrument.changed.is_connected(_csound_instrument_changed):
			instrument.changed.connect(_csound_instrument_changed)
		EditorInterface.edit_resource(instrument)
	elif mouse_button_index == MOUSE_BUTTON_RIGHT:
		delete_instrument_popup.set_position(get_screen_position() + get_local_mouse_position())
		delete_instrument_popup.reset_size()
		delete_instrument_popup.popup()
	print("adding an instrument", mouse_button_index, MOUSE_BUTTON_RIGHT)


func _csound_instrument_changed():
	print("resource_saved")
	var selected: TreeItem = tree.get_selected()
	var index = selected.get_metadata(0)
	var instrument: CsoundInstrument = CsoundServer.get_csound_instrument(get_index(), index)
	CsoundServer.set_edited(true)
	selected.set_text(0, instrument.get_name())


func _csound_delete_instrument(_option: int):
	print("deleting an instrument")
	updating_csound = true

	var selected: TreeItem = tree.get_selected()
	var index = selected.get_metadata(0)

	undo_redo.create_action("Remove Csound Instrument")
	var instrument = CsoundServer.get_csound_instrument(get_index(), index)
	undo_redo.add_do_method(CsoundServer, "remove_csound_instrument", get_index(), index)
	undo_redo.add_undo_method(CsoundServer, "add_csound_instrument", get_index(), instrument)
	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.commit_action()

	updating_csound = false


func _get_drag_data(at_position):
	if get_index() == 0:
		return

	var control: Control = Control.new()
	var panel: Panel = Panel.new()
	control.add_child(panel)
	panel.modulate = Color(1, 1, 1, 0.7)
	panel.add_theme_stylebox_override("panel", get_theme_stylebox("focus", "Button"))
	panel.set_size(get_size())
	panel.set_position(-at_position)
	set_drag_preview(control)
	var dictionary: Dictionary = {"type": "move_csound", "index": get_index()}

	if get_index() < CsoundServer.get_csound_count() - 1:
		emit_signal("drop_end_request")

	return dictionary


func _can_drop_data(_at_position, data):
	if get_index() == 0:
		return false

	if data.has("type") and data["type"] == "move_csound" and int(data["index"]) != get_index():
		hovering_drop = true
		return true

	return false


func _drop_data(_at_position, data):
	emit_signal("dropped", data["index"], get_index())


func _csound_popup_pressed(option: int):
	#if not has_focus():
	#	return
	match option:
		2:
			emit_signal("vol_reset_request")
		1:
			emit_signal("delete_request")
		0:
			emit_signal("duplicate_request", get_index())


func update_csound():
	if updating_csound:
		return

	updating_csound = true

	var index: int = get_index()

	var db_value: float = CsoundServer.get_csound_volume_db(index)
	slider.value = _scaled_db_to_normalized_volume(db_value)
	csound_name.text = CsoundServer.get_csound_name(index)

	if is_main:
		csound_name.editable = false

	solo.button_pressed = CsoundServer.is_csound_solo(index)
	mute.button_pressed = CsoundServer.is_csound_mute(index)
	bypass.button_pressed = CsoundServer.is_csound_bypassing(index)

	tab_container.current_tab = CsoundServer.get_csound_tab(index)
	var file = CsoundServer.get_csound_script(index)
	resource_picker.edited_resource = file

	if file:
		csound_file_label.text = file.resource_path.get_file()
	else:
		csound_file_label.text = "<empty>"

	_update_slider()

	updating_csound = false


func _update_slider():
	match CsoundServer.get_csound_tab(get_index()):
		0:
			if not slider in channel_container.get_children():
				named_channel_container.remove_child(slider)
				channel_container.add_child(slider)
				channel_container.move_child(slider, 0)
		1:
			if not slider in named_channel_container.get_children():
				channel_container.remove_child(slider)
				named_channel_container.add_child(slider)
				named_channel_container.move_child(slider, 0)


func _solo_toggled():
	updating_csound = true

	undo_redo.create_action("Toggle Csound Solo")
	undo_redo.add_do_method(CsoundServer, "set_csound_solo", get_index(), solo.is_pressed())
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_solo", get_index(), CsoundServer.is_csound_solo(get_index())
	)
	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.commit_action()

	updating_csound = false


func _mute_toggled():
	updating_csound = true

	undo_redo.create_action("Toggle Csound mute")
	undo_redo.add_do_method(CsoundServer, "set_csound_mute", get_index(), mute.is_pressed())
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_mute", get_index(), CsoundServer.is_csound_mute(get_index())
	)
	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.commit_action()

	updating_csound = false


func _bypass_toggled():
	updating_csound = true

	undo_redo.create_action("Toggle Csound bypass")
	undo_redo.add_do_method(CsoundServer, "set_csound_bypass", get_index(), bypass.is_pressed())
	undo_redo.add_undo_method(
		CsoundServer,
		"set_csound_bypass",
		get_index(),
		CsoundServer.is_csound_bypassing(get_index())
	)
	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.commit_action()

	updating_csound = false


func _name_changed(new_name: String):
	if updating_csound:
		return

	updating_csound = true
	#csound_name.release_focus()

	if new_name == CsoundServer.get_csound_name(get_index()):
		updating_csound = false
		return new_name

	var attempt: String = new_name
	var attempts: int = 1

	while true:
		var name_free: bool = true
		for i in range(0, CsoundServer.get_csound_count()):
			if CsoundServer.get_csound_name(i) == attempt:
				name_free = false
				break

		if name_free:
			break

		attempts += 1
		attempt = new_name + " " + str(attempts)

	var current: String = CsoundServer.get_csound_name(get_index())

	undo_redo.create_action("Rename Csound")
	undo_redo.add_do_method(editor_csound_instances, "_set_renaming_csound", true)
	undo_redo.add_undo_method(editor_csound_instances, "_set_renaming_csound", true)

	undo_redo.add_do_method(CsoundServer, "set_csound_name", get_index(), attempt)
	undo_redo.add_undo_method(CsoundServer, "set_csound_name", get_index(), current)

	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())

	undo_redo.add_do_method(editor_csound_instances, "_set_renaming_csound", false)
	undo_redo.add_undo_method(editor_csound_instances, "_set_renaming_csound", false)
	undo_redo.commit_action()

	updating_csound = false

	return attempt


func _resource_changed(resource: Resource):
	updating_csound = true

	undo_redo.create_action("Set Csound script")
	undo_redo.add_do_method(CsoundServer, "set_csound_script", get_index(), resource)
	undo_redo.add_undo_method(
		CsoundServer, "set_csound_script", get_index(), CsoundServer.get_csound_script(get_index())
	)
	undo_redo.add_do_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.add_undo_method(editor_csound_instances, "_update_csound_instance", get_index())
	undo_redo.commit_action()

	updating_csound = false


func _name_focus_exit():
	csound_name.text = _name_changed(csound_name.get_text())


func resource_saved(resource: Resource):
	resource_picker.resource_saved(resource)


func _resource_selected(resource: Resource, _inspect: bool):
	EditorInterface.edit_resource(resource)
