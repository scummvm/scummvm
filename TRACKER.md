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

- Confirmed in Ghidra how native startup pickup objects are gated behind a first-pass examine click.
  - `spawn_object_entity_from_record()` copies the object record's alternate inventory-sprite presence into entity byte `+9`, the operatable flag into entity byte `+0xb`, and the persistent identified-state byte at object offset `+0x4f` into entity byte `+10`.
  - In `run_harvester_main_loop()`, room objects with pickup or operate capability stay on cursor sequence `1` while entity byte `+10` is clear; once it is set, the same hover path switches to cursor sequence `5` for pickup-capable objects and cursor sequence `4` for non-pickup operatable objects.
  - Clicking that initial examine cursor calls `show_target_ident_text()`, then writes entity byte `+10 = 1` and persists object byte `+0x4f = 1`, so the next hover/click uses the unlocked pickup or operate path even if no IDENT panel was shown.
- Patched the startup stub to mirror that native behavior.
  - Startup pickup-capable and operatable room objects now resolve to the examine cursor until their first examine click marks them identified.
  - That first click now attempts to show the IDENT textbox when supported, but it still unlocks the object's later pickup or operate cursor even if the IDENT lookup has no usable panel, matching the native writeback behavior.

## Next Suggested Action

1. Runtime-test first-click IDENT gating on startup pickup objects such as `PC_PEN` against DOSBox, especially the exact cursor/prompt transition after dismissing the textbox.
2. Confirm whether any startup objects use the native "unlock on first examine" path without a valid IDENT panel, then mirror any edge-case prompt behavior if the current stub differs.
3. Flesh out the room-menu stub now that `Esc` reaches the correct native entry point, especially the per-item behaviors inside `run_main_menu()` such as resume/save/load/options/quit gating.
4. Parse startup NPC and timer records from `HARVEST.SCR`, then add persistent runtime state for the remaining confirmed exit and interaction opcodes: `SET_NPC`, `SET_TIMER`, and `KILL_TIMER`.
5. Confirm the native side effects of those opcodes in Ghidra before naming any new runtime structures or script helpers.
6. Parse `REGION` records from `HARVEST.SCR` into the startup script layer.
7. Mirror native startup transition handling for class `0x19` region entities.
  - Spawn enabled region hotspots from `room_setup` state.
  - Drive cursor sequence `6` over active regions.
  - Gate region activation on player overlap plus facing, following `check_player_region_interaction`.
