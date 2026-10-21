Hub_Mart_MapScripts::
	.byte 0

Hub_Mart_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Kaizo_Hub_ShopItems
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Hub_Mart_EventScript_MartUpgrades::
	goto Kaizo_Hub_EventScript_MartUpgradesNPC

Hub_Mart_EventScript_StarterUpgrades::
	goto Kaizo_Hub_EventScript_StarterUpgradesNPC

Hub_Mart_EventScript_FortuneUpgrades::
	goto Kaizo_Hub_EventScript_FortuneUpgradesNPC

Hub_Mart_EventScript_IVBooster::
	goto Kaizo_Hub_EventScript_IVBoosterNPC

Hub_Mart_EventScript_EVManager::
	goto Kaizo_Hub_EventScript_EVManagerNPC

Hub_Mart_EventScript_NatureChanger::
	goto Kaizo_Hub_EventScript_NatureChangerNPC

Hub_Mart_EventScript_AbilityChanger::
	goto Kaizo_Hub_EventScript_AbilityChangerNPC

Hub_Mart_EventScript_ExpMultiplier::
	goto Kaizo_Hub_EventScript_ExpMultiplierNPC

Hub_Mart_EventScript_BuildMaster::
	goto Kaizo_Hub_EventScript_BuildNPC

Hub_Mart_EventScript_CheatNPC::
	goto Kaizo_Hub_EventScript_CheatNPC

Hub_Mart_EventScript_PC::
	goto Kaizo_Hub_EventScript_PC
