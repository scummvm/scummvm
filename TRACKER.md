# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, corrected room-animation spawning so active Harvester room animations are displayed even when the authored script leaves their raw visibility flag cleared, which restores the missing `DRAWFIRE` fireplace animation on entry to `DRAWROOM` without rewriting script state. Verified with a successful `make -C build-vscode-harvester-debug -j4 engines/harvester/flow.o engines/harvester/room.o`.

## Next Suggested Action

- Do a runtime smoke test that enters `DRAWROOM` from `MAINHALL`, confirms `DRAWFIRE` is visible before interaction, and checks a few other authored active-only room animations to make sure the renderer now matches the original script semantics.
