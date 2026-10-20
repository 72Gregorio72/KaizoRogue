#!/usr/bin/env python3
"""
Generates web/data/pokedex.json from game source files
"""
import os
import re
import json

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SPECIES_H = os.path.join(ROOT, "include", "constants", "species.h")
RARITY_H = os.path.join(ROOT, "src", "data", "roguelike_rarity.h")
OUTPUT_JSON = os.path.join(ROOT, "web", "data", "pokedex.json")

# Mapping of species constant to National Dex ID
def parse_species():
    species_map = {}
    with open(SPECIES_H, "r", encoding="utf-8") as f:
        content = f.read()

    pattern = re.compile(r"SPECIES_([A-Z0-9_]+)\s*=\s*(\d+)")
    for match in pattern.finditer(content):
        name = match.group(1)
        sid = int(match.group(2))
        if sid > 0:
            species_map[name] = sid
    return species_map

# Parse rarity table
def parse_rarities():
    rarity_map = {}
    with open(RARITY_H, "r", encoding="utf-8") as f:
        content = f.read()

    pattern = re.compile(r"\[SPECIES_([A-Z0-9_]+)\]\s*=\s*RARITY_([A-Z]+)")
    for match in pattern.finditer(content):
        name = match.group(1)
        tier = match.group(2).lower() # common, uncommon, rare, epic, legendary
        rarity_map[name] = tier
    return rarity_map

# Known mega evolutions
MEGA_DATA = {
    "VENUSAUR": [{"name": "Venusaurite", "id": 292}],
    "CHARIZARD": [{"name": "Charizardite X", "id": 293}, {"name": "Charizardite Y", "id": 294}],
    "BLASTOISE": [{"name": "Blastoisinite", "id": 295}],
    "ALAKAZAM": [{"name": "Alakazite", "id": 296}],
    "GENGAR": [{"name": "Gengarite", "id": 297}],
    "KANGASKHAN": [{"name": "Kangaskhanite", "id": 301}],
    "PINSIR": [{"name": "Pinsirite", "id": 302}],
    "GYARADOS": [{"name": "Gyaradosite", "id": 303}],
    "AERODACTYL": [{"name": "Aerodactylite", "id": 304}],
    "MEWTWO": [{"name": "Mewtwonite X", "id": 305}, {"name": "Mewtwonite Y", "id": 306}],
    "AMPHAROS": [{"name": "Ampharosite", "id": 307}],
    "STEELIX": [{"name": "Steelixite", "id": 308}],
    "SCIZOR": [{"name": "Scizorite", "id": 309}],
    "HERACROSS": [{"name": "Heracronite", "id": 310}],
    "HOUNDOOM": [{"name": "Houndoominite", "id": 311}],
    "TYRANITAR": [{"name": "Tyranitarite", "id": 312}],
    "SCEPTILE": [{"name": "Sceptilite", "id": 313}],
    "BLAZIKEN": [{"name": "Blazikenite", "id": 314}],
    "SWAMPERT": [{"name": "Swampertite", "id": 315}],
    "GARDEVOIR": [{"name": "Gardevoirite", "id": 316}],
    "SABLEYE": [{"name": "Sablenite", "id": 317}],
    "MAWILE": [{"name": "Mawilite", "id": 318}],
    "AGGRON": [{"name": "Aggronite", "id": 319}],
    "MEDICHAM": [{"name": "Medichamite", "id": 320}],
    "MANECTRIC": [{"name": "Manectite", "id": 321}],
    "SHARPEDO": [{"name": "Sharpedonite", "id": 322}],
    "CAMERUPT": [{"name": "Cameruptite", "id": 323}],
    "ALTARIA": [{"name": "Altarianite", "id": 324}],
    "BANETTE": [{"name": "Banettite", "id": 325}],
    "ABSOL": [{"name": "Absolite", "id": 326}],
    "GLALIE": [{"name": "Glalitite", "id": 327}],
    "SALAMENCE": [{"name": "Salamencite", "id": 328}],
    "METAGROSS": [{"name": "Metagrossite", "id": 329}],
    "LATIAS": [{"name": "Latiasite", "id": 330}],
    "LATIOS": [{"name": "Latiosite", "id": 331}],
    "LOPUNNY": [{"name": "Lopunnite", "id": 332}],
    "GARCHOMP": [{"name": "Garchompite", "id": 333}],
    "LUCARIO": [{"name": "Lucarionite", "id": 334}],
    "ABOMASNOW": [{"name": "Abomasite", "id": 335}],
    "GALLADE": [{"name": "Galladite", "id": 336}],
    "AUDINO": [{"name": "Audinite", "id": 337}],
    "DIANCIE": [{"name": "Diancite", "id": 338}],
    "RAYQUAZA": [{"name": "Dragon Ascent (Mega)", "id": 0}]
}

def get_gen_and_region(dex_id):
    if dex_id <= 151: return 1, "Kanto"
    if dex_id <= 251: return 2, "Johto"
    if dex_id <= 386: return 3, "Hoenn"
    if dex_id <= 493: return 4, "Sinnoh"
    if dex_id <= 649: return 5, "Unova"
    if dex_id <= 721: return 6, "Kalos"
    if dex_id <= 809: return 7, "Alola"
    if dex_id <= 905: return 8, "Galar"
    return 9, "Paldea"

def clean_name(species_key):
    # Formats BULBASAUR -> Bulbasaur, NIDORAN_F -> Nidoran♀, etc.
    if species_key == "NIDORAN_F":
        return "Nidoran♀"
    if species_key == "NIDORAN_M":
        return "Nidoran♂"
    parts = species_key.split("_")
    return " ".join(p.capitalize() for p in parts)

def main():
    os.makedirs(os.path.dirname(OUTPUT_JSON), exist_ok=True)
    species_map = parse_species()
    rarity_map = parse_rarities()

    entries = []
    for sp_key, tier in rarity_map.items():
        if sp_key not in species_map:
            continue
        sid = species_map[sp_key]
        name = clean_name(sp_key)
        gen, region = get_gen_and_region(sid)

        megas = MEGA_DATA.get(sp_key, [])
        # Check evolutionary family for megas (e.g. Charmander -> Charizard)
        if not megas:
            for final_form, stones in MEGA_DATA.items():
                if sp_key in final_form or (sp_key.startswith("CHAR") and final_form == "CHARIZARD") or \
                   (sp_key.startswith("BULBA") and final_form == "VENUSAUR") or \
                   (sp_key.startswith("SQUIRT") and final_form == "BLASTOISE") or \
                   (sp_key.startswith("TREE") and final_form == "SCEPTILE") or \
                   (sp_key.startswith("TORCH") and final_form == "BLAZIKEN") or \
                   (sp_key.startswith("MUDK") and final_form == "SWAMPERT") or \
                   (sp_key.startswith("RIOLU") and final_form == "LUCARIO") or \
                   (sp_key.startswith("GIBLE") and final_form == "GARCHOMP") or \
                   (sp_key.startswith("BAGON") and final_form == "SALAMENCE") or \
                   (sp_key.startswith("BELDUM") and final_form == "METAGROSS") or \
                   (sp_key.startswith("RALTS") and final_form in ["GARDEVOIR", "GALLADE"]) or \
                   (sp_key.startswith("GASTLY") and final_form == "GENGAR") or \
                   (sp_key.startswith("ABRA") and final_form == "ALAKAZAM"):
                    megas = stones
                    break

        entries.append({
            "id": sid,
            "key": sp_key,
            "name": name,
            "gen": gen,
            "region": region,
            "tier": tier,
            "hasMega": len(megas) > 0,
            "megaStones": megas
        })

    entries.sort(key=lambda x: x["id"])

    with open(OUTPUT_JSON, "w", encoding="utf-8") as f:
        json.dump(entries, f, indent=2)

    print(f"Esportate {len(entries)} specie in {OUTPUT_JSON}")

if __name__ == "__main__":
    main()
