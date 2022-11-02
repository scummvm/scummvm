--note nuvie direction values aren't the same as the original it uses the following scheme
--701
--6 2
--543

DIR_NORTH     = 0
DIR_EAST      = 1
DIR_SOUTH     = 2
DIR_WEST      = 3
DIR_NORTHEAST = 4
DIR_SOUTHEAST = 5
DIR_SOUTHWEST = 6
DIR_NORTHWEST = 7
DIR_NONE      = 8


UI_STYLE_ORIG = 0
UI_STYLE_NEW  = 1
UI_STYLE_ORIG_PLUS_CUTOFF_MAP = 2
UI_STYLE_ORIG_PLUS_FULL_MAP   = 3

STACK_OBJECT_QTY = true

movement_offset_x_tbl  = {0, 1, 1, 1, 0, -1, -1, -1}
movement_offset_y_tbl = {-1, -1, 0, 1, 1, 1, 0, -1 }

if not setfenv then -- Lua 5.2
   -- based on http://lua-users.org/lists/lua-l/2010-06/msg00314.html
   -- this assumes f is a function
   local function findenv(f)
      local level = 1
      repeat
         local name, value = debug.getupvalue(f, level)
         if name == '_ENV' then return level, value end
         level = level + 1
      until name == nil
      return nil
   end
   getfenv = function (f) return(select(2, findenv(f)) or _G) end
   setfenv = function (f, t)
      local level = findenv(f)
      if level then debug.setupvalue(f, level, t) end
      return f
   end
end

function get_target()
   local loc = coroutine.yield("target")

   return loc
end

function get_direction(prompt)

	if prompt ~= nil then
		print(prompt)
	end

	local dir = coroutine.yield("dir")

	return dir
end

function really_get_direction(prompt)

	if prompt ~= nil then
		print(prompt)
	end

	local dir = coroutine.yield("need_dir")

	return dir
end

function direction_string(dir)
	if dir ~= nil then
		if dir == DIR_NORTH then return "north" end
		if dir == DIR_NORTHEAST then return "northeast" end
		if dir == DIR_EAST then return "east" end
		if dir == DIR_SOUTHEAST then return "southeast" end
		if dir == DIR_SOUTH then return "south" end
		if dir == DIR_SOUTHWEST then return "southwest" end
		if dir == DIR_WEST then return "west" end
		if dir == DIR_NORTHWEST then return "northwest" end
	end

	return "unknown"
end

local dir_rev_tbl =
{
   [DIR_NORTH] = DIR_SOUTH,
   [DIR_NORTHEAST] = DIR_SOUTHWEST,
   [DIR_EAST] = DIR_WEST,
   [DIR_SOUTHEAST] = DIR_NORTHWEST,
   [DIR_SOUTH] = DIR_NORTH,
   [DIR_SOUTHWEST] = DIR_NORTHEAST,
   [DIR_WEST] = DIR_EAST,
   [DIR_NORTHWEST] = DIR_SOUTHEAST
}

function direction_reverse(dir) return dir_rev_tbl[dir] end

local g_dir_offset_tbl =
{
   [DIR_NORTH] = {["x"]=0, ["y"]=-1},
   [DIR_NORTHEAST] = {["x"]=1, ["y"]=-1},
   [DIR_EAST] = {["x"]=1, ["y"]=0},
   [DIR_SOUTHEAST] = {["x"]=1, ["y"]=1},
   [DIR_SOUTH] = {["x"]=0, ["y"]=1},
   [DIR_SOUTHWEST] = {["x"]=-1, ["y"]=1},
   [DIR_WEST] = {["x"]=-1, ["y"]=0},
   [DIR_NORTHWEST] = {["x"]=-1, ["y"]=-1},
   [DIR_NONE] = {["x"]=0, ["y"]=0}
}

function direction_get_loc(dir, from_x, from_y)
	return g_dir_offset_tbl[dir].x + from_x, g_dir_offset_tbl[dir].y + from_y
end

function abs(val)
   if val < 0 then
      return -val
   end

   return val
end

--collect Yes/No input from user and return true if Yes selected. false otherwise.
function input_should_proceed()
   local input = input_select("yn", true)
   print("\n")
   if input == nil or input == "N" or input == "n" then
      return false
   end

   return true
end

function play_midgame_sequence(seq_num)
   local ui_style = game_get_ui_style()

   canvas_show()
   canvas_hide_all_sprites()
   canvas_set_opacity(0xff);
   canvas_set_update_interval(25)
   canvas_rotate_game_palette(true)

   local bg = sprite_new(nil, 8, 16, true)
   local avatar = sprite_new(nil, 8, 16, false)

   local text_sprite
   --local text_sprite_bg

   if ui_style == UI_STYLE_ORIG then
      canvas_set_solid_bg(false)
   else
   --[[
      text_sprite_bg = sprite_new(nil, 8, 160, true)
      text_sprite_bg.text_align = 2
      text_sprite_bg.text_color = 14
      --]]
      text_sprite = sprite_new(nil, 8, 160, true)
      text_sprite.text_align = 2
      text_sprite.text_color = 15
      bg.x = 80
      bg.y = 12
      avatar.x = 80
      avatar.y = 12
   end

   local midgame_data = midgame_load("midgame"..string.format("%x", seq_num)..".lzc")
   local i = 0
   local data = midgame_data[i]



   while data ~= nil do
      bg.image = data.images[0]
      if data.images[1] ~= nil then
         local gender = player_get_gender()
         avatar.image = data.images[1+gender]
         avatar.visible = true
      else
         avatar.visible = false
      end

      local j = 0
      local text = data.text[j]
      while text ~= nil do
         if text ~= "*END*" then
            if ui_style == UI_STYLE_ORIG then
               clear_scroll()
               print(text)
            else
               text_sprite.text = text
               --text_sprite_bg.text = text
            end

            local input = nil
            while input == nil do
               canvas_update()
               input = input_poll()
               if input ~= nil then
                  break
               end
            end
         end
         j = j + 1
         text = data.text[j]
      end
      i = i + 1
      data = midgame_data[i]
   end

   if ui_style == UI_STYLE_ORIG then
      clear_scroll()
   end
   canvas_set_solid_bg(true)
   canvas_rotate_game_palette(false)
   canvas_hide()
end

function get_wrapped_dist(pt1, pt2)
   local diff

   if pt2 >= pt1 then
      diff = pt2 - pt1
   else
      diff = pt1 - pt2
   end

   if diff > 512 then
      diff = 1024 - diff
   end

   return diff
end

function get_anim_index_for_tile(tile_number)
   local total_anims = anim_get_number_of_entries()
   for i=0,total_anims-1 do
      if anim_get_tile(i) == tile_number then
         return i
      end
   end

   return nil
end

function altcode_242_set_actor_talk_flag()
   print("NPC: ")
   local input = input_select(nil, true)
   local actor_num = tonumber(input, 16)
   local actor = Actor.get(actor_num)
   print("\n"..actor.name.."\n")
   print("flags: \n") --FIXME print talk flags
   print("\nBit: ")
   local bit = input_select_integer(nil, true)
   local value = Actor.get_talk_flag(actor, bit)
   local value_str = "off"
   if value == true then
      value_str = "on"
   end
   print(" is "..value_str..".\n")
   print("New value? ")
   value = input_select_integer(nil, true)

   value_str = "off"

   if value == 1 or value == "o" then
      value_str = "on"
      Actor.set_talk_flag(actor, bit)
   else
      Actor.clear_talk_flag(actor, bit)
   end
   print("\n"..value_str.."\n")
end

function altcode_250_create_object()
   print("Create Item:\nType:0x")
   local input = input_select(nil, true)
   local obj_n = tonumber(input, 16)
   local obj = Obj.new(obj_n)
   local tmp_obj = Obj.new(obj_n+1)
   if tmp_obj ~= nil and tmp_obj.tile_num - obj.tile_num > 1 then
      print("\nFrame:0x")
      input = input_select(nil, true)
      obj.frame_n = tonumber(input, 16)
   end

   print("\nQual:0x")
   input = input_select(nil, true)
   obj.quality = tonumber(input, 16)

   if obj.stackable or create_object_needs_quan(obj_n) then
      print("\nQuan:0x")
      input = input_select(nil, true)
      obj.qty = tonumber(input, 16)
   end
   Obj.moveToInv(obj, Actor.get(1).actor_num)
   print("\n")
end

function altcode_913_export_tmx_map_files()
   print("\nExport maps to savedir? ")

   if not input_should_proceed() then
      return
   end

   print("saving.\n")
   script_wait(1)
   if map_export_tmx_files() == true then
      print("done.\n\n")
   else
      print("error!!\n\n")
   end
end

function altcode_914_export_tileset()
   print("Exporting tileset to \"data/images/tiles/"..config_get_game_type().."/custom_tiles.bmp\" in the savegame directory.\n")
   if not tileset_export() then
      print("file already exists. Overwrite? ")
      if not input_should_proceed() then
         return
      end
      tileset_export(true)
   end
   print("done.\n\n")
end

function altcode_999_find_objs_on_map()
   print("Find Object\nObj_n: ")
   local input = input_select(nil, true)
   local obj_n = tonumber(input, 10)
   if obj_n == nil then
      print("Nothing.\n")
      return
   end
   print("\nFrame: ")
   input = input_select(nil, true)
   local frame_n = tonumber(input, 10)
   print("\nQuality: ")
   input = input_select(nil, true)
   local quality = tonumber(input, 10)
   print("\nz: ")
   input = input_select(nil, true)
   local z = tonumber(input, 10)
   if z == nil then
      z = 0
   end
   print("\n")
   for obj in find_obj(z, obj_n, frame_n, quality) do
      print(string.format("OBJ: (%x,%x,%x)\n", obj.x, obj.y, obj.z))
      party_move(obj.x, obj.y, obj.z)
      print("continue? ")
      input = input_select("yn", false)
      print("\n")
      if input == "N" or input == "n" then
         return
      end
   end
end

local altcode_tbl = {
   [242]=altcode_242_set_actor_talk_flag,
   [250]=altcode_250_create_object,
   [913]=altcode_913_export_tmx_map_files,
   [914]=altcode_914_export_tileset,
   [999]=altcode_999_find_objs_on_map,
}

function handle_alt_code(altcode)
   local func = altcode_tbl[altcode]
   if func ~= nil then
      func()
   end
end

function get_actor_or_obj_from_loc(location)
   local target = map_get_actor(location)
   if target == nil then
      target = map_get_obj(location)
   end
   return target
end

--load other common functions

local lua_file = nil

lua_file = nuvie_load("common/lang.lua");
lua_file();
lang_init("game")

lua_file = nuvie_load("common/actor.lua"); lua_file();
