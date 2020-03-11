local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17f, caster)

if actor == nil then return end

fade_actor_blue(actor)

if actor.alive == true then
	actor.hp = actor.max_hp
	magic_success()
else
	magic_no_effect()
end
