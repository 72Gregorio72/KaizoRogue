City_Hub_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, City_Hub_OnFrame
	.byte 0

City_Hub_OnFrame:
	map_script_2 VAR_TEMP_0, 0, Kaizo_Hub_EventScript_CheckRunRecap
	.2byte 0
