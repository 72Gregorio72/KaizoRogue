Kaizo_Hub_MapScripts::
	.byte 0

// NPC che genera il Seed e warpa il giocatore nella camera da letto
Kaizo_Hub_EventScript_TesterNPC::
	lock
	faceplayer
	getpartysize
	goto_if_gt VAR_RESULT, 0, Kaizo_Hub_EventScript_AskWarpOnly

	msgbox Kaizo_Hub_Text_Intro, MSGBOX_DEFAULT
	special GenerateNewKaizoSeedAndStarters

	msgbox Kaizo_Hub_Text_WarpToRoom, MSGBOX_DEFAULT
	closemessage

	// Teletrasporto diretto nella camera da letto (2F)
	warp MAP_PALLET_TOWN_PLAYERS_HOUSE_2F, 3, 6
	waitstate
	release
	end

Kaizo_Hub_EventScript_AskWarpOnly::
	msgbox Kaizo_Hub_Text_AskWarp, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kaizo_Hub_EventScript_Cancel
	msgbox Kaizo_Hub_Text_Warping, MSGBOX_DEFAULT
	closemessage
	warp MAP_PALLET_TOWN_PLAYERS_HOUSE_2F, 3, 6
	waitstate
	release
	end

Kaizo_Hub_EventScript_Cancel::
	release
	end

// --- BALL 1 (SINISTRA) ---
Kaizo_Hub_EventScript_StarterBall1::
	lock
	getpartysize
	goto_if_gt VAR_RESULT, 0, Kaizo_Hub_EventScript_BallAlreadyChosen
	setvar VAR_0x8004, 0
	special BufferKaizoStarterName
	showmonpic VAR_RESULT, 10, 3
	msgbox Kaizo_Hub_Text_ConfirmStarter, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kaizo_Hub_EventScript_TakeStarter
	hidemonpic
	release
	end

// --- BALL 2 (CENTRO) ---
Kaizo_Hub_EventScript_StarterBall2::
	lock
	getpartysize
	goto_if_gt VAR_RESULT, 0, Kaizo_Hub_EventScript_BallAlreadyChosen
	setvar VAR_0x8004, 1
	special BufferKaizoStarterName
	showmonpic VAR_RESULT, 10, 3
	msgbox Kaizo_Hub_Text_ConfirmStarter, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kaizo_Hub_EventScript_TakeStarter
	hidemonpic
	release
	end

// --- BALL 3 (DESTRA) ---
Kaizo_Hub_EventScript_StarterBall3::
	lock
	getpartysize
	goto_if_gt VAR_RESULT, 0, Kaizo_Hub_EventScript_BallAlreadyChosen
	setvar VAR_0x8004, 2
	special BufferKaizoStarterName
	showmonpic VAR_RESULT, 10, 3
	msgbox Kaizo_Hub_Text_ConfirmStarter, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Kaizo_Hub_EventScript_TakeStarter
	hidemonpic
	release
	end

Kaizo_Hub_EventScript_TakeStarter::
	hidemonpic
	special GiveSelectedStarter
	playfanfare MUS_OBTAIN_ITEM
	msgbox Kaizo_Hub_Text_ReceivedStarter, MSGBOX_DEFAULT
	waitfanfare
	release
	end

Kaizo_Hub_EventScript_BallAlreadyChosen::
	msgbox Kaizo_Hub_Text_AlreadyHaveMon, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_Intro:
	.string "Nuova run Kaizo Ironmon!\nGenerazione del Seed in corso...$"

Kaizo_Hub_Text_WarpToRoom:
	.string "Seed generato e mondo randomizzato!\nBuona fortuna per la run!$"

Kaizo_Hub_Text_ConfirmStarter:
	.string "Contiene {STR_VAR_1}!\nVuoi scegliere {STR_VAR_1}?$"

Kaizo_Hub_Text_ReceivedStarter:
	.string "Hai ottenuto il tuo starter Kaizo!$"

Kaizo_Hub_Text_AlreadyHaveMon:
	.string "Hai gia' scelto il tuo starter!$"

Kaizo_Hub_Text_Warping:
	.string "Teletrasporto in corso...$"

Kaizo_Hub_Text_AskWarp:
	.string "Vuoi tornare a casa tua a Pallet Town?$"
