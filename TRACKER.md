# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `886`
- Named/documented: `569`
- Still `FUN_*` / undocumented: `317`

## Last Confirmed Action

- Bounded the hidden Boyle keyword block around `0x2d620` and renamed its fixed FST helper as `play_c019b_fst_sequence @ 0x38620`.
  - The recovered branch map is now stable: `Amnesia` (`dialog.rsp 0x17`) plays `0xee` / `0xf2` / `0xf6` / `0xfa`; `Button`, `Epaulet`, `Shirt`, and `Postal Button` (`0x18` through `0x1b`) all funnel through the read-only `get_set_shared_dialogue_state_d2eb0` gate; `Arson` (`0x1d`) and the literal keyword `Arsonist` share the `0x11c` prompt plus the response menu at `dialog.rsp 0x1e`; `Blackmail` (`0x1f`) plays the `0x13d` through `0x14e` sequence; `BYE` (`0x20`) falls through the common exit path; and unknown keywords drop to `0x243`.
  - `play_c019b_fst_sequence` is only reached from the `d2eb0 == 0` evidence-topic branch, where Boyle plays `GRAPHIC/FST/C019B.FST` immediately before dialogue `0x10e`; the `d2eb0 != 0` side skips the FST and jumps straight to `0x115`.

## Next Suggested Action

1. Trace the adjacent unresolved shared-state writers at `0xd2ea4` / `0xd2ea8`.
   - Edna's `DNA_S_SUICIDE_NOTE` path still writes those two globals directly, and they sit in the same neutral-wrapper neighborhood as the now-bounded `d2e98` / `d2eb0` / `d2eec` Boyle/Herrill gates.
2. After the `0xd2ea4` / `0xd2ea8` writers are bounded, continue the wider wrapper scan outward from the same `0x38230`-`0x38380` cluster.
   - The surrounding dialogue-state helpers still offer the highest-signal path for recovering hidden startup-room branches without speculative renames.
