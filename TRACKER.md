# TRACKER

## Current Focus

- Verify the split between player-present carry pickup and no-player direct-to-inventory pickup directly from `run_harvester_main_loop`
- Keep the engine-side pickup path aligned with the native in-place scene mutation instead of forcing a room rebuild after plain pickup
- Re-check only the remaining in-game drag/drop edge cases that still need native capture confirmation

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited the native pickup branch in Ghidra and confirmed the deciding condition is player presence in the room, not the item name.
  - When the player is present, `run_harvester_main_loop` enters cursor-follow carry mode for pickup objects and keeps using the class `5` / `"your inventory"` overlap target for the explicit room-item-to-inventory handoff.
  - When the player is absent, the same loop moves pickup objects straight into `INVENTORY`, removes the live room entity in place, and skips the carry state entirely.
  - The pickup path itself does not rerun full room setup in the native binary; only later action-driven mutations or room transitions take the heavier refresh path.
- Adjusted the engine-side startup-room flow to match that split more closely.
  - Full rooms still use carried-item mode, closeup/no-player rooms now transfer pickup objects directly into inventory, and plain pickup no longer forces a full room scene rebuild.

## Next Suggested Action

1. Compare the native and engine behavior in-game for both pickup branches: `NEWSPAPER` in `PCLIVRM` should stay carried until it reaches `"your inventory"`, while `PC_PEN` / `QUARTER` in `PCDRWR` should jump straight into inventory with no interim carry mode.
2. If pickup-side visuals still diverge from DOSBox, inspect the native carried-entity anchor and the no-player closeup redraw order before changing any draw offsets.
