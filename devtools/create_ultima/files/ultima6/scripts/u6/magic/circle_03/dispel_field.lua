-- Dispel Field,  An Grav

-- 317 Fire Field
-- 318 Poison Field
-- 319 Protection Field
-- 320 Sleep Field

local obj = select_obj_with_projectile(0x17f)

if obj == nil then return end


if obj.obj_n >= 317 and obj.obj_n <= 320 then
  hit_anim(obj.x, obj.y)
  map_remove_obj(obj)
  obj = nil
  return magic_success()
end


obj = nil
magic_no_effect()
