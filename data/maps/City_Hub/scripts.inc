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

@ --- FOREST KEEPER NPC ---
City_Hub_EventScript_ForestKeeper::
	lock
	faceplayer
	msgbox City_Hub_Text_ForestKeeper_Intro, MSGBOX_DEFAULT
	setvar VAR_0x8008, 0
	goto City_Hub_EventScript_RoamingKeeperLoop

@ --- LAKE KEEPER NPC ---
City_Hub_EventScript_LakeKeeper::
	lock
	faceplayer
	msgbox City_Hub_Text_LakeKeeper_Intro, MSGBOX_DEFAULT
	setvar VAR_0x8008, 1
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_RoamingKeeperLoop::
	special Script_HubRoaming_PushSlotMenu
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, City_Hub_EventScript_RoamingKeeperExit
	goto_if_eq VAR_RESULT, 6, City_Hub_EventScript_RoamingKeeperExit
	goto_if_eq VAR_RESULT, 5, City_Hub_EventScript_RoamingClearAll
	copyvar VAR_0x8005, VAR_RESULT
	setvar VAR_0x8006, 4
	goto_if_eq VAR_0x8008, 0, City_Hub_CalcLocalIdDone
	setvar VAR_0x8006, 9
City_Hub_CalcLocalIdDone::
	addvar VAR_0x8006, VAR_0x8005
	special Script_HubRoaming_CheckSlotOccupied
	goto_if_eq VAR_RESULT, 1, City_Hub_EventScript_SlotAlreadyOccupied
	goto City_Hub_EventScript_PickBoxMon

City_Hub_EventScript_SlotAlreadyOccupied::
	msgbox City_Hub_Text_SlotOccupied, MSGBOX_DEFAULT
	dynmultipush City_Hub_Text_OptChange, 0
	dynmultipush City_Hub_Text_OptRemove, 1
	dynmultipush City_Hub_Text_OptCancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, City_Hub_EventScript_RoamingKeeperLoop
	goto_if_eq VAR_RESULT, 2, City_Hub_EventScript_RoamingKeeperLoop
	goto_if_eq VAR_RESULT, 1, City_Hub_EventScript_SlotRemoveSingle
	goto City_Hub_EventScript_PickBoxMon

City_Hub_EventScript_SlotRemoveSingle::
	special Script_HubRoaming_ClearSlot
	removeobject VAR_0x8006
	msgbox City_Hub_Text_SlotCleared, MSGBOX_DEFAULT
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_PickBoxMon::
	msgbox City_Hub_Text_ChooseMonPrompt, MSGBOX_DEFAULT
	chooseboxmon
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, City_Hub_EventScript_RoamingKeeperLoop
	special Script_HubRoaming_ValidateAndSet
	goto_if_eq VAR_RESULT, 0, City_Hub_EventScript_RoamingKeeperLoop
	goto_if_eq VAR_RESULT, 2, City_Hub_EventScript_EggError
	goto_if_eq VAR_RESULT, 3, City_Hub_EventScript_ForestWaterError
	goto_if_eq VAR_RESULT, 4, City_Hub_EventScript_LakeNotWaterError
	removeobject VAR_0x8006
	addobject VAR_0x8006
	msgbox City_Hub_Text_MonAddedSuccess, MSGBOX_DEFAULT
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_EggError::
	msgbox City_Hub_Text_EggError, MSGBOX_DEFAULT
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_ForestWaterError::
	msgbox City_Hub_Text_ForestWaterError, MSGBOX_DEFAULT
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_LakeNotWaterError::
	msgbox City_Hub_Text_LakeNotWaterError, MSGBOX_DEFAULT
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_RoamingClearAll::
	special Script_HubRoaming_ClearAll
	goto_if_eq VAR_0x8008, 1, City_Hub_ClearLakeObjects
	removeobject 4
	removeobject 5
	removeobject 6
	removeobject 7
	removeobject 8
	goto City_Hub_ClearDone
City_Hub_ClearLakeObjects::
	removeobject 9
	removeobject 10
	removeobject 11
	removeobject 12
	removeobject 13
City_Hub_ClearDone::
	msgbox City_Hub_Text_AllCleared, MSGBOX_DEFAULT
	goto City_Hub_EventScript_RoamingKeeperLoop

City_Hub_EventScript_RoamingKeeperExit::
	msgbox City_Hub_Text_KeeperExit, MSGBOX_DEFAULT
	release
	end

@ --- ROAMING POKÉMON INTERACTION SCRIPTS ---
City_Hub_EventScript_ForestMon0::
	setvar VAR_0x8004, 0
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_ForestMon1::
	setvar VAR_0x8004, 1
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_ForestMon2::
	setvar VAR_0x8004, 2
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_ForestMon3::
	setvar VAR_0x8004, 3
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_ForestMon4::
	setvar VAR_0x8004, 4
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_LakeMon0::
	setvar VAR_0x8004, 5
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_LakeMon1::
	setvar VAR_0x8004, 6
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_LakeMon2::
	setvar VAR_0x8004, 7
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_LakeMon3::
	setvar VAR_0x8004, 8
	goto City_Hub_EventScript_MonInteract
City_Hub_EventScript_LakeMon4::
	setvar VAR_0x8004, 9
	goto City_Hub_EventScript_MonInteract

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

City_Hub_EventScript_UseSurf::
	lockall
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
City_Hub_Text_ForestKeeper_Intro:
	.string "Benvenuto alla Riserva del Bosco!\n"
	.string "Qui puoi scegliere fino a 5 Pokémon\l"
	.string "dai tuoi Box che pascoleranno liberi.\p"
	.string "Nota bene: i Pokémon di tipo Acqua\n"
	.string "preferiscono il laghetto, quindi qui\l"
	.string "non possono entrare!$"

City_Hub_Text_LakeKeeper_Intro:
	.string "Benvenuto al Laghetto dell'Hub!\n"
	.string "Qui puoi scegliere fino a 5 Pokémon\l"
	.string "dai tuoi Box che nuoteranno liberi.\p"
	.string "Attenzione: nel laghetto possono\n"
	.string "entrare solo Pokémon di tipo Acqua!$"

City_Hub_Text_ChooseMonPrompt:
	.string "Scegli un Pokémon dai tuoi Box.$"

City_Hub_Text_SlotOccupied:
	.string "{STR_VAR_1} è attualmente in questo Slot.\n"
	.string "Cosa desideri fare?$"

City_Hub_Text_OptChange:
	.string "Scegli un altro$"

City_Hub_Text_OptRemove:
	.string "Rimuovi questo$"

City_Hub_Text_OptCancel:
	.string "Annulla$"

City_Hub_Text_SlotCleared:
	.string "Il Pokémon è stato rimosso dallo Slot.$"

City_Hub_Text_AllCleared:
	.string "Tutti i Pokémon dell'area sono stati rimossi.$"

City_Hub_Text_MonAddedSuccess:
	.string "{STR_VAR_1} è stato aggiunto con successo!\n"
	.string "Ora si godrà l'area in totale libertà!$"

City_Hub_Text_EggError:
	.string "Non puoi selezionare un Uovo!$"

City_Hub_Text_ForestWaterError:
	.string "{STR_VAR_1} è di tipo Acqua!\n"
	.string "Nel bosco non possono stare tipi Acqua.\l"
	.string "Portalo al laghetto a sinistra!$"

City_Hub_Text_LakeNotWaterError:
	.string "{STR_VAR_1} non è di tipo Acqua!\n"
	.string "Nel laghetto possono stare esclusivamente\l"
	.string "Pokémon di tipo Acqua!$"

City_Hub_Text_MonHappy:
	.string "{STR_VAR_1} sembra felice e rilassato!$"

City_Hub_Text_KeeperExit:
	.string "Torna pure quando vuoi cambiare i Pokémon!$"
