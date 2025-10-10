extends SceneTree

var main_csound: CsoundInstance
var csound: CsoundInstance


func _init():
	var main_scene = preload("res://main.tscn")
	var main = main_scene.instantiate()

	CsoundServer.connect("csound_layout_changed", csound_layout_changed)

	change_scene_to_packed(main_scene)

	await create_timer(2.0).timeout

	var result = csound.evaluate_code('return 2 + 2')

	print("output result is ", result)


func csound_layout_changed():
	main_csound = CsoundServer.get_csound("Main")
	csound = CsoundServer.get_csound("Csound")
