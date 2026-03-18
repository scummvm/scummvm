# TRACKER

## Current Focus

- Verify the corrected native startup-room pickup flow directly from `run_harvester_main_loop`, `run_inventory_screen`, and `handle_target_interaction`
- Keep the engine-side carry / prompt behavior grounded in confirmed `MEDFONT1` and `"your inventory"` usage before refining any remaining UI details
- Re-check only the remaining in-game drag/drop edge cases that still need native capture confirmation

## Progress

- Program: `HARVEST.LE`
- Total functions: `900`
- Named/documented: `587`
- Still `FUN_*` / undocumented: `313`

## Last Confirmed Action

- Revisited the native carry flow in Ghidra and corrected the pickup-side details.
  - `run_harvester_main_loop` starts carry mode directly from pickup objects, keeps the carried entity under the mouse, builds the bottom `USING_ON_ID` overlay with `g_medfont1_cft`, and recognizes the player entity's `"your inventory"` label as the explicit room-item-to-inventory handoff target.
  - `handle_target_interaction` special-cases class `5` / `"your inventory"` overlap to move non-inventory carried items into `INVENTORY`, while right-click, menu, and inventory reopen paths also stow the carried item.
- Adjusted the engine-side startup-room flow to match that corrected behavior more closely.
  - Room pickups now enter a carried-item state before inventory transfer, inventory-selected items render as dragged sprites both inside and outside the overlay, dragging an inventory item out still closes the panel while keeping it active, and the shared bottom prompt now uses the native `MEDFONT1` path instead of the GUI font.

## Next Suggested Action

1. Compare the native and engine startup-room flow in-game, focusing on room pickup to `"your inventory"` transfer, inventory drag-out prompt text, and cancel / reopen behavior while carrying.
2. If the carried-item sprite still looks offset versus DOSBox captures, inspect the selected-entity screen-position update in `run_harvester_main_loop` before changing the engine draw anchor.
