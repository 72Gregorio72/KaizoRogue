Kaizo_Hub_Mart_MapScripts::
	.byte 0

Kaizo_Hub_Mart_EventScript_Assistant::
	lock
	faceplayer
	msgbox Kaizo_Hub_Mart_Text_Assistant, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Mart_EventScript_Noticeboard::
	msgbox Kaizo_Hub_Mart_Text_Noticeboard, MSGBOX_SIGN
	end

Kaizo_Hub_Mart_Text_Assistant:
	.string "Benvenuto al Dojo dei Poke Mart!\p"
	.string "Il Maestro delle Arti qui presente\n"
	.string "ti concedera' licenze speciali per\l"
	.string "trasformare ogni minimarket della\l"
	.string "regione nel tuo centro di potenziamento!\p"
	.string "Piu' medaglie ottieni, piu' punti\n"
	.string "statistiche potrai distribuire!$"

Kaizo_Hub_Mart_Text_Noticeboard:
	.string "GUIDA AI POTENZIAMENTI POKé MART\p"
	.string "- DRAFT MOSSE: Aumenta il numero di\n"
	.string "  mosse tra cui scegliere (fino a 5).\p"
	.string "- STAT POINTS: Aumenta i punti BST\n"
	.string "  guadagnati per ogni medaglia.\p"
	.string "- LICENZE: Sbloccano IV Booster, EV\n"
	.string "  Trainer, Cambio Natura e Abilita'!$"
