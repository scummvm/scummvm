# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, confirmed in Ghidra `dispatch_room_event_actions` that native `KILL_PC` maps arg1 damage strings such as `SLASH` onto the player actor death damage type, then sets player HP to zero. Updated script interaction results and room interaction handling so script-driven `KILL_PC` starts the player defeat sequence after timer/object runtime mutations are applied.

## Next Suggested Action

- Re-test the PIT/JAWS roof timer chain through `JAWROOF10T` and verify `BODYCRSH.WAV` plays, `JAWROOF10` is materialized, the player uses the `SLASH` death animation bank, and the game-over return happens after the death animation completes.
