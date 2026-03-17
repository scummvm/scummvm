# TRACKER

## Current Focus

- Rebuild talk-handler ports from corrected native control flow
- Keep each NPC port scoped to confirmed native behavior only
- Commit each completed NPC update as an isolated change

## Progress

- Program: `HARVEST.LE`
- Total functions: `899`
- Named/documented: `586`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited dialogue-triggered FST playback from the native movie path and confirmed the display-surface mismatch.
  - `play_fst_sequence @ 0x72550` does not leave gameplay/dialogue playback on the active `640`-wide UI surface. It pauses music, switches back to the `320x200x8` movie surface with `configure_video_surface(&g_video_surface_context, 0x140, 8, 1)`, runs the decoded FST player, then restores the `640`-wide gameplay surface with `configure_video_surface(&g_video_surface_context, 0x280)` before reapplying the palette and mouse bounds.
  - The engine-side `FstPlayer` now mirrors that wrapper behavior, so dialogue FST files and other in-game movie playback no longer stay quarter-sized on the live `640x480` gameplay display.

## Next Suggested Action

1. Exercise the remaining in-game FST callers against the shared movie-surface switch and confirm whether any of them rely on an additional redraw step after playback returns to the `640`-wide gameplay UI.
   - The native wrapper now bounds the surface swap itself, but the surrounding caller-side redraw expectations still need a quick pass across dialogue-triggered clips and death-FST playback.
2. Apply any additional high-confidence Ghidra renames or comments that fall directly out of that next confirmed thread before the next commit.
