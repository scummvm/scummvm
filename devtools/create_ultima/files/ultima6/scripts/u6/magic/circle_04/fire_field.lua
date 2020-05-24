  loc = select_location_with_projectile(0x17e)

  if loc == nil then return end

  if map_can_put_obj(loc) and map_is_water(loc.x,loc.y,loc.z) == false then
     obj = Obj.new(317); --fire field
     obj.temporary = true
     obj.x,obj.y,obj.z = loc.x,loc.y,loc.z
     Obj.moveToMap(obj)
     fade_obj_in(obj)
     magic_success()
  else
     magic_not_possible()
  end
