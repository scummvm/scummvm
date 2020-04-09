local obj = select_obj_with_projectile(0x18b)

if obj == nil then return end

if obj.obj_n == 122 or --candle
	obj.obj_n == 145 or --candelabra
	obj.obj_n == 164 or --fireplace
	obj.obj_n == 90 or --torch
	(obj.obj_n == 206 and obj.frame_n == 1) then --flaming brazier
		fade_obj_blue(obj)
		if obj.frame_n == 1 or obj.frame_n == 3 then
			if obj.obj_n == 90 then
        		Obj.use(obj) -- we want to use usecode to remove the torch
      		else
        		obj.frame_n = obj.frame_n - 1;
      		end
      		return magic_success()
		end
end

return magic_no_effect()
