# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 13, 2026, removed the temporary focused `BIGEYE` / `BIG_EYE` debug flag, lifecycle logging, and post-detach overlap audit while keeping the `BIG_EYE` / `EYEDOOR` terminal death-frame preservation fix.

## Next Suggested Action

- Re-run the EYEHALL combat case with `--debugflags=room,scene,combat`, kill `BIG_EYE`, and confirm the death-complete line reports `preserve_runtime_actor=1` and that the final frame remains over the baked background while `BIGEYE` hotspot deletion still enables `EYE_MAIN`.
