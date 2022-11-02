  local actor = select_actor_with_projectile(0x17f)

  if actor == nil then return end

  actor.protected = true
  magic_success()
