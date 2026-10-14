#!/usr/bin/env python3
"""
check_rarity.py - Reads data/roguelike_rarity.json and displays counts and sample elements for each tier.
"""

import os
import json

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
JSON_PATH = os.path.join(REPO_ROOT, 'data', 'roguelike_rarity.json')

def main():
    if not os.path.exists(JSON_PATH):
        print(f"Error: {JSON_PATH} does not exist.")
        return

    with open(JSON_PATH, 'r', encoding='utf-8') as f:
        data = json.load(f)

    print("==================================================")
    print(f"  ROGUELIKE RARITY GROUND TRUTH ({JSON_PATH})")
    print("==================================================")

    for cat in ("pokemon", "moves", "items"):
        print(f"\n--- {cat.upper()} ---")
        if cat not in data:
            print("  Missing category in JSON!")
            continue
        total = 0
        for tier in ("COMMON", "UNCOMMON", "RARE", "EPIC"):
            items = data[cat].get(tier, [])
            count = len(items)
            total += count
            sample = ", ".join(items[:5])
            if count > 5:
                sample += f", ... (+{count - 5} more)"
            print(f"  {tier:8} ({count:4}): {sample}")
        print(f"  TOTAL   : {total:4}")

if __name__ == '__main__':
    main()
