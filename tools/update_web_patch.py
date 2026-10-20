#!/usr/bin/env python3
"""
Updates web/patch/kaizo.gba.gz and web/patch/meta.json from pokefirered.gba
"""
import os
import sys
import gzip
import shutil
import zlib
import hashlib
import json
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GBA_SRC = os.path.join(ROOT, "pokefirered.gba")
PATCH_DIR = os.path.join(ROOT, "web", "patch")
GZ_DST = os.path.join(PATCH_DIR, "kaizo.gba.gz")
META_DST = os.path.join(PATCH_DIR, "meta.json")

def main():
    if not os.path.exists(GBA_SRC):
        print(f"ERRORE: {GBA_SRC} non trovato! Esegui prima 'make firered'.", file=sys.stderr)
        sys.exit(1)

    os.makedirs(PATCH_DIR, exist_ok=True)

    print(f"Lettura {GBA_SRC}...")
    with open(GBA_SRC, "rb") as f:
        data = f.read()

    print(f"Dimensione ROM: {len(data):,} bytes ({len(data)/1024/1024:.1f} MB)")
    print("Compressione gzip...")
    with gzip.open(GZ_DST, "wb", compresslevel=9) as f_out:
        f_out.write(data)

    gz_size = os.path.getsize(GZ_DST)
    print(f"ROM compressa: {gz_size:,} bytes ({gz_size/1024/1024:.1f} MB)")

    crc32_val = zlib.crc32(data) & 0xffffffff
    sha1_val = hashlib.sha1(data).hexdigest()
    md5_val = hashlib.md5(data).hexdigest()

    # Find gKaizoWebPrerunConfig offset
    prerun_offset = 0
    elf_file = os.path.join(ROOT, "pokefirered.elf")
    if os.path.exists(elf_file):
        try:
            import subprocess
            out = subprocess.check_output(["arm-none-eabi-nm", elf_file], text=True)
            for line in out.splitlines():
                if "gKaizoWebPrerunConfig" in line:
                    addr = int(line.split()[0], 16)
                    prerun_offset = addr - 0x08000000
                    break
        except Exception as e:
            print("Avviso: Impossibile leggere nm su elf:", e)

    meta = {
        "version": "1.0",
        "gameTitle": "Pokémon KaizoRogue",
        "buildTime": time.strftime("%Y-%m-%d %H:%M:%S"),
        "targetSize": len(data),
        "targetCrc32": f"{crc32_val:08x}",
        "targetSha1": sha1_val,
        "targetMd5": md5_val,
        "prerunOffset": prerun_offset,
        "expectedBase": {
            "title": "POKEMON FIRE",
            "code": "BPRE",
            "maker": "01",
            "size": 16777216,
            "cleanCrc32": "dd88761c",
            "cleanSha1": "e26ee0d44e809351e8ce2d73a7200222aa408d43"
        }
    }

    with open(META_DST, "w", encoding="utf-8") as f:
        json.dump(meta, f, indent=2)

    print(f"Meta aggiornato in {META_DST}")
    print("Tutto pronto!")

if __name__ == "__main__":
    main()
