#!/usr/bin/env python3
"""
ScummVM - Graphic Adventure Engine Emulator
devtools/create_buried/create_buried_subtitles.py

Build tool for generating dists/engine-data/buried_subtitles.dat from
devtools/create_buried/subtitles.json.
"""

import argparse
import json
import os
import struct
import sys

def pack_dat(input_json_path, output_dat_path):
    with open(input_json_path, "r", encoding="utf-8") as f:
        tracks = json.load(f)

    # Filter and sort tracks
    valid_tracks = []
    for track in tracks:
        media_id = track.get("media_id", "").strip()
        subtitles = track.get("subtitles", [])
        if media_id and subtitles:
            valid_tracks.append((media_id, subtitles))

    valid_tracks.sort(key=lambda x: x[0])

    SUBTITLE_DATA_VERSION = 1
    num_tracks = len(valid_tracks)
    header_size = 4 + 2 + 2  # Magic (4) + Version (2) + NumTracks (2)
    toc_size = num_tracks * (16 + 4 + 2)  # 22 bytes per entry
    payload_start_offset = header_size + toc_size

    toc_bytes = bytearray()
    payload_bytes = bytearray()

    current_offset = payload_start_offset

    for media_id, subtitles in valid_tracks:
        # Encode Media ID (16 bytes, null padded)
        media_id_bytes = media_id.encode("ascii")[:16]
        media_id_padded = media_id_bytes.ljust(16, b"\x00")

        card_count = len(subtitles)
        toc_bytes += media_id_padded
        toc_bytes += struct.pack(">IH", current_offset, card_count)

        # Build payload for this track
        track_payload = bytearray()
        for card in subtitles:
            start_ms = card.get("start_ms", 0)
            end_ms = card.get("end_ms", 0)
            speaker = card.get("speaker", "").strip()
            text = card.get("text", "").strip()

            speaker_bytes = speaker.encode("utf-8")
            text_bytes = text.encode("utf-8")

            track_payload += struct.pack(">IIH", start_ms, end_ms, len(speaker_bytes))
            track_payload += speaker_bytes
            track_payload += struct.pack(">H", len(text_bytes))
            track_payload += text_bytes

        payload_bytes += track_payload
        current_offset += len(track_payload)

    # Write binary .dat file
    os.makedirs(os.path.dirname(output_dat_path) or ".", exist_ok=True)
    with open(output_dat_path, "wb") as f:
        f.write(b"BURS")  # Magic signature
        f.write(struct.pack(">H", SUBTITLE_DATA_VERSION))  # Data format version
        f.write(struct.pack(">H", num_tracks))
        f.write(toc_bytes)
        f.write(payload_bytes)

    print(f"Successfully generated {output_dat_path} ({len(valid_tracks)} tracks, {current_offset} bytes).")


def main():
    parser = argparse.ArgumentParser(description="Build ScummVM Buried in Time subtitle data file (.dat)")
    parser.add_argument("input_json", help="Path to devtools/create_buried/subtitles.json")
    parser.add_argument("output_dat", help="Path to dists/engine-data/buried_subtitles.dat")

    args = parser.parse_args()

    if not os.path.exists(args.input_json):
        print(f"Error: Input JSON file '{args.input_json}' not found.", file=sys.stderr)
        sys.exit(1)

    pack_dat(args.input_json, args.output_dat)


if __name__ == "__main__":
    main()
