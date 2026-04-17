# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/non-`FUN_*`: `868`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 17, 2026, checked Moynahan's accepted casket-photo trade against native `handle_talk_to_moynahan @ 0x24a20`: after `dialog.rsp[0x18d]` response 1, native hides the presented photo and `GLUE`, adds `GLUE` to inventory, and sets `JUST_GOT_GLUE`. Updated the port so the direct dialogue mutation queues a live room visual refresh, removing the stale embalming-room glue entity after the trade.

## Next Suggested Action

- Runtime-test the MBLM glue trade: give Moynahan `CASKET_PHOTO`, accept the glue response, confirm `GLUE` appears in inventory, `CASKET_PHOTO` is no longer visible in inventory, and clicking the former room glue spot no longer starts the "can't have that" interruption dialogue.
