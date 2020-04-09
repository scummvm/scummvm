local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17f, caster)

if actor == nil then return end

local from_x = caster.x
local from_y = caster.y
local from_z = caster.z

local random = math.random

for i=1,8 do
	local new_x = random(0, 10) + from_x - 5
	local new_y = random(0, 10) + from_y - 5
	if map_can_put(new_x, new_y, from_z) then
		local clone_actor = Actor.new(actor.obj_n, new_x, new_y, from_z)
		if clone_actor ~= nil then
			clone_actor.frame_n = actor.frame_n
			clone_actor.align = actor.align
			if actor.wt > WT_PLAYER then
				clone_actor.wt = actor.wt
			else
				clone_actor.wt = WT_ASSAULT
			end
			fade_actor_in(clone_actor)
			magic_success()
			return
		end
		break
	end
end

magic_no_effect()
