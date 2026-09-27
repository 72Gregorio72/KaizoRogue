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

// --- NPC POTENZIAMENTO IV ---
Kaizo_Hub_EventScript_IVBoosterNPC::
	lock
	faceplayer
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_IVBooster_NoPokemon
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_IVBooster_SingleMon
	msgbox Kaizo_Hub_Text_IV_ChooseMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kaizo_Hub_EventScript_IVBooster_Cancel
	goto Kaizo_Hub_EventScript_IVBooster_StartSession

Kaizo_Hub_EventScript_IVBooster_SingleMon::
	setvar VAR_0x8004, 0

Kaizo_Hub_EventScript_IVBooster_StartSession::
	special Script_IVBooster_Init
	msgbox Kaizo_Hub_Text_IV_Intro, MSGBOX_DEFAULT

Kaizo_Hub_EventScript_IVBooster_StatMenu::
	special Script_IVBooster_GetRemainingPoints
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_IVBooster_FinishedAllPoints
	msgbox Kaizo_Hub_Text_IV_SelectStat, MSGBOX_DEFAULT
	special Script_IVBooster_PushStatChoices
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 6, Kaizo_Hub_EventScript_IVBooster_Cancel
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_IVBooster_Cancel
	copyvar VAR_0x8005, VAR_RESULT
	special Script_IVBooster_SelectStat
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_IVBooster_StatMaxed
	msgbox Kaizo_Hub_Text_IV_AskPoints, MSGBOX_DEFAULT
	special Script_IVBooster_PushPointChoices
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_IVBooster_StatMenu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_IVBooster_StatMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_IVBooster_ApplyPoints
	playse SE_EXP
	msgbox Kaizo_Hub_Text_IV_StatUpgraded, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_IVBooster_StatMenu

Kaizo_Hub_EventScript_IVBooster_StatMaxed::
	msgbox Kaizo_Hub_Text_IV_StatAlreadyMax, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_IVBooster_StatMenu

Kaizo_Hub_EventScript_IVBooster_NoPokemon::
	msgbox Kaizo_Hub_Text_IV_NoPokemon, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_IVBooster_FinishedAllPoints::
	msgbox Kaizo_Hub_Text_IV_AllPointsUsed, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_IVBooster_Disappear

Kaizo_Hub_EventScript_IVBooster_Cancel::
	msgbox Kaizo_Hub_Text_IV_Goodbye, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_IVBooster_Disappear

Kaizo_Hub_EventScript_IVBooster_Disappear::
	closemessage
	playse SE_M_TELEPORT
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	setflag FLAG_HIDE_IV_BOOSTER_NPC
	fadescreenswapbuffers FADE_FROM_BLACK
	release
	end

Kaizo_Hub_Text_IV_NoPokemon:
	.string "Non hai nessun POKéMON con te!$"

Kaizo_Hub_Text_IV_ChooseMon:
	.string "Di quale POKéMON vuoi\n"
	.string "potenziare i valori IV?$"

Kaizo_Hub_Text_IV_Intro:
	.string "Posso aumentare gli IV del tuo\n"
	.string "{STR_VAR_1} fino a 5 punti totali!\p"
	.string "Scegli con saggezza come\n"
	.string "distribuire questo dono.$"

Kaizo_Hub_Text_IV_SelectStat:
	.string "Punti rimanenti: {STR_VAR_2}/5.\n"
	.string "Quale statistica vuoi potenziare?$"

Kaizo_Hub_Text_IV_AskPoints:
	.string "Di quanti punti vuoi potenziare\n"
	.string "la statistica {STR_VAR_1}?\l"
	.string "(Attuale: {STR_VAR_2}/31 - Punti rimasti: {STR_VAR_3})$"

Kaizo_Hub_Text_IV_StatUpgraded:
	.string "Statistica {STR_VAR_1} potenziata di +{STR_VAR_2}!\n"
	.string "Nuovo valore: {STR_VAR_3}/31.$"

Kaizo_Hub_Text_IV_StatAlreadyMax:
	.string "Questa statistica ha già 31 IV\n"
	.string "(il valore massimo possibile)!$"

Kaizo_Hub_Text_IV_AllPointsUsed:
	.string "Hai utilizzato tutti e 5 i punti\n"
	.string "IV a tua disposizione!\p"
	.string "Il mio lavoro qui è terminato.\n"
	.string "Buona fortuna per la tua run!$"

Kaizo_Hub_Text_IV_Goodbye:
	.string "Capisco. Il mio lavoro qui è terminato.\n"
	.string "Buona fortuna per la tua run!$"

