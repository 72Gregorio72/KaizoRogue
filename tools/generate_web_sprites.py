#!/usr/bin/env python3
"""
Generates transparent, cropped, authentic in-project sprites from graphics/pokemon/
into web/assets/sprites/ for web hub usage.
"""
import os
import re
import glob
import json
import zlib
import struct

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEX_PATH = os.path.join(ROOT, "web", "data", "pokedex.json")
OUT_DIR = os.path.join(ROOT, "web", "assets", "sprites")

def parse_pal(pal_path):
    if not pal_path or not os.path.exists(pal_path):
        return None
    with open(pal_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = [l.strip() for l in f.readlines() if l.strip()]
    if len(lines) > 3 and lines[0] == "JASC-PAL":
        colors = []
        for l in lines[3:]:
            parts = l.split()
            if len(parts) == 3:
                colors.extend([int(p) for p in parts])
        return bytes(colors)
    return None

def process_png(png_path, pal_bytes=None, crop_half_height=True):
    if not png_path or not os.path.exists(png_path):
        return None
    with open(png_path, "rb") as f:
        data = f.read()

    # Read IHDR
    ihdr_idx = data.find(b"IHDR")
    if ihdr_idx == -1:
        return None
    w, h, bit_depth, color_type = struct.unpack(">IIBB", data[ihdr_idx+4:ihdr_idx+14])
    new_h = (h // 2) if crop_half_height and h > 32 else h

    # If pal_bytes provided, replace PLTE
    plte_idx = data.find(b"PLTE")
    if plte_idx != -1 and pal_bytes:
        old_len = int.from_bytes(data[plte_idx-4:plte_idx], "big")
        # only replace if length matches or fits
        if len(pal_bytes) == old_len or len(pal_bytes) == 48:
            pal_to_use = pal_bytes[:old_len] if len(pal_bytes) >= old_len else pal_bytes.ljust(old_len, b"\x00")
            crc = zlib.crc32(b"PLTE" + pal_to_use).to_bytes(4, "big")
            data = data[:plte_idx] + b"PLTE" + pal_to_use + crc + data[plte_idx+4+old_len+4:]

    # Add tRNS if not present
    plte_idx = data.find(b"PLTE")
    if plte_idx != -1 and b"tRNS" not in data:
        plte_len = int.from_bytes(data[plte_idx-4:plte_idx], "big")
        insert_pos = plte_idx + 4 + plte_len + 4
        trns = b"\x00\x00\x00\x01tRNS\x00" + zlib.crc32(b"tRNS\x00").to_bytes(4, "big")
        data = data[:insert_pos] + trns + data[insert_pos:]

    if not crop_half_height or new_h == h:
        return data

    # Crop to top half
    idat_data = b""
    pos = 8
    while pos < len(data):
        chunk_len = int.from_bytes(data[pos:pos+4], "big")
        chunk_type = data[pos+4:pos+8]
        if chunk_type == b"IDAT":
            idat_data += data[pos+8:pos+8+chunk_len]
        pos += 8 + chunk_len + 4

    try:
        decomp = zlib.decompress(idat_data)
        row_bytes = 1 + (w * bit_depth + 7) // 8
        new_decomp = decomp[:row_bytes * new_h]
        new_idat = zlib.compress(new_decomp)
    except Exception:
        return data

    # Rebuild PNG
    out = data[:8]
    pos = 8
    while pos < len(data):
        chunk_len = int.from_bytes(data[pos:pos+4], "big")
        chunk_type = data[pos+4:pos+8]
        if chunk_type == b"IHDR":
            new_ihdr = struct.pack(">IIBB", w, new_h, bit_depth, color_type) + data[ihdr_idx+14:ihdr_idx+17]
            crc = zlib.crc32(b"IHDR" + new_ihdr).to_bytes(4, "big")
            out += len(new_ihdr).to_bytes(4, "big") + b"IHDR" + new_ihdr + crc
        elif chunk_type == b"IDAT":
            pass
        elif chunk_type == b"IEND":
            crc = zlib.crc32(b"IDAT" + new_idat).to_bytes(4, "big")
            out += len(new_idat).to_bytes(4, "big") + b"IDAT" + new_idat + crc
            out += data[pos:pos+8+chunk_len+4]
        else:
            out += data[pos:pos+8+chunk_len+4]
        pos += 8 + chunk_len + 4
    return out

def main():
    os.makedirs(os.path.join(OUT_DIR, "front"), exist_ok=True)
    os.makedirs(os.path.join(OUT_DIR, "front_shiny"), exist_ok=True)
    os.makedirs(os.path.join(OUT_DIR, "icon"), exist_ok=True)

    with open(os.path.join(ROOT, "src", "data", "graphics", "pokemon.h"), "r") as f:
        gfx_content = f.read()

    front_map, icon_map, pal_map, shiny_pal_map = {}, {}, {}, {}
    for m in re.finditer(r'const u32 gMonFrontPic_(\w+)\[\]\s*=\s*INCGFX_U32\(\"([^\"]+)\"', gfx_content):
        name, pth = m.group(1), os.path.join(ROOT, m.group(2))
        if name not in front_map or ("_gba" in front_map[name] and "_gba" not in pth):
            front_map[name] = pth

    for m in re.finditer(r'const u8 gMonIcon_(\w+)\[\]\s*=\s*INCGFX_U8\(\"([^\"]+)\"', gfx_content):
        name, pth = m.group(1), os.path.join(ROOT, m.group(2))
        if name not in icon_map or ("_gba" in icon_map[name] and "_gba" not in pth):
            icon_map[name] = pth

    for m in re.finditer(r'const u16 gMonPalette_(\w+)\[\]\s*=\s*INCGFX_U16\(\"([^\"]+)\"', gfx_content):
        name, pth = m.group(1), os.path.join(ROOT, m.group(2))
        if name not in pal_map or ("_gba" in pal_map[name] and "_gba" not in pth):
            pal_map[name] = pth

    for m in re.finditer(r'const u16 gMonShinyPalette_(\w+)\[\]\s*=\s*INCGFX_U16\(\"([^\"]+)\"', gfx_content):
        name, pth = m.group(1), os.path.join(ROOT, m.group(2))
        if name not in shiny_pal_map or ("_gba" in shiny_pal_map[name] and "_gba" not in pth):
            shiny_pal_map[name] = pth

    species_to_gfx = {}
    for hfile in sorted(glob.glob(os.path.join(ROOT, "src", "data", "pokemon", "species_info", "gen_*_families.h"))):
        with open(hfile, "r", encoding="utf-8") as f:
            text = f.read()
        for m in re.finditer(r'\[SPECIES_([A-Z0-9_]+)\]\s*=\s*\{(.*?)\n\s*\},', text, re.DOTALL):
            sp_key = m.group(1)
            body = m.group(2)
            f_pic = re.search(r'\.frontPic\s*=\s*gMonFrontPic_(\w+)', body)
            i_pic = re.search(r'\.iconSprite\s*=\s*gMonIcon_(\w+)', body)
            p_pic = re.search(r'\.palette\s*=\s*gMonPalette_(\w+)', body)
            sp_pic = re.search(r'\.shinyPalette\s*=\s*gMonShinyPalette_(\w+)', body)
            species_to_gfx[sp_key] = {
                "front": front_map.get(f_pic.group(1)) if f_pic else None,
                "icon": icon_map.get(i_pic.group(1)) if i_pic else None,
                "pal": pal_map.get(p_pic.group(1)) if p_pic else None,
                "shiny_pal": shiny_pal_map.get(sp_pic.group(1)) if sp_pic else None,
            }

    with open(DEX_PATH, "r", encoding="utf-8") as f:
        dex = json.load(f)

    count = 0
    for mon in dex:
        dex_id = mon["id"]
        key = mon["key"]
        gfx = species_to_gfx.get(key)
        
        front_p = gfx["front"] if gfx else None
        icon_p = gfx["icon"] if gfx else None
        pal_p = gfx["pal"] if gfx else None
        shiny_pal_p = gfx["shiny_pal"] if gfx else None

        # Fallback to direct directory if not found
        if not front_p or not os.path.exists(front_p):
            cand = os.path.join(ROOT, "graphics", "pokemon", key.lower())
            if os.path.exists(cand):
                f_try = os.path.join(cand, "anim_front.png")
                if not os.path.exists(f_try): f_try = os.path.join(cand, "front.png")
                if os.path.exists(f_try): front_p = f_try
                i_try = os.path.join(cand, "icon.png")
                if os.path.exists(i_try): icon_p = i_try
                sp_try = os.path.join(cand, "shiny.pal")
                if os.path.exists(sp_try): shiny_pal_p = sp_try

        # Secondary fallback stripping suffix (e.g. OGERPON_TEAL -> ogerpon)
        if not front_p or not os.path.exists(front_p):
            base_key = key.split("_")[0].lower()
            cand = os.path.join(ROOT, "graphics", "pokemon", base_key)
            if os.path.exists(cand):
                f_try = os.path.join(cand, "anim_front.png")
                if not os.path.exists(f_try): f_try = os.path.join(cand, "front.png")
                if os.path.exists(f_try): front_p = f_try
                i_try = os.path.join(cand, "icon.png")
                if os.path.exists(i_try): icon_p = i_try
                sp_try = os.path.join(cand, "shiny.pal")
                if os.path.exists(sp_try): shiny_pal_p = sp_try

        shiny_pal_bytes = parse_pal(shiny_pal_p)

        # Generate front normal
        if front_p and os.path.exists(front_p):
            res_front = process_png(front_p, None, crop_half_height=True)
            if res_front:
                with open(os.path.join(OUT_DIR, "front", f"{dex_id}.png"), "wb") as f:
                    f.write(res_front)

            # Generate front shiny
            res_front_shiny = process_png(front_p, shiny_pal_bytes, crop_half_height=True)
            if res_front_shiny:
                with open(os.path.join(OUT_DIR, "front_shiny", f"{dex_id}.png"), "wb") as f:
                    f.write(res_front_shiny)

        # Generate icon
        if icon_p and os.path.exists(icon_p):
            res_icon = process_png(icon_p, None, crop_half_height=True)
            if res_icon:
                with open(os.path.join(OUT_DIR, "icon", f"{dex_id}.png"), "wb") as f:
                    f.write(res_icon)

        count += 1

    print(f"Generati con successo tutti gli sprite per {count} Pokémon in {OUT_DIR}!")

if __name__ == "__main__":
    main()
