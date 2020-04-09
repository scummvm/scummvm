local USE_EVENT_USE = 0x01
local USE_EVENT_MOVE = 0x40
local USE_EVENT_READY = 0x0100

function search_container(obj)
	local child
	for child in container_objs(obj) do
		Obj.moveToMap(child, obj.x, obj.y, obj.z)
	end
end

function use_door(obj, actor)
	if bit32.btest(obj.quality, 0x80) then
	  printl("IT_IS_LOCKED")
	  return
	end

   if map_get_actor(obj.x, obj.y, obj.z) ~= nil then
      printl("BLOCKED")
      return
   end

   obj.frame_n = bit32.bxor(obj.frame_n, 2)

end

function use_crate(obj, target_obj, actor)
	if target_obj.frame_n == 2 then
		target_obj.frame_n = 1
		printl("YOU_PRIED_THE_NAILS_OUT_OF_THE_CRATE")
	else
		if target_obj.frame_n == 1 and obj.obj_n == 273 then
			target_obj.frame_n = 2
			printl("YOU_NAIL_THE_CRATE_SHUT")
		else
			printl("IT_HAS_NO_EFFECT")
		end
	end
end

function use_container(obj, actor)
	if obj.frame_n == 2 then
		if obj.obj_n == 86 or obj.obj_n == 284 then
			printl("IT_IS_NAILED_SHUT")
		else
			printl("IT_IS_LOCKED")
		end
	else
		if obj.frame_n == 1 then
			obj.frame_n = 0
			search_container(obj)
		else
			obj.frame_n = 1
		end
	end
end

function use_hammer_on_oxium_geode(obj, target_obj, actor)
   if target_obj.frame_n ~= 0 then
      printl("IT_HAS_NO_EFFECT")
      return
   end

   play_md_sfx(0x20)
   target_obj.frame_n = 1 --break the geode open

   if target_obj.on_map then
      local oxium = Obj.new(131) --OBJ_BLOB_OF_OXIUM
      oxium.qty = math.random(1, 50) + math.random(1, 50)
      Obj.moveToMap(oxium, target_obj.x, target_obj.y, target_obj.z)
   else
      --FIXME add to actor's inventory
      local oxium = Actor.inv_get_obj_n(actor, 131) --OBJ_BLOB_OF_OXIUM
      local qty = math.random(1, 50) + math.random(1, 50)
      if oxium ~= nil then
         oxium.qty = oxium.qty + qty
      else
         oxium = Obj.new(131, 0, 0, qty) --OBJ_BLOB_OF_OXIUM
         Actor.inv_add_obj(actor, oxium)
      end
   end

end

function use_prybar_on_hatch(obj, target_obj, actor)

	if actor.actor_num ~= 1 then
		printfl("IS_NOT_STRONG_ENOUGH", actor.name)
		return
	end

	local tesla = Actor.get(16)
	if Actor.get_talk_flag(tesla, 4) == false then
		Actor.set_talk_flag(tesla, 2)
		Actor.talk(tesla)
	else
		play_midgame_sequence(1)

		Actor.set_talk_flag(tesla, 5)
		target_obj.obj_n = 428
		target_obj.frame_n = 0;
		target_obj.x = target_obj.x + 1
		target_obj.y = target_obj.y + 1

		local blood = Actor.get(18)
		Actor.set_talk_flag(blood, 3)
		Actor.set_talk_flag(blood, 6)
		Actor.talk(blood)
	end
end

function use_sextant(obj, actor)
	if actor.z ~= 0 then
		printl("NOT_WHILE_UNDERGROUND")
		return
	end

	local lat_str = "N"
	local long_str = "W"

	local lat = math.modf(((actor.y - 512) * 240) / 1024)
	local long = math.modf(((actor.x - 512) * 360) / 1024)

	if lat > 0 then
		lat_str = "S"
	else
		if lat == 0 then
			lat_str = " "
		end
	end

	if long == 180 or long == -180 or long == 0 then
		long_str = " "
	else
		if long < 0 then
			long_str = "E"
		end
	end

	lat = math.abs(lat)
	long = 180 - math.abs(long)
	printl("YOU_ARE_SOMEWHERE_NEAR")
	print(" \nLat:" ..lat.." "..lat_str.."\nLong:"..long.." "..long_str.."\n")
end

function use_berry(obj, actor)
   local actor_num = actor.actor_num
   if actor_num == 6 then
      printl("A_MECHANICAL_PERSON_CANT_EAT_BERRIES")
      return
   end

   play_md_sfx(0x32)
   local berry_type = obj.obj_n - 73 --OBJ_BERRY
   local first_berry = true

   if (berry_type == 0 and actor_is_affected_by_purple_berries(actor_num))
      or (berry_type == 1 and actor_is_affected_by_green_berries(actor_num))
      or (berry_type == 2 and actor_is_affected_by_brown_berries(actor_num)) then
      printl("YOU_EAT_A_MARTIAN_BERRY_YOU_FEEL_AN_INCREASE_IN_THE_STRANGE")
   else
      printl("YOU_EAT_A_MARTIAN_BERRY_YOU_FEEL_A_STRANGE")
   end

   if berry_type == 0 then
      printl("RELATIONSHIP_WITH_THINGS_AROUND_YOU")
      actor_increment_purple_berry_count(actor_num)
   elseif berry_type == 1 then
      printl("SENSITIVITY_TO_THE_FEELINGS_OF_OBJECTS_AROUND_YOU")
      actor_increment_green_berry_count(actor_num)
   elseif berry_type == 2 then
      printl("SENSE_OF_YOUR_SPATIAL_LOCATION")
      actor_increment_brown_berry_count(actor_num)
   elseif berry_type == 3 then
      printl("SUDDEN_FLUSH_DIZZINESS_AND_NAUSEA")
      actor.poisoned = false
      Actor.hit(actor, math.random(5, 0x14) + math.random(5, 0x14))
      local counter = actor_get_blue_berry_counter()
      actor_set_blue_berry_counter(counter + math.random(1, 2))
   end

   if obj.qty == 1 then
      Obj.removeFromEngine(obj)
   else
      obj.qty = obj.qty - 1
   end
end

function use_misc_text(obj)

	local obj_n = obj.obj_n
	if obj_n == 65 or obj_n == 66 or obj_n == 67 or obj_n == 263 or obj_n == 267 or obj_n == 327 then
		printl("YOU_CANT_DIG_ANY_DEEPER")
	elseif obj_n == 427 then
		printl("IT_IS_STUCK")
	elseif obj_n == 288 then
		printl("THE_CONTROL_PANEL_OPERATES_THE_DREAM_MACHINE")
	elseif obj_n == 199 then
		printl("YOU_NEED_TO_USE_PLIERS_TO_ATTACH_THE_CABLE")
	elseif obj_n == 131 then
		printl("YOU_WILL_CHEW_IT_INSTINCTIVELY")
	elseif obj_n == 184 then
		printl("THE_PLATE_IN_THE_CAMERA_HAS_ALREADY_BEEN_EXPOSED")
	elseif obj_n == 442 or obj_n == 443 or obj_n == 444 then
		printl("YOU_MUST_USE_A_SHOVEL_TO_MOVE_THAT")
	elseif obj_n == 293 then
		printl("YOU_CANT_DETERMINE_HOW_TO_READ_THE_TIME")
	elseif obj_n == 77 or obj_n == 78 then
		printl("THE_BERRIES_ARE_NOT_EDIBLE")
	elseif obj_n == 323 then
		printl("YOU_DONT_KNOW_HOW_IT_WORKS")
	else
		printl("IT_HAS_NO_EFFECT")
	end
end

function get_pile_obj_num(map_tile)
	if (map_tile >= 32 and map_tile <= 36) or map_tile == 12 or map_tile == 13 or (map_tile >= 40 and map_tile <= 63) then
		return 258 --OBJ_DIRT_PILE
	elseif map_tile == 124 or map_tile == 125 or map_tile == 127 then
		return 256 --OBJ_CHUNK_OF_ICE
	end

	return 0 --CANNOT DIG HERE
end

function get_free_location_around_actor(actor)
	local x_tbl = {-1,0,1,1,1,0,-1,-1}
	local y_tbl = {-1,-1,-1,0,1,1,1,0}
	local pos = {}
	local i

	pos.z = actor.z

	for i=1,8 do
		pos.x = actor.x + x_tbl[i]
		pos.y = actor.y + y_tbl[i]
		if map_can_put(pos.x,pos.y,pos.z) == true and map_get_obj(pos.x,pos.y,pos.z) == nil then
			return pos
		end
	end

	return nil
end

function use_tool_on_ground(obj, target_obj, actor, target_x, target_y, target_z)
	if target_obj ~= nil then
		--check tile flag here.
		printl("THE_GROUND_IS_NOT_CLEAR_FOR_DIGGING")
		return
	end

	local map_tile = map_get_tile_num(target_x, target_y, target_z)

	local pile_obj_num = get_pile_obj_num(map_tile)

	if pile_obj_num == 0 then
		printl("IT_HAS_NO_EFFECT")
		return
	end

	local hole_obj_num = 257

	if pile_obj_num == 256 then
		hole_obj_num = 255
	end

	local hole = Obj.new(hole_obj_num)
	hole.temporary = true
	Obj.moveToMap(hole, target_x, target_y, target_z)

	local loc = get_free_location_around_actor(actor)

	if loc ~= nil then
		local pile = Obj.new(pile_obj_num)
		pile.temporary = true
		Obj.moveToMap(pile, loc.x, loc.y, loc.z)
	else
		Obj.removeFromEngine(hole)
		printl("YOU_CANT_DIG_HERE")
	end
end

function use_shovel_on_pile_to_hole(obj, target_obj, to_obj, actor)
	Obj.removeFromEngine(target_obj)
	Obj.removeFromEngine(to_obj)

	play_md_sfx(0x1b)
   play_md_sfx(0x1b)
	printl("YOU_FILLED_IN_THE_HOLE")
end

function use_shovel_on_ore_to_container(obj, target_obj, to_obj, actor)
   print("\n")
   local ore_quality = get_ore_container_quality(target_obj.obj_n)
   if to_obj.obj_n == 268 then --OBJ_MARTIAN_WHEEL_BARROW
      if to_obj.qty ~= 1 then
         play_md_sfx(0x1b)
         to_obj.qty = 1
         to_obj.quality = ore_quality
         printfl("YOU_PUT_THE_ORE_INTO_THE_WHEELBARROW", target_obj.name)
         Obj.removeFromEngine(target_obj)
      else
         printl("THERE_IS_NO_MORE_ROOM")
         play_md_sfx(5)
         return false
      end
   elseif to_obj.obj_n == 410 then --OBJ_RAIL_CAR
      local qty = to_obj.qty
      if to_obj.qty < 7 then
         if to_obj.qty > 0 and to_obj.quality ~= ore_quality then
            printl("YOU_CANT_MIX_DIFFERENT_THINGS_IN_THE_SAME_LOAD")
            play_md_sfx(5)
            return false
         else
            to_obj.quality = ore_quality
            to_obj.qty = to_obj.qty + 1
            if to_obj.qty == 1 or to_obj.qty == 7 then
               to_obj.frame_n = to_obj.frame_n + 2
            end
            printfl("YOU_PUT_THE_ORE_INTO_THE_RAIL_CAR", target_obj.name)
            Obj.removeFromEngine(target_obj)
         end
      else
         printl("THERE_IS_NO_MORE_ROOM")
         play_md_sfx(5)
         return false
      end
   end

   return true
end

function use_shovel_to_unload_container(obj, target_obj, to_obj, actor)
   if target_obj.qty == 0 then
      printl("THERE_IS_NOTHING_TO_UNLOAD")
      play_md_sfx(5)
      return
   end

   local ore = Obj.new(get_obj_num_from_ore_quality(target_obj.quality))

   --FIXME if to_obj == nil do something

   local success_flag = false
   if to_obj.obj_n == 268 or to_obj.obj_n == 410 then --OBJ_MARTIAN_WHEEL_BARROW
      success_flag = use_shovel_on_ore_to_container(obj, ore, to_obj, actor)
   elseif to_obj.obj_n == 233 then --OBJ_FURNACE
      success_flag = use_shovel_on_ore_to_furnace(obj, ore, to_obj, actor)
   elseif to_obj.obj_n == 257 then --OBJ_HOLE
      --FIXME need to wire up this logic.
   else
      --FIXME need to implement burying logic
   end

   if success_flag then
      target_obj.qty = target_obj.qty - 1
      if target_obj.obj_n == 410 then --OBJ_RAIL_CAR
         if target_obj.qty == 6 or target_obj.qty == 0 then
            target_obj.frame_n = target_obj.frame_n - 2
         end
      end
   end
end

function use_shovel_on_ore_to_furnace(obj, target_obj, to_obj, actor)
   local obj_n = target_obj.obj_n

   play_md_sfx(0x1b)

   Obj.removeFromEngine(target_obj)
   if obj_n == 444 then --OBJ_PILE_OF_COAL
      if to_obj.frame_n < 4 then
         activate_power_system()
      end
   else
      printl("IT_HAS_NO_EFFECT")
   end

   return true
end

function activate_power_system()
   printl("THE_COAL_BEGINS_MOVING_DOWN_THE_BELT")
   Actor.set_talk_flag(0x73, 2)
   Actor.set_talk_flag(0x71, 3)
   activate_power_update_tiles()
   update_conveyor_belt(false)
end

function activate_power_update_tiles()
   anim_play(9)
   anim_play(10)
   anim_play(11)
   anim_play(12)
   local loc = player_get_location()
   for obj in find_obj(loc.z, 208) do --OBJ_CRACK
      if obj ~= nil then
         if map_get_obj(obj.x, obj.y, obj.z, 209) == nil then
            local steam = Obj.new(209, 1)
            Obj.moveToMap(steam, obj.x, obj.y+1, obj.z)
         end
      end
   end

   for obj in find_obj(loc.z, 233) do --OBJ_FURNACE
      if obj ~= nil then
         local frame_n = obj.frame_n
         if frame_n == 1 or frame_n == 3 then
            obj.frame_n = frame_n + 4
         end
      end
   end

   for obj in find_obj(loc.z, 154) do --OBJ_LAMP
      if obj ~= nil then
         obj.frame_n = 1
      end
   end
   for obj in find_obj(4, 154) do --OBJ_LAMP
      if obj ~= nil then
         obj.frame_n = 1
      end
   end
end

function shutdown_power_update_tiles()
   anim_stop(9)
   anim_stop(10)
   anim_stop(11)
   anim_stop(12)
   local loc = player_get_location()
   for obj in find_obj(loc.z, 209) do --OBJ_STEAM_LEAK
      if obj ~= nil then
         Obj.removeFromEngine(obj)
      end
   end

   for obj in find_obj(loc.z, 233) do --OBJ_FURNACE
      if obj ~= nil then
         local frame_n = obj.frame_n
         if frame_n == 5 or frame_n == 7 then
            obj.frame_n = frame_n - 4
         end
      end
   end

   for obj in find_obj(loc.z, 154) do --OBJ_LAMP
      if obj ~= nil then
         obj.frame_n = 0
      end
   end
   for obj in find_obj(4, 154) do --OBJ_LAMP
      if obj ~= nil then
         obj.frame_n = 0
      end
   end
end

function update_conveyor_belt(can_stop)
   if Actor.get_talk_flag(0x73, 2) == false then
      return
   end
   local player_loc = player_get_location()

   if player_loc.z ~= 5 then
      return
   end

   --NOTE. The original game used generalised logic to search for conveyors.
   -- I haven't used that here as there is only one conveyor in the game.

   local x = 0x3c
   local y = 0x63
   local z = 5

   local conveyor = map_get_obj(x, y, z, 188) --OBJ_CONVEYOR_BELT

   while conveyor ~= nil do
      if conveyor.frame_n == 2 then
         local seam = map_get_obj(x, y, z, 189) --OBJ_CONVEYOR_BELT1
         if seam ~= nil then
            Obj.removeFromEngine(seam)
         end
      elseif conveyor.frame_n == 0 then
         if conveyor.qty == 0 then
            local seam = Obj.new(189)
            Obj.moveToMap(seam, x, y, z)
            conveyor.qty = 4
         end
         conveyor.qty = conveyor.qty - 1
      end

      local seam = map_get_obj(x, y, z, 189) --OBJ_CONVEYOR_BELT1
      if seam ~= nil then
         seam.x = seam.x + 1
      end

      for obj in objs_at_loc(x, y, z) do
         if obj.obj_n == 447 or obj.weight > 0 then --OBJ_HUGE_LUMP_OF_COAL
            obj.x = obj.x + 1
         end
      end

      local actor = map_get_actor(x, y, z)
      if actor ~= nil then
         Actor.move(actor, x+1, y, z)
      end

      x = x - 1
      conveyor = map_get_obj(x, y, z, 188) --OBJ_CONVEYOR_BELT
   end


   if can_stop and Actor.get_talk_flag(0x71, 3) then
      if math.random(0, 6) == 0 then
         printl("THE_CONVEYOR_BELT_STOPS")
         Actor.clear_talk_flag(0x73, 2)
         Actor.clear_talk_flag(0x71, 3)
         shutdown_power_update_tiles()
      end
   end


end

function midgame_cutscene_2()
   play_midgame_sequence(2)

   for tower in find_obj(0, 201) do --OBJ_TOWER_TOP
      if tower.x >= 0x3d0 and tower.x <= 0x3f0 and tower.y >= 0x1d0 and tower.y <= 0x1e7 then
         tower.frame_n = 4 + (tower.frame_n % 4)
      end
   end

   for cable in find_obj(0, 214) do --OBJ_POWER_CABLE
      if cable.x >= 0x3d0 and cable.x <= 0x3f0 and cable.y >= 0x1d0 and cable.y <= 0x1e7 then
         cable.obj_n = 215
      end
   end
end

function use_fixed_belt_on_bare_rollers(obj, target_obj, actor)
   local start_obj = nil
   local rollers = target_obj
   while rollers ~= nil do
      if rollers.frame_n == 0 then
         start_obj = rollers
         break
      end
      rollers = map_get_obj(rollers.x-1,rollers.y,rollers.z, rollers.obj_n)
   end

   if start_obj == nil then
      printl("OOOPS_THESE_ROLLERS_CAN_NEVER_BE_FIXED")
      return
   end

   rollers = start_obj
   local i = 4
   while rollers ~= nil do
      rollers.obj_n = 188
      if i == 0 then
         i = 4
         local belt_join_obj = Obj.new(189)
         Obj.moveToMap(belt_join_obj, rollers.x, rollers.y, rollers.z)
      else
         i = i - 1
      end
      rollers = map_get_obj(rollers.x+1,rollers.y,rollers.z, 192) --OBJ_BARE_ROLLERS
   end

   Obj.removeFromEngine(obj)
   Actor.set_talk_flag(0x72, 2)
end

function use_ruby_slippers(obj, actor)
   if obj.readied == false then
      --FIXME check that we can ready this object.
      Obj.removeFromEngine(obj)
      Actor.inv_ready_obj(actor, obj)
      return
   end

   if obj.quality == 2 then
      printl("YOU_MAY_USE_THE_RUBY_SLIPPERS_TO_GO_HOME")
      local input = input_select("yn", false)
      if input == "Y" then
         play_end_sequence()
      else
         play_midgame_sequence(13)
      end
   else
      printl("CLICK")
      obj.quality = obj.quality + 1
   end
end

function foes_are_nearby()
   local loc = player_get_location()
   local i
   for i=1,0xff do
      local actor = Actor.get(i)
      if actor.alive and (actor.align == ALIGNMENT_EVIL or actor.align == ALIGNMENT_CHAOTIC) and actor.z == loc.z then
         if get_wrapped_dist(actor.x, loc.x) < 40 and get_wrapped_dist(actor.y, loc.y) < 40 then
            return true
         end
      end
   end

   return false
end

function is_target_visible_to_actor(actor, target)
   --FIXME
   return true
end

function npcs_are_nearby()
   local loc = player_get_location()
   local i
   local player = Actor.get_player_actor()
   for i=1,0xff do
      local actor = Actor.get(i)
      if actor.alive and actor.asleep == false and actor.in_party == false and actor.z == loc.z then
         if get_wrapped_dist(actor.x, loc.x) < 6 and get_wrapped_dist(actor.y, loc.y) < 6 and is_target_visible_to_actor(player, actor) then
            return actor
         end
      end
   end

   return nil
end

function is_actor_able_to_talk_to_player(actor)
   local player = Actor.get_player_actor()
   local loc = player_get_location()
   if actor.alive and actor.asleep == false and actor.z == loc.z then
      if get_wrapped_dist(actor.x, loc.x) < 6 and get_wrapped_dist(actor.y, loc.y) < 6 and is_target_visible_to_actor(player, actor) then
         return true
      end
   end

   return false
end

function rest_heal_party(hours_to_rest)
   local actor
   for actor in party_members() do
      local hp = math.random(3,8) + math.random(3,8) * math.floor(hours_to_rest / 2)
      local max_hp = actor_get_max_hp(actor)
      if actor.hp + hp > max_hp then
         actor.hp = max_hp
      else
         actor.hp = actor.hp + hp
      end
   end
end

function rest_level_up_actor(actor)
   if actor.actor_num > 15 then
      return
   end

   local exp_level_tbl = {
   [0] = 0,
   [1] = 100,
   [2] = 200,
   [3] = 400,
   [4] = 800,
   [5] = 1600,
   [6] = 3200,
   [7] = 6400,
   [8] = 32767,
}

   if actor.exp <= exp_level_tbl[actor.level] then
      return
   end

   actor.level = actor.level + 1

   local max_hp = actor_get_max_hp(actor)
   if actor.hp + 30 > max_hp then
      actor.hp = max_hp
   else
      actor.hp = actor.hp + 30
   end

   Actor.show_portrait(actor)

   local obj_n = actor.obj_n
   local gender = math.random(0,1)
   if obj_n == 342 or obj_n == 343 or obj_n == 345 or (obj_n >= 347 and obj_n <= 353) then
      gender = 0
   elseif obj_n == 344 or obj_n == 346 or (obj_n >= 354 and obj_n <= 357) then
      gender = 1
   end

   local gender_pronoun = "He"
   if gender == 1 then
      gender_pronoun = "She"
   end

   printfl("HAS_A_DREAM", actor.name)
   printfl("SEES_THREE_STONE_OBELISKS", gender_pronoun)
   printfl("FEELS_DRAWN_TO_ONE_OF_THE_OBELISKS", gender_pronoun)
   printfl("DOES_TOUCH_THE_OBELISK", actor.name)
   printl("WHICH_BHS")

   local answer = input_select("bhs", false)

   if answer == "B" then
      if actor.int < 30 then
         actor.int = actor.int + 1
      end
   elseif answer == "H" then
      if actor.dex < 30 then
         actor.dex = actor.dex + 1
      end
   elseif answer == "S" then
      if actor.str < 30 then
         actor.str = actor.str + 1
      end
   end

end

function use_tent(obj, actor)
   if player_is_in_solo_mode() then
      printl("NOT_WHILE_IN_SOLO_MODE")
      play_md_sfx(5)
      return
   end

   if g_in_dream_mode then
     printl("YOU_CANT_SLEEP_IN_A_DREAM")
     play_md_sfx(5)
     return
  end

   local tent_loc = {}

   if obj.on_map then
      tent_loc.x = obj.x
      tent_loc.y = obj.y
      tent_loc.z = obj.z
   else
      tent_loc = player_get_location()
   end

   local x, y

   for y = tent_loc.y - 2, tent_loc.y do
      for x = tent_loc.x - 1, tent_loc.x + 1 do
         local map_obj = map_get_obj(x,y,tent_loc.z)
         if map_obj ~= nil and map_obj.obj_n ~= 106 then
            if tile_get_flag(map_obj.tile_num, 3, 4) == false then
               printfl("TENT_OBJ_IN_THE_WAY", map_obj.name)
               play_md_sfx(5)
               return
            end
         end
      end
   end

   for y = tent_loc.y - 2, tent_loc.y do
      for x = tent_loc.x - 1, tent_loc.x + 1 do
         if tile_get_flag(map_get_tile_num(x,y,tent_loc.z), 1, 1) == true then --if map tile is impassible
            printl("THE_GROUND_IS_NOT_FLAT_ENOUGH")
            play_md_sfx(5)
            return
         end
      end
   end

   printl("REST")

   if party_is_in_combat_mode() then
      print(" - ")
      printl("NOT_WHILE_IN_COMBAT_MODE")
      play_md_sfx(5)
      return
   end

   if foes_are_nearby() then
      printl("NOT_WHILE_FOES_ARE_NEAR")
      play_md_sfx(5)
      return
   end

   local npc = npcs_are_nearby()
   if npc ~= nil then
      printfl("IS_TOO_NEAR_TO_SETUP_CAMP", npc.name)
      play_md_sfx(5)
      return
   end

   --poison check

   local actor
   local poisoned = false
   for actor in party_members() do
      if actor.poisoned then
         poisoned = true
         printfl("IS_POISONED", actor.name)
      end
   end

   if poisoned then
      printl("DO_YOU_REALLY_WANT_TO_SLEEP")
      local answer = input_select("yn", false)
      if answer == "N" or answer == "n" then
         return
      end
   end

   local party_is_using_berries = false
   for actor in party_members() do
      local actor_num = actor.actor_num
      local green = actor_is_affected_by_green_berries(actor_num)
      local brown = actor_is_affected_by_brown_berries(actor_num)

      if brown or green then
         party_is_using_berries = true
         if brown and green then
            printfl("COMPLAINS_OF_TOO_MUCH_LIGHT_AND_INANIMATE", actor.name)
         elseif brown then
            printfl("COMPLAINS_OF_TOO_MUCH_LIGHT", actor.name)
         else --green
            printfl("COMPLAINS_OF_INANIMATE_THINGS_TALKING", actor.name)
         end
      end

   end

   if party_is_using_berries then
      if party_get_size() == 1 then
         printl("YOU_CANT_SLEEP")
      else
         printl("NOBODY_CAN_SLEEP")
      end

      return
   end

   local player = Actor.get_player_actor()
   player.x = tent_loc.x
   player.y = tent_loc.y

   local tent = Obj.new(134, 3)
   Obj.moveToMap(tent, player.x, player.y-1, player.z)

   tent = Obj.new(134, 5)
   Obj.moveToMap(tent, player.x+1, player.y-1, player.z)

   tent = Obj.new(134, 6)
   Obj.moveToMap(tent, player.x-1, player.y, player.z)

   tent = Obj.new(134, 9)
   Obj.moveToMap(tent, player.x+1, player.y, player.z)

   tent = Obj.new(134, 8)
   Obj.moveToMap(tent, player.x, player.y, player.z)

   party_move(player.x, player.y, player.z)

   script_wait(500)

   party_hide_all()

   tent.frame_n = 7

   local hour = clock_get_hour()
   local time
   local hours_to_rest

   if hour < 7 or hour > 16 then
      time = i18n("SUNRISE")
      if hour < 7 then
         hours_to_rest = 7 - hour
      else
         hours_to_rest = 24 - hour + 7
      end
   elseif hour <= 16 then
      time = i18n("SUNSET")
      hours_to_rest = 18 - hour
   end

   printfl("REST_UNTIL", time)
   local answer = input_select("yn", false)

   if answer == "N" or answer == "n" then
      printl("HOW_MANY_HOURS")
      hours_to_rest = input_select_integer("0123456789", true)
   end

   g_party_is_warm = true

   if g_hours_till_next_healing == 0 and hours_to_rest > 4 then
      rest_heal_party(hours_to_rest)
      g_hours_till_next_healing = 6
   end

   local can_level_up = false
   if hours_to_rest * 3 > party_get_size() then
      can_level_up = true
   end

   local i
   for i=0,hours_to_rest*3-1 do
      advance_time(20)
      script_wait(100)
      if i < party_get_size() then
         local actor = party_get_member(i)
         rest_level_up_actor(actor)
      end
   end

   local actor
   local poisoned = false
   for actor in party_members() do
      if actor.poisoned then
         if math.random(1, 8) + math.random(1, 8) >= 15 then
            actor.poisoned = false
            printfl("FEELS_BETTER", actor.name)
         else
            if actor.hp < hours_to_rest * 2 + 5 then
               actor.hp = 5
            else
               actor.hp = actor.hp - (hours_to_rest * 2 + 5)
            end
         end
      end
   end


   tent.frame_n = 8 --Open the tent flap
   party_show_all()

   party_move(player.x, player.y + 1, player.z)

   script_wait(500)

   --remove tent from map
   local z = player.z
   for tent in find_obj(z, 134) do
      if tent ~= nil and
      ((tent.x == tent_loc.x and tent.y == tent_loc.y-1) or
       (tent.x == wrap_coord(tent_loc.x+1,z) and tent.y == tent_loc.y-1) or
       (tent.x == wrap_coord(tent_loc.x-1,z) and tent.y == tent_loc.y) or
       (tent.x == wrap_coord(tent_loc.x+1,z) and tent.y == tent_loc.y) or
       (tent.x == tent_loc.x and tent.y == tent_loc.y))
      then
         Obj.removeFromEngine(tent)
      end
   end

   g_party_is_warm = false
end

function use_red_berry(obj, actor)
   if not party_is_in_combat_mode() then
      printl("THAT_WOULD_BE_A_WASTE_OUTSIDE_OF_COMBAT")
      return
   end

   if actor.frenzy == false then
      printfl("ENTERS_A_BATTLE_FRENZY", actor.name)
      play_md_sfx(0x32)
   end

   actor.frenzy = true

   local qty = obj.qty

   if qty > 1 then
      obj.qty = qty - 1
   else
      Obj.removeFromEngine(obj)
   end
end

function use_spittoon(obj, actor)
   if not actor.hypoxia then
      printl("YOU_SPIT_INTO_THE_SPITTOON")
   else
      printl("YOUR_MOUTH_IS_TOO_DRY")
   end
end

function use_gong(obj, target_obj, actor)
   printl("GONG")
   play_md_sfx(0xf)
end

function use_musical_instrument(obj, actor)

   local obj_n = obj.obj_n

   if obj_n == 280 then --OBJ_CYMBALS
      printl("CHING")
      play_md_sfx(0x36)
   elseif obj_n == 281 then --OBJ_TAMBORINE
      printl("SHING")
      play_md_sfx(0x35)
   elseif obj_n == 282 then --OBJ_DRUM
      printl("THUMP_THUMP")
      play_md_sfx(0x34)
      script_wait(100)
      play_md_sfx(0x34)
   elseif obj_n == 283 then --OBJ_ACCORDION
      printl("WHEEEEZE")
      play_md_sfx(0x37)
   else
      printfl("YOU_PLAY_THE", obj.name)
   end
end

function use_wrench_on_switchbar(obj, target_obj, actor)
   if target_obj.quality == 1 then
      if target_obj.on_map then
         local turntable = map_get_obj(target_obj.x-1, target_obj.y-1, target_obj.z, 413)
         if turntable ~= nil then
            target_obj.frame_n = turntable.frame_n
            target_obj.quality = 0
            printl("THE_SWITCH_IS_FASTENED")
            play_md_sfx(0x1f)
            return
         end
      end
      printl("THIS_SWITCH_CANNOT_BE_FIXED")
      play_md_sfx(0x5)
   else
      printl("THE_SWITCH_IS_LOOSE")
      play_md_sfx(0x1f)
      target_obj.quality = 1
   end
end

function use_wrench_on_drill(obj, target_obj, actor)
   local drill_cart
   if target_obj.on_map then
      drill_cart = map_get_obj(target_obj.x, target_obj.y, target_obj.z, 439)
   end

   if drill_cart ~= nil then
      local drill = Obj.new(441,1) --assembled drill
      Obj.moveToMap(drill, target_obj.x, target_obj.y, target_obj.z)
      Obj.removeFromEngine(target_obj)
      Obj.removeFromEngine(drill_cart)
      printl("THE_DRILLS_POWER_IS_CONNECTED")
      play_md_sfx(0x1f)
   else
      printl("THE_DRILL_MUST_BE_INSTALLED_ONTO_A_DRILL_CART")
   end
end

function use_wrench_on_panel(obj, target_obj, actor)
   if target_obj.on_map == false then
      printl("IT_HAS_NO_EFFECT")
      return
   end

   local quality = target_obj.quality
   local panel_qty = target_obj.qty
   if quality == 0 then
      target_obj.quality = 1
      printl("THE_PANEL_IS_LOOSE")
      play_md_sfx(0x1f)
   elseif bit32.btest(quality, 2) then
      printl("IT_MUST_BE_REPAIRED_FIRST")
      play_md_sfx(0x5)
   else
      local cabinet = map_get_obj(target_obj.x, target_obj.y, target_obj.z, 457)
      if cabinet == nil then
         printl("PANELS_ARE_ONLY_INSTALLED_ONTO_CABINETS")
         play_md_sfx(0x5)
      else
         quality = cabinet.quality
         if (quality == 0 and panel_qty ~= 0) or
            (quality ~= 0 and quality <= 3 and panel_qty == 0 and target_obj.frame_n ~= (quality - 1) ) or
            (quality ~= 0 and (quality > 3 or panel_qty ~= 0) and quality ~= panel_qty) then
            printl("THIS_CABINET_REQUIRES_A_DIFFERENT_TYPE_OF_PANEL")
            play_md_sfx(0x5)
         else
            target_obj.quality = target_obj.quality - 1
            printl("THE_PANEL_IS_FASTENED")
            play_md_sfx(0x1f)
            if target_obj.quality == 3 then
               Actor.set_talk_flag(0x74, 3)
               if Actor.get_talk_flag(0x74, 0) then
                  Actor.set_talk_flag(0x60, 2)
               end
            end
         end
      end
   end

end

function use_oxium_bin(obj, actor)
   if map_can_reach_point (actor.x, actor.y, obj.x, obj.y, obj.z) == false then
      printl("BLOCKED")
      return
   end

   local oxium = Obj.new(131) --OBJ_BLOB_OF_OXIUM
   oxium.qty = 20

   if Actor.can_carry_obj(actor, oxium) then
      Actor.inv_add_obj(actor, oxium, STACK_OBJECT_QTY)
      printl("YOU_GET_TWO_HANDFULS_OF_OXIUM_FROM_THE_BIN")
      Actor.set_talk_flag(0x12, 4)
   else
      printl("YOU_ARE_CARRYING_TOO_MUCH_ALREADY")
   end
end

function use_sledge_hammer_on_replacement_track_to_broken_track(obj, target_obj, to_obj, actor)
   play_md_sfx(0x20)
   printl("WHICH_SECTION_OF_RAIL_NEEDS_FIXING")
   Obj.removeFromEngine(target_obj)
   to_obj.obj_n = 414 --OBJ_TRACK
end

function use_pliers_on_spool_to_tower(obj, target_obj, to_obj, actor)

    if actor_find_max_wrapped_xy_distance(actor, to_obj.x, to_obj.y) > 1 then
      printl("THE_WORK_IS_TO_PRECISE_TO_PERFORM_TELEKINETICALLY")
      return
   end

   if Actor.get_talk_flag(0x73, 2) == false or Actor.get_talk_flag(0x71, 3) == true then
      printl("THE_CABLE_DOES_NOT_NEED_REPLACEMENT")
      return
   end

   if actor_is_holding_obj(actor, 38) == false then --OBJ_RUBBER_GLOVES
      Actor.hit(actor, math.random(0, math.floor(actor.max_hp/2)))
      local spector = Actor.get(2)
      if is_actor_able_to_talk_to_player(spector) then
         Actor.set_talk_flag(spector, 2)
         Actor.talk(spector)
      end
   else
      play_md_sfx(0x1f)
      Obj.removeFromEngine(target_obj)
      Actor.set_talk_flag(0x73, 4)
      play_midgame_sequence(3)
      for obj in find_obj_from_area(0x3d0, 0x1d0, 0, 32, 17) do
         if obj.obj_n == 215 then -- OBJ_POWER_CABLE1
            obj.obj_n = 214 -- OBJ_POWER_CABLE
         end
      end

      for obj in find_obj(0, 315) do --OBJ_CHAMBER1
         Obj.removeFromEngine(obj)
      end
   end

end

function use_gate(obj, actor)
   if bit32.btest(obj.quality, 128) == true then
      printl("IT_IS_APPARENTLY_LOCKED")
      return
   end

   local frame_n = obj.frame_n
   if frame_n == 0 or frame_n == 1 then
      obj.frame_n = 3
      obj.x = obj.x - 1
   elseif frame_n == 2 or frame_n == 3 then
      obj.frame_n = 1
      obj.x = obj.x + 1
   elseif frame_n == 4 or frame_n == 5 then
      obj.frame_n = 7
   elseif frame_n == 6 or frame_n == 7 then
      obj.frame_n = 5
   end


end

function use_switch_bar(obj, actor)
   if obj.frame_n == 0 then
      obj.frame_n = 1
   else
      obj.frame_n = 0
   end

   if obj.on_map == false or map_get_obj(obj.x-1,obj.y-1,obj.z, 413) == nil then
      printl("IT_HAS_NO_EFFECT")
      return
   end

   if obj.quality == 1 then
      printl("IT_TURNS_LOOSELY")
      return
   end

   local turntable = map_get_obj(obj.x-1,obj.y-1,obj.z, 413)
   turntable.frame_n = obj.frame_n

   local railcar = map_get_obj(obj.x-1,obj.y-1,obj.z, 410)
   if railcar ~= nil then
      railcar.frame_n = railcar.frame_n - (railcar.frame_n % 2)
      railcar.frame_n = railcar.frame_n + turntable.frame_n
   end
end

function use_reading_material(obj, actor)
   if obj.quality == 0 then
      local signatures = ""
      if Actor.get_talk_flag(5, 3) == true then
         signatures = signatures .. " Richard Sherman"
      end
      signatures = signatures .. "\n"

      if Actor.get_talk_flag(0x19, 3) == true then
         signatures = signatures .. "  Capt. Gregory Duprey"
      end
      signatures = signatures .. "\n"

      if Actor.get_talk_flag(0x1a, 3) == true then
         signatures = signatures .. "  Doctor David Yellin"
      end
      signatures = signatures .. "\n"

      display_text_in_scroll_gump(i18nf("AFFIDAVIT", player_get_name(), signatures))
   else
      local text = load_text_from_lzc("mdtext.lzc", obj.quality - 1)
      if text ~= nil then
         display_text_in_scroll_gump(text)
      else
         printl("YOU_CANT_READ_IT")
      end
   end

end

function use_pool_table(obj, actor)
   if Actor.inv_get_readied_obj_n(actor, ARM) ~= 401 then --OBJ_POOL_CUE
      printl("YOU_NEED_A_POOL_CUE")
      return
   end
   local pool_table1 = map_get_obj(wrap_coord(obj.x - 1, obj.z), obj.y, obj.z, 400) -- OBJ_UNK_400
   local rand = math.random
   if pool_table1 == nil then
      pool_table1 = Obj.new(400, rand(0, 6))
      Obj.moveToMap(pool_table1, wrap_coord(obj.x - 1, obj.z), obj.y, obj.z)
   end

   local pool_table2 = map_get_obj(obj.x, obj.y, obj.z, 400) -- OBJ_UNK_400
   if pool_table2 == nil then
      pool_table2 = Obj.new(400, rand(0, 6))
      Obj.moveToMap(pool_table2, wrap_coord(obj.x - 1, obj.z), obj.y, obj.z)
   end

   for i=1,10 do
      if i~= 1 then
         script_wait(rand(200,500))
      end
      play_md_sfx(0x1c + rand(0, 2))
      script_wait(rand(10,200))
      play_md_sfx(0x1c + rand(0, 2))

      pool_table1.frame_n = rand(0, 6)
      pool_table2.frame_n = rand(0, 6)

      if rand(0, 40) >= actor_dex_adj(actor) then
         break
      end
   end

   if rand(0, 40) < actor_dex_adj(actor) then
      printl("GOOD_SHOT_OLD_BEAN")
   end
end

function use_ready_obj(obj, actor)
   if not Actor.can_carry_obj(actor, obj) then
      printl("YOU_ARE_CARRYING_TOO_MUCH_ALREADY")
      return
   end
   if obj.readied == true then
      return
   end

   Obj.removeFromEngine(obj)
   Obj.moveToInv(obj, actor.actor_num)

   Actor.inv_ready_obj(actor, obj)
end

function use_heartstone_on_metal_woman(obj, target_obj, actor)
   if target_obj.quality == 1 then
      printl("THE_ROBOT_ALREADY_HAS_A_HEARTSTONE")
   else
      target_obj.quality = 1
      Obj.removeFromEngine(obj)
      play_md_sfx(0x1f)
      printl("THE_HEARTSTONE_IS_INSTALLED")
   end

end

function use_manuscript_on_mailbox(obj, target_obj, actor)
   local twain = Actor.get(0x57)

   finish_dream_quest(twain)
   Actor.set_talk_flag(twain, 6)
   Actor.talk(twain)
   wake_from_dream()
end

function use_assembled_drill(obj, actor)

   play_md_sfx(0x10)

   local x = obj.x
   local y = obj.y
   local z = obj.z

   if obj.frame_n == 1 then
      x = x - 1
   elseif obj.frame_n == 3 then
      y = y - 1
   elseif obj.frame_n == 4 then
      y = y + 1
   else
      x = x + 1
   end

   local target_obj
   for obj in objs_at_loc(x, y, z) do
      if obj.obj_n == 445 --OBJ_IRON_ORE
         or obj.obj_n == 446 -- OBJ_VEIN_OF_COAL
         or obj.obj_n == 213 then --OBJ_CAVE_IN
         target_obj = obj
         break
      end
   end

   if target_obj == nil then
      target_obj = map_get_obj(x, y, z, 213, true)
   end

   local drilled_matterial

   if target_obj == nil then
      if can_drill_at_loc(x, y, z) == true then
         drilled_matterial = 442 --OBJ_PILE_OF_ROCKS
      else
         local target_actor = map_get_actor(x, y, z)
         if target_actor ~= nil then
            --FIXME attack actor here. 40 points of damage
         else
            printl("THERE_IS_NOTHING_TO_DRILL_INTO")
         end
         return
      end
   elseif target_obj.obj_n == 445 then --OBJ_IRON_ORE
      drilled_matterial = 443 --OBJ_PILE_OF_IRON_ORE
   elseif target_obj.obj_n == 446 then --OBJ_VEIN_OF_COAL
      drilled_matterial = 444 --OBJ_PILE_OF_COAL
   end

   if drilled_matterial ~= nil then
      local spoil_location = get_free_location_around_drill(obj)
      if spoil_location ~= nil then
         local spoil_obj = Obj.new(drilled_matterial)
         Obj.moveToMap(spoil_obj, spoil_location)
      else
         printl("THERE_IS_NO_ROOM_LEFT_FOR_THE_ORE")
      end
   end

   if target_obj then
      if target_obj.quality > 1 then
         target_obj.quality = target_obj.quality - 1
      else
         Obj.removeFromEngine(target_obj)
      end
   end

   if drilled_matterial == nil then
      Obj.removeFromEngine(target_obj)
   end

end

function get_free_location_around_drill(drill)
   local x_tbl = {-1,0,1,1,1,0,-1,-1}
   local y_tbl = {-1,-1,-1,0,1,1,1,0}
   local pos = {}
   local i

   pos.z = drill.z

   for i=1,8 do
      pos.x = drill.x + x_tbl[i]
      pos.y = drill.y + y_tbl[i]
      local obj = map_get_obj(pos.x, pos.y, pos.z)
      if obj == nil or is_blood(obj.obj_num) then
         if tile_get_flag(map_get_tile_num(pos), 1, 1) == false then --not blocked.
            return pos
         end
      end
   end

   return nil
end

function get_ore_container_quality(ore_obj_num)
   local tbl = {[258]=1,[442]=2,[443]=3,[444]=4} --OBJ_DIRT_PILE, OBJ_PILE_OF_ROCKS, OBJ_PILE_OF_IRON_ORE, OBJ_PILE_OF_COAL
   local quality = tbl[ore_obj_num]
   if quality == nil then
      quality = 1
   end

   return quality
end

function get_obj_num_from_ore_quality(ore_quality)
   if ore_quality == 2 then
      return 442 --OBJ_PILE_OF_ROCKS
   elseif ore_quality == 3 then
      return 443 --OBJ_PILE_OF_IRON_ORE
   elseif ore_quality == 4 then
      return 444 --OBJ_PILE_OF_COAL
   end

   return 258 --OBJ_DIRT_PILE
end

function can_drill_at_loc(x,y,z)
   local tile_num = map_get_tile_num(x, y, z)

   if tile_num >= 0xf0 and tile_num <= 0xfb then
      return true
   end

   return false
end

local use_shovel_on_tbl = {
--on
   [255]=use_misc_text,
   [257]=use_misc_text,
   [258]={[257]=use_shovel_on_pile_to_hole},
   [268]={
      [233]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_MARTIAN_WHEEL_BARROW to OBJ_FURNACE
      [257]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_MARTIAN_WHEEL_BARROW to OBJ_HOLE
      [268]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_MARTIAN_WHEEL_BARROW to OBJ_MARTIAN_WHEEL_BARROW
      [410]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_MARTIAN_WHEEL_BARROW to OBJ_RAIL_CAR
   },
   [410]={
      [233]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_RAIL_CAR to OBJ_FURNACE
      [257]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_RAIL_CAR to OBJ_HOLE
      [268]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_RAIL_CAR to OBJ_MARTIAN_WHEEL_BARROW
      [410]=use_shovel_to_unload_container, --use OBJ_SHOVEL on OBJ_RAIL_CAR to OBJ_RAIL_CAR
   },
   [442]={
      [268]=use_shovel_on_ore_to_container, --use OBJ_SHOVEL on OBJ_PILE_OF_ROCKS to OBJ_MARTIAN_WHEEL_BARROW
      [410]=use_shovel_on_ore_to_container}, --use OBJ_SHOVEL on OBJ_PILE_OF_ROCKS to OBJ_RAIL_CAR
   [443]={
      [268]=use_shovel_on_ore_to_container, --use OBJ_SHOVEL on OBJ_PILE_OF_IRON_ORE to OBJ_MARTIAN_WHEEL_BARROW
      [410]=use_shovel_on_ore_to_container}, --use OBJ_SHOVEL on OBJ_PILE_OF_IRON_ORE to OBJ_RAIL_CAR
   [444]={
      [268]=use_shovel_on_ore_to_container, --use OBJ_SHOVEL on OBJ_PILE_OF_COAL to OBJ_MARTIAN_WHEEL_BARROW
      [410]=use_shovel_on_ore_to_container}, --use OBJ_SHOVEL on OBJ_PILE_OF_COAL to OBJ_RAIL_CAR
   [0]=use_tool_on_ground, --hole in ice, hole
}

function use_oil_on_door(obj, target_obj, actor)
   if obj.obj_n == 235 and obj.frame_n ~= 4 then
      printl("IT_HAS_NO_EFFECT")
      return
   end

   if target_obj ~= nil then
      if bit32.band(target_obj.quality, 0x80) == 0 then
         printl("THIS_DOOR_IS_NOT_RUSTED")
      else
         target_obj.quality = bit32.band(target_obj.quality, 0x7f) --unset bit 7
         printl("THE_DOOR_IS_UNSTUCK")
         play_md_sfx(4)
         if obj.stackable then
            if obj.qty == 1 then
               Obj.removeFromEngine(obj)
            else
               obj.qty = obj.qty - 1
            end
         else
            Obj.removeFromEngine(obj)
         end
      end
   end
end

function use_oil_on_dream_door(obj, target_obj, actor, target_x, target_y, target_z)
   if map_get_tile_num(target_x, target_y, target_z) == 8 then
      local melies = Actor.get(0x51)
      Actor.set_talk_flag(melies, 6)
      Actor.talk(melies)
      finish_dream_quest(melies)
      wake_from_dream()
   else
      printl("IT_HAS_NO_EFFECT")
   end
end

function use_hand_mirror(obj, actor)
   if actor.x >= 0x76 and actor.x <= 0x78 and actor.y >= 0xca and actor.y <= 0xcc and actor.z == 2 then
      local lowell = Actor.get(0x50)

      Actor.set_talk_flag(lowell, 6)
      Actor.talk(lowell)
      finish_dream_quest(lowell)
      wake_from_dream()
   else
      printl("YOU_SEE_YOURSELF")
   end

end

function use_radium(obj, target_obj, actor)
   actor_radiation_check(actor, obj)
   if obj.obj_n == 448 then --OBJ_BLOCK_OF_RADIUM
      if target_obj.obj_n == 448 and target_obj.on_map then
         local power_unit = map_get_obj(obj.x, obj.y, obj.z, 290) --OBJ_POWER_UNIT
         if power_unit ~= nil then
            target_obj = power_unit
         end
      end

      if target_obj.obj_n == 290 then --OBJ_POWER_UNIT
         if target_obj.frame_n == 1 then
            printl("RADIUM_HAS_ALREADY_BEEN_INSTALLED")
         else
            Obj.removeFromEngine(obj)
            play_md_sfx(4)
            printl("THE_RADIUM_HAS_BEEN_INSTALLED")
            target_obj.frame_n = 1
            Actor.set_talk_flag(0x74, 2)
            if Actor.get_talk_flag(0x74, 0) then
               Actor.set_talk_flag(0x60, 2)
            end
         end

      else
         printl("IT_HAS_NO_EFFECT")
      end

   elseif obj.obj_n == 449 then --OBJ_CHIP_OF_RADIUM
      if target_obj.qty < 0xf0 then
         local qty = input_select_obj_qty(obj)
         if qty == 0 then
            return
         end

         if target_obj.qty + qty * 30 > 0xf0 then
            qty = math.ceil((0xf0 - target_obj.qty) / 30)
            printfl("THE_OBJ_ONLY_NEEDED_N_RADIUM_BLOCKS", target_obj.name, qty)
            printfl("THE_OBJ_IS_FULLY_CHARGED", target_obj.name)
            target_obj.qty = 0xf0
         else
            target_obj.qty = target_obj.qty + qty * 30
         end


         if obj.qty == qty then
            Obj.removeFromEngine(obj)
         else
            obj.qty = obj.qty - qty
         end
      else
         printfl("THE_OBJ_IS_FULLY_CHARGED", target_obj.name)
      end

   end

end

function use_head_gear(obj, target_obj, actor)
   local machine
   if target_obj.obj_n == 289 then --OBJ_DREAM_MACHINE2
      local machine = map_get_obj(target_obj.x, target_obj.y, target_obj.z, 288)
      if machine == nil then
         printl("IT_HAS_NO_EFFECT")
         return
      end
   else
      machine = target_obj
   end

   local loc = machine.xyz

   machine = Obj.new(100)
   machine.frame_n = 1
   Obj.moveToMap(machine, loc)

   printl("THE_HEADGEAR_IS_INSTALLED")

   Actor.set_talk_flag(0x74, 4)
   if Actor.get_talk_flag(0x74, 0) then
      Actor.set_talk_flag(0x60, 2)
   end

end

function use_dream_machine(panel_obj)
   if Actor.get_talk_flag(0x46, 3) then
      printl("THE_DREAM_MACHINES_SEEM_TO_HAVE_CEASED_FUNCTIONING")
      return
   end

   local z = panel_obj.z
   local power_unit
   local dream_quality
   local headgear_installed = false
   local seat_x, seat_y
   for obj in find_obj_from_area(panel_obj.x - 5, panel_obj.y - 5, z, 11, 11) do
      local obj_n = obj.obj_n
      if obj_n == 290 and obj.frame_n == 1 then --OBJ_POWER_UNIT
         power_unit = obj
      elseif obj_n == 100 then --OBJ_DREAM_MACHINE
         headgear_installed = true
      elseif obj_n == 289 then --OBJ_DREAM_MACHINE2
         seat_x = obj.x
         seat_y = obj.y
      elseif obj_n == 288 then --OBJ_DREAM_MACHINE2
         dream_quality = obj.quality
      end
   end

   local actor_in_seat = map_get_actor(seat_x, seat_y, z)

   if power_unit == nil or dream_quality == nil or not headgear_installed then
      printl("THE_MACHINE_DOES_NOT_WORK")
      return
   end

   local martian_obj = map_get_obj(seat_x, seat_y, z, 291) --OBJ_UNCONSCIOUS_MARTIAN
   if martian_obj ~= nil then
      if Actor.get_talk_flag(0x21, 2) or not Actor.get_talk_flag(0x61, 4) then
         printl("IT_HAS_NO_EFFECT")
      else
         play_midgame_sequence(5)
         local martian = Actor.get(0x21)
         Actor.talk(martian)
         Actor.set_talk_flag(martian, 2)
         martian.x = seat_x
         martian.y = seat_y + 1
         martian.z = panel_obj.z
         kill_actor(martian)
         Obj.removeFromEngine(martian_obj)
      end

      return
   end

   local metal_woman_obj = map_get_obj(seat_x, seat_y, z, 287) --OBJ_METAL_WOMAN
   if metal_woman_obj ~= nil then
      if metal_woman_obj.quality == 0 then
         printl("IT_HAS_NO_EFFECT")
      else
         --FIXME implement metal woman activation logic
      end
   elseif actor_in_seat ~= nil then
      if z ~= 0 then
         wake_from_dream()
      else
         if dream_quality < 2 then
            Actor.set_talk_flag(0x60, 3)
         end
         if dream_quality == 3 then
            if Actor.get_talk_flag(0x46, 0) then
               --FIXME call sub_3F624
            else
               printl("THE_MACHINE_DOES_NOT_WORK")
            end
         else
            actor_use_dream_machine(actor_in_seat, dream_quality)
         end

      end
   else
      printl("THERE_IS_NOBODY_SITTING_IN_THE_MACHINE")
   end

end

function use_sprayer_system(panel_obj)
end

function use_lens_controls(panel_obj)
end

function use_pump_controls(panel_obj)
end

local use_panel_tbl = {
   [0]=function() printl("YOU_ARE_COMPLETELY_UNSURE_WHAT_YOU_JUST_DID") end,
   [1]=function() printl("YOU_ACTUATE_THE_MECHANISM_TO_NO_APPARENT_EFFECT") end,
   [2]=function() printl("STATUS_LIGHTS_CHANGE_BUT_YOU_SEE_NO_OTHER_EFFECT") end,
   [3]=function() printl("LIGHTS_FLASH_AND_CHANGE_COLOR_BUT_NOTHING_ELSE_HAPPENS") end,
   [4]=use_dream_machine,
   [5]=use_sprayer_system,
   [6]=use_lens_controls,
   [7]=use_pump_controls,
}

function use_panel(obj, actor)
   if bit32.band(obj.quality, 2) ~= 0 then
      printl("THE_PANEL_IS_BROKEN")
      return
   end
   if bit32.band(obj.quality, 1) ~= 0 then
      printl("THE_PANEL_IS_NOT_INSTALLED")
      return
   end

   local cabinet = map_get_obj(obj.x, obj.y, obj.z, 457) --OBJ_CABINET
   if cabinet == nil then
      printl("PANELS_ARE_ONLY_INSTALLED_ONTO_CABINETS")
      return
   end

   local quality = cabinet.quality
   if use_panel_tbl[quality] ~= nil then
      use_panel_tbl[quality](obj)
   else
      printl("IT_HAS_NO_EFFECT")
   end
end

function has_minotaur_left_the_shop(cur_z)
   for i=1,0xff do
      local actor = Actor.get(i)
      if actor.obj_n == 398 then --OBJ_MINOTAUR
         if actor.z == cur_z and not Actor.get_talk_flag(0x54, 6)
                 and actor.x >= 0x37 and actor.y <= 0x37 then
            return true
         end
      end
   end
   return false
end

function use_switch(obj, actor)
   local switch_qty = obj.qty
   local switch_quality = obj.quality
   local target_obj

   local num_switches = 0
   local switches = {}

   for obj in find_obj_from_area(obj.x - 32, obj.y - 32, obj.z, 64, 64) do
      if obj.obj_n == 179 or obj.obj_n == 227 then --OBJ_CLOSED_DOOR, OBJ_DOOR3
         if bit32.band(obj.quality, 0x7f) == switch_qty then
            target_obj = obj
         end
      elseif (obj.obj_n == 311 or obj.obj_n == 312 or obj.obj_n == 196) then --and obj.quality < 2 then
         if num_switches < 4 then
            num_switches = num_switches + 1
            switches[num_switches] = obj
         end
      end
   end

   if target_obj == nil then
      printl("STRANGELY_NOTHING_HAPPENS")
      return
   end

   if num_switches == 0 then
      printl("STRANGELY_IT_DOESNT_MOVE")
      return
   end

   local frame_n = 0
   if switch_quality == 1 then
      if bit32.band(target_obj.quality, 0x80) == 0 then
         target_obj.quality = bit32.bor(target_obj.quality, 0x80)
      else
         target_obj.quality = bit32.band(target_obj.quality, 0x7f)
         frame_n = 1
      end
   else
      local old_frame_n = target_obj.frame_n
      target_obj.frame_n = bit32.band(old_frame_n, 2) + 5
      play_door_sfx()
      if old_frame_n < 4 then
         target_obj.frame_n = bit32.band(old_frame_n, 2) + 9
         frame_n = 1
      else
         target_obj.frame_n = bit32.band(old_frame_n, 2) + 1

         if g_in_dream_mode and g_current_dream_stage == 0x44 and has_minotaur_left_the_shop(target_obj.z) then
            complete_tiffany_stage()
            return
         end
      end
      play_door_sfx()
   end

   for i=1,num_switches do
      switches[i].frame_n = frame_n
   end
   play_md_sfx(0x11)
end

function use_drawbridge_lever(obj, actor)
   --FIXME
end

function use_cheat_lever(obj, actor)
   --FIXME
end

function use_cheat_lever2(obj, actor)
   --FIXME
end

local switch_qual_tbl = {
   [1] = use_switch,
   [2] = use_switch,
   [10] = use_drawbridge_lever,
   [20] = use_cheat_lever,
   [21] = use_cheat_lever2,
}

function use_switch_device(obj, actor)
   if switch_qual_tbl[obj.quality] ~= nil then
      switch_qual_tbl[obj.quality](obj, actor)
   else
      printl("WHAT_AN_ODD_LEVER")
   end
end

function use_dreamstuff(obj, actor)
   printl("YOU_IMAGINE")

   for item in container_objs(obj) do
      if item.obj_n < 342 then -- OBJ_POOR_MONK
         --object
         print(item.look_string.."\n")
         if obj.on_map then
            Obj.moveToMap(item, obj.xyz)
         else
            local parent = obj.parent
            if parent.luatype == "actor" then
               Obj.moveToInv(item, parent.actor_num)
            else
               Obj.moveToCont(item, parent)
            end
         end
      else
         --actor
         print(item.name.."\n")
         local spawned_actor = Actor.new(item.obj_n, actor.x, actor.y, actor.z)

         actor_init(spawned_actor, obj.quality + 1) -- alignment
         toss_actor(spawned_actor, actor.x, actor.y, actor.z)
         spawned_actor.wt = item.quality
      end
      Obj.removeFromEngine(obj)
      return
   end

   printl("NOTHING")
end

function use_obj_on_spray_gun(obj, target_obj, actor)
   if obj.obj_n == 119 and target_obj.quality == 0 then --OBJ_BOTTLE_OF_GREEN_PAINT
      target_obj.quality = 1
      target_obj.qty = 20
      printl("SPRAY_GUN_GREEN_PAINT")
   elseif obj.obj_n == 128 and target_obj.quality ~= 0 then --OBJ_WEED_KILLER
      target_obj.quality = 0
      target_obj.qty = 10
      printl("SPRAY_GUN_WEED_KILLER")
   else
      local spray_gun_qty = target_obj.qty + 10
      if spray_gun_qty > 0x1e then
         spray_gun_qty = 0x1e
      end
      target_obj.qty = spray_gun_qty
      printl("SPRAY_GUN_10_MORE_CHARGES")
   end

   Obj.removeFromEngine(obj)
end

function use_spray_gun(obj, target_obj, actor)
   if obj.qty == 0 then
      printl("THERE_IS_NOTHING_IN_THE_GUN")
      return
   end

   if obj.quality ~= 0 then
      obj.qty = obj.qty - 1
      if target_obj.luatype == "actor" and target_obj.obj_n == 145 then --OBJ_MONSTER_FOOTPRINTS
         target_obj.obj_n = 364 --OBJ_PROTO_MARTIAN
         printfl("BECOMES_VISIBLE", target_obj.name)
      else
         if target_obj.luatype == "actor" and not target_obj.visible then
            target_obj.visible = true
            printfl("BECOMES_VISIBLE", target_obj.name)
         elseif target_obj.luatype == "obj" and target_obj.invisible then
            target_obj.invisible = false
            printfl("BECOMES_VISIBLE", target_obj.name)
         else
            printl("IT_HAS_NO_EFFECT")
         end
      end
   elseif target_obj.luatype == "actor" then
      attack_target_with_weapon(actor, target_obj.x, target_obj.y, obj)
   elseif is_plant_obj(target_obj) then
      hit_target(target_obj, RED_HIT_TILE)
      printl("YOU_KILLED_THE_PLANT")
      if target_obj.obj_n == 205 then --OBJ_VINE
         Actor.set_talk_flag(0x30, 7)
         Obj.removeFromEngine(target_obj)
      elseif target_obj.obj_n == 408 then --OBJ_TREE
         target_obj.obj_n = 166 --OBJ_DEAD_TREE
      else
         Obj.removeFromEngine(target_obj)
      end
   else
      obj.qty = obj.qty - 1
      printl("IT_HAS_NO_EFFECT")
   end
end

local usecode_table = {
--OBJ_RUBY_SLIPPERS
[12]=use_ruby_slippers,
--OBJ_RUBBER_GLOVES
[38]=use_ready_obj,
--OBJ_SLEDGE_HAMMER
[52]={
--on
   [404]={ --OBJ_REPLACEMENT_TRACK
   --to
      [405]=use_sledge_hammer_on_replacement_track_to_broken_track
   }
},
--OBJ_PLIERS
[53]={
--on
   [199]={--OBJ_CABLE_SPOOL
   --to
      [201]=use_pliers_on_spool_to_tower, --OBJ_TOWER_TOP
      [215]=use_pliers_on_spool_to_tower, --OBJ_POWER_CABLE1
   }
},
--OBJ_PICK
[65]={[255]=use_misc_text,[257]=use_misc_text}, --hole in ice, hole
--OBJ_SHOVEL
[66]=use_shovel_on_tbl,
--OBJ_HOE
[67]={[255]=use_misc_text,[257]=use_misc_text}, --hole in ice, hole
--OBJ_BERRY
[73]=use_berry,
--OBJ_BERRY1
[74]=use_berry,
--OBJ_BERRY2
[75]=use_berry,
--OBJ_BERRY3
[76]=use_berry,
--OBJ_BERRY4
[77]=use_red_berry,
--OBJ_CLUMP_OF_ROUGE_BERRIES
[78]=use_misc_text,
[86]=use_container,
[87]=use_container,
--OBJ_MANUSCRIPT
[88]={
   --on
   [84]=use_manuscript_on_mailbox, --OBJ_MAILBOX
},
[96]=use_sextant,
[102]={[86]=use_crate,[427]=use_prybar_on_hatch},
[104]=use_container,
--OBJ_FOLDED_TENT
[106]=use_tent,
--OBJ_BOTTLE_OF_GREEN_PAINT
[119]={
   --on
   --OBJ_WEED_SPRAYER
   [129]=use_obj_on_spray_gun,
   --OBJ_SPRAY_GUN
   [261]=use_obj_on_spray_gun,
},
--OBJ_CAN_OF_LAMP_OIL
[124]={
   --on
   --OBJ_CLOSED_DOOR
   [179]=use_oil_on_door,
   --OBJ_DOOR3
   [227]=use_oil_on_door,
   --OBJ_CLOSED_HATCH
   [421]=use_oil_on_door,
   --OBJ_DOOR
   [152]=use_oil_on_door,
   --OBJ_DOOR1
   [219]=use_oil_on_door,
   --OBJ_DOOR2
   [222]=use_oil_on_door,
   [0]=use_oil_on_dream_door,
},
--OBJ_WEED_KILLER
[128]={
   --on
   --OBJ_WEED_SPRAYER
   [129]=use_obj_on_spray_gun,
   --OBJ_SPRAY_GUN
   [261]=use_obj_on_spray_gun,
},
--OBJ_WEED_SPRAYER
[129]={
   --on
   [0]=use_spray_gun
},
--OBJ_BLOB_OF_OXIUM
[131]=use_misc_text,
--OBJ_WRENCH
[135]={
--on
   [411]=use_wrench_on_switchbar,
   [440]=use_wrench_on_drill,
   [458]=use_wrench_on_panel
},
--OBJ_TONGS
[136]=use_ready_obj,
--OBJ_REPAIRED_BELT
[144]={
--on
   [192]=use_fixed_belt_on_bare_rollers,
},
--OBJ_HAND_MIRROR
[147]=use_hand_mirror,
--OBJ_BOOK
[148]=use_reading_material,
--OBJ_NOTE
[151]=use_reading_material,
--OBJ_DOOR
[152]=use_door,
[181]=use_gate,
--OBJ_CAMERA
[184]=use_misc_text,
--OBJ_LEVER
[196]=use_switch_device,
--OBJ_CABLE_SPOOL
[199]=use_misc_text,
[212]=use_oxium_bin,
[222]=use_door,
--OBJ_GONG_HAMMER
[231]={
--on
   [130]=use_hammer_on_oxium_geode, --OBJ_OXIUM_GEODE
   [298]=use_gong, --OBJ_GONG
   --FIXME OBJ_BRASS_CHEST, OBJ_OBSIDIAN_BOX, OBJ_STEAMER_TRUNK, OBJ_OPEN_BRASS_TRUNK use_crate
   },
--OBJ_POTASH (this is actually oil when frame_n == 4)
[235]={
--on
   --OBJ_CLOSED_DOOR
   [179]=use_oil_on_door,
   --OBJ_DOOR3
   [227]=use_oil_on_door,
   --OBJ_CLOSED_HATCH
   [421]=use_oil_on_door,
   --OBJ_DOOR
   [152]=use_oil_on_door,
   --OBJ_DOOR1
   [219]=use_oil_on_door,
   --OBJ_DOOR2
   [222]=use_oil_on_door,
   [0]=use_oil_on_dream_door,
},
--OBJ_SCROLL
[243]=use_reading_material,
--OBJ_SPRAY_GUN
[261]={
   --on
   [0]=use_spray_gun
},
--OBJ_MARTIAN_HOE
[263]={[255]=use_misc_text,[257]=use_misc_text}, --hole in ice, hole
--OBJ_MARTIAN_SHOVEL
[267]=use_shovel_on_tbl,
[273]={[86]=use_crate}, --Hammer needs more codes
--OBJ_CYMBALS
[280]=use_musical_instrument,
--OBJ_TAMBORINE
[281]=use_musical_instrument,
--OBJ_DRUM
[282]=use_musical_instrument,
--OBJ_ACCORDION
[283]=use_musical_instrument,
[284]=use_container,
--OBJ_SPITTOON
[286]=use_spittoon,
--OBJ_DREAM_MACHINE1
[288]=use_misc_text,
--OBJ_MARTIAN_CLOCK
[293]=use_misc_text,
--OBJ_HEADGEAR
[296]={
   --on
   [288]=use_head_gear, --OBJ_DREAM_MACHINE1
   [289]=use_head_gear, --OBJ_DREAM_MACHINE2
},
--OBJ_SWITCH
[311]=use_switch_device,
--OBJ_SWITCH1
[312]=use_switch_device,
--OBJ_OXYGENATED_AIR_MACHINE
[323]=use_misc_text,
--OBJ_MARTIAN_PICK
[327]={[255]=use_misc_text,[257]=use_misc_text}, --hole in ice, hole
[331]=use_dreamstuff,
[399]=use_pool_table,
--OBJ_POOL_QUE
[401]=use_ready_obj,
[411]=use_switch_bar,
--OBJ_CLOSED_HATCH
[421]=use_door,
--OBJ_HEART_STONE
[426]={
   --on
   [287]= use_heartstone_on_metal_woman, --OBJ_METAL_WOMAN
},
[427]=use_misc_text,
--OBJ_ASSEMBLED_DRILL
[441]=use_assembled_drill,
--OBJ_PILE_OF_ROCKS
[442]=use_misc_text,
--OBJ_PILE_OF_IRON_ORE
[443]=use_misc_text,
--OBJ_PILE_OF_COAL
[444]=use_misc_text,
--OBJ_BLOCK_OF_RADIUM
[448]={
   --on
   [290]=use_radium, --OBJ_POWER_UNIT
   [448]=use_radium, --OBJ_BLOCK_OF_RADIUM
},
--OBJ_CHIP_OF_RADIUM
[449]={
   --on
   [240]=use_radium, --OBJ_HEAT_RAY_GUN
   [241]=use_radium, --OBJ_FREEZE_RAY_GUN
},
--OBJ_PANEL
[458]=use_panel,
}

function ready_winged_shoes(obj, actor)
   local player_loc = player_get_location()
   if player_loc.z == 2 then
      local bridge = Obj.new(146)
      bridge.temporary = false
      Obj.moveToMap(bridge, 0xc9, 0x9b, 2)
   end

   return true
end

function ready_tongs(obj, actor)
   printl("THE_TONGS_WILL_NOW_PROTECT_YOUR_HANDS")
   return true
end

function ready_throw_rug(obj, actor)
   obj.obj_n = 162 --Change to red cape
   return true
end

local usecode_ready_obj_table = {
[15]=ready_winged_shoes,
[136]=ready_tongs,
[161]=ready_throw_rug,
}

function unready_winged_shoes(obj, actor)
   printl("THEY_WONT_COME_OFF")
   return false
end

local usecode_unready_obj_table = {
[15]=unready_winged_shoes,
}

function move_drill(obj, rel_x, rel_y)
  if rel_x ~= 0 and rel_y ~= 0 then
    return false
  end

  if rel_x < 0 then
    obj.frame_n = 1
  elseif rel_x > 0 then
      obj.frame_n = 4
  elseif rel_y < 0 then
      obj.frame_n = 3
  elseif rel_y > 0 then
      obj.frame_n = 5
  end

  return true
end

function move_wheelbarrow(obj, rel_x, rel_y)
  if rel_x ~= 0 and rel_y ~= 0 then
    return false
  end

  if rel_x < 0 then
    obj.frame_n = 3
  elseif rel_x > 0 then
      obj.frame_n = 1
  elseif rel_y < 0 then
      obj.frame_n = 0
  elseif rel_y > 0 then
      obj.frame_n = 2
  end

  return true
end

function move_rail_cart(obj, rel_x, rel_y)
   local frame_n = obj.frame_n
   if rel_x ~= 0 and rel_y ~= 0 then
      printl("IT_WONT_GO_IN_THAT_DIRECTION")
      return false
   end
   if (rel_x < 0 or rel_x > 0) and (frame_n == 1 or frame_n == 3 or frame_n == 5) then
      printl("IT_WONT_GO_IN_THAT_DIRECTION")
      return false
   end
   if (rel_y < 0 or rel_y > 0) and (frame_n == 0 or frame_n == 2 or frame_n == 4) then
      printl("IT_WONT_GO_IN_THAT_DIRECTION")
      return false
   end
   if check_for_track(obj, rel_x, rel_y) == false then
      printl("IT_WONT_GO_IN_THAT_DIRECTION")
      return false
   end

   move_car_obj(obj, rel_x, rel_y)
   return false
end

function check_for_track(car, rel_x, rel_y)
   local x = car.x + rel_x
   local y = car.y + rel_y

   for obj in objs_at_loc(x, y, car.z) do
   	if (obj.obj_n >= 412 and obj.obj_n <= 414) or obj.obj_n == 419 or obj.obj_n == 175 or obj.obj_n == 163 then --track object

   	  local track_frame_n = obj.frame_n
   	  if (car.frame_n % 2) == track_frame_n or track_frame_n == 2 or obj.obj_n == 412 or obj.obj_n == 175 or obj.obj_n == 163 then
   	     return true
   	  else
   	     return false
   	  end
   	elseif is_blood(obj.obj_n) == false and obj.obj_n ~= 417 then --MARTIAN_BARGE
   	  return false
   	end
   end

   if map_get_obj(x, y, car.z, 175, true) then
      return true
   end

   if map_get_obj(x, y, car.z, 412, true) then
      return true
   end

   local tile_num = map_get_tile_num(x,y, car.z)
   if is_track_tile(tile_num) then
      return true
   end

   return false
end

function move_car_obj(obj, rel_x, rel_y)
   obj.x = obj.x + rel_x
   obj.y = obj.y + rel_y
   return true
end

function move_plank(obj, rel_x, rel_y)
   obj.x = obj.x + rel_x
   obj.y = obj.y + rel_y
   return true
end

function is_track_tile(tile_num)
   if tile_num == 108 or tile_num == 109 or tile_num == 110 or tile_num == 77 or tile_num == 79 then
      return true
   end
   return false
end

local usecode_move_obj_table = {
[268]=move_wheelbarrow,
[395]=move_plank,
[410]=move_rail_cart,
[441]=move_drill,
}

function has_usecode(obj, usecode_type)
      --print("has_usecode("..obj.obj_n..", "..usecode_type..")\n")
   if usecode_type == USE_EVENT_USE and usecode_table[obj.obj_n] ~= nil then
      return true
   elseif usecode_type == USE_EVENT_READY and (usecode_ready_obj_table[obj.obj_n] ~= nil or usecode_unready_obj_table[obj.obj_n] ~= nil)then
      return true
   elseif usecode_type == USE_EVENT_MOVE and usecode_move_obj_table[obj.obj_n] ~= nil then
      return true
   end

   return false
end

function use_obj_on_to(obj, target_obj, actor, use_to_tbl)
	local dir = get_direction(i18n("TO"))
	if dir == nil then
		printl("NOTHING")
		return
	end

	local to_x, to_y = direction_get_loc(dir, actor.x, actor.y)

	local to_obj = map_get_obj(to_x, to_y, actor.z)

	if to_obj ~= nil then
		print(to_obj.name.."\n\n")
		local func = use_to_tbl[to_obj.obj_n]
		if func ~= nil then
			func(obj, target_obj, to_obj, actor)
		end
	else
		printl("SOMETHING")
	end
end

function use_obj_on(obj, actor, use_on_tbl)
	local dir = get_direction(i18n("ON"))
	if dir == nil then
      printl("NOTHING")
		return
	end

	local target_x, target_y = direction_get_loc(dir, actor.x, actor.y)

   local target_entity = map_get_actor(target_x, target_y, actor.z)
   if target_entity == nil then
      target_entity = map_get_obj(target_x, target_y, actor.z)
   end

	if target_entity ~= nil then
		print(target_entity.name.."\n\n")
		local on = use_on_tbl[target_entity.obj_n]
		if on ~= nil then
			if type(on) == "function" then
				local func = on
				func(obj, target_entity, actor)
			else
				use_obj_on_to(obj, target_entity, actor, on)
			end
		else
			local func = use_on_tbl[0]
			if func ~= nil then
				func(obj, target_entity, actor)
			else
				printl("NO_EFFECT")
			end
		end
	else
		local func = use_on_tbl[0]
		if func ~= nil then
			func(obj, nil, actor, target_x, target_y, actor.z)
		else
			printl("NOTHING")
		end
	end
end

function can_interact_with_obj(actor, obj)
    if obj.on_map then
        --FIXME get_combat_range()
        local distance = actor_find_max_wrapped_xy_distance(actor, obj.x, obj.y)
        if actor_is_affected_by_purple_berries(actor.actor_num) and distance <= actor.level then
            return true
        end

        if distance > 1 then
            printl("OUT_OF_RANGE")
            return false
        end
    end

    return true
end

function use_obj(obj, actor)
    if not can_interact_with_obj(actor, obj) then
        return
    end

	if type(usecode_table[obj.obj_n]) == "function" then
		local func = usecode_table[obj.obj_n]
		if func ~= nil then
			print("\n")
			func(obj, actor)
		end
	else
		use_obj_on(obj, actor, usecode_table[obj.obj_n])
   end

   update_objects_around_party()
end

function ready_obj(obj, actor)
   if type(usecode_ready_obj_table[obj.obj_n]) == "function" and obj.readied == false then
      local func = usecode_ready_obj_table[obj.obj_n]
      if func ~= nil then
         print("\n")
         return func(obj, actor)
      end
   end

   if type(usecode_unready_obj_table[obj.obj_n]) == "function" and obj.readied == true then
      local func = usecode_unready_obj_table[obj.obj_n]
      if func ~= nil then
         print("\n")
         return func(obj, actor)
      end
   end

   return true
end

function move_obj(obj, rel_x, rel_y)
  if usecode_move_obj_table[obj.obj_n] ~= nil then
    return usecode_move_obj_table[obj.obj_n](obj, rel_x, rel_y)
  end
	return true
end

function is_ranged_select(operation)
    return actor_is_affected_by_purple_berries(Actor.get_player_actor().actor_num)
end
