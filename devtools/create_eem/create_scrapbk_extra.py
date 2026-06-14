#!/usr/bin/env python3
#
# Extract the floppy-only pre-scrapbook dialog records into an EEM engine
# data file consumed by the CD runtime.

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


MAGIC = b"EEMSBX02"
VERSION = 2
CASE_COUNT = 55
NO_VOICE = 0xFFFF
RESTORED_VOICE_FIRST_CASE = 1
RESTORED_VOICE_LAST_CASE = 24


@dataclass(frozen=True)
class ScrapbookRecord:
    pic_id: int
    pic_x: int
    pic_y: int
    balloon: int
    balloon_x: int
    balloon_y: int
    jake_text: bytes
    jenny_text: bytes
    voice_jake: int = NO_VOICE
    voice_jenny: int = NO_VOICE
    voice_nancy: int = NO_VOICE


def read_u16(data: bytes, offset: int) -> int:
    if offset < 0 or offset + 2 > len(data):
        return 0
    return struct.unpack_from("<H", data, offset)[0]


def c_string(data: bytes, offset: int) -> bytes:
    if offset <= 0 or offset >= len(data):
        return b""
    end = data.find(b"\0", offset)
    if end < 0:
        end = len(data)
    return data[offset:end]


def valid_floppy_record(data: bytes, offset: int) -> bool:
    if offset < 0 or offset + 11 > len(data):
        return False
    return offset + 11 + data[offset + 10] <= len(data)


def sorted_case_numbers(data_dir: Path) -> list[int]:
    out: list[int] = []
    for path in data_dir.glob("M*.BIN"):
        stem = path.stem
        if len(stem) > 1 and stem[1:].isdigit():
            out.append(int(stem[1:]))
    return sorted(out)


def extract_floppy_tail(path: Path) -> list[ScrapbookRecord]:
    data = path.read_bytes()
    if len(data) < 0x14 or read_u16(data, 0) <= 0x100:
        raise ValueError(f"{path} is not an EEM floppy mystery file")

    notes_off = read_u16(data, 0x08)
    solved_off = read_u16(data, 0x12)
    if solved_off <= 0 or solved_off >= len(data):
        raise ValueError(f"{path} has no valid solved chain")

    count = data[solved_off]
    pos = solved_off + 1
    record_offsets: list[int] = []
    for _ in range(count):
        if not valid_floppy_record(data, pos):
            raise ValueError(f"{path} has a malformed solved-chain record at 0x{pos:04x}")
        record_offsets.append(pos)
        pos += 11 + data[pos + 10]

    if len(record_offsets) < 3:
        return []

    records: list[ScrapbookRecord] = []
    for offset in record_offsets[-3:]:
        text_count = data[offset + 10]
        if text_count != 1:
            raise ValueError(
                f"{path} tail record at 0x{offset:04x} has {text_count} text entries"
            )

        note_idx = data[offset + 11] & 0x7F
        note_entry = notes_off + note_idx * 7
        if note_entry + 6 > len(data):
            raise ValueError(f"{path} tail record references invalid note {note_idx}")

        records.append(ScrapbookRecord(
            pic_id=read_u16(data, offset + 0),
            pic_x=read_u16(data, offset + 2),
            pic_y=data[offset + 4],
            balloon=data[offset + 5],
            balloon_x=read_u16(data, offset + 6),
            balloon_y=data[offset + 8],
            jake_text=c_string(data, read_u16(data, note_entry + 2)),
            jenny_text=c_string(data, read_u16(data, note_entry + 4)),
        ))

    return records


def sdx_entry_count(cd_dir: Path, case_num: int) -> int:
    path = cd_dir / f"M{case_num}.SDX"
    if not path.exists():
        return 0
    return path.stat().st_size // 12


def valid_cd_clue_block(data: bytes, offset: int) -> bool:
    if offset in (0, 0xFFFF) or offset + 4 > len(data):
        return False
    count = read_u16(data, offset)
    return 0 < count <= 64 and offset + 4 + count * 62 <= len(data)


def add_voice_ref(used: set[int], raw_voice: int, max_entries: int) -> None:
    if raw_voice in (0, 0xFFFF):
        return
    if raw_voice <= max_entries:
        used.add(raw_voice - 1)


def collect_cd_voice_usage(
    cd_dir: Path,
    case_num: int,
) -> tuple[set[int], list[int], list[int]]:
    path = cd_dir / f"M{case_num}.BIN"
    if not path.exists():
        return set(), [], []

    data = path.read_bytes()
    max_entries = sdx_entry_count(cd_dir, case_num)
    used: set[int] = set()
    solved_jenny: list[int] = []
    solved_jake: list[int] = []

    def parse_clue_block(offset: int, solved: bool = False) -> None:
        if not valid_cd_clue_block(data, offset):
            return
        count = read_u16(data, offset)
        for entry_idx in range(count):
            entry = offset + 4 + entry_idx * 62
            voice_jenny = read_u16(data, entry + 0x18)
            voice_jake = read_u16(data, entry + 0x1A)

            # Original _DisplayClue gates voice playback on the Jenny/default
            # slot; a Jake-only value with a zero Jenny slot is not played.
            if voice_jenny in (0, 0xFFFF):
                continue
            add_voice_ref(used, voice_jenny, max_entries)
            if solved and voice_jenny <= max_entries:
                solved_jenny.append(voice_jenny - 1)

            add_voice_ref(used, voice_jake, max_entries)
            if solved and voice_jake not in (0, 0xFFFF) and voice_jake <= max_entries:
                solved_jake.append(voice_jake - 1)

    parse_clue_block(read_u16(data, 0x00) + 4)
    parse_clue_block(read_u16(data, 0x10), solved=True)

    site_index_off = read_u16(data, 0x06)
    num_sites = read_u16(data, 0x14)
    for site_idx in range(min(num_sites, 128)):
        site_index = site_index_off + site_idx * 6
        if site_index + 6 > len(data):
            break
        site_data_off = read_u16(data, site_index)
        parse_clue_block(read_u16(data, site_index + 2))

        if site_data_off + 8 > len(data):
            continue
        hotspot_count = read_u16(data, site_data_off + 6)
        hotspot_table = read_u16(data, site_index + 4)
        for hot_idx in range(min(hotspot_count, 300)):
            hotspot = hotspot_table + hot_idx * 14
            if hotspot + 14 > len(data):
                break
            parse_clue_block(read_u16(data, hotspot + 8))

    gallery_off = read_u16(data, 0x0C)
    num_suspects = read_u16(data, 0x1A)
    if gallery_off and gallery_off + num_suspects * 0x46 <= len(data):
        for suspect_idx in range(num_suspects):
            entry = gallery_off + suspect_idx * 0x46
            add_voice_ref(used, read_u16(data, entry + 4), max_entries)
            add_voice_ref(used, read_u16(data, entry + 6), max_entries)

    kd_text_off = read_u16(data, 0x0E)
    if kd_text_off and kd_text_off + 0x12 < len(data):
        digital = kd_text_off + 0x12
        for kdspeak in (0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11):
            for slot in (kdspeak * 2 + 1, kdspeak * 2 + 2):
                raw_voice = read_u16(data, digital + slot * 2)
                add_voice_ref(used, raw_voice, max_entries)

    return used, solved_jenny, solved_jake


def contiguous_unused_after(used: set[int], sequence: list[int], max_entries: int) -> list[int]:
    if not sequence:
        return []

    out: list[int] = []
    entry = sequence[-1] + 1
    while entry < max_entries and entry not in used:
        out.append(entry)
        entry += 1
    return out


def sound_duration(cd_dir: Path, case_num: int, entry: int) -> float:
    sdx_path = cd_dir / f"M{case_num}.SDX"
    sdb_path = cd_dir / f"M{case_num}.SDB"
    if not sdx_path.exists() or not sdb_path.exists():
        return 0.0

    sdx_data = sdx_path.read_bytes()
    if entry < 0 or entry * 12 + 12 > len(sdx_data):
        return 0.0

    offset = struct.unpack_from("<I", sdx_data, entry * 12)[0]
    uncompressed_size = struct.unpack_from("<I", sdx_data, entry * 12 + 8)[0]
    with sdb_path.open("rb") as sdb:
        sdb.seek(offset)
        tc_raw = sdb.read(1)
    if not tc_raw:
        return 0.0

    tc = tc_raw[0]
    sample_rate = 1000000 // (256 - tc) if tc < 0xFF else 44100
    return uncompressed_size / sample_rate if sample_rate else 0.0


def find_nancy_voice(
    cd_dir: Path,
    case_num: int,
    used: set[int],
    partner_voices: set[int],
    first_partner_voice: int,
) -> int:
    max_entries = sdx_entry_count(cd_dir, case_num)

    for entry in range(max(0, first_partner_voice), max_entries):
        if entry in used or entry in partner_voices:
            continue
        if sound_duration(cd_dir, case_num, entry) < 1.2:
            continue
        return entry

    return NO_VOICE


def attach_cd_voices(cd_dir: Path, case_num: int,
                     records: list[ScrapbookRecord]) -> list[ScrapbookRecord]:
    if len(records) < 3:
        return records
    if not RESTORED_VOICE_FIRST_CASE <= case_num <= RESTORED_VOICE_LAST_CASE:
        return records

    used, solved_jenny, solved_jake = collect_cd_voice_usage(cd_dir, case_num)
    max_entries = sdx_entry_count(cd_dir, case_num)
    jenny_candidates = contiguous_unused_after(used, solved_jenny, max_entries)
    jake_candidates = contiguous_unused_after(used, solved_jake, max_entries)

    voice_jenny = jenny_candidates[:2] if len(jenny_candidates) >= 2 else []
    voice_jake = jake_candidates[:2] if len(jake_candidates) >= 2 else []
    partner_voices = set(voice_jenny) | set(voice_jake)
    voice_nancy = NO_VOICE
    if len(voice_jenny) >= 2 and len(voice_jake) >= 2:
        first_partner_voice = min(voice_jenny[0], voice_jake[0])
        voice_nancy = find_nancy_voice(cd_dir, case_num, used,
                                       partner_voices, first_partner_voice)

    out: list[ScrapbookRecord] = []
    for idx, record in enumerate(records):
        extra_idx = idx - 1
        out.append(ScrapbookRecord(
            pic_id=record.pic_id,
            pic_x=record.pic_x,
            pic_y=record.pic_y,
            balloon=record.balloon,
            balloon_x=record.balloon_x,
            balloon_y=record.balloon_y,
            jake_text=record.jake_text,
            jenny_text=record.jenny_text,
            voice_jake=voice_jake[extra_idx] if 0 <= extra_idx < len(voice_jake) else NO_VOICE,
            voice_jenny=voice_jenny[extra_idx] if 0 <= extra_idx < len(voice_jenny) else NO_VOICE,
            voice_nancy=voice_nancy if idx == 0 else NO_VOICE,
        ))
    return out


def encode_records(cases: list[list[ScrapbookRecord]]) -> bytes:
    table_size = CASE_COUNT * 8
    header = bytearray()
    header += MAGIC
    header += struct.pack("<HH", VERSION, CASE_COUNT)
    table = bytearray(table_size)
    body = bytearray()

    for case_num, records in enumerate(cases):
        if not records:
            continue

        offset = len(header) + table_size + len(body)
        struct.pack_into("<IHH", table, case_num * 8, offset, len(records), 0)
        for record in records:
            if len(record.jake_text) > 0xFFFF or len(record.jenny_text) > 0xFFFF:
                raise ValueError(f"case {case_num} contains a text string longer than 65535 bytes")
            body += struct.pack(
                "<HHBBHBBHHHHH",
                record.pic_id,
                record.pic_x,
                record.pic_y,
                record.balloon,
                record.balloon_x,
                record.balloon_y,
                0,
                record.voice_jake,
                record.voice_jenny,
                record.voice_nancy,
                len(record.jake_text),
                len(record.jenny_text),
            )
            body += record.jake_text
            body += record.jenny_text

    return bytes(header + table + body)


def create_scrapbook_extra(floppy_dir: Path, cd_dir: Path, out_path: Path) -> None:
    cases: list[list[ScrapbookRecord]] = [[] for _ in range(CASE_COUNT)]
    voiced_cases = 0
    nancy_cases = 0

    for case_num in sorted_case_numbers(floppy_dir):
        if case_num == 0 or case_num >= CASE_COUNT:
            continue
        records = extract_floppy_tail(floppy_dir / f"M{case_num}.BIN")
        records = attach_cd_voices(cd_dir, case_num, records)
        if any(record.voice_jake != NO_VOICE or record.voice_jenny != NO_VOICE
               for record in records):
            voiced_cases += 1
        if any(record.voice_nancy != NO_VOICE for record in records):
            nancy_cases += 1
        cases[case_num] = records

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(encode_records(cases))
    print(f"Wrote {out_path}")
    print(f"Cases with text: {sum(1 for records in cases if records)}")
    print(f"Cases with mapped CD voice clips: {voiced_cases}")
    print(f"Cases with mapped Nancy clips: {nancy_cases}")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Create EEM SCRAPBK_EXTRA.ANI from floppy mystery data."
    )
    parser.add_argument(
        "--floppy-dir",
        type=Path,
        default=Path("../../eem-full-game/floppy/EAKIDS/EEM"),
        help="Directory containing English floppy M*.BIN data.",
    )
    parser.add_argument(
        "--cd-dir",
        type=Path,
        default=Path("../../eem-full-game/cd"),
        help="Directory containing CD M*.BIN/M*.SDX data for voice mapping.",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=Path("files/eem/SCRAPBK_EXTRA.ANI"),
        help="Output file inside devtools/create_eem/files/eem.",
    )
    args = parser.parse_args()

    create_scrapbook_extra(args.floppy_dir, args.cd_dir, args.out)


if __name__ == "__main__":
    main()
