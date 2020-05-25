local lua_file = nil

--load common functions
lua_file = nuvie_load("common/common.lua"); lua_file();


SFX_BLOCKED = 0
SFX_HIT = 1
SFX_FOUNTAIN = 2
SFX_DEATH = 3
SFX_RUBBER_DUCK = 4
SFX_BROKEN_GLASS = 5
SFX_BELL = 6
SFX_FIRE = 7
SFX_CLOCK = 8
SFX_PROTECTION_FIELD = 9
SFX_WATER_WHEEL = 10
SFX_MISSLE = 11
SFX_EXPLOSION = 12
SFX_ATTACK_SWING = 13
SFX_SUCCESS = 14
SFX_FAILURE = 15
SFX_CORPSER_DRAGGED_UNDER = 16
SFX_CORPSER_REGURGITATE = 17
SFX_CASTING_MAGIC_P1 = 18
SFX_CASTING_MAGIC_P1_2 = 19
SFX_CASTING_MAGIC_P1_3 = 20
SFX_CASTING_MAGIC_P1_4 = 21
SFX_CASTING_MAGIC_P1_5 = 22
SFX_CASTING_MAGIC_P1_6 = 23
SFX_CASTING_MAGIC_P1_7 = 24
SFX_CASTING_MAGIC_P1_8 = 25
SFX_CASTING_MAGIC_P2 = 26
SFX_CASTING_MAGIC_P2_2 = 27
SFX_CASTING_MAGIC_P2_3 = 28
SFX_CASTING_MAGIC_P2_4 = 29
SFX_CASTING_MAGIC_P2_5 = 30
SFX_CASTING_MAGIC_P2_6 = 31
SFX_CASTING_MAGIC_P2_7 = 32
SFX_CASTING_MAGIC_P2_8 = 33
SFX_AVATAR_DEATH = 34
SFX_KAL_LOR = 35
SFX_SLUG_DISSOLVE = 36
SFX_HAIL_STONE = 37

SFX_EARTH_QUAKE = 39

FADE_COLOR_RED = 12
FADE_COLOR_BLUE = 9

TIMER_LIGHT = 0
TIMER_INFRAVISION = 1
TIMER_STORM = 13
TIMER_TIME_STOP = 14
TIMER_ECLIPSE = 15

OBJLIST_OFFSET_VANISH_OBJ = 0x1c13
OBJLIST_OFFSET_MOONSTONES = 0x1c1b
OBJLIST_OFFSET_KEG_TIMER  = 0x1c4b

g_vanish_obj = {["obj_n"] = 0, ["frame_n"] = 0}

g_keg_timer = 0
g_armageddon = false
g_avatar_died = false -- used so we don't keep casting once Avatar is dead
function is_avatar_dead()
	return g_avatar_died
end

--used with triple crossbow and magic wind spells.
g_projectile_offset_tbl =
{
 {
  4,5,5,5,5,6,6,6,6,6,6,
  4,4,5,5,5,6,6,6,6,6,7,
  4,4,4,5,5,6,6,6,6,7,7,
  4,4,4,4,5,6,6,6,7,7,7,
  4,4,4,4,4,6,6,7,7,7,7,
  4,4,4,4,4,0,0,0,0,0,0,
  3,3,3,3,2,2,0,0,0,0,0,
  3,3,3,2,2,2,1,0,0,0,0,
  3,3,2,2,2,2,1,1,0,0,0,
  3,2,2,2,2,2,1,1,1,0,0,
  2,2,2,2,2,2,1,1,1,1,0
 },
 {
  2,2,2,2,2,2,3,3,3,3,4,
  1,2,2,2,2,2,3,3,3,4,4,
  1,1,2,2,2,2,3,3,4,4,4,
  1,1,1,2,2,2,3,4,4,4,4,
  1,1,1,1,2,2,4,4,4,4,4,
  0,0,0,0,0,0,4,4,4,4,4,
  0,0,0,0,0,6,6,5,5,5,5,
  0,0,0,0,7,6,6,6,5,5,5,
  0,0,0,7,7,6,6,6,6,5,5,
  0,0,7,7,7,6,6,6,6,6,5,
  0,7,7,7,7,6,6,6,6,6,6
 }
}

--moonstone data is loaded from objlist in load_game()
g_moonstone_loc_tbl =
{
 {x=0x3A7, y=0x106, z=0},
 {x=0x1F7, y=0x166, z=0},
 {x=0x9F,  y=0x3AE, z=0},
 {x=0x127, y=0x26,  z=0},
 {x=0x33F, y=0x0A6, z=0},
 {x=0x147, y=0x336, z=0},
 {x=0x17,  y=0x16,  z=1},
 {x=0x397, y=0x3A6, z=0}
}

g_show_stealing = config_get_boolean_value("config/ultima6/show_stealing")

-- some common functions

function set_g_show_stealing(stealing)
	g_show_stealing = stealing
end

function dbg(msg_string)
	--io.stderr:write(msg_string)
end

function alignment_is_evil(align)
   if align == ALIGNMENT_EVIL or align == ALIGNMENT_CHAOTIC then return true end

   return false
end

function advance_game_time(nturns)
 if nturns == 0 then return end

 coroutine.yield("adv_game_time", nturns);
end

function get_obj_from_inventory(actor)
	local obj = coroutine.yield("inv_obj", actor)
	return obj
end

function get_obj()
	local obj = coroutine.yield("obj")
	return obj
end

function actor_talk(actor)
	coroutine.yield("talk", actor)
	return
end

function get_spell()
	local spell_num = coroutine.yield("spell")
	return spell_num
end

function obj_new(obj_n, frame_n, status, qty, quality, x, y, z)
  local obj = {}

  obj["obj_n"] = obj_n or 0
  obj["frame_n"] = frame_n or 0
  obj["status"] = status or 0
  obj["qty"] = qty or 0
  obj["quality"] = quality or 0

  obj["x"] = x or 0
  obj["y"] = y or 0
  obj["z"] = z or 0

  return obj
end

--FIXME need a better way of doing this. Remove need for deprecated setfenv() function.
function run_script(script)
  local t = {};
  setmetatable(t, {__index = _G});
  local body = nuvie_load(script);
  setfenv(body, t);
  body();
end

function look_obj(obj)
   print("Thou dost see " .. obj.look_string);
   local weight = obj.weight; --FIXME this could be a problem if we want to change Lua_number type to int.
   if weight ~= 0 then
      if obj.qty > 1 and obj.stackable then
         print(". They weigh");
      else
         print(". It weighs");
      end

      print(string.format(" %.1f", weight).." stones");
   end

   --FIXME usecode look description should be lua code.
   if usecode_look(obj) then
      print("\n")
      return false
   end

   local dmg = weapon_dmg_tbl[obj.obj_n];
   if dmg ~= nil then
      if weight ~= 0 then
         print(" and")
      else
         print(". It")
      end

      print(" can do "..dmg.." point")
      if dmg > 1 then print("s") end
      print(" of damage")

   end

   local ac = armour_tbl[obj.obj_n]
   if ac ~= nil then
      if weight ~= 0 or dmg ~= 0 then
         print(" and")
      else
         print(". It")
      end

      print(" can absorb "..ac.." point")
      if ac > 1 then print("s") end
      print(" of damage")
   end

   print(".\n");
   local player_loc = player_get_location();
   if g_show_stealing == true and obj.getable == true and player_loc.z == 0 and obj.ok_to_take == false then
      if math.abs(player_loc.x - obj.x) > 1 or math.abs(player_loc.y - obj.y) > 1 then
        print("PRIVATE PROPERTY\n")
      else
        print("PRIVATE PROPERTY")
      end
   end

   return true
end

function player_subtract_karma(k)
	local karma = player_get_karma() - k
	if karma < 0 then karma = 0 end

	player_set_karma(karma)
end

function player_add_karma(k)
local karma = player_get_karma() + k
if karma >= 100 then karma = 99 end

player_set_karma(karma)
end

function party_heal()
	for actor in party_members() do
		actor.asleep = false
		actor.poisoned = false
		actor.paralyzed = false
		actor_remove_charm(actor)
		actor.hp = actor.max_hp
	end
end

function explosion(tile_num, x, y)
	play_sfx(SFX_EXPLOSION)
	return explosion_start(tile_num, x, y)
end

function projectile(tile_num, start_x, start_y, end_x, end_y, speed, spin)

	if spin == nil then spin = 0 end

	local rotate_offset = 0
	local src_tile_y_offset = 0

	if tile_num == 547 then --spear
		rotate_offset = 45
	elseif tile_num == 566 then --bow
		rotate_offset = 90
		src_tile_y_offset = 4
	elseif tile_num == 567 then --crossbow
		rotate_offset = 90
		src_tile_y_offset = 3
	end

	play_sfx(SFX_MISSLE)
	projectile_anim(tile_num, start_x, start_y, end_x, end_y, speed, false, rotate_offset, spin, src_tile_y_offset)
end

function fade_obj_blue(obj)
  fade_obj(obj, FADE_COLOR_BLUE, 20)
end

function fade_actor_blue(actor)
	Actor.black_fade_effect(actor, FADE_COLOR_BLUE, 20)
end

function fade_obj_out(obj)
	obj.invisible = true
	fade_tile(obj.x, obj.y, obj.z, obj.tile_num)
	obj.invisible = false
end

function fade_obj_in(obj)
	obj.invisible = true
	fade_tile(obj.x, obj.y, obj.z, nil, obj.tile_num)
	obj.invisible = false
end

function fade_actor_in(actor)
	local new_tile_num = actor.tile_num
	actor.visible = false
	fade_tile(actor.x, actor.y, actor.z, nil, new_tile_num)
	actor.visible = true
end

--tile_num, readied location
local g_readiable_objs_tbl = {
[0x200] = 0, [0x201] = 0, [0x202] = 0, [0x203] = 0, [0x204] = 0, [0x205] = 0, [0x206] = 0, [0x207] = 0,

[0x219] = 1, [0x250] = 1, [0x251] = 1, [0x252] = 1, [0x217] = 1, [0x101] = 1,

[0x220] = 2, [0x221] = 2, [0x223] = 2, [0x224] = 2, [0x225] = 2, [0x226] = 2, [0x227] = 2, [0x22A] = 2,
[0x22F] = 2, [0x230] = 2, [0x238] = 2, [0x254] = 2, [0x256] = 2, [0x255] = 2, [0x259] = 2, [0x262] = 2,
[0x263] = 2, [0x264] = 2, [0x270] = 2, [0x271] = 2, [0x272] = 2, [0x273] = 2, [0x274] = 2, [0x275] = 2,
[0x279] = 2, [0x27D] = 2, [0x27E] = 2, [0x27F] = 2, [0x280] = 2, [0x281] = 2, [0x28C] = 2, [0x28E] = 2,
[0x29D] = 2, [0x2A2] = 2, [0x2A3] = 2, [0x2B9] = 2,

[0x210] = 4, [0x211] = 4, [0x212] = 4, [0x213] = 4, [0x214] = 4, [0x215] = 4, [0x216] = 4, [0x218] = 4,
[0x219] = 4, [0x28c] = 4, [0x28e] = 4, [0x29d] = 4, [0x257] = 4,
[0x208] = 5, [0x209] = 5, [0x20a] = 5, [0x20b] = 5, [0x20c] = 5, [0x20d] = 5, [0x20e] = 5, [0x20f] = 5,
[0x222] = 5,

[0x21a] = 7, [0x21b] = 7,

[0x228] = 8, [0x229] = 8, [0x231] = 8, [0x235] = 8, [0x22b] = 8, [0x22c] = 8, [0x22d] = 8, [0x22e] = 8,

[0x258] = 9, [0x37d] = 9, [0x37e] = 9, [0x37f] = 9
}

function actor_is_readiable_obj(actor)
	if g_readiable_objs_tbl[actor.tile_num] ~= nil then
		return true
	end

	return false
end

function obj_is_readiable(obj)
	if g_readiable_objs_tbl[obj.tile_num] ~= nil then
		return true
	end

	return false
end

function is_time_stopped()
	if timer_get(TIMER_TIME_STOP) ~= 0 then return true end

	return false
end

function load_game()
	objlist_seek(OBJLIST_OFFSET_VANISH_OBJ)
	local tmp_obj_dat = objlist_read2()

	local obj_n = tmp_obj_dat
	local frame_n = 0
	if tmp_obj_dat > 1023 then
		frame_n = tmp_obj_dat - 1023
		obj_n = tmp_obj_dat - frame_n
	end

	g_vanish_obj.obj_n = obj_n
	g_vanish_obj.frame_n = frame_n

	--Load moonstone locations.
	objlist_seek(OBJLIST_OFFSET_MOONSTONES)

	for i=1,8 do

		local x = objlist_read2()
		local y = objlist_read2()
		local z = objlist_read2()
		dbg("moonstone["..i.."] at ("..x..","..y..","..z..")\n")
		g_moonstone_loc_tbl[i] = {x=x, y=y, z=z}
	end

	objlist_seek(OBJLIST_OFFSET_KEG_TIMER)
	g_keg_timer = objlist_read2()
	set_g_armageddon(false)
	g_avatar_died = false
end

function save_game()
	objlist_seek(OBJLIST_OFFSET_VANISH_OBJ)

	local tmp_obj_dat = g_vanish_obj.obj_n
	local frame_n = g_vanish_obj.frame_n

	if frame_n > 0 then
		tmp_obj_dat = tmp_obj_dat + (frame_n + 1023)
	end

	objlist_write2(tmp_obj_dat)

	--Save moonstone locations
	objlist_seek(OBJLIST_OFFSET_MOONSTONES)

	for i=1,8 do
		local loc = g_moonstone_loc_tbl[i]
		objlist_write2(loc.x)
		objlist_write2(loc.y)
		objlist_write2(loc.z)
	end

	objlist_seek(OBJLIST_OFFSET_KEG_TIMER)
	objlist_write2(g_keg_timer)
end

function moonstone_set_loc(phase, x, y, z)
	if phase < 1 or phase > 8 then return false end

	g_moonstone_loc_tbl[phase] = {x=x, y=y, z=z}
	return true
end

function moonstone_get_loc(phase)
	if phase < 1 or phase > 8 then return nil end

	return g_moonstone_loc_tbl[phase]
end

function update_moongates(show_moongates)
	local i, loc
	for i,loc in ipairs(g_moonstone_loc_tbl) do
		local moongate = map_get_obj(loc.x, loc.y, loc.z, 0x55) --moongate
		if show_moongates == true then
			if moongate == nil and loc.x ~= 0 then
				moongate = Obj.new(0x55, 1)
				Obj.moveToMap(moongate, loc)
			end
		else --hide moongate
			if moongate ~= nil then
				map_remove_obj(moongate)
			end
		end
	end
end

function use_keg(obj)
	if obj.frame_n ~= 0 then
		print("\nNo effect\n")
		return
	end

	if g_keg_timer > 0 then
		print("\nNot now\n")
	else
		obj.frame_n = 1
		print("\nPowder lit!\n")
		g_keg_timer = 3
	end
end

function explode_keg()
	--try to find lit keg on the current game map.
	local loc = player_get_location()
	for obj in find_obj(loc.z, 223, 1) do --keg obj, frame_n = 1
		if obj ~= nil then
			explode_obj(obj)
		end
	end

	--try to explode lit kegs in the party's inventory
	local party_actor
	for party_actor in party_members() do
		local obj = Actor.inv_get_obj_n(party_actor, 223, 1) --keg with frame_n = 1
		if obj ~= nil then
			explode_obj(obj, party_actor)
		end
	end
end

function explode_obj(obj, actor)
	dbg("Exploding "..obj.name.."\n")
	local x, y, z

	if actor ~= nil then
		x = actor.x
		y = actor.y
		z = actor.z
	else
		x = obj.x
		y = obj.y
		z = obj.z
	end

	Obj.removeFromEngine(obj)

	local hit_items = explosion(0x189, x, y)
	local random = math.random
	local k, v
	for k,v in pairs(hit_items) do
		if v.luatype == "actor" then
			actor_hit(v, random(1, 0x3c))
		end
		if g_avatar_died == true then
			explode_surrounding_objects(x, y, z)
			actor_avatar_death(Actor.get(1))
			return -- don't keep exploding once Avatar is dead
		end
	end

	explode_surrounding_objects(x, y, z)
end

function explode_surrounding_objects(x, y, z)
	--blow up doors and other kegs
	for x = x - 2,x + 2 do
		for y = y - 2,y + 2 do
			local map_obj = map_get_obj(x, y, z, 223)
			if map_obj ~= nil then
				explode_obj(map_obj)
			end

			map_obj = map_get_obj(x, y, z, 0x12c) --steel door
			if map_obj == nil or map_obj.frame_n == 0xc then
				map_obj = map_get_obj(x, y, z, 0x129) --oaken door
				if map_obj == nil or map_obj.frame_n == 0xc then
					map_obj = map_get_obj(x, y, z, 0x12a) --windowed door
					if map_obj == nil or map_obj.frame_n == 0xc then
						map_obj = map_get_obj(x, y, z, 0x12b) --cedar door
					end
				end
			end

			if map_obj ~= nil and map_obj.frame_n <= 0xc then
				Obj.removeFromEngine(map_obj)
				print("\nThe door is blown up!\n")
			end
		end
	end
end

function set_g_armageddon(val)
	g_armageddon = val
	set_armageddon(val)
end

function create_object_needs_quan(obj_n)
-- obj.stackable is already checked
	return false
end

--load actor functions
actor_load = nuvie_load("u6/actor.lua");
if type(actor_load) == "function" then
   actor_load()
else
   if type(actor_load) == "string" then
      io.stderr:write(actor_load);
   end
end

-- init magic
magic_init = nuvie_load("u6/magic.lua"); magic_init();

-- init usecode
usecode_init = nuvie_load("u6/usecode.lua"); usecode_init();

player_init = nuvie_load("u6/player.lua"); player_init();
