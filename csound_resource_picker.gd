extends EditorResourcePicker
class_name CsoundResourcePicker

var MENU_MAKE_UNIQUE = 4

var label: LineEdit
var saved_menu_node: PopupMenu


func _init(file_label: LineEdit):
	label = file_label
	connect("resource_changed", _resource_changed)


func _set_create_options(menu_node):
	if menu_node and not menu_node.is_connected("id_pressed", _edit_menu_pressed):
		menu_node.connect("id_pressed", _edit_menu_pressed)
	saved_menu_node = menu_node


func _edit_menu_pressed(p_which):
	if p_which == MENU_MAKE_UNIQUE:
		label.text = "<new_file>"


func _resource_changed(resource: Resource):
	if resource:
		label.text = resource.resource_path.get_file()
	else:
		label.text = "<empty>"


func resource_saved(resource: Resource):
	if edited_resource == resource:
		label.text = edited_resource.resource_path.get_file()
