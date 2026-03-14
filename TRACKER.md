# TRACKER

## Last Confirmed Action

- Continued the `run_fst_sequence_player` pass far enough to formalize the on-disk movie layout and the special-case censorship side table.
  - Created the exact FST types in Ghidra:
    - `FstFileHeader` size `0x20`
    - `FstFrameIndexEntry` size `0x6`
    - `FstCensorshipToggleEntry` size `0x118`
  - Applied `FstCensorshipToggleEntry[25]` at `0xc1014` and labeled it `g_fst_censorship_toggle_entries`.
    - The record layout is now explicit: `sequence_name[0x100]` plus six 32-bit `toggle_frame_indices`.
    - `run_fst_sequence_player` scans those 25 entries by FST basename and, on matching frame indices, toggles between restoring the saved palette and blitting `GRAPHIC\\OTHER\\CENSORED.PCX`.
  - Confirmed the top-level FST playback layout from the binary call setup:
    - a `0x20`-byte header
    - `frame_count` frame-index records of `6` bytes each
    - per-frame reads of `video_size + audio_size`
    - audio pacing based on `get_pcm_byte_rate(...) / frame_rate`
- Extended the runtime-entity pass into the shared movement/update helpers.
  - Renamed `update_visible_entity_screen_position` at `0x4c860`, `update_render_entity_screen_position` at `0x5cc10`, `do_rects_overlap` at `0x5e0d0`, and `compute_rect_area` at `0x5e0c0`.
  - `update_render_entity_screen_position` now has a bounded role: when depth is unchanged it only updates dirty rectangles for the old/new screen bounds, and when depth changes it removes the entity from the render list, rescales it, reinserts it by depth, and merges the resulting dirty region.
  - A scripted scan of offset `+0x109c` confirmed that the collision/blocking-entity pointer written by `tick_entity_visual_state` is consumed by `update_actor_runtime_state`; no separate movement subsystem has shown up yet.
- Closed the shared packed-callback adapter pass and reclassified the remaining `0xc0c70` records as mixed Borland object-lifecycle descriptors rather than as one more PCM-only callback table.
  - Created and renamed `report_undefined_constructor_or_destructor_called` at `0x81448`.
    - It emits the exact string `undefined constructor or destructor called!` through the Borland runtime fatal path, which is what identifies the shared packed adapter field as an undefined ctor/dtor fallback.
  - Renamed the recovered lifecycle helpers bound by those mixed records:
    - `set_vesa_video_mode_3` at `0x3c8a0`
    - `reset_video_surface_context` at `0x3c8c0`
    - `reset_town_script_runtime_state` at `0x48000`
    - `install_keyboard_irq1_handler_state` at `0x48700`
    - `remove_keyboard_irq1_handler_state` at `0x48660`
  - Labeled the newly explicit state objects:
    - `g_town_script_runtime_state` at `0xd3050`
    - `g_keyboard_irq1_handler_state` at `0xd5900`
  - The remaining unresolved first record at `0xc0c90` is still an extended music/callback-context variant, but the adapter pass itself is now exhausted cleanly.
- Extended the PCM descriptor-blob pass with one new high-confidence stream helper and a bounded record shape.
  - Created and renamed `release_music_stream_buffers` at `0x1a440`.
    - The code frees the music stream's decode/ring buffers, unlocks their linear regions, and stops the active voice slot only when the slot/token still matches that stream.
    - The current table xref at `0xc0c9c` is what ties that callback back to `g_music_stream_state` at `0xca1d4`.
  - The smaller packed records at `0xc0ccc`, `0xc0cec`, and `0xc0d0c` now have a confirmed repeated shape:
    - direct callback pointer
    - state-object pointer
    - secondary callback, shared adapter `0x81448`, repeated direct callback, and state size, all packed as 24-bit values shifted left by 8 inside 32-bit slots
  - The first entry at `0xc0c90` is an extended music/callback-context variant of that layout, but its remaining packed size/order fields still do not converge cleanly enough for a formal struct.
- Confirmed and implemented the next room-setup runtime-entity slice.
  - `tick_entity_visual_state` now has a bounded collision path: Z-interval overlap first, then screen-bounds overlap, then opaque-pixel overlap.
  - `spawn_object_entity_from_record` classifies `640x480` bitmap objects rooted at initial `(0, 0)` as class `3` room backgrounds, and classifies hotspot-only objects as `0x15` / `0x16`.
  - `show_entity_visual` only applies the centered-anchor path when the internal `+0x4f` flag is clear, which separates the centered `ANIM` path from the top-left room-object spawn path.
- Typed the callback/driver object rooted at `0xca194` conservatively enough to stop conflating it with `PcmSoundState`.
  - `g_sound_driver_callback_context` at `0xca194` is now typed as a minimal `SoundDriverCallbackContext`.
  - The proven fields are now explicit in the type and labels:
    - `+0x14` is `selected_voice_slot_index`, also labeled as `g_selected_sound_voice_slot_index` at `0xca1a8`
    - `+0x18` overlaps the existing `g_sound_voice_bank_index`
    - `+0x20` overlaps the existing `g_sound_driver_initialized`
  - The descriptor blob entry at `0xc0c98` points directly at this object, which confirms that the table-driven wrappers around `0x18380` / `0x183c0` / `0x183e0` / `0x18400` are operating on the current-bank driver context rather than on a `PcmSoundState`.
- Current live-state counts:
  - `HARVEST.LE` currently has `764` total functions
  - `404` have custom/documented names
  - `360` still remain unnamed / `FUN_*`

## Next Suggested Action

- Highest-value targets:
  - continue `run_fst_sequence_player` into the inner frame decoder, especially the 4x4 block expansion path and the banked-blit branch around the scanline-bank callback
  - if the inner decode loop still resists clean factoring, shift back outward and recover the remaining `update_actor_runtime_state` consumers of the confirmed blocking-entity pointer at `+0x109c`
