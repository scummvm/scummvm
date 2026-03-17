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

- Fixed the room-loop crash exposed by the recovered movie-surface switch.
  - The native `play_fst_sequence @ 0x72550` surface swap remains correct, but `StartupRoomSystem::runRoomLoop` had cached `_engine.getScreen()` at room entry and kept reusing that freed `Graphics::Screen` for room redraws, dialogue-backdrop capture, and ESC room-menu capture after `setDisplayMode()` recreated the screen.
  - Room rendering now resolves the active screen on demand in those post-FST paths, so dialogue movie playback can return to the `640`-wide gameplay UI without leaving the room loop holding a stale screen pointer.

## Next Suggested Action

1. Audit the remaining long-lived `_engine.getScreen()` caches that can survive an FST-triggered mode swap, especially startup/menu loops that redraw after returning from shared movie playback.
2. Exercise other in-game FST callers, including death-FST and menu-adjacent playback, to confirm whether any caller still needs an explicit active-screen refresh or redraw after the native `320x200` movie surface returns to the gameplay UI.
