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

// --- NPC REGOLATORE EXP ---
Kaizo_Hub_EventScript_ExpMultiplierNPC::
	lock
	faceplayer
	goto_if_ne VAR_EXP_MULTIPLIER, 0, Kaizo_Hub_EventScript_ExpMultiplier_HasSetting
	setvar VAR_EXP_MULTIPLIER, 100

Kaizo_Hub_EventScript_ExpMultiplier_HasSetting::
	special BufferCurrentExpMultiplierName
	msgbox Kaizo_Hub_Text_Exp_Intro, MSGBOX_DEFAULT
	dynmultipush Kaizo_Hub_Text_Exp_0x, 1
	dynmultipush Kaizo_Hub_Text_Exp_05x, 50
	dynmultipush Kaizo_Hub_Text_Exp_1x, 100
	dynmultipush Kaizo_Hub_Text_Exp_15x, 150
	dynmultipush Kaizo_Hub_Text_Exp_2x, 200
	dynmultipush Kaizo_Hub_Text_Exp_3x, 300
	dynmultipush Kaizo_Hub_Text_Exp_5x, 500
	dynmultipush Kaizo_Hub_Text_Exp_10x, 1000
	dynmultistack 0, 0, FALSE, 6, FALSE, VAR_EXP_MULTIPLIER, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_ExpMultiplier_Cancel
	copyvar VAR_EXP_MULTIPLIER, VAR_RESULT
	special BufferCurrentExpMultiplierName
	msgbox Kaizo_Hub_Text_Exp_Changed, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_ExpMultiplier_Cancel::
	msgbox Kaizo_Hub_Text_Exp_NoChange, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_Exp_Intro:
	.string "Posso regolare l'esperienza ottenuta\n"
	.string "nelle battaglie durante la run.\p"
	.string "Moltiplicatore attuale: {STR_VAR_1}\p"
	.string "Quale moltiplicatore vuoi impostare?$"

Kaizo_Hub_Text_Exp_Changed:
	.string "Moltiplicatore EXP impostato su:\n"
	.string "{STR_VAR_1}! Buona continuazione!$"

Kaizo_Hub_Text_Exp_NoChange:
	.string "Nessuna modifica effettuata.\n"
	.string "Torna pure se vuoi cambiare l'EXP!$"

Kaizo_Hub_Text_Exp_0x:
	.string "0x (Nessuna EXP)$"

Kaizo_Hub_Text_Exp_05x:
	.string "0.5x (Dimezzata)$"

Kaizo_Hub_Text_Exp_1x:
	.string "1x (Normale)$"

Kaizo_Hub_Text_Exp_15x:
	.string "1.5x$"

Kaizo_Hub_Text_Exp_2x:
	.string "2x (Doppia)$"

Kaizo_Hub_Text_Exp_3x:
	.string "3x (Tripla)$"

Kaizo_Hub_Text_Exp_5x:
	.string "5x$"

Kaizo_Hub_Text_Exp_10x:
	.string "10x$"

