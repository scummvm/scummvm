local caster = magic_get_caster()
local obj = select_obj_with_projectile(0x17f, caster)

if obj == nil then return end

if tile_get_flag(obj.tile_num, 3, 3) == true then

	local replicated_obj = Obj.new(obj.obj_n, obj.frame_n)

	replicated_obj.qty = 1
	replicated_obj.status = 0x21 --OK, TEMP

	local loc = {}, i
	local random = math.random
	local caster_x = caster.x
	local caster_y = caster.y

	loc.z = caster.z

	for i=1,8 do
		loc.x = caster_x + random(0, 10) - 5
		loc.y = caster_y + random(0, 10) - 5

		if map_can_put_obj(loc) == true then
			Obj.moveToMap(replicated_obj, loc)
			fade_obj_in(replicated_obj)

			return magic_success()
		end
	end

	magic_failed()
else
	magic_no_effect()
end
