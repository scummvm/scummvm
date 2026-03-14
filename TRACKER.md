# TRACKER

## Last Confirmed Action

- Extended the startup-room lift from the cursor-only runtime entity into the room object / `ANIM` spawn path.
  - `room_setup` decompilation confirms the post-wait rebuild order: visible room objects whose `current_owner_or_room` matches the destination room are spawned before matching `ANIM` records whose `active` or `visible` state is set.
  - `parse_anim_record` is now bounded enough to model conservatively: one unresolved leading numeric field, then `x`, `y`, `frame_delay`, `room_name`, `anim_path`, `anim_name`, and the script booleans `active`, `visible`, `loop`, `backward`, `ping_pong`, `remove`.
  - ScummVM now parses `ANIM` records from `HARVEST.SCR`, lifts deduped visible room objects plus active/visible room anims into the runtime-entity subsystem, and ticks scene ABMs through the recovered frame-delay path instead of baking them into the background scene.
  - The current lift is still intentionally short of the original render-list ordering, overlap checks, and collision/runtime trigger behavior; this pass only covers the spawn and per-frame visual tick slice.
- Current live-state counts:
  - `HARVEST.LE` currently has `755` total functions
  - `387` have custom/documented names
  - `368` still remain unnamed / `FUN_*`

## Next Suggested Action

- Highest-value targets:
  - recover the original render-list ordering helpers next, especially the insertion/sort behavior around `add_entity_to_render_list`, before trying to match room foreground/background overlap exactly
  - then lift the low-risk subset of `spawn_object_entity_from_record` that chooses between sprite-backed objects and pure hotspot boxes, so active-but-invisible objects can participate in startup-room interactions without being rendered
  - keep collision / overlap behavior tied to recovered helpers such as `do_entity_screen_bounds_overlap` and the cursor-hit tests, rather than inventing a scene graph policy in ScummVM first
