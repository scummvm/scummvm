# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, confirmed via Ghidra that native class-`4` room NPCs queue a live death state first, play a death bank (`0x3c..0x45` for gore/monsterfy or `0x46..0x4f` for bludge), and only then finalize suppression and run the NPC `onDeathActionTag`. Patched ScummVM so BIG_EYE and other class-`4` NPCs now use that queued death flow, and gated room-NPC talk hover/dialogue on both `deathDamageType == 0` and an installed dialogue handler. Verified with `make -j4 engines/harvester/flow.o engines/harvester/room.o engines/harvester/dialogue.o engines/harvester/script.o` and `git diff --check`.

## Next Suggested Action

- Re-run the in-engine `EYEHALL` repro against DOSBox and confirm the live BIG_EYE kill now shows the native death bank before `EYEDOOR`, that the hover falls back to the `BIGEYE` examine hotspot instead of `Talk to big eye`, and that no other class-`4` NPCs regress under the new handler/death-type talk gate.
