# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `929`
- Named/documented: `825`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, checked the Inner Sanctum Herrill/Grand Muck dialogue branch against Ghidra. Confirmed the native branch plays the logged dialogue, then calls `queue_live_named_npc_death_or_monsterfy_transition` with `GRAND_MUCK` before setting `PC_TALKED_TO_HERRILL`. Updated the Harvester dialogue shim to queue the live Grand Muck NPC transition instead of finalizing the runtime NPC immediately.

## Next Suggested Action

- Re-test `INNERSANCTUM` from entrance `L2_2_M1`. Verify the Herrill intro dialogue ends by visibly transitioning Grand Muck in-place, and that `PC_TALKED_TO_HERRILL` prevents the intro from replaying on re-entry.
