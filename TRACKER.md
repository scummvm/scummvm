# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 1, 2026, rechecked the native shotgun-shell progression in Ghidra after seeing zero shells on a legacy ScummVM save: decoded `HARVEST.SCR` confirms `CLOAK_ADD -> ADD2INV SHOTGUN -> ADD_SHOTGUN_SHELLS 2`, the `SHOTSHELL` pickup object uses `ADD12SHELLS -> ADD_SHOTGUN_SHELLS 14`, and `run_inventory_screen` does not provide a native shell-box use path beyond the six consumables (`SANDWICH`, `SANDWICH2`, `SYRINGE`, `ST_ASPRIN`, `ST_COUGHM`, `ST_VITAMN`). Updated the ScummVM Harvester pre-v18 save fallback to infer the confirmed shotgun shell baseline from legacy inventory objects, so old saves with `SHOTGUN` start at `2` and old saves carrying `SHOTSHELL` recover to the native `15`-shell cap.

## Next Suggested Action

- Load the same legacy save again and open inventory without repicking anything: confirm the shotgun now arms with `2` shells, then check the same save after the `SHOTSHELL` box is already in inventory and confirm the recovered count is `15` without needing a right-click use action. If either value still disagrees, capture the `Harvester: inferred legacy pre-v18 combat resources ...` log line from load plus the next `inventory combat toggle` log line.
