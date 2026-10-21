Hub_GameCorner_MapScripts::
	.byte 0

Hub_GameCorner_EventScript_SlotMachine::
	lockall
	special Script_Gacha_GetTicketsAndPoints
	goto_if_eq VAR_RESULT, 0, Hub_GameCorner_EventScript_SlotMachine_NoTickets
	msgbox Hub_GameCorner_Text_SelectBanner, MSGBOX_DEFAULT
	dynmultipush Hub_GameCorner_Text_BannerAll, 0
	dynmultipush Hub_GameCorner_Text_BannerGen1, 1
	dynmultipush Hub_GameCorner_Text_BannerGen2, 2
	dynmultipush Hub_GameCorner_Text_BannerGen3, 3
	dynmultipush Hub_GameCorner_Text_BannerGen4, 4
	dynmultipush Hub_GameCorner_Text_BannerGen5, 5
	dynmultipush Hub_GameCorner_Text_BannerGen6, 6
	dynmultipush Hub_GameCorner_Text_BannerGen7, 7
	dynmultipush Hub_GameCorner_Text_BannerGen8, 8
	dynmultipush Hub_GameCorner_Text_BannerGen9, 9
	dynmultipush Hub_GameCorner_Text_Cancel, 10
	dynmultistack 0, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Hub_GameCorner_EventScript_SlotMachine_Exit
	goto_if_eq VAR_RESULT, 10, Hub_GameCorner_EventScript_SlotMachine_Exit
	copyvar VAR_0x8004, VAR_RESULT
	special Script_Gacha_GetTicketsAndPoints
	goto_if_lt VAR_RESULT, 10, Hub_GameCorner_EventScript_SlotMachine_Single
	msgbox Hub_GameCorner_Text_SelectPullCount, MSGBOX_DEFAULT
	dynmultipush Hub_GameCorner_Text_PullSingle, 0
	dynmultipush Hub_GameCorner_Text_PullMulti10, 1
	dynmultipush Hub_GameCorner_Text_Cancel, 2
	dynmultistack 0, 0, FALSE, 3, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, Hub_GameCorner_EventScript_SlotMachine_Exit
	goto_if_eq VAR_RESULT, 2, Hub_GameCorner_EventScript_SlotMachine_Exit
	goto_if_eq VAR_RESULT, 1, Hub_GameCorner_EventScript_SlotMachine_Multi10
Hub_GameCorner_EventScript_SlotMachine_Single::
	setvar VAR_0x8005, 0
	goto Hub_GameCorner_EventScript_SlotMachine_Start
Hub_GameCorner_EventScript_SlotMachine_Multi10::
	setvar VAR_0x8005, 1
Hub_GameCorner_EventScript_SlotMachine_Start::
	special Script_Gacha_SetBanner
	special OpenGachaSlotMachine
	releaseall
	end

Hub_GameCorner_EventScript_SlotMachine_NoTickets::
	playse SE_BOO
	msgbox Hub_GameCorner_Text_NoTickets, MSGBOX_DEFAULT
	releaseall
	end

Hub_GameCorner_EventScript_SlotMachine_Exit::
	releaseall
	end

Hub_GameCorner_EventScript_Clerk1::
	lock
	faceplayer
	msgbox Hub_GameCorner_Text_ClerkRosterAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Hub_GameCorner_EventScript_OpenRoster
	msgbox Hub_GameCorner_Text_ClerkGoodbye, MSGBOX_DEFAULT
	release
	end

Hub_GameCorner_EventScript_OpenRoster::
	special OpenGachaRosterPC
	release
	end

Hub_GameCorner_EventScript_Clerk2::
	lock
	faceplayer
	msgbox Hub_GameCorner_Text_ClerkInfo, MSGBOX_DEFAULT
	release
	end

Hub_GameCorner_Text_SelectBanner:
	.string "SLOT GACHA MACHINE\n"
	.string "Biglietti Gacha posseduti: {STR_VAR_1}\p"
	.string "Scegli il Banner su cui tentare la sorte:\n"
	.string "Schiuderai solo Pokémon di quella Gen!$"

Hub_GameCorner_Text_BannerAll:
	.string "Tutte le Generazioni$"

Hub_GameCorner_Text_BannerGen1:
	.string "Gen 1 (Kanto)$"

Hub_GameCorner_Text_BannerGen2:
	.string "Gen 2 (Johto)$"

Hub_GameCorner_Text_BannerGen3:
	.string "Gen 3 (Hoenn)$"

Hub_GameCorner_Text_BannerGen4:
	.string "Gen 4 (Sinnoh)$"

Hub_GameCorner_Text_BannerGen5:
	.string "Gen 5 (Unova)$"

Hub_GameCorner_Text_BannerGen6:
	.string "Gen 6 (Kalos)$"

Hub_GameCorner_Text_BannerGen7:
	.string "Gen 7 (Alola)$"

Hub_GameCorner_Text_BannerGen8:
	.string "Gen 8 (Galar)$"

Hub_GameCorner_Text_BannerGen9:
	.string "Gen 9 (Paldea)$"

Hub_GameCorner_Text_Cancel:
	.string "Annulla$"

Hub_GameCorner_Text_SelectPullCount:
	.string "Scegli quanti roll effettuare:\p"
	.string "Roll Singolo: 1 Biglietto\n"
	.string "Multi Roll 10x: 10 Biglietti$"

Hub_GameCorner_Text_PullSingle:
	.string "Roll Singolo (1 Biglietto)$"

Hub_GameCorner_Text_PullMulti10:
	.string "Roll 10x (10 Biglietti)$"

Hub_GameCorner_Text_NoTickets:
	.string "Non hai abbastanza Biglietti Gacha!\n"
	.string "Completa run o missioni per trovarne altri.$"

Hub_GameCorner_Text_ClerkRosterAsk:
	.string "Benvenuto al Gacha Corner!\p"
	.string "Vuoi accedere al PC Roster per gestire\n"
	.string "i tuoi Pokémon sbloccati e lo Starter?$"

Hub_GameCorner_Text_ClerkGoodbye:
	.string "Buona fortuna con le Slot Machine!$"

Hub_GameCorner_Text_ClerkInfo:
	.string "COME FUNZIONANO LE GACHA SLOT:\p"
	.string "1. Avvicinati a una qualsiasi Slot Machine\n"
	.string "e premi A per iniziare.\p"
	.string "2. Puoi scegliere il Banner della tua\n"
	.string "generazione preferita o tutte le Gen.\p"
	.string "3. Puoi trovare Pokémon Comuni, Non Comuni,\n"
	.string "Rari, Epici, Leggendari e Shiny!\p"
	.string "4. I duplicati si fondono migliorando\n"
	.string "automaticamente i loro IV massimi!$"
