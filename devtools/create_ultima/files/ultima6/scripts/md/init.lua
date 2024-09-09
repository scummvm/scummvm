local lua_file = nil

--load common functions
lua_file = nuvie_load("common/common.lua"); lua_file();

OBJLIST_OFFSET_HOURS_TILL_NEXT_HEALING  = 0x1cf2
OBJLIST_OFFSET_PREV_PLAYER_X            = 0x1d03
OBJLIST_OFFSET_PREV_PLAYER_Y            = 0x1d04
OBJLIST_OFFSET_1D22_UNK                 = 0x1d22
OBJLIST_OFFSET_DREAM_MODE_FLAG          = 0x1d29

OBJLIST_OFFSET_BERRY_COUNTERS           = 0x1d2f
OBJLIST_OFFSET_DREAM_STAGE              = 0x1d53

function dbg(msg_string)
   --io.stderr:write(msg_string)
end

g_hours_till_next_healing = 0
g_in_dream_mode = false

local g_selected_obj
local g_attack_target

function update_watch_tile()
   local anim_index = get_anim_index_for_tile(616) --616 = watch tile
   if anim_index ~= nil then
      local new_watch_tile = 416 + clock_get_hour() % 12
      anim_set_first_frame(anim_index, new_watch_tile)
   end
end

function load_game()
   g_selected_obj = nil
   g_attack_target = nil

   objlist_seek(OBJLIST_OFFSET_HOURS_TILL_NEXT_HEALING)
   g_hours_till_next_healing = objlist_read1()

   objlist_seek(OBJLIST_OFFSET_DREAM_MODE_FLAG)
   g_in_dream_mode = bit32.btest(objlist_read2(), 0x10)
   map_enable_temp_actor_cleaning(not g_in_dream_mode)

   if g_in_dream_mode then
      lock_inventory_view(Actor.get(0))
   end

   objlist_seek(OBJLIST_OFFSET_DREAM_STAGE)
   g_current_dream_stage = objlist_read2()

   objlist_seek(OBJLIST_OFFSET_PREV_PLAYER_X)
   g_prev_player_x = objlist_read1()

   objlist_seek(OBJLIST_OFFSET_PREV_PLAYER_Y)
   g_prev_player_y = objlist_read1()

   objlist_seek(OBJLIST_OFFSET_1D22_UNK)
   g_objlist_1d22_unk = objlist_read1()

   update_watch_tile()
end

function save_game()
   objlist_seek(OBJLIST_OFFSET_HOURS_TILL_NEXT_HEALING)
   objlist_write1(g_hours_till_next_healing)

   objlist_seek(OBJLIST_OFFSET_DREAM_MODE_FLAG)
   local bytes = objlist_read2()
   if g_in_dream_mode then
      bytes = bit32.bor(bytes, 0x10)
   else
      bytes = bit32.band(bytes, 0xFFEF)
   end
   objlist_seek(OBJLIST_OFFSET_DREAM_MODE_FLAG)
   objlist_write2(bytes)

   objlist_seek(OBJLIST_OFFSET_DREAM_STAGE)
   objlist_write2(g_current_dream_stage)

   objlist_seek(OBJLIST_OFFSET_PREV_PLAYER_X)
   objlist_write1(g_prev_player_x)

   objlist_seek(OBJLIST_OFFSET_PREV_PLAYER_Y)
   objlist_write1(g_prev_player_y)

   objlist_seek(OBJLIST_OFFSET_1D22_UNK)
   objlist_write1(g_objlist_1d22_unk)
end



local g_container_obj_tbl = {
[80]=1, [81]=1, [82]=1,
[83]=1, [85]=1, [86]=1,
[87]=1, [89]=1,[304]=1,
[139]=1,[341]=1,[257]=1,
[104]=1,[284]=1,[285]=1
}

function is_blood(obj_num)
   if obj_num == 334 or obj_num == 335 or obj_num == 336 then --OBJ_BLOOD, OBJ_ICHOR, OBJ_SAP
      return true
   end

   return false
end

function is_container_obj(obj_num)
   if g_container_obj_tbl[obj_num] ~= nil then
      return true
   end
   return false
end

--OBJ_CLOSED_DOOR, OBJ_GLOW_WORM, OBJ_DEVIL_POD, OBJ_DEVIL_PLANT, OBJ_GLASS_PITCHER
local attackable_obj_tbl = {
   [0xB3]=1,
   [0x184]=1,
   [0x133]=1,
   [0x12C]=1,
   [0xD9]=1,
}

function is_obj_attackable(obj)
   return attackable_obj_tbl[obj.obj_n] ~= nil
end

--OBJ_VINE, OBJ_TREE, OBJ_PORCUPOD, OBJ_FLOWER, OBJ_HARD_SHELLED_PLANT
--OBJ_DEVIL_PLANT, OBJ_DEVIL_POD, OBJ_HEDGE
local plant_obj_tbl = {
   [0xCD]=1,
   [0x198]=1,
   [0xA8]=1,
   [0xAA]=1,
   [0xAB]=1,
   [0x12C]=1,
   [0x133]=1,
   [0x12B]=1,
}

function is_plant_obj(obj)
   return plant_obj_tbl[obj.obj_n] ~= nil
end

--OBJ_DOLLAR, OBJ_RUBLE, OBJ_PHOTOGRAPH, OBJ_THREAD, OBJ_BOX_OF_CIGARS
--OBJ_MATCH, OBJ_BOOK, OBJ_MAP, OBJ_NOTE, OBJ_WORMSBANE_SEED
--OBJ_SCROLL, OBJ_LIVE_MARTIAN_SEED, OBJ_PILE_OF_COAL
--OBJ_HUGE_LUMP_OF_COAL, OBJ_CHUNK_OF_ICE
local burnable_obj_tbl = {
   [0x18]=1,
   [0x84]=1,
   [0x0B9]=1,
   [0x3E]=1,
   [0x65]=1,
   [0x6B]=1,
   [0x94]=1,
   [0x96]=1,
   [0x97]=1,
   [0x9E]=1,
   [0x0F3]=1,
   [0x0FC]=1,
   [0x1BC]=1,
   [0x1BF]=1,
   [0x100]=1,
}

function is_obj_burnable(obj)
   return burnable_obj_tbl[obj.obj_n] ~= nil
end

function is_open_water_at_loc(x, y, z)
   local tile_num = map_get_tile_num(x, y, z)
   local is_water = tile_get_flag(tile_num, 1, 0)
   if not is_water then
      return false
   end

   if not tile_get_flag(tile_num, 1, 1) then -- not blocked
      return false
   end

   for obj in objs_at_loc(location) do
      tile_num = obj.tile_num
      --FIXME original does this too
      -- sub     ax, word_40FA2
      if tile_get_flag(tile_num, 3, 1) or tile_get_flag(tile_num, 3, 2) then --SUPPORT_OBJECT or FORCE_PASSABLE
         return false
      end
   end

   return true
end

function search(obj)
   if obj.on_map == false then
      return
   end

   local found_obj = false
   local child
   local first_loop = true
   local prev_obj = nil
   for child in container_objs(obj) do
      if prev_obj ~= nil then
         printfl("SEARCH_NEXT_OBJ", prev_obj.look_string)
         Obj.moveToMap(prev_obj, obj.x, obj.y, obj.z)
      end

      if first_loop == true then
         found_obj = true
         printfl("SEARCHING_HERE_YOU_FIND", child.look_string)
         Obj.moveToMap(child, obj.x, obj.y, obj.z)
      else
         prev_obj = child
      end

      script_wait(50)
      first_loop = false
   end

   if prev_obj ~= nil then
      printfl("SEARCH_LAST_OBJ", prev_obj.look_string)
      Obj.moveToMap(prev_obj, obj.x, obj.y, obj.z)
   end

   if found_obj == false then
      printl("SEARCHING_HERE_YOU_FIND_NOTHING")
   else
      print(".\n")
   end

end

--tile_num, readied location
local g_readiable_objs_tbl = {
-- 0 = head
[512] = 0, --cap
[513] = 0, --cowboy hat
[514] = 0, --pith hat
[515] = 0, --military hat
[516] = 0, --derby
-- 1 = neck
[517] = 1, --kerchief
[518] = 1, --silk scarf
[519] = 1, --muffler
[1048] = 1, --martian jewelry
[1049] = 1, --martian jewelry
[1050] = 1, --martian jewelry
[1051] = 1, --martian jewelry
[1052] = 1, --martian jewelry
[1053] = 1, --martian jewelry
[1054] = 1, --martian jewelry
[1055] = 1, --martian jewelry
-- 3 = 1 handed
[527] = 3, --bloody saber
[552] = 3, --derringer
[553] = 3, --revolver
[560] = 3, --hatchet
[561] = 3, --axe
[562] = 3, --ball-peen hammer
[565] = 3, --knife
[566] = 3, --machete
[567] = 3, --saber
[622] = 3, --pry bar
[650] = 3, --weed sprayer
[667] = 3, --tongs
[1068] = 3, --martian ritual pod knife
-- 2 = torso
[528] = 2, --cloth jacket
[529] = 2, --wool sweater
[530] = 2, --cape
[531] = 2, --duster
[532] = 2, --wool overcoat
[533] = 2, --sheepskin jacket
[534] = 2, --arctic parka
[608] = 2, --electric belt
-- 3 = shield hand
[630] = 3, --torch
[631] = 3, --lit torch
[632] = 3, --candlestick
[633] = 3, --lit candlestick
[634] = 3, --candelabra
[635] = 3, --lit candelabra
[636] = 3, --oil lamp
[637] = 3, --lit oil lamp
[638] = 3, --lantern
[639] = 3, --lit lantern
-- 6 = UNK pants or dress FIXME: putting this here for now
[536] = 6, --cotton pants
[537] = 6, --cotton dress
[538] = 6, --denim jeans
[539] = 6, --wool pants
[540] = 6, --wool dress
[541] = 6, --chaps and jeans
-- 7 = feet
[520] = 7, --man's shoes
[521] = 7, --woman's shoes
[522] = 7, --riding boots
[523] = 7, --ruby slippers
[524] = 7, --thigh boots
[525] = 7, --hip boots
[526] = 7, --winged shoes
[542] = 7, --man's shoes
-- 4 = two handed
[551] = 4, --Cupid's bow and arrows
[554] = 4, --shotgun
[555] = 4, --rifle
[556] = 4, --Belgian combine
[557] = 4, --elephant gun
[558] = 4, --sling
[559] = 4, --bow
[563] = 4, --sledge hammer
[576] = 4, --pick
[577] = 4, --shovel
[578] = 4, --hoe
[579] = 4, --rake
[580] = 4, --pitchfork
[581] = 4, --cultivator
[582] = 4, --scythe
[583] = 4, --saw
[717] = 4, --throw rug
[718] = 4, --red cape
[1066] = 4, --heat ray gun
[1067] = 4, --freeze ray gun
[1093] = 4, --spray gun
[1095] = 4, --martian hoe (couldn't be equipped in original)
[1096] = 4, --martian scythe (couldn't be equipped in original)
[1097] = 4, --martian pitchfork (couldn't be equipped in original)
[1098] = 4, --martian rake (couldn't be equipped in original)
[1099] = 4, --martian shovel (couldn't be equipped in original)
[1188] = 4, --M60 machine gun
[1206] = 4, --martian pick (couldn't be equipped in original)
[1897] = 4, --pool cue
-- 5 = finger gloves and bracelets FIXME: putting this here for now
[544] = 5, --lady's silk gloves
[545] = 5, --driving gloves
[546] = 5, --cotton work gloves
[547] = 5, --work gloves
[548] = 5, --wool mittens
[549] = 5, --rubber gloves
[550] = 5 --welding gloves
}

function wrap_coord(coord, level)
   local map_stride = 1024
   if level ~= 0 then
      map_stride = 256
   end

   if coord < 0 then
      return map_stride + coord
   end

   return coord % map_stride
end

function can_move_obj(obj, rel_x, rel_y)
   local z = obj.z
   return map_can_put_obj(wrap_coord(obj.x+rel_x,z), wrap_coord(obj.y+rel_y,z), z)
end

function obj_get_readiable_location(obj)
   if g_readiable_objs_tbl[obj.tile_num] ~= nil then
      return g_readiable_objs_tbl[obj.tile_num]
   end

   return -1
end

function update_lamp_posts()
   --Turn lamps on/off if we have power and it is dark.
   local frame_n = 3
   local hour = clock_get_hour()
   if Actor.get_talk_flag(0x73, 4) and (hour < 6 or hour > 17) then
      frame_n = 7
   end

   local loc = player_get_location()
   for obj in find_obj(loc.z, 228) do --OBJ_LAMP_POST
      if obj ~= nil then
         obj.frame_n = frame_n
      end
   end
end

local PLAY_ASYNC = true
function play_md_sfx(sfx_id, play_async)
   --FIXME
end

function play_door_sfx()
   --FIXME
end

function create_object_needs_quan(obj_n)
-- obj.stackable is already checked
   if obj_n == 196 or obj_n == 311 or obj_n == 312 then --OBJ_LEVER, OBJ_SWITCH, OBJ_SWITCH1
      return true
   else
      return false
   end
end

function input_select_obj_qty(obj)
   if not obj.stackable then
      return 1
   end

   printl("HOW_MANY")
   local qty = input_select_integer("0123456789", true)
   if qty > obj.qty then
      return obj.qty
   end
   return qty
end

--load actor functions
local actor_load = nuvie_load("md/actor.lua");
if type(actor_load) == "function" then
   actor_load()
else
   if type(actor_load) == "string" then
      --io.stderr:write(actor_load);
   end
end

look_init = nuvie_load("md/look.lua"); look_init();

combat_init = nuvie_load("md/combat.lua"); combat_init();

-- init usecode
usecode_init = nuvie_load("md/usecode.lua"); usecode_init();

talk_init = nuvie_load("md/talk.lua"); talk_init();

player_init = nuvie_load("md/player.lua"); player_init();

worktype_init = nuvie_load("md/worktype.lua"); worktype_init();

dreamworld_init = nuvie_load("md/dreamworld.lua"); dreamworld_init();
