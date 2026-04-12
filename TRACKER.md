# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `928`
- Named/documented: `824`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 12, 2026, added the in-engine `DEBUG_PATHFINDING` toggle. When enabled, it overlays the room movement band, outlines scene entities that can affect player pathfinding, and gates verbose click/target/blocker movement logging behind the `pathfinding` debug channel.

## Next Suggested Action

- Run Harvester with `--debugflags=pathfinding`, toggle `DEBUG_PATHFINDING` in the debugger console, then click through rooms with suspicious placement to compare the blue movement band, green blocker outlines, and emitted click/step/blocker logs.
