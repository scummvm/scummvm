local USE_EVENT_USE = 0x01

local usecode_table = {--[[
[86]={["func"]=use_container},
[87]={["func"]=use_container},
[102]={["on"]={[86]=use_crate,[427]=use_prybar_on_hatch}},
[104]={["func"]=use_container},
[152]={["func"]=use_door},
[222]={["func"]=use_door},
[273]={["on"]={[86]=use_crate}}, --Hammer needs more codes
[284]={["func"]=use_container},
[421]={["func"]=use_door},
[427]={["func"]=use_hatch},
--]]
}


function has_usecode(obj, usecode_type)
   if usecode_type == USE_EVENT_USE and usecode_table[obj.obj_n] ~= nil then
      return true
   end

   return false
end

function use_obj(obj, actor)
   if usecode_table[obj.obj_n].on ~= nil then

      local target_obj = get_target_obj("On - ")
      if target_obj ~= nil then
         print(target_obj.name .. "\n")
         local func = usecode_table[obj.obj_n].on[target_obj.obj_n]
         if func ~= nil then
            func(obj, target_obj, actor)
         else
            print("\nNo effect\n")
         end
      else
         print("nothing!\n")
      end
   else
      local func = usecode_table[obj.obj_n].func
      if func ~= nil then
         func(obj, actor)
      end
   end
end

function move_obj(obj, rel_x, rel_y)
   return false
end

function is_ranged_select(operation)
   return false
end