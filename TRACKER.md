# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 11, 2026, verified native ESC menu filtering in Ghidra. `run_main_menu` loads `main_menu_2` into the Save slot, then replaces that slot with a single blank string when `g_player_present_in_room == 0`; `room_setup` uses that no-player path for closeup rooms that spawn `EXIT_BM` and `EXIT_HS` instead of the player. Updated the ScummVM in-room ESC menu to build a displayed item list and blank Save when the current `RoomSetupState` has no entrance/player, matching the closeup behavior without changing the raw `MENU.INI` item list.

## Next Suggested Action

- In-engine, enter the `CHESSHEAD` closeup from `CHESSM`, press Escape, and confirm the Save row is blank/unselectable while Load, Options, Help, and Quit still behave normally. Then exit the closeup and confirm Save reappears in the ordinary `CHESSROOM` ESC menu.
