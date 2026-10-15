#!/usr/bin/env python3
"""
gen_rarity.py - Generates data/roguelike_rarity.json according to KaizoRogue rarity rules.
Preserves existing manual overrides in data/roguelike_rarity.json if it already exists.
"""

import os
import sys
import json
import re
import glob

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
JSON_PATH = os.path.join(REPO_ROOT, 'data', 'roguelike_rarity.json')

TIERS = ["COMMON", "UNCOMMON", "RARE", "EPIC"]

BABY_POKEMON = {
    'SPECIES_PICHU', 'SPECIES_CLEFFA', 'SPECIES_IGGLYBUFF', 'SPECIES_TOGEPI',
    'SPECIES_TYROGUE', 'SPECIES_SMOOCHUM', 'SPECIES_ELEKID', 'SPECIES_MAGBY',
    'SPECIES_AZURILL', 'SPECIES_WYNAUT', 'SPECIES_BUDEW', 'SPECIES_CHINGLING',
    'SPECIES_BONSLY', 'SPECIES_MIME_JR', 'SPECIES_HAPPINY', 'SPECIES_MUNCHLAX',
    'SPECIES_RIOLU', 'SPECIES_MANTYKE', 'SPECIES_TOXEL', 'SPECIES_COSMOG', 'SPECIES_COSMOEM'
}

COMPETITIVE_NFE = {
    'SPECIES_CHANSEY', 'SPECIES_DOUBLADE', 'SPECIES_PORYGON2', 'SPECIES_DUSCLOPS',
    'SPECIES_SCYTHER', 'SPECIES_HAUNTER', 'SPECIES_KADABRA', 'SPECIES_MAGNETON',
    'SPECIES_RHYDON', 'SPECIES_ELECTABUZZ', 'SPECIES_MAGMAR', 'SPECIES_TANGELA',
    'SPECIES_SNEASEL', 'SPECIES_GLIGAR', 'SPECIES_MISDREAVUS', 'SPECIES_ROSELIA',
    'SPECIES_YANMA', 'SPECIES_PILOSWINE', 'SPECIES_BISHARP', 'SPECIES_GOLBAT',
    'SPECIES_PRIMEAPE', 'SPECIES_DIPPLIN', 'SPECIES_DURALUDON'
}

PSEUDO_LEGENDARY_BABIES = {
    'SPECIES_DRATINI', 'SPECIES_DRAGONAIR', 'SPECIES_LARVITAR', 'SPECIES_PUPITAR',
    'SPECIES_BAGON', 'SPECIES_SHELGON', 'SPECIES_BELDUM', 'SPECIES_METANG',
    'SPECIES_GIBLE', 'SPECIES_GABITE', 'SPECIES_DEINO', 'SPECIES_ZWEILOUS',
    'SPECIES_GOOMY', 'SPECIES_SLIGGOO', 'SPECIES_SLIGGOO_HISUIAN',
    'SPECIES_JANGMO_O', 'SPECIES_HAKAMO_O', 'SPECIES_DREEPY', 'SPECIES_DRAKLOAK',
    'SPECIES_FRIGIBAX', 'SPECIES_ARCTIBAX'
}

def parse_c_entries(filepath, prefix):
    content = open(filepath, 'r', encoding='utf-8', errors='ignore').read()
    entries = {}
    i = 0
    n = len(content)
    while i < n:
        idx = content.find('[' + prefix, i)
        if idx == -1:
            break
        close_bracket = content.find(']', idx)
        if close_bracket == -1:
            break
        name = content[idx+1:close_bracket].strip()
        brace_open = content.find('{', close_bracket)
        if brace_open == -1:
            break
        depth = 1
        pos = brace_open + 1
        while pos < n and depth > 0:
            if content[pos] == '{':
                depth += 1
            elif content[pos] == '}':
                depth -= 1
            pos += 1
        entries[name] = content[brace_open:pos]
        i = pos
    return entries

def load_defines(species_files):
    defines = {}
    for sf in species_files:
        for line in open(sf, 'r', encoding='utf-8', errors='ignore'):
            m = re.match(r'\s*#define\s+([A-Za-z0-9_]+)\s+(.+)', line)
            if m:
                name = m.group(1)
                val = m.group(2).strip()
                if name not in defines:
                    defines[name] = val
    return defines

def resolve_val(raw, defines):
    raw = raw.strip()
    if raw.isdigit():
        return int(raw)
    m = re.search(r'\?\s*([0-9]+)\s*:', raw)
    if m:
        return int(m.group(1))
    if raw in defines:
        def_val = defines[raw]
        if def_val.isdigit():
            return int(def_val)
        m = re.search(r'\?\s*([0-9]+)\s*:', def_val)
        if m:
            return int(m.group(1))
    return 0

# -------------------------------------------------------------
# 1. POKÉMON CLASSIFICATION
# -------------------------------------------------------------
def classify_pokemon(name, blk, defines):
    # Exclude mega, gigantamax, totem, primal, none, egg
    if name in ('SPECIES_NONE', 'SPECIES_EGG'):
        return None
    if bool(re.search(r'\.isMegaEvolution\s*=\s*TRUE', blk)) or '_MEGA' in name:
        return None
    if bool(re.search(r'\.isGigantamax\s*=\s*TRUE', blk)) or '_GMAX' in name:
        return None
    if bool(re.search(r'\.isTotem\s*=\s*TRUE', blk)) or '_TOTEM' in name:
        return None
    if bool(re.search(r'\.isPrimalReversion\s*=\s*TRUE', blk)) or '_PRIMAL' in name:
        return None

    # Check if species has valid name or 0 BST
    hp_m = re.search(r'\.baseHP\s*=\s*([^,\n]+)', blk)
    atk_m = re.search(r'\.baseAttack\s*=\s*([^,\n]+)', blk)
    def_m = re.search(r'\.baseDefense\s*=\s*([^,\n]+)', blk)
    spe_m = re.search(r'\.baseSpeed\s*=\s*([^,\n]+)', blk)
    spa_m = re.search(r'\.baseSpAttack\s*=\s*([^,\n]+)', blk)
    spd_m = re.search(r'\.baseSpDefense\s*=\s*([^,\n]+)', blk)

    hp = resolve_val(hp_m.group(1), defines) if hp_m else 0
    atk = resolve_val(atk_m.group(1), defines) if atk_m else 0
    defn = resolve_val(def_m.group(1), defines) if def_m else 0
    spe = resolve_val(spe_m.group(1), defines) if spe_m else 0
    spa = resolve_val(spa_m.group(1), defines) if spa_m else 0
    spd = resolve_val(spd_m.group(1), defines) if spd_m else 0
    raw_bst = hp + atk + defn + spe + spa + spd

    if raw_bst == 0:
        return None

    is_mythical = bool(re.search(r'\.isMythical\s*=\s*TRUE', blk))
    is_ub = bool(re.search(r'\.isUltraBeast\s*=\s*TRUE', blk))
    is_paradox = bool(re.search(r'\.isParadox\s*=\s*TRUE', blk))
    no_eggs = ('EGG_GROUP_NO_EGGS_DISCOVERED' in blk) and (name not in BABY_POKEMON)

    # Automatic Epic for Legendaries / Mythicals / UBs / Paradox
    if is_mythical or is_ub or is_paradox or no_eggs:
        return 'EPIC'

    effective_bst = raw_bst

    # Extreme passive abilities
    if 'ABILITY_TRUANT' in blk or 'ABILITY_SLOW_START' in blk:
        effective_bst -= 180
    elif 'ABILITY_HUGE_POWER' in blk or 'ABILITY_PURE_POWER' in blk:
        effective_bst += 120

    # Min-max distribution bonus (+30 if max(Atk, SpA) >= 125 and Spe >= 90)
    if max(atk, spa) >= 125 and spe >= 90:
        effective_bst += 30

    # Evolution stage filter
    has_evo = bool(re.search(r'\.evolutions\s*=\s*EVOLUTION\(', blk))
    if has_evo and name not in COMPETITIVE_NFE and name not in PSEUDO_LEGENDARY_BABIES:
        if effective_bst >= 490:
            effective_bst = 489

    # User example specific: Butterfree & early fully-evolved bugs in Uncommon
    if name in ('SPECIES_BUTTERFREE', 'SPECIES_BEEDRILL', 'SPECIES_BEAUTIFLY', 'SPECIES_DUSTOX', 'SPECIES_VIVILLON'):
        effective_bst = max(effective_bst, 420)

    if effective_bst < 420:
        return 'COMMON'
    elif effective_bst < 490:
        return 'UNCOMMON'
    elif effective_bst < 560:
        return 'RARE'
    else:
        return 'EPIC'

# -------------------------------------------------------------
# 2. MOVES CLASSIFICATION (Balanced for Kaizo Ironmon)
# -------------------------------------------------------------
def classify_move(name, blk):
    if name in ('MOVE_NONE', 'MOVE_STRUGGLE'):
        return None
    # Skip Z-Moves, Max Moves, Shadow moves, unfinished
    if name.startswith('MOVE_Z_') or name.startswith('MOVE_MAX_') or name.startswith('MOVE_G_MAX_'):
        return None
    if 'gNotDoneYetDescription' in blk:
        return None
    if re.search(r'\.name\s*=\s*COMPOUND_STRING\("(-|\?|)"\)', blk):
        return None

    cat_m = re.search(r'\.category\s*=\s*(DAMAGE_CATEGORY_[A-Z]+)', blk)
    cat = cat_m.group(1) if cat_m else 'DAMAGE_CATEGORY_PHYSICAL'

    pwr_m = re.search(r'\.power\s*=\s*([^,\n]+)', blk)
    pwr = resolve_val(pwr_m.group(1), {}) if pwr_m else 0

    acc_m = re.search(r'\.accuracy\s*=\s*([^,\n]+)', blk)
    acc = resolve_val(acc_m.group(1), {}) if acc_m else 0

    prio_m = re.search(r'\.priority\s*=\s*(-?\d+)', blk)
    prio = int(prio_m.group(1)) if prio_m else 0

    # ----------------- STATUS MOVES -----------------
    if cat == 'DAMAGE_CATEGORY_STATUS':
        # Specific user overrides
        if name in ('MOVE_QUIVER_DANCE', 'MOVE_SHELL_SMASH', 'MOVE_SPORE', 'MOVE_TAIL_GLOW', 'MOVE_SHIFT_GEAR'):
            return 'EPIC'
        if name in ('MOVE_DRAGON_DANCE', 'MOVE_TOXIC', 'MOVE_STEALTH_ROCK', 'MOVE_CALM_MIND'):
            return 'RARE'
        if name in ('MOVE_SILK_TRAP', 'MOVE_BULK_UP', 'MOVE_WILL_O_WISP'):
            return 'RARE'
        if name in ('MOVE_GROWL', 'MOVE_SAND_ATTACK', 'MOVE_TAIL_WHIP', 'MOVE_SPLASH', 'MOVE_LEER'):
            return 'COMMON'

        score = 40  # base default for status

        # 1. Supreme setup (+2 offensive stats, multi-stat sweeps, belly drum) -> EPIC
        if any(kw in blk for kw in [
            'EFFECT_BELLY_DRUM', 'EFFECT_GEOMANCY', 'EFFECT_NO_RETREAT', 'EFFECT_CLANGOROUS_SOUL',
            'EFFECT_EXTREME_EVOBOOST', 'EFFECT_REVIVAL_BLESSING', 'EFFECT_SHED_TAIL', 'EFFECT_SHELL_SMASH',
            'EFFECT_QUIVER_DANCE', 'EFFECT_SHIFT_GEAR', 'EFFECT_TAIL_GLOW'
        ]):
            score = 135
        elif 'EFFECT_STAT_CHANGE' in blk and any(f'.{s} = 2' in blk or f'.{s} = 3' in blk for s in ['attack', 'spAtk']):
            score = 130
        elif 'EFFECT_STAT_CHANGE' in blk and ('.attack = 1' in blk or '.spAtk = 1' in blk) and '.speed = 1' in blk:
            score = 130

        # 2. Pure Recovery & Substitute -> EPIC (healing resources are vital in Kaizo Ironmon!)
        elif any(kw in blk for kw in [
            'EFFECT_RESTORE_HP', 'EFFECT_ROOST', 'EFFECT_SOFTBOILED', 'EFFECT_SHORE_UP',
            'EFFECT_SLACK_OFF', 'EFFECT_MILK_DRINK', 'EFFECT_STRENGTH_SAP', 'EFFECT_WISH',
            'EFFECT_HEAL_ORDER', 'EFFECT_PURIFY', 'EFFECT_JUNGLE_HEALING', 'EFFECT_LIFE_DEW'
        ]):
            score = 130
        elif any(kw in blk for kw in ['EFFECT_SUBSTITUTE']):
            score = 130
        elif name in ('MOVE_SPORE'):
            score = 130

        # 3. High-tier status control, semi-recovery, hazards, protect -> RARE
        elif any(kw in blk for kw in [
            'EFFECT_PROTECT', 'EFFECT_DETECT', 'EFFECT_SPIKY_SHIELD', 'EFFECT_BANEFUL_BUNKER',
            'EFFECT_KINGS_SHIELD', 'EFFECT_OBSTRUCT', 'EFFECT_SILK_TRAP', 'EFFECT_BURNING_BULWARK'
        ]):
            score = 115
        elif any(kw in blk for kw in [
            'EFFECT_STEALTH_ROCK', 'EFFECT_SPIKES', 'EFFECT_TOXIC_SPIKES', 'EFFECT_STICKY_WEB',
            'EFFECT_CEASELESS_EDGE', 'EFFECT_STONE_AXE'
        ]):
            score = 115
        elif any(kw in blk for kw in [
            'EFFECT_NON_VOLATILE_STATUS', 'EFFECT_WILL_O_WISP', 'EFFECT_TOXIC', 'EFFECT_THUNDER_WAVE',
            'EFFECT_GLARE', 'EFFECT_YAWN', 'EFFECT_DARK_VOID', 'EFFECT_SLEEP_POWDER', 'EFFECT_HYPNOSIS'
        ]):
            score = 105
        elif any(kw in blk for kw in [
            'EFFECT_PARTING_SHOT', 'EFFECT_TAUNT', 'EFFECT_ENCORE', 'EFFECT_TORMENT',
            'EFFECT_DISABLE', 'EFFECT_HAZE', 'EFFECT_CLEAR_SMOG', 'EFFECT_DESTINY_BOND',
            'EFFECT_PAIN_SPLIT', 'EFFECT_LEECH_SEED', 'EFFECT_BATON_PASS', 'EFFECT_TRICK', 'EFFECT_SWITCHEROO'
        ]):
            score = 105
        elif any(kw in blk for kw in [
            'EFFECT_SYNTHESIS', 'EFFECT_MOONLIGHT', 'EFFECT_MORNING_SUN'
        ]):
            score = 100
        elif 'EFFECT_STAT_CHANGE' in blk and ('.self = TRUE' in blk or 'self' in blk):
            # Single stage setup like Calm Mind, Bulk Up, Curse, Coil, Iron Defense, Amnesia, Agility
            if any(f'.{s} = 1' in blk for s in ['attack', 'spAtk', 'defense', 'spDefense']):
                score = 100
            else:
                score = 75

        # 4. Mid-tier support: Screens, weather, terrain, speed control -> UNCOMMON
        elif any(kw in blk for kw in ['EFFECT_LIGHT_SCREEN', 'EFFECT_REFLECT', 'EFFECT_AURORA_VEIL']):
            score = 80
        elif any(kw in blk for kw in ['EFFECT_WEATHER', 'EFFECT_TERRAIN', 'EFFECT_TAILWIND', 'EFFECT_TRICK_ROOM']):
            score = 75
        elif any(kw in blk for kw in ['EFFECT_CONFUSION', 'EFFECT_ATTRACT', 'EFFECT_REST', 'EFFECT_SLEEP_TALK']):
            score = 70
        elif any(kw in blk for kw in ['EFFECT_DEFOG', 'EFFECT_RAPID_SPIN']):
            score = 75
        elif 'EFFECT_STAT_CHANGE' in blk and ('.target = TARGET_OPPONENT' in blk or '.target = TARGET_SELECTED' in blk):
            if any(f'.{s} = -2' in blk for s in ['defense', 'spDefense', 'speed', 'attack', 'spAtk']):
                score = 70  # Strong debuff (e.g. Screech, Metal Sound, Fake Tears, Scary Face)
            else:
                score = 40  # Weak debuff (e.g. Growl, Leer)
        else:
            score = 35

        if score < 65:
            return 'COMMON'
        elif score < 95:
            return 'UNCOMMON'
        elif score < 125:
            return 'RARE'
        else:
            return 'EPIC'

    # ----------------- DAMAGE MOVES -----------------
    if pwr == 0:
        return 'COMMON'

    # OHKO moves (Fissure, Guillotine, Horn Drill, Sheer Cold) -> COMMON/GIMMICK (30% accuracy is suicidal in Ironmon)
    if any(kw in blk for kw in ['EFFECT_OHKO']):
        return 'COMMON'

    # Specific overrides from user prompt
    if name in ('MOVE_CLOSE_COMBAT', 'MOVE_V_CREATE', 'MOVE_MAKE_IT_RAIN'):
        return 'EPIC'
    if name in ('MOVE_FLAMETHROWER', 'MOVE_THUNDERBOLT', 'MOVE_SHADOW_BALL'):
        return 'RARE'
    if name in ('MOVE_BITE', 'MOVE_SWIFT', 'MOVE_BUBBLE_BEAM'):
        return 'UNCOMMON'
    if name in ('MOVE_TACKLE', 'MOVE_WATER_GUN', 'MOVE_PECK'):
        return 'COMMON'

    # Multi-hit (2-5 hits -> treat as pwr * 3.1)
    is_multihit = bool(re.search(r'\.(strikeCount|multiHit)\s*=\s*(TRUE|[2-9])', blk)) or ('2 to 5' in blk)
    effective_pwr = pwr * 3.1 if is_multihit else pwr

    # Base calculation
    accuracy_val = acc if acc > 0 else 100
    power_score = effective_pwr * (accuracy_val / 100.0)

    # ACCURACY PENALTIES / BONUSES (Essential for 1-mon Ironmon survival!)
    if acc == 0:
        power_score += 15  # Never misses (Aerial Ace, Aura Sphere, Swift, Shock Wave, etc.)
    elif acc <= 70:
        power_score -= 40  # Huge risk: Focus Blast, Blizzard, Thunder, Dynamic Punch
    elif acc <= 80:
        power_score -= 25  # High risk: Hydro Pump, Stone Edge, Fire Blast
    elif acc <= 85:
        power_score -= 15  # Moderate risk: Play Rough, Megahorn, Rock Slide
    elif acc <= 95:
        power_score -= 5

    # DRAINING MOVES (Drains HP to heal self -> game-changer in Kaizo Ironmon!)
    if 'MOVE_EFFECT_ABSORB' in blk or '.healingMove' in blk or 'EFFECT_ABSORB' in blk or 'EFFECT_PARABOLIC_CHARGE' in blk:
        power_score += 45

    # PRIORITY
    if prio > 0:
        power_score += prio * 25
    elif prio < 0:
        power_score += prio * 20

    # HIGH CRITICAL HIT
    if 'criticalHitStage' in blk or 'alwaysCriticalHit' in blk:
        power_score += 12

    # HIGH-IMPACT SIDE EFFECTS
    if name == 'MOVE_FAKE_OUT':
        power_score += 35
    elif 'MOVE_EFFECT_FLINCH' in blk:
        power_score += 15

    if name == 'MOVE_NUZZLE':
        power_score += 40
    elif name == 'MOVE_MORTAL_SPIN':
        power_score += 35

    # 100% Stat changes
    if 'MOVE_EFFECT_STAT_PLUS' in blk and ('.self = TRUE' in blk or 'self' in blk):
        power_score += 35
    elif 'MOVE_EFFECT_STAT_MINUS' in blk and ('.self = TRUE' not in blk):
        # 100% enemy debuff (Acid Spray, Icy Wind, Bulldoze, Mystical Fire, etc.)
        power_score += 25
    elif 'MOVE_EFFECT_STAT_PLUS' in blk:
        power_score += 20
    elif 'MOVE_EFFECT_STAT_MINUS' in blk:
        power_score += 15

    # Status alterations
    if any(st in blk for st in ['MOVE_EFFECT_BURN', 'MOVE_EFFECT_PARALYSIS', 'MOVE_EFFECT_SLEEP', 'MOVE_EFFECT_FREEZE', 'MOVE_EFFECT_POISON', 'MOVE_EFFECT_TOXIC']):
        chance_m = re.search(r'\.chance\s*=\s*(\d+)', blk)
        chance = int(chance_m.group(1)) if chance_m else 0
        if chance == 0 or chance >= 50:
            power_score += 40
        elif chance >= 30:
            power_score += 30  # Scald, Lava Plume (burn halves attack!)
        elif chance >= 10:
            power_score += 15

    # MALUS: Recoil & Self-Damage (Fatal or massive HP self-damage in Ironmon!)
    if any(kw in blk for kw in ['EFFECT_MAX_HP_50_RECOIL', 'EFFECT_CHLOROBLAST', 'EFFECT_EXPLOSION', 'EFFECT_SELF_DESTRUCT', 'EFFECT_MIND_BLOWN', 'EFFECT_STEEL_BEAM', 'EFFECT_FINAL_GAMBIT']):
        power_score -= 100
    elif any(kw in blk for kw in ['EFFECT_RECOIL', 'MOVE_EFFECT_RECOIL_HP_25']):
        power_score -= 25

    # MALUS: Charge / Recharge turns (Giving free turns is lethal)
    if any(kw in blk for kw in ['MOVE_EFFECT_RECHARGE', 'EFFECT_RECHARGE']):
        power_score -= 50
    elif any(kw in blk for kw in ['EFFECT_TWO_TURNS_ATTACK', 'EFFECT_SOLAR_BEAM', 'EFFECT_SKULL_BASH', 'EFFECT_SKY_ATTACK', 'EFFECT_RAZOR_WIND', 'EFFECT_FREEZE_SHOCK', 'EFFECT_ICE_BURN', 'EFFECT_METEOR_BEAM']):
        power_score -= 35
    elif any(kw in blk for kw in ['EFFECT_SEMI_INVULNERABLE', 'EFFECT_FLY', 'EFFECT_DIG', 'EFFECT_DIVE', 'EFFECT_BOUNCE']):
        power_score -= 15

    # MALUS: Stat drop on user (Close Combat, Overheat, etc.)
    if 'MOVE_EFFECT_STAT_MINUS' in blk and ('.self = TRUE' in blk or 'self' in blk):
        power_score -= 20

    if power_score < 65:
        return 'COMMON'
    elif power_score < 95:
        return 'UNCOMMON'
    elif power_score < 125:
        return 'RARE'
    else:
        return 'EPIC'

# -------------------------------------------------------------
# 3. ITEMS CLASSIFICATION (Balanced for Kaizo Ironmon)
# -------------------------------------------------------------
def classify_item(name, blk):
    if name == 'ITEM_NONE' or name.startswith('ITEM_TM') or name.startswith('ITEM_HM'):
        return None
    if 'gQuestionMarksItemName' in blk or '??????' in blk:
        return None

    pocket_m = re.search(r'\.pocket\s*=\s*(POCKET_[A-Z_]+)', blk)
    pocket = pocket_m.group(1) if pocket_m else 'POCKET_ITEMS'
    if pocket in ('POCKET_KEY_ITEMS', 'POCKET_TM_HM'):
        return None

    imp_m = re.search(r'\.importance\s*=\s*(\d+)', blk)
    if imp_m and int(imp_m.group(1)) > 0:
        return None

    price_m = re.search(r'\.price\s*=\s*([^,\n]+)', blk)
    price = resolve_val(price_m.group(1), {}) if price_m else 0

    hold_m = re.search(r'\.holdEffect\s*=\s*(HOLD_EFFECT_[A-Z_]+)', blk)
    hold = hold_m.group(1) if hold_m else 'HOLD_EFFECT_NONE'

    sort_m = re.search(r'\.sortType\s*=\s*(ITEM_TYPE_[A-Z_]+)', blk)
    sort = sort_m.group(1) if sort_m else 'ITEM_TYPE_UNCATEGORIZED'

    # 1. EPIC (Top game-changing items: Max PP, Rare Candies, Full revives/elixirs, Master Ball, Mega/Z-items)
    epic_items = {
        'ITEM_MASTER_BALL', 'ITEM_ABILITY_PATCH', 'ITEM_ABILITY_CAPSULE',
        'ITEM_PP_MAX', 'ITEM_PP_UP', 'ITEM_GOLD_BOTTLE_CAP', 'ITEM_LUCKY_EGG',
        'ITEM_RARE_CANDY', 'ITEM_EXP_CANDY_XL', 'ITEM_EXP_CANDY_L',
        'ITEM_MAX_REVIVE', 'ITEM_REVIVAL_HERB', 'ITEM_SACRED_ASH',
        'ITEM_MAX_ELIXIR', 'ITEM_ELIXIR'
    }
    if name in epic_items or hold in ('HOLD_EFFECT_Z_CRYSTAL', 'HOLD_EFFECT_MEGA_STONE') or sort in ('ITEM_TYPE_MEGA_STONE', 'ITEM_TYPE_Z_CRYSTAL'):
        return 'EPIC'

    # 2. RARE (Vital recovery, Leppa berry, damage resist berries, competitive hold items)
    rare_items = {
        'ITEM_FULL_RESTORE', 'ITEM_MAX_POTION', 'ITEM_HYPER_POTION',
        'ITEM_MAX_ETHER', 'ITEM_ETHER', 'ITEM_LEPPA_BERRY',
        'ITEM_REVIVE', 'ITEM_BOTTLE_CAP', 'ITEM_EXP_CANDY_M',
        'ITEM_SITRUS_BERRY', 'ITEM_LUM_BERRY'
    }
    if name in rare_items:
        return 'RARE'

    rare_holds = {
        'HOLD_EFFECT_CHOICE_BAND', 'HOLD_EFFECT_CHOICE_SCARF', 'HOLD_EFFECT_CHOICE_SPECS',
        'HOLD_EFFECT_FOCUS_SASH', 'HOLD_EFFECT_LIFE_ORB', 'HOLD_EFFECT_LEFTOVERS',
        'HOLD_EFFECT_EVIOLITE', 'HOLD_EFFECT_ASSAULT_VEST', 'HOLD_EFFECT_HEAVY_DUTY_BOOTS',
        'HOLD_EFFECT_LOADED_DICE', 'HOLD_EFFECT_WEAKNESS_POLICY', 'HOLD_EFFECT_ROCKY_HELMET',
        'HOLD_EFFECT_AIR_BALLOON', 'HOLD_EFFECT_BOOSTER_ENERGY', 'HOLD_EFFECT_CLEAR_AMULET',
        'HOLD_EFFECT_COVERT_CLOAK', 'HOLD_EFFECT_SHELL_BELL', 'HOLD_EFFECT_RESIST_BERRY'
    }
    if hold in rare_holds:
        return 'RARE'

    if price >= 4000 and pocket == 'POCKET_ITEMS':
        return 'RARE'

    # 3. UNCOMMON (Mid-tier healing, pinch berries, evolution items, type boosts, good pokeballs)
    uncommon_items = {
        'ITEM_SUPER_POTION', 'ITEM_MOOMOO_MILK', 'ITEM_LEMONADE', 'ITEM_SODA_POP',
        'ITEM_FRESH_WATER', 'ITEM_ENERGY_ROOT', 'ITEM_ENERGY_POWDER', 'ITEM_HEAL_POWDER',
        'ITEM_FULL_HEAL', 'ITEM_LAVA_COOKIE', 'ITEM_OLD_GATEAU', 'ITEM_CASTELIACONE',
        'ITEM_LUMIOSE_GALETTE', 'ITEM_SHALOUR_SABLE', 'ITEM_BIG_MALASADA', 'ITEM_PEWTER_CRUNCHIES',
        'ITEM_EXP_CANDY_S', 'ITEM_EXP_CANDY_XS'
    }
    if name in uncommon_items:
        return 'UNCOMMON'

    if pocket == 'POCKET_POKE_BALLS' and price >= 500:
        return 'UNCOMMON'

    if sort in ('ITEM_TYPE_EVOLUTION_STONE', 'ITEM_TYPE_EVOLUTION_ITEM', 'ITEM_TYPE_NATURE_MINT', 'ITEM_TYPE_STAT_BOOST_DRINK'):
        return 'UNCOMMON'

    uncommon_holds = {
        'HOLD_EFFECT_TYPE_POWER', 'HOLD_EFFECT_PLATE', 'HOLD_EFFECT_GEMS',
        'HOLD_EFFECT_SCOPE_LENS', 'HOLD_EFFECT_WISE_GLASSES', 'HOLD_EFFECT_MUSCLE_BAND',
        'HOLD_EFFECT_EXPERT_BELT', 'HOLD_EFFECT_WIDE_LENS', 'HOLD_EFFECT_ZOOM_LENS',
        'HOLD_EFFECT_BIG_ROOT', 'HOLD_EFFECT_LIGHT_CLAY', 'HOLD_EFFECT_RESTORE_PCT_HP',
        'HOLD_EFFECT_CURE_STATUS', 'HOLD_EFFECT_MICLE_BERRY', 'HOLD_EFFECT_CUSTAP_BERRY',
        'HOLD_EFFECT_JABOCA_BERRY', 'HOLD_EFFECT_ROWAP_BERRY', 'HOLD_EFFECT_KEE_BERRY',
        'HOLD_EFFECT_MARANGA_BERRY', 'HOLD_EFFECT_ENIGMA_BERRY', 'HOLD_EFFECT_CONFUSE_FLAVOR'
    }
    if hold in uncommon_holds:
        return 'UNCOMMON'

    if price >= 1000 and pocket == 'POCKET_ITEMS':
        return 'UNCOMMON'

    # 4. COMMON (Basic potions, status cures, single-status berries, X-items, basic balls)
    return 'COMMON'

# -------------------------------------------------------------
# MAIN GENERATOR
# -------------------------------------------------------------
def main():
    force = '--force' in sys.argv
    force_moves_items = '--force-moves-items' in sys.argv or '--recreate-moves-items' in sys.argv
    add_missing = '--add-missing' in sys.argv

    # 1. Check for existing Ground Truth JSON
    existing_data = None
    existing_tiers = {"pokemon": {}, "moves": {}, "items": {}}

    if os.path.exists(JSON_PATH) and not force:
        try:
            with open(JSON_PATH, 'r', encoding='utf-8') as f:
                existing_data = json.load(f)
            print(f"Loaded existing Ground Truth from {JSON_PATH}")
            cats_to_load = ("pokemon",) if force_moves_items else ("pokemon", "moves", "items")
            for cat in cats_to_load:
                if cat in existing_data:
                    for tier, names in existing_data[cat].items():
                        for nm in names:
                            existing_tiers[cat][nm] = tier
        except Exception as e:
            print(f"Warning: could not parse existing JSON: {e}")
            existing_data = None

    # 2. Parse C sources
    species_files = sorted(glob.glob(os.path.join(REPO_ROOT, 'src', 'data', 'pokemon', 'species_info', 'gen_*_families.h')))
    defines = load_defines(species_files)

    all_species = {}
    for sf in species_files:
        all_species.update(parse_c_entries(sf, 'SPECIES_'))

    moves_path = os.path.join(REPO_ROOT, 'src', 'data', 'moves_info.h')
    all_moves = parse_c_entries(moves_path, 'MOVE_')

    items_path = os.path.join(REPO_ROOT, 'src', 'data', 'items.h')
    all_items = parse_c_entries(items_path, 'ITEM_')

    # Output structure
    result = {
        "pokemon": {t: [] for t in TIERS},
        "moves": {t: [] for t in TIERS},
        "items": {t: [] for t in TIERS}
    }

    # Helper to insert into result
    def add_entry(cat, name, computed_tier):
        if existing_data is not None and (cat not in ("moves", "items") or not force_moves_items):
            # If entry already existed in JSON, keep its tier!
            if name in existing_tiers[cat]:
                actual_tier = existing_tiers[cat][name]
                result[cat][actual_tier].append(name)
                return
            elif not add_missing and len(existing_tiers[cat]) > 0:
                # If user removed this element manually and add_missing is False, don't re-add!
                return
        result[cat][computed_tier].append(name)

    # Classify Pokémon
    for name, blk in all_species.items():
        tier = classify_pokemon(name, blk, defines)
        if tier:
            add_entry("pokemon", name, tier)

    # Classify Moves
    for name, blk in all_moves.items():
        tier = classify_move(name, blk)
        if tier:
            add_entry("moves", name, tier)

    # Classify Items
    for name, blk in all_items.items():
        tier = classify_item(name, blk)
        if tier:
            add_entry("items", name, tier)

    # Sort each tier list alphabetically and ensure uniqueness
    for cat in ("pokemon", "moves", "items"):
        for t in TIERS:
            result[cat][t] = sorted(list(set(result[cat][t])))

    # Write out JSON
    os.makedirs(os.path.dirname(JSON_PATH), exist_ok=True)
    with open(JSON_PATH, 'w', encoding='utf-8') as f:
        json.dump(result, f, indent=2)

    print(f"Successfully wrote {JSON_PATH}")
    for cat in ("pokemon", "moves", "items"):
        print(f"\n[{cat.upper()}]")
        total = 0
        for t in TIERS:
            cnt = len(result[cat][t])
            total += cnt
            print(f"  {t:8}: {cnt:4}")
        print(f"  TOTAL   : {total:4}")

if __name__ == '__main__':
    main()
