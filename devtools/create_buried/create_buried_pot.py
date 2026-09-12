#!/usr/bin/env python3
"""
ScummVM - Graphic Adventure Engine Emulator
devtools/create_buried/create_buried_pot.py

GNU Gettext .POT translation template exporter for Buried in Time subtitles.

Reads consolidated subtitles from devtools/create_buried/subtitles.json,
merges sequential cards for the same speaker within each track to allow natural
translation across sentence boundaries, and outputs a fresh .pot file.
"""

import argparse
import datetime
import json
import os
import pathlib
import sys

# All of the game's media files are prefixed with some context about when or how they appear in
# the game. This can be used to provide translators some additional context.
PREFIX_DESCRIPTIONS = {
    'AI': "AI voiceover comments (Arthur or station computers)",
    'CG': "Château Gaillard, France (1204 AD)",
    'DS': "Leonardo da Vinci's Studio, Italy (1488 AD)",
    'FA': "Farnstein's Lab (2247 AD) or Gage's Apartment (2318 AD)",
    'INN': "Interactive News Network Broadcasts",
    'INTRO': "Gage's Apartment (2318 AD)",
    'IO': "Tutorial (No location)",
    'MY': "Chichen Itza, Mexico (1050 AD)"
}


def get_location_desc(media_id: str) -> str:
    for prefix in sorted(PREFIX_DESCRIPTIONS.keys(), key=len, reverse=True):
        if media_id.startswith(prefix):
            return PREFIX_DESCRIPTIONS[prefix]
    return "Unknown Environment"


def merge_card_texts(cards: list) -> str:
    texts = [c.get('text', '').strip() for c in cards if c.get('text', '').strip()]
    return " ".join(texts)


def escape_po_string(s: str) -> str:
    s = s.replace("\\", "\\\\")
    s = s.replace('"', '\\"')
    s = s.replace("\n", "\\n")
    return s


def generate_pot(input_json_path: pathlib.Path, output_pot_path: pathlib.Path):
    with open(input_json_path, "r", encoding="utf-8") as f:
        tracks = json.load(f)

    unique_speakers = set()
    track_entries = []  # (media_id, speaker, combined_text)

    # Since the JSON already splits up dialog for best playback in English, lines need to be stitched
    # back together for the .pot file so that translators can work with full sentences or paragraphs
    # and not sentence fragments.
    for track in tracks:
        media_id = track.get("media_id", "").strip().upper()
        cards = track.get("subtitles", [])
        if not media_id or not cards:
            continue

        current_speaker = None
        current_cards = []

        for card in cards:
            speaker = card.get("speaker", "").strip()
            if speaker:
                unique_speakers.add(speaker)

            if current_speaker is None:
                current_speaker = speaker
                current_cards = [card]
            elif speaker == current_speaker:
                current_cards.append(card)
            else:
                text = merge_card_texts(current_cards)
                if text:
                    track_entries.append((media_id, current_speaker, text))
                current_speaker = speaker
                current_cards = [card]

        if current_cards:
            text = merge_card_texts(current_cards)
            if text:
                track_entries.append((media_id, current_speaker, text))

    track_entries.sort(key=lambda x: x[0])

    now_str = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")

    header = f"""# English translation transcriptions for Buried in Time in ScummVM.
# Copyright (C) {datetime.datetime.now().year} ScummVM Team
# This file is distributed under the same license as the ScummVM package.
#
# TRANSLATOR CONTEXT & GAME OVERVIEW:
# -----------------------------------------------------------------------------
# Game: The Journeyman Project 2: Buried in Time
# Protagonist: Agent 5 (Gage Blackwood), Temporal Security Agency (TSA)
# Plot: Frame-up mystery. Gage travels across time to gather evidence clearing
#       himself of altering history for personal gain.
# Key Characters:
#   - Gage Blackwood (Agent 5): Player character, TSA temporal agent.
#   - Arthur: Gage's AI companion (witty, sarcastic, highly informative).
#   - Michelle Visard (Agent 3): Fellow agent.
# Locations:
#   - Château Gaillard, France (1204 AD)
#   - Leonardo da Vinci's Studio, Italy (1488 AD)
#   - Chichen Itza, Mexico (1050 AD)
#   - Farnstein's Laboratory (2247 AD) or Gage's Apartment (2318 AD)
# -----------------------------------------------------------------------------
msgid ""
msgstr ""
"Project-Id-Version: Buried in Time\\n"
"Report-Msgid-Bugs-To: https://bugs.scummvm.org/\\n"
"POT-Creation-Date: {now_str}\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"

"""

    lines = [header]

    # Section 1: Character & Speaker Names
    lines.append("# =============================================================================\n")
    lines.append("# SECTION 1: Character & Speaker Names\n")
    lines.append("# =============================================================================\n\n")

    for speaker in sorted(list(unique_speakers)):
        if not speaker:
            continue
        lines.append(f"#. Character / Speaker Name\n")
        lines.append(f'msgid "{escape_po_string(speaker)}"\n')
        lines.append('msgstr ""\n\n')

    # Section 2: Dialogue Tracks
    lines.append("# =============================================================================\n")
    lines.append("# SECTION 2: Subtitle Dialogue Lines (Sorted Alphabetically by Track ID)\n")
    lines.append("# =============================================================================\n\n")

    for media_id, speaker, text in track_entries:
        loc_desc = get_location_desc(media_id)
        lines.append(f"#. Location: {loc_desc}\n")
        lines.append(f"#. Track ID: {media_id}\n")
        if speaker:
            lines.append(f"#. Speaker: {speaker}\n")

        escaped_text = escape_po_string(text)
        lines.append(f'msgid "{escaped_text}"\n')
        lines.append('msgstr ""\n\n')

    output_pot_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_pot_path, "w", encoding="utf-8") as f:
        f.writelines(lines)

    print(f"Successfully generated {output_pot_path} ({len(unique_speakers)} speakers, {len(track_entries)} dialogue entries).")


def main():
    parser = argparse.ArgumentParser(description="Generate GNU Gettext .POT translation template for Buried in Time subtitles")
    parser.add_argument("input_json", help="Path to devtools/create_buried/subtitles.json")
    parser.add_argument("output_pot", help="Path to output .pot file")

    args = parser.parse_args()

    input_path = pathlib.Path(args.input_json)
    output_path = pathlib.Path(args.output_pot)

    if not input_path.exists():
        print(f"Error: Input JSON file '{input_path}' not found.", file=sys.stderr)
        sys.exit(1)

    generate_pot(input_path, output_path)


if __name__ == "__main__":
    main()
