# TRACKER

## Last Confirmed Action

- Implemented the first Harvester runtime-entity slice directly from the recovered cursor path.
  - Added a minimal `runtime_entity` module that loads ABM resources, tracks the recovered frame/timer/sequence state, applies sequence changes in 10-frame blocks, and advances visual state from the same timing model used by `tick_entity_visual_state`.
  - `HarvesterEngine` now owns the runtime-entity manager instead of leaving cursor animation logic inside `StartupFlow`.
  - The startup cursor is now spawned from `1:/GRAPHIC/POINTERS/POINTERS.ABM` through the runtime-entity subsystem, switched to sequence `7`, hidden during the wait transition, and advanced through the recovered `sync cursor position + tick` path each frame.
  - `StartupArt` no longer treats `POINTERS.ABM` as static startup UI art; it only keeps the wait/menu resources while the runtime subsystem owns cursor visuals.
- Current live-state counts:
  - `HARVEST.LE` currently has `751` total functions
  - `377` have custom/documented names
  - `374` still remain unnamed / `FUN_*`

## Next Suggested Action

- Highest-value targets:
  - type the runtime entity record more aggressively around the still-unmodeled header/state fields (`+0x4c` .. `+0x54`, `+0x1084`, `+0x109c`, `+0x1124`, `+0x1130`) so the ScummVM runtime-entity layer can expand beyond cursor animation without speculative flags
  - use that typed record to lift `spawn_object_entity_from_record` and `spawn_anim_entity_from_record` next; those are the cleanest path from the current startup-room placeholder scene into real runtime entities
  - after the object/anim spawn path is cleaner, revisit `room_setup` and the render-list tick helpers so room overlays and scripted anims move through the same subsystem as the cursor
