#!/usr/bin/env python3
import os
import re
import sys
import glob
import struct
import zlib
import subprocess
from collections import Counter
from concurrent.futures import ProcessPoolExecutor

SWIM_DIR = 'Swimming Overworld Sprites/Swimming'
LEVI_DIR = 'Swimming Overworld Sprites/Levitates'
OUT_DIR = 'graphics/swimming_mons'
OUT_HEADER = 'src/data/object_events/swimming_mon_pics.h'

def read_jasc_pal(path):
    with open(path) as f:
        lines = [line.strip() for line in f if line.strip()]
    count = int(lines[2])
    pal = []
    for l in lines[3:3+count]:
        r, g, b = map(int, l.split())
        pal.append((r, g, b))
    return pal

def decode_png_rgba(path):
    with open(path, 'rb') as f:
        data = f.read()
    idx = 8
    idat = bytearray()
    while idx < len(data):
        l, = struct.unpack('>I', data[idx:idx+4])
        c = data[idx+4:idx+8]
        cd = data[idx+8:idx+8+l]
        idx += 12 + l
        if c == b'IHDR':
            width, height, bit_depth, color_type = struct.unpack('>IIBB', cd[:10])
        elif c == b'IDAT':
            idat.extend(cd)
    raw = zlib.decompress(idat)
    bpp = 4
    stride = width * bpp
    pixels = []
    pos = 0
    prev_row = bytearray(stride)
    curr_row = bytearray(stride)
    for y in range(height):
        filter_type = raw[pos]
        pos += 1
        for i in range(stride):
            x_val = raw[pos + i]
            a = curr_row[i - bpp] if i >= bpp else 0
            b = prev_row[i]
            c = prev_row[i - bpp] if i >= bpp else 0
            if filter_type == 0: val = x_val
            elif filter_type == 1: val = (x_val + a) & 0xff
            elif filter_type == 2: val = (x_val + b) & 0xff
            elif filter_type == 3: val = (x_val + ((a + b) >> 1)) & 0xff
            elif filter_type == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                val = (x_val + pr) & 0xff
            curr_row[i] = val
        pos += stride
        prev_row[:] = curr_row
        row_pixels = [tuple(curr_row[x*4 : x*4+4]) for x in range(width)]
        pixels.append(row_pixels)
    return width, height, pixels

def closest_color(rgb, pal):
    best_idx = 1
    best_dist = 999999999
    r1, g1, b1 = rgb
    for i in range(1, len(pal)):
        r2, g2, b2 = pal[i]
        d = (r1 - r2)**2 + (g1 - g2)**2 + (b1 - b2)**2
        if d < best_dist:
            best_dist = d
            best_idx = i
            if d == 0:
                break
    return best_idx

def write_indexed_png(path, width, height, palette, indexed_rows):
    ihdr = struct.pack('>IIBBBBB', width, height, 4, 3, 0, 0, 0)
    plte = bytearray()
    for r, g, b in palette:
        plte.extend([r, g, b])
    while len(plte) < 48:
        plte.extend([0, 0, 0])
    raw = bytearray()
    for row in indexed_rows:
        raw.append(0)
        for x in range(0, width, 2):
            p1 = row[x]
            p2 = row[x+1] if x+1 < width else 0
            raw.append((p1 << 4) | (p2 & 0xf))
    idat = zlib.compress(bytes(raw))
    def chunk(tag, data):
        crc = zlib.crc32(tag + data) & 0xffffffff
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', crc)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', ihdr) + chunk(b'PLTE', plte) + chunk(b'IDAT', idat) + chunk(b'IEND', b''))

def downsample_cell(px, cell_r, cell_c):
    top_y = cell_r * 64
    left_x = cell_c * 64
    grid = []
    for y in range(32):
        row = []
        for x in range(32):
            block = [
                px[top_y + y*2][left_x + x*2],
                px[top_y + y*2][left_x + x*2 + 1],
                px[top_y + y*2 + 1][left_x + x*2],
                px[top_y + y*2 + 1][left_x + x*2 + 1]
            ]
            opaque = [b[:3] for b in block if b[3] > 128]
            if len(opaque) >= 2:
                row.append(Counter(opaque).most_common(1)[0][0])
            elif len(opaque) == 1 and any(b[3] > 200 for b in block):
                row.append(opaque[0])
            else:
                row.append(None)
        grid.append(row)
    return grid

def convert_single_species(args):
    sp_name, src_png, pal_path, out_lz_path = args
    if not os.path.exists(pal_path) or not os.path.exists(src_png):
        return None

    try:
        pal = read_jasc_pal(pal_path)
        pal_map = {rgb: i for i, rgb in enumerate(pal)}
        w, h, px = decode_png_rgba(src_png)

        # 6 standard follower frames downsampled from 64x64 to 32x32:
        # South stand, South step, North stand, North step, West stand, West step
        frame_coords = [(0, 1), (0, 0), (3, 1), (3, 0), (1, 1), (1, 0)]
        frames = [downsample_cell(px, r, c) for r, c in frame_coords]

        rows = []
        for y in range(32):
            row = []
            for f in frames:
                for x in range(32):
                    c = f[y][x]
                    if c is None:
                        row.append(0)
                    elif c in pal_map:
                        row.append(pal_map[c])
                    else:
                        row.append(closest_color(c, pal))
            rows.append(row)

        tmp_png = out_lz_path.replace('.lz', '.tmp.png')
        tmp_4bpp = out_lz_path.replace('.lz', '.tmp.4bpp')

        write_indexed_png(tmp_png, 192, 32, pal, rows)
        subprocess.run(['tools/gbagfx/gbagfx', tmp_png, tmp_4bpp, '-mwidth', '4', '-mheight', '4'], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        subprocess.run(['tools/gbagfx/gbagfx', tmp_4bpp, out_lz_path], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        if os.path.exists(tmp_png): os.remove(tmp_png)
        if os.path.exists(tmp_4bpp): os.remove(tmp_4bpp)

        return sp_name
    except Exception as e:
        print(f"Error converting {sp_name}: {e}", file=sys.stderr)
        return None

def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # 1. Palettes
    species_pals = {}
    with open('src/data/graphics/pokemon.h') as f:
        for line in f:
            m = re.search(r'gOverworldPalette_([A-Za-z0-9_]+)\[\]\s*=\s*INCGFX_U16\(\"([^\"]+)\"', line)
            if m:
                species_pals[m.group(1).upper()] = m.group(2)

    # 2. Available sprites in pack
    all_files = {}
    for d in [SWIM_DIR, LEVI_DIR]:
        if not os.path.exists(d): continue
        for f in os.listdir(d):
            if f.endswith('.png'):
                base = f[:-4]
                norm = re.sub(r'[^A-Z0-9]', '', base.upper())
                all_files[norm] = os.path.join(d, f)

    # Special aliases
    aliases = {
        'NIDORAN_F': 'NIDORAN_femaleE',
        'NIDORAN_M': 'NIDORANmA',
        'HO_OH': 'HOOH',
        'MR_MIME': 'MRMIME',
        'MIME_JR': 'MIMEJR',
        'PORYGON_Z': 'PORYGONZ',
        'FARFETCHD': 'FARFETCHD',
        'SIRFETCHD': 'SIRFETCHD',
        'FLABEBE': 'FLABEBE',
        'TYPE_NULL': 'TYPENULL',
        'JANGMO_O': 'JANGMOO',
        'HAKAMO_O': 'HAKAMOO',
        'KOMMO_O': 'KOMMOO',
        'TAPU_KOKO': 'TAPUKOKO',
        'TAPU_LELE': 'TAPULELE',
        'TAPU_BULU': 'TAPUBULU',
        'TAPU_FINI': 'TAPUFINI',
        'CHI_YU': 'CHIYU',
        'CHIEN_PAO': 'CHIENPAO',
        'WO_CHIEN': 'WOCHIEN',
        'TING_LU': 'TINGLU',
        'GREAT_TUSK': 'GREATTUSK',
        'SCREAM_TAIL': 'SCREAMTAIL',
        'BRUTE_BONNET': 'BRUTEBONNET',
        'FLUTTER_MANE': 'FLUTTERMANE',
        'SLITHER_WING': 'SLITHERWING',
        'SANDY_SHOCKS': 'SANDYSHOCKS',
        'IRON_TREADS': 'IRONTREADS',
        'IRON_BUNDLE': 'IRONBUNDLE',
        'IRON_HANDS': 'IRONHANDS',
        'IRON_JUGULIS': 'IRONJUGULIS',
        'IRON_MOTH': 'IRONMOTH',
        'IRON_THORNS': 'IRONTHORNS',
        'ROARING_MOON': 'ROARINGMOON',
        'IRON_VALIANT': 'IRONVALIANT',
        'WALKING_WAKE': 'WALKINGWAKE',
        'IRON_LEAVES': 'IRONLEAVES',
        'GOUGING_FIRE': 'GOUGINGFIRE',
        'RAGING_BOLT': 'RAGINGBOLT',
        'IRON_BOULDER': 'IRONBOULDER',
        'IRON_CROWN': 'IRONCROWN',
    }

    # 3. Species list from species.h and resolving enum values to avoid collisions
    species_list = []
    resolved = {}
    cur_val = 0
    with open('include/constants/species.h') as f:
        for line in f:
            m = re.match(r'^\s*SPECIES_([A-Z0-9_]+)\s*(?:=\s*([^,\n]+))?', line)
            if m:
                s = m.group(1)
                raw = m.group(2)
                if raw:
                    val = re.sub(r'^SPECIES_', '', raw.strip())
                    if val.isdigit():
                        cur_val = int(val)
                    elif val in resolved:
                        cur_val = resolved[val]
                resolved[s] = cur_val
                cur_val += 1
                if not s.startswith('CUSTOM_') and s not in ['NONE', 'EGG']:
                    species_list.append(s)

    tasks = []
    species_meta = {}

    for sp in species_list:
        norm = re.sub(r'[^A-Z0-9]', '', sp)
        src_png = None
        if norm in all_files:
            src_png = all_files[norm]
        elif sp in aliases and re.sub(r'[^A-Z0-9]', '', aliases[sp].upper()) in all_files:
            src_png = all_files[re.sub(r'[^A-Z0-9]', '', aliases[sp].upper())]
        else:
            clean = re.sub(r'_(ALOLA|GALAR|HISUI|PALDEA|MEGA|MEGA_X|MEGA_Y|GMAX|NORMAL|STANDARD|PLANT|WEST|SPRING|INCARNATE|ORDINARY|ARIA|RED|RED_STRIPED|ALTERED|LAND)$', '', sp)
            clean_norm = re.sub(r'[^A-Z0-9]', '', clean)
            if clean_norm in all_files:
                src_png = all_files[clean_norm]

        if not src_png:
            continue

        pal_path = None
        sp_norm_pal = re.sub(r'[^A-Z0-9]', '', sp)
        for k, p in species_pals.items():
            if re.sub(r'[^A-Z0-9]', '', k) == sp_norm_pal:
                pal_path = p
                break
        if not pal_path:
            clean = re.sub(r'_(ALOLA|GALAR|HISUI|PALDEA|MEGA|MEGA_X|MEGA_Y|GMAX|NORMAL|STANDARD|PLANT|WEST|SPRING|INCARNATE|ORDINARY|ARIA|RED|RED_STRIPED|ALTERED|LAND)$', '', sp)
            clean_norm = re.sub(r'[^A-Z0-9]', '', clean)
            for k, p in species_pals.items():
                if re.sub(r'[^A-Z0-9]', '', k) == clean_norm:
                    pal_path = p
                    break

        if not pal_path or not os.path.exists(pal_path):
            continue

        out_lz_path = os.path.join(OUT_DIR, f"{sp.lower()}.4bpp.lz")
        species_meta[sp] = out_lz_path
        tasks.append((sp, src_png, pal_path, out_lz_path))

    print(f"Converting {len(tasks)} swimming sprites with 2:1 proportional downsampling...")
    with ProcessPoolExecutor() as executor:
        results = list(executor.map(convert_single_species, tasks))

    successful_species = [r for r in results if r is not None]
    print(f"Successfully converted {len(successful_species)} sprites.")

    # 4. Generate C header
    print(f"Writing {OUT_HEADER}...")
    with open(OUT_HEADER, 'w') as f:
        f.write("// Auto-generated by tools/build_swimming_sprites.py - DO NOT EDIT\n\n")
        f.write("#ifndef GUARD_SWIMMING_MON_PICS_H\n")
        f.write("#define GUARD_SWIMMING_MON_PICS_H\n\n")

        for sp in successful_species:
            out_lz = species_meta[sp]
            f.write(f'const u32 gObjectEventPic_Swimming_{sp}[] = INCBIN_U32("{out_lz}");\n')

        f.write("\n")
        for sp in successful_species:
            f.write(f'static const struct SpriteFrameImage sPicTable_Swimming_{sp}[] = {{\n')
            f.write(f'    overworld_ascending_frames(gObjectEventPic_Swimming_{sp}, 4, 4),\n')
            f.write(f'}};\n\n')

        f.write("const struct ObjectEventGraphicsInfo gSwimmingMonGraphicsInfo[NUM_SPECIES] = {\n")
        seen_ids = set()
        for sp in successful_species:
            sp_id = resolved.get(sp)
            if sp_id in seen_ids:
                continue
            seen_ids.add(sp_id)

            f.write(f'    [SPECIES_{sp}] = {{\n')
            f.write(f'        .tileTag = TAG_NONE,\n')
            f.write(f'        .paletteTag = OBJ_EVENT_PAL_TAG_DYNAMIC,\n')
            f.write(f'        .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,\n')
            f.write(f'        .size = 512,\n')
            f.write(f'        .width = 32,\n')
            f.write(f'        .height = 32,\n')
            f.write(f'        .paletteSlot = PALSLOT_NPC_1,\n')
            f.write(f'        .shadowSize = SHADOW_SIZE_M,\n')
            f.write(f'        .inanimate = FALSE,\n')
            f.write(f'        .compressed = OW_GFX_COMPRESS,\n')
            f.write(f'        .tracks = TRACKS_NONE,\n')
            f.write(f'        .oam = &gObjectEventBaseOam_32x32,\n')
            f.write(f'        .subspriteTables = sOamTables_32x32,\n')
            f.write(f'        .anims = sAnimTable_Following,\n')
            f.write(f'        .images = sPicTable_Swimming_{sp},\n')
            f.write(f'    }},\n')
        f.write("};\n\n")

        f.write("static inline const struct ObjectEventGraphicsInfo *SpeciesToSwimmingGraphicsInfo(enum Species species, bool32 shiny, bool32 female)\n")
        f.write("{\n")
        f.write("    if (species < NUM_SPECIES && gSwimmingMonGraphicsInfo[species].images != NULL)\n")
        f.write("        return &gSwimmingMonGraphicsInfo[species];\n")
        f.write("    return NULL;\n")
        f.write("}\n\n")

        f.write("#endif // GUARD_SWIMMING_MON_PICS_H\n")

    print("Done!")

if __name__ == '__main__':
    main()
