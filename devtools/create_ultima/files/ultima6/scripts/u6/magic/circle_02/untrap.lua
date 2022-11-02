local obj = select_obj()

magic_casting_fade_effect()

if obj == nil then magic_no_effect() return end

local found = false

fade_obj_blue(obj)

if (obj.obj_n >= 297 and obj.obj_n <= 300) or obj.obj_n == 98 then -- if door or chest
  local child
  for child in container_objs(obj) do  -- look through container for effect object.
    if child.obj_n == 337 then --effect
      found = true
      Obj.removeFromCont(child)
      break
    end
  end
end

if found == true then
	magic_success()
else
	magic_no_effect()
end
