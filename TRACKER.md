# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/non-`FUN_*`: `868`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 17, 2026, checked the day/night palette handoff against native `dispatch_room_event_actions @ 0x60ee0`, `room_setup @ 0x73540`, and decoded CD1/CD2 `HARVEST.SCR`: `SET_FLAG DAY_FLAG` does not alter the active palette, live `SET_REGION` mutates region entities in place, and only `CHANGE_LIGHTING` changes the live palette (`DIM` to `0.6`, `NORMAL` to `1.0`, `NONE` to black). Updated the port so live same-room mutations preserve the current palette state while room-entry mutations can still set the initial room fade target.

## Next Suggested Action

- Runtime-test the fire-station cloth day-4 transition and a normal nightfall transition: the text box should remain on the preexisting dim/night palette until the following `CHANGE_LIGHTING NONE` clears to black, with the next loaded room fading to the brightness implied by its room setup and `DAY_FLAG`.
