# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/non-`FUN_*`: `868`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 17, 2026, fixed the room-exit global timer sync path so `DNALFT` exit commands that run `SET_TIMER EDNA_BURNING_TIMER OFF` / `KILL_TIMER EDNA_BURNING_TIMER` also reconfigure the live preserved global timer entity before the next room setup carries it forward.

## Next Suggested Action

- Runtime-test the DNA fire day transition: light Edna's diner, exit through `DNAEXT_X2` to Day 6, then remain in PC/Hank dialogue past the old countdown window and confirm `EDNA_BURNING_TIMER` no longer fires `GRAPHIC/FST/PCBURN.FST`.
