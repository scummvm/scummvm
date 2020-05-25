local caster = magic_get_caster()
loc = player_get_location()
actor = Actor.new(367, loc.x, loc.y-1, loc.z, caster.align, 8)
