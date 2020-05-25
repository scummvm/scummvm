local caster = magic_get_caster()
local obj = select_obj()

magic_casting_fade_effect(caster)

if obj == nil then magic_no_effect() return end

fade_obj_blue(obj)
local found = false

local dice_roll = math.random(1, 0x1e)
local actor_int = actor_int_adj(caster)

if actor_int >= dice_roll then
	local child
	for child in container_objs(obj) do  -- look through container for effect object.
		if child.obj_n == 337 then --effect
			found = true
			print("\nIt's trapped.\n");
			break
		end
	end
	child = nil
end

if found == false then
    print("\nNo trap.\n");
end

obj = nil
