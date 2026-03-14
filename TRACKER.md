# TRACKER

## Last Confirmed Action

- Confirmed the `room_setup` palette handoff and runtime cursor animation path before extending the startup stub.
  - Renamed `FUN_00023a30` to `ramp_palette_brightness`; it fades a palette in `0.1` steps on an approximately 4 ms timer by repeatedly calling `upload_palette_to_vga`.
  - `upload_palette_to_vga` forces DAC color 0 to black and scales the remaining palette entries by a brightness factor before writing ports `0x3c8` / `0x3c9`.
  - `set_entity_animation_sequence` maps each cursor sequence to a 10-frame run, so sequence `7` is `POINTERS.ABM` frames `70..79`.
  - `sync_cursor_entity_position` updates `g_cursor_entity` to `g_cursor_screen_x` / `g_cursor_screen_y` and then advances the normal entity visual-state tick, which is why the runtime cursor animates inside the dirty-rect render path.
  - ScummVM startup flow now mirrors that evidence closely enough for the stub: it shows the wait transition first, fades room palettes in from black toward `1.0` or `0.6`, and animates the startup cursor with the recovered sequence-`7` timing instead of blitting frame `0`.
- Current live-state counts:
  - `HARVEST.LE` currently has `751` total functions
  - `377` have custom/documented names
  - `374` still remain unnamed / `FUN_*`

## Next Suggested Action

- Highest-value targets:
  - type the `ROOM` record more aggressively from `room_setup`, starting with the already supported fields around room name, palette path, music path, enter/exit tags, dimmable, and the perspective/scale values still showing as raw offsets
  - type the runtime cursor/entity record next around the confirmed animation fields at `+0x105c`, `+0x1060`, `+0x106c`, `+0x1078`, `+0x112c`, and `+0x1130`
  - once those structs are cleaner, revisit `room_setup`, `spawn_object_entity_from_record`, `spawn_anim_entity_from_record`, and `dispatch_room_event_actions` so the remaining entity-build side of the first room can move out of placeholder status without speculative names
