local caster = magic_get_caster()
local obj = select_obj_with_projectile(0x17f, caster)

if obj == nil then return end

if tile_get_flag(obj.tile_num, 3, 3) == true then
 	g_vanish_obj.obj_n = obj.obj_n
	g_vanish_obj.frame_n = obj.frame_n

	fade_obj_out(obj)
	map_remove_obj(obj)

	magic_success()
else
	magic_no_effect()
end
