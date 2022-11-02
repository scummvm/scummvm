function open_gates_at_olympus_mons()
   local gate = map_get_obj(0x2c3, 0x1f3, 0, 181) -- OBJ_GATE
   if gate ~= nil then
      gate.x = 0x2c2
      gate.frame_n = 3
   else
      printl("AARGH")
   end

   gate = map_get_obj(0x2c4, 0x1f3, 0, 181) -- OBJ_GATE
   if gate ~= nil then
      gate.frame_n = 7
   else
      printl("AARGH")
   end

end

function open_dream_machine_door()
   local door = map_get_obj(0x2c7, 0x1dc, 0, 152) --OBJ_DOOR
   if door ~= nil then
      door.frame_n = 7
   end
end

function talk_script_fix_panels()
   local numPanels = 0
   for actor in party_members() do
      for obj in actor_inventory(actor) do
         if obj.obj_n == 458 then --OBJ_PANEL
            numPanels = numPanels + 1
            play_md_sfx(4)
            obj.quality = bit32.band(obj.quality, 0xfd)
            obj.qty = 4
         end
      end
   end

   if numPanels <= 1 then
      Actor.clear_talk_flag(0x39, 3)
   else
      Actor.set_talk_flag(0x39, 3)
   end

end

function talk_script_spawn_monster_footprints()
   g_objlist_1d22_unk = 0
   local player_loc = player_get_location()
   for i=0,5 do
      local monster = Actor.new(145, player_loc.x, player_loc.y, player_loc.z) --OBJ_MONSTER_FOOTPRINTS

      actor_init(monster)
      toss_actor(monster, player_loc.x, player_loc.y, player_loc.z, 0)
      monster.wt = 0x8
      g_objlist_1d22_unk = g_objlist_1d22_unk + 1
   end
end

local talk_script_tbl = {
   [1]=talk_script_fix_panels,
   [6]=open_gates_at_olympus_mons,
   [7]=open_dream_machine_door,
   [9]=function() talk_script_status = 9 end,
   [0xA]=function() talk_script_status = 0xA end,
   [0x35]=talk_script_spawn_monster_footprints,
}

function talk_script(script_number)
   if talk_script_tbl[script_number] ~= nil then
      talk_script_tbl[script_number]()
   else
      print("Attempting to run talk script #"..script_number.."\n")
   end
end

local talk_script_status = -1

local talk_script_post_action_tbl = {
   [0x9]=function() play_end_sequence() end,
   [0xA]=function() end,
   [0x34]=wake_from_dream,
   [0x36]=wake_from_dream,
   [0x38]=function() end,
   [0x65]=function() end,
   [0x66]=function() end,
   [0x67]=function() end,
   [0x68]=function() end,
   [0x69]=function() end,
}

function talk_to_actor(actor)
   local actor_num = actor.actor_num

   if actor_num < 2 then
      if g_in_dream_mode then
         printl("YOU_TRY_TO_WAKE_YOURSELF_UP")
         local player_loc = player_get_location()
         if player_loc.z ~= 3 then
            wake_from_dream()
         end
      elseif player_is_in_solo_mode() then
         printl("NOT_WHILE_IN_SOLO_MODE")
      else
         printl("YOU_ARENT_YET_THAT_INSANE")
      end
      return true
   end

   if actor.obj_n == 391 then --your mother
      if player_get_gender() == 0 then
         printl("NO_BACKTALK_FROM_YOU_YOUNG_MAN")
      else
         printl("NO_BACKTALK_FROM_YOU_YOUNG_WOMAN")
      end
      return true
   end

   print(actor.name.."\n")
   Actor.talk(actor_num)
   print("\n")

   if talk_script_post_action_tbl[talk_script_status] ~= nil then
      talk_script_post_action_tbl[talk_script_status]()
   end
   talk_script_status = -1

   return true
end

function talk_conveyor()
   talk_to_actor(Actor.get(0x72))
end

function talk_tower()
   talk_to_actor(Actor.get(0x73))
end

function talk_dream_machine()
   talk_to_actor(Actor.get(0x74))
end

local talk_obj_tbl = {
   [0xC] = function() printl("YOU_RECEIVE_A_MOMENTARY_IMPRESSION") end,
   [0x5C] = function() printl("THIS_WAS_WORN_LAST_BY_A_DYING_MAN") end,
   [0x64] = talk_dream_machine,
   [0xBC] = talk_conveyor,
   [0xBF] = talk_conveyor,
   [0xC0] = talk_conveyor,
   [0xC8] = talk_tower,
   [0xC9] = talk_tower,
   [0xD6] = talk_tower,
   [0xD7] = talk_tower,
   [0xE7] = function() printl("THIS_WAS_USED_BY_A_MARTIAN_MEASURING_TIME") end,
   [0xEA] = function() printl("THIS_JEWELRY_WAS_WORN_BY") end,
   [0xF6] = function() printl("THESE_FOOTBAGS_WERE_WORN_BY") end,
   [0xF7] = function() printl("THIS_SAW_WAS_USED_BY") end,
   [0xF9] = function() printl("THIS_DEVICE_WAS_USED_BY") end,
   [0x113] = function() printl("THIS_SCULPTURE_HAD_GREAT_RITUAL_OR_RELIGIOUS_SIGNIFICANCE") end,
   [0x120] = talk_dream_machine,
   [0x121] = talk_dream_machine,
   [0x122] = talk_dream_machine,

}

function talk_to_obj(obj)

   local player = Actor.get_player_actor()
   if actor_is_affected_by_green_berries(player.actor_num) then
      if bit32.band(obj.status, 1) == 1 then
         local talk_function = talk_obj_tbl[obj.obj_n]
         if talk_function ~= nil then
            talk_function()
            return true
         end
      else
         printl("YOU_RECIEVE_NO_PSYCHIC_IMPRESSIONS")
      end
   else
      printl("NOTHING")
   end

   return false
end
