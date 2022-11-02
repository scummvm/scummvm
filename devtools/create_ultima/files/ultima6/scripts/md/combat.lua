-- [objectnum] = range
local range_weapon_tbl = {
   [41]  = 4,
   [42]  = 5,
   [43]  = 4,
   [44]  = 9,
   [45]  = 9,
   [46]  = 9,
   [47]  = 4,
   [48]  = 5,
   [240] = 6,
   [241] = 6,
   [129] = 2,
   [261] = 2,
   [313] = 9,
   [40]  = 6,
   [366] = 3,
   [386] = 4,
   [364] = 3,
   [384] = 2,
}

function get_weapon_range(weapon)
   local range = range_weapon_tbl[weapon.obj_n]
   if range == nil then
      range = 1
   end
   return range
end

local weapon_dmg_tbl = {
   [16] = 30, --bloody saber
   [40] = 1, --Cupid's bow and arrows (charms)
   [41] = 15, --derringer
   [42] = 18, --revolver
   [43] = 20, --shotgun
   [44] = 30, --rifle
   [45] = 30, --Belgian combine
   [46] = 45, --elephant gun
   [47] = 8, --sling
   [48] = 12, --bow
   [49] = 15, --hatchet
   [50] = 20, --axe
   [51] = 10, --ball-peen hammer
   [52] = 25, --sledge hammer
   [54] = 10, --knife
   [55] = 20, --machete
   [56] = 25, --saber
   [65] = 15, --pick
   [66] = 8, --shovel
   [67] = 10, --hoe
   [68] = 10, --rake
   [69] = 15, --pitchfork
   [70] = 12, --cultivator
   [71] = 20, --scythe
   [72] = 10, --saw
   [102] = 12, --pry bar
   --[109] = 1, --torch
   --[110] = 1, --lit torch
   --[111] = 1, --candlestick
   --[112] = 1, --lit candle
   --[113] = 1, --candelabra
   --[114] = 1, --lit andelabra
   --[115] = 1, --oil lamp
   --[116] = 1, --lit oil lamp
   --[117] = 1, --lantern
   --[118] = 1, --lit lantern
   [129] = 60, --weed sprayer -- FIXME: no damage normally. Only effects plants?
   --[136] = 1, --tongs
   [241] = 20, --heat ray gun
   [242] = 10, --freeze ray gun
   --[243] = 1, --martian ritual pod knife
   [261] = 60, --spray gun -- FIXME: no damage normally. Only effects plants?
   [263] = 10, --martian hoe (couldn't be equipped in original)
   [264] = 20, --martian scythe (couldn't be equipped in original)
   [265] = 15, --martian pitchfork (couldn't be equipped in original)
   [266] = 10, --martian rake (couldn't be equipped in original)
   [267] = 8, --martian shovel (couldn't be equipped in original)
   [313] = 254, --M60 machine gun (scripted to only attack and kill the big bad)
   [327] = 15, --martian pick (couldn't be equipped in original)
   [401] = 12, --pool cue
}

function get_weapon_damage(weapon)
   local dmg
   if weapon.luatype == "actor" then
      dmg = actor_get_damage(weapon)
   else
      dmg = weapon_dmg_tbl[weapon.obj_n]
   end

   if dmg == nil then
      dmg = -1
   end

   return dmg
end

function sub_1B432(attacker, target)

end

function actor_get_weapon(attacker, target)
   local int_test = false
   local selected_weapon = attacker
   if not actor_has_bad_alignment(attacker) then
      if math.random(1, 0x1e) <= actor_int_adj(attacker) then
         int_test = true
      end
   end
   local range = actor_get_combat_range(attacker, target.x, target.y)

   for obj in actor_inventory(actor) do
      if get_weapon_damage(obj) > 0 and get_weapon_range(obj) >= range then
         if (obj.obj_n ~= 129 and obj.obj_n ~= 261) --OBJ_WEED_SPRAYER, OBJ_SPRAY_GUN
                 or (obj.quality ~= 0 and obj.invisible)
                 or (obj.quality == 0 and (is_plant_obj(target) or (target.luatype=="actor" and (is_actor_stat_bit_set(target.obj_n, 7) and not is_actor_stat_bit_set(target.obj_n, 14))))) then

            local weapon_requires_int = false
            if obj.obj_n == 240 --OBJ_HEAT_RAY_GUN
               or obj.obj_n == 241 --OBJ_FREEZE_RAY_GUN
               or obj.obj_n == 45 then
               weapon_requires_int = true
            end

            --if int_test == true and weapon_requires_int == true or obj.obj_n == 43 and not sub_1B432(attacker, target) then --OBJ_SHOTGUN
         end

      end
   end

end

function attack_dex_saving_throw(attacker, defender, weapon)
   if defender == nil or defender.luatype == "obj" then
      return true
   end

   local attacker_value
   if weapon.luatype == "actor" and is_actor_stat_bit_set(weapon.obj_n, 5) then
      attacker_value = actor_str_adj(attacker)
   else
      attacker_value = actor_dex_adj(attacker)
   end

   local defender_value = actor_dex_adj(defender)

   if math.random(1, 30) >= math.floor((defender_value + 30 - attacker_value) / 2) then
      return true
   end

   return false
end

function out_of_ammo(attacker, weapon, print_message) -- untested function

   local weapon_obj_n = weapon.obj_n

   if ((weapon_obj_n == 41 or weapon_obj_n == 42) and Actor.inv_has_obj_n(attacker, 57) == false) --derringer, revolver, pistol rounds
           or (weapon_obj_n == 43 and Actor.inv_has_obj_n(attacker, 58) == false) --shotgun, shotgun shell
           or (weapon_obj_n == 44 and Actor.inv_has_obj_n(attacker, 59) == false) --rifle, rifle round
           or (weapon_obj_n == 45 and weapon.quality == 0 and (Actor.inv_has_obj_n(attacker, 58) == false or Actor.inv_has_obj_n(attacker, 59) == false)) --belgian combine (combine), shotgun shell, rifle round
           or (weapon_obj_n == 45 and weapon.quality == 1 and Actor.inv_has_obj_n(attacker, 59) == false) --belgian combine (rifle), rifle round
           or (weapon_obj_n == 45 and weapon.quality == 2 and Actor.inv_has_obj_n(attacker, 58) == false) --belgian combine (shotgun), shotgun shell
           or (weapon_obj_n == 46 and Actor.inv_has_obj_n(attacker, 60) == false) --elephant gun, elephant gun round
           or (weapon_obj_n == 47 and Actor.inv_has_obj_n(attacker, 63) == false) --sling, sling stone
           or ((weapon_obj_n == 240 or weapon_obj_n == 241 or weapon_obj_n == 129 or weapon_obj_n == 261) and weapon.qty == 0) then --heat ray gun, freeze ray gun, weed sprayer, spray gun
      if(print_message) then
         printl("OUT_OF_AMMUNITION")
         play_md_sfx(5)
      end
      return true
   end

   if weapon_obj_n == 48 and Actor.inv_has_obj_n(attacker, 64) == false then --bow, arrows
      if(print_message) then
         printl("OUT_OF_ARROWS")
         play_md_sfx(5)
      end
      return true
   end

   return false
end

function attack_with_freezeray(actor, target_actor, damage)
   if actor_tbl[target_actor.obj_n] ~= nil
           and (is_actor_stat_bit_set(target_actor.obj_n, 14) or is_actor_stat_bit_set(target_actor.obj_n, 7)) then
      target_actor.paralyzed = true
      printfl("ACTOR_PARALYZED", target_actor.name)
      if not is_actor_stat_bit_set(target_actor.obj_n, 7)
              or is_actor_stat_bit_set(target_actor.obj_n, 14) then
         hit_target(target_actor, BLUE_HIT_TILE)
      else
         actor_take_hit(actor, target_actor, damage, 1)
      end
   else
      printl("IT_HAS_NO_EFFECT")
   end
end

function check_ammo(actor, weapon)
   local obj_n = weapon.obj_n
   if obj_n == 47 and not Actor.inv_has_obj_n(actor, 63) then --OBJ_SLING, OBJ_SLING_STONE
      return 1
   end

   if (obj_n == 41 or obj_n == 42) and not Actor.inv_has_obj_n(actor, 57) then --OBJ_DERRINGER, OBJ_REVOLVER, OBJ_PISTOL_ROUND
      return 1
   end

   if obj_n == 46 and not Actor.inv_has_obj_n(actor, 60) then --OBJ_ELEPHANT_GUN, OBJ_ELEPHANT_GUN_ROUND
      return 1
   end

   if obj_n == 44 and not Actor.inv_has_obj_n(actor, 59) then --OBJ_RIFLE, OBJ_RIFLE_ROUND
      return 1
   end

   if obj_n == 43 and not Actor.inv_has_obj_n(actor, 58) then --OBJ_SHOTGUN, OBJ_SHOTGUN_SHELL
      return 1
   end

   if (obj_n == 129 or obj_n == 261) and weapon.qty ~= 0 then --OBJ_WEED_SPRAYER, OBJ_SPRAY_GUN
      return 1
   end

   return 0
end

function attack_target_with_weapon(actor, target_x, target_y, weapon)
   local target_range = actor_get_combat_range(actor, target_x, target_y)
   local weapon_range = get_weapon_range(weapon)

   if target_range > weapon_range then
      return 2 --out of range
   end

   local ret = check_ammo(actor, weapon)
   if ret ~= 0 then
      return ret
   end

   local obj_n = weapon.obj_n


   local var_10 = 0
   local var_12 = 0
   --OBJ_HEAT_RAY_GUN
   --OBJ_FREEZE_RAY_GUN
   --OBJ_BELGIAN_COMBINE
   if obj_n == 240 or obj_n == 241 or obj_n == 45 then
      if weapon.quality < 2 then
         var_10= 1
      end

      if weapon.quality ~= 1 then
         var_12 = 1
      end

      if obj_n == 45 then --OBJ_BELGIAN_COMBINE
         if var_10 ~= 0 then
            if not Actor.inv_has_obj_n(actor, 59) then --OBJ_RIFLE_ROUND
               var_10 = 0
            end
         end
         if var_12 ~= 0 then
            if not Actor.inv_has_obj_n(actor, 58) then --OBJ_SHOTGUN_SHELL
               var_12 = 0
            end
         end
      else
         if weapon.qty == 0 then
            return 1
         end
         if weapon.qty < (var_12 * 4) + var_10 then
            var_12 = 0
         end
      end

      if var_10 == 0 and var_12 == 0 then
         return 1
      end
   else
      if obj_n == 43 then --OBJ_SHOTGUN
         var_10 = 0
         var_12 = 1
      else
         var_10 = 1
         var_12 = 0
      end
   end

   local damage_mode = 0
   if obj_n == 240 then --OBJ_HEAT_RAY_GUN
      damage_mode = 3
   end

   if obj_n == 241 then --OBJ_FREEZE_RAY_GUN
      damage_mode = 1
   end

   g_attack_target = g_selected_obj

   if var_10 == 0 then
      spread_weapon_damage(actor, target_x, target_y, weapon)
      return 0
   end

   local is_ranged_attack = false
   if target_range > 1 then
      is_ranged_attack = true
   end

   if (obj_n >= 40 and obj_n <= 48) --OBJ_CUPIDS_BOW_AND_ARROWS, OBJ_BOW
      or obj_n == 129 --OBJ_WEED_SPRAYER
         or obj_n == 261 --OBJ_SPRAY_GUN
         or obj_n == 240 --OBJ_HEAT_RAY_GUN
         or obj_n == 241 --OBJ_FREEZE_RAY_GUN
   then
      is_ranged_attack = true
   end

   local damage
   if weapon.luatype == "actor" then
      damage = actor_get_damage(actor)
      if damage == nil then
         damage = 1
      end
   else
      damage = get_weapon_damage(weapon)
      if damage < 0 then
         damage = 1
      end
   end

   local does_damage

   local target =  find_rockworm_actor(g_attack_target)
   if target ~= nil
           and target.luatype == "actor"
           and obj_n ~= 129 --OBJ_WEED_SPRAYER
           and obj_n ~= 261 then --OBJ_SPRAY_GUN
      does_damage = attack_dex_saving_throw(actor, target, weapon)
   end

   if is_ranged_attack then
      fire_range_based_weapon(actor, target_x, target_y, weapon)
   end

   target = find_rockworm_actor(g_attack_target)

   if not is_ranged_attack and map_is_on_screen(actor.xyz) then
      play_md_sfx(0)
   end

   if does_damage == nil then
      does_damage = true
      if target ~= nil
              and target.luatype == "actor"
              and obj_n ~= 129 --OBJ_WEED_SPRAYER
              and obj_n ~= 261 then --OBJ_SPRAY_GUN
         does_damage = attack_dex_saving_throw(actor, target, weapon)
      end
   end

   if not does_damage then
      play_md_sfx(3)
   end

   if does_damage
      and target ~= nil
      and (target.luatype == "obj" or target.actor_num ~= actor.actor_num) then
      if obj_n == 241 then --OBJ_FREEZE_RAY_GUN
         if target.obj_n == 160 and target.frame_n == 1 then
            printl("THE_WATER_FREEZES")
            target.frame_n = 2
         else
            if target.luatype == "actor" then
               attack_with_freezeray(actor, target, damage)
            end
         end
      elseif obj_n == 129 or obj_n == 261 then --OBJ_WEED_SPRAYER, OBJ_SPRAY_GUN
         if weapon.quality ~= 0 then
            if target.luatype == "actor" and target.obj_n == 145 then --OBJ_MONSTER_FOOTPRINTS
               target.obj_n = 364 --OBJ_PROTO_MARTIAN
               printfl("BECOMES_VISIBLE", target.name)
            elseif target.luatype == "obj" and target.invisible then
               target.invisible = false
               printfl("BECOMES_VISIBLE", target.name)
            else
               printl("IT_HAS_NO_EFFECT")
            end
         else
            if actor_tbl[target.obj_n] ~= nil
               and not is_actor_stat_bit_set(target.obj_n, 14)
               and is_actor_stat_bit_set(target.obj_n, 7) then
                  actor_take_hit(actor, target, damage, 2)
            else
               printl("IT_HAS_NO_EFFECT")
            end
         end
      elseif obj_n == 40 then --OBJ_CUPIDS_BOW_AND_ARROWS
         if target.luatype == "actor" and target.align ~= ALIGNMENT_GOOD then
            target.old_align = target.align
            target.align = ALIGNMENT_GOOD
            target.charmed = true
            printfl("ACTOR_CHARMED", target.name)
         end
      else
         actor_take_hit(actor, target, damage, damage_mode)
      end

   end

   if var_12 ~= 0 then
      spread_weapon_damage(actor, target_x, target_y, weapon)
   end

   return 0
end

local spread_weapon_sfx_tbl = {
   [0x2b]=8,
   [0x2d]=8,
   [0xf0]=0xa,
   [0xf1]=0xa,
}
local spread_weapon_tile_num_tbl = {
   [0x2b]=0x106,
   [0x2d]=0x106,
   [0xf0]=0x14e,
   [0xf1]=0x14f,
}
local spread_weapon_damage_tbl = {
   [0x2b]=0x14,
   [0x2d]=0x14,
   [0xf0]=0x19,
   [0xf1]=0xa,
}

function spread_weapon_damage(actor, target_x, target_y, weapon)
   if spread_weapon_sfx_tbl[weapon.obj_n] ~= nil then
      play_md_sfx(spread_weapon_sfx_tbl[weapon.obj_n])
   end

   --FIXME spread weapon anim here.
   local hit_items = projectile_anim_multi (spread_weapon_tile_num_tbl[weapon.obj_n], actor.x, actor.y, {{x=target_x, y=target_y, z=actor.z}, {x=target_x+1, y=target_y-1, z=actor.z}}, 2, 1, 0)

   local k, v
   for k,v in pairs(hit_items) do
      if weapon.obj_n == 241 then --OBJ_FREEZE_RAY_GUN
         if v.obj_n == 160 and v.frame_n == 1 then --OBJ_EMPTY_BUCKET
            printl("THE_WATER_FREEZES")
            v.frame_n = 2
         elseif v.luatype == "actor" then
            if math.random(1, 0x2d) > actor_dex_adj(v) then
               attack_with_freezeray(actor, v, 10)
            else
               printfl("ACTOR_DODGES", v.name)
               play_md_sfx(3)
            end
         end
      elseif v.luatype == "obj" or math.random(1, 0x2d) > actor_dex_adj(v) then
         local dmg_mode = 0
         if weapon.obj_n == 240 then --OBJ_HEAT_RAY_GUN
            dmg_mode = 3
         end
         actor_take_hit(actor, v, spread_weapon_damage_tbl[weapon.obj_n], dmg_mode)
      else
         printfl("ACTOR_DODGES", v.name)
         play_md_sfx(3)
      end
   end

   if weapon.obj_n == 43 or weapon.obj_n == 45 then --OBJ_SHOTGUN, OBJ_BELGIAN_COMBINE
      Actor.inv_remove_obj_qty(actor, 58, 1) --OBJ_SHOTGUN_SHELL
   else
      weapon.qty = weapon.qty - 4
      if weapon.qty < 0 then
         weapon.qty = 0
      end
   end

end

local projectile_tbl = {
   [41]={tile_num=0x103, sfx_id=7, ammo_obj_n=57},
   [42]={tile_num=0x103, sfx_id=7, ammo_obj_n=57},
   [44]={tile_num=0x103, sfx_id=8, ammo_obj_n=59},
   [45]={tile_num=0x103, sfx_id=8, ammo_obj_n=59},
   [46]={tile_num=0x103, sfx_id=8, ammo_obj_n=60},
   [47]={tile_num=0x23E, sfx_id=6, ammo_obj_n=63},
   [48]={tile_num=0x23F, sfx_id=6, ammo_obj_n=64},
   [40]={tile_num=0x23F, sfx_id=6, ammo_obj_n=-3},
   [240]={tile_num=0x16B, sfx_id=10, ammo_obj_n=-2},
   [241]={tile_num=0x16A, sfx_id=10, ammo_obj_n=-2},
   [129]={tile_num=0x10A, sfx_id=6, ammo_obj_n=-2},
   [261]={tile_num=0x10A, sfx_id=6, ammo_obj_n=-2},
   [313]={tile_num=0x10B, sfx_id=50, ammo_obj_n=-3},
   [366]={tile_num=0x16F, sfx_id=34, ammo_obj_n=-3},
   [386]={tile_num=0x16C, sfx_id=34, ammo_obj_n=-3},
   [364]={tile_num=0x16D, sfx_id=6, ammo_obj_n=-3},
   [384]={tile_num=0x16E, sfx_id=6, ammo_obj_n=-3},
}

function fire_range_based_weapon(attacker, target_x, target_y, weapon)
   local projectile_info = projectile_tbl[weapon.obj_n]
   if projectile_info == nil then
      projectile_info = {tile_num=weapon.tile_num, sfx_id=0, ammo_obj_n=-1 }
   end

   play_md_sfx(projectile_info.sfx_id)

   projectile_anim(projectile_info.tile_num, attacker.x, attacker.y, target_x, target_y, 4, false, 0)

   if projectile_info.ammo_obj_n > 0 then
      Actor.inv_remove_obj_qty(attacker, projectile_info.ammo_obj_n, 1)
   elseif projectile_info.ammo_obj_n == -2 then
      weapon.qty = weapon.qty - 1
   elseif projectile_info.ammo_obj_n == -1 and actor_get_combat_range(attacker, target_x, target_y) > 1 then
      if is_open_water_at_loc(target_x, target_y, attacker.z) then
         Obj.removeFromEngine(weapon)
      else
         Obj.moveToMap(weapon, target_x, target_y, attacker.z)
      end
      --FIXME original updated readied weapons here. We might also need to do that.
   end
end
