  local caster = magic_get_caster()
  local actor = select_actor_with_projectile(0x17e, caster)

  if actor == nil then return end

  spell_take_fire_dmg(caster, actor)
