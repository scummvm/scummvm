# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 11, 2026, verified the solved Chessmaster dialogue branch in Ghidra. Native `handle_talk_to_chessmaster` sets `CHESS_SOLVED`, plays `graphic\fst\c125.fst`, then calls `queue_named_npc_death_or_monsterfy_transition("CHESSMASTER", "SLASH")`; it does not immediately finalize or hide the NPC record. The decoded `HARVEST.SCR` confirms `CHESSMASTER` has `on_death='ADD_CHESS_CU'`, and that action reveals `CHESSM` before skipping king growth when `CHESS_SOLVED` is true. Updated the ScummVM solved-puzzle shortcut to queue the live NPC death transition instead of finalizing it immediately, so the room loop can play the death animation and dispatch `ADD_CHESS_CU` at the terminal frame.

## Next Suggested Action

- In-engine, replay the solved Chessmaster dialogue path from `CHESS_SOLVED` through `C125.FST` and the queued NPC death transition. Confirm that `combat npc death start`/`complete` logs appear for `CHESSMASTER`, `ADD_CHESS_CU` reveals the `CHESSM` body/hotspot, the live `CHESSMASTER` actor does not respawn, and examining the body reaches the `CHESSHEAD` closeup/key interaction.
