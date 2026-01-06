extends Node

func _ready() -> void:
	var app := VoxelGDCPP.new()
	app.initialize()
	app.test()
