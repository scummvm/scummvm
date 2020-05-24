local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17f, caster)

if actor == nil then return end

fade_actor_blue(actor)

if actor.alive == true then
	local hp = actor.hp
	local max_hp = actor.max_hp

	hp = hp + math.random(1, 0x1e)
	if hp > max_hp then
		hp = max_hp
	end

	actor.hp = hp

	magic_success()
else
	magic_no_effect()
end
