# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, verified native `PC_GOTO_XZ` handling in Ghidra. `dispatch_room_event_actions` opcode `0x1e` removes the player combat avatar, parses arg2 as Z and arg1 as X, calls `set_entity_screen_position`, and reinserts the avatar; this changes X/Z while preserving the current screen Y and sprite scale. Updated ScummVM `PC_GOTO_XZ` handling to match that behavior for the CHESSKING battle setup.

## Next Suggested Action

- In-engine, replay the CHESSMASTER refusal path into `ADD_CHESS_KING_MONSTER` and confirm `PC_GOTO_XZ 30,20` leaves Steve at the native screen Y while CHESSKING starts at logical Z 20 and can be targeted/hit with the shotgun.
