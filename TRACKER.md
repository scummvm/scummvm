# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, rechecked the native room-entry monster wake-up path in Ghidra before tightening CAINS pursuit timing: `spawn_monster_entity_from_record` at `0x53a10` seeds the live monster desired state byte to `0x07` or `0x0e` immediately from the player's relative X position, so fresh class-6 spawns are primed for pursuit as soon as the room loop starts. Updated the ScummVM Harvester room-combat shim to allow one immediate pursue evaluation for newly entered active monsters instead of waiting for the first animation-driven movement tick, which tightens room-entry aggro without inventing extra AI semantics. Verified with a successful `make -j4 engines/harvester/room.o`.

## Next Suggested Action

- Runtime-smoke the CAINS room entry again without touching movement keys: enter `CAINS`, wait one second, and confirm the log now emits `Harvester: combat monster chase move ...` before the first player input. If CAIN still stays planted until input, capture the exact first second of post-entry combat logs so we can compare the room-loop ordering against native `run_harvester_main_loop -> tick_monster_entity_runtime`.
