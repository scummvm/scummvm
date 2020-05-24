local USE_EVENT_USE = 0x01

function use_telescope(obj, actor)
	if obj.on_map == true then
		mapwindow_center_at_location(obj.x, obj.y, obj.z)
	else -- FIXME - should probably work in the inventory (can't be done without cheating though)
		print("Not usable\n")
		return
	end
	local dir = really_get_direction("Direction-")
	if dir == nil or dir == DIR_NONE then
		mapwindow_center_at_location(actor.x, actor.y, actor.z)
		print("nowhere.\n")
		return
	end
	local dir_string = direction_string(dir)
	dir_string = dir_string:gsub("^%l", string.upper)
	print(dir_string..".\n")

	local loc = mapwindow_get_location()

	--FIXME need to fade out blacking.

	mapwindow_set_enable_blacking(false)

	for i=0,40 do
		loc.x,loc.y = direction_get_loc(dir,loc.x, loc.y)
		mapwindow_set_location(loc.x, loc.y, loc.z)
		script_wait(50)
	end

	script_wait(600)

	mapwindow_set_enable_blacking(true)

	mapwindow_center_at_location(actor.x, actor.y, actor.z)
	print("\nDone\n")

end

function use_silver_horn(obj, actor)
	local i
	for i=0,0xff do
		local tmp_actor = Actor.get(i)
		if tmp_actor.obj_n == 413 and tmp_actor.alive == true and actor_find_max_xy_distance(tmp_actor, actor.x, actor.y) <= 8 then
			print("Not now!\n")
			return
		end
	end

	local random = math.random

	for i=1,3 do
		local new_x = random(1, 7) + random(1, 7) + actor.x - 8
		local new_y = random(1, 7) + random(1, 7) + actor.y - 8
		local snake = Actor.new(413, new_x, new_y, actor.z, ALIGNMENT_CHAOTIC)
	end

	print("Silver snakes are generated!\n")
end


local usecode_table = {
[154]=use_telescope,
[313]=use_silver_horn
}

function has_usecode(obj, usecode_type)
	if usecode_type == USE_EVENT_USE and usecode_table[obj.obj_n] ~= nil then
		return true
	end

	return false
end

function use_obj(obj, actor)
	if type(usecode_table[obj.obj_n]) == "function" then
		local func = usecode_table[obj.obj_n]
		if func ~= nil then
			print("\n")
			func(obj, actor)
		end
	else
		use_obj_on(obj, actor, usecode_table[obj.obj_n])
	end
end

function move_obj(obj, rel_x, rel_y)
  return false
end

function is_ranged_select(operation)
    return false
end
