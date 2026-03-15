# TRACKER

## Current Focus

- Ghidra-backed startup engine alignment from `run_harvester_main_loop`
- Match the native startup room idle/movement behavior before expanding further subsystem coverage
- Keep symbol recovery and engine changes tied to direct script data, call sites, and visible side effects

## Progress

- Program: `HARVEST.LE`
- Total functions: `774`
- Named/documented: `468`
- Still `FUN_*` / undocumented: `306`

## Last Confirmed Action

- Traced the native startup/event one-shot audio opcode split in Ghidra and aligned the startup stub with the confirmed slot model.
  - `START_WAV` (`dispatch_room_event_actions` case `0x1b`) uses an 8-slot rotating pool keyed by sample path.
  - `START_SINGLE_WAV` (`0x2b`) uses a dedicated single sample slot.
  - `LOAD_WAV` / `PLAY_WAV` / `DELETE_WAV` (`0x30` / `0x31` / `0x32`) manage four indexed preloaded sample slots.
  - Decoded `HARVEST.SCR` confirms those preloaded slots are used by startup tags such as `CEM2_STARTUP`, `CEM10_STARTUP`, and `ARRESTED_STARTUP`, with later timers/actions driving `PLAY_WAV` and exit tags issuing `DELETE_WAV`.
- Patched the startup ScummVM path to preserve that distinction instead of collapsing every effect to one immediate handle.
  - Startup room setup state and object interactions now carry explicit audio commands rather than a single `soundPath`.
  - The engine now executes rotating, single, and indexed preloaded startup sample commands separately while keeping looping room music independent.

## Next Suggested Action

1. Recover the startup-room exit-command path so `onExitCommand`/`EXITCOM` actions can drive native-style cleanup such as `DELETE_WAV` and any other room-teardown effects.
2. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
3. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
