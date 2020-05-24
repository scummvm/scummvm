local obj = select_obj()

magic_casting_fade_effect()

if obj == nil then magic_no_effect() return end

if obj.obj_n >= 297 and obj.obj_n <= 300 and obj.frame_n >= 13 then --magically locked door
	obj.frame_n = obj.frame_n - 8
	return print("\nunlocked!\n");
elseif obj.obj_n == 0x62 and obj.frame_n == 3 then --magically locked chest
	obj.frame_n = 1
	return print("\nunlocked!\n");
end

magic_no_effect()
