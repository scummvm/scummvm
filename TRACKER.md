# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `923`
- Named/non-`FUN_*`: `864`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 16, 2026, updated ScummVM's Chessmaster three-move puzzle playback to match the newly confirmed native FLC sequencing. The engine now presents the decoded first `CHESMOV*.FLC` frame before advancing and limits the three move FLC runs to the native counts `0x1d`, `0x1d`, and `0xe` instead of always playing each file to EOF. Verified `engines/harvester/npc/chessmaster_dialogue.o` builds.

## Next Suggested Action

- Runtime-test the Chessmaster dialogue path: choose the chess puzzle response, click the three correct hotspots, and verify `CHESMOV1.FLC`, `CHESMOV2.FLC`, and `CHESMOV3.FLC` transition at the same points as native before `C125.FST` and the Chessmaster death/monsterfy state. Also verify a wrong hotspot still branches to the `KINGGROW.FST` failure path.
