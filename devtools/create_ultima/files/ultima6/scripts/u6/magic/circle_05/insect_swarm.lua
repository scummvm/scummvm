magic_casting_effect()

local random = math.random
local loc = caster_get_location()
local caster = magic_get_caster()
local from_z = loc.z
local i, j

magic_casting_fade_effect(caster)

for i=1,8 do
   for j=1,8 do
      local new_x = random(0, 10) + loc.x - 5
      local new_y = random(0, 10) + loc.y - 5
      if map_can_put(new_x, new_y, from_z) then
         actor = Actor.new(0x157, new_x, new_y, from_z) --insect
         if actor ~= nil then
         	actor.align = caster.align
         	actor.wt = WT_ASSAULT
         end
         break
      end
   end
end

magic_success()
