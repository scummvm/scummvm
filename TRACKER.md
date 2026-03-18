# TRACKER

## Current Focus

- Verify the startup-room inventory item carry/use handoff directly from `run_inventory_screen` and `run_harvester_main_loop`
- Keep the engine-side inventory interaction flow grounded in confirmed Ghidra behavior before adding any extra room UI polish
- Re-check only the remaining inventory drag/use details that still need native capture confirmation

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Recovered the native inventory-item carry handoff from Ghidra.
  - `run_inventory_screen` sets the active-item latch, rebuilds the `Use %s on ...` prompt, and returns the selected item to `run_harvester_main_loop` once it leaves the inventory panel bounds.
  - `run_harvester_main_loop` then keeps that item active under the mouse until use or cancel, updates the `USING_ON_ID` prompt from overlapping room targets, and clears the carry state on right-click / menu / inventory reopen paths.
- Adjusted the engine-side startup-room inventory flow to match the verified behavior more closely.
  - Selected inventory items can now leave the overlay and remain active in room mode, room clicks route through the active-item state instead of requiring the overlay to stay open, and cancel paths clear the carry state without inventing new cursor semantics.

## Next Suggested Action

1. Compare the native and engine startup-room inventory flow in-game, focusing on the item drag-out transition, prompt text, and cancel behavior after an item is brought into room mode.
2. If the carried-item sprite anchor still looks off versus native captures, inspect the selected-entity positioning path in `run_harvester_main_loop` further before changing the draw offset.
