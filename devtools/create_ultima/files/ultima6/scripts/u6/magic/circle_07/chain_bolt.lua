local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x188, caster)

if actor == nil then return end

local random = math.random

local exp = actor_hit(actor, random(1, 0x1e))
if exp ~= 0 then
	caster.exp = caster.exp + exp
end

actor_yell_for_help(caster, actor, 1)
actor_hit_msg(actor)

local i,j

for i=0,6 do
	if g_avatar_died == true then
		break -- don't keep casting once Avatar is dead
	end
	local var_c = 32769
	local new_target = nil
	for j=1,0xff do
		local tmp_actor = Actor.get(j)

		if tmp_actor.obj_n ~= 0 and tmp_actor.alive == true and tmp_actor.actor_num ~= actor.actor_num and actor_ok_to_attack(actor, tmp_actor) == true then
			local target_x = tmp_actor.x
			local target_y = tmp_actor.y

			if tmp_actor.z == caster.z and target_x > caster.x - 5 and target_x < caster.x + 5 and target_y > caster.y - 5 and target_y < caster.y + 5 then

				local val = (target_x - actor.x) * (target_x - actor.x) + (target_y - actor.y) * (target_y - actor.y)
				dbg("and here val = "..val.."\n")
				if val > 0 then
					if val <= var_c then
						if val == var_c then
							if random(0, 1) == 0 then
								new_target = tmp_actor
							end
						else
							var_c = val
							new_target = tmp_actor
						end
					end
				end
			end
		end
	end

	if new_target == nil then
		break
	end

	projectile(0x188, actor.x, actor.y, new_target.x, new_target.y, 2, 0)

	actor = new_target

	local exp = actor_hit(actor, random(1, 0x1e))
	if exp ~= 0 then
	caster.exp = caster.exp + exp
	end

	actor_yell_for_help(caster, actor, 1)
	actor_hit_msg(actor)
end
