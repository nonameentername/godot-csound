@tool
extends Control
class_name EditorCsoundDrop

var hovering_drop: bool


func _init():
	add_user_signal("dropped")
	hovering_drop = false


func _notification(what):
	match what:
		NOTIFICATION_DRAW:
			draw_style_box(get_theme_stylebox("normal", "Button"), Rect2(Vector2(), size))
			if hovering_drop:
				var accent: Color = get_theme_color("accent_color", "Editor")
				accent.v *= 0.7
				draw_rect(Rect2(Vector2(), size), accent, false)

		NOTIFICATION_MOUSE_ENTER:
			if not hovering_drop:
				hovering_drop = true
				queue_redraw()

		NOTIFICATION_MOUSE_EXIT:
			if hovering_drop:
				hovering_drop = false
				queue_redraw()

		NOTIFICATION_DRAG_END:
			if hovering_drop:
				hovering_drop = false
				queue_redraw()


func _can_drop_data(_at_position, data):
	return data.has("type") and data["type"] == "move_csound"


func _drop_data(_at_position, data):
	emit_signal("dropped", data["index"], CsoundServer.get_csound_count())
