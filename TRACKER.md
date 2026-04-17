# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/non-`FUN_*`: `868`
- Remaining `FUN_*`: `59`
- Remaining real-space `FUN_*`: `16`
- Remaining zero-xref `.image` `FUN_*` artifacts: `43`

## Last Confirmed Action

- On April 17, 2026, verified the Dad meat-permission handoff against `HARVEST.SCR` and native `handle_talk_to_dad @ 0x2b020`: the script defines separate `MEAT_PERMISSION0` unsigned and `MEAT_PERMISSION` signed inventory objects, and the native Dad branch adds `MEAT_PERMISSION` then hides `MEAT_PERMISSION0` for owner `INVENTORY`. Updated the port to hide the unsigned slip after awarding the signed slip, and renamed the confirmed native helpers `get_set_shared_dialogue_state_d2eb4` and `play_c112s4_fst_sequence`.

## Next Suggested Action

- Runtime-test the Dad permission-slip handoff: obtain `MEAT_PERMISSION0` from the Butcher, show it to Dad after the permission state is armed, then confirm the inventory contains `MEAT_PERMISSION` and no longer shows `MEAT_PERMISSION0`.
