local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x188, caster)

if actor == nil then return end

if caster_is_player() then
	print("\n")
end

local exp = actor_hit(actor, math.random(1, 30))
if exp ~= 0 then
	caster.exp = caster.exp + exp
end

actor_hit_msg(actor)

actor_yell_for_help(caster, actor, 1)
