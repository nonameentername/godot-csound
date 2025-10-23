extends Node2D

var csound: CsoundInstance

@export var midi_file: Resource


func _ready():
	print ("godot-csound version: ", CsoundServer.get_version(), " build: ", CsoundServer.get_build())
	CsoundServer.connect("csound_layout_changed", csound_layout_changed)


func csound_layout_changed():
	csound = CsoundServer.get_csound("Main")
	csound.send_control_channel("cutoff", 1)


func _process(_delta):
	pass


func _on_check_button_toggled(toggled_on: bool):
	if toggled_on:
		csound.note_on(0, 60, 90)
	else:
		csound.note_off(0, 60)


func _on_check_button_2_toggled(toggled_on: bool):
	if toggled_on:
		csound.note_on(1, 64, 90)
	else:
		csound.note_off(1, 64)


func _on_check_button_3_toggled(toggled_on: bool):
	if toggled_on:
		csound.note_on(2, 67, 90)
	else:
		csound.note_off(2, 67)


func _on_v_slider_volume_value_changed(value: float):
	csound.send_control_channel("cutoff", value)


func _on_v_slider_tempo_value_changed(value: float):
	csound.event_string('i "update_tempo" 0 -1 %d' % value)


func _on_button_pressed():
	csound.play_midi(midi_file)
