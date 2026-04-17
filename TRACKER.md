# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/non-`FUN_*`: `868`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 17, 2026, fixed region-target movement completion so pending exits such as `BWLNT1_2_BWLNT2` keep walking until both target depth and target screen Y are reached, while ordinary floor clicks still use the native-style depth slack.

## Next Suggested Action

- Runtime-test `BOWLSNTRY1` after entering from `RECEPTION_2_BOWLSNTRY1`: click or double-click the bottom exit region and confirm the player reaches the trigger and runs `BWLNTR1_2_BWLNTR2`.
