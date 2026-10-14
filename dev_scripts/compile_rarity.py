#!/usr/bin/env python3
"""
compile_rarity.py - Compiles data/roguelike_rarity.json into src/data/roguelike_rarity.h
This is invoked during build whenever data/roguelike_rarity.json changes.
"""

import os
import sys
import json

def main():
    if len(sys.argv) < 3:
        repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
        input_json = os.path.join(repo_root, 'data', 'roguelike_rarity.json')
        output_header = os.path.join(repo_root, 'src', 'data', 'roguelike_rarity.h')
    else:
        input_json = sys.argv[1]
        output_header = sys.argv[2]

    if not os.path.exists(input_json):
        print(f"Error: {input_json} does not exist.")
        sys.exit(1)

    with open(input_json, 'r', encoding='utf-8') as f:
        data = json.load(f)

    lines = []
    lines.append("//")
    lines.append("// DO NOT MODIFY THIS FILE DIRECTLY!")
    lines.append(f"// It is auto-generated from {os.path.relpath(input_json)} by dev_scripts/compile_rarity.py")
    lines.append("// Modify data/roguelike_rarity.json instead to change tiers or pools.")
    lines.append("//")
    lines.append("#ifndef GUARD_DATA_ROGUELIKE_RARITY_H")
    lines.append("#define GUARD_DATA_ROGUELIKE_RARITY_H")
    lines.append("")
    lines.append('#include "constants/species.h"')
    lines.append('#include "constants/moves.h"')
    lines.append('#include "constants/items.h"')
    lines.append('#include "roguelike_rarity.h"')
    lines.append("")

    categories = [
        ("Pokemon", "pokemon", "sRarityPokemon", "NUM_SPECIES"),
        ("Moves", "moves", "sRarityMoves", "MOVES_COUNT_ALL"),
        ("Items", "items", "sRarityItems", "ITEMS_COUNT")
    ]

    tier_names = ["COMMON", "UNCOMMON", "RARE", "EPIC"]

    for title, cat_key, var_prefix, max_const in categories:
        cat_dict = data.get(cat_key, {})
        all_elements = []

        lines.append(f"// =============================================================")
        lines.append(f"// {title.upper()} RARITY POOLS")
        lines.append(f"// =============================================================")

        ALIASES = {
            "ITEM_POKEBALL": "ITEM_POKE_BALL",
            "MOVE_MEGA_HORN": "MOVE_MEGAHORN",
        }

        item_to_tier = {}

        for t in tier_names:
            raw_items = cat_dict.get(t, [])
            items = []
            for item in raw_items:
                item = ALIASES.get(item, item)
                if item not in items:
                    items.append(item)
                item_to_tier[item] = t

            all_elements.extend(items)
            lines.append(f"static const u16 {var_prefix}_{t.capitalize()}[] = {{")
            for item in items:
                lines.append(f"    {item},")
            lines.append("};")
            lines.append("")

        # Combined "All" pool
        lines.append(f"static const u16 {var_prefix}_All[] = {{")
        for item in all_elements:
            lines.append(f"    {item},")
        lines.append("};")
        lines.append("")

        # Fast lookup array: id -> tier (0 = RARITY_NONE by default)
        lines.append(f"static const u8 sRarityTierTable_{title}[{max_const}] = {{")
        for item, t in item_to_tier.items():
            lines.append(f"    [{item}] = RARITY_{t},")
        lines.append("};")
        lines.append("")

    lines.append("#endif // GUARD_DATA_ROGUELIKE_RARITY_H\n")

    os.makedirs(os.path.dirname(output_header), exist_ok=True)
    with open(output_header, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))

    print(f"Compiled {input_json} -> {output_header}")

if __name__ == '__main__':
    main()
