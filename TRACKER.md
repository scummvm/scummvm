# TRACKER

## Current Focus

- Ghidra-only recovery in `HARVEST.LE`
- Continue scanning high-confidence `FUN_*` clusters from proven caller/callee paths
- Prefer dialogue/text/NPC-state helpers tied to already-confirmed record layouts

## Progress

- Program: `HARVEST.LE`
- Total functions: `886`
- Named/documented: `568`
- Still `FUN_*` / undocumented: `318`

## Last Confirmed Action

- Traced the neutral shared talk-state wrappers at `0x38230`, `0x382a0`, and `0x38380`, then renamed them in Ghidra as `get_set_shared_dialogue_state_d2e98`, `get_set_shared_dialogue_state_d2eb0`, and `get_set_shared_dialogue_state_d2eec`.
  - The recovered gameplay callers are all read-only: Herrill's `GASCAN` path calls `0x38230` at `0x2e056` with `eax = 1` / `edx = 0`, Herrill's `HAVE_LODGE_APP` continuation calls `0x38380` at `0x2e10f` with the same read-only convention, and the only current caller of `0x382a0` is a hidden Boyle dialogue branch at `0x2d6e3`, which also passes `eax = 1` / `edx = 0`.
  - No gameplay-side setter call has been recovered yet for `0xd2e98`, `0xd2eb0`, or `0xd2eec`. In the current database, those globals are otherwise only touched by the shared reset helper and, for `0xd2e98`, save/load serialization.

## Next Suggested Action

1. Continue the wider helper scan around the `0x38230`-`0x38380` dialogue-state wrapper cluster, starting with the hidden Boyle block around `0x2d620`.
   - That block now owns the only recovered gameplay read of `get_set_shared_dialogue_state_d2eb0`, and it is adjacent to the already-bounded `get_set_boyle_gascan_application_state @ 0x38240` plus the still-unresolved helper at `0x38620`.
2. After the Boyle-side helper cluster is bounded, trace the adjacent unresolved shared-state writers at `0xd2ea4` / `0xd2ea8`.
   - Edna's `DNA_S_SUICIDE_NOTE` path still writes those two globals directly, and they sit in the same neutral-wrapper neighborhood as the newly bounded `d2e98` / `d2eb0` / `d2eec` gates.
