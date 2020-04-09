ALIGNMENT_DEFAULT = 0
ALIGNMENT_NEUTRAL = 1
ALIGNMENT_EVIL    = 2
ALIGNMENT_GOOD    = 3
ALIGNMENT_CHAOTIC = 4

HEAD   = 0
NECK   = 1
BODY   = 2
ARM    = 3
ARM_2  = 4
HAND   = 5
HAND_2 = 6
FOOT   = 7


g_combat_range_tbl = {
0, 1, 2, 3, 4, 5, 6, 7,
1, 1, 2, 3, 4, 5, 6, 7,
2, 2, 2, 3, 4, 5, 6, 7,
3, 3, 3, 4, 5, 6, 7, 7,
4, 4, 4, 5, 6, 7, 7, 8,
5, 5, 5, 6, 7, 7, 8, 8,
6, 6, 6, 7, 7, 8, 8, 8,
7, 7, 7, 7, 8, 8, 8, 8}

function get_combat_range(absx, absy)
   if absx < 8 and absy < 8 then
      return g_combat_range_tbl[absx * 8 + absy + 1]
   end

   return 9
end

function get_weapon_range(obj_n)

   local range = g_range_weapon_tbl[obj_n]

   if range == nil then
      return 1
   end

   return range
end

function actor_randomise_stat(base_stat)
   local tmp = math.floor(base_stat/2)
   if tmp == 0 then
      return base_stat
   end

   return math.random(0, tmp) + math.random(0, tmp)  + base_stat - tmp
end

function actor_is_holding_obj(actor, obj_n)
   local hand

   hand = Actor.inv_get_readied_obj_n(actor, HAND)
   if hand == obj_n then
      return true
   end
   hand = Actor.inv_get_readied_obj_n(actor, HAND_2)
   if hand == obj_n then
      return true
   end

   return false
end

function actor_has_free_arm(actor)
   if Actor.inv_get_readied_obj_n(actor, ARM) == -1
           or Actor.inv_get_readied_obj_n(actor, ARM_2) == -1 then
      return true
   end

   return false
end

function actor_find_max_xy_distance(actor, x, y)
   x, y = abs(actor.x - x), abs(actor.y - y)
   return (x > y) and x or y
end

function actor_find_max_wrapped_xy_distance(actor, x, y)
   x, y = get_wrapped_dist(actor.x, x), get_wrapped_dist(actor.y, y)
   return (x > y) and x or y
end

function actor_get_combat_range(actor, target_x, target_y)
   --FIXME might need to adjust the src position for multi-tile actors.
   x, y = get_wrapped_dist(actor.x, target_x), get_wrapped_dist(actor.y, target_y)
   return get_combat_range(x, y)
end

function subtract_map_movement_pts(actor)
   local points = map_get_impedence(actor.x, actor.y, actor.z, false) + 5
   subtract_movement_pts(actor, points)
end

function actor_move_towards_loc(actor, map_x, map_y)
   --dgb("move actor "..actor.name.." from ("..actor.x..","..actor.y..") towards ("..map_x..","..map_y..") ")
   local var_2 = (word_30A6B == 1) and 0 or 1
   local var_6 = 1
   local diff_x = map_x - actor.x
   local diff_y = map_y - actor.y

   if (diff_x == 0 and diff_y == 0) or actor.wt == WT_STATIONARY then subtract_movement_pts(actor, 5) return 0 end

   local x_direction, y_direction

   if diff_x ~= 0 then
      x_direction = (diff_x >= 0) and DIR_EAST or DIR_WEST
   else
      x_direction = (math.random(0, 1) == 0) and DIR_WEST or DIR_EAST
   end

   if diff_y ~= 0 then
      y_direction = (diff_y >= 0) and DIR_SOUTH or DIR_NORTH
   else
      y_direction = (math.random(0, 1) == 0) and DIR_SOUTH or DIR_NORTH
   end

   unk_30A72 = 0

   local var_4

   if abs(diff_x) >= 4 or abs(diff_y) >= 4 then
      var_4 = (math.random(1, abs(diff_x) + abs(diff_y)) <= abs(diff_x)) and 1 or 0
   else
      if abs(diff_x) > abs(diff_y) then
         var_4 = 0
      else
         if abs(diff_x) < abs(diff_y) then
            var_4 = 1
         else
            var_4 = math.random(0, 1)
         end
      end

      --var_4 = (abs(diff_x) >= abs(diff_y) or abs(diff_x) ~= abs(diff_y) or math.random(0, 1) == 0) and 0 or 1
   end
   ----dgb("var_4 = "..var_4.."\n")
   if var_4 == 0 then
      if actor_move(actor, x_direction, var_2) == 0 then
         if actor_move_diagonal(actor, x_direction, y_direction) == 0 then
            if actor_move(actor, y_direction, var_2) == 0 then
               if math.random(0, 1) ~= 0 or actor_move(actor, (y_direction == DIR_NORTH) and DIR_SOUTH or DIR_NORTH, 1) == 0 then

                  subtract_map_movement_pts(actor)
                  var_6 = 0 --didn't move anywhere
               end
            end
         end
      end

   else

      if actor_move(actor, y_direction, var_2) == 0 then
         if actor_move_diagonal(actor, x_direction, y_direction) == 0 then
            if actor_move(actor, x_direction, var_2) == 0 then
               if math.random(0, 1) ~= 0 or actor_move(actor, (x_direction == DIR_EAST) and DIR_WEST or DIR_EAST, 1) == 0 then

                  subtract_map_movement_pts(actor)
                  var_6 = 0 --didn't move anywhere
               end
            end
         end
      end

   end

   unk_30A72 = 1
   ----dgb("var_6 = "..var_6)
   --dgb(" now at ("..actor.x..","..actor.y..") dir="..actor.direction.."\n")
   return var_6

end

function toss_actor(actor, from_x, from_y, from_z, arg_0)

   local random = math.random
   local player_loc = player_get_location()

   for i=1,8 do

      local new_x = random(1, 4) + random(1, 4) + from_x - 5
      local new_y = random(1, 4) + random(1, 4) + from_y - 5

      if arg_0 == 0
              or player_loc.x - 5 > new_x or player_loc.x + 5 < new_x or player_loc.y - 5 > new_y or player_loc.y + 5 < new_y then

         if Actor.move(actor, new_x, new_y, from_z) == true then return true end

      end
   end

   return false
end

function actor_has_bad_alignment(actor)
   return actor.wt == ALIGNMENT_EVIL or actor.wt == ALIGNMENT_CHAOTIC
end
