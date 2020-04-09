function finish_dream_quest(actor)
   local schedule = Actor.get_schedule(actor, 0)

   Actor.move(actor, schedule.x, schedule.y, schedule.z)

   local obj = map_get_obj(schedule.x, schedule.y, schedule.z, 461) --OBJ_DREAM_TELEPORTER
   Obj.removeFromEngine(obj)
   obj = map_get_obj(schedule.x, schedule.y-1, schedule.z, 292) --OBJ_OBELISK
   Obj.removeFromEngine(obj)
   Actor.set_talk_flag(actor, 7)

   if Actor.get_talk_flag(0x56, 7)
           and Actor.get_talk_flag(0x54, 7)
           and Actor.get_talk_flag(0x52, 7)
           and Actor.get_talk_flag(0x55, 7) then
      Actor.set_talk_flag(0x60, 4)
   end

   if Actor.get_talk_flag(0x50, 7)
           and Actor.get_talk_flag(0x51, 7)
           and Actor.get_talk_flag(0x52, 7)
           and Actor.get_talk_flag(0x53, 7)
           and Actor.get_talk_flag(0x54, 7)
           and Actor.get_talk_flag(0x55, 7)
           and Actor.get_talk_flag(0x56, 7)
           and Actor.get_talk_flag(0x57, 7) then
      Actor.set_talk_flag(0x20, 4)
   end

end

function wake_from_dream()
   --FIXME dreamworld_cleanup_state() The original calls this with actor zero as an argument.
   g_objlist_1d22_unk = 0
   local minutes = 60 - clock_get_minute()
   local current_hour = clock_get_hour()
   current_hour = current_hour + 1
   if current_hour >= 8 then
      minutes = minutes + (24 - current_hour + 8) * 60
   else
      minutes = minutes + (8 - current_hour) * 60
   end
   clock_inc(minutes)
   --FIXME reset walk_direction_modifier

   for actor in party_members() do
      actor_clear_berry_counters(actor.actor_num)
      if actor.poisoned then
         if actor.hp <= 30 then
            if actor.hp <= 10 then
               actor.hp = 1
            else
               actor.hp = math.random(1, 10)
            end
         else
            actor.hp = math.random(24, 30)
         end
      end
   end
   local dream_actor = Actor.get(0)
   party_set_in_vehicle(false)
   dream_actor.visible = false
   party_show_all()
   party_update_leader()
   party_set_combat_mode(false)
   g_in_dream_mode = false
   map_enable_temp_actor_cleaning(true)
   printl("YOU_WAKE_UP")
   unlock_inventory_view()
   --remove dream actor's inventory
   for obj in actor_inventory(dream_actor, true) do
      Obj.removeFromEngine(obj)
   end
   local avatar = Actor.get(1)
   avatar.mpts = avatar.dex
   avatar.wt = WT_PLAYER
end


function actor_use_dream_machine(actor, dream_quality)
   if actor.actor_num ~= 1 then
      --FIXME advance time till dawn.
      printfl("ACTOR_DREAMS_UNTIL_DAWN_WHILE_THE_PARTY_WAITS", actor.name)
      actor.asleep = true
      g_party_is_warm = true
      advance_time(60 - clock_get_minute())
      while clock_get_hour() ~= 8 do
         advance_time(20)
         script_wait(100)
      end
      g_party_is_warm = false
   else
      play_midgame_sequence(4)
      party_set_party_mode()
      local dream_actor = Actor.get(0)

      local dream_x, dream_y
      if dream_quality == 1 then
         dream_x, dream_y = 0x64, 0x3b
      else
         dream_x, dream_y = 0x93, 0x34
      end

      --FIXME need to copy over more data from avatar actor.
      dream_actor.obj_n = actor.obj_n
      dream_actor.frame_n = actor.frame_n
      dream_actor.base_obj_n = actor.base_obj_n
      dream_actor.wt = WT_PLAYER
      dream_actor.visible = true
      dream_actor.hp = actor.max_hp

      Actor.move(dream_actor, dream_x, dream_y, 2)
      player_set_actor(dream_actor)
      party_set_in_vehicle(true)
      party_hide_all()
      g_in_dream_mode = true
      map_enable_temp_actor_cleaning(false)
      g_prev_player_x = 0
      g_prev_player_y = 0
      g_current_dream_stage = 0
      lock_inventory_view(Actor.get(0))
   end
end

function cleanup_cliff_fall()
   local dream_actor = Actor.get(0)
   local avatar = Actor.get(1)
   dream_actor.obj_n = avatar.base_obj_n
   dream_actor.frame_n = 9
end

local dreamworld_cleanup_tbl = {
   [5]=function() end,
   [0x20]=function() end,
   [0x40]=cleanup_cliff_fall,
   [0x44]=function() end,
   [0xa0]=function() end,
}

function fall_from_cliff()
   printl("YOU_FALL_OFF_THE_CLIFF")
   local dream_actor = Actor.get(0)
   if player_get_gender() == 0 then
      dream_actor.obj_n = 0x126
   else
      dream_actor.obj_n = 0x127
   end
   dream_actor.frame_n = 0

   for y=dream_actor.y-8,dream_actor.y do
      local actor = map_get_actor(dream_actor.x, y, dream_actor.z)
      if actor ~= nil and actor.obj_n == 391 then --OBJ_YOUR_MOTHER
         Actor.kill(actor, false)
      end
   end

end

local dreamworld_unk_tbl = {
   [0x4]=function() end,
   [0x40]=fall_from_cliff,
   [0x85]=function() end,
   [0xA1]=function() end,
   [0xA4]=function() end,
   [0xC0]=function() end,
   [0xC1]=function() end,
   [0xC4]=function() end,
   [0xC5]=function() end,
   [0xE0]=function() end,
}

function spawn_your_mother()
   local player_loc = player_get_location()
   local mother = Actor.new(391, player_loc.x, player_loc.y-1,player_loc.z)
   actor_init(mother)
   mother.wt = 0x15
   mother.combat_mode = 0x15
   mother.visible = true
   Actor.move(mother, player_loc.x, player_loc.y-1,player_loc.z)
end

function create_pitcher(x, y, z)
   local obj = Obj.new(217, math.floor(math.random(0, 5) / 2))
   obj.qty = 3
   Obj.moveToMap(obj, x, y, z)
end

function setup_tiffany_stage()
   local tiffany = Actor.get(0x54)

   Actor.clear_talk_flag(tiffany, 0)
   Actor.clear_talk_flag(tiffany, 6)

   local player_loc = player_get_location()
   local z = player_loc.z
   for obj in find_obj_from_area(0x21, 0x33, z, 0x1c, 0x10) do
      local obj_n = obj.obj_n
      --OBJ_RED_THROW_RUG, OBJ_RED_CAPE, OBJ_GLASS_PITCHER, OBJ_BROKEN_CRYSTAL, OBJ_MINOTAUR
      if obj_n == 161 or obj_n == 162 or obj_n == 217 or obj_n == 218 or obj_n == 398 then
         Obj.removeFromEngine(obj)
      end
   end
   local rug = Obj.new(161) --OBJ_RED_THROW_RUG
   Obj.moveToMap(rug, 0x24, 0x36, z)

   for i=57,65 do
      if i ~= 0x3e then
         create_pitcher(0x37, i, z)
      end
      if i > 0x3a and i ~= 0x40 then
         create_pitcher(0x39, i, z)
      end
      create_pitcher(0x3b, i, z)
   end

   create_pitcher(0x38, 0x39, z)
   create_pitcher(0x38, 0x41, z)
   create_pitcher(0x3a, 0x39, z)
   create_pitcher(0x3a, 0x41, z)

   local minotaur = Actor.new(398, 0x3b, 0x41, z)

   actor_init(minotaur)
   Actor.move(minotaur, 0x3b, 0x41, z)
   minotaur.wt = 0x16

end

function complete_tiffany_stage()
   local tiffany = Actor.get(0x54)
   finish_dream_quest(tiffany)
   Actor.set_talk_flag(tiffany, 1)
   Actor.talk(tiffany)
   wake_from_dream()
end

local dreamworld_init_tbl = {
   [0x5]=function() end,
   [0x20]=function() end,
   [0x25]=spawn_your_mother,
   [0x44]=setup_tiffany_stage,
   [0x60]=function() end,
   [0xA5]=function() end,
   [0xC0]=function() end,
   [0xC4]=function() end,
   [0xC5]=function() end,
}

function dreamworld_cleanup_state(obj)
   local dream_actor = Actor.get(0)
   local new_stage =  bit32.band(obj.status, 0xe5)
   if g_current_dream_stage ~= 0 and new_stage ~= 0xa5 and new_stage ~= 0xe5 then
      if dreamworld_cleanup_tbl[g_current_dream_stage] ~= nil then
         dreamworld_cleanup_tbl[g_current_dream_stage]()
      end

   end

--   print("new stage="..new_stage.."\n")

   if new_stage == 1 then
      g_current_dream_stage = 0
      return
   elseif new_stage == 0x24 then
      wake_from_dream()
      return
   elseif g_current_dream_stage ~= 0 and dreamworld_unk_tbl[new_stage] ~= nil then
      dreamworld_unk_tbl[new_stage]()
   end

   if new_stage ~= 0xa5 then
      --FIXME clean up temp objects
      g_objlist_1d22_unk = 0
   end

   actor_clear_berry_counters(dream_actor.actor_num)
   local player_loc = player_get_location()
   player_move(obj.quality, obj.qty, player_loc.z)

   if new_stage == 0xe5 then
      if not Actor.get_talk_flag(0x66, 3) then
         g_objlist_1d22_unk = 6
      end
   else
      if dreamworld_init_tbl[new_stage] ~= nil then
         dreamworld_init_tbl[new_stage]()
      end

      g_current_dream_stage = new_stage
   end

end
