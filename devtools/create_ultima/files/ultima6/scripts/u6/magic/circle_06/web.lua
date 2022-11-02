local caster = magic_get_caster()

local loc = select_location_with_prompt("Location: ")

print("\n")

magic_casting_fade_effect(caster)

if loc == nil then magic_no_effect() return end

local target_x = loc.x
local target_y = loc.y
local target_z = caster.z


local targets = {}
local i,j
for i=target_x-1,target_x+1 do
	for j = target_y-1,target_y+1 do
		table.insert(targets,{x=i,y=j,z=target_z})
	end
end

projectile_anim_multi(564, caster.x, caster.y, targets, 2, 0, 0) --564 = web tile num

local hit_x, hit_y =  map_line_hit_check(caster.x, caster.y, loc.x, loc.y, loc.z)

if hit_x ~= loc.x or hit_y ~= loc.y then magic_blocked() return end

local k, v
for k,v in ipairs(targets) do
	if map_can_put_obj(v) then
		local obj = Obj.new(0x35, 0) --web obj
		Obj.moveToMap(obj, v)
	end
end

magic_success()
