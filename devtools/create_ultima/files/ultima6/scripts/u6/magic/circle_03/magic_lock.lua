local obj = select_obj()

magic_casting_fade_effect()

if obj == nil then magic_no_effect() return end

if obj.obj_n >= 297 and obj.obj_n <= 300 then
   if obj.frame_n <= 7 then
      if obj.frame_n <= 3 then
         print("\nCan't (Un)lock an opened door\n")
      else
          --FIXME need to handle a door which is already magically locked.
         if obj.frame_n >= 5 and obj.frame_n <= 7 then
            obj.frame_n = obj.frame_n + 8
            print("\nmagically locked!\n")
         end
      end
      magic_success()
   end
elseif obj.obj_n == 0x62 and (obj.frame_n == 1 or obj.frame_n == 2) then
	obj.frame_n = 3
	print("\nmagically locked!\n")
else
  magic_no_effect()
end
