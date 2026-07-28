#!/usr/bin/env python3
"""
ScummVM - Graphic Adventure Engine Emulator
devtools/create_buried/create_buried_subtitles.py

Build tool for generating dists/engine-data/buried_subtitles.dat from
devtools/create_buried/subtitles.json.

1. HEADER (8 bytes):
   - Magic Signature (4 bytes): ASCII string "BURS" (0x42555253).
   - Format Version  (2 bytes): uint16 BE, e.g. version 1.
   - Track Count     (2 bytes): uint16 BE, total number of subtitle tracks (N).

2. TABLE OF CONTENTS (TOC) (N x 22 bytes):
   Located immediately after the header at byte offset 8.
   For each track (i = 0 .. N-1):
   - Media ID       (16 bytes): ASCII string, fixed-length, null-padded (\x00).
   - Payload Offset  (4 bytes): uint32 BE, absolute byte offset in file.
   - Card Count      (2 bytes): uint16 BE, number of subtitle cards (M).

3. TRACK PAYLOAD DATA:
   Located at the byte offset specified in the TOC for each track.
   For each card entry in the track (j = 0 .. M-1):
   - Start Time      (4 bytes): uint32 BE, start timestamp in milliseconds.
   - End Time        (4 bytes): uint32 BE, end timestamp in milliseconds.
   - Speaker Length  (2 bytes): uint16 BE, length of speaker name in bytes (L_spk).
   - Speaker Name    (L_spk B ): UTF-8 encoded string (e.g. "Arthur").
   - Text Length     (2 bytes): uint16 BE, length of subtitle text in bytes (L_txt).
   - Text String     (L_txt B ): UTF-8 encoded dialogue string.
===============================================================================
"""

import argparse
import json
import os
import struct
import sys

SUBTITLE_DATA_VERSION = 1


def pack_dat(input_json_path, output_dat_path):
    """
    Parses consolidated JSON subtitle data and serializes it into the binary
    buried_subtitles.dat format expected by ScummVM's SubtitleManager.
    """
    with open(input_json_path, "r", encoding="utf-8") as f:
        tracks = json.load(f)

    # Filter out empty entries and sort tracks alphabetically by Media ID
    valid_tracks = []
    for track in tracks:
        media_id = track.get("media_id", "").strip()
        subtitles = track.get("subtitles", [])
        if media_id and subtitles:
            valid_tracks.append((media_id, subtitles))

    valid_tracks.sort(key=lambda x: x[0])

    num_tracks = len(valid_tracks)

    # Header calculations:
    # 4 bytes (Magic "BURS") + 2 bytes (Version) + 2 bytes (NumTracks) = 8 bytes
    header_size = 8
    # TOC entry size: 16 bytes (Media ID) + 4 bytes (Offset) + 2 bytes (Card Count) = 22 bytes
    toc_size = num_tracks * 22
    payload_start_offset = header_size + toc_size

    toc_bytes = bytearray()
    payload_bytes = bytearray()

    current_offset = payload_start_offset

    for media_id, subtitles in valid_tracks:
        # Encode Media ID as 16-byte fixed-length ASCII string, padded with null bytes
        media_id_bytes = media_id.encode("ascii")[:16]
        media_id_padded = media_id_bytes.ljust(16, b"\x00")

        card_count = len(subtitles)

        # Pack TOC entry: 16-byte Media ID + uint32 BE payload offset + uint16 BE card count
        toc_bytes += media_id_padded
        toc_bytes += struct.pack(">IH", current_offset, card_count)

        # Serialize track payload (cards)
        track_payload = bytearray()
        for card in subtitles:
            start_ms = card.get("start_ms", 0)
            end_ms = card.get("end_ms", 0)
            speaker = card.get("speaker", "").strip()
            text = card.get("text", "").strip()

            speaker_bytes = speaker.encode("utf-8")
            text_bytes = text.encode("utf-8")

            # Pack card header: uint32 BE start_ms + uint32 BE end_ms + uint16 BE speaker_len
            track_payload += struct.pack(">IIH", start_ms, end_ms, len(speaker_bytes))
            track_payload += speaker_bytes

            # Pack text: uint16 BE text_len + UTF-8 text string
            track_payload += struct.pack(">H", len(text_bytes))
            track_payload += text_bytes

        payload_bytes += track_payload
        current_offset += len(track_payload)

    # Write final binary .dat file
    os.makedirs(os.path.dirname(output_dat_path) or ".", exist_ok=True)
    with open(output_dat_path, "wb") as f:
        f.write(b"BURS")  # Magic 4-byte signature
        f.write(struct.pack(">H", SUBTITLE_DATA_VERSION))  # Format Version (uint16 BE)
        f.write(struct.pack(">H", num_tracks))             # Track Count (uint16 BE)
        f.write(toc_bytes)                                  # Table of Contents
        f.write(payload_bytes)                              # Track Payloads

    print(f"Successfully generated {output_dat_path} ({len(valid_tracks)} tracks, {current_offset} bytes).")


def main():
    parser = argparse.ArgumentParser(
        description="Build ScummVM Buried in Time subtitle data file (buried_subtitles.dat)"
    )
    parser.add_argument("input_json", help="Path to devtools/create_buried/subtitles.json")
    parser.add_argument("output_dat", help="Path to dists/engine-data/buried_subtitles.dat")

    args = parser.parse_args()

    if not os.path.exists(args.input_json):
        print(f"Error: Input JSON file '{args.input_json}' not found.", file=sys.stderr)
        sys.exit(1)

    pack_dat(args.input_json, args.output_dat)


if __name__ == "__main__":
    main()
