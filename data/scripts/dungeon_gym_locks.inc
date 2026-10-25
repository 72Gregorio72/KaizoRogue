EventScript_GymExitBlocked::
	lockall
	textcolor NPC_TEXT_COLOR_NEUTRAL
	msgbox gText_CantLeaveGymYet, MSGBOX_DEFAULT
	closemessage
	special Script_StepPlayerAwayFromExit
	waitmovement 0
	releaseall
	end

EventScript_DungeonExitBlocked::
	lockall
	textcolor NPC_TEXT_COLOR_NEUTRAL
	msgbox gText_CantLeaveDungeonYet, MSGBOX_DEFAULT
	closemessage
	special Script_StepPlayerAwayFromExit
	waitmovement 0
	releaseall
	end

gText_CantLeaveGymYet::
	.string "Non puoi andartene finché non hai\n"
	.string "sconfitto il Capopalestra!$"

gText_CantLeaveDungeonYet::
	.string "Non puoi tornare indietro finché non\n"
	.string "hai completato il dungeon!$"
