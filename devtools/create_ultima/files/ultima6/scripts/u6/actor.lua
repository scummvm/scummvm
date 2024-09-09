--io.stderr:write("actor.lua get here\n")


--Worktypes
WT_NOTHING                = 0x0  --do nothing

WT_FOLLOW                 = 0x1  --follow avatar (in party)

WT_PLAYER                 = 0x2  --player mode

WT_FRONT                  = 0x3  --combat front
WT_REAR                   = 0x4  --combat rear
WT_FLANK                  = 0x5  --combat flank
WT_BERSERK                = 0x6  --combat berserk
WT_RETREAT                = 0x7  --combat retreat
WT_ASSAULT                = 0x8  --combat assault
WT_FLEE                   = 0x9  --run away from party
WT_LIKE                   = 0xa  --like the party. eg dog
WT_UNFRIENDLY             = 0xb  --wander around but attack if any party member gets too close.
WT_WANDER_NEAR_PLAYER     = 0xc  --guards use this.
WT_TANGLE                 = 0xd  --tangle vine
WT_STATIONARY             = 0xe  --stationary attack
WT_GUARD_WALK_EAST_WEST   = 0xf
WT_GUARD_WALK_NORTH_SOUTH = 0x10
--11
WT_GUARD_ARREST_PLAYER    = 0x12
WT_UNK_13                 = 0x13 --FIXME I think this is repel undead

WT_WALK_TO_LOCATION       = 0x86
WT_FACE_NORTH             = 0x87
WT_FACE_EAST              = 0x88
WT_FACE_SOUTH             = 0x89
WT_FACE_WEST              = 0x8a
WT_WALK_NORTH_SOUTH       = 0x8b
WT_WALK_EAST_WEST         = 0x8c
WT_WALK_SOUTH_NORTH       = 0x8d
WT_WALK_WEST_EAST         = 0x8e
WT_WANDER_AROUND          = 0x8f
WT_WORK                   = 0x90
WT_SLEEP                  = 0x91


WT_UNK_94                 = 0x94
WT_PLAY_LUTE              = 0x95
WT_BEG                    = 0x96

WT_BELL_RINGER            = 0x98
WT_BRAWLING               = 0x99
WT_MOUSE                  = 0x9a
WT_ATTACK_PARTY           = 0x9b


wt_rear_max_monster = nil
wt_rear_min_monster = nil
wt_rear_max_party = nil
wt_rear_min_party = nil

combat_avg_x, combat_avg_y, party_avg_x, party_avg_y = -1, -1, -1, -1

wt_front_target_actor = nil

wt_num_monsters_near = 0

g_time_stopped = false

g_update_volcano = false;

--Actor stats table
--[objnum] = {str,dex,int,hp,dmg,alignment,can talk,drops blood,?,?,?,lives in water,flies-immune to tremor,repel undead,poisonous,strength_based,double dmg from fire,immune to magic (fire only),immune to poison,undead and immune to death spells,immune to sleep spell,{spell table},{weapon table},{armor table},{treasure table},exp_related see actor_hit()}
actor_tbl = {
--Acid Slug
[364] = {5, 5, 2, 10, 1, ALIGNMENT_CHAOTIC, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, {}, {}, {}, {}, 0},
--Alligator
[429] = {20, 10, 3, 30, 10, ALIGNMENT_CHAOTIC, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, {}, {}, {}, {}, 6},
--Ant, Giant
[427] = {16, 15, 2, 10, 8, ALIGNMENT_CHAOTIC, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, {}, {}, {}, {}, 3},
--Avatar
[410] = {25, 25, 25, 30, 4, ALIGNMENT_GOOD, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Bat, Giant
[344] = {5, 25, 4, 5, 6, ALIGNMENT_CHAOTIC, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Peasant
[384] = {8, 8, 9, 6, 3, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {38,34}, {185}, {}, 0},
--Bird
[356] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
----Child
[381] = {8, 8, 9, 6, 2, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Corpser
[357] = {17, 6, 3, 40, 15, ALIGNMENT_CHAOTIC, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, {}, {}, {}, {}, 0},
--Cow
[428] = {14, 7, 5, 10, 3, ALIGNMENT_NEUTRAL, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {}, {}, {129}, 0},
--Cyclops
[424] = {27, 8, 7, 40, 20, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {44,33,34}, {20,3,13}, {98}, 4},
--Daemon
[367] = {35, 26, 31, 100, 20, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, {17,128,34,80}, {}, {}, {}, 10},
--Deer
[350] = {11, 18, 5, 8, 2, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {129}, 0},
--Dog
[349] = {5, 16, 8, 6, 3, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Dragon
[411] = {40, 35, 27, 150, 30, ALIGNMENT_EVIL, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, {34,83,68,96,119}, {}, {}, {98,88,88}, 12},
--Drake
[369] = {22, 22, 13, 50, 10, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, {34,68}, {}, {}, {88,88}, 4},
--Woman
[387] = {10, 14, 15, 8, 4, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {38}, {185}, {88}, 0},
--Farmer
[385] = {15, 15, 10, 8, 4, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {105,101}, {17}, {88}, 0},
--Fighter
[376] = {20, 17, 11, 20, 6, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {43,42}, {21,10,3,22}, {88,88}, 0},
--Gargoyle, Winged
[362] = {15, 32, 33, 50, 6, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, {68,65,69}, {46,41,47,49}, {18,1}, {58}, 5},
--Gargoyle, Worker
[363] = {25, 19, 8, 30, 15, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {34,49,37}, {1,9,18,20}, {}, 10},
--Gazer
[355] = {2, 10, 21, 20, 1, ALIGNMENT_EVIL, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, {20,68,80}, {}, {}, {}, 0},
--Ghost
[352] = {1, 10, 9, 20, 12, ALIGNMENT_EVIL, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, {128,99}, {}, {}, {}, 0},
--Gremlin
[353] = {2, 26, 7, 5, 1, ALIGNMENT_CHAOTIC, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Guard
[382] = {9, 30, 9, 100, 8, ALIGNMENT_EVIL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {42,47}, {22,10,3}, {88}, 0},
--Headless
[370] = {19, 13, 8, 20, 12, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {34,36}, {17,9}, {88}, 2},
--Horse
[430] = {22, 27, 7, 15, 3, ALIGNMENT_NEUTRAL, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Horse with rider
[431] = {22, 27, 7, 15, 3, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Hydrae
[374] = {26, 11, 2, 50, 15, ALIGNMENT_CHAOTIC, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, {69,96,98,68}, {}, {}, {}, 0},
--Insects
[343] = {1, 40, 1, 1, 1, ALIGNMENT_CHAOTIC, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, {}, {}, {}, {}, 0},
--Jester
[383] = {14, 16, 12, 8, 4, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {38,33}, {17}, {88}, 0},
--Lord British
[409] = {30, 30, 30, 255, 255, ALIGNMENT_GOOD, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {}, {}, {}, 30},
--Musician
[386] = {12, 16, 14, 8, 4, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {41,33,43}, {19,1,9}, {88,158}, 0},
--Mage
[378] = {10, 14, 22, 30, 4, ALIGNMENT_EVIL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {17,34,20,49,68,80,69,83}, {38}, {17,1}, {58,98,88}, 0},
--Merchant
[380] = {12, 12, 18, 8, 4, ALIGNMENT_EVIL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {38,43}, {17,1}, {88}, 0},
--Mimic
[98] = {22, 9, 8, 30, 15, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, {18,80}, {}, {}, {98,98}, 5},
--Mongbat
[372] = {20, 27, 13, 30, 20, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, {}, {38,43,36,37}, {9,17,18}, {98,88}, 4},
--Mouse
[354] = {2, 25, 3, 5, 2, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Rabbit
[359] = {3, 9, 3, 5, 2, ALIGNMENT_NEUTRAL, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Rat, Giant
[342] = {5, 13, 4, 8, 6, ALIGNMENT_CHAOTIC, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, {}, {}, {}, {}, 0},
--Reaper
[347] = {21, 20, 16, 30, 20, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, {17,80,66,68}, {}, {}, {58,88}, 4},
--Rot Worm
[360] = {3, 17, 2, 5, 2, ALIGNMENT_CHAOTIC, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, {}, {}, {}, {}, 0},
--Scorpion, Giant
[426] = {13, 13, 3, 20, 8, ALIGNMENT_CHAOTIC, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, {}, {}, {}, {}, 3},
--Sea Serpent
[346] = {17, 17, 9, 70, 30, ALIGNMENT_EVIL, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, {34,83,51}, {}, {}, {}, 2},
--Sheep
[348] = {5, 10, 4, 6, 2, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {129}, 0},
--Ship
[412] = {30, 30, 30, 30, 30, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 30},
--Silver Serpent
[413] = {60, 21, 8, 200, 60, ALIGNMENT_CHAOTIC, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 15},
--Skeleton
[368] = {10, 14, 7, 20, 6, ALIGNMENT_EVIL, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, {}, {43,36,37,41,38}, {9,1}, {88}, 0},
--Slime
[375] = {6, 6, 2, 15, 4, ALIGNMENT_CHAOTIC, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, {}, {}, {}, {}, 0},
--Snake
[358] = {3, 7, 6, 5, 2, ALIGNMENT_CHAOTIC, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 1},
--Spider, Giant
[361] = {9, 11, 4, 10, 8, ALIGNMENT_CHAOTIC, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, {89}, {}, {}, {129}, 0},
--Squid, Giant
[345] = {24, 20, 8, 50, 20, ALIGNMENT_EVIL, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, {17,68}, {}, {}, {}, 0},
--Swashbuckler
[377] = {15, 18, 12, 15, 4, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {43,41,33}, {19,1,9}, {88,88}, 0},
--Tangle Vine
[366] = {20, 25, 1, 30, 1, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, {}, {}, {}, {}, 4},
--Troll
[371] = {18, 14, 9, 15, 6, ALIGNMENT_EVIL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {37,36,43,41,34}, {9,18,1,19}, {98,88,88}, 4},
--Villager
[379] = {11, 12, 12, 8, 4, ALIGNMENT_CHAOTIC, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, {}, {43,33}, {17}, {88}, 0},
--Wisp
[373] = {1, 40, 20, 40, 1, ALIGNMENT_CHAOTIC, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, {20,69,129}, {}, {}, {}, 0},
--Wolf
[351] = {8, 16, 8, 8, 4, ALIGNMENT_CHAOTIC, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--TV Flower
[365] = {20, 5, 1, 30, 1, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, {20}, {}, {}, {}, 4},
--Skiff
[414] = {20, 20, 20, 20, 20, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 20},
--Raft
[415] = {10, 10, 10, 10, 10, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 10},
--Balloon
[423] = {10, 10, 10, 10, 10, ALIGNMENT_CHAOTIC, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 10},
--Cat
[388] = {4, 18, 8, 6, 2, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Sinjen (Yew Stocks guy)
[263] = {11, 12, 12, 8, 4, ALIGNMENT_CHAOTIC, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0}
}

monster_inv_food = { 129, 133, 128 } --portion of meat, ham, loaf of bread

function actor_put_to_sleep(actor)
	if actor.obj_n == 431 then --horse with rider
		Actor.use(actor) --dismount from horse
	end

	if actor.actor_num ~= 0 then
		actor.asleep = true
		if actor.in_party == true then
			party_update_leader()
		end
	end
end

function actor_can_turn_invisible(obj_n)

	if obj_n < 0x1aa and obj_n ~= 0x19b and obj_n ~= 0x176 and obj_n ~= 0x19d and obj_n ~= 0x177 then --giant scorpion, dragon, hydra, silver serpent, slime
		return true
	end

	return false
end

-- returns true if defender loses the check
function actor_int_check(defender, attacker)

   if is_god_mode_enabled() then
      if defender.in_party then
        return true
      end
      if attacker.in_party then
        return false
      end
   end
   if (math.floor(actor_int_adj(attacker) / 2) + 15) - actor_int_adj(defender) > math.random(1, 30) then
      return true -- defender lost
   end

   return false
-- if((actor1_int_adj / 2 + 15) - actor2_int_adj > random(1,30))
--return

end

function subtract_movement_pts(actor, pts)
   if actor.obj_n == 0x1af then pts = math.floor(pts / 2) end --if horse with rider

   --fixme check with the original there are two timers that are checked but never seem to be used.

   if pts < 1 then pts = 1 end

   actor.mpts = actor.mpts - pts

   if g_avatar_died == true then
      actor_avatar_death()
   end
end

function actor_move(actor, direction, flag)
   ----dgb("actor_move("..actor.name..", "..direction_string(direction)..", "..flag..") actor("..actor.x..","..actor.y..")\n");
   local x,y,z = actor.x, actor.y, actor.z
   if direction == DIR_NORTH then y = y - 1 end
   if direction == DIR_SOUTH then y = y + 1 end
   if direction == DIR_EAST then x = x + 1 end
   if direction == DIR_WEST then x = x - 1 end

   actor.direction = direction
   local did_move = Actor.move(actor, x, y, z)

   --actor.direction = direction

   if did_move then
      if actor.obj_n == 0x177 then slime_update_frames() end
      subtract_map_movement_pts(actor)
      ----dgb("actor_move() did move actor("..actor.x..","..actor.y..")\n");
   end

   return did_move and 1 or 0
end

function actor_move_diagonal(actor, x_direction, y_direction)
   local x,y,z = actor.x, actor.y, actor.z
   local direction

   if y_direction == DIR_NORTH then
      y = y - 1
      direction = x_direction == DIR_EAST and DIR_NORTHEAST or DIR_NORTHWEST
   end
   if y_direction == DIR_SOUTH then
      y = y + 1
      direction = x_direction == DIR_EAST and DIR_SOUTHEAST or DIR_SOUTHWEST
   end
   if x_direction == DIR_EAST then
      x = x + 1
      direction = y_direction == DIR_NORTH and DIR_NORTHEAST or DIR_SOUTHEAST
   end
   if x_direction == DIR_WEST then
      x = x - 1
      direction = y_direction == DIR_NORTH and DIR_NORTHWEST or DIR_SOUTHWEST
   end

   ----dgb("actor_move_diagonal("..actor.name..", "..direction_string(direction)..")\n");
   actor.direction = y_direction
   local did_move = Actor.move(actor, x, y, z)

   if did_move then
      ----dgb("did move\n");
      if actor.obj_n == 0x177 then slime_update_frames() end
      subtract_map_movement_pts(actor)

      --dgb("set dir = "..direction_string(direction).." y_dir ="..direction_string(y_direction).." ")
   end

   return did_move and 1 or 0
end

function actor_map_dmg(actor, map_x, map_y, map_z)
	local obj_n = actor.obj_n
	local actor_type = actor_tbl[obj_n]

	if actor.alive == false or actor.hit_flag == true then
		return
	end

	if obj_n ~= 0x1a7 and obj_n ~= 0x19e and obj_n ~= 0x19f and obj_n ~= 0x19c --balloon, skiff, raft, ship, immune to map dmg
	   and (actor_type == nil or actor_type[13] == 0) and actor.protected == false and actor.in_vehicle == false then
		local map_tile = map_get_dmg_tile_num(map_x, map_y, map_z)
		if map_tile ~= nil then
			actor_tile_dmg(actor, map_tile)

			if map_tile == 732 then
				local trap_obj = map_get_obj(map_x, map_y, map_z, 0xad) --trap
				if trap_obj ~= nil then
					trap_obj.invisible = false
				end
			end
		end
	end
end

function actor_tile_dmg(actor, map_tile)
		if is_god_mode_enabled() and actor.in_party then
			return
		end
		local obj_n = actor.obj_n
		local actor_type = actor_tbl[actor.obj_n]
		local dmg = 0
		local random = math.random
		if map_tile ~= nil then
			actor.hit_flag = true
			if map_tile == 564 then
				--web
				if obj_n ~= 0x169 then --giant spider
					local str = actor_str_adj(actor)
					if str < random(1, 0x1e) then
						actor.paralyzed = true
						print("`"..actor.name.." is stuck in a web!\n")
					end
				end
			elseif (map_tile >= 2 and map_tile <= 5) or map_tile == 1165 then --swamp tiles or poison field.
				--poison
				local swamp_boots = Actor.inv_get_readied_obj_n(actor, 7) == 0x1c --swamp boots
				if swamp_boots == false or map_tile == 1165 then
					if (actor_type == nil or actor_type[19] == 0) and actor.poisoned == false then --19 immune to poison
						actor.poisoned = true
						print(actor.name.." poisoned!\n")
						hit_anim(actor.x, actor.y)
					end
				end
			elseif (map_tile >= 220 and map_tile <= 223) or map_tile == 890 or map_tile == 1124 or map_tile == 1125 or map_tile == 1130 or map_tile == 1131 or map_tile == 1164 or map_tile == 1193 then
				--fire
				if map_tile == 1164 then
					dmg = random(1, 0x1e)
				else
					dmg = random(1, 8)
				end
				if actor_type ~= nil and actor_type[17] == 1 then --double damage from fire
					dmg = dmg * 2
				end

				actor_hit(actor, dmg, nil)

			elseif map_tile == 732 or map_tile == 1010 then
				--trap, log saw

				actor_hit(actor, random(1, 0x1e), nil)

			elseif (map_tile >= 1458 and map_tile <= 1463) then
				if obj_n ~= 0x16e then --tangle vine
					actor_hit(actor, random(1, 8), nil)
				end
			elseif map_tile == 562 or map_tile == 731 then
				--force field, spikes
				actor_hit(actor, random(1, 8), nil)

			elseif map_tile == 1167 then
				--sleepfield
				if (actor_type == nil or actor_type[21] == 0) and actor.asleep == false then --21 immune to sleep spell
					actor_put_to_sleep(actor)
				end
			end
		end

end

function actor_get_max_magic_points(actor)
   local obj_n = actor.base_obj_n
   if obj_n == 410 then --avatar
      return actor.int * 2
   end

   if obj_n == 378 then --mage
      return actor.int
   end

   if obj_n == 377 or obj_n == 386 then --swashbuckler, musician
      return math.floor(actor.int/2)
   end

   return 0
end

function actor_str_adj(actor)

   local str = actor.str

   if actor.cursed == false then return str end

   if str <= 3 then
      return 1
   end

   return str - 3
end

function actor_dex_adj(actor)

   local dex = actor.dex
   if actor.cursed == true then
      if dex <= 3 then
         dex = 1
      else
         dex = dex - 3
      end
   end

   if g_time_stopped == true or actor.asleep == true then
     dex = 1
   end

   return dex
end

function actor_int_adj(actor)
   local int = actor.int

   if actor.cursed == true then
     int = int - 3
   end

   if int < 1 then int = 1 end

   return int
end

--
-- actor_combat_hit_check(attacker, foe, weapon_obj)
--

function actor_combat_hit_check(attacker, foe, weapon_obj)

   if foe == nil then return false end
	if is_god_mode_enabled() and attacker.in_party then
		return true
	end
	if foe.luatype ~= "actor" or weapon_obj.obj_n == 48 then --48 glass sword
      --dgb(foe.luatype)
		return true
	end

	local use_str = false
	local obj_n = weapon_obj.obj_n

	if weapon_obj == attacker then
		if actor_tbl[obj_n][16] == 1 then --actor uses strength
			use_str = true
		end
	else
		if obj_n == 4 or obj_n == 34 or obj_n == 39 or obj_n == 44 then --spiked helm, club, mace, two-handed hammer
			use_str = true
		end
	end

   local attack_role

   if use_str then
      attack_role = actor_str_adj(attacker)
   else
      attack_role = actor_dex_adj(attacker)
   end

   local foe_dex = actor_dex_adj(foe)
   local roll = math.random(1,30)

   --dgb("foe_dex = "..foe_dex.." attack_role = "..attack_role.." random(1,30) = "..roll.."\n\n")

   if math.floor((foe_dex + 30 - attack_role) / 2) >= roll then
      return false
   end

   return true
end

--
-- actor_init(actor)
--

function actor_init(actor, alignment)

 local actor_base = actor_tbl[actor.obj_n]
 if actor_base ~= nil then

   actor.str = actor_randomise_stat(actor_base[1])
   actor.dex = actor_randomise_stat(actor_base[2])
   actor.int = actor_randomise_stat(actor_base[3])
   local hp = actor_randomise_stat(actor_base[4])
   actor.hp = hp
   actor.level = math.floor((hp + 29) / 30)
   actor.align = actor_base[6]
 else
   actor.str = 10
   actor.dex = 10
   actor.int = 10
   actor.hp = 30
   actor.level = 1
   --actor.status = 32
 end

   if alignment ~= nil and alignment ~= ALIGNMENT_DEFAULT then
      actor.align = alignment
   end

   actor.wt = 8
   actor.combat_mode = 8
   actor.magic = actor_get_max_magic_points(actor)
   actor.mpts = actor.dex
   actor.exp = 100

	if actor.obj_n == 0x16d then --tangle vine pod. Add tangle vines.
		local actor_x = actor.x
		local actor_y = actor.y
		local actor_z = actor.z
		local a

    	if map_can_put(actor_x + 1, actor_y, actor_z) then
    		a = Actor.new(0x16e, actor_x + 1, actor_y, actor_z, actor.align, WT_TANGLE)
    		if a ~= nil then a.direction = DIR_EAST end
    	end
     	if map_can_put(actor_x - 1, actor_y, actor_z) then
     		a = Actor.new(0x16e, actor_x - 1, actor_y, actor_z, actor.align, WT_TANGLE)
     		if a ~= nil then a.direction = DIR_WEST end
     	end
     	if map_can_put(actor_x, actor_y + 1, actor_z) then
     		a = Actor.new(0x16e, actor_x, actor_y + 1, actor_z, actor.align, WT_TANGLE)
     		if a ~= nil then a.direction = DIR_SOUTH end
     	end
		if map_can_put(actor_x, actor_y - 1, actor_z) then
     	   	a = Actor.new(0x16e, actor_x, actor_y - 1, actor_z, actor.align, WT_TANGLE)
     	   	if a ~= nil then a.direction = DIR_NORTH end
     	end
	end

 if actor_base ~= nil then

   local obj,i,v,j,qty,chance,chest

   --add spells
   for i,v in ipairs(actor_base[22]) do
        obj = Obj.new(336) --charge
        obj.quality = v
        obj.qty = 1
        obj.status = 56
        Actor.inv_add_obj(actor, obj)
   end

   --actor weapon
   chance = 2
   for i,v in ipairs(actor_base[23]) do
      if math.random(1,chance) == 1 then
         if v >= 36 and v <= 38 then --spear, throwing axe, dagger
            qty = math.random(6, 12)
         else
            qty = 1
         end

         for j=1,qty do
            obj = Obj.new(v)
            obj.status = 57
            if v == 90 then
               obj.qty = 1
            else
               obj.qty = 0
            end
            Actor.inv_add_obj(actor, obj)
            Actor.inv_ready_obj(actor, obj)
            if v == 41 or v == 54 then --bow, magic bow
               obj = Obj.new(55) --arrow
               obj.status = 49
               obj.qty = math.random(12, 24)
               Actor.inv_add_obj(actor, obj)
            elseif v == 42 or v == 50 then --crossbow, triple crossbow
               obj = Obj.new(56) --bolt
               obj.status = 49
               obj.qty = math.random(12, 24)
               Actor.inv_add_obj(actor, obj)
            end
         end
      end

      chance = chance * 2
   end

   -- actor armor
   chance = 2
   for i,v in ipairs(actor_base[24]) do

      if math.random(1,chance) == 1 then

        obj = Obj.new(v)
        obj.status = 57
        obj.qty = 1
        obj.status = 56
        Actor.inv_add_obj(actor, obj)
        Actor.inv_ready_obj(actor, obj)
      end

      chance = chance * 2
   end

   --actor treasure
   chance = 2
   for i,v in ipairs(actor_base[25]) do

      if math.random(1,chance) == 1 then
         if v == 98 then --chest
            chest = Obj.new(v)
            chest.frame_n = 1
            chest.status = 41
            chest.qty = math.random(1, 100)
            Actor.inv_add_obj(actor, chest)
            if math.random(0, 1) == 1 then
               obj = Obj.new(337) --effect
               obj.status = 41
               obj.qty = 1
               obj.quality = 22
               Obj.moveToCont(obj, chest)
            end
            if math.random(0, 3) ~= 0 then
               obj = Obj.new(88) --gold coin
               obj.status = 41
               obj.qty = math.random(1, 100)
               Obj.moveToCont(obj, chest)
            end
            if math.random(0, 3) ~= 0 then
               obj = Obj.new(monster_inv_food[math.random(1, 3)])
               obj.status = 41
               obj.qty = math.random(1, 10)
               Obj.moveToCont(obj, chest)
            end
            if math.random(0, 3) == 0 then
               obj = Obj.new(90) --torch
               obj.status = 41
               obj.qty = math.random(1, 6)
               Obj.moveToCont(obj, chest)
            end
            if math.random(0, 10) == 0 then
               obj = Obj.new(77) --gem
               obj.status = 41
               obj.qty = math.random(1, 4)
               Obj.moveToCont(obj, chest)
            end
         elseif v == 58 then --spell
            if math.random(0, 3) ~= 0 and actor.obj_n ~= 362 then --Gargoyle
               obj = Obj.new(88)
               obj.status = 49
               obj.qty = math.random(1, 100)
               Actor.inv_add_obj(actor, obj)
            end
            if math.random(0, 3) == 0 then
               obj = Obj.new(monster_inv_food[math.random(1, 3)])
               obj.status = 49
               obj.qty = math.random(1, 100)
               Actor.inv_add_obj(actor, obj)
            end
            if math.random(0, 3) == 0 then
               obj = Obj.new(275) --potion
               obj.frame_n = math.random(0, 7) --random potion type
               obj.status = 49
               obj.qty = 1
               Actor.inv_add_obj(actor, obj)
            end
         elseif v == 88 then --gold coin
            obj = Obj.new(v)
            obj.status = 49
            obj.qty = math.random(1,30) + math.random(1, 30)
            Actor.inv_add_obj(actor, obj)
         else
            obj = Obj.new(v)
            obj.quality = 49
            obj.qty = 1
            Actor.inv_add_obj(actor, obj)
         end

      end

      chance = chance * 2
   end
 end
end

-- [objectnum] = range
   g_range_weapon_tbl = {
      [40] = 2, -- morning star
      [47] = 2, -- halberd
      [38] = 3, -- dagger
      [83] = 5, -- flask of oil
      [33] = 4, -- sling
      [37] = 3, -- throwing axe
      [36] = 4, -- spear
      [41] = 5, -- bow
      [42] = 7, -- crossbow
      [57] = 7, -- spellbook
      [336] = 5, -- charge
      [49] = 5, -- boomerang
      [50] = 7, -- triple crossbow
      [412] = 5, -- ship
      [91] = 4, -- Zu Ylem
      [54] = 7, -- magic bow
      [79] = 7, -- lightning wand
      [80] = 7, -- fire wand
   }

local projectile_weapon_tbl =
{
--obj_n = {tile_num, initial_tile_rotation, speed, rotation_amount}
[33] = {398, 0, 2, 0}, -- sling
[36] = {547, 45,3, 0}, -- spear
[37] = {548, 0, 2, 10}, -- throwing axe
[38] = {549, 0, 2, 10}, -- dagger
[41] = {566, 90,4, 0}, -- bow
[42] = {567, 90,4, 0}, -- crossbow
[49] = {560, 0, 4, 10}, -- boomerang
[83] = {601, 0, 2, 0}, -- flask of oil
[50] = {567, 90,4, 0}, -- triple crossbow
[412] = {399, 0,2, 0}, -- ship
[91] = {612, 0, 2, 10}, -- Zu Ylem
[54] = {566, 90,4, 0}, -- magic bow
[79] = {392, 0, 2, 0}, -- lightning wand
[80] = {393, 0, 2, 0}, -- fire wand
}

weapon_dmg_tbl = {
[4] = 4, --spiked helm
[33] = 6, --sling
[34] = 8, --club
[35] = 8, --main gauche
[36] = 10, --spear
[37] = 10, --throwing axe
[38] = 6, --dagger
[39] = 15, --mace
[40] = 15, --morning star
[41] = 10, --bow
[42] = 12, --crossbow
[43] = 15, --sword
[44] = 20, --two-handed hammer
[45] = 20, --two-handed axe
[46] = 20, --two-handed sword
[47] = 30, --halberd
[49] = 8, --boomerang
[50] = 12, --triple crossbow
[48] = 255, --glass sword
[13] = 4, --spiked shield
[221] = 90, --cannon
[83] = 4, --flask of oil
[54] = 20, --magic bow
[109] = 2, --rolling pin
[100] = 6, --scythe
[101] = 4, --pitchfork
[103] = 4, --pick
[104] = 4, --shovel
[105] = 4, --hoe
[113] = 4, --cleaver
[114] = 4, --knife
[141] = 4, --decorative sword
[412] = 30, --ship
[91] = 0, --Zu Ylem
[78] = 4, --staff
[79] = 30, --lightning wand
[80] = 20, --fire wand
}

armour_tbl =
{
[1] = 1, --leather helm
[2] = 2, --chain coif
[3] = 3, --iron helm
[4] = 3, --spiked helm
[5] = 2, --winged helm
[6] = 2, --brass helm
[7] = 3, --Gargoyle Helm
[8] = 5, --magic helm
[9] = 2, --wooden shield
[10] = 3, --curved heater
[11] = 3, --winged shield
[12] = 3, --kite shield
[13] = 2, --spiked shield
[14] = 2, --black shield
[15] = 4, --door shield
[16] = 5, --magic shield
[17] = 1, --cloth armour
[18] = 2, --leather armor
[19] = 3, --ring mail
[20] = 4, --scale mail
[21] = 5, --chain mail
[22] = 7, --plate mail
[23] = 10, --magic armour
[35] = 1, --main gauche
[24] = 2, --spiked collar
[256] = 5, --protection ring
}

function get_weapon_dmg(weapon_obj_n)
   local dmg = weapon_dmg_tbl[weapon_obj_n]

   if dmg == nil and actor_tbl[weapon_obj_n] ~= nil then
      dmg = actor_tbl[weapon_obj_n][5]
   end

   return dmg
end

function actor_get_ac(actor)

   local ac = 0
   local obj

   for obj in actor_inventory(actor) do
      if obj.readied then

         local armour = armour_tbl[obj.obj_n]
         if armour ~= nil then
            ac = ac + armour
         end
      end
   end

   if actor.cursed == true then
      ac = ac - 3
   end

   if actor.protected == true then
      ac = ac + 3
   end

   return ac
end

function actor_select_obj_from_tbl(actor, obj_list_tbl)
	local obj
	local selected_obj = nil
	local random = math.random
	for obj in actor_inventory(actor) do
		if obj_list_tbl[obj.obj_n] ~= nil then
			if selected_obj == nil or random(0, 1) == 0 then
				selected_obj = obj
			end
		end
	end

	return selected_obj
end

function acid_slug_dissolve_item(target_actor)
	local acid_slug_items = { [0x2] = 1,
	[0x3] = 1,
	[0x4] = 1,
	[0x5] = 1,
	[0x7] = 1,
	[0x0A] = 1,
	[0x0B] = 1,
	[0x0C] = 1,
	[0x0D] = 1,
	[0x0F] = 1,
	[0x13] = 1,
	[0x14] = 1,
	[0x15] = 1,
	[0x16] = 1,
	[0x23] = 1,
	[0x26] = 1,
	[0x2B] = 1,
	[0x71] = 1,
	[0x72] = 1 }

	local obj = actor_select_obj_from_tbl(target_actor, acid_slug_items)

	if obj ~= nil then
		play_sfx(SFX_SLUG_DISSOLVE, true)
		print("A slug dissolves "..target_actor.name.."'s "..obj.name.."!\n")
		Actor.inv_remove_obj(target_actor, obj)
		obj = nil
	end
end

function gremlin_steal_item(target_actor)
	local gremlin_items = { [0x80] = 1,
	[0x81] = 1,
	[0x82] = 1,
	[0x83] = 1,
	[0x84] = 1,
	[0x85] = 1,
	[0x87] = 1,
	[0xD1] = 1,
	[0xD2] = 1,
	[0xD3] = 1 }

	local obj = actor_select_obj_from_tbl(target_actor, gremlin_items)

	if obj ~= nil then
		play_sfx(SFX_FAILURE, true)
		print("`"..target_actor.name.."  has been robbed!\n")
		Actor.inv_remove_obj(target_actor, obj)
		obj = nil
	end
end

function actor_take_hit(attacker, defender, max_dmg)
   if attacker.obj_n == 357 then --corpser
      attacker.frame_n = 1 --reveal corpser.
   end
   if max_dmg == -1 then
      max_dmg = 1
   elseif max_dmg > 1 and max_dmg ~= 255 then
      max_dmg = math.random(1, max_dmg)
   end

   local ac
   local defender_type = defender.luatype
   if defender_type == "actor" then
      ac = actor_get_ac(defender)
      if max_dmg ~= 255 and ac > 0 then
         max_dmg = max_dmg - math.random(1, ac)
      end
      --dgb("\nac = "..ac.."\n")
   else
      ac = 0 --object
   end
   if is_god_mode_enabled() then
     if defender_type == "actor" and defender.in_party then
--        print("`"..defender.name.." grazed.\n")
        return
     elseif attacker.in_party then
        max_dmg = 255
     end
   end
   if max_dmg > 0 then
      if defender_type == "actor" and defender.wt > 1 and defender.wt < 16 then
         --FIXME defender now targets attacker. I think.
      end

      local exp_gained = actor_hit(defender, max_dmg, attacker)

      attacker.exp = attacker.exp + exp_gained
   else
      print("`"..defender.name.." grazed.\n")
   end

   if defender_type == "actor" then

      actor_yell_for_help(attacker, defender, max_dmg)

      local defender_obj_n = defender.obj_n

      if defender.alive == true
        or defender_obj_n == 0x1a7 --balloon
        or defender_obj_n == 0x19e --skiff
        or defender_obj_n == 0x19f --raft
        or defender_obj_n == 0x19c then --ship

         local attacker_obj_n = attacker.obj_n

         if attacker_obj_n == 0x165 then --corpser
         	play_sfx(SFX_CORPSER_DRAGGED_UNDER, true)
            print("`"..defender.name.." dragged under!\n")
            defender.corpser_flag = true
            if defender.in_party == true then
               party_update_leader()
            end
         end

         if attacker_obj_n == 0x16c then --acid slug
         	acid_slug_dissolve_item(defender)
         end

         if attacker_obj_n == 0x161 then --gremlin
         	gremlin_steal_item(defender)
         end

         local actor_type = actor_tbl[attacker_obj_n]
         if actor_type ~= nil and actor_type[15] == 1 and math.random(0, 3) == 0 and defender.actor_num ~= 0 then --actor is poisonous, don't poison vehicles.
         	defender.poisoned = true
         	print("`"..defender.name.." poisoned!\n")
         end

         if max_dmg > 0 then
         	actor_hit_msg(defender)
         end
      else
         print("`"..defender.name.." killed!\n")
         --FIXME do party roster change. maybe
      end

   end
end

function actor_hit(defender, max_dmg, attacker, no_hit_anim)

	local defender_obj_n = defender.obj_n
	local exp_gained = 0

	if defender_obj_n == 0x1a7 then -- balloon.
		return 0
	end

	if defender.luatype == "actor" then
		if is_god_mode_enabled() then
			if defender.in_party then
				return 0
			else
				max_dmg = 255
			end
		end
		--actor logic here
		defender.hit_flag = true
		if no_hit_anim == nil then
			hit_anim(defender.x, defender.y)
		end
		if defender_obj_n == 409 then --lord british
			defender.hp = 0xff
		elseif defender_obj_n == 414 or defender_obj_n == 415 then --skiff, raft
			-- hit random party member.
			local num_in_party = party_get_size()
			local party_member_num = 0
			if num_in_party > 1 then
				party_member_num = math.random(0, num_in_party - 1)
			end

			local rand_party_member = party_get_member(party_member_num)
			actor_hit(rand_party_member, max_dmg, attacker, true)
			actor_hit_msg(rand_party_member)
		else
			-- actor hit logic here
			if max_dmg >= defender.hp then

				if defender.hp ~= 0 then
					if defender.in_party == true then player_subtract_karma(5) end --allowing your party to die is bad karma.
					defender.hp = 0
--					defender.alive = false -- This doesn't work. The variable is get only
					if alignment_is_evil(defender.align) then
						local actor_base = actor_tbl[defender.obj_n]
						if actor_base ~= nil then
							--exp = (base_str + base_dex + base_int + base_hp + base_dmg + base_exp_gain) / 4
							exp_gained = math.floor((actor_base[1] + actor_base[2] + actor_base[3] + actor_base[4] + actor_base[5] + actor_base[26]) / 4)
						end
					end

					defender.visible = true

					if defender.obj_n == 412 and defender.actor_num == 0 then --handle ship destruction
						Actor.unlink_surrounding_objs(defender, true) --unlink ship front/back and make objects temporary.
						defender.base_obj_n = 0x19f --raft
						--try to toss raft out from center of the ship.
						if actor_move(defender, DIR_NORTH, 1) == 0 then
							if actor_move(defender, DIR_SOUTH, 1) == 0 then
								if actor_move(defender, DIR_EAST, 1) == 0 then
									actor_move(defender, DIR_WEST, 1)
								end
							end
						end
						defender.hp = 10
						--defender.mpts = 1
						party_move(defender.x, defender.y, defender.z)
					else
						actor_dead(defender)

						defender.wt = WT_NOTHING
						if defender.in_party == true and defender.in_vehicle == false then
							party_update_leader()
						end
					end
				end
			else
				defender.hp = defender.hp - max_dmg
				if defender.wt == WT_FLEE then --flee
					defender.wt = WT_ASSAULT --assault
				end
				if defender_obj_n == 375 then --slime
					slime_divide(defender)
				end
			end
		end
	else
		--object logic here
		if defender_obj_n == 0x16e then -- tangle vine
			--FIXME do something with tangle vine. -- I think the message is all that needed added
			print("\nTangle vine killed!\n") -- The pod doesn't have this text in the original for some reason
		elseif defender.stackable == false and defender.qty ~= 0 then
			hit_anim(defender.x, defender.y)
			if (defender_obj_n < 0x129 or defender_obj_n > 0x12c or defender.frame_n < 0xc) --check frame_n for door objects
				and (defender_obj_n ~= 0x62 or defender.frame_n ~= 3) then --don't attack open chests
				if defender.qty <= max_dmg then
					print("\n`"..defender.name .. " broken!\n")

					local child
					for child in container_objs(defender) do  -- look through container for effect object.
					  if child.obj_n == 0x151 then --effect
					  	if attacker ~= nil then
					  	  actor_use_effect(attacker, child)
					  	end
					  	break
					  else
					  	Obj.moveToMap(child, defender.x, defender.y, defender.z)
					  end
					end

					if defender_obj_n == 0x7b then --mirror
						play_sfx(SFX_BROKEN_GLASS)
						defender.frame_n = 2
						player_subtract_karma(10)
					else
						map_remove_obj(defender)
					end
				else
					defender.qty = defender.qty - max_dmg
				end
			end
		end
	end

	return exp_gained
end

function actor_hit_msg(actor)

	if actor.obj_n == 0x1a7 then return end --balloon

	local hp = actor.hp

	if hp == 0 then return end

	local di = math.floor((hp * 4) / actor.max_hp)

	local s

	if di < 4 then
		s = "\n`"..actor.name.." "
	end

	if di == 0 then
		s = s.."critical!\n"
		local wt = actor.wt
		if actor_int_adj(actor) >= 5 and wt ~= WT_BERSERK and wt ~= WT_STATIONARY and wt > WT_FRONT then
			actor.wt = WT_RETREAT
		end
	elseif di < 4 then

		if di == 1 then
			s = s.."heavily "
		elseif di == 2 then
			s = s.."lightly "
		elseif di == 3 then
			s = s.."barely "
		end
		s = s.."wounded.\n"
	end

	if s ~= nil then
		print(s)
	end
end

function actor_dead(actor)
	local actor_base = actor_tbl[actor.obj_n]
	if actor_base ~= nil then
		if actor.obj_n == 0x163 then --gazer
			Actor.new(0x157, actor.x, actor.y, actor.z) --insect
		end

		if actor_base[9] == 1 then --fades away. magical creature.
			Actor.fade_out(actor, 20) --FIXME make fade speed configurable.
		end
	end

	local in_vehicle = actor.in_vehicle

	if in_vehicle == false and actor_base ~= nil and actor_base[8] == 1 or actor.obj_n == 0x187 or actor.obj_n == 0x188 then --farmer, musician
		--add some blood.
		--dgb("\nAdding Blood\n")
		local blood_obj = Obj.new(0x152, math.random(0,2))
		blood_obj.temporary = true
		Obj.moveToMap(blood_obj, actor.x, actor.y, actor.z)
	end

	if actor.actor_num ~= 1 then --avatar
		local create_body = true
		if in_vehicle == true then
			create_body = false -- don't drop body when party member dies on raft.
		end

		if actor_is_readiable_obj(actor) then
			local animated_obj = Obj.new(actor.obj_n)
			Obj.moveToMap(animated_obj, actor.x, actor.y, actor.z)
		end
		if actor.obj_n == 431 then --horse with rider
			Actor.new(430, actor.x, actor.y, actor.z, ALIGNMENT_NEUTRAL, 12) --horse, wt wander
		end
		Actor.kill(actor, create_body)
	else
		actor.hp = 0 -- hackish way of making the Avatar not extinguish his torch
		g_avatar_died = true
	end

	if actor.obj_n == 0x177 then
		slime_update_frames()
	end
end

function slime_divide(slime)

	local random = math.random
	local from_x, from_y, from_z = slime.x, slime.y, slime.z
	local i
	for i=1,8 do

		local new_x = random(1, 2) + from_x - 1
		local new_y = random(1, 2) + from_y - 1

		if map_can_put(new_x, new_y, from_z) then
			Actor.clone(slime, new_x, new_y, from_z)
			slime_update_frames()
			print("Slime divides!\n")
			return
		end
	end
end
--[[
function bitor(x, y)
 local p = 1
  while p < x do p = p + p end
  while p < y do p = p + p end
  local z = 0
  repeat
   if p <= x or p <= y then
     z = z + p
     if p <= x then
     x = x - p
     end
     if p <= y then y = y - p end

   end
  p = p * 0.5
  until p < 1
  return z
end
--]]
function slime_update_frames()

	local i, j
	local pow = math.pow

	for i=1,0xff do
		local actor = Actor.get(i)
		if actor.obj_n == 0x177 and actor.alive then --slime
			local new_frame_n = 0;
			local idx = 0
			for j = 1,8,2 do
				local tmp_actor = map_get_actor(actor.x + movement_offset_x_tbl[j], actor.y + movement_offset_y_tbl[j], actor.z)
				if tmp_actor ~= nil and tmp_actor.obj_n == 0x177 and tmp_actor.alive then
					new_frame_n = new_frame_n + pow(2,idx)
				end
				idx = idx + 1
			end
			actor.frame_n = new_frame_n
		end
	end
end

function combat_range_check_target(actor_attacking)

   if actor_attacking.wt > 1 and actor_attacking.wt < 0x10 then
--[[ FIXME
      target = actor_attacking.target_obj
      if target ~= actor_attacking then

         if target.obj_n ~= 0 and target.alive == true and target.asleep == false and target.paralysed == false and target.corpser_flag == false then

            if abs(actor_attacking.x - target.x) < 2 then

               if abs(actor_attacking.y - target.y) < 2 and (time_stop_spell_timer == 0 or target.in_party == true) then

                  return false
               end
            end
         end
      end
--]]
   end

return true
end

--check for arrows or bolts if attacking with a bow or crossbow
function out_of_ammo(attacker, weapon, print_message)

   local weapon_obj_n = weapon.obj_n

   if (weapon_obj_n == 41 or weapon_obj_n == 54) and Actor.inv_has_obj_n(attacker, 55) == false then --bow, magic bow, arrows
      if(print_message) then
         print("Out of arrows!\n")
      end
      return true --no arrows, bail out
   end
   if (weapon_obj_n == 42 or weapon_obj_n == 50) and Actor.inv_has_obj_n(attacker, 56) == false then --crossbow, triple crossbow, bolts
      if(print_message) then
         print("Out of bolts!\n")
      end
      return true --no bolts, bail out
   end
   return false
end

--
-- actor_attack(attacker, target, weapon)
--

function actor_attack(attacker, target_x, target_y, target_z, weapon, foe)

   local random = math.random
   local weapon_obj_n = weapon.obj_n
   local weapon_quality = weapon.quality

	if foe ~= nil and foe.actor_num == attacker.actor_num then
		print("\n"..attacker.name.." is try to attack itself. Report me!\n")  -- this shouldn't happen
		return
	end

   if foe == nil then
      foe = map_get_obj(target_x, target_y, target_z);
   end

   --dgb("\nactor_attack()\nrange = " .. get_attack_range(attacker.x,attacker.y, target_x, target_y).." weapon range="..get_weapon_range(weapon_obj_n))
   if weapon_obj_n ~= attacker.obj_n then
      --dgb("\nweapon = "..weapon.name.."obj_n = "..weapon_obj_n.."\n")
   end

   if Actor.get_range(attacker, target_x, target_y) > get_weapon_range(weapon_obj_n) then
      return
   end

   --check for arrows or bolts if attacking with a bow or crossbow

   if (out_of_ammo(attacker, weapon, false) == true) then
      return --no arrows or bolts, bail out
   end


   local x_diff = target_x - attacker.x
   local y_diff = target_y - attacker.y
   local attacker_direction

   if abs(x_diff) <= abs(y_diff) then
     attacker_direction = (y_diff >= 0) and DIR_SOUTH or DIR_NORTH
   else
     attacker_direction = (x_diff >= 0) and DIR_EAST or DIR_WEST
   end

   local actor_obj_n = attacker.obj_n
   --[[
   if actor_obj_n == 0x19c then --ship

      if(((*(&objlist_npc_movement_flags + actor_attacking) & 6 xor attacker_direction) & 2) == 0)
      {
         attacker_direction = attacker_direction xor 2;
      }
      else
      {
         attacker_direction = *(objlist_npc_movement_flags + actor_attacking) & 7;
      }
   end
--]]

   if attacker.obj_n ~= 412 and actor_obj_n ~= 413 then --don't change direction for ship, silver serpent.
      attacker.direction = attacker_direction
   end

   if actor_obj_n >= 0x170 and actor_obj_n <= 0x174 then -- skeleton, mongbat
      local frame_n = attacker.frame_n
      attacker.frame_n = (frame_n - (frame_n % 4)) + 2
   end


   local is_range_weapon = false

   if weapon_obj_n ~= 40 and weapon_obj_n ~= 47 then -- morning star, halberd
      if abs(x_diff) > 1 or abs(y_diff) > 1 or projectile_weapon_tbl[weapon_obj_n] ~= nil then
         is_range_weapon = true
         if combat_range_check_target(attacker) == false then return 1 end
      end
   end

   local dmg = get_weapon_dmg(weapon_obj_n)
   if dmg == nil then
      dmg = 1
   end

   --FIXME run unknown func sub_1EABC here.
   local player_loc = player_get_location()

   if weapon_obj_n == 0x150 or weapon_obj_n == 0x39 then --charge, spellbook

      if weapon_quality == 0x81 and actor_obj_n == 0x175 then -- special wisp magic (wisp teleport), wisp

         if toss_actor(attacker, player_loc.x, player_loc.y, player_loc.z) == true then

            print("Wisp teleports!\n")
         end

         return
      end

      local spell_retcode = 0

      magic_cast_spell(weapon_quality, attacker, {x = target_x, y = target_y, z = target_z})

      if weapon_quality == 0x50 and spell_retcode == 0xfe then
         print("`"..foe.name .. " is charmed.\n")
      elseif weapon_quality == 0x45 and spell_retcode == 0 then
         print("`"..foe.name .. " is paralyzed.\n")
      end

      return
   end


   --weapon here.

   local hit_actor = actor_combat_hit_check(attacker, foe, weapon)
   local num_bolts
   local coll_tiles

   if is_range_weapon == true then
      -- Get new target coordinates if the attack roll failed
      -- This is done before collision checking, which means even blocked targets can cause a miss
      -- Attacks on empty tiles alway hit
      if hit_actor == false and foe ~= nil then
            target_x = target_x + random(0, 2) - 1
            target_y = target_y + random(0, 2) - 1
      end

      if weapon_obj_n == 0x32 then --triple crossbow
         num_bolts = Actor.inv_get_obj_total_qty(attacker, 0x38)
         if num_bolts > 3 then num_bolts = 3 end
      else
         num_bolts = 1
      end

      -- Handle collision checking, projectile animation,
      -- ammo removal/placement on map and certain special effects
      coll_tiles = combat_range_weapon_1D5F9(attacker, target_x, target_y, target_z, weapon, num_bolts)

      -- For each projectile, coll_tiles now contains the new target tile
      -- coordinates after collision checking and the foe on that tile (or nil if no actor/obj)

      -- For convenience:
      foe = coll_tiles[1].foe

      -- If the first attack was a miss, do another attack roll
      -- It is possible to roll against the same target that was missed on the previous try a second time
      if hit_actor == false then
         hit_actor = actor_combat_hit_check(attacker, foe, weapon)
      end

   else --standard weapon
      if actor_find_max_xy_distance(attacker, player_loc.x, player_loc.y) < 6 then
         --play_sound_effect(0x11, 0);
         play_sfx(SFX_ATTACK_SWING, true)
      end
   end

   if weapon_obj_n == 0x32 then --triple crossbow

      local i

      -- Do attack roll for the extra projectiles and deal damage to each target hit
      for i=1,num_bolts do
         if i > 1 then
            foe = coll_tiles[i].foe
            hit_actor = actor_combat_hit_check(attacker, foe, weapon)
         end
         if hit_actor == true then
            if foe ~= nil and foe.obj_n ~= attacker.obj_n then actor_take_hit(attacker, foe, dmg) end
            if g_avatar_died == true then --TODO: Is this really needed?
               return
            end
         end
      end
      return
   else --not triple crossbow
      if hit_actor == true and foe ~= nil and attacker.obj_n ~= foe.obj_n then

         local actor_base = actor_tbl[foe.obj_n]
         if actor_base ~= nil and actor_base[14] == 1 -- takes half dmg
            and weapon_obj_n ~= 0x30 and weapon_obj_n ~= 0x32 and weapon_obj_n ~= 0x36 then --glass sword, triple crossbow, magic bow
            dmg = math.floor((dmg + 1) / 2)
         end

         if weapon_obj_n ~= 0x5b then --Zu Ylem
            actor_take_hit(attacker, foe, dmg)
         end

         if weapon_obj_n == 0x30 then
            print("Thy sword hath shattered!\n")
            Actor.inv_remove_obj(attacker, weapon)
         end
      end

      if weapon_obj_n == 0x31 then --boomerang
         if attacker.x ~= 0 and attacker.y ~= 0 then --hack to stop return projectile if the avatar has died
            projectile(projectile_weapon_tbl[weapon_obj_n][1], coll_tiles[1].x, coll_tiles[1].y, attacker.x, attacker.y, projectile_weapon_tbl[weapon_obj_n][3], projectile_weapon_tbl[weapon_obj_n][4])
         end
      end
   end
end


function combat_range_weapon_1D5F9(attacker, target_x, target_y, target_z, weapon, num_bolts)
   -- Handles projectiles:
   -- 1. Collision checking
   -- 2. Animation
   -- 3. Ammo removal
   -- 4. Placement on map (throwing weapons)
   -- 5. Zu Ylem sleep effect
   -- Returns new target tiles/foes after collision checking (The original used globals for this)

   local weapon_obj_n = weapon.obj_n
   local random = math.random
   local coll_tiles = {} --collision tile and foe for each projectile
   local pre_coll_tiles = {} --tile traversed before the collision occurred for each projectile
   local foe
   local targets

   --set up targets table
   if weapon_obj_n == 0x32 then --triple crossbow
      local index = ((target_y - attacker.y + 5) * 11) + (target_x - attacker.x + 5) + 1
      targets = {
         {x=target_x,
          y=target_y,
          z=target_z},
         {x=target_x + movement_offset_x_tbl[g_projectile_offset_tbl[1][index]+1],
          y=target_y + movement_offset_y_tbl[g_projectile_offset_tbl[1][index]+1],
          z=target_z},
         {x=target_x + movement_offset_x_tbl[g_projectile_offset_tbl[2][index]+1],
          y=target_y + movement_offset_y_tbl[g_projectile_offset_tbl[2][index]+1],
          z=target_z}
      }
   else --not triple crossbow
      targets = {
         {x=target_x,
          y=target_y,
          z=target_z}
      }
   end

   -- Collision checking
   -- Sets up two tables:
   -- coll_tiles: New target tile and foe for each projectile after collision check
   -- pre_coll_tiles: Tile traversed before each collision (used for animation and thrown weapon placement)
   for i=1,num_bolts do
      targ = targets[i]
      local coll_x, coll_y, pre_coll_x, pre_coll_y = map_line_hit_check(attacker.x, attacker.y, targ.x, targ.y, targ.z)
      foe = map_get_actor(coll_x, coll_y, targ.z)
      if foe == nil then foe = map_get_obj(coll_x, coll_y , targ.z) end
      table.insert(coll_tiles, {x = coll_x, y = coll_y, z = targ.z, foe = foe})
      table.insert(pre_coll_tiles, {x = pre_coll_x, y = pre_coll_y, z = targ.z})
   end

   -- Projectile animation
   -- TODO: Animation currently does its own collision checking. This should only be done once.
   if weapon_obj_n == 0x32 then --triple crossbow
      projectile_anim_multi(projectile_weapon_tbl[weapon_obj_n][1], attacker.x, attacker.y, coll_tiles, projectile_weapon_tbl[weapon_obj_n][3], 0, projectile_weapon_tbl[weapon_obj_n][2])
   else
      projectile(projectile_weapon_tbl[weapon_obj_n][1], attacker.x, attacker.y, coll_tiles[1].x, coll_tiles[1].y, projectile_weapon_tbl[weapon_obj_n][3], projectile_weapon_tbl[weapon_obj_n][4])
   end

   if weapon_obj_n == 0x5b then
      --Zu Ylem
      if foe ~= nil and foe.obj_n ~= 0 then
         Actor.inv_remove_obj_qty(attacker, 0x5b, 1)
         spell_put_actor_to_sleep(attacker, foe)
      end
   elseif weapon_obj_n == 0x4f or weapon_obj_n == 0x50 then
      --lightning wand, fire wand
      if random(1, 100) == 37 then
      	Actor.inv_remove_obj(attacker, weapon)
      	print("A wand has vanished!\n")
      end
   elseif weapon_obj_n == 0x53 then
      --flask of oil

      Actor.inv_remove_obj_qty(attacker, 0x53, 1)

      if map_is_water(pre_coll_tiles[1].x, pre_coll_tiles[1].y, pre_coll_tiles[1].z, true) == false then
	      local obj = Obj.new(317); --fire field
          obj.temporary = true
	      Obj.moveToMap(obj, pre_coll_tiles[1].x, pre_coll_tiles[1].y, pre_coll_tiles[1].z)
      end

   elseif weapon_obj_n == 0x24 or weapon_obj_n == 0x25 or weapon_obj_n == 0x26 then
      --spear, throwing axe, dagger

      -- Only remove thrown weapon from attacker if distance to target is > 1,
      -- else just skip handling and keep the item
      if abs(coll_tiles[1].x - attacker.x) > 1 or abs(coll_tiles[1].y - attacker.y) > 1 then
         --remove unreadied throwing weapons from inventory first so weapon stays readied
         local removal_candidate = nil
         for inv_obj in actor_inventory(attacker) do
            if inv_obj.obj_n == weapon_obj_n then
               removal_candidate = inv_obj
               if removal_candidate.readied ~= true then
                  break
               end
            end
         end
         if removal_candidate ~= nil then
            Actor.inv_remove_obj(attacker, removal_candidate)

            local function can_place_at(x, y, z)
               -- Helper function to check if thrown weapon can be placed at target coordinates

               local dest_tile_num = map_get_tile_num(x, y, z, true)

               -- Tile flags: water and impassable
               local bool is_impassable_water = tile_get_flag(dest_tile_num, 1, 0) and tile_get_flag(dest_tile_num, 1, 1)
               -- Ethereal void tile
               local bool is_ethereal_void = dest_tile_num >= 252 and dest_tile_num <= 255

               -- Do not place on impassable water or ethereal void
               -- unless there already is an object at the destination
               -- (passable water is OK, e.g. shore tiles)
               return not ((is_impassable_water or is_ethereal_void) and map_get_obj(x, y, z) == nil)
            end

            -- Do not place thrown weapon if target is an actor
            if (coll_tiles[1].foe == mil or coll_tiles[1].foe.luatype ~= "actor")
                  and can_place_at(pre_coll_tiles[1].x, pre_coll_tiles[1].y, pre_coll_tiles[1].z) then
               local obj = Obj.new(weapon_obj_n);
               obj.ok_to_take = true
               obj.temporary = true
               Obj.moveToMap(obj, pre_coll_tiles[1].x, pre_coll_tiles[1].y, pre_coll_tiles[1].z)
            end
         end
      end
   elseif weapon_obj_n == 0x29 or weapon_obj_n == 0x2a or weapon_obj_n == 0x32 or weapon_obj_n == 0x36 then
      --bow, crossbow, triple crossbow, magic bow
      local projectile_obj = nil
      if weapon_obj_n == 0x29 or weapon_obj_n == 0x36 then --bow, magic bow
      	projectile_obj = 0x37 --arrow
      else
      	projectile_obj = 0x38 --bolt
      end

      local qty = 1
      if weapon_obj_n == 0x32 then -- triple crossbow
      	qty = 3
      end

      Actor.inv_remove_obj_qty(attacker, projectile_obj, qty)
   end

   return coll_tiles --original always returns 1
end

--
-- actor_get_weapon()
--
function actor_get_weapon(attacker, foe)

   if foe == nil then return nil end

   local in_party = attacker.in_party
   local range = Actor.get_range(attacker, foe.x, foe.y)
   --dgb("range = "..range.."\n")
   local max_dmg = 0
   local obj, weapon

   for obj in actor_inventory(attacker) do
      if obj.obj_n == 336 and timer_get(TIMER_STORM) == 0 and math.random(0,3) == 0 and math.random(0, obj.quality) < 16 then --charge (spell)
         --dgb("magic object quality = "..obj.quality.."\n");
         return obj
      else
         if in_party == false or obj.readied == true then
            local dmg = get_weapon_dmg(obj.obj_n)
            if dmg ~= nil and dmg > max_dmg and get_weapon_range(obj.obj_n) >= range then
               max_dmg = dmg
               weapon = obj
            end
         end
      end
   end

   if weapon == nil then --attack with bare hands.
   	weapon = attacker
   end

   --dgb("weapon: "..weapon.name.." dmg="..get_weapon_dmg(weapon.obj_n).."\n")

   return weapon
end

--
-- actor_calculate_avg_coords()
--
function actor_calculate_avg_coords()
   local n = 0
   local avg_x = 0
   local avg_y = 0

   local player_loc = player_get_location()

   local player_x = player_loc.x
   local player_y = player_loc.y

   local player = Actor.get_player_actor()
   local player_dir = player.direction

   local actor
   for actor in party_members() do
      if actor.wt ~= WT_FLANK and actor.wt ~= WT_BERSERK then
         n = n + 1
         avg_x = avg_x + actor.x
         avg_y = avg_y + actor.y
      end
   end

   if n > 0 then
      party_avg_x = math.floor(avg_x / n)
      party_avg_y = math.floor(avg_y / n)
   else
      party_avg_x = player_x
      party_avg_y = player_y
   end

   n = 0
   avg_x = 0
   avg_y = 0

   local i
   for i=0,0xff do
      actor = Actor.get(i)

      if actor.obj_n ~= 0 and actor.alive then

         if (actor.align == ALIGNMENT_EVIL or actor.align == ALIGNMENT_CHAOTIC) then
            if actor.wt ~= WT_RETREAT or (abs(actor.x -  player_x) <= 5 and abs(actor.y - player_y) <= 5) then
               if abs(actor.x - player_x) < 0x18 and abs(actor.y - player_y) < 0x18 then
                  n = n + 1
                  avg_x = avg_x + actor.x
                  avg_y = avg_y + actor.y
               end
            end
         end
      end
   end

   wt_num_monsters_near = n

   if n > 0 then
      combat_avg_x = math.floor((avg_x + n / 2) / n)
      combat_avg_y = math.floor((avg_y + n / 2) / n)

      wt_rear_max_monster = nil
      wt_rear_min_monster = nil
      --wt_rear_max_party = nil
      --wt_rear_min_party = nil

      local tmp_x = combat_avg_x - party_avg_x
      local tmp_y = combat_avg_y - party_avg_y

      wt_front_target_actor = nil
      local min_m_pos = 0
      local max_m_pos = 0
      local min_p_pos = 0
      local max_p_pos = 0
      --var_16 = 0x8000

      local var_A
      for i=0,0xff do
         actor = Actor.get(i)
         if actor.obj_n ~= 0 and actor.alive and (actor.wt == WT_FRONT or actor.wt == WT_PLAYER) then
            --var_C = ((actor.x - party_avg_x) * tmp_x) + ((actor.y - party_avg_y) * tmp_y)
            var_A = ((actor.x - party_avg_x) * tmp_y) - ((actor.y - party_avg_y) * tmp_x)
          --  --dgb("ok "..actor.name.."\n")
            if actor.in_party == false then
               if wt_front_target_actor == nil then wt_front_target_actor = actor end
               if var_A > max_m_pos then max_m_pos, wt_rear_max_monster = var_A, actor end
               if var_A < min_m_pos then min_m_pos, wt_rear_min_monster = var_A, actor end
            else
           --dgb("yup var_A["..i.."] = "..var_A.."\n")
               if var_A > max_p_pos then
               	--dgb("var_A > max_p_pos\n")
               	max_p_pos, wt_rear_max_party = var_A, actor
               end
               if var_A < min_p_pos then
                --dgb("var_A < min_p_pos\n")
               	min_p_pos, wt_rear_min_party = var_A, actor
               end
               --if var_C > var_16 then var_16, unk_3DEAD = var_C, actor end
            end
         end
      end
      --FIXME I'm not really sure what the original does when these aren't set
      if wt_rear_max_party == nil then wt_rear_max_party = player end
      if wt_rear_min_party == nil then wt_rear_min_party = player end
   else
      --this is used by party members when in combat_front worktype.
      combat_avg_x = player_x + movement_offset_x_tbl[(player_dir*2) + 1]
      combat_avg_y = player_y + movement_offset_y_tbl[(player_dir*2) + 1]
   end

   if combat_avg_x == party_avg_x and combat_avg_y == party_avg_y then
      combat_avg_x = combat_avg_x + movement_offset_x_tbl[(player_dir*2) + 1]
      combat_avg_y = combat_avg_y + movement_offset_y_tbl[(player_dir*2) + 1]
   end
   --dgb("PartyAvg("..party_avg_x..","..party_avg_y..") CombatAvg("..combat_avg_x..","..combat_avg_y..") Player("..player_x..","..player_y..") Dir = "..player_dir.." numMonsters = "..wt_num_monsters_near.."\n")
end
--[[
function trace (event, line)
local s = debug.getinfo(2).name
if s == nil then
	s = "Unk"
end
--io.stderr:write(s .. ":" .. line.."\n")
end

   debug.sethook(trace, "l")
--]]

--
-- actor_update_all()
--
function actor_update_all()
--dgb("actor_update_all()\n")
   if g_avatar_died == true then
      actor_avatar_death()
      return
   end
   g_time_stopped = is_time_stopped()

   actor_calculate_avg_coords()
   local actor
   local selected_actor
   repeat
      selected_actor = nil
      local di = 0
      local dex_6 = 1
      repeat
         local player_loc = player_get_location()
         local var_C = (player_loc.x - 16) - (player_loc.x - 16) % 8
         local var_A = (player_loc.y - 16) - (player_loc.y - 16) % 8

         for actor in party_members() do
            if actor.wt == WT_FOLLOW and actor.mpts < 0 then
               actor.mpts = 0
            end
         end

         local player_z = player_loc.z
         for i=0,0xff do
            local actor = Actor.get(i)
            if actor.obj_n ~= 0 and actor.z == player_z and actor.mpts > 0 and actor.paralyzed == false and actor.asleep == false and actor.wt ~= WT_NOTHING and actor.alive == true then
                  if abs(actor.x - var_C) > 0x27 or abs(actor.y - var_A) > 0x27 then
                     if actor.wt >= 0x83 and actor.wt <= 0x86 then
                        --move actor to schedule location if it isn't on screen
                        local sched_loc = actor.sched_loc
                        if map_is_on_screen(sched_loc.x, sched_loc.y, sched_loc.z) == false then
                        	Actor.move(actor, sched_loc.x, sched_loc.y, sched_loc.z)
                        	actor_wt_walk_to_location(actor) --this will cancel the pathfinder and set the new worktype
                        	subtract_movement_pts(actor, 10)
                        	----dgb("\nActor SCHEDULE TELEPORT "..actor.actor_num.." to ("..sched_loc.x..","..sched_loc.y..","..sched_loc.z..")\n")
                        end
                     end
                  else
                     if actor.wt ~= WT_FOLLOW then
                        if actor.wt == 0x80 then
                           -- actor_set_worktype_from_schedule(actor)
                           actor.wt = actor.sched_wt
                        end

                        local dx = (actor.mpts * dex_6) - actor.dex * di
                        if actor.mpts >= actor.dex or dx > 0 or dx == 0 and actor.dex > dex_6 then
                           selected_actor = actor
                           di = actor.mpts
                           dex_6 = actor.dex
                        end

                        if actor.mpts >= actor.dex then
                           break
                        end
                     end
                  end
            end
         end

         if di <= 0 then
            advance_time(1)
         end
      until di > 0

      if selected_actor.corpser_flag == true then
         actor_corpser_regurgitation(selected_actor)
      end

      if selected_actor.corpser_flag == false then
         if selected_actor.wt ~= WT_PLAYER and selected_actor.wt ~= WT_FOLLOW then
            --dgb("perform_worktype("..selected_actor.name.."("..selected_actor.actor_num..") dex = "..selected_actor.dex.." mpts = "..selected_actor.mpts..").\n")
            perform_worktype(selected_actor)
            --sub_1C34A() --update map window??
            if selected_actor.wt > 1 and selected_actor.wt < 0x10 then
               --FIXME targeting?? do *(&objlist_ptr_unk_18f1 + actor_num) = actor_num
            end
         end

         --sub_4726()
      end

   until selected_actor.obj_n ~= 0 and selected_actor.wt == WT_PLAYER and selected_actor.corpser_flag == false

   if selected_actor ~= nil then --swap player to next party member with 'command' combat worktype.
    local old_player = Actor.get_player_actor()
	player_set_actor(selected_actor)
	old_player.wt = WT_PLAYER --reset worktype to player as it gets changed to follow in Player::set_actor() :-(
   end

	if g_update_volcano == true then
		local obj = find_volcano_near_player()
		if obj ~= nil then
			if obj.obj_n == 307 then --volcano
				play_sfx(SFX_EARTH_QUAKE, false)
				quake_start(1,200)
			else
				local temp_fumarole = Obj.new(obj) --duplicate the fumarole object so we can explode it.
				explode_obj(temp_fumarole)
			end
		end
		g_update_volcano = false
	end

   display_prompt(true)
end

function advance_time(num_turns)
	local teleport = num_turns >= 60
	--dgb("advance_time("..num_turns..")")
	local time_stop_timer = timer_get(TIMER_TIME_STOP)

	if time_stop_timer ~= 0 then
		if time_stop_timer > num_turns then
			timer_set(TIMER_TIME_STOP, time_stop_timer - num_turns)
			return
		end

		timer_set(TIMER_TIME_STOP, 0)
	end

	timer_update_all(num_turns)

	--update keg timer. explode kegs.
	if g_keg_timer > 0 then
		g_keg_timer = g_keg_timer - num_turns
		if g_keg_timer <= 0 then
			explode_keg()
			g_keg_timer = 0
		end
	end

	local random = math.random
	local cloak_readied = false
	local i
	for i=0,0xff do
		local actor = Actor.get(i)

		if actor.obj_n ~= 0 then

			if actor.mpts >= 0 then
				m = actor.dex
			else
				m = actor.mpts + actor.dex
			end

			actor.mpts = m
			if actor.wt == WT_FOLLOW and actor.corpser_flag == true then
				actor_corpser_regurgitation(actor) --hack the original does this in party_move() but we don't have that in script yet.
			end

			if actor.in_party == true then
				local obj
				for obj in actor_inventory(actor) do
					if obj.readied == true then
						local obj_n = obj.obj_n
						local obj_name = nil
						if obj_n == 0x5a and obj.frame_n == 1 then --lit torch
							if random(0, 1) == 0 then
								if obj.qty > num_turns then
									obj.qty = obj.qty - num_turns
								else
									--print("A torch burned out.\n")
									Actor.inv_unready_obj(actor, obj)
								end
							end
						elseif obj_n == 0x102 then --invisibility ring
							if random(0, 1000) == 429 then
								local two_rings = (Actor.inv_get_readied_obj_n(actor, 5) == 0x102
								                   and Actor.inv_get_readied_obj_n(actor, 6) == 0x102)
								if two_rings == false then
									actor.visible = true
								end
								obj_name = "ring"
							end
						elseif obj_n == 0x101 then -- regeneration ring
							--FIXME the original increments a counter here.
							local hp = actor.hp + num_turns
							local max_hp = actor.max_hp
							if hp > max_hp then
								hp = max_hp
							end
							actor.hp = hp

							if random(0, 1000) == 734 then
								obj_name = "ring"
							end
						elseif obj_n == 0x51 then --storm cloak
							if random(0, 1000) == 588 then
								obj_name = "cloak"
								timer_set(TIMER_STORM, 0)
							else
								cloak_readied = true
							end
						end

						if obj_name ~= nil then
							Actor.inv_remove_obj(actor, obj)
							print("A "..obj_name.." has vanished!\n")
						end
					end
				end
			end
			local j
			for j=1,num_turns do
				actor_update_flags(actor)
			end

			actor_map_dmg(actor, actor.x, actor.y, actor.z)
			actor.hit_flag = false
		end
	end

	if cloak_readied == true then
		timer_set(TIMER_STORM, 1)
	end

	local minute = clock_get_minute()

	clock_inc(num_turns)

	if minute + num_turns >= 60 then

		update_actor_schedules(teleport)

		--update magic points
		local party_actor
		for party_actor in party_members() do
			local max_magic = actor_get_max_magic_points(party_actor)
			if max_magic ~= 0 then -- Avatar, Mage, Swashbuckler, Musician
				local magic_pts = party_actor.magic + party_actor.level
				if magic_pts > max_magic then
					magic_pts = max_magic
				end
				party_actor.magic = magic_pts
			end
		end

		player_dec_alcohol(1)
	end
	if g_avatar_died == true then
		actor_avatar_death()
	end

	if random(0,7) == 0 then
		g_update_volcano = true
	end
end

function actor_update_flags(actor)

	local random = math.random

	if actor.alive then
		local var_8 = 0 --FIXME get proper value for var_8.
		if actor.visible == false and var_8 == 0 and random(0, 0x3f) == 0
		   and Actor.inv_get_readied_obj_n(actor, 5) ~= 0x102 --invisibility rings
		   and Actor.inv_get_readied_obj_n(actor, 6) ~= 0x102 then
			if actor_int_adj(actor) <= random(1, 0x1e) then
				actor.visible = true
			end
		end

		if actor.protected == true and random(0, 0x3f) == 0 then
			if actor_int_adj(actor) <= random(1, 0x1e) then
				actor.protected = false
			end
		end

		if actor.cursed == true and random(0, 15) == 0 then
			if actor_int_adj(actor) >= random(1, 0x1e) then
				actor.cursed = false
			end
		end

		if actor.charmed == true and random(0, 7) == 0 then
			if actor_int_adj(actor) >= random(1, 0x1e) then
				actor_remove_charm(actor)
			end
		end

		if actor.paralyzed == true and random(0, 3) == 0 then
			if actor_str_adj(actor) >= random(1, 0x1e) then
				actor.paralyzed = false
				party_update_leader()
			end
		end

		if actor.asleep == true and actor.wt ~= WT_SLEEP and random(0, 15) == 0 then
			actor.asleep = false
			actor.hit_flag = true
			actor.frame_n = actor.old_frame_n
			if actor.in_party == true then
				party_update_leader()
			end
		end

		if actor.poisoned == true and actor.protected == false and random(0, 7) == 0 then
			if map_is_on_screen(actor.x, actor.y, actor.z) == true then
				actor_hit(actor, 1)
			else -- don't show the animation or play the sfx
				actor_hit(actor, 1, nil, true)
			end
		end
	end

end

function actor_corpser_regurgitation(actor)
   --dgb("actor_corpser_regurgitation("..actor.name..")\n");
   if actor.corpser_flag == false then return end

   if actor.wt == WT_PLAYER then
      print("\n"..actor.name..":\nARGH!\n")
   end

   local random = math.random
   local val = random(1, 0x1e)

   if val < actor_str_adj(actor) then
      play_sfx(SFX_CORPSER_REGURGITATE, true)
      print("`"..actor.name.." regurgitated!\n")
      actor.corpser_flag = false
      if actor.in_party == true then
         party_update_leader()
      end
   else
      actor_hit(actor, random(1, 0xf))
      actor.mpts = 0
   end

end

function actor_remove_charm(actor)

	if actor.charmed == true then
		actor.charmed = false;
		actor.align = actor.old_align

		if actor.in_party == true then
			actor.align = ALIGNMENT_GOOD
		end
		party_update_leader()
		--[[ FIXME need to add functions to check for combat_mode and solo_mode
		if party_in_combat_mode() then
			actor.wt = combat_mode
		else
			if solo_mode() then
				actor.wt = WT_NOTHING
			else
				actor.wt = WT_FOLLOW
			end
		end
		--]]
		return true
	end

	return false
end

function actor_yell_for_help(attacking_actor, defending_actor, dmg)

   if defending_actor.hp == 0 then
     return
   end

   if defending_actor.luatype == "actor" and defending_actor.align == ALIGNMENT_NEUTRAL and defending_actor.in_party == false then
      local actor_base = actor_tbl[defending_actor.obj_n]

      if attacking_actor.wt == WT_PLAYER and actor_base ~= nil then
         if actor_base[7] ~= 0 and dmg > 0 then  --[7] == can_talk
            print("`"..defending_actor.name.." yells for help!\n")
            activate_city_guards()
         end
      end

      if defending_actor.wt >= 0x80 then
         if attacking_actor.wt == WT_PLAYER then
            if defending_actor.obj_n == 0x188 then -- musician
               defending_actor.frame_n = defending_actor.old_frame_n
            end
            defending_actor.wt = WT_ATTACK_PARTY
         end

      else

         if attacking_actor.align ~= ALIGNMENT_GOOD then
            defending_actor.align = ALIGNMENT_GOOD
         else
            defending_actor.align = ALIGNMENT_CHAOTIC
         end

         if defending_actor.temp == true then
            defending_actor.wt = WT_ASSAULT
         end
      end

   end

end

function can_get_obj_override(obj) --meat, ribs, dead animal. These are toptile objects and need an override check.
	if obj.obj_n == 209 or obj.obj_n == 210 or obj.obj_n == 211 then
		return true
	end
	return false
end

function actor_get_obj(actor, obj)

	if obj.getable == false then
		print("\n\nNot possible.")
		return false
	end

	if Actor.can_carry_obj_weight(actor, obj) == false then
		print("\n\nThe total is too heavy.")
		return false
	end

	local player_loc = player_get_location()
	local i
	local caught_stealing = false
	if player_loc.z == 0 and obj.ok_to_take == false then --stealing logic only applies to surface level.
		player_subtract_karma(1)
		play_sfx(SFX_FAILURE, true)
		for i=1,202 do --perm actors
			local actor = Actor.get(i)

			if actor.alive == true and actor.z == player_loc.z and actor.in_party == false and actor.align == ALIGNMENT_NEUTRAL and actor.asleep == false then
				if actor_find_max_xy_distance(actor, player_loc.x, player_loc.y) < 0x6 then
					print("\n\n\"Stop Thief!!!\"\n")
					activate_city_guards()
					caught_stealing = true
					break
				end
			end
		end

		if caught_stealing == false then
			print("\n\nStealing!!!\n")
		end
	end

	obj.ok_to_take = true

   Obj.moveToInv(obj, actor.actor_num)
   subtract_movement_pts(actor, 3)

	return true
end

function activate_city_guards()
--dgb("activate_city_guards()")
   local i
   local player_loc = player_get_location()

   player_subtract_karma(5)

   for i=1,0xff do
      local actor = Actor.get(i)

      if actor.alive == true and actor.z == player_loc.z and actor.in_party == false and actor.align == ALIGNMENT_NEUTRAL then
         if actor.obj_n == 0x17e then -- guard
            if actor_find_max_xy_distance(actor, player_loc.x, player_loc.y) < 0x20 then
               actor.wt = WT_GUARD_ARREST_PLAYER
            end
         else
            if actor.wt ~= WT_STATIONARY
             and actor.wt ~= WT_NOTHING
             and actor.wt ~= WT_PLAYER
             and actor.paralyzed == false
             and actor.asleep == false
             and actor.corpser_flag == false then
               if actor_find_max_xy_distance(actor, player_loc.x, player_loc.y) < 0x6 then
                  if actor.wt < 0x80 then
                     actor.wt = WT_UNK_13
                  else
                     if actor.wt > 0x86 then
                        if actor.obj_n == 0x188 then
                           actor.frame_n = actor.old_frame_n
                        end
                        actor.wt = WT_ATTACK_PARTY
                     end
                  end
               end
            end
         end
      end
   end
end

function actor_catch_up_to_party(actor)
   local target_actor = nil
   local actor_x = actor.x
   local actor_y = actor.y
   local var_2 = 0x32
   local party_actor
   local random = math.random

   for party_actor in party_members() do

      if party_actor.alive == true and actor_ok_to_attack(actor, party_actor) == true then

         local party_actor_x = party_actor.x
         local party_actor_y = party_actor.y
         local var_4 = (party_actor_x - actor_x) * (party_actor_x - actor_x) + (party_actor_y - actor_y) * (party_actor_y - actor_y)
         if var_4 < var_2 or var_4 == var_2 and random(0, 1) ~= 0 then
            var_2 = var_4
            target_actor = party_actor
         end
      end
   end

   if target_actor == nil then
      actor_move_towards_loc(actor, party_avg_x, party_avg_y)
      return false
   end

   actor_move_towards_loc(actor, target_actor.x, target_actor.y)

   if abs(target_actor.x - actor.x) > 1 or abs(target_actor.y - actor.y) > 1 then
     return false
   end

   return true --reached party
end

function caught_by_guard(actor)

   local lord_british = Actor.get(6)

   --FIXME we don't want people going to jail before they answer the copy protection questions.
   --if (objlist_talk_flags[5] & 0x80) == 0 then
   --   actor.wt = 0x81
   --   return
   --end

   Actor.show_portrait(actor)

   print("\n\"Thou art under arrest!\"\n\n\"Wilt thou come quietly?\"\n\n:")

   local var_6 = input_select("yn", false)

   actor.wt = 0x81

   Actor.hide_portrait()

   if var_6 == "Y" then
      print("es\n\nThe guard strikes thee unconscious!\n\nThou dost awaken to...\n")
      --sub_2ACA1()

      fade_out()

      if party_is_in_combat_mode() then
         party_set_combat_mode(false)
      end

      player_move(0xe7, 0xba, 0, true)

      local cur_hour = clock_get_hour()
      while cur_hour ~= 8 do
      	advance_time(60)
      	cur_hour = clock_get_hour()
      	--FIXME need to pause and update screen so player can see the sun-moon display move.
      end

      for party_actor in party_members() do
         for var_4 in actor_inventory(party_actor, true) do -- recursively search containers in inventory.
            --if((*(var_4 + objlist_obj_flags) & 0x18) == 0 || sub_CC5E(var_4, *(di + objlist_party_roster)) == 0)
            --   break

            if var_4.obj_n == 0x3f then --lockpick
               Actor.inv_remove_obj(party_actor, var_4)
            end

            if var_4.obj_n == 0x40 and var_4.quality == 9 then --key

               local obj = map_get_obj(0xeb, 0xb7, 0, 0xb1) --desk
               if obj ~= nil then
                  Obj.moveToCont(var_4, obj)
               end
            end
         end
      end

      local obj = map_get_obj(0xe7, 0xb8, 0, 0x12c) --steal door
      if obj ~= nil then
         obj.frame_n = 9 --close and lock door.
      end

      fade_in()

   --[[
      word_31D08 = 1
      sub_E5F2()
      music_play_song()
      ax = sub_46DC()
   --]]
   else

      print("o\n\n\"Then defend thyself, rogue!\"\n")
      activate_city_guards()
      actor.wt = WT_ASSAULT
      actor.align = ALIGNMENT_EVIL

      local i
      for i=1,0xff do
         local a = Actor.get(i)
         if a.wt == WT_GUARD_ARREST_PLAYER then
            a.wt = WT_ASSAULT
            a.align = ALIGNMENT_EVIL
         end
      end

   end

   return
end

local tangle_vine_frame_n_tbl = {
1, 3, 1, 4,
5, 0, 4, 0,
1, 2, 1, 5,
2, 0, 3, 0
}

function move_tanglevine(actor, new_direction)
   local actor_x = actor.x
   local actor_y = actor.y
   local old_direction = actor.direction

   if new_direction ~= direction_reverse(old_direction) then

      if actor_move(actor, new_direction, 1) ~= 0 then
         local player_loc = player_get_location()

         local tangle_obj = Obj.new(0x16e) --tanglevine

         tangle_obj.frame_n = tangle_vine_frame_n_tbl[old_direction * 4 + new_direction + 1]
         tangle_obj.status = 0x20
         tangle_obj.quality = 0 --actor.id_n

         Obj.moveToMap(tangle_obj, actor_x, actor_y, player_loc.z)

         actor.frame_n = tangle_vine_frame_n_tbl[new_direction * 4 + math.random(0, 3) + 1]

         return true
      end
   end

   return false
end

function actor_wt_front(actor)
	--dgb("actor_wt_front("..actor.name..")\n")
	if actor_wt_front_1FB6E(actor) ~= 0 then
		if actor.in_party == true then
			actor_wt_attack(actor)
		else
			if actor_find_max_xy_distance(actor, party_avg_x, party_avg_y) >= 8 then
				subtract_movement_pts(actor, 5)
			else
				if actor ~= wt_front_target_actor then
					actor_wt_attack(actor)
				else
					if math.random(0, 7) ~= 0 then
						actor_wt_attack(actor)
					else
						actor_move_towards_loc(actor, party_avg_x, party_avg_y)
					end
				end
			end
		end

		return
	end

	return
end

function actor_wt_front_1FB6E(actor)
   --dgb("actor_wt_front_1FB6E()\n")
   if (wt_num_monsters_near == 0 or wt_front_target_actor == nil) and actor.in_party == false then
      subtract_movement_pts(actor, 5)
      return 1
   end

   local player_loc = player_get_location()

   local centre_x,centre_y,diff_x,diff_y,var_1E,var_24,var_14,var_12,var_20,var_1C,var_10,target_x,target_y,var_E
   local actor_x = actor.x
   local actor_y = actor.y
   if actor.in_party == false then
      centre_x = party_avg_x
      centre_y = party_avg_y
      diff_x = wt_front_target_actor.x - centre_x
      diff_y = wt_front_target_actor.y - centre_y
   else
      centre_x = player_loc.x
      centre_y = player_loc.y
      diff_x = combat_avg_x - centre_x
      diff_y = combat_avg_y - centre_y
   end

   --dgb("actor_wt_front_1FB6E() actor = ("..actor_x..","..actor_y..") centre = ("..centre_x..","..centre_y..") player = ("..player_loc.x..","..player_loc.y..")\n")
   var_1E = (actor_x - centre_x) * diff_y - (actor_y - centre_y) * diff_x
   if var_1E <= 0 then
      var_24 = 0
   else
      var_24 = 1
   end

   if var_1E == 0 then
      var_24 = math.random(0, 1)
   end

   if diff_y <= 0 then
      var_14 = -1
   else
      var_14 = 1
   end

   if diff_x <= 0 then
      var_12 = 1
   else
      var_12 = -1
   end

   if var_24 == 0 then
      var_14 = -var_14
      var_12 = -var_12
   end

   local tmp_actor
   if actor.in_party == true then
      tmp_actor = Actor.get_player_actor()
   else
      tmp_actor = wt_front_target_actor
   end

   --dgb("tmp_actor = "..tmp_actor.name.." at ("..tmp_actor.x..","..tmp_actor.y..")\n")
   var_20 = (tmp_actor.x - centre_x) * diff_x + (tmp_actor.y - centre_y) * diff_y
   if actor.in_party == true then
      var_20 = var_20 + abs(diff_x) + abs(diff_y)
   end

   var_1C = diff_x * diff_x + diff_y * diff_y
   if var_1C == 0 then
      var_1C = 1
   end

   --dgb("getting target var_20 = "..var_20.." diff_x = "..diff_x.." diff_y = "..diff_y.." var_1C = "..var_1C.."\n")
   target_x = math.floor((var_20 * diff_x) / var_1C) + centre_x
   target_y = math.floor((var_20 * diff_y) / var_1C) + centre_y

   unk_30A72 = 0

   local chunk_x = player_loc.x - 16
   chunk_x = chunk_x - (chunk_x % 8)

   local chunk_y = player_loc.y - 16
   chunk_y = chunk_y - (chunk_y % 8)

   local found_actor = false
   repeat
      if target_x < chunk_x or chunk_x + 0x27 < target_x or target_y < chunk_y or chunk_y + 0x27 < target_y or (actor_x == target_x and actor_y == target_y) then
         unk_30A72 = 1
         --dgb("combat_front returned. too far away. actor=("..actor_x..","..actor_y..") target=("..target_x..","..target_y..") chunk=("..chunk_x..","..chunk_y..")\n")
         return 1
      end

      if map_get_actor(target_x, target_y, player_loc.z) ~= nil then
         found_actor = true
      else
         found_actor = false
      end

      if found_actor then
         var_10 = (target_x + var_14 - centre_x) * diff_x + (target_y - centre_y) * diff_y
         var_E = (target_x - centre_x) * diff_x + (target_y + var_12 - centre_y) * diff_y

         if abs(var_10 - var_20) >= abs(var_E - var_20) then
            target_y = target_y + var_12
         else
            target_x = target_x + var_14
         end
      end
   until found_actor == false


   local mpts = actor.mpts
   if actor_move_towards_loc(actor, target_x, target_y) ~= 0 then
      if actor.in_party == false or actor.x == target_x and actor.y == target_y then
         return 0
      end

      if wt_num_monsters_near == 0 then
         actor.mpts = mpts
      end

      if actor_move_towards_loc(actor, target_x, target_y) ~= 0 then
         return 0
      end
   else
      actor.mpts = mpts
   end

   actor_wt_attack(actor)

   return 0
end

function actor_wt_rear(actor)
   --dgb("actor_wt_rear()\n")
   local var_C = 0
   local player_loc = player_get_location()
   local var_2,var_4,avg_y,avg_x,dx,ax,avg_x_diff, avg_y_diff
   if actor.in_party == false then

      if wt_num_monsters_near == 0 then subtract_movement_pts(actor, 5) return end

      var_4 = wt_rear_min_monster
      var_2 = wt_rear_max_monster

      if var_4 == nil then var_4 = actor end
      if var_2 == nil then var_2 = actor end

      avg_x = combat_avg_x
      avg_y = combat_avg_y
      avg_x_diff = party_avg_x - combat_avg_x
      avg_y_diff = party_avg_y - combat_avg_y

   else

      if wt_num_monsters_near == 0 then actor_move_towards_loc(actor, player_loc.x, player_loc.y) return end

      var_4 = wt_rear_max_party
      var_2 = wt_rear_min_party
      avg_x = party_avg_x
      avg_y = party_avg_y
      avg_x_diff = combat_avg_x - party_avg_x
      avg_y_diff = combat_avg_y - party_avg_y
   end

   local var_10 = 0x7fff
   local align = actor.align
   local i, var_12
   for i=0,0xff do
      local a = Actor.get(i)

      if a.alive and a.wt == WT_FRONT and a.align == align then

         var_12 = (a.x - avg_x) * avg_x_diff + (a.y - avg_y) * avg_y_diff

         if var_12 < var_10 then var_10 = var_12 end

      end

   end

   var_12 = (actor.x - avg_x) * avg_x_diff + (actor.y - avg_y) * avg_y_diff
   local mpts = actor.mpts
   local var_C
   if actor.in_party == false or actor_find_max_xy_distance(actor, player_loc.x, player_loc.y) <= 3 then

      if var_12 < var_10 then

         var_12 = (actor.x - avg_x) * avg_y_diff - (actor.y - avg_y) * avg_x_diff

         dx = (var_4.x - avg_x) * avg_y_diff
         if dx - (var_4.y - avg_y) * avg_x_diff >= var_12 then

            dx = (var_2.x - avg_x) * avg_y_diff
            ax = (var_2.y - avg_y) * avg_x_diff
            if dx - ax <= var_12 then

               var_C = 1

            else

               ax = (actor_move_towards_loc(actor, actor.x + avg_y_diff, actor.y - avg_x_diff) and -1 or 0) + 1
               var_C = ax
            end

         else

            ax = (actor_move_towards_loc(actor, actor.x - avg_y_diff, actor.y + avg_x_diff) and -1 or 0) + 1
            var_C = ax
         end

      else

         ax = (actor_move_towards_loc(actor, actor.x - avg_x_diff, actor.y - avg_y_diff) and -1 or 0) + 1
         var_C = ax
      end

   else

      ax = (actor_move_towards_loc(actor, player_loc.x, player_loc.y) and -1 or 0) + 1
      var_C = ax
   end

   if var_C ~= 0 then

      actor.mpts = mpts
      actor_wt_attack(actor)
   end


end

function actor_wt_flank(actor)

   local player_loc = player_get_location()
   local player_x = player_loc.x
   local player_y = player_loc.y

   local random = math.random
   local abs = abs

   if wt_num_monsters_near == 0 or actor.in_party == false and actor_find_max_xy_distance(actor, player_x, player_y) > 7 then

      if actor.in_party == true and actor_find_max_xy_distance(actor, player_x, player_y) > 2 then
         local mpts = actor.mpts
         actor_move_towards_loc(actor, player_x, player_y)
         actor.mpts = mpts
      end

      subtract_movement_pts(actor, 5)
      return
   end

   local actor_align = actor.align
   local actor_x = actor.x
   local actor_y = actor.y
   local tmp_x = combat_avg_x - party_avg_x
   local tmp_y = combat_avg_y - party_avg_y
   local var_20 = (actor_x - party_avg_x) * tmp_y - (actor_y - party_avg_y) * tmp_x

   local var_1E, var_10, var_E

   if var_20 <= 0 then

      var_1E = 0
   else

      var_1E = 1
   end

   if var_20 == 0 then

      var_1E = random(0, 1)
   end
   if var_1E == 0 then

      var_10 = -tmp_y
      var_E = tmp_x
      var_20 = -var_20
   else

      var_10 = tmp_y
      var_E = -tmp_x
   end

   local var_1A = -0x8000
   local target_actor = nil

   local i, tmp_actor
   for i=1,0xff do
      tmp_actor = Actor.get(i)

      if tmp_actor ~= nil
       and tmp_actor.alive == true
       and tmp_actor.align ~= actor_align
       and actor_ok_to_attack(actor, tmp_actor) == true
       and (actor_align ~= ALIGNMENT_GOOD or alignment_is_evil(tmp_actor.align) == true)
       and (actor_align ~= ALIGNMENT_EVIL or tmp_actor.align == ALIGNMENT_GOOD or tmp_actor.align == ALIGNMENT_CHAOTIC) then

         local target_x = tmp_actor.x
         local target_y = tmp_actor.y

         if (tmp_actor.wt ~= WT_RETREAT and tmp_actor.wt ~= WT_FLEE and tmp_actor.wt ~= WT_MOUSE or player_x - 5 <= target_x and player_x + 5 >= target_x and player_y - 5 <= target_y and player_y + 5 >= target_y)
          and (abs(target_x - player_x) <= 7 and abs(target_y - player_y) <= 7 or abs(target_x - actor_x) <= 5 and abs(target_y - actor_y) <= 5) then

            local var_1C = (target_x - party_avg_x) * tmp_y - (target_y - party_avg_y) * tmp_x
            if var_1E == 0 then

               var_1C = -var_1C
            end

            if var_1C > var_1A then

               var_1A = var_1C
               target_actor = tmp_actor
            end
         end
      end
   end

   if target_actor == nil then
      actor_move_towards_player(actor)
      return
   end

   g_obj = target_actor
   tmp_x = target_actor.x
   tmp_y = target_actor.y

   local should_move_actor = false
   local weapon_obj = actor_get_weapon(actor, target_actor)
   local weapon_range = get_weapon_range(weapon_obj.obj_n)
   local attack_range = Actor.get_range(actor, tmp_x, tmp_y)

   if attack_range < 9 and attack_range <= weapon_range then

      if map_can_reach_point(actor_x, actor_y, tmp_x, tmp_y, actor.z) == false then
         if random(0, 1) == 0 then
            tmp_x = target_actor.y - actor_y + actor_x
            tmp_y = actor_y - target_actor.x - actor_x
         else
            tmp_x = actor_x - target_actor.y - actor_y
            tmp_y = target_actor.x - actor_x + actor_y
         end
         should_move_actor = true
      else
         actor_attack(actor, tmp_x, tmp_y, actor.z, weapon_obj, target_actor)
         subtract_movement_pts(actor, 10)
      end
   else

      if var_10 <= 0 then

         if var_10 < 0 then

            tmp_x = tmp_x - 1
         end
      else

         tmp_x = tmp_x + 1
      end
      if var_E <= 0 then
         if var_E < 0 then
            tmp_y = tmp_y - 1
         end
      else
         tmp_y = tmp_y + 1
      end
      should_move_actor = true
   end

   if should_move_actor == true then

      local mpts = actor.mpts
      if actor_move_towards_loc(actor, tmp_x, tmp_y) == 0 then

         actor.mpts = mpts
         actor_wt_attack(actor)
      end
   end

   return
end


function actor_wt_berserk(actor)
   local target_actor = nil
   local alignment = actor.align
   local actor_x = actor.x
   local actor_y = actor.y
   local max_stats = 0

   local i
   for i=1,0xff do
      local tmp_actor = Actor.get(i)

      if tmp_actor ~= nil
       and tmp_actor.alive == true
       and tmp_actor.actor_num ~= actor.actor_num
       and actor_find_max_xy_distance(tmp_actor, actor_x, actor_y) <= 8
       and actor_ok_to_attack(actor, tmp_actor) == true
       and (alignment ~= ALIGNMENT_GOOD or tmp_actor.align == ALIGNMENT_EVIL)
       and (alignment ~= ALIGNMENT_EVIL or tmp_actor.align == ALIGNMENT_GOOD)
       and (alignment ~= ALIGNMENT_CHAOTIC or (tmp_actor.align ~= ALIGNMENT_CHAOTIC and tmp_actor.align ~= ALIGNMENT_NEUTRAL)) then
         local combined_stats = tmp_actor.str + tmp_actor.dex + tmp_actor.int
         if combined_stats > max_stats then
            max_stats = combined_stats
            target_actor = tmp_actor
         end
      end
   end

   if target_actor == nil then
      actor_wt_attack(actor)
      subtract_movement_pts(actor, 5)
      return
   end

   local target_x = target_actor.x
   local target_y = target_actor.y
   local weapon_obj = actor_get_weapon(actor, target_actor)
   local weapon_range = get_weapon_range(weapon_obj.obj_n)
   local attack_range = Actor.get_range(actor, target_x, target_y)

   if attack_range < 9 and attack_range <= weapon_range then

      --g_obj = target_actor
      if map_can_reach_point(actor_x, actor_y, target_x, target_y, actor.z) == false then

         if math.random(0, 1) == 0 then
            target_x = target_actor.y - actor_y + actor_x
            target_y = actor_y - target_actor.x - actor_x
         else
            target_x = actor_x - target_actor.y - actor_y
            target_y = target_actor.x - actor_x + actor_y
         end
      else
         actor_attack(actor, target_x, target_y, actor.z, weapon_obj, target_actor)
         subtract_movement_pts(actor, 15)
         return
      end

   end

   local mpts = actor.mpts
   if actor_move_towards_loc(actor, target_x, target_y) == 0 then
      actor.mpts= mpts
      actor_wt_attack(actor)
   end

   subtract_movement_pts(actor, 5)
   return
end

function actor_wt_guard_arrest_player(actor)
	if actor_catch_up_to_party(actor) == true then
		caught_by_guard(actor)
	end
end

function actor_wt_combat_tanglevine(actor)
   local random = math.random
   local di
   local abs = abs

   local target = actor_find_target(actor)
   if target ~= nil then

      local target_x = target.x
      local target_y = target.y
      local actor_x = actor.x
      local actor_y = actor.y
      if abs(target_x - actor_x) < 2 and abs(target_y - actor_y) < 2 and random(0, 1) ~= 0 then

         actor_attack(actor, target_x, target_y, actor.z, actor, target)
         subtract_movement_pts(actor, 10)
         return
      end

      if abs(target_x - actor_x) < 5 and abs(target_y - actor_y) < 5 and random(0, 3) == 0 then

         target_x = target_x - actor_x
         target_y = target_y - actor_y

         if abs(target_x) <= abs(target_y) then

            di = (target_y <= 0) and DIR_NORTH or DIR_SOUTH

            if move_tanglevine(actor, di) == 0  then

               di = (target_x <= 0) and DIR_WEST or DIR_EAST
               move_tanglevine(actor, di)
               return
            end

         else

            di = (target_x <= 0) and DIR_WEST or DIR_EAST

            if move_tanglevine(actor, di) == 0 then

               di = (target_y <= 0) and DIR_NORTH or DIR_SOUTH
               move_tanglevine(actor, di)
               return
            end
         end
      end
   end

   if random(0, 3) == 0 then

      di = random(0, 3) --random direction north south east west
      if actor.direction == di then

         di = direction_reverse(di)
      end

      move_tanglevine(actor, di)

   else
      actor.mpts = 0
   end

   return
end


function actor_wt_combat_stationary(actor)
   local rand = math.random
   local align = actor.align
   if align == ALIGNMENT_NEUTRAL then

      subtract_movement_pts(actor, 5)
      return
   end

   local i
   for i=1,0x10 do

      local target_x = rand(0, 8) + actor.x - 4
      local target_y = rand(0, 8) + actor.y - 4
--      local g_obj = sub_1D351(actor, target_x, target_y)
      local target_actor = map_get_actor(target_x, target_y, actor.z)

      if target_actor ~= nil and actor_ok_to_attack(actor, target_actor) == true and target_actor.alive == true and target_actor.align ~= align and target_actor.align ~= ALIGNMENT_NEUTRAL then

         actor_attack(actor, target_x, target_y, actor.z, actor_get_weapon(actor, target_actor), target_actor)
         subtract_movement_pts(actor, 10)
         return
      end

   end

   subtract_movement_pts(actor, 5)

   return
end

function actor_wt_walk_straight(actor)
   if math.random(0, 1) == 0 then subtract_movement_pts(actor, 5) return end

   local wt = actor.wt
   local sched = actor.sched_loc
   local dir

   if wt < WT_WALK_NORTH_SOUTH or actor.x ~= sched.x or actor.y ~= sched.y then
      dir = actor.direction
   else
      if wt == WT_WALK_NORTH_SOUTH then dir = DIR_NORTH end
      if wt == WT_WALK_EAST_WEST then dir = DIR_EAST end
      if wt == WT_WALK_SOUTH_NORTH then dir = DIR_SOUTH end
      if wt == WT_WALK_WEST_EAST then dir = DIR_WEST end
      actor.direction = dir
   end

   local mpts = actor.mpts

   if actor_move(actor, dir, 1) == 0 then

      dir = direction_reverse(dir)

      actor.mpts = mpts
      actor_move(actor, dir, 1)
      actor.direction = dir
   end

end

function actor_wt_wander_around(actor)
   local rand = math.random
   if rand(0, 7) ~= 0 then subtract_movement_pts(actor, 5) return end

   local random_wander_range = function ()
      local i = 0
      while rand(0, 1) ~= 0 do i = i + 1 end
      if rand(0, 1) ~= 0 then i = -i end
      return i
   end

   local abs=abs
   local sched = actor.sched_loc
   local sched_x_offset = actor.x - sched.x;
   local sched_y_offset = actor.y - sched.y;
   local direction

   if abs(sched_y_offset) - abs(sched_x_offset) >= random_wander_range() then
      direction = (random_wander_range() <= sched_y_offset) and DIR_NORTH or DIR_SOUTH
   else
      direction = (random_wander_range() <= sched_x_offset) and DIR_WEST or DIR_EAST
   end

   actor.direction = direction
   actor_move(actor, direction, 1)

   return
end

function actor_move_towards_player(actor)
local rand = math.random
if actor.wt ~= WT_STATIONARY then
   if actor.in_party == true then
      local player_loc = player_get_location()
      local x,y = player_loc.x, player_loc.y
      if actor.x ~= x and actor.y ~= y and rand(0, 3) == 0 then
         actor_move_towards_loc(actor, x, y)
         return
      end
   end

   if rand(0, 7) == 0 then
      actor_move(actor, rand(0, 3), 1)
      return
   end
end
subtract_movement_pts(actor, 5)
end

function actor_wt_attack(actor)
--dgb("actor_wt_attack()\n");

   g_obj = actor_find_target(actor)
   if g_obj ~= nil then
   	--dgb("target at ("..g_obj.x..","..g_obj.y..")\n")
   end

   local weapon_obj = actor_get_weapon(actor, g_obj)

   if g_obj ~= nil then

      local target_x = g_obj.x
      local target_y = g_obj.y
      local actor_x = actor.x
      local actor_y = actor.y
      local weapon_range = get_weapon_range(weapon_obj.obj_n)

      if abs(target_x - actor_x) < 8 and abs(target_y - actor_y) < 8 and
       Actor.get_range(actor, target_x, target_y) <= weapon_range then

         if sub_1D59F(actor, target_x, target_y, weapon_range, 0) == true then
            actor_attack(actor, g_obj.x, g_obj.y, g_obj.z, weapon_obj, g_obj)
            subtract_movement_pts(actor, 10)
            return
         end

         if math.random(0, 1) == 0 then

            target_x = g_obj.y - actor_y + actor_x
            target_y = actor_y - g_obj.x - actor_x

         else

            target_x = actor_x - g_obj.y - actor_y
            target_y = g_obj.x - actor_x + actor_y
         end

         actor_move_towards_loc(actor, target_x, target_y)
         return
      end

      if actor.wt ~= 3 then actor_move_towards_loc(actor, target_x, target_y) return end

      subtract_movement_pts(actor, 5)
      return
   end

   if actor.wt ~= 3 then actor_move_towards_player(actor) return end


   subtract_movement_pts(actor, 5)

end

function actor_wt_timid(actor)
	local player_loc = player_get_location()
	if actor_find_max_xy_distance(actor, player_loc.x, player_loc.y) > 7 then
	   actor.mpts = 0
	   return
	end
	local target = actor_find_target(actor)
	if target == nil then
	   actor_move_towards_player(actor)
	   return
	end
	local diff_x = target.x - actor.x
	local diff_y = target.y - actor.y
	local var_4
	if actor.wt ~= WT_RETREAT then
	   var_4 = 3
	else
	   var_4 = 8
	end

	if abs(diff_x) < var_4 and abs(diff_y) < var_4 then
	   local var_2 = actor.mpts
	   if actor_move_towards_loc(actor, actor.x - diff_x, actor.y - diff_y) == 0 then
	      --original logic -- actor.mpts = var_2
	      if actor.wt ~= WT_MOUSE and actor.wt ~= WT_FLEE then
	         actor.mpts = var_2 --moved here.
	         actor_wt_attack(actor)
	      end
	   end
	else
	   if actor.align == ALIGNMENT_GOOD and actor.wt ~= WT_FLEE and actor.wt ~= WT_MOUSE then
	      actor_move_towards_loc(actor, party_avg_x, party_avg_y)
	   else
	      actor_move_towards_player(actor)
	   end
	end

	if actor.wt == WT_RETREAT then
	   if math.random(0, 3) == 0 then
	      if actor.level > actor.hp then
	         actor.hp = actor.hp + 1
	      end

	      if math.floor((actor.hp * 4) / actor.level) > 0 then
	         actor.wt = actor.combat_mode
	      end
	   end
	end
end

function actor_wt_like(actor)
	local actor_x = actor.x
	local actor_y = actor.y
	local party_actor
	local random = math.random

	for party_actor in party_members() do

	   if abs(party_actor.x - actor_x) < 3 and abs(party_actor.y - actor_y) < 3 then
	      if random(0, 1) == 0 then
	         actor_move_towards_loc(actor, party_avg_x, party_avg_y)
	         return
	      end

	      break
	   end
	end

	if random(0, 1) == 0 then
	   subtract_movement_pts(actor, 5)
	else
	   if random(0, 1) == 0 then
	      if random(0, 1) == 0 then
	         actor_y = actor_y - 10
	      else
	         actor_y = actor_y + 10
	      end
	   else
	      if random(0, 1) == 0 then
	         actor_x = actor_x - 10
	      else
	         actor_x = actor_x + 10
	      end
	   end
	   actor_move_towards_loc(actor, actor_x, actor_y)
	end

	return
end

function actor_wt_unfriendly(actor)
	local actor_x = actor.x
	local actor_y = actor.y
	local party_actor
	local random = math.random

	for party_actor in party_members() do
	   if abs(party_actor.x - actor_x) < 3 and abs(party_actor.y - actor_y) < 3 then

	      if random(0, 7) == 0 then
	         actor.wt = WT_ASSAULT
	         actor_wt_attack(actor)
	         return
	      end

	      break
	   end
	end

	if random(0, 1) == 0 then
	   subtract_movement_pts(actor, 5)
	else
	   if random(0, 1) == 0 then
	      if random(0, 1) == 0 then
	         actor_y = actor_y - 10
	      else
	         actor_y = actor_y + 10
	      end
	   else
	      if random(0, 1) == 0 then
	         actor_x = actor_x - 10
	      else
	         actor_x = actor_x + 10
	      end
	   end
	   actor_move_towards_loc(actor, actor_x, actor_y)
	end

	return
end

function actor_wt_walk_to_location(actor)
	Actor.walk_path(actor)
	if Actor.is_at_scheduled_location(actor) then
		actor.wt = actor.sched_wt
		if actor.wt == WT_SLEEP then
			-- add extinguish light source code if actors with a light source actually go to sleep
			actor_wt_sleep(actor)
		end
	end

	subtract_movement_pts(actor, 5) --FIXME what's the movement cost for pathfinding?
end

function actor_wt_sleep(actor)
--dgb("actor_wt_sleep("..actor.name..") at "..actor.x..","..actor.y..","..actor.z.."\n")
	local obj = map_get_obj(actor.x, actor.y, actor.z, 0xa3) --bed
	if obj ~= nil then
		if obj.frame_n == 1 or obj.frame_n == 5 then --horizontal bed
			actor.obj_n = 0x92 -- person sleeping
			actor.frame_n = 0
		else
			if obj.frame_n == 7 or obj.frame_n == 10 then --vertical bed
				actor.obj_n = 0x92 -- person sleeping
				actor.frame_n = 1
			end
		end
	end
end

function actor_wt_beg(actor)
	local player_loc = player_get_location()
	if actor.z == player_loc.z and abs(actor.x - player_loc.x) < 5 or abs(actor.y - player_loc.y) < 5 then
		if actor_catch_up_to_party(actor) == true then
			actor.wt = WT_WORK
			Actor.talk(actor)
		end
	else
		actor_wt_wander_around(actor)
	end
end

function actor_wt_brawling(actor)
	if math.random(0, 3) == 0 then
		local target = actor_find_target(actor)
		local player_loc = player_get_location()
		if target ~= nil and actor_find_max_xy_distance(target, player_loc.x, player_loc.y) < 5 then
			if actor_find_max_xy_distance(actor, target.x, target.y) < 2 then
				hit_anim(target.x, target.y)
				actor_move_towards_loc(actor, player_loc.x, player_loc.y)
			else
				actor_move_towards_loc(actor, target.x, target.y)
			end

			return
		end
	end

	actor_wt_wander_around(actor)
end

wt_tbl = {
[WT_NOTHING] = {"WT_NOTHING", perform_worktype},
[WT_FRONT] = {"WT_FRONT", actor_wt_front},
[WT_REAR] = {"WT_REAR", actor_wt_rear},
[WT_FLANK] = {"WT_FLANK", actor_wt_flank},
[WT_BERSERK] = {"WT_BERSERK", actor_wt_berserk},
[WT_RETREAT] = {"WT_RETREAT", actor_wt_timid},
[WT_ASSAULT] = {"WT_ASSAULT", actor_wt_attack},
[WT_FLEE] = {"WT_FLEE", actor_wt_timid},
[WT_LIKE] = {"WT_LIKE", actor_wt_like},
[WT_UNFRIENDLY] = {"WT_UNFRIENDLY", actor_wt_unfriendly},
[WT_WANDER_NEAR_PLAYER] = {"WT_WANDER_NEAR_PLAYER", actor_move_towards_player},
[WT_TANGLE] = {"WT_TANGLE", actor_wt_combat_tanglevine},
[WT_STATIONARY] = {"WT_STATIONARY", actor_wt_combat_stationary},
[WT_GUARD_WALK_EAST_WEST] = {"WT_GUARD_WALK_EAST_WEST", actor_wt_walk_straight},
[WT_GUARD_WALK_NORTH_SOUTH] = {"WT_GUARD_WALK_NORTH_SOUTH", actor_wt_walk_straight},
--11
[WT_GUARD_ARREST_PLAYER] = {"WT_GUARD_ARREST_PLAYER", actor_wt_guard_arrest_player},
[WT_UNK_13] = {"WT_UNK_13", actor_wt_timid},
[WT_WALK_TO_LOCATION] = {"WT_WALK_TO_LOCATION", actor_wt_walk_to_location},
[WT_WALK_NORTH_SOUTH] = {"WT_WALK_NORTH_SOUTH", actor_wt_walk_straight},
[WT_WALK_EAST_WEST]   = {"WT_WALK_EAST_WEST", actor_wt_walk_straight},
[WT_WALK_SOUTH_NORTH] = {"WT_WALK_SOUTH_NORTH", actor_wt_walk_straight},
[WT_WALK_WEST_EAST]   = {"WT_WALK_WEST_EAST", actor_wt_walk_straight},
[WT_WANDER_AROUND] = {"WT_WANDER_AROUND", actor_move_towards_player},
[WT_WORK] = {"WT_WORK", actor_wt_wander_around},
[WT_UNK_94] = {"WT_UNK_94", actor_wt_wander_around},
[WT_BEG] = {"WT_BEG", actor_wt_beg},
[WT_ATTACK_PARTY] = {"WT_ATTACK_PARTY", actor_wt_attack},
[WT_BRAWLING] = {"WT_BRAWLING", actor_wt_brawling},
[WT_MOUSE] = {"WT_MOUSE", actor_wt_timid}

--[WT_] = {"WT_", actor_wt_rear}
}

function actor_ok_to_attack(actor, target_actor)

   if target_actor.visible == false and (target_actor.in_party == false or actor.in_party == false) then return false end

   if target_actor.z ~= actor.z then return false end

   if target_actor.obj_n == 0x165 and target_actor.frame_n == 0 then return false end --corpser underground

   if target_actor.corpser_flag == true then return false end

   --FIXME need to check tileflag3 bit 4 is not set. The Ignore flag.
   return true

end

function actor_find_target(actor)

   local target_actor = nil
   local align = actor.align
   local actor_x = actor.x
   local actor_y = actor.y
   local var_2 = 0x7fff
   local i
   local player_loc = player_get_location()
   local player_x = player_loc.x
   local player_y = player_loc.y

   for i=0,0xff do

      local tmp_actor = Actor.get(i)

      if tmp_actor.obj_n ~= 0 and tmp_actor.alive == true and tmp_actor.actor_num ~= actor.actor_num and actor_ok_to_attack(actor, tmp_actor) == true then

         if actor.wt == WT_FLEE or
            actor.wt == WT_MOUSE or
            actor.wt == WT_UNK_13 or
            actor.wt == WT_RETREAT or
            actor.wt == WT_BRAWLING or
            (align ~= ALIGNMENT_NEUTRAL or actor.wt == WT_ATTACK_PARTY and tmp_actor.align == ALIGNMENT_GOOD) and
            (align ~= ALIGNMENT_CHAOTIC or tmp_actor.align ~= ALIGNMENT_CHAOTIC) and
            tmp_actor.align ~= ALIGNMENT_NEUTRAL and
            (align ~= ALIGNMENT_GOOD or alignment_is_evil(tmp_actor.align) == true) and
            (align ~= ALIGNMENT_EVIL or tmp_actor.align == ALIGNMENT_GOOD or tmp_actor.align == ALIGNMENT_CHAOTIC) then

            local target_x = tmp_actor.x
            local target_y = tmp_actor.y

            if actor_find_max_xy_distance(actor, target_x, target_y) <= 8 and (tmp_actor.wt ~= WT_RETREAT or abs(target_x - player_x) <= 5 and abs(target_y - player_y) <= 5) then

               local var_6 = (target_x - actor_x)^2 + (target_y - actor_y)^2
               if var_6 < var_2 or var_6 == var_2 and math.random(0, 1) ~= 0 then

                  var_2 = var_6
                  target_actor = tmp_actor
               end
            end
         end
      end

   end

   return target_actor
end

--FIXME this is a line of sight check for combat.
function sub_1D59F(actor, target_x, target_y, weapon_obj)
   return map_can_reach_point(actor.x, actor.y, target_x, target_y, actor.z)
end



function perform_worktype(actor)

   if g_time_stopped == true then
      actor.mpts = 0
      return
   end

   if wt_tbl[actor.wt] == nil then
      subtract_movement_pts(actor, 5)
      return
   end

   --dgb("wt = "..wt_tbl[actor.wt][1].."\n")

   if actor.mpts > 0 then
   	local func = wt_tbl[actor.wt][2]
   	func(actor)
   end

   if actor.mpts == 0 then subtract_movement_pts(actor, 0xa) end

end

function spell_put_actor_to_sleep(attacker, foe)

   --dgb("spell_put_actor_to_sleep("..attacker.name..",foe)\n")

   local actor_base = actor_tbl[foe.obj_n]
   if actor_base == nil or actor_base[21] == 0 then -- 21 is immune to sleep
      if actor_int_check(foe, attacker) == false then
         hit_anim(foe.x, foe.y)
         actor_put_to_sleep(foe)
         return 0xfe
      else
         return 1
      end
   end

   return 2
end

function spell_poison_actor(attacker, foe)
	local actor_base = actor_tbl[foe.obj_n]
	if actor_base == nil or actor_base[19] == 1 or foe.actor_num == 0 then return 2 end --immune to poison

	if math.floor((math.floor(actor_str_adj(foe) / 2) + 0x1e - actor_int_adj(attacker)) / 2) <= math.random(1, 0x1e) then
		foe.poisoned = true
		hit_anim(foe.x, foe.y)
		return -1
	end

	return 1
end

function spell_take_fire_dmg(attacker, foe)
   local actor_base = actor_tbl[foe.obj_n]
   if actor_base == nil or actor_base[18] == 1 then return end --immune to magic

   local dmg = math.random(1, 0x14)

   if actor_base == nil or actor_base[17] == 1 then dmg = dmg * 2 end --double dmg from fire

   local exp = actor_hit(foe, dmg)
   if exp ~= 0 then
      attacker.exp = attacker.exp + exp
   end

   actor_yell_for_help(attacker, foe, 1)
   actor_hit_msg(foe)
end

function spell_charm_actor(attacker, foe)
	if actor_int_check(foe, attacker) == true then return false end

	if foe.charmed == true then
		actor_remove_charm(foe)
	else
		foe.charmed = true
		foe.old_align = foe.align
		foe.align = attacker.align
		hit_anim(foe.x, foe.y)
		print("\n"..foe.name.." is charmed.\n")
		if foe.in_party == true then
			party_update_leader()
		end
	end

	return true
end

function spell_kill_actor(attacker, foe)
	local actor_base = actor_tbl[foe.obj_n]
	if actor_base ~= nil and actor_base[20] == 1 then return 2 end --immune to corp spells

	if actor_int_check(foe, attacker) == true then return 1 end

	local exp = actor_hit(foe, foe.hp)
	if exp ~= 0 then
		attacker.exp = attacker.exp + exp
	end

	actor_yell_for_help(attacker, foe, 1)
	actor_hit_msg(foe)

	return -1
end

function spell_hit_actor(attacker, foe, spell_num)

	if actor_int_check(foe, attacker) == true then return false end
	local random = math.random
	local dmg = 0

	if spell_num == 5 then
		dmg = random(1, 0xa)
	elseif spell_num == 0x62 then
		dmg = random(1, 0x1e)
	elseif spell_num == 0x32 then
		dmg = foe.hp - 1
	end

	print("\n")

	local exp = actor_hit(foe, dmg)
	if exp ~= 0 then
		attacker.exp = attacker.exp + exp
	end

	actor_hit_msg(foe)

	actor_yell_for_help(attacker, foe, 1)

	return true
end

function actor_use_effect(actor, effect)
	local random = math.random
	local effect_type = random(0, 3)
    Obj.removeFromEngine(effect) -- moved here so it won't be seen in chest
	if effect_type == 0 then
		print("Acid!\n")
		actor_hit(actor, random(1, 0x14))

	elseif effect_type == 1 then
		print("Poison!\n")
		actor.poisoned = true
		hit_anim(actor.x, actor.y)

	elseif effect_type == 2 then
		print("Bomb!\n")
		local hit_items = explosion(0x17e, actor.x, actor.y)

		for k,v in pairs(hit_items) do
			if v.luatype == "actor" then
				actor_hit(v, random(1, 0x14))
			end
		end

	elseif effect_type == 3 then
		print("Gas!\n")
		local hit_items = explosion(0x17c, actor.x, actor.y)

		for k,v in pairs(hit_items) do
			if v.luatype == "actor" then
				v.poisoned = true
				hit_anim(v.x, v.y)
			end
		end
	end

end

function actor_resurrect(actor)
	local karma = player_get_karma()
	if karma < 98 then
		actor.exp = math.floor((actor.exp * karma) / 100)
	end
end

function get_LB_to_throne()
	local cur_hour = clock_get_hour()
-- LB is on throne at 8-12 and 14-18
	while cur_hour < 8 or cur_hour > 17 or (cur_hour > 11 and cur_hour < 14) do
		advance_time(60)
		cur_hour = clock_get_hour()
	end
end

function actor_avatar_death()

	--FIXME the hit tile is displayed constantly while the death tune is playing.
	g_avatar_died = false -- before get_LB_to_throne()
	local avatar = Actor.get(1)
	if avatar.obj_n == 431 then --horse with rider
		Actor.use(avatar) --dismount from horse
	end
	avatar.asleep = true --we do this so it looks like the avatar is dead.
	print("\nAn unending darkness engulfs thee...\n\n")
	play_sfx(SFX_AVATAR_DEATH, true)
	fade_out()
	print("A voice in the darkness intones, \"KAL LOR!\"\n")
	play_sfx(SFX_KAL_LOR, true)
	avatar.asleep = false
	party_dismount_from_horses();
	if g_armageddon == false then
		party_resurrect_dead_members()
	end

	--unready inventory objects.
	for obj in actor_inventory(avatar) do
		if obj.readied and obj.obj_n ~= 76 then -- don't display message for amulet of submission
				Actor.inv_unready_obj(avatar, obj)
		end
	end
	party_heal() -- need to heal before player_move
	party_update_leader()
	player_move(0x133, 0x160, 0, true)
	party_exit_vehicle(0x133, 0x160, 0)
	get_LB_to_throne()

	for i=0,0xff do
		local actor = Actor.get(i)
		actor.mpts = 0
		if actor.temp == false and actor.wt == WT_ATTACK_PARTY then
			actor.wt = WT_WALK_TO_LOCATION
		end
	end
	avatar.mpts=1

	actor_resurrect(avatar)
	party_set_combat_mode(false)
	party_set_party_mode()
	fade_in()
end

--io.stderr:write("actor.lua loaded\n")
