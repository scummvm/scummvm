# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `929`
- Named/documented: `825`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, checked the Inner Sanctum post-Muckey path against decoded `HARVEST.SCR` and Ghidra. Confirmed the `{COMMAND "KILL_HERR2" ...}` script line is commented out and skipped by the native parser, while `update_actor_runtime_state` has a hard-coded `HERRILL_LOG` removal path during Muckey combat state. Updated ScummVM room combat handling to queue Herrill's live NPC death when `MUCKEY` completes death and triggers `ACTV_INNERSAN_DOOR`.

## Next Suggested Action

- Re-test `INNERSANCTUM` from entrance `L2_2_M1`. Kill `GRAND_MUCK`/`MUCKEY`, verify `HERRILL_LOG` plays his death before `KILL_HERMUCK_T` starts the `C002A`/`C002B` flic chain, and verify re-entry no longer respawns Herrill.
