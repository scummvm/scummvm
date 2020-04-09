local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x236, caster)

if actor == nil then return end

local random = math.random

if  (((actor_dex_adj(actor) / 2) + 0x1e) - actor_int_adj(caster)) / 2 <= random(1, 0x1e) then

	print("\n")

	local dmg = random(1, 0xa)
	local exp = actor_hit(actor, dmg)
	if exp ~= 0 then
		caster.exp = caster.exp + exp
	end

	actor_hit_msg(actor)

	actor_yell_for_help(caster, actor, 1)

else
	magic_failed()
end
