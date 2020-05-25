local obj = select_obj_with_projectile(0x18d)

if obj == nil then return end

if obj.obj_n == 122 or --candle
	obj.obj_n == 145 or --candelabra
	obj.obj_n == 164 or --fireplace
	obj.obj_n == 90 or --torch
	(obj.obj_n == 206 and obj.frame_n == 0) then --flaming brazier
		fade_obj_blue(obj)
		if obj.frame_n == 0 or obj.frame_n == 2 then
			if obj.obj_n == 90 then
				Obj.use(obj) -- we want to use usecode to remove the torch
			else
				obj.frame_n = obj.frame_n + 1;
			end

			return magic_success()
		end
elseif obj.obj_n == 0xdf and obj.frame_n == 0 then --unlit keg
	use_keg(obj)
	return magic_success()
end

return magic_no_effect()
