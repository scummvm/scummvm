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
			local actor_type = actor_tbl[actor.obj_n]
			if actor_type ~= nil and actor_type[14] == 1 and actor_int_check(actor, caster) == false then --14 repel undead flag
				actor.wt = WT_UNK_13
				dbg("setting actor["..actor.actor_num.."] to timid worktype.\n")
			end
		end
	end
end

magic_success()
