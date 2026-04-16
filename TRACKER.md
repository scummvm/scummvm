# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `929`
- Named/documented: `825`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On April 16, 2026, checked the Inquisitor live `MONSTERFY` spawn path against Ghidra. Confirmed native `update_actor_runtime_state` copies the dying NPC entity render Z into the target monster record before spawning it, native NPC/monster records spawn with z extent `5`, and native active monster spawn seeds a standing facing frame rather than a walking loop. Updated the Harvester shim to mirror those three behaviors.

## Next Suggested Action

- Re-test `PAIN` through `INQUIST_ATTACK_TIMER` and Inquisitor defeat. Verify the monster appears without a screen-depth jump, remains visually stable while standing, then uses the walking bank only while actually chasing.
