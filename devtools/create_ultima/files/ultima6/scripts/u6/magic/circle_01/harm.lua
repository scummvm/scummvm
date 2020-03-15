local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17f, caster)

if actor == nil then return end

if spell_hit_actor(caster, actor, 5) == false then --5 = harm spell number
	magic_failed()
end
