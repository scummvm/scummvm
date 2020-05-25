local obj = select_obj()

magic_casting_fade_effect()

if obj ~= nil and g_armageddon == false then
  fade_obj_blue(obj)
-- if statement is redundant with ObjManager::is_corpse(Obj *obj) but not really worth adding to the scripting language
  --dead body, mouse, dead gargoyle, dead cyclops;
  if (obj.obj_n == 339 or obj.obj_n == 0x162 or obj.obj_n == 0x155 or obj.obj_n == 0x154 or
      obj.obj_n == 349 or obj.obj_n == 134) then --dog, horse carcass
    if obj.quality < 203 then --greater than 202 temp actors
      if obj.quality ~= 0 then
        local actor = Actor.get(obj.quality)
        Actor.resurrect(actor, obj.x, obj.y, obj.z, obj)
        magic_success()
      else
        print("\nThe corpse is too old!\n")
      end
    end
  else
    magic_no_effect()
  end
else
  magic_no_effect()
end
