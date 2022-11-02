function select_target_actor(src_actor)
   local var_10 = 0x7fff
   local target_actor
   for i=0,0xff do
      local actor = Actor.get(i)
      local actor_align = actor.align
      local actor_obj_n = actor.obj_n
      local src_actor_align = src_actor.align
      local src_actor_obj_n = src_actor.obj_n
      if actor_obj_n ~= 0 and actor.alive and actor.visible and actor.actor_num ~= src_actor.actor_num and actor_obj_n ~= 381 then --OBJ_DUST_DEVIL
         if src_actor_align ~= ALIGNMENT_NEUTRAL or actor_align ~= ALIGNMENT_NEUTRAL then
            if src_actor_align ~= ALIGNMENT_CHAOTIC or (src_actor_obj_n ~= actor_obj_n
                    and (src_actor_obj_n ~= 372 or (actor_obj_n ~= 370 and actor_obj_n ~= 371))
                    and ((src_actor_obj_n ~= 362 and src_actor_obj_n ~= 373 and src_actor_obj_n ~= 254)
                        or (actor_obj_n ~= 362 and actor_obj_n ~= 373 and actor_obj_n ~= 254)))
                    and (src_actor_align ~= ALIGNMENT_GOOD or actor_align == ALIGNMENT_EVIL or actor_align == ALIGNMENT_CHAOTIC)
                    and (src_actor_align ~= ALIGNMENT_EVIL or actor_align == ALIGNMENT_GOOD or actor_align == ALIGNMENT_CHAOTIC)
                    and actor_find_max_wrapped_xy_distance(src_actor, actor.x, actor.y) <= 0xb
                    and (actor.wt ~= 7 or actor_find_max_wrapped_xy_distance(Actor.get_player_actor(), actor.x, actor.y) <= 5)
             then
               local var_C = get_wrapped_dist(actor.x, src_actor.x)^2 + get_wrapped_dist(actor.y, src_actor.y)^2
               if var_C < var_10
                       or (var_C == var_10 and actor_get_damage(actor) > actor_get_damage(target_actor)) then
                  var_10 = var_C
                  target_actor = actor
               end
            end
         end
      end
   end
   return target_actor
end

function worktype_8_combat_attack(actor)
   g_selected_obj = select_target_actor(actor)

end

function worktype_15_your_mother(actor)
   if actor_move(actor, DIR_SOUTH) == false then
      local target_actor = select_target_actor(actor)
      if target_actor ~= nil then
         --FIXME do combat range check. sub_1B305
         actor_take_hit(actor, target_actor, 6, 0)
      end
   end
   subtract_movement_pts(actor, 3)
end

function worktype_stomp_around(actor)
   subtract_movement_pts(actor, 5)
   if actor.obj_n == 398 and g_current_dream_stage == 0x44 then
      if math.random(0, 1) == 0 then
         play_md_sfx(0, true)
         quake_start(1,50)
      else
         return
      end
   end

   local direction = math.random(0, 3)
   actor_move(actor, direction, 0)
   for obj in objs_at_loc(actor.xyz) do
      actor_hit(obj, math.random(0, 0xa) + math.random(0, 0xa), 0)
   end

end

function worktype_16_minotaur(actor)
   if g_current_dream_stage == 0x60 then
      Actor.kill(actor)
      return
   end

   local dream_actor = Actor.get(0)
   local target
   if Actor.inv_has_obj_n(dream_actor, 162) then --OBJ_RED_CAPE
      target = dream_actor
      g_selected_obj = target
   else
      for obj in find_obj_from_area(0x21, 0x33, 2, 0x1c, 0x10) do
         if obj.obj_n == 162 then --OBJ_RED_CAPE
            target = obj
         end
      end
   end

   if target == nil then
      worktype_stomp_around(actor)
      return
   end

   if actor_get_combat_range(actor, target.x, target.y) <= 1 then
      if target.luatype == "actor" then
         attack_target_with_weapon(actor, target.x, target.y, actor)
         subtract_movement_pts(actor, 0xa)
         return
      end

      hit_target(target, RED_HIT_TILE)
      if not Actor.get_talk_flag(0x54, 6) and target.x >= 0x37 and target.y <= 0x37 then
         complete_tiffany_stage()
         return
      end
   end

   actor_move_towards_loc(actor, target.x, target.y)
   play_md_sfx(0, true)
   quake_start(1,50)
   for obj in objs_at_loc(actor.xyz) do
      actor_hit(obj, math.random(1, 0xa) + math.random(1, 0xa), 0)
   end
   subtract_movement_pts(actor, 5)

end



function worktype_99_coker_move_to_coal_vein(actor)
   if actor_move(actor, DIR_NORTH) == false then
      local vein = map_get_obj(actor.x, actor.y-1, actor.z, 446) --OBJ_VEIN_OF_COAL FIXME should be -2 not -1 need to fix actor_move for coker.
      if vein ~= nil then
         if map_is_on_screen(actor.x, actor.y, actor.z) then
            play_md_sfx(0x10)
         end
         actor.wt = 0x9A
      end
   end
end

function worktype_9A_coker_drop_coal(actor)
   local obj = map_get_obj(actor.x, actor.y+1, actor.z, 188) --OBJ_CONVEYOR_BELT
   if obj ~= nil then
      local coal = Obj.new(447) --OBJ_HUGE_LUMP_OF_COAL
      Obj.moveToMap(coal, actor.x, actor.y+1, actor.z)
   else
      obj = map_get_obj(actor.x, actor.y+1, actor.z, 192) --OBJ_BARE_ROLLERS
      if obj == nil then
         actor_move(actor, DIR_SOUTH)
         return
      end
   end
   actor.wt = 0x9B
end

function worktype_9B_coker_wait_for_coal_to_move_away(actor)
   local obj = map_get_obj(actor.x, actor.y+1, actor.z, 447) --OBJ_HUGE_LUMP_OF_COAL
   if obj == nil then
      obj = map_get_obj(actor.x, actor.y+1, actor.z, 192) --OBJ_BARE_ROLLERS
      if obj == nil then
         actor.wt = 0x99
      end
   end
end

function worktype_9D_stoker_wait_for_coal(actor)
   local coal
   coal = map_get_obj(actor.x, actor.y+1, actor.z, 447) --OBJ_HUGE_LUMP_OF_COAL

   if coal ~= nil then
      while coal ~= nil do
         Obj.removeFromEngine(coal)
         coal = map_get_obj(actor.x, actor.y+1, actor.z, 447) --OBJ_HUGE_LUMP_OF_COAL
      end
      actor.wt = 0x9E
   end

end

function worktype_9E_stoker_walk_to_furnace(actor)
   if actor_move(actor, DIR_NORTH) == false then
      local furnace = map_get_obj(actor.x, actor.y-1, actor.z, 233)
      if furnace == nil then
         furnace = map_get_obj(actor.x+1, actor.y-1, actor.z, 233)
      end

      if furnace ~= nil then
         if Actor.get_talk_flag(0x72, 2) == false then
            activate_power_system()
         else
            if Actor.get_talk_flag(0x73, 2) == false or Actor.get_talk_flag(0x71, 3) == true then
               if Actor.get_talk_flag(0x71, 3) == true then
                  Actor.clear_talk_flag(0x73, 2)
                  Actor.clear_talk_flag(0x71, 3)
                  --FIXME sub_3F740
               end
               Actor.set_talk_flag(0x73, 2)
               activate_power_update_tiles()
               activate_tower_electricity()
               midgame_cutscene_2()
            end
         end
         actor.wt = 0x9C
      else
         stoker_blocked(actor)
      end
   end
end

function activate_tower_electricity()
   for obj in find_obj(0, 201) do --OBJ_TOWER_TOP
      if obj ~= nil then
         if obj.x >= 0x3d0 and obj.x <= 0x3f0 and obj.y >= 0x1d0 and obj.y <= 0x1e7 then
            local frame_n = obj.frame_n
            obj.frame_n = bit32.bor(frame_n, 4)
         end
      end
   end

   for obj in find_obj(0, 214) do --OBJ_POWER_CABLE
      if obj ~= nil then
         if obj.x >= 0x3d0 and obj.x <= 0x3f0 and obj.y >= 0x1d0 and obj.y <= 0x1e7 then
            obj.obj_n = 215 --OBJ_POWER_CABLE1
         end
      end
   end

end

function stoker_blocked(stoker)
   if map_is_on_screen(stoker.x, stoker.y, stoker.z) then
      printl("STOKERS_PATH_IS_BLOCKED")
      play_md_sfx(0)
   end
end

function worktype_9C_stoker_return_to_conveyor_belt(actor)
   if map_get_obj(actor.x, actor.y+2, actor.z, 191) == nil then --OBJ_CONVEYOR_BELT2
      if actor_move(actor, DIR_SOUTH) == false then
         stoker_blocked(actor)
      end
   else
      actor.wt = 0x9D
   end
end

local worktype_tbl = {
   [0x15]=worktype_15_your_mother,
   [0x16]=worktype_16_minotaur,
   [0x99]=worktype_99_coker_move_to_coal_vein,
   [0x9a]=worktype_9A_coker_drop_coal,
   [0x9b]=worktype_9B_coker_wait_for_coal_to_move_away,
   [0x9c]=worktype_9C_stoker_return_to_conveyor_belt,
   [0x9d]=worktype_9D_stoker_wait_for_coal,
   [0x9e]=worktype_9E_stoker_walk_to_furnace,
}

function perform_worktype(actor)
   --print("wt="..actor.wt.."\n")
   local mpts = actor.mpts
   if worktype_tbl[actor.wt] ~= nil then
      local func = worktype_tbl[actor.wt]
      func(actor)
   end

   if mpts == actor.mpts then
      subtract_movement_pts(actor, 10)
   end
end
