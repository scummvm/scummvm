# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/non-`FUN_*`: `868`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 17, 2026, verified native cursor-enabled region targeting and player-region collision in Ghidra, then fixed `RMNBATH` bath-exit handling by allowing fast exit clicks there and adjusting side-region X targets so the native-style horizontal slack cannot stop the player outside narrow side exits such as `RNMBTH_BATH`.

## Next Suggested Action

- Runtime-test `RMNBATH` after entering from `BATHROOM_2_RMNBATH`: double-click `RNMBTH_BATH` and confirm it immediately runs `RMNBATHM_BATHROOM`, then single-click the same region and confirm walking still reaches the trigger.
