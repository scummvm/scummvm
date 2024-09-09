--io.stderr:write("actor.lua get here\n")

--Worktypes
WT_NOTHING                = 0x0  --do nothing

WT_FOLLOW                 = 0x1  --follow avatar (in party)

WT_PLAYER                 = 0x2  --player mode

WT_SLEEP                  = 0x91

ACTOR_STAT_DMG = 7
--Actor stats table
--[obj_num] = {str,dex,int,hp,alignment,,damage??,,,,,,,,,,,,,,,,}
actor_tbl = {
--OBJ_YOURSELF5
[343] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_YOURSELF6
[344] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_POOR_MONK
[342] = {22,22,22,60,ALIGNMENT_EVIL,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_SCIENTIST
[345] = {15,15,25,30,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_YOUNG_WOMAN
[346] = {18,18,22,60,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_COWBOY
[347] = {20,25,18,60,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_GENTLEMAN
[348] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_UNIFORMED_MAN
[349] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_ADVENTURER
[350] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_COMMON_FELLOW
[351] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_MAN
[352] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_MAN_IN_WHITE
[353] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_LADY
[354] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_WOMAN
[355] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_ADVENTURESS
[356] = {25,25,25,90,ALIGNMENT_GOOD,0,6,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_METAL_WOMAN1
[357] = {25,25,25,90,ALIGNMENT_GOOD,0,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_MECHANICAL_MAN
[358] = {25,15,10,40,ALIGNMENT_GOOD,4,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_MARTIAN
[369] = {15,15,20,30,ALIGNMENT_NEUTRAL,0,6,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_SEXTELLEGER
[359] = {20,20,10,45,ALIGNMENT_CHAOTIC,4,35,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_BUSHALO
[360] = {25,12,11,35,ALIGNMENT_NEUTRAL,8,20,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0},
--OBJ_PLANTELLOPE
[361] = {15,22,9,20,ALIGNMENT_NEUTRAL,1,12,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_ROCKWORM2
[372] = {20,15,10,40,ALIGNMENT_CHAOTIC,8,25,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_WISP
[377] = {20,20,20,80,ALIGNMENT_NEUTRAL,3,30,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0},
--OBJ_AIRSQUID
[378] = {10,20,15,10,ALIGNMENT_EVIL,0,6,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0},
--OBJ_AIRSQUID1
[380] = {15,15,8,30,ALIGNMENT_EVIL,4,20,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_GIANT_MAW
[373] = {30,10,9,100,ALIGNMENT_CHAOTIC,3,30,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_TENTACLE1
[362] = {20,20,9,10,ALIGNMENT_CHAOTIC,2,20,1,1,1,0,0,0,0,0,1,0,1,0,0,0,0,0},
--OBJ_CREEPING_CACTUS
[383] = {20,12,3,30,ALIGNMENT_CHAOTIC,4,25,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_AMMONOID
[363] = {15,15,3,20,ALIGNMENT_CHAOTIC,10,12,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_PROTO_MARTIAN
[364] = {20,19,10,20,ALIGNMENT_EVIL,2,10,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0},
--OBJ_MONSTER_FOOTPRINTS
[145] = {20,19,10,20,ALIGNMENT_EVIL,2,12,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0},
--OBJ_POD_DEVIL
[384] = {15,18,3,25,ALIGNMENT_CHAOTIC,4,12,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_JUMPING_BEAN
[385] = {10,22,10,10,ALIGNMENT_CHAOTIC,1,4,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_HEDGEHOG
[365] = {17,12,8,20,ALIGNMENT_CHAOTIC,3,20,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_CREEPER
[374] = {15,15,3,20,ALIGNMENT_CHAOTIC,1,16,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_AGROBOT
[376] = {29,18,10,30,ALIGNMENT_CHAOTIC,10,20,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
--OBJ_CANAL_WORM
[379] = {20,20,15,30,ALIGNMENT_CHAOTIC,5,25,0,0,0,1,0,0,1,0,1,0,0,0,0,0,0,0},
--OBJ_DUST_DEVIL
[381] = {30,30,30,255,ALIGNMENT_NEUTRAL,99,60,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
--OBJ_SAND_TRAPPER
[386] = {25,25,10,40,ALIGNMENT_CHAOTIC,4,18,1,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0},
--OBJ_OXY_LEECH
[375] = {10,25,12,20,ALIGNMENT_CHAOTIC,1,10,0,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_CAVE_WORM
[366] = {15,15,15,30,ALIGNMENT_EVIL,2,15,0,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_JANITOR
[367] = {20,15,10,30,ALIGNMENT_CHAOTIC,10,20,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
--OBJ_BUSHRAT
[387] = {10,25,10,20,ALIGNMENT_EVIL,2,12,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_PLANTHER
[368] = {15,22,12,20,ALIGNMENT_CHAOTIC,4,20,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_MINOTAUR
[398] = {20,18,8,90,ALIGNMENT_NEUTRAL,2,15,0,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0},
--OBJ_YOUR_MOTHER
[391] = {12,20,15,255,ALIGNMENT_CHAOTIC,9,1,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0},
--OBJ_TREE
[408] = {25,18,5,90,ALIGNMENT_NEUTRAL,3,12,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_SOMETHING_YOU_SHOULDNT2
[424] = {25,18,5,90,ALIGNMENT_NEUTRAL,3,12,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
--OBJ_HUGE_RED_SPOT
[403] = {20,20,10,255,ALIGNMENT_EVIL,99,12,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}

function  is_actor_stat_bit_set(obj_n, bit_number)
   local stats = actor_tbl[obj_n]
   if stats ~= nil then
      if stats[23 - bit_number] == 1 then
         return true
      end
   end

   return false
end

g_party_is_warm = false

-- Berry logic
function actor_is_affected_by_purple_berries(actor_num)

   if actor_num < 16 and timer_get(actor_num*3) > 0 then
      return true
   end

   return false
end

function actor_is_affected_by_green_berries(actor_num)

   if actor_num < 16 and timer_get(actor_num*3+1) > 0 then
      return true
   end

   return false
end

function actor_is_affected_by_brown_berries(actor_num)

   if actor_num < 16 and timer_get(actor_num*3+2) > 0 then
      return true
   end

   return false
end

function actor_get_purple_berry_count(actor_num)
   if actor_num < 16 then
      return timer_get(actor_num*3)
   end

   return 0
end

function actor_get_green_berry_count(actor_num)
   if actor_num < 16 then
      return timer_get(actor_num*3+1)
   end

   return 0
end

function actor_get_brown_berry_count(actor_num)
   if actor_num < 16 then
      return timer_get(actor_num*3+2)
   end

   return 0
end

function actor_decrement_berry_counter(actor, berry_type)
   local actor_num = actor.actor_num
   local count = timer_get(actor_num*3+berry_type)
   if count > 0 and math.random(1, actor_int_adj(actor) * 4) == 1 then
      timer_set(actor_num*3+berry_type, count - 1)
      if (actor_num == 0 and g_in_dream_mode) or
         (actor_num ~= 0 and g_in_dream_mode == false) then
         play_md_sfx(0x32)
         printl("A_PSYCHIC_POWER_FADES")
      end
   end
end

function actor_decrement_berry_counters(actor)
   actor_decrement_berry_counter(actor, 0) --purple
   actor_decrement_berry_counter(actor, 1) --green
   actor_decrement_berry_counter(actor, 2) --brown
end

function actor_increment_berry_counter(actor_num, berry_type)
   local count = timer_get(actor_num*3+berry_type)
   if actor_num < 16 and count < 10 then
      timer_set(actor_num*3+berry_type, count + 1)
   end
end

function actor_increment_purple_berry_count(actor_num)
   actor_increment_berry_counter(actor_num, 0)
end

function actor_increment_green_berry_count(actor_num)
   actor_increment_berry_counter(actor_num, 1)
end

function actor_increment_brown_berry_count(actor_num)
   actor_increment_berry_counter(actor_num, 2)
end

function actor_get_blue_berry_counter()
   return timer_get(16*3)
end

function actor_set_blue_berry_counter(new_value)
   timer_set(16*3, new_value)
end

function actor_clear_berry_counters(actor_num)
   timer_set(actor_num*3, 0)
   timer_set(actor_num*3+1, 0)
   timer_set(actor_num*3+2, 0)
end

function actor_get_damage(actor)
   local stat = actor_tbl[actor]
   if stat ~= nil then
      return stat[ACTOR_STAT_DMG]
   end

   return nil
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
   actor.hp = actor_randomise_stat(actor_base[4])

   actor.level = 0
   actor.align = actor_base[5]
 else
   actor.str = 15
   actor.dex = 15
   actor.int = 15
   actor.hp = 30
   actor.level = 1
   actor.align = ALIGNMENT_CHAOTIC
 end

 if alignment ~= nil and alignment ~= ALIGNMENT_DEFAULT then
   actor.align = alignment
 end

 actor.wt = 8
 actor.combat_mode = 8
 actor.mpts = actor.dex
 actor.exp = 0

end


function subtract_map_movement_pts(actor)
   local points = map_get_impedence(actor.x, actor.y, actor.z, false) + 5
   subtract_movement_pts(actor, points)
end

function actor_divide(actor)

   local random = math.random
   local from_x, from_y, from_z = actor.x, actor.y, actor.z

   for i=1,8 do

      local new_x = wrap_coord(random(1, 2) + from_x - 1, from_z)
      local new_y = wrap_coord(random(1, 2) + from_y - 1, from_z)

      if map_can_put(new_x, new_y, from_z) then

         --FIXME need to call sub_2C657 which I think updates the existing actor
         Actor.clone(actor, new_x, new_y, from_z)
         printfl("ACTOR_DIVIDES", actor.name)
         return
      end
   end
end

function defender_update_alignment(attacker, defender, damage)
   if defender.luatype ~= "actor" then
      return
   end

   if defender.align == ALIGNMENT_NEUTRAL and defender.in_party == false and (defender.wt <= 0x10 or defender.wt >= 0x80) then
      if attacker.wt == WT_PLAYER and is_actor_stat_bit_set(defender.obj_n, 14) and damage > 0 then
         printfl("ACTOR_ATTACKS", defender.name)
      end

      if defender.actor_num >= 0xc0 or defender.wt < 0x80 then
         if attacker.align == ALIGNMENT_GOOD and defender.align ~= ALIGNMENT_EVIL and defender.wt ~= ALIGNMENT_CHAOTIC then
            defender.align = ALIGNMENT_EVIL
         end

         if defender.wt >= 0xc0 then
            defender.wt = 8
         end
      else
         if attacker.wt == WT_PLAYER then
            defender.wt = 0xa0
         end
      end
   end
end

function canal_worm_eat_body(worm_actor)
   --FIXME
   --only eat if a body was created when the actor was killed.
end

function actor_handle_damage(defender)
   if defender.obj_n == 370 or defender.hp == 0 then --OBJ_ROCKWORM_BASE
      return
   end

   local hp_level = math.floor(((defender.hp * 4) / defender.max_hp))
   if hp_level == 0 then
      printfl("ACTOR_CRITICAL", defender.name)
      --FIXME giant_maw check
      local wt = defender.wt
      if wt == 0xd then
         defender.wt = 0x11
         --FIXME if actor qaul ~= 0 then set actor.get(qual).wt = 0x11
      elseif wt ~= 6 and wt ~= 0xe and wt ~= 2 and wt ~= 1 and wt ~= 0 and wt ~= 0x11 and wt ~= 0x19
            and ( is_actor_stat_bit_set(defender.obj_n, 7) or is_actor_stat_bit_set(defender.obj_n, 14) ) then
         if actor_int_adj(defender) >= 5 and defender.obj_n ~= 145 then --OBJ_MONSTER_FOOTPRINTS
            defender.wt = 7
         end
      end
   elseif hp_level < 4 then
      local suffix
      if is_actor_stat_bit_set(defender.obj_n, 14) and is_actor_stat_bit_set(defender.obj_n, 7) then
         suffix = i18n("DAMAGED")
      else
         suffix = i18n("WOUNDED")
      end

      local damage_type
      if hp_level == 1 then
         damage_type = i18n("HEAVILY")
      elseif hp_level == 2 then
         damage_type = i18n("LIGHTLY")
      elseif hp_level == 3 then
         damage_type = i18n("BARELY")
      end
      printfl("ACTOR_HIT_MESSAGE", defender.name, damage_type, suffix)

   end
end

function actor_take_hit(attacker, defender, max_dmg, damage_mode)
   if max_dmg == -1 then
      max_dmg = 1
   elseif max_dmg > 1 and max_dmg < 255 then
      max_dmg = math.random(1, max_dmg)
   end

   local armour_value = 0
   if damage_mode ~= 2 then
      if defender.luatype == "actor" then
         armour_value = actor_get_ac(defender)
      elseif defender.obj_n == 388 and defender.frame_n == 0 then --OBJ_GLOW_WORM
         armour_value = 2
      end
   end

   if armour_value > 0 and max_dmg < 255 then
      max_dmg = max_dmg - math.random(1, armour_value)
   end

   if max_dmg > 0 then
      if (attacker.align == ALIGNMENT_EVIL or attacker.align == ALIGNMENT_CHAOTIC)
              and defender.luatype == "actor" and defender.in_party
              and party_is_in_combat_mode() == false
              and player_is_in_solo_mode() == false then
         party_set_combat_mode(true)
      end

      attacker.exp = attacker.exp + actor_hit(defender, max_dmg, damage_mode)
   else
      --FIXME call either sub_19088 or sub_18F7D
      printfl("ACTOR_GRAZED", defender.name)
   end

   if defender.luatype == "actor" then
      --FIXME only call if defender == selected obj
      defender_update_alignment(attacker, defender, max_dmg)
      if defender.hp == 0 then
         if attacker.obj_n == 379 then --OBJ_CANAL_WORM
            canal_worm_eat_body(attacker)
         end
      else
         if is_actor_stat_bit_set(attacker.obj_n, 2) and math.random(0, 3) == 0 and actor_immune_to_dmg(defender) == false then
            printfl("ACTOR_PARALYZED", defender.name)
            defender.paralyzed = true
         end

         if defender.alive and max_dmg > 0 then
            actor_handle_damage(defender)
         end
      end
   end

end

function actor_immune_to_dmg(actor)
   local actor_num = actor.actor_num
   local obj_n = actor.obj_n
   if obj_n == 381 or obj_n == 390 or obj_n == 391 or obj_n == 358 or obj_n == 382 then
      return true
   end

   if actor_num == 0x5c or actor_num == 0x5d or actor_num == 0x59 or actor_num == 0x5a or actor_num == 0x54 or actor_num == 0x52
      or actor_num == 0x6d or actor_num == 0x68 or actor_num == 0x67 or actor_num == 0x69 or actor_num == 0x77 or actor_num == 0x78
      or actor_num == 0x40 or actor_num == 0x3c then
      return true
   end

   return false
end

function actor_print_custom_hit_message(actor)
   local actor_num = actor.actor_num
   if actor_num == 0x69 then
      printfl("ACTOR_CRITICAL", Actor.get(0x19).name)
   elseif actor_num == 0x3c then
      printfl("ACTOR_CRITICAL", actor.name)
   elseif actor_num == 0x52 then
      printfl("ACTOR_HIT_MESSAGE", actor.name, i18n("HEAVILY"), i18n("WOUNDED"))
   elseif actor_num == 0x68 then
      if Actor.get_talk_flag(actor, 2) then
         printfl("ACTOR_CRITICAL", actor.name)
      else
         printfl("ACTOR_HIT_MESSAGE", actor.name, i18n("HEAVILY"), i18n("WOUNDED"))
      end
      Actor.set_talk_flag(actor, 5)
      Actor.talk(actor)
   else
      printl("IT_HAS_NO_EFFECT")
   end
end

local RED_HIT_TILE = 257
local BLUE_HIT_TILE = 258

function hit_target(target, hit_tile)
   if map_is_on_screen(target.xyz) then
      if target.luatype == "actor" and target.in_party then
         play_md_sfx(1, PLAY_ASYNC)
      else
         play_md_sfx(2, PLAY_ASYNC)
      end
      hit_anim(target.x, target.y) --FIXME need to apply hit colour tile param here.
   end

   if target.luatype == "actor" then
      --FIXME
--      les     bx, objlist_unk_1af1_ptr
--      mov     al, es:[bx+si]
--      or      al, 8
--      mov     es:[bx+si], al

      if target.asleep then
         target.asleep = false
      end
   end

end

-- Hit an actor or object
function actor_hit(defender, max_dmg, damage_mode)


   local defender_obj_n = defender.obj_n
   local exp_gained = 0
   local player_loc = player_get_location()

   if defender.z ~= player_loc.z or max_dmg < 0 then
      return 0
   end

   if defender.luatype == "actor" then
      print("actor_hit("..defender.actor_num..")\n")

      if defender.actor_num == 0 and defender.hp <= max_dmg and g_current_dream_stage == 0xe0 then
        max_dmg = defender.hp - 1
      end

      if damage_mode == 1 then
         hit_target(defender, BLUE_HIT_TILE)
      else
         hit_target(defender, RED_HIT_TILE)
      end

      if actor_immune_to_dmg(defender) then
         actor_print_custom_hit_message(defender)
         defender.hp = 0xff
         if defender_obj_n == 391 then --OBJ_YOUR_MOTHER
            local gender_title = ""
            if player_get_gender() == 0 then
               gender_title = i18n("MAN")
            else
               gender_title = i18n("WOMAN")
            end
            printfl("HOW_DARE_YOU_YOUNG_PERSON", gender_title)
            actor_take_hit(defender, Actor.get(0), 45, 0)
            actor_take_hit(defender, Actor.get(0), 45, 0)
            actor_take_hit(defender, Actor.get(0), 45, 0)
         elseif defender.actor_num == 0x6d or defender.actor_num == 0x67 then
            Actor.set_talk_flag(Actor.get(0x6b), 3)
            local dream_actor = Actor.get(0)
            Actor.move(dream_actor, g_prev_player_x, g_prev_player_y, dream_actor.z)
         elseif defender.actor_num == 0x40 then --Rasputin
            --FIXME attack_rasputin()
         end
      else
         if damage_mode == 3 and is_actor_stat_bit_set(defender_obj_n, 3) then
            printl("IT_HAS_NO_EFFECT")
            return 0
         end
         if is_actor_stat_bit_set(defender_obj_n, 4) then
            max_dmg = max_dmg * 2
         end

         if (damage_mode == 0 and actor_num ~= 0) or g_current_dream_stage ~= 0xe0 then --or word_41184 ~= 0xe0
            if math.random(6, 0x64) <= max_dmg then
               --FIXME actor_add_blood()
            end
            local hp = defender.hp
            if hp <= max_dmg then
               --FIXME exp_gained = kill_actor()
            else
               defender.hp = hp - max_dmg

               if defender.wt == 9 or defender.wt == 11 then
                  defender.wt = 8
               elseif defender.wt == 10 or defender.wt == 12 then
                  defender.wt = 0x1b
               end

               if is_actor_stat_bit_set(defender_obj_n, 6) then
                  actor_divide(defender)
               end
            end
         end
      end
   else
      actor_hit_obj(defender, max_dmg, damage_mode)
   end

   return exp_gained
end

function attack_bucket(bucket, damage_mode)
   if damage_mode == 3 and bucket.frame_n == 2 then
      printl("THE_ICE_THAWS")
      bucket.frame_n = 1
   elseif damage_mode == 1 and bucket.frame_n == 1 then
      printl("THE_WATER_FREEZES")
      bucket.frame_n = 2
   end
end

function actor_hit_obj(obj, dmg, damage_mode)
   if obj.qty == 0 or not is_obj_attackable(obj) then
      if obj.obj_n == 160 then --OBJ_BUCKET
         attack_bucket(obj, damage_mode)
      end
      return
   end

   if damage_mode == 0 or damage_mode == 3 or is_plant_obj(obj) then

      if damage_mode == 1 then
         hit_target(obj, BLUE_HIT_TILE)
      else
         hit_target(obj, RED_HIT_TILE)
      end

      if damage_mode == 3 and is_obj_burnable(obj) then
         dmg = obj.qty
      end

      if obj.qty <= dmg then
         if obj.obj_n == 217 then --OBJ_GLASS_PITCHER
            play_md_sfx(0x1a)
            obj.obj_n = 218 --OBJ_BROKEN_CRYSTAL
            obj.frame_n = 0
            if g_in_dream_mode and g_current_dream_stage == 0x44 then
               local pitcher
               for tmp_obj in find_obj_from_area(0x21, 0x33, 2, 0x1c, 0x10) do
                  if tmp_obj.obj_n == 217 then --OBJ_GLASS_PITCHER
                     pitcher = tmp_obj
                  end
               end
               if pitcher == nil then
                  Actor.set_talk_flag(0x54, 6)
               end
            end
         else
            play_md_sfx(0)
            Obj.removeFromEngine(obj)
         end
      else
         if obj.obj_n == 307 then --OBJ_DEVIL_POD
            printl("THE_POD_SPLITS_OPEN")
            local pod_devil = Actor.new(384, obj.x, obj.y, obj.z)

            actor_init(pod_devil)
            Actor.move(pod_devil, obj.x, obj.y, obj.z)
            Obj.removeFromEngine(obj)
         else
            if obj.obj_n == 388 then --OBJ_GLOW_WORM
               obj.frame_n = 0
               obj.quality = math.random(0xb, 0x12)
               play_md_sfx(0x22)
            end
            obj.qty = obj.qty - dmg
         end

      end

   end

end

function actor_move(actor, direction, flag)
   ----dgb("actor_move("..actor.name..", "..direction_string(direction)..", "..flag..") actor("..actor.x..","..actor.y..")\n");
   local x,y,z = actor.x, actor.y, actor.z
   if direction == DIR_NORTH then y = y - 1 end
   if direction == DIR_SOUTH then y = y + 1 end
   if direction == DIR_EAST then x = x + 1 end
   if direction == DIR_WEST then x = x - 1 end

   if actor.obj_n ~= 382 then --COKER FIXME need to have an update frame function here.
      actor.direction = direction
   end

   local did_move = Actor.move(actor, x, y, z)

   --FIXME need more logic here.
   --footprints, bots etc.

   if did_move then
      subtract_map_movement_pts(actor)
      ----dgb("actor_move() did move actor("..actor.x..","..actor.y..")\n");
   end

   return did_move
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
      subtract_map_movement_pts(actor)
   end

   return did_move and 1 or 0
end

function actor_update_frame(actor, direction)
   local obj_n = actor.obj_n

   --actor.direction = direction
   if obj_n >= 342 or obj_n <= 358 then --human actors
      --print("actor: "..actor.actor_num.."("..actor.x..","..actor.y..","..actor.z..")")
      for obj in objs_at_loc(actor.x, actor.y, actor.z) do
         if obj ~= nil then
            local tmp_obj_n = obj.obj_n
            --print("actor:"..actor.actor_num.." obj: " .. tmp_obj_n)
            if tmp_obj_n == 406 or (tmp_obj_n == 407) or tmp_obj_n == 216 or tmp_obj_n == 289 then
               --sit here
               if tmp_obj_n ==  406 then -- OBJ_BENCH
                  actor.frame_n = 3
               elseif tmp_obj_n == 407 then -- OBJ_COUCH
                  actor.frame_n = 4 + 3
               elseif tmp_obj_n == 289 then -- OBJ_DREAM_MACHINE2
                  actor.frame_n = 8 + 3
               else
                  actor.frame_n = math.floor(direction/2) * 4 + 3
               end

               return
            end
         end
      end
      --standing

   end
end

function revive_avatar()
   if g_in_dream_mode == true then
      --FIXME died in dream world. old_player_x == 0
      --FIXME respawn into dreamworld
   else
      -- normal avatar death
      avatar_falls_unconscious()
   end
end

function actor_resurrect(actor)
   --FIXME do we need to do anything here?
end

function avatar_falls_unconscious()
   printl("OVERCOME_BY_YOUR_WOUNDS_YOU_FALL_UNCONSCIOUS")

   fade_out()

   local input
   while input == nil do
      --canvas_update()
      input = input_poll()
      if input ~= nil then
         break
      end
   end

   local location
   local target
   if Actor.get_talk_flag(0x46, 3) then
      location = i18n("THE_OLYMPUS_MINE")
      target={x=0xa4,y=0xc3,z=4}
   else
      location = i18n("THE_SPACE_CAPSULE")
      target={x=0x19d,y=0x278,z=0}
   end

   printfl("YOU_AWAKEN_BACK_AT_FEELING_RESTORED", location)

   input_select(nil, true)

   party_resurrect_dead_members()

   for actor in party_members() do
      actor.hp = actor.max_hp
      actor.mpts = actor_dex_adj(actor)
   end

   party_move(target)
   local hour = clock_get_hour()
   local minutes = clock_get_minute()
   minutes = 60 - minutes
   if hour < 7 then
      hour = 6 - hour
   else
      hour = 24 - (hour + 1) + 7
   end

   clock_inc(hour * 60 + minutes + math.random(0,59)) --advance time to between 7am and 8am on the next day
   update_watch_tile()
   advance_time(0)

   party_update_leader()
   party_set_combat_mode(false)
   party_set_party_mode()

   local blood = Actor.get(0x12)
   if blood.alive then
      Actor.set_talk_flag(blood, 5)
      Actor.talk(blood)
   end

   fade_in()
end

function party_update()
   local avatar = Actor.get(1)
   if avatar.hp == 0 or (g_in_dream_mode == true and Actor.get(0).alive == false) then
      if g_in_dream_mode == true then
         g_objlist_1d22_unk = 0
         if g_prev_player_x == 0 then
            printl("YOU_SHAKE_YOURSELF_AWAKE_FROM_THE_NIGHTMARE")
            wake_from_dream()
         else
            local avatar = Actor.get(1)
            local dream_actor = Actor.get(0)
            dream_actor.hp = avatar.max_hp
            dream_actor.poisoned = false
            Actor.move(dream_actor, g_prev_player_x, g_prev_player_y, dream_actor.z)
            party_set_combat_mode(false)
            printl("YOU_FEEL_YOUR_DREAM_CONSCIOUSNESS_RETURNING")
            if g_current_dream_stage == 0xc0 then
               if not Actor.get_talk_flag(0x66, 2)
                  and Actor.get_talk_flag(0x66, 3)
                  and Actor.get_talk_flag(0x66, 4)
                  and not Actor.get_talk_flag(0x66, 5)
               then
                  for i=0,0xff do
                     local maw = Actor.get(i)
                     if maw.obj_n == 373 and maw.wt == 0x12 and maw.x == 0x87 and maw.y == 0x17 then
                        Actor.kill(maw, false)
                     end
                  end
                  if map_get_obj(0x7f, 0x18, dream_actor.z, 224) == nil then --OBJ_BRIDGE
                     local bridge = Obj.new(224, 3)
                     Obj.moveToMap(bridge, 0x7f, 0x18, dream_actor.z)
                  end
               end
               Actor.set_talk_flag(0x66, 7)
               local raxachk = Actor.get(0x66)
               Actor.talk(raxachk)
            end
         end
      else
         revive_avatar()
      end
   end

end

function actor_update_all()
   party_update()
   --pathfinding here.

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
            --FIXME need to check 11000b not zero on obj_flags
            if actor.obj_n ~= 0 and actor.z == player_z and actor.mpts > 0 and actor.paralyzed == false and (actor.asleep == false or actor.wt == 0x80) and actor.wt ~= WT_NOTHING and actor.alive == true then
                  --FIXME need to check map wrapping here.
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

                        local dex_adjusted = actor_dex_adj(actor)
                        local dx = (actor.mpts * dex_6) - dex_adjusted * di
                        if actor.mpts >= dex_adjusted or dx > 0 or dx == 0 and dex_adjusted > dex_6 then
                           selected_actor = actor
                           di = actor.mpts
                           dex_6 = dex_adjusted
                        end

                        if dex_adjusted <= actor.mpts then
                           break
                        end
                     end
                  end
            end
         end

         if di <= 0 then
            for i=0,0xff do
               local actor = Actor.get(i)
               local dex_adjusted = actor_dex_adj(actor)
               if actor.mpts >= 0 then
                  actor.mpts = dex_adjusted
               else
                  actor.mpts = actor.mpts + dex_adjusted
               end
            end
            advance_time(1)
         end

      until di > 0

      if selected_actor.wt ~= WT_PLAYER and selected_actor.wt ~= WT_FOLLOW then
         --print("perform_worktype("..selected_actor.name.."("..selected_actor.actor_num..") dex = "..selected_actor.dex.." mpts = "..selected_actor.mpts..").\n")
         perform_worktype(selected_actor)
         party_update()
         if selected_actor.wt > 1 and selected_actor.wt < 0x1b then
            --FIXME targeting?? do *(&bjlist_unk_19f1_ptr + actor_num) = actor_num
         end
      end

   until selected_actor.obj_n ~= 0 and selected_actor.wt == WT_PLAYER

   if selected_actor ~= nil then --swap player to next party member with 'command' combat worktype.
    local old_player = Actor.get_player_actor()
    player_set_actor(selected_actor)
    old_player.wt = WT_PLAYER --reset worktype to player as it gets changed to follow in Player::set_actor() :-(
   end


   display_prompt(true)
end



local projectile_weapon_tbl = --FIXME weed sprayer and spray gun
{
--obj_n = {tile_num, initial_tile_rotation, speed, rotation_amount} --FIXME: all untested
[40] = {575, 90,4, 0}, -- Cupid's bow and arrows
[41] = {259, 90,4, 0}, -- derringer --FIXME: tile_num, rot, speed, amount
[42] = {259, 90,4, 0}, -- revolver --FIXME: rot, speed, amount
[43] = {262, 90,4, 0}, -- shotgun --FIXME: rot, speed, amount
[44] = {259, 90,4, 0}, -- rifle --FIXME: rot, speed, amount
[45] = {259, 90,4, 0}, -- Belgian combine rifle --FIXME: rot, speed, amount (has 3 modes)
--[45] = {262, 90,4, 0}, -- Belgian combine spread(has two) --FIXME: rot, speed, amount
[46] = {259, 90,4, 0}, -- elephant gun --FIXME: tile_num, rot, speed, amount
[47] = {398, 0, 2, 0}, -- sling --FIXME: tile_num, rot, speed, amount
[48] = {575, 90,4, 0}, -- bow
[241] = {334, 90,4, 0}, -- heat ray gun --FIXME: rot, speed, amount (has 3 modes)
[242] = {335, 90,4, 0}, -- freeze ray gun --FIXME: rot, speed, amount (has 3 modes)
[313] = {267, 90,4, 0}, -- M60 machine gun --FIXME: rot, speed, amount (if even used)

}

armour_tbl =
{
--[1] = 0, --cap
--[2] = 0, --cowboy hat
[3] = 1, --pith helmet
[4] = 2, --military helmet
--[5] = 0, --derby
--[6] = 0, --kerchief
--[7] = 0, --silk scarf
--[8] = 0, --muffler
--[9] = 0, --man's shoes
--[10] = 0, --woman's shoes
[11] = 1, --riding boots
--[12] = 0, --ruby slippers
[13] = 2, --thigh boots
[14] = 3, --hip boots
--[15] = 0, --winged shoes
--[17] = 0, --cloth jacket
--[18] = 0, --wool sweater
--[19] = 0, --cape
--[20] = 0, --duster
[21] = 1, --wool overcoat
[22] = 2, --sheepskin jacket
[23] = 2, --arctic parka
--[25] = 0, --cotton pants
--[26] = 0, --cotton dress
[27] = 1, --denim jeans
--[28] = 0, --wool pants
--[29] = 0, --wool dress
[30] = 3, --chaps and jeans
--[31] = 0, --man's shoes
--[33] = 0, --lady's silk gloves
[34] = 1, --driving gloves
--[35] = 0, --cotton work gloves
[36] = 2, --work gloves
--[37] = 0, --wool mittens
[38] = 1, --rubber gloves
[39] = 3, --welding gloves
--[90] = 0, --electric belt?
--[234] = 0, --martian jewelry
}

function actor_get_ac(actor)
   local ac = 0

   for obj in actor_inventory(actor) do
      if obj.readied then

         local armour = armour_tbl[obj.obj_n]
         if armour ~= nil then
            ac = ac + armour
         end
      end
   end
   return ac
end

local clothing_warmth_tbl = {
[1] =    1, -- OBJ_CAP
[2] =    2, -- OBJ_COWBOY_HAT
[3] =    3, -- OBJ_PITH_HELMET
[4] =    3, -- OBJ_MILITARY_HELMET
[5] =    2, -- OBJ_DERBY
[6] =    1, -- OBJ_KERCHIEF
[7] =    1, -- OBJ_SILK_SCARF
[8] =    2, -- OBJ_MUFFLER
[9] =    1, -- OBJ_MANS_SHOES
[10] =   1, -- OBJ_WOMANS_SHOES
[11] =   2, -- OBJ_RIDING_BOOTS
[14] =   4, -- OBJ_HIP_BOOTS
[13] =   3, -- OBJ_THIGH_BOOTS
[17] =   2, -- OBJ_CLOTH_JACKET
[18] =   3, -- OBJ_WOOL_SWEATER
[19] =   3, -- OBJ_CAPE
[20] =   4, -- OBJ_DUSTER
[21] =   4, -- OBJ_WOOL_OVERCOAT
[22] =   3, -- OBJ_SHEEPSKIN_JACKET
[23] =   5, -- OBJ_ARCTIC_PARKA
[90] =  12, -- OBJ_ELECTRIC_BELT
[25] =   2, -- OBJ_COTTON_PANTS
[26] =   2, -- OBJ_COTTON_DRESS
[27] =   2, -- OBJ_DENIM_JEANS
[28] =   3, -- OBJ_WOOL_PANTS
[29] =   3, -- OBJ_WOOL_DRESS
[30] =   3, -- OBJ_CHAPS_AND_JEANS
[33] =   1, -- OBJ_LADYS_SILK_GLOVES
[34] =   1, -- OBJ_DRIVING_GLOVES
[35] =   1, -- OBJ_COTTON_WORK_GLOVES
[36] =   2, -- OBJ_WORK_GLOVES
[37] =   3, -- OBJ_WOOL_MITTENS
[38] =   1, -- OBJ_RUBBER_GLOVES
[39] =   3, -- OBJ_WELDING_GLOVES
}

function clothing_get_warmth_rating(obj)
   local rating = clothing_warmth_tbl[obj.obj_n]
   if rating == nil then
      rating = 0
   end

   return rating
end

local lit_lightsource_tbl = {
[110] =  1, -- OBJ_LIT_TORCH
[112] =  1, -- OBJ_LIT_CANDLE
[114] =  1, -- OBJ_LIT_CANDELABRA
[116] =  1, -- OBJ_LIT_OIL_LAMP
[118] =  1, -- OBJ_LIT_LANTERN
}

function is_lit_lightsource(obj)
   if lit_lightsource_tbl[obj.obj_n] ~= nil then
      return true
   end

   return false
end

function actor_str_adj(actor)
   local actor_num = actor.actor_num
   local str = actor.str

   if actor.hypoxia then
      str = str - 3
   end

   if actor_is_affected_by_purple_berries(actor_num) then
      str = str - 3
   end

   if actor.frenzy then
      str = str + 3
      if str > 30 then
         str = 30
      end
   end

   if str <= 3 then
      return 1
   end

   return str
end

function actor_dex_adj(actor)

   local dex = actor.dex
   if actor.hypoxia then
      if dex <= 3 then
         dex = 1
      else
         dex = dex - 3
      end
   end

   if actor.frenzy then
      dex = dex + 3
      if dex >= 30 then
         dex = 30
      end
   end

   if actor.asleep then
      dex = 1
   end

   return dex
end

function actor_int_adj(actor)
   local int = actor.int

   if actor.hypoxia == true or (actor.frenzy and actor.actor_num ~= 1) then
      int = int - 3
   end

   if int < 1 then int = 1 end

   return int
end

function actor_map_dmg(actor, map_x, map_y, map_z)
   local obj_n = actor.obj_n
   local actor_type = actor_tbl[obj_n]
   local player_loc = player_get_location()


   if is_actor_stat_bit_set(obj_n, 8) or is_actor_stat_bit_set(obj_n, 10)
           or actor.z ~= player_loc.z
           or actor_find_max_wrapped_xy_distance(actor, player_loc.x, player_loc.y) > 40 then
      return
   end

   if actor.alive == false or actor.hit_flag == true then
      return
   end
--print("actor_map_dmg("..actor.actor_num..")\n")

   for obj in objs_at_loc(actor.x, actor.y, actor.z) do
      local tile_num = obj.tile_num_original
      if tile_get_flag(tile_num, 3, 2) == false and tile_get_flag(tile_num, 1, 3) == true then --force passable and damaging
         actor_take_damage_from_obj(actor, obj)
      end

   end

   actor_update_frame(actor, actor.direction)
end

function actor_take_damage_from_obj(actor, obj)
   if actor_immune_to_dmg(actor) or actor.alive == false then
      return
   end

   local obj_n = obj.obj_n
   local damage = 0
   local damage_mode = 0

   if obj_n == 168 then --OBJ_PORCUPOD
      damage = math.random(1, 0x14)
   elseif obj_n == 209 then --OBJ_STEAM_LEAK
      damage_mode = 3
      if actor.actor_num ~= 6 then
         damage = math.random(math.floor(actor.hp / 2), math.floor(actor.max_hp / 2))
      end
   elseif obj_n == 215 then --OBJ_POWER_CABLE1
      play_md_sfx(0x15)
      damage = math.random(1, 0x14) + math.random(1, 0x14)
   elseif obj_n == 381 then --OBJ_DUST_DEVIL
      damage = math.random(1, 0xa) + math.random(1, 0xa)
   elseif obj_n == 463 then --hidden effect obj
      --FIXME here
   end

   actor_hit(actor, damage, damage_mode)
end

function actor_remove_charm(actor)

   actor.charmed = false;
   actor.align = actor.old_align

   if actor.in_party then
      actor.align = ALIGNMENT_GOOD
   end

   if party_is_in_combat_mode() then
      actor.wt = actor.combat_mode
   else
      if player_is_in_solo_mode() then
         actor.wt = WT_NOTHING
      else
         actor.wt = WT_FOLLOW
      end
   end

   party_update_leader()

end

function advance_time(num_turns)
   --FIXME
   local rand = math.random
   local hour = clock_get_hour()

   local quake = Actor.get_talk_flag(0x46, 3) --rasputin

   if quake then
      if rand(0, 4) == 0 then
         quake_start(1, 200)
      end
   end

   local max_light = 0

   local actor_num
   for actor_num=0,0xff do
      local actor = Actor.get(actor_num)
      if actor.alive then
         if g_in_dream_mode == false and actor.in_party and actor_num ~= 0 then
            if num_turns > 0 and actor.asleep == false then --FIXME I think we also need to check distance < 0x27 from either mapwindow or player.
               if actor_num == 6 or Actor.get_talk_flag(0x10, 5) == false or Actor.inv_has_obj_n(actor, 131) then --OBJ_BLOB_OF_OXIUM
                  if actor.hypoxia then
                     actor.hypoxia = false
                     printfl("BREATHES_EASIER", actor.name)
                  end
               else
                  if actor.hypoxia == false then
                     actor.hypoxia = true
                     printfl("GASPS_FOR_AIR", actor.name)
                  end
               end

               local warmth_rating = 0
               local obj
               for obj in actor_inventory(actor) do
                  if obj.readied then
                     warmth_rating = warmth_rating + clothing_get_warmth_rating(obj)

                     if is_lit_lightsource(obj) then
                        if rand(0, 1) == 1 then
                           if obj.quality <= num_turns then
                              if obj.obj_num == 116 --OBJ_LIT_OIL_LAMP
                                 or obj.obj_num == 118 then --OBJ_LIT_LANTERN
                                 if obj.obj_num == 116 then --OBJ_LIT_OIL_LAMP
                                    obj.obj_n = 115 --OBJ_OIL_LAMP
                                 else --OBJ_LIT_LANTERN
                                    obj.obj_n = 117--OBJ_LANTERN
                                 end
                                 obj.quality = 0
                                 printfl("THE_IS_OUT_OF_FUEL", obj.name)
                              else
                                 printfl("WENT_OUT", obj.look_string)
                                 Obj.removeFromEngine(obj)
                              end
                           else
                              obj.quality = obj.quality - num_turns
                           end
                        end
                     --FIXME update max_light here. probably not needed as light is updated elsewhere in nuvie.
                     --if max_light < obj.light then
                     --max_light = obj.light
                     --end
                     end
                  end
               end

               if g_party_is_warm or actor.z ~= 0 then
                  if actor.cold then
                     actor.cold = false
                     printfl("FEELS_WARMER", actor.name)
                  end
               else
                  local cold_status = 0

                  if hour <= 3 or hour >= 22 then
                     if warmth_rating >= 10 then
                        cold_status = 1
                     else
                        cold_status = 2
                     end
                  elseif hour <= 6 or hour >= 18 then
                     if warmth_rating < 10 then
                        cold_status = 1
                     end
                  end

                  if actor_num == 6 then
                     cold_status = 0
                  end

                  if cold_status == 0 then
                     if actor.cold then
                        actor.cold = false
                        printfl("FEELS_WARMER", actor.name)
                     end
                  else
                     if actor.cold == false then
                        actor.cold = true
                        printfl("IS_FREEZING", actor.name)
                     end
                     if num_turns ~= 0 then
                        for i=1,num_turns do
                           if rand(0, 1) == 0 then
                              if cold_status == 2 then
                                 printfl("IS_FREEZING", actor.name)
                                 actor_hit(actor, rand(1, 2))
                              end
                           else
                              printfl("IS_FREEZING", actor.name)
                              actor_hit(actor, rand(1, 2))
                              if cold_status == 2 then
                                 actor_hit(actor, rand(1, 2))
                              end
                           end
                        end
                     end
                  end
               end

            end
         end

         if num_turns ~= 0 then
            for i=1,num_turns do
               --FIXME what does  word_4E6FA do?

               --remove battle frenzy from actor when the party exits combat mode
               if actor.frenzy and not party_is_in_combat_mode() then
                  actor.frenzy = false
               end

               if actor.poisoned then
                  if rand(0, 25) == 0 then
                     actor.poisoned = false
                  end
               end

               if actor.charmed and rand(0, 0x19) == 0 then
                  actor_remove_charm(actor)
               end

               if actor.paralyzed then
                  if actor_num == 6 or (rand(0, 3) == 0 and actor.str >= rand(1, 0x1e)) then --FIXME used adjusted str
                     actor.paralyzed = false
                  end
               end

               if actor.asleep and actor.wt ~= WT_SLEEP and (not g_party_is_warm or not actor.in_party) then
                  --FIXME check sub_2B0EC(actor.x,actor.y,actor.z)
                  if rand(0,0x14) == 0 then
                     actor.asleep = false
                     --FIXME bit 3 set on 1af1 flags
                  end
               end

               if actor.poisoned and actor_num ~= 6 and rand(0, 7) == 0 then
                  actor_hit(actor, 1)
               end

               if actor_num < 8 then
                  actor_decrement_berry_counters(actor)

                  for obj in actor_inventory(actor, true) do
                     local obj_n = obj.obj_n
                     if obj_n == 160 and obj.frame_n > 1 then --OBJ_EMPTY_BUCKET with ice
                        if rand(0, 100) < 10 then
                           printl("SOME_ICE_HAS_MELTED")
                           obj.frame_n = 1
                        end
                     elseif obj_n == 256 then --OBJ_CHUNK_OF_ICE
                        if rand(0, 100) < 10 then
                           printl("SOME_ICE_HAS_MELTED")
                           Obj.removeFromEngine(obj)
                        end
                     elseif obj_n == 448 or (obj_n == 449 and actor_num ~= 6 and actor.poisoned == false) then --OBJ_BLOCK_OF_RADIUM, OBJ_CHIP_OF_RADIUM
                        if obj.in_container == false or obj.parent.obj_n ~= 139 then --OBJ_LEAD_BOX
                           printfl("IS_POISONED", actor.name)
                           actor.poisoned = true
                        end
                     end
                  end
               end

            end
         end

         actor_map_dmg(actor, actor.x, actor.y, actor.z)
         actor.hit_flag = false
      end
   end

   local minute = clock_get_minute()

   clock_inc(num_turns)

   if minute + num_turns >= 60 then

      update_watch_tile()

      update_actor_schedules()
      if g_hours_till_next_healing > 0 then
         g_hours_till_next_healing = g_hours_till_next_healing - 1
      end

      update_lamp_posts()

      local blue_berry_counter = actor_get_blue_berry_counter()
      if blue_berry_counter > 0 then
         actor_get_blue_berry_counter(blue_berry_counter - 1)
      end

      if not g_party_is_warm and not g_in_dream_mode and Actor.get_talk_flag(0x10, 5) then
         for actor in party_members() do
            if actor.actor_num ~= 6 and not actor.asleep then
               local oxium = Actor.inv_get_obj_n(actor, 131) --OBJ_BLOB_OF_OXIUM
               if oxium ~= nil then
                  if oxium.qty > 1 then
                     oxium.qty = oxium.qty - 1
                     if actor.hypoxia then
                        actor.hypoxia = false
                        printfl("BREATHES_EASIER", actor.name)
                     end
                  else
                     Obj.removeFromEngine(oxium)
                     if Actor.inv_get_obj_n(actor, 131) ~= nil then
                        if actor.hypoxia then
                           actor.hypoxia = false
                           printfl("BREATHES_EASIER", actor.name)
                        end
                     else
                        if actor.hypoxia == false then
                           actor.hypoxia = true
                           printfl("GASPS_FOR_AIR", actor.name)
                        end
                     end
                  end
               else
                  if actor.hypoxia == false then
                     actor.hypoxia = true
                     printfl("GASPS_FOR_AIR", actor.name)
                  end
               end
            end
         end
      end

   end
end

function can_get_obj_override(obj)
   return false
end

function subtract_movement_pts(actor, points)
   if actor.actor_num < 16 then
      if party_is_in_combat_mode() == false then
         points = points - 2
      end
   end

   if points < 1 then
      points = 1
   end

   actor.mpts = actor.mpts - points
end

function actor_radiation_check(actor, obj)
   if obj.obj_n == 448 or obj.obj_n == 449 then --OBJ_BLOCK_OF_RADIUM, OBJ_CHIP_OF_RADIUM

      local actor_num = actor.actor_num
      if actor_num == 6
         or actor_is_affected_by_purple_berries(actor_num) -- purple berries protect against radiation
         or Actor.inv_get_readied_obj_n(actor, ARM) == 136 --OBJ_TONGS
         or Actor.inv_get_readied_obj_n(actor, ARM_2) == 136 then
         return
      end

      actor.poisoned = true
      printl("OUCH_IT_IS_VERY_HOT")
   end
end

function actor_get_obj(actor, obj, container) -- FIXME need to limit inventory slots

   --FIXME handle getting into container.

	if obj.getable == false then
		printnl("THAT_IS_NOT_POSSIBLE")
		return false
	end

   if actor_find_max_wrapped_xy_distance(actor, obj.x, obj.y) > 1 then
      play_md_sfx(0x32)
      projectile_anim(obj.tile_num, obj.x, obj.y, actor.x, actor.y, 4, false, 0)
   end

	if Actor.can_carry_obj_weight(actor, obj) == false then
		printl("THE_TOTAL_IS_TOO_HEAVY")
		return false
	end

   if not Actor.can_carry_obj(actor, obj) then
      printnl("YOU_ARE_CARRYING_TOO_MUCH_ALREADY")
      return false
   end

   subtract_movement_pts(actor, 3)

   actor_radiation_check(actor, obj)

   if obj.obj_n == 256 then -- OBJ_CHUNK_OF_ICE
      printnl("THE_ICE_IS_MELTING")
   end

   if obj.obj_n == 110 -- OBJ_LIT_TORCH
           or obj.obj_n == 112
           or obj.obj_n == 114
           or obj.obj_n == 116
           or obj.obj_n == 118 then
      if not actor_has_free_arm(actor) then
         printl("YOUR_HANDS_ARE_FULL")
         return false
      end

      Obj.moveToInv(obj, actor.actor_num)
      Actor.inv_ready_obj(actor, obj)
      if obj.obj_n == 110 then -- OBJ_LIT_TORCH
         obj.quality = 0xb4
      end

      advance_time(0)

      return true
   end

   if obj.obj_n == 411 -- OBJ_SWITCH_BAR
           or obj.obj_n == 458 -- OBJ_PANEL
           or obj.obj_n == 314 -- OBJ_TRACKING_MOTOR
           or obj.obj_n == 206 then -- OBJ_TIFFANY_LENS
      if obj.quality % 2 == 0 then
         printnl("WONT_BUDGE")
         return false
      end
   end

   Obj.moveToInv(obj, actor.actor_num)

	return true
end

function actor_get_max_hp(actor)
   if actor.actor_num == 6 then
      return 0xf0
   end

   if actor.in_party then
      return actor.str * 2 + actor.level * 24
   end

   --FIXME return actor max hp from stat table.
   return 1;
end



--function actor_take_hit(actor, damage)
--   local hp = actor.hp
--   if damage >= hp and actor.actor_num == 1 then
--      hit_anim(actor.x, actor.y)
--      actor.hp = 0
--   else
--      Actor.hit(actor, damage)
--   end
--end

function kill_actor(actor)
   actor.hp=0
   actor_dead(actor)
end

function actor_dead(actor)
   --FIXME
end

function get_portrait_number(actor)
   local NO_PORTRAIT = 255
   local idx = actor.actor_num
   if idx >= 0xa and idx <= 0xf then
      idx = 0x51
   end

   if idx >= 0xbf then
      return NO_PORTRAIT
   end

   if idx <= 1 then
      if player_get_gender() == 1 then
         idx = 0
      else
         idx = 1
      end
   elseif idx == 6 and Actor.get_talk_flag(actor, 1) then
      idx = 0x16
   elseif actor.obj_flag_0 then
      if player_get_gender() == 1 then
         idx = 0x73
      else
         idx = 0x72
      end
   elseif idx == 0x21 and not Actor.get_talk_flag(actor, 2) then
      idx = 0x61
   elseif idx == 0x15 or (idx >= 0x47 and idx <= 0x4a) or idx == 0x4e then
      idx = 0x14
   elseif idx >= 0x4b and idx <= 0x4d then
      idx = 0x16
   elseif idx == 0x1e then
      idx = 0x18
   elseif idx == 0x46 then
      idx = 0x40
   elseif idx >= 0x50 and idx <= 0x57 then
      idx = idx - 0x26
   elseif idx == 0x5e then
      idx = 0x6
   elseif idx == 0x68 or idx == 0x77 or idx == 0x78 then
      idx = 0x67
   elseif idx == 0x69 then
      idx = 0x19
   elseif idx == 0x6d then
      idx = 0x2
   end

   return idx
end

function find_rockworm_actor(obj)
   if obj ~= nil then
      if obj.obj_n == 371 or obj.obj_n == 370 then --OBJ_ROCKWORM1, OBJ_ROCKWORM_BASE
         local actor = Actor.get(obj.quality)
         if actor.alive then
            return actor
         end
      end
   end

   return obj
end
