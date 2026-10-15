Kaizo_Hub_Starter_MapScripts::
	.byte 0

Kaizo_Hub_Starter_EventScript_Assistant::
	lock
	faceplayer
	msgbox Kaizo_Hub_Starter_Text_Assistant, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Starter_EventScript_Computer::
	msgbox Kaizo_Hub_Starter_Text_Computer, MSGBOX_SIGN
	end

Kaizo_Hub_Starter_EventScript_DnaTank::
	msgbox Kaizo_Hub_Starter_Text_DnaTank, MSGBOX_SIGN
	end

Kaizo_Hub_Starter_Text_Assistant:
	.string "Questo e' il Laboratorio di Genetica\n"
	.string "degli Starter!\p"
	.string "Analizziamo il codice dei Pokémon\n"
	.string "offerti all'inizio dell'avventura.\p"
	.string "Parla con il Capo Ricercatore per\n"
	.string "sbloccare reroll e mosse di qualita'!$"

Kaizo_Hub_Starter_Text_Computer:
	.string "DATI RICERCA STARTER\p"
	.string "Livello Reroll sbloccato:\n"
	.string "Permette di scegliere i tipi da Oak\l"
	.string "e rimescolare i Pokémon sul tavolo!\p"
	.string "Livello Mosse:\n"
	.string "Garantisce mosse Non Comuni o Rare\l"
	.string "durante il draft iniziale!$"

Kaizo_Hub_Starter_Text_DnaTank:
	.string "Una capsula di stasi contenente dati\n"
	.string "genetici ad alta concentrazione!\p"
	.string "Emana un'energia misteriosa...$"
