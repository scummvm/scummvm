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

- Traced the native startup-room audio paths in Ghidra and aligned the startup engine with the confirmed split between looping room music and one-shot samples.
  - `room_setup` starts `g_current_room_def->music_path` through `start_music_stream(&g_music_stream_state, ...)` and caches the active filename in `g_current_music_path`.
  - `dispatch_room_event_actions` opcode `0x19` is the scripted music-spool path; it starts a replacement stream immediately and updates `g_current_music_path` without requiring a room transition.
  - `dispatch_room_event_actions` opcodes `0x1b` and `0x2b` load one-shot samples through `load_sound_sample` / `start_sound_state_playback`, separate from the persistent music stream.
  - `SOUND/MUSIC/MORNING2.CMP` is an `FCMP` file with a 14-byte header (`magic`, compressed payload size, sample rate, decoded bits-per-sample) followed by raw DVI/IMA ADPCM payload.
- Patched the startup ScummVM path to mirror that split.
  - Added looping startup-room music playback on top of `FCMP`/`CMP` and `WAV` decoding.
  - Room-entry `musicPath` now starts persistent background music, while interaction-triggered `SPOOL_MUSIC` commands now swap that background stream without reusing the one-shot sample handle.
  - One-shot startup sounds continue to use their own handle so room music can continue underneath them.

## Next Suggested Action

1. Recover the native startup/event opcode split for `LOAD_WAV`, `START_WAV`, and `START_SINGLE_WAV` so the stub can mirror the rotating versus dedicated sound-sample slots instead of collapsing every effect to one handle.
2. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
3. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
