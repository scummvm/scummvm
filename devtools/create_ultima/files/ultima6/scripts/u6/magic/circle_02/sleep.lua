  local caster = magic_get_caster()
  local actor = select_actor_with_projectile(0x17d, caster)

  if actor == nil then return end

  local ret = spell_put_actor_to_sleep(caster,actor)

  if ret == 1 then
  		magic_failed()
  elseif ret == 2 then
  		magic_no_effect()
  end
