# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `923`
- Named/non-`FUN_*`: `864`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 16, 2026, fixed inventory use/carry prompt labels so `ObjectRecord.interactionLabel` is preferred over the inventory `*_STEXT` tooltip text, matching the Ghidra `run_inventory_screen` and `run_harvester_main_loop` label-field evidence and preventing `right_click_to_view` suffixes from leaking into `Use ... on ...` prompts. Verified `engines/harvester/script.o` builds.

## Next Suggested Action

- Runtime-test selecting `COMPLETED_LODGE_APPLICATION` in inventory, then hovering inventory targets and room/NPC targets in `SERGEANTRM`; confirm the use prompt shows the clean interaction labels such as `completed Lodge application` and does not include the right-click tooltip suffix.
