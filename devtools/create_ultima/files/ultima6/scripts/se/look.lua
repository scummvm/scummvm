local look_usecode = {
}

function look_obj(obj)
   printfl("YOU_SEE", obj.look_string);

   --FIXME usecode look description should be lua code.
   if usecode_look(obj) then
      print("\n")
      return false
   end

   print(".\n\n");

   if look_usecode[obj.obj_n] ~= nil then
      look_usecode[obj.obj_n](obj)
      print("\n")
   end

   if is_container_obj(obj.obj_n) then
      search(obj)
   end

   return false
end
