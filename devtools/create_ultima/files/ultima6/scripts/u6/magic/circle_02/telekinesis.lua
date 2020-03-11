local caster = magic_get_caster()
item = select_actor_or_obj_with_projectile(0x17f)

if item == nil then return end

if item.luatype == "obj" then
	fade_obj_blue(item)
	if item.obj_n == 0x120 or item.obj_n == 0x10c then --crank, lever
		Obj.use(item)
	elseif item.weight >= 0.01 then
		local dir = get_direction("Direction-")
		print("`"..direction_string(dir).."\n")
		local new_x, new_y = direction_get_loc(dir, item.x, item.y)
		if map_can_put_obj(new_x, new_y, item.z) == true then
			Obj.moveToMap(item, new_x, new_y, item.z)
		else
			print("\nBlocked\n")
		end
	else
		print("\nNot Possible\n")
	end
else
	magic_no_effect() --do nothing with actors.
end
