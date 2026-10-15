Kaizo_Hub_Fortune_MapScripts::
	.byte 0

Kaizo_Hub_Fortune_EventScript_Apprentice::
	lock
	faceplayer
	msgbox Kaizo_Hub_Fortune_Text_Apprentice, MSGBOX_DEFAULT
	release
	end

Kaizo_Hub_Fortune_EventScript_ShrineSign::
	msgbox Kaizo_Hub_Fortune_Text_ShrineSign, MSGBOX_SIGN
	end

Kaizo_Hub_Fortune_Text_Apprentice:
	.string "Questo e' il Tempio della Sorte!\p"
	.string "Il Maestro Gentiluomo sa come canalizzare\n"
	.string "l'energia dei Frammenti Kaizo per piegare\l"
	.string "la probabilita' a tuo favore!\p"
	.string "Se vuoi trovare strumenti e mostri rari\n"
	.string "durante il cammino, sei nel posto giusto!$"

Kaizo_Hub_Fortune_Text_ShrineSign:
	.string "SANTUARIO DELLA FORTUNA\p"
	.string "Aumentando il livello di Fortuna Drop,\n"
	.string "le probabilita' di estrarre ricompense\l"
	.string "Comuni si riducono, lasciando spazio\l"
	.string "a ricompense Rare ed Epiche!$"
