Kaizo_Hub_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Kaizo_Hub_OnFrame
	.byte 0

Kaizo_Hub_OnFrame:
	map_script_2 VAR_TEMP_0, 0, Kaizo_Hub_EventScript_CheckRunRecap
	.2byte 0

Kaizo_Hub_EventScript_CheckRunRecap::
	setvar VAR_TEMP_0, 1
	special Script_Meta_GetLastRunRecap
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_RecapDone
	lockall
	playfanfare MUS_LEVEL_UP
	msgbox Kaizo_Hub_Text_RunRecap, MSGBOX_DEFAULT
	waitfanfare
	releaseall
Kaizo_Hub_EventScript_RecapDone::
	end

// NPC che genera il Seed e warpa il giocatore nella camera da letto
Kaizo_Hub_EventScript_TesterNPC::
	lock
	faceplayer
	msgbox Kaizo_Hub_Text_Intro, MSGBOX_DEFAULT
	special Script_Kaizo_PrepareNewRun
	goto_if_eq VAR_0x8004, 1, Kaizo_Hub_EventScript_StartRunWithGacha
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_StartRunDirect
	msgbox Kaizo_Hub_Text_PartyReturnedToPC, MSGBOX_DEFAULT
Kaizo_Hub_EventScript_StartRunDirect::
	msgbox Kaizo_Hub_Text_WarpToRoom, MSGBOX_DEFAULT
	closemessage
	warp MAP_PALLET_TOWN_PLAYERS_HOUSE_2F, 3, 6
	waitstate
	release
	end

Kaizo_Hub_EventScript_StartRunWithGacha::
	playfanfare MUS_LEVEL_UP
	msgbox Kaizo_Hub_Text_StartingWithGachaStarter, MSGBOX_DEFAULT
	waitfanfare
	msgbox Kaizo_Hub_Text_WarpToRoom, MSGBOX_DEFAULT
	closemessage
	warp MAP_PALLET_TOWN_PLAYERS_HOUSE_2F, 3, 6
	waitstate
	release
	end

Kaizo_Hub_EventScript_PC::
	lock
	faceplayer
	playse SE_PC_ON
	msgbox Kaizo_Hub_Text_PCIntro, MSGBOX_DEFAULT
	dynmultipush Kaizo_Hub_Text_PCOpt_Roster, 0
	dynmultipush Kaizo_Hub_Text_PCOpt_Classic, 1
	dynmultipush Kaizo_Hub_Text_PCOpt_Exit, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_PCTurnOff
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_PCTurnOff
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_OpenRosterPC
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_OpenClassicPC
	goto Kaizo_Hub_EventScript_PCTurnOff

Kaizo_Hub_EventScript_OpenRosterPC::
	playse SE_PC_LOGIN
	special OpenGachaRosterPC
	goto Kaizo_Hub_EventScript_PCTurnOff

Kaizo_Hub_EventScript_OpenClassicPC::
	playse SE_PC_LOGIN
	msgbox gText_StorageSystemOpened, MSGBOX_DEFAULT
	special ShowPokemonStorageSystemPC
	goto Kaizo_Hub_EventScript_PCTurnOff

Kaizo_Hub_EventScript_PCTurnOff::
	playse SE_PC_OFF
	msgbox Kaizo_Hub_Text_PCGoodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_Cancel::
	release
	end

Kaizo_Hub_Text_PartyReturnedToPC:
	.string "I Pokémon che avevi in squadra sono stati\n"
	.string "riposti al sicuro nel PC per la run!$"

Kaizo_Hub_Text_PCIntro:
	.string "BILL: Ciao! Questo e' il Terminale PC\n"
	.string "del Memoriale dei Caduti.\p"
	.string "Tutti i tuoi Pokémon caduti nelle run\n"
	.string "sono salvati con statistiche, mosse,\l"
	.string "abilita', natura e livello!\p"
	.string "Vuoi accedere ai Box del PC?$"

Kaizo_Hub_Text_PCGoodbye:
	.string "BILL: Se ritiri un Pokémon nella Hub,\n"
	.string "verra' riposto automaticamente nel PC\l"
	.string "appena farai partire una nuova run!$"

Kaizo_Hub_Text_PCOpt_Roster:
	.string "PC Roster & Starter Run$"

Kaizo_Hub_Text_PCOpt_Classic:
	.string "Deposito Box Classico$"

Kaizo_Hub_Text_PCOpt_Exit:
	.string "Spegni PC$"


Kaizo_Hub_EventScript_Receptionist::
	lock
	faceplayer
	msgbox Kaizo_Hub_Text_Receptionist, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_CheatNPC::
	lock
	faceplayer
	msgbox Kaizo_Hub_Text_CheatIntro, MSGBOX_DEFAULT
Kaizo_Hub_EventScript_CheatMenu::
	special Script_Meta_PushCheatChoices
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_CheatExit
	goto_if_eq VAR_RESULT, 4, Kaizo_Hub_EventScript_CheatExit
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_CheatMaxAll
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_CheatMoney
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_CheatGacha
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_CheatReset
	goto Kaizo_Hub_EventScript_CheatExit

Kaizo_Hub_EventScript_CheatMaxAll::
	special Script_Meta_CheatMaxAll
	playfanfare MUS_OBTAIN_ITEM
	msgbox Kaizo_Hub_Text_CheatMaxAllDone, MSGBOX_DEFAULT
	waitfanfare
	goto Kaizo_Hub_EventScript_CheatMenu

Kaizo_Hub_EventScript_CheatMoney::
	special Script_Meta_CheatInfiniteMoneyAndShards
	playfanfare MUS_OBTAIN_ITEM
	msgbox Kaizo_Hub_Text_CheatMoneyDone, MSGBOX_DEFAULT
	waitfanfare
	goto Kaizo_Hub_EventScript_CheatMenu

Kaizo_Hub_EventScript_CheatGacha::
	special Script_Meta_CheatAddTicketsAndPoints
	playfanfare MUS_OBTAIN_ITEM
	msgbox Kaizo_Hub_Text_CheatGachaDone, MSGBOX_DEFAULT
	waitfanfare
	goto Kaizo_Hub_EventScript_CheatMenu

Kaizo_Hub_EventScript_CheatReset::
	special Script_Meta_CheatReset
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_CheatResetDone, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_CheatMenu

Kaizo_Hub_EventScript_CheatExit::
	msgbox Kaizo_Hub_Text_CheatGoodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_MainMonument::
	msgbox Kaizo_Hub_Text_MainMonument, MSGBOX_SIGN
	end

Kaizo_Hub_EventScript_MartDoorSign::
	msgbox Kaizo_Hub_Text_MartDoorSign, MSGBOX_SIGN
	end

Kaizo_Hub_EventScript_StarterDoorSign::
	msgbox Kaizo_Hub_Text_StarterDoorSign, MSGBOX_SIGN
	end

Kaizo_Hub_EventScript_FortuneDoorSign::
	msgbox Kaizo_Hub_Text_FortuneDoorSign, MSGBOX_SIGN
	end

Kaizo_Hub_EventScript_PierSign::
	msgbox Kaizo_Hub_Text_PierSign, MSGBOX_SIGN
	end

Kaizo_Hub_Text_CheatIntro:
	.string "DEBUGGER KAIZO:\n"
	.string "Posso modificare i salvataggi per\n"
	.string "facilitare il testing della Hub!$"

Kaizo_Hub_Text_CheatMaxAllDone:
	.string "Tutti i Powerup sono stati portati\n"
	.string "al livello MASSIMO!\p"
	.string "- Draft Mosse: 5 scelte\n"
	.string "- Punti Stat: +20 BST\n"
	.string "- Licenze IV, EV, Natura, Abilita'\n"
	.string "- Reroll Starter: 2\n"
	.string "- Mosse Starter: Rarita' Rara\n"
	.string "- Tasca Iniziale: Baccacedro\n"
	.string "- Dea Bendata: Livello 3 (MAX)$"

Kaizo_Hub_Text_CheatMoneyDone:
	.string "Ricevuti 999.999 Frammenti Kaizo\n"
	.string "e 999.999 Pomenny!$"

Kaizo_Hub_Text_CheatResetDone:
	.string "Tutti i potenziamenti e le risorse\n"
	.string "sono stati azzerati ai valori iniziali.$"

Kaizo_Hub_Text_CheatGoodbye:
	.string "Buon testing! Torna quando vuoi.$"

Kaizo_Hub_Text_Receptionist:
	.string "Benvenuto al Villaggio Kaizo!\p"
	.string "Esplora le strutture dedicate attorno\n"
	.string "alla piazzetta per sbloccare upgrade:\p"
	.string "- Ovest: Emporio e Dojo Licenze\n"
	.string "- Nord: Laboratorio Ricerca Starter\n"
	.string "- Est: Santuario della Sorte\p"
	.string "Quando sei pronto, parla con l'Arbitro\n"
	.string "al centro per iniziare la tua run!$"

Kaizo_Hub_Text_MainMonument:
	.string "PIAZZETTA CENTRALE KAIZO\n"
	.string "Il cuore del villaggio aperto.\p"
	.string "- PIAZZA: Arbitro Run e PC Memoriale\n"
	.string "- NORD: Laboratorio Ricerca Starter\n"
	.string "- OVEST: Emporio e Dojo Licenze\n"
	.string "- EST: Santuario della Sorte\n"
	.string "- SUD: Molo di Partenza$"

Kaizo_Hub_Text_MartDoorSign:
	.string "EMPORIO KAIZO & BUILD DOJO\n"
	.string "Licenze di Mercato e Personalizzazione.$"

Kaizo_Hub_Text_StarterDoorSign:
	.string "LABORATORIO RICERCA GENETICA\n"
	.string "Miglioramenti per Pokemon Iniziali.$"

Kaizo_Hub_Text_FortuneDoorSign:
	.string "SANTUARIO DELLA SORTE\n"
	.string "Preghiere per il bottino raro ed epico.$"

Kaizo_Hub_Text_PierSign:
	.string "MOLO KAIZO\n"
	.string "Verso l'orizzonte della sfida Ironmon.$"

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

// --- NPC GESTORE / RIDISTRIBUTORE EV ---
Kaizo_Hub_EventScript_EVManagerNPC::
	lock
	faceplayer
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_EVManager_NoPokemon
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_EVManager_SingleMon
	msgbox Kaizo_Hub_Text_EV_ChooseMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kaizo_Hub_EventScript_EVManager_Cancel
	goto Kaizo_Hub_EventScript_EVManager_StartSession

Kaizo_Hub_EventScript_EVManager_SingleMon::
	setvar VAR_0x8004, 0

Kaizo_Hub_EventScript_EVManager_StartSession::
	special Script_EVManager_Init
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_EVManager_NoEVsEarned
	msgbox Kaizo_Hub_Text_EV_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kaizo_Hub_EventScript_EVManager_Cancel

Kaizo_Hub_EventScript_EVManager_StatMenu::
	special Script_EVManager_GetRemainingPoints
	msgbox Kaizo_Hub_Text_EV_SelectStat, MSGBOX_DEFAULT
	special Script_EVManager_PushStatChoices
	dynmultistack 0, 0, FALSE, 8, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_EVManager_Cancel
	copyvar VAR_0x8005, VAR_RESULT
	special Script_EVManager_SelectStat
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_EVManager_Saved
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_EVManager_Cancel
	msgbox Kaizo_Hub_Text_EV_AskPoints, MSGBOX_DEFAULT
	special Script_EVManager_PushPointChoices
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_EVManager_StatMenu
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_EVManager_StatMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_EVManager_ApplyPoints
	playse SE_PIN
	goto Kaizo_Hub_EventScript_EVManager_StatMenu

Kaizo_Hub_EventScript_EVManager_Saved::
	playse SE_EXP
	msgbox Kaizo_Hub_Text_EV_SavedSuccess, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_EVManager_NoEVsEarned::
	msgbox Kaizo_Hub_Text_EV_NoEVs, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_EVManager_NoPokemon::
	msgbox Kaizo_Hub_Text_EV_NoPokemon, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_EVManager_Cancel::
	msgbox Kaizo_Hub_Text_EV_Goodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_EV_NoPokemon:
	.string "Non hai nessun POKéMON con te!$"

Kaizo_Hub_Text_EV_ChooseMon:
	.string "Di quale POKéMON vuoi\n"
	.string "ridistribuire i Punti EV?$"

Kaizo_Hub_Text_EV_NoEVs:
	.string "{STR_VAR_1} non ha ancora\n"
	.string "guadagnato alcun Punto EV (0/510).\p"
	.string "Sconfiggi altri POKéMON durante la\n"
	.string "run per accumulare EV da ridistribuire!$"

Kaizo_Hub_Text_EV_Intro:
	.string "{STR_VAR_1} ha accumulato\n"
	.string "{STR_VAR_2} Punti EV totali!\p"
	.string "Vuoi resettarli e ridistribuirli\n"
	.string "come preferisci tra le statistiche?$"

Kaizo_Hub_Text_EV_SelectStat:
	.string "Punti EV rimasti da assegnare: {STR_VAR_2}\n"
	.string "Scegli una statistica su cui lavorare:$"

Kaizo_Hub_Text_EV_AskPoints:
	.string "Modifica EV per {STR_VAR_1}:\n"
	.string "(Attuale: {STR_VAR_2}/252 - Rimasti: {STR_VAR_3})$"

Kaizo_Hub_Text_EV_SavedSuccess:
	.string "Gli EV di {STR_VAR_1} sono stati\n"
	.string "ridistribuiti con successo!\p"
	.string "Torna pure quando vuoi riorganizzarli!$"

Kaizo_Hub_Text_EV_Goodbye:
	.string "Nessuna modifica applicata agli EV.\n"
	.string "Torna quando vuoi!$"

// --- NPC CAMBIO NATURA RANDOM ---
Kaizo_Hub_EventScript_NatureChangerNPC::
	lock
	faceplayer
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_Nature_NoPokemon
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_Nature_SingleMon
	msgbox Kaizo_Hub_Text_Nature_ChooseMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kaizo_Hub_EventScript_Nature_Cancel
	goto Kaizo_Hub_EventScript_Nature_Confirm

Kaizo_Hub_EventScript_Nature_SingleMon::
	setvar VAR_0x8004, 0

Kaizo_Hub_EventScript_Nature_Confirm::
	special Script_NatureChanger_Init
	msgbox Kaizo_Hub_Text_Nature_AskReroll, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kaizo_Hub_EventScript_Nature_Cancel
	special Script_NatureChanger_Apply
	playse SE_EXP
	msgbox Kaizo_Hub_Text_Nature_Success, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_Nature_NoPokemon::
	msgbox Kaizo_Hub_Text_Nature_NoPokemon, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_Nature_Cancel::
	msgbox Kaizo_Hub_Text_Nature_Goodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_Nature_NoPokemon:
	.string "Non hai nessun POKéMON con te!$"

Kaizo_Hub_Text_Nature_ChooseMon:
	.string "Di quale POKéMON vuoi\n"
	.string "cambiare la Natura?$"

Kaizo_Hub_Text_Nature_AskReroll:
	.string "Vuoi cambiare la Natura di {STR_VAR_1}?\n"
	.string "(Natura attuale: {STR_VAR_2})\p"
	.string "Ne ricevera' una nuova casuale!$"

Kaizo_Hub_Text_Nature_Success:
	.string "La Natura di {STR_VAR_1} e' cambiata in:\n"
	.string "{STR_VAR_2}!\p"
	.string "Le sue statistiche sono state aggiornate.\n"
	.string "Torna quando vuoi per un altro reroll!$"

Kaizo_Hub_Text_Nature_Goodbye:
	.string "Nessuna modifica alla Natura.\n"
	.string "Torna pure quando vuoi!$"

// --- NPC CAMBIO ABILITA RANDOM ---
Kaizo_Hub_EventScript_AbilityChangerNPC::
	lock
	faceplayer
	specialvar VAR_RESULT, CalculatePlayerPartyCount
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_Ability_NoPokemon
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_Ability_SingleMon
	msgbox Kaizo_Hub_Text_Ability_ChooseMon, MSGBOX_DEFAULT
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, Kaizo_Hub_EventScript_Ability_Cancel
	goto Kaizo_Hub_EventScript_Ability_Confirm

Kaizo_Hub_EventScript_Ability_SingleMon::
	setvar VAR_0x8004, 0

Kaizo_Hub_EventScript_Ability_Confirm::
	special Script_AbilityChanger_Init
	msgbox Kaizo_Hub_Text_Ability_AskReroll, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kaizo_Hub_EventScript_Ability_Cancel
	special Script_AbilityChanger_Apply
	playse SE_EXP
	msgbox Kaizo_Hub_Text_Ability_Success, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_Ability_NoPokemon::
	msgbox Kaizo_Hub_Text_Ability_NoPokemon, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_Ability_Cancel::
	msgbox Kaizo_Hub_Text_Ability_Goodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_Ability_NoPokemon:
	.string "Non hai nessun POKéMON con te!$"

Kaizo_Hub_Text_Ability_ChooseMon:
	.string "Di quale POKéMON vuoi\n"
	.string "cambiare l'Abilita'?$"

Kaizo_Hub_Text_Ability_AskReroll:
	.string "Vuoi cambiare l'Abilita' di {STR_VAR_1}?\n"
	.string "(Abilita' attuale: {STR_VAR_2})\p"
	.string "Ne ricevera' una casuale da tutto il gioco!$"

Kaizo_Hub_Text_Ability_Success:
	.string "L'Abilita' di {STR_VAR_1} e' cambiata in:\n"
	.string "{STR_VAR_2}!\p"
	.string "Torna quando vuoi per un altro reroll!$"

Kaizo_Hub_Text_Ability_Goodbye:
	.string "Nessuna modifica all'Abilita'.\n"
	.string "Torna pure quando vuoi!$"

// ========================================================
// META PROGRESSION UPGRADE NPCS
// ========================================================

Kaizo_Hub_EventScript_MartUpgradesNPC::
	lock
	faceplayer
	special Script_Meta_ShowShardsWindow
	msgbox Kaizo_Hub_Text_MartUpgrades_Intro, MSGBOX_DEFAULT
Kaizo_Hub_EventScript_MartUpgrades_Menu::
	special Script_Meta_PushMartChoices
	dynmultistack 0, 0, FALSE, 7, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_MartUpgrades_Exit
	goto_if_eq VAR_RESULT, 6, Kaizo_Hub_EventScript_MartUpgrades_Exit
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Meta_BuyMartUpgrade
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_MartUpgrades_Success
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_MartUpgrades_NoShards
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_MartUpgrades_Maxed
	goto Kaizo_Hub_EventScript_MartUpgrades_Exit

Kaizo_Hub_EventScript_MartUpgrades_Success::
	playse SE_EXP
	special Script_Meta_ShowShardsWindow
	msgbox Kaizo_Hub_Text_UpgradeSuccess, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_MartUpgrades_Menu

Kaizo_Hub_EventScript_MartUpgrades_NoShards::
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_NotEnoughShards, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_MartUpgrades_Menu

Kaizo_Hub_EventScript_MartUpgrades_Maxed::
	msgbox Kaizo_Hub_Text_AlreadyMaxed, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_MartUpgrades_Menu

Kaizo_Hub_EventScript_MartUpgrades_Exit::
	special Script_Meta_HideShardsWindow
	msgbox Kaizo_Hub_Text_UpgradesGoodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_StarterUpgradesNPC::
	lock
	faceplayer
	special Script_Meta_ShowShardsWindow
	msgbox Kaizo_Hub_Text_StarterUpgrades_Intro, MSGBOX_DEFAULT
Kaizo_Hub_EventScript_StarterUpgrades_Menu::
	special Script_Meta_PushStarterChoices
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_StarterUpgrades_Exit
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_StarterUpgrades_Exit
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Meta_BuyStarterUpgrade
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_StarterUpgrades_Success
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_StarterUpgrades_NoShards
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_StarterUpgrades_Maxed
	goto Kaizo_Hub_EventScript_StarterUpgrades_Exit

Kaizo_Hub_EventScript_StarterUpgrades_Success::
	playse SE_EXP
	special Script_Meta_ShowShardsWindow
	msgbox Kaizo_Hub_Text_UpgradeSuccess, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_StarterUpgrades_Menu

Kaizo_Hub_EventScript_StarterUpgrades_NoShards::
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_NotEnoughShards, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_StarterUpgrades_Menu

Kaizo_Hub_EventScript_StarterUpgrades_Maxed::
	msgbox Kaizo_Hub_Text_AlreadyMaxed, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_StarterUpgrades_Menu

Kaizo_Hub_EventScript_StarterUpgrades_Exit::
	special Script_Meta_HideShardsWindow
	msgbox Kaizo_Hub_Text_UpgradesGoodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_EventScript_FortuneUpgradesNPC::
	lock
	faceplayer
	special Script_Meta_ShowShardsWindow
	msgbox Kaizo_Hub_Text_FortuneUpgrades_Intro, MSGBOX_DEFAULT
Kaizo_Hub_EventScript_FortuneUpgrades_Menu::
	special Script_Meta_PushFortuneChoices
	dynmultistack 0, 0, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_FortuneUpgrades_Exit
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_FortuneUpgrades_Exit
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Meta_BuyFortuneUpgrade
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_FortuneUpgrades_Success
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_FortuneUpgrades_NoShards
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_FortuneUpgrades_Maxed
	goto Kaizo_Hub_EventScript_FortuneUpgrades_Exit

Kaizo_Hub_EventScript_FortuneUpgrades_Success::
	playse SE_EXP
	special Script_Meta_ShowShardsWindow
	msgbox Kaizo_Hub_Text_UpgradeSuccess, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_FortuneUpgrades_Menu

Kaizo_Hub_EventScript_FortuneUpgrades_NoShards::
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_NotEnoughShards, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_FortuneUpgrades_Menu

Kaizo_Hub_EventScript_FortuneUpgrades_Maxed::
	msgbox Kaizo_Hub_Text_AlreadyMaxed, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_FortuneUpgrades_Menu

Kaizo_Hub_EventScript_FortuneUpgrades_Exit::
	special Script_Meta_HideShardsWindow
	msgbox Kaizo_Hub_Text_UpgradesGoodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_RunRecap:
	.string "RESOCONTO ULTIMA RUN\p"
	.string "Livello raggiunto: {STR_VAR_1}\n"
	.string "Medaglie ottenute: {STR_VAR_2}\p"
	.string "Ricompense: {STR_VAR_3}!\n"
	.string "Ben tornato nell'HUB!$"

Kaizo_Hub_Text_StartingWithGachaStarter:
	.string "Preparati!\p"
	.string "Inizierai la run con il tuo Starter\n"
	.string "del Roster: {STR_VAR_1} (Liv. 5)!\p"
	.string "La selezione da Oak verra' saltata\n"
	.string "e potrai andare subito all'avventura!$"

Kaizo_Hub_Text_CheatGachaDone:
	.string "Hai ottenuto +10 Biglietti Gacha\n"
	.string "e +50 Punti per ogni rarita'!$"

Kaizo_Hub_Text_MartUpgrades_Intro:
	.string "Benvenuto! Posso potenziare i servizi\n"
	.string "offerti nei Poke Mart durante le tue run.\p"
	.string "Usa i Frammenti Kaizo per sbloccare\n"
	.string "licenze e aumentare le scelte di draft!$"

Kaizo_Hub_Text_StarterUpgrades_Intro:
	.string "Saluti! Studio il potenziale genetico\n"
	.string "dei Pokemon iniziali.\p"
	.string "Con i Frammenti Kaizo posso sbloccarti\n"
	.string "reroll, mosse migliori e strumenti!$"

Kaizo_Hub_Text_FortuneUpgrades_Intro:
	.string "La fortuna aiuta gli audaci!\p"
	.string "Investi Frammenti Kaizo per aumentare\n"
	.string "la probabilita' di trovare drop Rari\l"
	.string "ed Epici durante l'avventura!$"

Kaizo_Hub_Text_UpgradeSuccess:
	.string "Potenziamento acquistato con successo!\n"
	.string "Sara' attivo a partire dalla prossima run.$"

Kaizo_Hub_Text_NotEnoughShards:
	.string "Non hai abbastanza Frammenti Kaizo\n"
	.string "per questo potenziamento!$"

Kaizo_Hub_Text_AlreadyMaxed:
	.string "Questo potenziamento e' gia' al livello\n"
	.string "massimo o gia' sbloccato!$"

Kaizo_Hub_Text_UpgradesGoodbye:
	.string "Torna quando vuoi per altri\n"
	.string "potenziamenti!$"

// ========================================================
// GACHA MACHINE & ROSTER BUILD SYSTEM
// ========================================================

Kaizo_Hub_EventScript_GachaMachine::
	lock
	faceplayer
	playse SE_PIN
	special Script_Gacha_GetTicketsAndPoints
	msgbox Kaizo_Hub_Text_GachaIntro, MSGBOX_DEFAULT
Kaizo_Hub_EventScript_GachaMenu::
	dynmultipush Kaizo_Hub_Text_GachaOpt_Pull, 0
	dynmultipush Kaizo_Hub_Text_GachaOpt_Roster, 1
	dynmultipush Kaizo_Hub_Text_GachaOpt_Info, 2
	dynmultipush Kaizo_Hub_Text_GachaOpt_Exit, 3
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_GachaExit
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_GachaExit
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_GachaPull
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_GachaRoster
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_GachaInfo
	goto Kaizo_Hub_EventScript_GachaExit

Kaizo_Hub_EventScript_GachaInfo::
	msgbox Kaizo_Hub_Text_GachaInfoText, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_GachaPull::
	special Script_Gacha_GetTicketsAndPoints
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_GachaNoTickets
	special OpenGachaSlotMachine
	special Script_Gacha_GetTicketsAndPoints
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_GachaNoTickets::
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_GachaNoTickets, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_GachaBoxFull::
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_GachaBoxFull, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_GachaNewMon::
	playfanfare MUS_LEVEL_UP
	msgbox Kaizo_Hub_Text_GachaNewHatched, MSGBOX_DEFAULT
	waitfanfare
	msgbox Kaizo_Hub_Text_GachaAddedToRoster, MSGBOX_DEFAULT
	special Script_Gacha_GetTicketsAndPoints
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_GachaDuplicateMon::
	playfanfare MUS_OBTAIN_ITEM
	msgbox Kaizo_Hub_Text_GachaDuplicateHatched, MSGBOX_DEFAULT
	waitfanfare
	msgbox Kaizo_Hub_Text_GachaDuplicateStats, MSGBOX_DEFAULT
	dynmultipush Kaizo_Hub_Text_GachaDup_KeepNew, 0
	dynmultipush Kaizo_Hub_Text_GachaDup_KeepOld, 1
	dynmultistack 0, 0, FALSE, 2, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Gacha_HandleDuplicate
	playse SE_EXP
	msgbox Kaizo_Hub_Text_GachaFusedSuccess, MSGBOX_DEFAULT
	special Script_Gacha_GetTicketsAndPoints
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_GachaRoster::
	special OpenGachaRosterPC
	special Script_Gacha_GetTicketsAndPoints
	goto Kaizo_Hub_EventScript_GachaMenu
Kaizo_Hub_EventScript_GachaMonBuildMenu::
	msgbox Kaizo_Hub_Text_GachaMonDetails, MSGBOX_DEFAULT
	special Script_Gacha_PushBuildOptions
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_GachaRoster
	goto_if_eq VAR_RESULT, 5, Kaizo_Hub_EventScript_GachaRoster
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_BuildToggleStarter
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_BuildNature
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_BuildAbility
	goto_if_eq VAR_RESULT, 3, Kaizo_Hub_EventScript_BuildMove
	goto_if_eq VAR_RESULT, 4, Kaizo_Hub_EventScript_BuildFuseMon
	goto Kaizo_Hub_EventScript_GachaRoster

Kaizo_Hub_EventScript_GachaRosterEmpty::
	msgbox Kaizo_Hub_Text_GachaRosterEmpty, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaMenu

Kaizo_Hub_EventScript_BuildToggleStarter::
	special Script_Gacha_ToggleStarter
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_StarterSet
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_StarterRemoved, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu
Kaizo_Hub_EventScript_StarterSet::
	playfanfare MUS_LEVEL_UP
	msgbox Kaizo_Hub_Text_StarterSetSuccess, MSGBOX_DEFAULT
	waitfanfare
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu

Kaizo_Hub_EventScript_BuildNature::
	special Script_Gacha_PushNatureList
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_BuildNoPoints
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_GachaMonBuildMenu
	goto_if_eq VAR_RESULT, 0xFF, Kaizo_Hub_EventScript_GachaMonBuildMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Gacha_ApplyNature
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_BuildNatureSuccess
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu
Kaizo_Hub_EventScript_BuildNatureSuccess::
	playse SE_EXP
	msgbox Kaizo_Hub_Text_BuildNatureSuccess, MSGBOX_DEFAULT
	special Script_Gacha_SelectRosterMon
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu

Kaizo_Hub_EventScript_BuildAbility::
	special Script_Gacha_PushAbilityList
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_BuildNoPoints
	dynmultistack 0, 0, FALSE, 4, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_GachaMonBuildMenu
	goto_if_eq VAR_RESULT, 0xFF, Kaizo_Hub_EventScript_GachaMonBuildMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Gacha_ApplyAbility
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_BuildAbilitySuccess
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu
Kaizo_Hub_EventScript_BuildAbilitySuccess::
	playse SE_EXP
	msgbox Kaizo_Hub_Text_BuildAbilitySuccess, MSGBOX_DEFAULT
	special Script_Gacha_SelectRosterMon
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu

Kaizo_Hub_EventScript_BuildMove::
	special Script_Gacha_PushMoveChoices
	goto_if_eq VAR_RESULT, 2, Kaizo_Hub_EventScript_BuildNoPoints
	dynmultistack 0, 0, FALSE, 5, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Kaizo_Hub_EventScript_GachaMonBuildMenu
	goto_if_eq VAR_RESULT, 0, Kaizo_Hub_EventScript_GachaMonBuildMenu
	copyvar VAR_0x8005, VAR_RESULT
	special Script_Gacha_ApplyMove
	goto_if_eq VAR_RESULT, 1, Kaizo_Hub_EventScript_BuildMoveSuccess
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu
Kaizo_Hub_EventScript_BuildMoveSuccess::
	playfanfare MUS_LEVEL_UP
	msgbox Kaizo_Hub_Text_BuildMoveSuccess, MSGBOX_DEFAULT
	waitfanfare
	special Script_Gacha_SelectRosterMon
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu

Kaizo_Hub_EventScript_BuildFuseMon::
	msgbox Kaizo_Hub_Text_BuildConfirmFuse, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Kaizo_Hub_EventScript_GachaMonBuildMenu
	special Script_Gacha_FuseSelectedMon
	playse SE_EXP
	msgbox Kaizo_Hub_Text_BuildFuseDone, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaRoster

Kaizo_Hub_EventScript_BuildNoPoints::
	playse SE_NOT_EFFECTIVE
	msgbox Kaizo_Hub_Text_BuildNotEnoughPoints, MSGBOX_DEFAULT
	goto Kaizo_Hub_EventScript_GachaMonBuildMenu

Kaizo_Hub_EventScript_GachaExit::
	msgbox Kaizo_Hub_Text_GachaGoodbye, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Text_GachaIntro:
	.string "DISTRIBUTORE GACHA UOVA\p"
	.string "Biglietti Gacha posseduti: {STR_VAR_1}\p"
	.string "Punti Rarita' accumulati:\n"
	.string "{STR_VAR_2}\n"
	.string "{STR_VAR_3}$"

Kaizo_Hub_Text_GachaOpt_Pull:
	.string "Schiudi Uovo (Slot Machine)$"

Kaizo_Hub_Text_GachaOpt_Roster:
	.string "PC Roster & Starter Run$"

Kaizo_Hub_Text_GachaOpt_Info:
	.string "Info Regole Gacha$"

Kaizo_Hub_Text_GachaOpt_Exit:
	.string "Esci$"

Kaizo_Hub_Text_GachaInfoText:
	.string "COME FUNZIONA IL GACHA:\p"
	.string "1. Ottieni Biglietti sconfiggendo i\n"
	.string "Capipalestra durante le run.\p"
	.string "2. Le uova schiudono Pokemon base a\n"
	.string "Liv. 5 con IV, Natura e Abilita' random\n"
	.string "e mossa iniziale Scratch.\p"
	.string "3. Se trovi un duplicato, puoi scegliere\n"
	.string "se tenere il nuovo o il vecchio:\n"
	.string "quello fuso ti dara' 10 Punti Rarita'!\p"
	.string "4. Spendi i Punti Rarita' per buildare\n"
	.string "la Natura (5 pt), l'Abilita' (10 pt)\n"
	.string "o la Mossa iniziale (10 pt) di qualsiasi\n"
	.string "Pokemon della stessa rarita'!\p"
	.string "5. Imposta un Pokemon come Starter per\n"
	.string "portarlo direttamente nella run!$"

Kaizo_Hub_Text_GachaNoTickets:
	.string "Non hai abbastanza Biglietti Gacha!\n"
	.string "Vincili battendo i Capipalestra nelle run!$"

Kaizo_Hub_Text_GachaBoxFull:
	.string "Il tuo Roster e' pieno (max 30 Pokemon)!\n"
	.string "Fondi prima un Pokemon dal menu Roster.$"

Kaizo_Hub_Text_GachaNewHatched:
	.string "Dall'Uovo {STR_VAR_2} e' nato\n"
	.string "{STR_VAR_1} (Liv. 5)!\p"
	.string "Natura e Abilita':\n"
	.string "{STR_VAR_3}\n"
	.string "Mossa Iniziale: Scratch$"

Kaizo_Hub_Text_GachaAddedToRoster:
	.string "{STR_VAR_1} e' stato aggiunto al tuo\n"
	.string "Roster dell'Hub!$"

Kaizo_Hub_Text_GachaDuplicateHatched:
	.string "Dall'Uovo e' nato un {STR_VAR_1} duplicato!$"

Kaizo_Hub_Text_GachaDuplicateStats:
	.string "Confronto statistiche:\p"
	.string "Vecchio {STR_VAR_1} nel Roster:\n"
	.string "{STR_VAR_2}\p"
	.string "Nuovo {STR_VAR_1} schiuso:\n"
	.string "{STR_VAR_3}\p"
	.string "Quale vuoi tenere? L'altro verra' fuso\n"
	.string "in +10 Punti Rarita'!$"

Kaizo_Hub_Text_GachaDup_KeepNew:
	.string "Tieni il NUOVO (Fondi il Vecchio)$"

Kaizo_Hub_Text_GachaDup_KeepOld:
	.string "Tieni il VECCHIO (Fondi il Nuovo)$"

Kaizo_Hub_Text_GachaFusedSuccess:
	.string "Hai fuso {STR_VAR_1}!\p"
	.string "Ottenuti: +10 Punti {STR_VAR_2}!\n"
	.string "Nuovo totale: {STR_VAR_3} Punti.$"

Kaizo_Hub_Text_GachaRosterEmpty:
	.string "Non possiedi nessun Pokemon nel Roster!\n"
	.string "Tira un uovo per ottenere il tuo primo mon.$"

Kaizo_Hub_Text_GachaMonDetails:
	.string "POKEMON: {STR_VAR_1}\n"
	.string "Natura e Abilita': {STR_VAR_2}\n"
	.string "Mossa: {STR_VAR_3}$"

Kaizo_Hub_Text_StarterSetSuccess:
	.string "{STR_VAR_1} e' ora il tuo Starter attivo!\p"
	.string "Quando partirai per una nuova run,\n"
	.string "inizierai direttamente con {STR_VAR_1}!$"

Kaizo_Hub_Text_StarterRemoved:
	.string "{STR_VAR_1} non e' piu' lo Starter attivo.\n"
	.string "Inizierai le run con la scelta da Oak.$"

Kaizo_Hub_Text_BuildNatureSuccess:
	.string "La natura di {STR_VAR_1} e' ora {STR_VAR_2}!\n"
	.string "Punti rimanenti: {STR_VAR_3}.$"

Kaizo_Hub_Text_BuildAbilitySuccess:
	.string "L'abilita' di {STR_VAR_1} e' ora {STR_VAR_2}!\n"
	.string "Punti rimanenti: {STR_VAR_3}.$"

Kaizo_Hub_Text_BuildMoveSuccess:
	.string "{STR_VAR_1} ha imparato {STR_VAR_2}\n"
	.string "come mossa iniziale di partenza!\p"
	.string "Punti rimanenti: {STR_VAR_3}.$"

Kaizo_Hub_Text_BuildConfirmFuse:
	.string "Sei sicuro di voler fondere {STR_VAR_1}?\n"
	.string "Verra' rimosso dal tuo Roster e riceverai\l"
	.string "+10 Punti di questa rarita'!$"

Kaizo_Hub_Text_BuildFuseDone:
	.string "Hai fuso {STR_VAR_1}!\p"
	.string "Ottenuti: +10 Punti {STR_VAR_2}!\n"
	.string "Totale punti: {STR_VAR_3}.$"

Kaizo_Hub_Text_BuildNotEnoughPoints:
	.string "Non hai abbastanza Punti Rarita' di questa\n"
	.string "categoria per effettuare la modifica!$"

Kaizo_Hub_Text_GachaGoodbye:
	.string "Torna presto a trovarmi!$"



