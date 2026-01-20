extends Node

func _ready() -> void:
	get_viewport().debug_draw = Viewport.DebugDraw.DEBUG_DRAW_WIREFRAME
	var app := VoxelGDCPP.new()
	app.initialize()
