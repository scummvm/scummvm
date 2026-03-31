# TRACKER

## Progress

- Program: `HARVEST.LE`
- Total functions: `927`
- Named/documented: `823`
- Remaining `FUN_*`: `104`
- Remaining undocumented total: `104`

## Last Confirmed Action

- On March 31, 2026, rechecked `RenderEntityRuntime` against `spawn_monster_entity_from_record`, `spawn_player_combat_avatar`, `set_player_combat_loadout`, `spawn_npc_entity_from_record`, and `update_actor_runtime_state`; confirmed `+0x11b8` is the reciprocal shove/collision partner pointer and renamed it to `collision_partner_entity`, renamed `+0x11bc` to `player_combat_loadout_id`, and documented the additional confirmed actor-tail roles (`+0x112c engage_distance`, `+0x113c attack_contact_frame_offset`, `+0x1180 attack_damage_amount`, `+0x1188 last_hit_combat_flags`, `+0x118c combat_capability_flags`, `+0x11a4 combat_target_entity`) in ScummVM comments and `ARCHITECTURE.md`. Verified with `git diff --check`.

## Next Suggested Action

- Isolate the remaining unresolved `RenderEntityRuntime` tail/shared-header slots around `+0x119c`, `+0x11cc`, and `+0x4f/+0x50/+0x51/+0x53/+0x54` with save/load or DOSBox combat traces before renaming them, because the current evidence is still strong enough to classify behavior but not to assign stable semantic names.
