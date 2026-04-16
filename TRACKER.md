# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `923`
- Named/non-`FUN_*`: `864`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 16, 2026, fixed Wasp Woman keyword `BYE` handling so the generic/default `BYE` exits even after later topic buffers such as zero-based `dialog.rsp[0x30b]`, matching the native `handle_talk_to_wasp_woman @ 0x2fde0` top-of-loop `dialog.rsp[0x301]` compare. Corrected the Ghidra plate comment and `ARCHITECTURE.md` note. Verified `engines/harvester/npc/wasp_woman_dialogue.o` builds.

## Next Suggested Action

- Runtime-test Wasp Woman after selecting `Pleasure`: when the keyword buffer is zero-based `dialog.rsp[0x30b]` (`Wasps/Sacrifice/Pleasure`), select the default `BYE` entry and confirm the dialogue closes without re-opening the keyword menu.
