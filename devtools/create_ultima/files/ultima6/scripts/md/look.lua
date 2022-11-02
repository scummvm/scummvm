function look_pocketwatch(obj)
   printl("THE_TIME_IS")
   local am = true
   local hour = clock_get_hour()
   local minute = clock_get_minute()
   if hour >= 12 then
   am = false
   end
   if hour > 12 then
   hour = hour - 12
   end
   local time = "TIME_AM"
   if am == false then
   time = "TIME_PM"
   end

   printfl(time, hour, minute)
end

function look_cannon(obj)
   if obj.quality ~= 0 then
      printl("THE_CANNON_WILL_FIRE_STEEL_CANNON_BALLS")
   end
end

function look_barrow(obj)
   if obj.qty == 0 then
      printl("IT_IS_EMPTY")
      return
   end

   local quality = obj.quality
   local material
   if quality == 1 then
      material = i18n("DIRT")
   elseif quality == 2 then
      material = i18n("ROCK")
   elseif quality == 3 then
      material = i18n("IRON_ORE")
   elseif quality == 4 then
      material = i18n("COAL")
   end

   if obj.qty == 1 then
      printfl("IT_HAS_1_LOAD_OF", material)
   else
      printfl("IT_HAS_N_LOADS_OF", obj.qty, material)
   end
end

function get_lat_long_string(x, y)
   local lat_str = "N"
   local long_str = "W"

   local lat = math.modf(((y - 512) * 240) / 1024)
   local long = math.modf(((x - 512) * 360) / 1024)

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

   return lat..lat_str.." "..long..long_str
end

function look_marker_flag(obj)
   local names = {
   [1]="Coprates Chasma",
   [2]="Arsia Mons",
   [3]="Pavonis Mons",
   [4]="Ascraeus Mons",
   [5]="Alber Tholus",
   [6]="Elysium Mons",
   [7]="Hecates Tholus",
   [8]="Terra Sirenum",
   [9]="Noctis Labyrinthus",
   [10]="Deuteronicus Mensae",
   [11]="Syrtis Major Planum",
   [12]="Olympus Mons",
   }

   if obj.z == 0 then
      if obj.quality <= 12 then
         if obj.quality ~= 0 then
            print(names[obj.quality])
            print(" "..get_lat_long_string(obj.x,obj.y).."\n")
         end
      else
         printl("AARGH")
      end
   end
end

function look_broken_strap(obj)
   local spector = Actor.get(2)
   Actor.set_talk_flag(spector, 6)
   Actor.talk(spector)
end

function look_metal_woman(obj)
   if obj.quality ~= 0 then
      printl("IT_HAS_A_HEARTSTONE")
   end
end

function look_covered_martian_seed(obj)
   if obj.frame_n < 4 then
      local quality = obj.quality
      if quality == 15 then
         printl("IT_IS_GROWING")
      elseif quality == 16 then
         printl("IT_IS_RIPE")
      else
         printl("IT_IS_NOT_GROWING")
      end
   end
end

function look_sprayer_system(obj)
   local quality = obj.quality

   if bit32.btest(quality, 1) then
      local actor = Actor.get(0x3e)
      if Actor.get_talk_flag(actor, 5) == true then
         if bit32.btest(quality, 2) then
            printl("IT_IS_READY_TO_USE")
         else
            printl("THE_RUBBER_IN_THE_SPRAYER_SYSTEM_IS_NOT_FLESH_COLORED")
         end
      else
         printl("IT_IS_READY_TO_USE")
      end
   else
      printl("THERE_IS_NO_RUBBER_IN_THE_SPRAYER_SYSTEM")
      if Actor.get_talk_flag(actor, 5) == true and bit32.btest(quality, 2) == false then
         printl("IT_IS_ALSO_NOT_FLESH_COLORED")
      end
   end
end

function look_cabinet(obj)
   local quality = obj.quality
   if quality == 0 or quality > 7 then
      printl("YOU_CANNOT_DECIPHER_WHAT_IT_CONTROLS")
   else
      printl("CABINET_"..quality)
   end
end

function look_tracking_motor(obj)
   local quality = obj.quality

   if bit32.btest(quality, 1) and obj.on_map == true then
      printl("IT_APPEARS_TO_BE_LOOSE")
   end

   if bit32.btest(quality, 2) then
      printl("IT_APPEARS_TO_BE_BROKEN")
   end

end

function look_panel(obj)
   local qty = obj.qty
   if qty == 0 then
      local frame_n = obj.frame_n
      if frame_n == 0 then
         printl("THESE_APPEAR_TO_BE_MECHANICAL_CONTROLS")
      elseif frame_n == 1 then
         printl("THESE_APPEAR_TO_BE_VALVE_CONTROLS")
      elseif frame_n == 2 then
         printl("THESE_APPEAR_TO_BE_ELECTRICAL_CONTROLS")
      end
   elseif qty == 4 then
      printl("IT_APPEARS_TO_CONTROL_A_DREAM_MACHINE")
   elseif qty == 5 then
      printl("IT_APPEARS_TO_CONTROL_A_SPRAY_SYSTEM")
   elseif qty == 6 then
      printl("IT_APPEARS_TO_CONTROL_THE_RUBY_LENS_SYSTEM")
   elseif qty == 7 then
      printl("IT_APPEARS_TO_CONTROL_THE_CISTERN_VALVES")
   else
      printl("YOU_CANNOT_DECIPHER_ITS_PURPOSE")
   end

   local quality = obj.quality
   if bit32.btest(quality, 1) and obj.on_map == true then
      printl("THE_PANEL_IS_LOOSE")
   end

   if bit32.btest(quality, 2) then
      printl("THE_PANEL_IS_BROKEN")
   end
end

function print_number_of_charges(qty)
   if qty == 1 then
      printl("AND_HAS_1_CHARGE_LEFT")
   else
      printfl("AND_HAS_N_CHARGES_LEFT", qty)
   end
end

function look_portable_sprayer(obj)
   local contents
   if obj.quality == 0 then
      contents = tile_get_description(649)
   else
      contents = tile_get_description(640)
   end

   printfl("IT_IS_LOADED_WITH", contents)
   print_number_of_charges(obj.qty)
end

function get_weapon_mode_string(obj)
   local mode
   local quality = obj.quality
   if quality == 0 then
      mode = i18n("COMBINATION")
   elseif quality == 1 then
      mode = i18n("RIFLE")
   else
      mode = i18n("SHOTGUN")
   end

   return mode
end

function look_ray_gun(obj)
   printfl("IT_IS_SET_TO", get_weapon_mode_string(obj))
   print_number_of_charges(obj.qty)
end

function look_belgian_combine(obj)
   printfl("IT_IS_SET_TO", get_weapon_mode_string(obj))
   print(".\n")
end

function look_switch_bar(obj)
   local quality = obj.quality
   if quality == 1 then
      printl("IT_SEEMS_TO_BE_LOOSE")
   elseif quality == 2 then
      printl("IT_IS_NOT_INSTALLED")
   end
end

function look_light_source(obj)
   local obj_n = obj.obj_n
   local qty = obj.qty
   local quality = obj.quality

   if (obj_n == 109 or obj_n == 110) and qty > 1 then
      return
   end

   if quality > 30 then
      printl("PLENTY_OF")
   elseif quality > 6 and quality <= 30 then
      printfl("IT_HAS_N_MORE_MINUTES_OF", quality)
   elseif quality > 0 and quality <= 6 then
      printl("ITS_ALMOST_OUT_OF")
   elseif obj_n == 115 or obj_n == 117 or obj_n == 116 or obj_n == 118 then
      printl("NO")
   else
      printl("PLENTY_OF")
   end

   if obj_n == 115 or obj_n == 117 or obj_n == 116 or obj_n == 118 then
      printl("FUEL")
   else
      printl("WICK")
   end

end

function look_door(obj)
   local quality = obj.quality
   if quality >= 128 then
      quality = quality - 128
      if quality >= 64 then
         printl("IT_IS_APPARENTLY_RUSTED_SHUT")
      else
         printl("IT_IS_APPARENTLY_LOCKED")
      end
   end
end

function look_obelisk(obj)
   if obj.quality == 0 then
      return
   end

   local ui_style = game_get_ui_style()

   canvas_show()
   canvas_hide_all_sprites()
   canvas_set_opacity(0xff);
   canvas_set_update_interval(25)
   canvas_rotate_game_palette(true)

   local obelisk = sprite_new(nil, 184, 0, true)

   local text_sprite
   --local text_sprite_bg

   if ui_style == UI_STYLE_ORIG then
      canvas_set_solid_bg(false)
   else
      text_sprite = sprite_new(nil, 8, 164, true)
      text_sprite.text_align = 2
      text_sprite.text_color = 15
      text_sprite.text = "Obelisk."
      obelisk.x = 96
      obelisk.y = 41
   end

   obelisk.image = image_load("mdream.lzc", obj.quality-1)

   local input = nil
   while input == nil do
      canvas_update()
      input = input_poll()
      if input ~= nil then
         break
      end
   end

   canvas_set_solid_bg(true)
   canvas_rotate_game_palette(false)
   canvas_hide()
end

local look_usecode = {
[45]=look_belgian_combine,
[91]=look_pocketwatch,
[98]=look_pocketwatch,
[109]=look_light_source, -- OBJ_TORCH
[110]=look_light_source, -- OBJ_LIT_TORCH
[111]=look_light_source, -- OBJ_CANDLESTICK
[112]=look_light_source, -- OBJ_LIT_CANDLE
[113]=look_light_source, -- OBJ_CANDELABRA
[114]=look_light_source, -- OBJ_LIT_CANDELABRA
[115]=look_light_source, -- OBJ_OIL_LAMP
[116]=look_light_source, -- OBJ_LIT_OIL_LAMP
[117]=look_light_source, -- OBJ_LANTERN
[118]=look_light_source, -- OBJ_LIT_LANTERN
[129]=look_portable_sprayer, --OBJ_WEED_SPRAYER
[172]=look_marker_flag,
[179]=look_door, --OBJ_CLOSED_DOOR
[227]=look_door, --OBJ_DOOR3
[240]=look_ray_gun, --OBJ_HEAT_RAY_GUN
[241]=look_ray_gun, --OBJ_FREEZE_RAY_GUN
[251]=look_covered_martian_seed,
[261]=look_portable_sprayer, --OBJ_SPRAY_GUN
[268]=look_barrow, --OBJ_MARTIAN_WHEEL_BARROW
[276]=look_sprayer_system,
[287]=look_metal_woman,
[292]=look_obelisk,
[314]=look_tracking_motor,
[333]=look_cannon,
[410]=look_barrow, --OBJ_RAIL_CAR
[411]=look_switch_bar,
[457]=look_cabinet,
[458]=look_panel,
[460]=look_broken_strap,
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
