local caster = magic_get_caster()
magic_casting_effect()
magic_casting_fade_effect(caster)

local random = math.random

local i = random(0, 0xf)

local obj_n = 0

if i <= 3 then
	obj_n = 0x173 --troll
elseif i <= 6 then
	obj_n = 0x156 --giant rat
elseif i <= 9 then
	obj_n = 0x169 --giant spider
elseif i <= 0xc then
	obj_n = 0x158 --giant bat
else
	obj_n = 0x166 --snake
end

local from_x = caster.x
local from_y = caster.y
local from_z = caster.z

for i=1,8 do
	local new_x = random(0, 10) + from_x - 5
	local new_y = random(0, 10) + from_y - 5
	if map_can_put(new_x, new_y, from_z) then
		local actor = Actor.new(obj_n, new_x, new_y, from_z)
		if actor ~= nil then
			actor.align = caster.align
			actor.wt = WT_ASSAULT
			magic_success()
			return
		end
		break
	end
end

magic_failed()
