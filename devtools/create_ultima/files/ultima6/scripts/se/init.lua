local lua_file = nil

--load common functions
lua_file = nuvie_load("common/common.lua"); lua_file();

function dbg(msg_string)
	--io.stderr:write(msg_string)
end

function load_game()
end

function save_game()
end

local g_tile_to_object_map = {
-- Trees
[50] = 5000, [51] = 5000, [64] = 5000, [65] = 5000, [66] = 5000,
[67] = 5000, [68] = 5000, [69] = 5000, [70] = 5000, [71] = 5000,
[72] = 5000, [192] = 5000, [193] = 5000, [194] = 5000, [195] = 5000,
[198] = 5000, [179] = 5000, [180] = 5000, [130] = 5000,
-- Oven Fire
[16] = 5001, [17] = 5001, [18] = 5001, [19] = 5001, [20] = 5001,
[21] = 5001, [22] = 5001, [23] = 5001, [24] = 5001, [25] = 5001,
[26] = 5001, [27] = 5001, [170] = 5001, [171] = 5001,
-- Yucca Plant
[52] = 5002,
}

function get_tile_to_object_mapping(tile_num)
    return g_tile_to_object_map[tile_num]
end

local g_is_object_a_tile = {
[5000] = true, [5001] = true, [5002] = true
}

function is_tile_object(obj_num)
    if g_is_object_a_tile[obj_num] ~= nil then
        return true
    end
    return false
end

local g_container_obj_tbl = {
[59] = 1,  [60] = 1,  [97] = 1,
[182] = 1, [183] = 1, [184] = 1
}

function is_container_obj(obj_num)
   if g_container_obj_tbl[obj_num] ~= nil then
      return true
   end
   return false
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
-- 1 = neck
[630] = 1, --tooth necklace
[631] = 1, --jade necklace
[696] = 1, --lei
-- 3 = 1 handed
[512] = 3, --spear of shamap
[541] = 3, --black staff
[545] = 3, --club
[546] = 3, --obsidian knife
[547] = 3, --spear
[548] = 3, --throwing axe
[549] = 3, --axe
[550] = 3, --rock hammer
[554] = 3, --obsidian sword
[558] = 3, --atl atl
[560] = 3, --boomerang
[565] = 3, --knife
[574] = 3, --scissors
[677] = 3, --metal hammer
[700] = 3, --grenade
[701] = 3, --lit grenade
[900] = 3, --torch
[901] = 3, --lit torch
[1028] = 3, --device
[1029] = 3, --activated device
-- 2 = torso
[518] = 2, --cloth armor
[519] = 2, --leather armour
[520] = 2, --bark armor
-- 3 = shield hand
[513] = 3, --shield of krukk
[524] = 3, --bark shield
[525] = 3, --leather shield
[526] = 3, --stegosaurus shield
[606] = 3, --kotl shield
-- 7 = feet
-- 4 = two handed - FIXME: guns (561, 571, 572) can't be equiped by natives
[552] = 4, --bow
[553] = 4, --blowgun
[553] = 4, --two handed hammer
[561] = 4, --modern rifle
[571] = 4, --bamboo flintlock
[572] = 4, --loaded bamboo flintlock
[640] = 4, --fire extinguisher
[676] = 4, --fire axe
[689] = 4, --bamboo pole
[692] = 4, --digging stick
[904] = 4, --fishing pole
-- 9 = finger
[600] = 9, --ring (not equipable in original)
}

function obj_is_readiable(obj)
	if g_readiable_objs_tbl[obj.tile_num] ~= nil then
		return true
	end

	return false
end

function obj_get_readiable_location(obj)
	if g_readiable_objs_tbl[obj.tile_num] ~= nil then
		return g_readiable_objs_tbl[obj.tile_num]
	end

	return -1
end

function create_object_needs_quan(obj_n)
-- obj.stackable is already checked
	return false
end

function talk_to_obj(obj)
   printl("NOTHING")
   return false
end

--load actor functions
local actor_load = nuvie_load("se/actor.lua");
if type(actor_load) == "function" then
	actor_load()
else
	if type(actor_load) == "string" then
		--io.stderr:write(actor_load);
	end
end

look_init = nuvie_load("se/look.lua"); look_init();

-- init usecode
usecode_init = nuvie_load("se/usecode.lua"); usecode_init();

player_init = nuvie_load("se/player.lua"); player_init();
