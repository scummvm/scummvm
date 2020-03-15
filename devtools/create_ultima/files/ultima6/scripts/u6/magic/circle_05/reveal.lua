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
			if actor.visible == false then
				actor.visible = true
			end
		end
	end
end

magic_success()
