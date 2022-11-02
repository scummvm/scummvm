local obj = select_obj()

magic_casting_fade_effect()

if obj == nil then magic_no_effect() return end

fade_obj_blue(obj)

if (obj.obj_n >= 297 and obj.obj_n <= 300) or obj.obj_n == 98 then -- if door or chest

  -- find existing effect in obj container.
  local found_effect = false
  local child
  for child in container_objs(obj) do
    if child ~= nil and child.obj_n == 337 then
      found_effect = true
    end
  end

  child = nil

  --add effect if no existing effect found in container.
  if found_effect == false then
    local effect = Obj.new(337)
    Obj.moveToCont(effect, obj)
    effect = nil
    obj = nil
    return magic_success()
  end
end

magic_no_effect()

obj = nil
