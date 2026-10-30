#!/usr/bin/env python3
#
# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""Extract editable Hollywood Monsters translations and build hollywood.dat (ZIP)."""

import argparse
from datetime import datetime, timezone
import hashlib
from pathlib import Path
import struct
import subprocess
import tempfile
import zipfile


SOURCE_MD5 = "44b549a5ccc2c970d4aff469d9baa47b"
PATCH_MD5 = "e9fdadc6eaaf661c25a5962c8dfde8a6"
EXE_MD5 = "6fe8665180fda1a7ef6250ead7c1da50"
KEY_SIZE = 321
SMALL_ROW_SIZE = 41
STAGE_CUE_SIZE = 100000
STATIC_CUE_SIZE = 20000
SCENE_WIDTH = 1024
SCENE_HEIGHT = 480

# Archive fingerprints and image chunks: (layout, width, height, palette chunk).
# Block-list dimensions and placement are read from the image itself.
GRAPHICS = {
    "B00": ("c0b181579bddf0cb4f7b134d88ce26d6", "82f77daa0109b304b7922053b4f11c08",
            {3: ("blocks", 0, 0, 1)}),
    "C00": ("0ef72ddd9503fb7a4f739c9b8f1d3dd4", "7930e2841aa87ad470bf03956c5511bd",
            {2: ("blocks", 0, 0, 1)}),
    "D00": ("3b1ebdcacf6367bd4de630eb05e44dec", "0dd293ba1bede6b766ab0c3e2b69ac38",
            {2: ("blocks", 0, 0, 1)}),
    "D06": ("9c8d9724e7b75933e8209ce33567621d", "a72738f84f3cca5c8eb295f6d126833a",
            {20: ("blocks", 0, 0, 1)}),
    "E01": ("00358f2f7f839cbf4709641b801c4b62", "41df4fb41533dfec74dd1c7ea822e923",
            {27: ("blocks", 0, 0, 1)}),
    "E13": ("bf717ed29f52c1fb50f3f40b92043b86", "cb8398d39768b68f437dfccefd1a54c5",
            {10: ("pixels", 104, 240, 1)}),
    "H00": ("9e8f82bf9825f21303682ee836ec818f", "a32a4070a09404994d74cfe5a425dbdf",
            {2: ("blocks", 0, 0, 1)}),
    "I04": ("9331e5153e9ec087d2ad31fc475ddc2a", "043f859f4cbef1c4b30d8e17f97cd00e",
            {1: ("pixels", 1024, 480, 0), 2: ("pixels", 200, 93, 0),
             3: ("pixels", 200, 93, 0), 4: ("pixels", 200, 93, 0),
             5: ("pixels", 200, 93, 0), 6: ("pixels", 200, 93, 0),
             16: ("pixels", 1024, 480, 15)}),
    "I06": ("3bccb3dbccab50a5e35d122abdb23ce3", "bb9d8e7fcc51d6a03c2c3a40d71ebb65",
            {5: ("blocks", 0, 0, 0)}),
    "I10": ("f0a40dd00972254e1900223c773605dd", "655ad8babdffea840117141a123aca3d",
            {16: ("blocks", 0, 0, 1), 17: ("blocks", 0, 0, 1)}),
    "I14": ("9ab222857e8319d674bdcabcdbf9cb85", "808d03e38bbe82131b082760347af0cc",
            {2: ("blocks", 0, 0, 1)}),
}

# Only translated game strings, excluding code, filenames and runtime messages.
INTERFACE_OFFSETS = {
    "passageName": 0xfeb6c,
    "changeDrink": 0xfeb78,
    "openTap": 0xfeb88,
    "stopMakingCocktails": 0xfeb98,
    "loading": 0xfebb4,
    "optionsText": 0x116dd4,
    "optionsVoice": 0x116dd9,
    "optionsBoth": 0x116de0,
    "optionsSave": 0x116de8,
    "optionsLoad": 0x116df0,
    "optionsQuit": 0x116df8,
    "optionsPlay": 0x116e00,
    "optionsMusicVolume": 0x116e08,
    "optionsSfxVolume": 0x116e18,
    "optionsVoiceVolume": 0x116e24,
    "optionsTextSpeed": 0x116e32,
    "optionsQuitYes": 0x116e40,
    "optionsQuitNo": 0x116e44,
    "optionsQuitPrompt": 0x116e48,
    "savePosition": 0x116e58,
    "saveDescription": 0x116e78,
    "saving": 0x116e94,
    "loadPosition": 0x116ea0,
    "loadingSave": 0x116eb8,
    "saveNoSpace1": 0x116ec4,
    "saveNoSpace2": 0x116ee8,
    "noSavegames": 0x116f14,
    "optionsMusicOn": 0x116f30,
    "optionsMusicOff": 0x116f3c,
    "optionsSfxOn": 0x116f48,
    "optionsSfxOff": 0x116f50,
    "optionsTestOn": 0x116f58,
    "optionsTestOff": 0x116f60,
    "relationWith": 0x1185ce,
    "relationTo": 0x1185d4,
    "wideCoffinName": 0x1186e4,
    "girlInJacuzzi": 0x1187f8,
}


def read_checked(path, expected_md5):
    data = path.read_bytes()
    if hashlib.md5(data).hexdigest() != expected_md5:
        raise ValueError(f"Unsupported input: {path}")
    return data


def quote_text(raw):
    text = raw.decode("ascii", errors="surrogateescape")
    text = text.replace("%", "'").replace("\udcf0", "\u201c").replace("\udcf1", "\u201d")
    if any((ord(c) < 32 and c not in "\t\v") or 0xdc80 <= ord(c) <= 0xdcff for c in text):
        raise ValueError("Unexpected character in translated text")
    restored = text.replace("'", "%").replace("\u201c", "\udcf0").replace("\u201d", "\udcf1")
    if restored.encode("ascii", errors="surrogateescape") != raw:
        raise ValueError("Text cannot be converted without changing its bytes")
    text = text.replace("\\", "\\\\").replace('"', '\\"').replace("\t", "\\t").replace("\v", "\\v")
    return '"' + text + '"'


def extract_rows(patched, key, offset, count, row_size, first_id, section):
    rows = []
    for record_id in range(first_id, first_id + count):
        encoded = patched[offset:offset + row_size]
        if len(encoded) != row_size:
            raise ValueError("Truncated text row")
        decoded = bytes((value - key[column]) & 255 for column, value in enumerate(encoded))
        end = decoded.find(b"\0")
        if end < 0:
            raise ValueError("Unterminated text row")
        rows += [f'msgid "{section}.{record_id}"', f"msgstr {quote_text(decoded[:end])}", ""]
        offset += row_size
    return rows, offset


def extract_text(patched, original):
    key = patched[:KEY_SIZE]
    offsets = struct.unpack_from("<1021I", patched, KEY_SIZE)
    sections = []
    spans = []
    for index, cue_offset in ((0, 0x1135), (50, 0x5f58)):
        small, large = struct.unpack_from("<BH", patched, cue_offset + STATIC_CUE_SIZE)
        sections.append((f"static{index}", offsets[index], small, large, 1))
        spans.append((offsets[index], offsets[index] + small * SMALL_ROW_SIZE + large * KEY_SIZE))

    position = spans[0][1]
    while position < offsets[50]:
        indices = [index for index, offset in enumerate(offsets) if offset == position]
        if len(indices) != 1:
            raise ValueError("Stage boundary does not match offset table")
        small, large = struct.unpack_from("<BH", patched, position + STAGE_CUE_SIZE)
        start = position + STAGE_CUE_SIZE + 3
        position = start + small * SMALL_ROW_SIZE + large * KEY_SIZE
        sections.append((f"stage{indices[0]}", start, small, large, 500))
        spans.append((start, position))
    if position != offsets[50] or spans[1][1] != len(patched):
        raise ValueError("Unexpected data outside text sections")

    # Every change must be in text rows. Keep all cue tables in the original file.
    previous = 0
    for start, end in sorted(spans):
        if start < previous or patched[previous:start] != original[previous:start]:
            raise ValueError("Patch changes non-text data in RESOURCE.003")
        previous = end
    if patched[previous:] != original[previous:]:
        raise ValueError("Patch changes trailing non-text data")

    output = []
    row_count = 0
    for name, start, small, large, first_large in sections:
        small_rows, position = extract_rows(patched, key, start, small, SMALL_ROW_SIZE, 1, f"{name}-small")
        large_rows, position = extract_rows(patched, key, position, large, KEY_SIZE, first_large, f"{name}-large")
        output += small_rows + large_rows
        row_count += small + large
    return output, row_count


def extract_interface(executable):
    offsets = dict(INTERFACE_OFFSETS)
    offsets["action0"] = offsets["relationTo"]
    for index in range(1, 9):
        offsets[f"action{index}"] = 0x1185dc + (index - 1) * 12
    for index, offset in enumerate((0x118b8c, 0x118b84, 0x118b7c, 0x118b74,
                                   0x118b6c, 0x118b65, 0x118b60), 2):
        offsets[f"verb{index}"] = offset
    output = []
    for name, offset in offsets.items():
        end = executable.index(b"\0", offset)
        output += [f'msgid "interface.{name}"', f"msgstr {quote_text(executable[offset:end])}", ""]
    return output, len(offsets)


def read_chunks(data):
    offsets = struct.unpack_from("<40I", data)
    sizes = struct.unpack_from("<40I", data, 160)
    chunks = []
    for offset, size in zip(offsets, sizes):
        if size and (offset < 320 or offset + size > len(data)):
            raise ValueError("Invalid image archive chunk")
        chunks.append(data[offset:offset + size] if size else b"")
    return chunks


def decode_blocks(data):
    count = struct.unpack_from("<H", data)[0]
    position = 2
    rows = []
    for _ in range(count):
        x, y, width = struct.unpack_from("<HHH", data, position)
        position += 6
        if width == 0 or x + width > SCENE_WIDTH or y >= SCENE_HEIGHT or position + width > len(data):
            raise ValueError("Invalid image block")
        rows.append((x, y, data[position:position + width]))
        position += width
    if not rows or position != len(data):
        raise ValueError("Unexpected data outside image blocks")
    return rows


def crop_original_blocks(data, x, y, width, height):
    canvas = bytearray(SCENE_WIDTH * SCENE_HEIGHT)
    for block_x, block_y, pixels in decode_blocks(data):
        start = block_y * SCENE_WIDTH + block_x
        canvas[start:start + len(pixels)] = pixels
    return b"".join(canvas[row * SCENE_WIDTH + x:row * SCENE_WIDTH + x + width]
                    for row in range(y, y + height))


def remap_palette(pixels, original, translated):
    if len(original) != 768 or len(translated) != 768:
        raise ValueError("Palette replacement requires 256 colors")
    mapping = bytearray(range(256))
    for index in range(256):
        color = original[index * 3:index * 3 + 3]
        if color == translated[index * 3:index * 3 + 3]:
            continue
        for replacement in range(256):
            if color == translated[replacement * 3:replacement * 3 + 3]:
                mapping[index] = replacement
                break
    return bytes(mapping[pixel] for pixel in pixels)


def encode_bitmap(pixels, width, height, palette):
    if len(pixels) != width * height or len(palette) > 768 or any(value > 63 for value in palette):
        raise ValueError("Invalid indexed image")
    palette = palette.ljust(768, b"\0")
    colors = b"".join(bytes((palette[i + 2] * 4, palette[i + 1] * 4, palette[i] * 4, 0))
                      for i in range(0, 768, 3))
    padding = b"\0" * (-width % 4)
    rows = b"".join(pixels[row * width:(row + 1) * width] + padding
                    for row in range(height - 1, -1, -1))
    data_offset = 14 + 40 + len(colors)
    header = struct.pack("<2sIHHI", b"BM", data_offset + len(rows), 0, 0, data_offset)
    header += struct.pack("<IiiHHIIiiII", 40, width, height, 1, 8, 0, len(rows), 0, 0, 256, 256)
    return header + colors + rows


def extract_images(patch_directory, original_directory):
    image_index = bytearray(struct.pack("<4sHH", b"HMTI", 1, len(GRAPHICS)))
    image_index += SOURCE_MD5.encode("ascii")
    files = {}
    for extension, (source_md5, patch_md5, images) in GRAPHICS.items():
        name = f"RESOURCE.{extension}"
        original = read_chunks(read_checked(original_directory / name, source_md5))
        translated = read_chunks(read_checked(patch_directory / name, patch_md5))
        entries = []
        allowed = set(images)
        for _, _, _, palette in images.values():
            if original[palette] != translated[palette]:
                allowed.add(palette)
        if any(before != after for index, (before, after) in enumerate(zip(original, translated))
               if index not in allowed):
            raise ValueError(f"Unexpected non-image changes in {name}")

        for index, (layout, width, height, palette) in images.items():
            file_name = f"images/{extension.lower()}-{index}.bmp"
            mask_name = f"images/{extension.lower()}-{index}.mask"
            x = y = 0
            palette_chunk = 255
            before, after = original[index], translated[index]
            if layout == "blocks":
                rows = decode_blocks(after)
                x, y, first_row = rows[0]
                width, height = len(first_row), len(rows)
                if any((row_x, row_y, len(pixels)) != (x, y + row, width)
                       for row, (row_x, row_y, pixels) in enumerate(rows)):
                    raise ValueError(f"Translated image is not rectangular: {name}/{index}")
                before = crop_original_blocks(before, x, y, width, height)
                after = b"".join(pixels for _, _, pixels in rows)
            if len(before) != width * height or len(after) != width * height:
                raise ValueError(f"Unexpected image dimensions: {name}/{index}")

            if original[palette] != translated[palette]:
                if layout != "pixels":
                    raise ValueError("Palette remapping is only supported for full pixel images")
                before = remap_palette(before, original[palette], translated[palette])
                palette_chunk = palette
                entries.append(struct.pack("<BBBBHH", palette, index, 2, 255, 0, 0))

            # Keep unchanged pixels in the original game, including palette-only remaps.
            mask = bytes(old != new for old, new in zip(before, after))
            pixels = bytes(new if changed else 0 for new, changed in zip(after, mask))
            if bytes(new if changed else old for old, new, changed in zip(before, pixels, mask)) != after:
                raise ValueError("Image overlay cannot reproduce the translated pixels")
            files[file_name] = encode_bitmap(pixels, width, height, translated[palette])
            files[mask_name] = mask
            # Target chunk, bitmap chunk, layout, palette chunk, x, y.
            entries.append(struct.pack("<BBBBHH", index, index, 0 if layout == "blocks" else 1,
                                       palette_chunk, x, y))
        image_index += extension.encode("ascii") + source_md5.encode("ascii")
        image_index += struct.pack("<B", len(entries)) + b"".join(entries)
    return bytes(image_index), files


def extract_translation(patch_directory, original_text_archive, output):
    patched = read_checked(patch_directory / "RESOURCE.003", PATCH_MD5)
    executable = read_checked(patch_directory / "MONSTERS.EXE", EXE_MD5)
    original = read_checked(original_text_archive, SOURCE_MD5)
    rows, row_count = extract_text(patched, original)
    interface, interface_count = extract_interface(executable)
    images, image_files = extract_images(patch_directory, original_text_archive.parent)
    attribution = [
        "# Hollywood Monsters - English fan translation",
        "# Translation v1.0.0 by Nikos Daskos (ShinjiGR).",
        "# Used with permission. Retain this attribution in derivative works.",
    ]
    po = [
        *attribution,
        "# Identifiers refer to game records; the original Spanish script is not included.",
        "# Text must fit the original font and record sizes (40 or 320 bytes).",
        "", 'msgid ""', 'msgstr ""',
        '"Project-Id-Version: Hollywood Monsters fan translation 1.0.0\\n"',
        f'"PO-Revision-Date: {datetime.now(timezone.utc):%Y-%m-%d %H:%M%z}\\n"',
        '"Last-Translator: Nikos Daskos (ShinjiGR)\\n"',
        '"Language-Team: English\\n"',
        '"Language: en\\n"',
        '"X-Language-name: English\\n"',
        '"MIME-Version: 1.0\\n"',
        '"Content-Type: text/plain; charset=UTF-8\\n"',
        '"Content-Transfer-Encoding: 8bit\\n"',
        '"Plural-Forms: nplurals=2; plural=(n != 1);\\n"',
        "", *interface, *rows,
    ]
    output.mkdir()
    (output / "en.po").write_text("\n".join(po), encoding="utf-8")
    (output / "monsters.en.idx").write_bytes(images)
    (output / "credits.txt").write_text("\n".join(line[2:] for line in attribution) + "\n", encoding="utf-8")
    for name, data in image_files.items():
        path = output / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    print(f"Extracted {row_count} text rows and {interface_count} interface strings into {output / 'en.po'}")
    print(f"Extracted {len(image_files) // 2} image overlays, including the diary palette")


def pack_translation(source, output, compiler, po=None):
    files = {name: (source / name).read_bytes() for name in ("monsters.en.idx", "credits.txt")}
    if files["monsters.en.idx"][:6] != b"HMTI\x01\x00":
        raise ValueError("Unsupported image index")
    for extension, (_, _, images) in GRAPHICS.items():
        for index in images:
            for suffix in ("bmp", "mask"):
                name = f"images/{extension.lower()}-{index}.{suffix}"
                files[name] = (source / name).read_bytes()

    with tempfile.TemporaryDirectory() as directory:
        directory = Path(directory)
        catalogs = []
        # Avoid TranslationManager's development override from ScummVM's GUI po/ directory.
        for path in sorted(po or source.glob("*.po")):
            catalog_source = directory / f"monsters-{path.stem}.po"
            catalog_source.write_bytes(path.read_bytes())
            catalogs.append(str(catalog_source))
        if not catalogs:
            raise ValueError("No PO catalogs found")
        catalog = directory / "monsters.dat"
        subprocess.run([str(compiler.resolve()), str(catalog), *catalogs], check=True)
        files["monsters.dat"] = catalog.read_bytes()
    with zipfile.ZipFile(output, "w") as destination:
        for name, data in sorted(files.items()):
            destination.writestr(zipfile.ZipInfo(name), data, compress_type=zipfile.ZIP_DEFLATED, compresslevel=9)
    print(f"{output}: {output.stat().st_size} bytes (compiled text catalog and image overlays)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    extract = commands.add_parser("extract", help="Extract editable PO text, image overlays and metadata")
    extract.add_argument("patch_directory", type=Path)
    extract.add_argument("original_text_archive", type=Path, help="Unmodified Spanish RESOURCE.003")
    extract.add_argument("output", type=Path, help="New source directory")
    pack = commands.add_parser("pack", help="Compile PO text and rebuild hollywood.dat as a ZIP archive")
    pack.add_argument("source", type=Path, help="Directory produced by extract")
    pack.add_argument("output", type=Path, help="Output hollywood.dat (replaced if it exists)")
    pack.add_argument("--po", type=Path, nargs="+", help="PO files to compile instead of SOURCE/*.po")
    pack.add_argument("--compiler", type=Path,
                      default=Path(__file__).resolve().parent / "create_translations/create_translations")
    args = parser.parse_args()
    try:
        if args.command == "extract":
            extract_translation(args.patch_directory, args.original_text_archive, args.output)
        else:
            pack_translation(args.source, args.output, args.compiler, args.po)
    except (OSError, ValueError, struct.error, subprocess.CalledProcessError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
