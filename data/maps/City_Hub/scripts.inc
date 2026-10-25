City_Hub_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, City_Hub_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, City_Hub_OnFrame
	.byte 0

City_Hub_OnTransition:
	special Script_UpdateHubRoamingMons
	end

City_Hub_OnFrame:
	map_script_2 VAR_TEMP_0, 0, Kaizo_Hub_EventScript_CheckRunRecap
	.2byte 0

@ --- ROAMING POKÉMON INTERACTION SCRIPT ---
City_Hub_EventScript_ForestKeeper::
City_Hub_EventScript_LakeKeeper::
City_Hub_EventScript_ForestMon0::
City_Hub_EventScript_ForestMon1::
City_Hub_EventScript_ForestMon2::
City_Hub_EventScript_ForestMon3::
City_Hub_EventScript_ForestMon4::
City_Hub_EventScript_LakeMon0::
City_Hub_EventScript_LakeMon1::
City_Hub_EventScript_LakeMon2::
City_Hub_EventScript_LakeMon3::
City_Hub_EventScript_LakeMon4::
City_Hub_EventScript_MonInteract::
	lock
	faceplayer
	special Script_HubMon_Interact
	goto_if_eq VAR_RESULT, SPECIES_NONE, City_Hub_EventScript_MonInteract_End
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	waitmoncry
	msgbox City_Hub_Text_MonHappy, MSGBOX_DEFAULT
City_Hub_EventScript_MonInteract_End::
	release
	end

@ --- SURF SCRIPT ---
City_Hub_EventScript_UseSurf::
	lockall
	checkitem ITEM_HM03, 1
	goto_if_eq VAR_RESULT, TRUE, City_Hub_EventScript_DoUseSurf
	giveitem ITEM_HM03, 1
City_Hub_EventScript_DoUseSurf::
	msgbox City_Hub_Text_WantToSurf, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, City_Hub_EventScript_ReleaseUseSurf
	setfieldeffectargument 0, 0
	dofieldeffect FLDEFF_USE_SURF
City_Hub_EventScript_ReleaseUseSurf::
	releaseall
	end

@ --- TEXT STRINGS ---
City_Hub_Text_WantToSurf:
	.string "L'acqua è limpida e calma…\n"
	.string "Vuoi fare surf?$"

City_Hub_Text_MonHappy:
	.string "{STR_VAR_1} sembra felice di\n"
	.string "passeggiare qui nell'Hub!$"
