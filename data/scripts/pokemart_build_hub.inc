Common_EventScript_PokemartBuildHub::
	special Script_MartBuild_CheckCanUse
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_AlreadyUsed
	special Script_MartBuild_InitSession
	msgbox PokemartBuild_Text_Welcome, MSGBOX_DEFAULT

PokemartBuild_EventScript_MainMenu::
	msgbox PokemartBuild_Text_WhatToDo, MSGBOX_DEFAULT
	special Script_MartBuild_PushMainMenuChoices
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_ExitMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_MartBuild_SelectOption
	goto_if_eq VAR_RESULT, 100, PokemartBuild_EventScript_OptionAlreadyUsed
	switch VAR_RESULT
	case 0, PokemartBuild_EventScript_StatPoints
	case 1, PokemartBuild_EventScript_MoveDraft
	case 2, PokemartBuild_EventScript_MoveRelearner
	case 3, PokemartBuild_EventScript_IVBooster
	case 4, PokemartBuild_EventScript_EVManager
	case 5, PokemartBuild_EventScript_NatureChanger
	case 6, PokemartBuild_EventScript_AbilityChanger
	case 7, PokemartBuild_EventScript_Shop
	case 8, PokemartBuild_EventScript_ExitMenu
	goto PokemartBuild_EventScript_MainMenu

PokemartBuild_EventScript_OptionAlreadyUsed::
	msgbox PokemartBuild_Text_OptionAlreadyUsed, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

PokemartBuild_EventScript_AlreadyUsed::
	msgbox PokemartBuild_Text_AlreadyUsedPrompt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, PokemartBuild_EventScript_Shop
	msgbox PokemartBuild_Text_AlreadyUsedBye, MSGBOX_DEFAULT
	setvar VAR_RESULT, 0
	return

PokemartBuild_EventScript_Shop::
	setvar VAR_RESULT, 99
	return

PokemartBuild_EventScript_ExitMenu::
	special Script_MartBuild_ExitSession
	goto_if_eq VAR_RESULT, 1, PokemartBuild_EventScript_SessionSaved
	msgbox PokemartBuild_Text_ExitNoChanges, MSGBOX_DEFAULT
	setvar VAR_RESULT, 0
	return

PokemartBuild_EventScript_SessionSaved::
	playse SE_SAVE
	msgbox PokemartBuild_Text_FinalizedSuccess, MSGBOX_DEFAULT
	setvar VAR_RESULT, 0
	return

@ --- 1. STAT POINTS ---
PokemartBuild_EventScript_StatPoints::
	setvar VAR_0x8004, 0
	special Script_StatPoints_Init
	goto_if_eq VAR_RESULT, 1, PokemartBuild_EventScript_StatPoints_Capped
	goto_if_eq VAR_RESULT, 2, PokemartBuild_EventScript_StatPoints_NoPoints

PokemartBuild_EventScript_StatPoints_Menu::
	special Script_StatPoints_GetRemainingPoints
	msgbox PokemartBuild_Text_StatPoints_SelectStat, MSGBOX_DEFAULT
	special Script_StatPoints_PushStatChoices
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_StatPoints_Done
	copyvar VAR_0x8005, VAR_RESULT
	special Script_StatPoints_SelectStat
	goto_if_eq VAR_RESULT, 1, PokemartBuild_EventScript_StatPoints_Done
	msgbox PokemartBuild_Text_StatPoints_AskAmount, MSGBOX_DEFAULT
	special Script_StatPoints_PushAmountChoices
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_StatPoints_Menu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_StatPoints_Menu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_StatPoints_Apply
	playse SE_PIN
	setvar VAR_0x8005, 0
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_StatPoints_Menu

PokemartBuild_EventScript_StatPoints_Done::
	goto PokemartBuild_EventScript_MainMenu

PokemartBuild_EventScript_StatPoints_Capped::
	msgbox PokemartBuild_Text_StatPoints_CappedMsg, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

PokemartBuild_EventScript_StatPoints_NoPoints::
	msgbox PokemartBuild_Text_StatPoints_NoPointsMsg, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

@ --- 2. MOVE DRAFT (5 RANDOM MOVES) ---
PokemartBuild_EventScript_MoveDraft::
	msgbox PokemartBuild_Text_MoveDraft_Intro, MSGBOX_DEFAULT
	special Script_MoveDraft_Roll
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_MoveDraft_Cancel
	goto_if_eq VAR_RESULT, 5, PokemartBuild_EventScript_MoveDraft_Cancel
	copyvar VAR_0x8005, VAR_RESULT
	special Script_MoveDraft_Select
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_MainMenu
	special ChooseMonForMoveTutor
	goto_if_eq VAR_RESULT, FALSE, PokemartBuild_EventScript_MoveDraft_Cancel
	playse SE_EXP
	msgbox PokemartBuild_Text_MoveDraft_Success, MSGBOX_DEFAULT
	setvar VAR_0x8005, 1
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_MainMenu

PokemartBuild_EventScript_MoveDraft_Cancel::
	msgbox PokemartBuild_Text_MoveDraft_Cancelled, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

@ --- 3. MOVE RELEARNER ---
PokemartBuild_EventScript_MoveRelearner::
	call Common_EventScript_MoveRelearner
	setvar VAR_0x8005, 2
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_MainMenu

@ --- 4. MODIFY IVS ---
PokemartBuild_EventScript_IVBooster::
	setvar VAR_0x8004, 0
	special Script_IVBooster_Init

PokemartBuild_EventScript_IVBooster_Menu::
	special Script_IVBooster_GetRemainingPoints
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_IVBooster_Finished
	msgbox PokemartBuild_Text_IV_SelectStat, MSGBOX_DEFAULT
	special Script_IVBooster_PushStatChoices
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 6, PokemartBuild_EventScript_MainMenu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_MainMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_IVBooster_SelectStat
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_IVBooster_StatMaxed
	msgbox PokemartBuild_Text_IV_AskPoints, MSGBOX_DEFAULT
	special Script_IVBooster_PushPointChoices
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_IVBooster_Menu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_IVBooster_Menu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_IVBooster_ApplyPoints
	playse SE_PIN
	setvar VAR_0x8005, 3
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_IVBooster_Menu

PokemartBuild_EventScript_IVBooster_StatMaxed::
	msgbox PokemartBuild_Text_IV_StatMaxed, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_IVBooster_Menu

PokemartBuild_EventScript_IVBooster_Finished::
	msgbox PokemartBuild_Text_IV_Finished, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

@ --- 5. TRAIN EVS ---
PokemartBuild_EventScript_EVManager::
	setvar VAR_0x8004, 0
	special Script_EVManager_Init
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_EVManager_NoEVs

PokemartBuild_EventScript_EVManager_Menu::
	special Script_EVManager_GetRemainingPoints
	msgbox PokemartBuild_Text_EV_SelectStat, MSGBOX_DEFAULT
	special Script_EVManager_PushStatChoices
	dynmultistack 0, 0, FALSE, 8, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_MainMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_EVManager_SelectStat
	goto_if_eq VAR_RESULT, 1, PokemartBuild_EventScript_EVManager_Saved
	goto_if_eq VAR_RESULT, 2, PokemartBuild_EventScript_MainMenu
	msgbox PokemartBuild_Text_EV_AskPoints, MSGBOX_DEFAULT
	special Script_EVManager_PushPointChoices
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, PokemartBuild_EventScript_EVManager_Menu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, PokemartBuild_EventScript_EVManager_Menu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_EVManager_ApplyPoints
	playse SE_PIN
	setvar VAR_0x8005, 4
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_EVManager_Menu

PokemartBuild_EventScript_EVManager_Saved::
	playse SE_EXP
	msgbox PokemartBuild_Text_EV_SavedSuccess, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

PokemartBuild_EventScript_EVManager_NoEVs::
	msgbox PokemartBuild_Text_EV_NoEVs, MSGBOX_DEFAULT
	goto PokemartBuild_EventScript_MainMenu

@ --- 6. CHANGE NATURE ---
PokemartBuild_EventScript_NatureChanger::
	setvar VAR_0x8004, 0
	special Script_NatureChanger_Init
	msgbox PokemartBuild_Text_Nature_AskReroll, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemartBuild_EventScript_MainMenu
	special Script_NatureChanger_Apply
	playse SE_EXP
	msgbox PokemartBuild_Text_Nature_Success, MSGBOX_DEFAULT
	setvar VAR_0x8005, 5
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_MainMenu

@ --- 7. CHANGE ABILITY ---
PokemartBuild_EventScript_AbilityChanger::
	setvar VAR_0x8004, 0
	special Script_AbilityChanger_Init
	msgbox PokemartBuild_Text_Ability_AskReroll, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PokemartBuild_EventScript_MainMenu
	special Script_AbilityChanger_Apply
	playse SE_EXP
	msgbox PokemartBuild_Text_Ability_Success, MSGBOX_DEFAULT
	setvar VAR_0x8005, 6
	special Script_MartBuild_MarkOptionUsed
	goto PokemartBuild_EventScript_MainMenu


@ --- ENGLISH TEXTS AND DIALOGUES ---

PokemartBuild_Text_Welcome:
	.string "Welcome to the Poké Mart Build Hub!\p"
	.string "Here you can customize and enhance\n"
	.string "your POKéMON (Available once per city).\p"
	.string "Once you finish and make changes,\n"
	.string "your build for this city will be saved!$"

PokemartBuild_Text_WhatToDo:
	.string "What would you like to modify?$"

PokemartBuild_Text_OptionAlreadyUsed:
	.string "You have already used this option\n"
	.string "in this session!\p"
	.string "Please choose a different modification.$"

PokemartBuild_Text_AlreadyUsedPrompt:
	.string "Welcome to the Poké Mart!\p"
	.string "You have already completed your build\n"
	.string "customization for this city.\p"
	.string "Would you like to buy standard items?$"

PokemartBuild_Text_AlreadyUsedBye:
	.string "Please visit us again in the next city!$"

PokemartBuild_Text_ExitNoChanges:
	.string "No modifications were made.\n"
	.string "Please visit us again when you are ready!$"

PokemartBuild_Text_FinalizedSuccess:
	.string "Your build has been saved for this city!\n"
	.string "Good luck on your upcoming battles!$"

PokemartBuild_Text_StatPoints_SelectStat:
	.string "Available stat points: {STR_VAR_3}\n"
	.string "Current BST: {STR_VAR_2}/500\n"
	.string "Choose a stat to modify:$"

PokemartBuild_Text_StatPoints_AskAmount:
	.string "Stat: {STR_VAR_1}\n"
	.string "(Current Base: {STR_VAR_2} - Bonus: +{STR_VAR_3})\n"
	.string "Choose points to add or remove:$"

PokemartBuild_Text_StatPoints_CappedMsg:
	.string "{STR_VAR_1}'s BST has already reached\n"
	.string "or exceeds 500 ({STR_VAR_2} BST).\p"
	.string "It cannot receive any more stat points!$"

PokemartBuild_Text_StatPoints_NoPointsMsg:
	.string "You have no stat points to distribute.\p"
	.string "Defeat Gym Leaders to earn\n"
	.string "+20 Points for each badge won!\n"
	.string "(Current badges: {STR_VAR_3}/8)$"

PokemartBuild_Text_MoveDraft_Intro:
	.string "Here are 5 moves drafted for you!\n"
	.string "Choose a move to teach:$"

PokemartBuild_Text_MoveDraft_Success:
	.string "The move was taught successfully!$"

PokemartBuild_Text_MoveDraft_Cancelled:
	.string "No move was taught.$"

PokemartBuild_Text_IV_SelectStat:
	.string "Available IV points: {STR_VAR_2}\n"
	.string "Choose a stat to boost:$"

PokemartBuild_Text_IV_AskPoints:
	.string "Stat: {STR_VAR_1}\n"
	.string "Current IV: {STR_VAR_2}/31\n"
	.string "How many IV points to add?$"

PokemartBuild_Text_IV_StatMaxed:
	.string "This stat is already at 31 IVs (Max)!$"

PokemartBuild_Text_IV_Finished:
	.string "All IVs are maxed out!$"

PokemartBuild_Text_EV_SelectStat:
	.string "Remaining EV points: {STR_VAR_2}\n"
	.string "Choose a stat to modify:$"

PokemartBuild_Text_EV_AskPoints:
	.string "Modify EVs for {STR_VAR_1}:\n"
	.string "(Current: {STR_VAR_2}/252 - Remaining: {STR_VAR_3})$"

PokemartBuild_Text_EV_SavedSuccess:
	.string "EVs have been successfully updated!$"

PokemartBuild_Text_EV_NoEVs:
	.string "{STR_VAR_1} has not accumulated any EVs\n"
	.string "to redistribute yet (0/510).$"

PokemartBuild_Text_Nature_AskReroll:
	.string "Would you like to change {STR_VAR_1}'s Nature?\n"
	.string "(Current Nature: {STR_VAR_2})\p"
	.string "It will receive a new random Nature!$"

PokemartBuild_Text_Nature_Success:
	.string "{STR_VAR_1}'s Nature changed to:\n"
	.string "{STR_VAR_2}!$"

PokemartBuild_Text_Ability_AskReroll:
	.string "Would you like to change {STR_VAR_1}'s Ability?\n"
	.string "(Current Ability: {STR_VAR_2})\p"
	.string "It will receive a new random Ability!$"

PokemartBuild_Text_Ability_Success:
	.string "{STR_VAR_1}'s Ability changed to:\n"
	.string "{STR_VAR_2}!$"
