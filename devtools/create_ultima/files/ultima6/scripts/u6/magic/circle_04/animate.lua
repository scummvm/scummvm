local obj = select_obj_with_projectile(0x17f)

if obj == nil then return end

if obj_is_readiable(obj) then
	actor = Actor.new(obj.obj_n, obj.x, obj.y, obj.z, ALIGNMENT_GOOD, 8)
	map_remove_obj(obj)
	magic_success()
else
	magic_no_effect()
end
