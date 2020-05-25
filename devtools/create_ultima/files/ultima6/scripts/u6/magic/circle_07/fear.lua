local caster = magic_get_caster()

local x = caster.x
local y = caster.y
local z = caster.z
local loc_x = x
local loc_y = y

magic_casting_fade_effect(caster)

for x = x - 5,loc_x + 5 do
	for y = y - 5,loc_y + 5 do
		local actor = map_get_actor(x, y, z)

		if actor ~= nil then
			if actor.align ~= ALIGNMENT_GOOD then
				if actor_int_check(actor, caster) == false then
					local actor_base = actor_tbl[actor.obj_n]
					if actor_base == nil or actor_base[20] == 0 then -- if not undead
						actor.wt = WT_UNK_13
					end
				end
			end
		end
	end
end

magic_success()
