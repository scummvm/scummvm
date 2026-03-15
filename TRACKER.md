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

- Confirmed in Ghidra how the native player idle overlay is spawned and timed.
  - `run_harvester_main_loop()` waits until `max(activity_timestamp, reset_timestamp) + 3000`, turns the player to facing `0`, then spawns `IDLE_ANIM` from `1:\GRAPHIC\ROOMANIM\PCLOUN02.ABM` at the player render entity's current screen `x`, current screen `y + 4`, and current `z`.
  - The native idle overlay runs at animation rate `0x1e` (`30`), loops by jumping from frame `0xb2` back to `0x0f`, and exits by running from `0xb2` through `0xbf` before removing the overlay and showing the player entity again.
  - These timings match the current startup constants; the remaining placement-sensitive part was how the overlay anchor was derived from the rendered player position.
- Patched the startup stub to mirror that placement more closely.
  - The startup idle overlay now anchors to the player entity's current rendered screen rect (`left/top + 4`) instead of the entity's pre-offset internal coordinates, which avoids the visible screen drift caused by ABM frame offsets moving under a fixed internal origin.
  - The startup idle timing stays at the native `30` rate with the existing `0x0f`/`0xb2`/`0xbf` frame gates.

## Next Suggested Action

1. Recover the render/entity runtime structure around `spawn_scaled_abm_entity_from_resource()`, `hide_entity_visual()`, `update_render_entity_screen_position()`, `tick_entity_visual_state()`, and `update_actor_runtime_state()`.
  - That struct is now the highest-leverage missing piece: it governs actor screen anchoring, frame offsets, hitmasks, z overlap, blocker-history detours, and the remaining startup/combat avatar behavior that still reads as raw offsets in Ghidra.
2. Runtime-test `PCROOM`, `PCX1`, and `PCDRWR` against DOSBox with the new movement and idle-overlay path, focusing on spawn placement, walk speed, blocker stops, idle stability, and the restore point after returning from closeups.
3. Recover more of `update_actor_runtime_state()` if obstacle navigation still differs.
  - The startup stub now matches native target axes, step sizes, opaque blocking, and idle-overlay placement, but it still stops/slide-tests directly instead of reproducing the native blocker-history detour slots at `+0x108c`, `+0x1090`, `+0x1094`, `+0x1098`, and `+0x109c`.
4. Confirm whether startup room records ever override the default `zVelocityStep = 1.0f`; the current startup parser still relies on the struct default rather than a recovered script field.
5. Flesh out the room-menu stub now that `Esc` reaches the correct native entry point, especially the per-item behaviors inside `run_main_menu()` such as resume/save/load/options/quit gating.
