# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, verified native CHESSMASTER refusal sequencing in Ghidra: `handle_talk_to_chessmaster` dispatches `ADD_CHESS_KING_MONSTER` before playing `graphic\\fst\\kinggrow.fst`, and native `PC_GOTO_XZ` continues to arg4 instead of ending the command chain. Updated ScummVM script/dialogue handling so `SET_MONSTER`, `PC_GOTO_XZ 30,20`, and `CHESS_MOVE_PC2` resolve before `KINGGROW.FST`.

## Next Suggested Action

- In-engine, replay the CHESSMASTER refusal path and confirm the trace now applies `PC_GOTO_XZ 30,20` and `CHESS_MOVE_PC2` before `KINGGROW.FST`, then verify CHESSKING can be targeted/hit with the shotgun at logical Z 20.
