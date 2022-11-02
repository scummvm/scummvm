SDLK_UP           = 82 + 1073741824
SDLK_DOWN         = 81 + 1073741824
SDLK_RIGHT        = 79 + 1073741824
SDLK_LEFT         = 80 + 1073741824

g_img_tbl = {}

local function poll_for_esc()
	local input = input_poll()
	if input ~= nil and input == 27 then
		return true
	end

	if engine_should_quit() == 1 then
		return true
	end

	return false
end

local function wait_for_input()
	local input = nil
	while input == nil do
		canvas_update()
		input = input_poll()
			if input ~= nil then
				break
			end

			if engine_should_quit() == 1 then
				break
			end
	end

	return input
end

local function should_exit(input)
	if engine_should_quit() == 1 then
		return true
	end

	if input ~=nil and input == 27 then
		return true
	end

	return false
end

local function fade_out()
	local i
	for i=0xff,0,-3 do
		canvas_set_opacity(i)
		canvas_update()

		if engine_should_quit() == 1 then
			break
		end
	end

	return false
end

local function fade_out_sprite(sprite, speed)
	local i
	if speed ~= nil then
		speed = -speed
	else
		speed = -3
	end

	for i=0xff,0,speed do
		sprite.opacity = i
		canvas_update()

		if engine_should_quit() == 1 then
			break
		end
	end

	return false
end

local function fade_in()
	local i
	for i=0x0,0xff,3 do
		canvas_set_opacity(i)
		canvas_update()

		if engine_should_quit() == 1 then
			break
		end
	end

	return false
end

local function fade_in_sprite(sprite, speed)
	local i
	if speed ~= nil then
		speed = speed
	else
		speed = 3
	end

	for i=0,0xff,speed do
		sprite.opacity = i
		canvas_update()

		if engine_should_quit() == 1 then
			break
		end
	end

	return false
end

local function load_images(filename)
	g_img_tbl = image_load_all(filename)
end

g_clock_tbl = {}

local function load_clock()
	g_clock_tbl["h1"] = sprite_new(nil, 0xdd, 0x14, true)
	g_clock_tbl["h2"] = sprite_new(nil, 0xdd+4, 0x14, true)
	g_clock_tbl["m1"] = sprite_new(nil, 0xdd+0xa, 0x14, true)
	g_clock_tbl["m2"] = sprite_new(nil, 0xdd+0xe, 0x14, true)
end

local function display_clock()
	local t = os.date("*t")
	local hour = t.hour
	local minute = t.min

	if hour > 12 then
		hour = hour - 12
	end

	if hour < 10 then
		g_clock_tbl["h1"].image = g_img_tbl[12]
	else
		g_clock_tbl["h1"].image = g_img_tbl[3]
	end

	if hour >= 10 then
		hour = hour - 10
	end

	g_clock_tbl["h2"].image = g_img_tbl[hour+2]

	g_clock_tbl["m1"].image = g_img_tbl[math.floor(minute / 10) + 2]
	g_clock_tbl["m2"].image = g_img_tbl[(minute % 10) + 2]

end

g_lounge_tbl = {}

g_tv_base_x = 0xe5
g_tv_base_y = 0x32

local function load_lounge()

	g_lounge_tbl["bg0"] = sprite_new(g_img_tbl[15], 210, 0, true)
	g_lounge_tbl["bg"] = sprite_new(g_img_tbl[0], 0, 0, true)
	g_lounge_tbl["bg1"] = sprite_new(g_img_tbl[1], 320, 0, true)
	g_lounge_tbl["avatar"] = sprite_new(g_img_tbl[0xd], 0, 63, true)

	g_lounge_tbl["tv"] = {sprite_new(nil, g_tv_base_x ,g_tv_base_y, true), sprite_new(nil, g_tv_base_x ,g_tv_base_y, true), sprite_new(nil, g_tv_base_x ,g_tv_base_y, true), sprite_new(nil, g_tv_base_x ,g_tv_base_y, true), sprite_new(nil, g_tv_base_x ,g_tv_base_y, true)}

	for i=1,5 do
		local sprite = g_lounge_tbl["tv"][i]
		sprite.clip_x = g_tv_base_x
		sprite.clip_y = g_tv_base_y
		sprite.clip_w = 57
		sprite.clip_h = 37
	end

	g_lounge_tbl["finger"] = sprite_new(g_img_tbl[0xe], 143, 91, true)

	g_lounge_tbl["tv_static"] = image_new(57,37)

	load_clock()
end

g_tv_loop_pos = 0
g_tv_loop_cnt = 0
g_tv_cur_pos = 1
g_tv_cur_program = 3

g_tv_news_image = 0
g_tv_news_image_tbl = {0x5,0x6,0x7,0x0D,0x0E,0x18,0x19,0x1F,0x20}

g_tv_programs = {
{0x82,0x82,0x80,0x3,0x2,0x8A,0x2,0x8A,0x1,0x8A,0x1,0x8A,0x0,0x8A,0x0,0x8A,0x1,0x8A,0x1,0x81}, --breathing
{0x82,0x82,0x80,0x28,0x3,0x8B,0x81}, --intestines
{0x82,0x82,0x80,0x4,0x4,0x81,0x80,0x4,0x8,0x81,0x80,0x4,0x9,0x81,0x80,0x4,0x0A,0x81,0x80,0x4,0x0B,0x81,0x80,0x4,0x0C,0x81}, --vacuum
{0x82,0x82,0x87,0x80,0x46,0x0F,0x86,0x84,0x9,0x10,0x10,0x10,0x11,0x11,0x11,0x12,0x12,0x12,0x81}, --tv anchor man
{0x82,0x82,0x80,0x32,0x83,0x81}, --endless road
{0x82,0x82,0x80,0x5,0x27,0x8A,0x28,0x8A,0x29,0x81,0x80,0x6,0x2A,0x8A,0x2A,0x8A,0x2B,0x8A,0x2B,0x8A,0x2C,0x8A,0x2C,0x8A,0x2D,0x8A,0x2D,0x81,0x80,0x0A,0x2E,0x8A,0x2F,0x8A,0x30,0x8A,0x84,0x9,0x2E,0x2E,0x2E,0x2F,0x2F,0x2F,0x30,0x30,0x30,0x8A,0x2E,0x8A,0x2F,0x8A,0x30,0x81}, --rock band
{0x82,0x82,0x80,0x55,0x16,0x17,0x84,0x0C,0x13,0x13,0x13,0x13,0x14,0x14,0x14,0x14,0x15,0x15,0x15,0x15,0x88,0x81,0x80,0x0F,0x16,0x84,0x2,0x1A,0x1B,0x89,0x88,0x81,0x80,0x3,0x16,0x1A,0x89,0x88,0x81,0x80,0x3,0x16,0x1C,0x88,0x81,0x80,0x3,0x16,0x1D,0x88,0x81,0x80,0x3,0x16,0x1E,0x88,0x81,0x80,0x3,0x16,0x23,0x88,0x81,0x80,0x3,0x16,0x24,0x88,0x81,0x80,0x32,0x16,0x88,0x81} --pledge now!
}


g_tv_x_off = {
0x0,0x0,0x0,0x0,0x0,0x1F,0x1F,0x1F,0x0,0x0,0x0,0x0,0x0,0x1F,0x1F,
0x0,0x9,0x9,0x9,0x0C,0x0C,0x0C,0x0,0x4,0x1F,0x1F,0x4,0x0,0x4,
0x4,0x4,0x1F,0x1F,0x0,0x0,0x6,0x6,0x8,0x4,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
}

g_tv_y_off = {
0x0,0x0,0x0,0x0,0x0,0x2,0x2,0x2,0x0,0x0,0x0,0x0,0x0,0x2,0x2,0x0,0x7,0x7,
0x7,0x3,0x3,0x3,0x0,0x2,0x2,0x2,0x0,0x0,0x0,0x0,0x1,0x2,0x2,0x0,0x0,0x3,
0x8,0x1D,0x1C,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0
}

g_tv_pledge_counter = 0
g_tv_pledge_image = 37

g_tv_road_offset = 0xe

local function display_tv_sprite(s_idx, image_num)
	local sprite = g_lounge_tbl.tv[s_idx]
	if sprite ~= nil then
		sprite.image = g_img_tbl[0x10+image_num]
		sprite.x = g_tv_base_x + g_tv_x_off[image_num+1]
		sprite.y = g_tv_base_y + g_tv_y_off[image_num+1]
		if g_tv_base_x < 0 then
			sprite.clip_x = 0
		else
			sprite.clip_x = g_tv_base_x
		end
		sprite.clip_y = g_tv_base_y
		sprite.visible = true
	end
end

local function display_tv()
	local should_exit = false
	local s_idx = 1
	for i=1,5 do
		g_lounge_tbl["tv"][i].visible = false
	end

	g_lounge_tbl.finger.visible = false

	while should_exit == false do
		local item = g_tv_programs[g_tv_cur_program][g_tv_cur_pos]
		if item < 0x80 then
			display_tv_sprite(s_idx, item)
			s_idx = s_idx + 1
		elseif item == 0x82 then
			--static
			local sprite = g_lounge_tbl.tv[s_idx]
			if sprite ~= nil then
				sprite.image = g_lounge_tbl["tv_static"]
				image_static(sprite.image)
				sprite.x = g_tv_base_x
				sprite.y = g_tv_base_y
				sprite.clip_x = g_tv_base_x
				sprite.clip_y = g_tv_base_y
				sprite.visible = true
			end
			g_lounge_tbl.finger.visible = true
			should_exit = true
		elseif item == 0x80 then --loop start
			g_tv_cur_pos = g_tv_cur_pos + 1
			g_tv_loop_cnt = g_tv_programs[g_tv_cur_program][g_tv_cur_pos]
			g_tv_loop_pos = g_tv_cur_pos
		elseif item == 0x81 then --loop end
			if g_tv_loop_cnt > 0 then
				g_tv_cur_pos = g_tv_loop_pos
				g_tv_loop_cnt = g_tv_loop_cnt - 1
			end
			should_exit = true
		elseif item == 0x83 then --endless road
			g_tv_road_offset = g_tv_road_offset - 1
			if g_tv_road_offset == 0 then
				g_tv_road_offset = 0xe
			end
			g_tv_y_off[0x23] = 0x15 - g_tv_road_offset
			display_tv_sprite(s_idx, 0x22)
			s_idx = s_idx + 1
			g_tv_y_off[0x23] = 0x24 - g_tv_road_offset
			display_tv_sprite(s_idx, 0x22)
			s_idx = s_idx + 1
			display_tv_sprite(s_idx, 0x21)
			s_idx = s_idx + 1
		elseif item == 0x84 then --select random image from list.
			local rand_len = g_tv_programs[g_tv_cur_program][g_tv_cur_pos+1]
			item = g_tv_programs[g_tv_cur_program][g_tv_cur_pos+math.random(1,rand_len)+1]
			display_tv_sprite(s_idx, item)
			s_idx = s_idx + 1
			g_tv_cur_pos = g_tv_cur_pos + 1 + rand_len
		elseif item == 0x86 then --display news image
			display_tv_sprite(s_idx, g_tv_news_image)
			s_idx = s_idx + 1
		elseif item == 0x87 then --select news image
			g_tv_news_image = g_tv_news_image_tbl[math.random(1, 9)]
		elseif item == 0x88 then --pledge now!
			g_tv_pledge_counter = g_tv_pledge_counter + 1
			if g_tv_pledge_counter % 4 == 0 then
				display_tv_sprite(s_idx, g_tv_pledge_image)
				s_idx = s_idx + 1
			end

			if g_tv_pledge_counter == 16 then
				if g_tv_pledge_image == 37 then
					g_tv_pledge_image = 38
				else
					g_tv_pledge_image = 37
				end
				g_tv_pledge_counter = 0
			end
		elseif item == 0x89 then
			display_tv_sprite(s_idx, math.random(50, 52))
			s_idx = s_idx + 1
		elseif item == 0x8a then
			should_exit = true
		elseif item == 0x8b then
			canvas_rotate_palette(0xe0, 5)
		end

		g_tv_cur_pos = g_tv_cur_pos + 1
		if g_tv_programs[g_tv_cur_program][g_tv_cur_pos] == nil then
			g_tv_cur_program = g_tv_cur_program + 1
			g_tv_cur_pos = 1

			if g_tv_programs[g_tv_cur_program] == nil then
				g_tv_cur_program = 1
			end
		end
	end
end

g_tv_update = true

local function display_lounge()
	display_clock()
	if g_tv_update == true then
		display_tv()
		g_tv_update = false
	else
		g_tv_update = true
	end
end

local function scroll_clock()
g_clock_tbl["h1"].x = g_clock_tbl["h1"].x - 1
g_clock_tbl["h2"].x = g_clock_tbl["h2"].x - 1
g_clock_tbl["m1"].x = g_clock_tbl["m1"].x - 1
g_clock_tbl["m2"].x = g_clock_tbl["m2"].x - 1
end

local function scroll_lounge()
g_lounge_tbl.bg.x = g_lounge_tbl.bg.x - 1
g_lounge_tbl.bg1.x = g_lounge_tbl.bg1.x - 1
g_tv_base_x = g_tv_base_x - 1
g_lounge_tbl.avatar.x = g_lounge_tbl.avatar.x - 2
g_lounge_tbl.finger.x = g_lounge_tbl.finger.x - 2

if g_lounge_tbl.bg1.x < 166 and g_lounge_tbl.bg1.x % 2 == 0 then
	g_lounge_tbl.bg0.x = g_lounge_tbl.bg0.x - 1
end

scroll_clock()
end

local function hide_lounge()

	g_lounge_tbl["bg0"].visible = false
	g_lounge_tbl["bg"].visible = false
	g_lounge_tbl["bg1"].visible = false
	g_lounge_tbl["avatar"].visible = false
	g_lounge_tbl["tv"].visible = false
end

local function lounge_sequence()

	load_lounge()

	canvas_set_palette("palettes.int", 1)

	--for i=0,0xff do
		--display_lounge()
		--canvas_update()
	--end
	local scroll_img = image_load("blocks.shp", 1)
	local scroll = sprite_new(scroll_img, 1, 0x9f, true)

	local x, y = image_print(scroll_img, "Upon your world, five seasons have passed since your ", 8, 312, 8, 10, 0x3e)
	image_print(scroll_img, "triumphant homecoming from Britannia.", 8, 312, x, y, 0x3e)

	local input = input_poll()

	while input == nil do
		display_lounge()
		canvas_update()
		input = input_poll()
	end

	if should_exit(input) then
		return false
	end

	scroll_img = image_load("blocks.shp", 2)
	x, y = image_print(scroll_img, "You have traded the Avatar's life of peril and adventure ", 7, 310, 7, 8, 0x3e)
	x, y = image_print(scroll_img, "for the lonely serenity of a world at peace. But ", 7, 310, x, y, 0x3e)
	x, y = image_print(scroll_img, "television supermen cannot take the place of friends ", 7, 310, x, y, 0x3e)
	image_print(scroll_img, "who died at your side!", 7, 310, x, y, 0x3e)

	scroll.image = scroll_img
	scroll.x = 1
	scroll.y = 0x98

	input = input_poll()

	while input == nil do
		display_lounge()
		canvas_update()
		input = input_poll()
	end

	if should_exit(input) then
		return false
	end

	scroll_img = image_load("blocks.shp", 0)
	image_print(scroll_img, "Outside, a chill wind rises...", 39, 200, 39, 8, 0x3e)

	scroll.image = scroll_img
	scroll.x = 0x21
	scroll.y = 0x9d

	for i=0,319 do
		scroll_lounge()
		display_lounge()
		canvas_update()
		input = input_poll()
		if should_exit(input) then
			return false
		end
	end

	input = input_poll()
	while input == nil do
		canvas_update()
		input = input_poll()
	end

	if should_exit(input) then
		return false
	end

	hide_lounge()
	scroll.visible = false

	return true
end

g_window_tbl = {}
local function load_window()
	local rand = math.random
	g_window_tbl["cloud_x"] = -400

	g_window_tbl["sky"] = sprite_new(g_img_tbl[1], 0, 0, true)
	g_window_tbl["cloud1"] = sprite_new(g_img_tbl[0], g_window_tbl["cloud_x"], -5, true)
	g_window_tbl["cloud2"] = sprite_new(g_img_tbl[0], g_window_tbl["cloud_x"], -5, true)
	g_window_tbl["clouds"] = {}

	local i
	for i=1,5 do
		table.insert(g_window_tbl["clouds"], sprite_new(g_img_tbl[rand(2,3)], rand(0,320) - 260, rand(0, 30), true))
	end

	g_window_tbl["lightning"] = sprite_new(nil, 0, 0, false)
	g_window_tbl["ground"] = sprite_new(g_img_tbl[10], 0, 0x4c, true)
	g_window_tbl["trees"] = sprite_new(g_img_tbl[8], 0, 0, true)

	g_window_tbl["strike"] = sprite_new(g_img_tbl[rand(19,23)], 158, 114, false)



	--FIXME rain here.
	local rain = {}
	local i
	for i = 0,100 do
		rain[i] = sprite_new(g_img_tbl[math.random(4,7)], math.random(0,320), math.random(0,200), false)
	end
	g_window_tbl["rain"] = rain

	g_window_tbl["frame"] = sprite_new(g_img_tbl[28], 0, 0, true)
	g_window_tbl["window"] = sprite_new(g_img_tbl[26], 0x39, 0, true)
	g_window_tbl["door_left"] = sprite_new(g_img_tbl[24], 320, 0, true)
	g_window_tbl["door_right"] = sprite_new(g_img_tbl[25], 573, 0, true)

	g_window_tbl["flash"] = 0
	g_window_tbl["drops"] = 0
	g_window_tbl["rain_delay"] = 20
	g_window_tbl["lightning_counter"] = 0

end

local function hide_window()
	g_window_tbl["sky"].visible = false
	g_window_tbl["cloud1"].visible = false
	g_window_tbl["cloud2"].visible = false

	local i
	for i=1,5 do
		g_window_tbl["clouds"][i].visible = false
	end

	g_window_tbl["lightning"].visible = false
	g_window_tbl["ground"].visible = false
	g_window_tbl["trees"].visible = false

	g_window_tbl["strike"].visible = false

	local i
	for i = 0,100 do
		g_window_tbl["rain"][i].visible = false
	end

	g_window_tbl["frame"].visible = false
	g_window_tbl["window"].visible = false
	g_window_tbl["door_left"].visible = false
	g_window_tbl["door_right"].visible = false

end

local function display_window()

	local i
	local rain = g_window_tbl["rain"]
	local rand = math.random
	local c_num

	--FIXME display clouds here.
	local cloud
	for i,cloud in ipairs(g_window_tbl["clouds"]) do
		if cloud.x > 320 then
			cloud.x = rand(0, 320) - 320
			cloud.y = rand(0, 30)
		end

		cloud.x = cloud.x + 2
	end

	g_window_tbl["cloud_x"] = g_window_tbl["cloud_x"] + 1
	if g_window_tbl["cloud_x"] == 320 then
		g_window_tbl["cloud_x"] = 0
	end

	g_window_tbl["cloud1"].x = g_window_tbl["cloud_x"]
	g_window_tbl["cloud2"].x = g_window_tbl["cloud_x"] - 320

	if rand(0, 6) == 0 and g_window_tbl["lightning_counter"] == 0 then --fixme var_1a, var_14
		g_window_tbl["lightning_counter"] = rand(1, 4)
		g_window_tbl["lightning"].image = g_img_tbl[rand(11,18)]
		g_window_tbl["lightning"].visible = true
		g_window_tbl["lightning_x"] = rand(-5,320)
		g_window_tbl["lightning_y"] = rand(-5,200)
	end

	if g_window_tbl["lightning_counter"] > 0 then
		g_window_tbl["lightning"].x = g_window_tbl["lightning_x"] + rand(0, 3)
		g_window_tbl["lightning"].y = g_window_tbl["lightning_y"] + rand(0, 3)
	end

	if (g_window_tbl["lightning_counter"] > 0 and rand(0,3) == 0) or g_window_tbl["strike"].visible == true then
		canvas_set_palette_entry(0x58, 0x40,0x94,0xfc)
		canvas_set_palette_entry(0x5a, 0x40,0x94,0xfc)
		canvas_set_palette_entry(0x5c, 0x40,0x94,0xfc)
	else
		canvas_set_palette_entry(0x58, 0x20,0xa4,0x80)
		canvas_set_palette_entry(0x5a, 0x14,0x8c,0x74)
		canvas_set_palette_entry(0x5c, 0x0c,0x74,0x68)
	end

	if rand(0,1) == 0 then
		g_window_tbl["strike"].image = g_img_tbl[rand(19,23)]
	end

	if g_window_tbl["flash"] > 0 then
		g_window_tbl["flash"] = g_window_tbl["flash"] - 1
	else
		if rand(0,5) == 0 or g_window_tbl["strike"].visible == true then
			g_window_tbl["window"].image = g_img_tbl[27]
			g_window_tbl["flash"] = rand(1, 5)
		else
			g_window_tbl["window"].image = g_img_tbl[26]
		end
	end

	if g_window_tbl["drops"] < 100 then
		if rand(0,g_window_tbl["rain_delay"]) == 0 then
			g_window_tbl["rain_delay"] = g_window_tbl["rain_delay"] - 2
			if g_window_tbl["rain_delay"] < 1 then
				g_window_tbl["rain_delay"] = 1
			end
			i = g_window_tbl["drops"]
			rain[i].visible = true
			rain[i].image = g_img_tbl[rand(4,7)]
			rain[i].y = -4
			rain[i].x = rand(0,320)
			g_window_tbl["drops"] = i + 1
		end
	end

	for i = 0,g_window_tbl["drops"] do

		if rain[i].visible == true then
			rain[i].x = rain[i].x + 2
			rain[i].y = rain[i].y + 8
			if rain[i].x > 320 or rain[i].y > 200 then
				rain[i].visible = false
			end
		else
			rain[i].visible = true
			rain[i].image = g_img_tbl[rand(4,7)]
			rain[i].y = -4
			rain[i].x = rand(0,320)
		end
	end

	if g_window_tbl["lightning_counter"] > 0 then
		g_window_tbl["lightning_counter"] = g_window_tbl["lightning_counter"] - 1
	end
end

local function window_update()
	local input = input_poll()

	while input == nil do
		display_window()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
		canvas_update()
	end

	return should_exit(input)
end

local function window_sequence()
	load_images("intro_2.shp")

	load_window()

	canvas_set_palette("palettes.int", 2)

	local i = 0
	local input
	while i < 20 do
		display_window()
		canvas_update()
		input = input_poll()
		if should_exit(input) then
			return false
		end

		i = i + 1
		canvas_update()
	end

	local scroll_img = image_load("blocks.shp", 1)
	local scroll = sprite_new(scroll_img, 1, 0x98, true)

	local x, y = image_print(scroll_img, "...and in moments, the storm is upon you.", 8, 312, 36, 14, 0x3e)

	if window_update() == true then
		return false
	end

	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "Tongues of lightning lash the sky, conducting an unceasing ", 8, 310, 8, 10, 0x3e)
	image_print(scroll_img, "crescendo of thunder....", 8, 310, x, y, 0x3e)
	scroll.image = scroll_img

	if window_update() == true then
		return false
	end

	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "In a cataclysm of sound and light, a bolt of searing ", 8, 310, 8, 10, 0x3e)
	image_print(scroll_img, "blue fire strikes the earth!", 8, 310, x, y, 0x3e)
	scroll.image = scroll_img

	g_window_tbl["strike"].visible = true

	if window_update() == true then
		return false
	end

	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "Lightning among the stones!", 8, 310, 73, 10, 0x3e)
	image_print(scroll_img, "Is this a sign from distant Britannia?", 8, 310, 41, 18, 0x3e)
	scroll.image = scroll_img

	--scroll window.
	i = 0
	while i < 320 do

		display_window()
		canvas_update()
		input = input_poll()
		if should_exit(input) then
			return false
		end

		canvas_update()

		g_window_tbl["window"].x = g_window_tbl["window"].x - 2
		g_window_tbl["frame"].x = g_window_tbl["frame"].x - 2
		g_window_tbl["door_left"].x = g_window_tbl["door_left"].x - 2
		g_window_tbl["door_right"].x = g_window_tbl["door_right"].x - 2
		i = i + 2
		if i > 160 and g_window_tbl["strike"].visible == true then
			g_window_tbl["strike"].visible = false
		end
	end

	if window_update() == true then
		return false
	end

	scroll.visible = false
	i = 0
	while i < 68 do

		display_window()
		canvas_update()
		input = input_poll()
		if should_exit(input) then
			return false
		end

		canvas_update()
		input = input_poll()

		g_window_tbl["door_left"].x = g_window_tbl["door_left"].x - 1
		g_window_tbl["door_right"].x = g_window_tbl["door_right"].x + 1
		i = i + 1
	end

	scroll_img = image_load("blocks.shp", 2)
	x, y = image_print(scroll_img, "You bolt from your house, stumbling, running blind in the", 7, 310, 8, 12, 0x3e)
	x, y = image_print(scroll_img, " storm. Into the forest, down the path, through the ", 7, 310, x, y, 0x3e)
	image_print(scroll_img, "rain... to the stones.", 7, 310, x, y, 0x3e)
	scroll.image = scroll_img
	scroll.visible = true

	if window_update() == true then
		return false
	end

	scroll.visible = false

end

local function stones_rotate_palette()
	if g_pal_counter == 4 then
		canvas_rotate_palette(144, 16)
		g_pal_counter = 0
	else
		g_pal_counter = g_pal_counter + 1
	end
end

local function stones_update()
	local input = input_poll()

	while input == nil do
		stones_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	end

	return should_exit(input)
end

local function stones_shake_moongate()
	local input = input_poll()

	while input == nil do
		stones_rotate_palette()
		g_stones_tbl["moon_gate"].x = 0x7c + math.random(0, 1)
		g_stones_tbl["moon_gate"].y = 5 + math.random(0, 3)
		canvas_update()
		stones_rotate_palette()
		g_stones_tbl["moon_gate"].x = 0x7c + math.random(0, 1)
		g_stones_tbl["moon_gate"].y = 5 + math.random(0, 3)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	end

	return should_exit(input)
end

g_pal_counter = 0

g_stones_tbl = {}

local function stones_sequence()

	load_images("intro_3.shp")

	canvas_set_palette("palettes.int", 3)

	g_stones_tbl["bg"] = sprite_new(g_img_tbl[0], 0, 0, true)
	g_stones_tbl["stone_cover"] = sprite_new(g_img_tbl[3], 0x96, 0x64, false)
	g_stones_tbl["gate_cover"] = sprite_new(g_img_tbl[4], 0x5e, 0x66, false)
	g_stones_tbl["hand"] = sprite_new(g_img_tbl[1], 0xbd, 0xc7, false)
	g_stones_tbl["moon_gate"] = sprite_new(g_img_tbl[2], 0x7c, 0x64, false)
	g_stones_tbl["moon_gate"].clip_x = 0
	g_stones_tbl["moon_gate"].clip_y = 0
	g_stones_tbl["moon_gate"].clip_w = 320
	g_stones_tbl["moon_gate"].clip_h = 0x66
	g_stones_tbl["avatar"] = sprite_new(g_img_tbl[7], -2, 0x12, false)

	local scroll_img = image_load("blocks.shp", 2)
	local scroll = sprite_new(scroll_img, 1, 0xc, true)

	local x, y = image_print(scroll_img, "Near the stones, the smell of damp, blasted earth hangs ", 7, 303, 7, 8, 0x3e)
	x, y = image_print(scroll_img, "in the air. In a frozen moment of lightning-struck ", 7, 303, x, y, 0x3e)
	x, y = image_print(scroll_img, "daylight, you glimpse a tiny obsidian stone in the ", 7, 303, x, y, 0x3e)
	x, y = image_print(scroll_img, "midst of the circle!", 7, 303, x, y, 0x3e)

	fade_in()

	if stones_update() == true then
		return false
	end

	g_stones_tbl["stone_cover"].visible = true
	g_stones_tbl["hand"].visible = true

	scroll_img = image_load("blocks.shp", 0)
	image_print(scroll_img, "Wondering, you pick it up....", 8, 234, 0x2a, 8, 0x3e)
	scroll.image = scroll_img
	scroll.x = 0x21
	scroll.y = 0x1e

	local i
	for i=0xc7,0x54,-2  do
		--		display_stones()
		g_stones_tbl["hand"].y = i
		canvas_update()
		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	if stones_update() == true then
		return false
	end

	g_stones_tbl["bg"].image = g_img_tbl[5]
	g_stones_tbl["stone_cover"].visible = false
	g_stones_tbl["gate_cover"].visible = true

	scroll_img = image_load("blocks.shp", 1)
	x, y = image_print(scroll_img, "...and from the heart of the stones, a softly glowing door ", 7, 303, 7, 10, 0x3e)
	image_print(scroll_img, "ascends in silence!", 7, 303, x, y, 0x3e)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0

	g_stones_tbl["moon_gate"].visible = true

	for i=0x64,0x5,-1  do
		--		display_stones()
		stones_rotate_palette()
		g_stones_tbl["moon_gate"].y = i
		canvas_update()
		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	for i=0x54,0xc7,2  do
		--		display_stones()
		stones_rotate_palette()
		g_stones_tbl["hand"].y = i
		canvas_update()
		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	g_stones_tbl["hand"].visible = false

	if stones_update() == true then
		return false
	end

	g_stones_tbl["hand"].image = g_img_tbl[6]
	g_stones_tbl["hand"].x = 0x9b
	g_stones_tbl["hand"].visible = true
	scroll.visible = false

	for i=0xc7,0x44,-2  do
		stones_rotate_palette()
		g_stones_tbl["hand"].y = i
		g_stones_tbl["bg"].y = g_stones_tbl["bg"].y - 1
		g_stones_tbl["gate_cover"].y = g_stones_tbl["gate_cover"].y - 1
		g_stones_tbl["moon_gate"].y = g_stones_tbl["moon_gate"].y - 1
		canvas_update()
		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	scroll_img = image_load("blocks.shp", 2)
	x, y = image_print(scroll_img, "Exultant memories wash over you as you clutch the stone. ", 7, 303, 7, 8, 0x3e)
	x, y = image_print(scroll_img, "When last you saw an orb such as this, it was cast down ", 7, 303, x, y, 0x3e)
	image_print(scroll_img, "by Lord British to banish the tyrant Blackthorn!", 7, 303, x, y, 0x3e)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x98
	scroll.visible = true

	if stones_update() == true then
		return false
	end

	scroll.visible = false

	for i=0x44,0xc7,2  do
		stones_rotate_palette()
		g_stones_tbl["hand"].y = i
		g_stones_tbl["bg"].y = g_stones_tbl["bg"].y + 1
		g_stones_tbl["gate_cover"].y = g_stones_tbl["gate_cover"].y + 1
		g_stones_tbl["moon_gate"].y = g_stones_tbl["moon_gate"].y + 1
		canvas_update()
		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	g_stones_tbl["hand"].visible = false

	scroll_img = image_load("blocks.shp", 2)
	image_print(scroll_img, "But your joy soon gives way to apprehension.", 7, 303, 16, 8, 0x3e)
	image_print(scroll_img, "The gate to Britannia has always been blue...", 7, 303, 18, 24, 0x3e)
	image_print(scroll_img, "as blue as the morning sky.", 7, 303, 76, 32, 0x3e)
	scroll.image = scroll_img
	scroll.visible = true

	if stones_update() == true then
		return false
	end

	scroll_img = image_load("blocks.shp", 1)
	x,y = image_print(scroll_img, "Abruptly, the portal quivers and begins to sink ", 7, 303, 7, 10, 0x3e)
	image_print(scroll_img, "into the ground.  Its crimson light wanes!", 7, 303, x, y, 0x3e)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0

	if stones_shake_moongate() == true then
		return false
	end

	scroll_img = image_load("blocks.shp", 1)
	x,y = image_print(scroll_img, "Desperation makes the decision an easy one.", 7, 303, 22, 14, 0x3e)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0

	if stones_shake_moongate() == true then
		return false
	end

	scroll.visible = false

	g_stones_tbl["avatar"].visible = true

	canvas_set_palette_entry(0x19, 0, 0, 0)

	for i=0,19,1 do
		g_stones_tbl["avatar"].image = g_img_tbl[7+i]

		local j
		for j=0,4 do
			canvas_update()
			stones_rotate_palette()
			g_stones_tbl["moon_gate"].x = 0x7c + math.random(0, 1)
			g_stones_tbl["moon_gate"].y = 5 + math.random(0, 3)
		end

		g_stones_tbl["avatar"].y = g_stones_tbl["avatar"].y - 3

		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end


	for i=0xff,0,-3 do
		canvas_update()
		stones_rotate_palette()
		g_stones_tbl["moon_gate"].x = 0x7c + math.random(0, 1)
		g_stones_tbl["moon_gate"].y = 5 + math.random(0, 3)

		g_stones_tbl["avatar"].opacity = i

		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	canvas_set_palette_entry(0x19, 0x74, 0x74, 0x74)

	g_stones_tbl["moon_gate"].x = 0x7c

	for i=0x5,0x64,1  do
		stones_rotate_palette()
		g_stones_tbl["moon_gate"].y = i
		canvas_update()
		local input = input_poll()
		if input ~= nil and should_exit(input) then
			return false
		end
	end

	g_stones_tbl["bg"].image = g_img_tbl[0]
	g_stones_tbl["moon_gate"].visible = false
	g_stones_tbl["gate_cover"].visible = false
	g_stones_tbl["stone_cover"].visible = true

	if stones_update() == true then
		return false
	end

	return true
end

local function play()
mouse_cursor_set_pointer(0)
mouse_cursor_visible(false)
load_images("intro_1.shp")
music_play("bootup.m")
--[ [
canvas_set_palette("palettes.int", 0)
canvas_set_update_interval(25)

local img = g_img_tbl[0x45]

local background = sprite_new(img)

background.x = 0
background.y = 0
background.opacity = 0
background.visible = true

music_play("bootup.m")
for i=0,0xff,3 do
	background.opacity = i
	if poll_for_esc() == true then return end
	canvas_update()
end

for i=0xff,0,-3 do
background.opacity = i
	if poll_for_esc() == true then return end

--if input ~= nil and input == ESCAPE_KEY then
--	return
--end
canvas_update()
end

img = g_img_tbl[0x46]

background.image = img

for i=0,0xff,3 do
	background.opacity = i
		if poll_for_esc() == true then return end

	canvas_update()
end

for i=0xff,0,-3 do
background.opacity = i
	if poll_for_esc() == true then return end

canvas_update()
end

background.visible = false



if lounge_sequence() == false then
	return
end

if window_sequence() == false then
	return
end

fade_out()
hide_window()
--] ]
stones_sequence()
end

local function gypsy_update_bubbles(bubble_image)
	if g_gypsy_tbl["bubble_counter"] == 0 then
		image_bubble_effect(bubble_image)
		g_gypsy_tbl["bubble_counter"] = 3
	else
		g_gypsy_tbl["bubble_counter"] = g_gypsy_tbl["bubble_counter"] - 1
	end
end


local function gypsy_ab_select(question)
	local a_lookup_tbl = {
	0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 2, 2, 2,
	2, 2, 3, 3, 3, 3, 4, 4,
	4, 5, 5, 6,
	}

	local b_lookup_tbl = {
	1, 2, 3, 4, 5, 6, 7, 2,
	3, 4, 5, 6, 7, 3, 4, 5,
	6, 7, 4, 5, 6, 7, 5, 6,
	7, 6, 7, 7,
	}
	if g_highlight_index == nil then
		g_highlight_index = 0
	end

	local input = nil
	while input == nil do
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if input == 65 or input == 97 or (input == 13 and g_highlight_index == 0) then
				return a_lookup_tbl[question - 104] + 1
			elseif input == 66 or input == 98 or input == 13 then
				return b_lookup_tbl[question - 104] + 1
			elseif input == SDLK_RIGHT and g_highlight_index == 0 then -- right
				g_highlight_index = 1
				g_ab_highlight.x = g_ab_highlight.x + 17
			elseif input == SDLK_LEFT and g_highlight_index == 1 then -- left
				g_highlight_index = 0
				g_ab_highlight.x = g_ab_highlight.x - 17
			elseif input == 0 then
				local y = get_mouse_y()
				if(y > 173 and y < 190) then
					local x = get_mouse_x()
					if x > 278 and x < 296 then
						return a_lookup_tbl[question - 104] + 1
					elseif x > 296 and x < 313 then
						return b_lookup_tbl[question - 104] + 1
					end
				end
			end
			input = nil
		end
		if g_gypsy_tbl["jar_liquid"].visible == true then
			gypsy_update_bubbles(g_gypsy_tbl["jar_liquid"].image)
		end
	end
end


local gypsy_question_text = {
"\"Entrusted to deliver an uncounted purse of gold, thou dost meet a poor beggar. Dost thou A) deliver the gold Honestly, knowing the trust in thee was well-placed; or B) show Compassion and give the beggar a coin, knowing it won't be missed?\127",
"\"Thou has been prohibited by thy absent Lord from joining thy friends in a close-pitched battle. Dost thou A) refrain, so thou may Honestly claim obedience; or B) show Valor and aid thy comrades, knowing thou may deny it later?\127",
"\"A merchant owes thy friend money, now long past due. Thou dost see the same merchant drop a purse of gold. Dost thou A) Honestly return the purse intact; or B) Justly give thy friend a portion of the gold first?\127",
"\"Thee and thy friend are valiant but penniless warriors. Thou both set forth to slay a mighty dragon. Thy friend dost think he slew it, but the killing blow was thine. Dost thou A) Honestly claim the reward; or B) Sacrifice the gold for the sake of thy friendship?\127",
"\"Thou art sworn to protect thy Lord at any cost, yet thou knowest he hath committed a crime. Authorities ask thee of the affair. Dost thou A) Break thine oath by Honestly speaking; or B) uphold Honor by silently keeping thine oath?\127",
"\"Thy friend doth seek admittance to thy spiritual order. Thou art asked to vouch for his purity of spirit, of which thou art uncertain. Dost thou A) Honestly express thy doubt; or B) Vouch for him, hoping for his Spiritual improvement?\127",
"\"Thy Lord doth mistakenly believe he slew a dragon. Thou hast proof that thy lance felled the beast. When asked, dost thou A) Honestly claim the kill; or B) Humbly permit thy Lord his belief?\127",
"\"Thou dost manage to disarm thy mortal enemy in a duel. He is at thy mercy. Dost thou A) Show Compassion by permitting him to yield; or B) Slay him, as expected of a Valiant duelist?\127",
"\"After twenty years thou hast found the slayer of thy best friends. The villain proves to be a man who provides the sole support for a young girl. Dost thou A) Spare him in Compassion for the child; or B) Slay him in the name of Justice?\127",
"\"Thee and thy friends hath been routed and ordered to retreat. In defiance of thy orders, dost thou A) Stop in Compassion to aid a wounded comrade; or B) Sacrifice thyself to slow the pursuing enemy so that others might escape?\127",
"\"Thou art sworn to uphold a Lord who doth participate in the forbidden torture of prisoners. Each night their cries of pain reach thee. Dost thou A) Show Compassion by reporting the deeds; or B) Honor thy oath and ignore the deeds?\127",
"\"Thou hast been taught to preserve all life as sacred. A man lies fatally stung by a venomous serpent. He prays for a merciful death. Dost thou A) Show Compassion and end his pain; or B) heed thy Spiritual beliefs and turn away?\127",
"\"The Captain of the King's Guard asks that one among thee visit a hospital to cheer the children with tales of valiant personal deeds. Dost thou A) Show thy Compassion and play the braggart; or B) Humbly let another go?\127",
"\"Thou hast been sent to secure a needed treaty with a distant Lord. Thy host is agreeable to thy proposal but insults thy country at dinner. Dost thou A) Valiantly bear the slurs; or B) Justly rise and demand an apology?\127",
"\"A burly knight accosts thee and demands thy food. Dost thou A) Valiantly refuse and engage the knight, or B) Sacrifice thy rations unto the hungry knight?\127",
"\"During battle thou art ordered to guard thy commander's empty tent. The battle goes poorly and thou dost yearn to aid thy fellows. Dost thou A) Valiantly enter the battle; or B) Honor thy post as guard?\127",
"\"A local bully pushes for a fight. Dost thou A) Valiantly trounce the rogue; or B) Decline, knowing in thy Spirit that no lasting good will come of it?\127",
"\"Although a simple fisherman, thou art also a skillful swordsman. Thy Lord dost seek to assemble a peacetime guard. Dost thou A) Answer the call, so that all may witness thy Valor; or B) Humbly decline the offer to join thy Lord's largely ceremonial knighthood?\127",
"\"During a pitched battle, thou dost see a fellow desert his post, endangering many. As he flees, he is set upon by several enemies. Dost thou A) Justly let him fight alone; or B) Risk the Sacrifice of thine own life to aid him?\127",
"\"Thou hast sworn to do thy Lord's bidding in all. He covets a piece of land and orders its owner removed. Dost thou A) Serve Justice, refusing to act, thus being disgraced; or B) Honor thine oath and evict the landowner?\127",
"\"Thou dost believe that virtue resides in all people. Thou dost see a rogue steal from thy Lord. Dost thou A) Call him to Justice; or B) Personally try to sway him back to the Spiritual path of good?\127",
"\"Unwitnessed, thou hast slain a mighty dragon in defense of thy life. A poor warrior claims the offered reward. Dost thou A) Justly step forward to claim the bounty; or B) Humbly go about thy life, secure in thy self-esteem?\127",
"\"Thou art a bounty hunter sworn to return an alleged murderer. After his capture, thou dost come to believe him innocent. Dost thou A) Sacrifice thy sizable bounty for thy belief; or B) Honor thine oath to return him as promised?\127",
"\"Thou hast spent thy life in charitable and righteous work. Thine uncle the innkeeper lies ill and asks thee to take over his tavern. Dost thou A) Sacrifice thy life of purity to aid thy kin; or B) Decline and follow the call of Spirituality?\127",
"\"Thou art an elderly, wealthy merchant. Thy end is near. Dost thou A) Sacrifice all thy wealth to feed hundreds of starving children, receiving public adulation; or B) Humbly live out thy life, willing thy fortune to thy heirs?\127",
"\"In thy youth thou didst pledge to marry thy sweetheart. Now thou art on a sacred quest in distant lands. Thy sweetheart doth ask thee to keep thy vow. Dost thou A) Honor thy pledge to wed; or B) Follow thy Spiritual crusade?\127",
"\"Though thou art but a peasant shepherd, thou art discovered to be the sole descendant of a noble family long thought extinct. Dost thou A) Honorably take up the arms of thy ancestors; or B) Humbly resume thy life of simplicity and peace?\127",
"\"Thy parents wish thee to become an apprentice. Two positions are available. Dost thou A) Become an acolyte in a worthy Spiritual order; or B) Become an assistant to a humble village cobbler?\127",
}

	local gypsy_questions = {
	-1, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x69, -1, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
	0x6A, 0x70, -1, 0x76, 0x77, 0x78, 0x79, 0x7A,
	0x6B, 0x71, 0x76, -1, 0x7B, 0x7C, 0x7D, 0x7E,
	0x6C, 0x72, 0x77, 0x7B, -1, 0x7F, 0x80, 0x81,
	0x6D, 0x73, 0x78, 0x7C, 0x7F, -1, 0x82, 0x83,
	0x6E, 0x74, 0x79, 0x7D, 0x80, 0x82, -1, 0x84,
	0x6F, 0x75, 0x7A, 0x7E, 0x81, 0x83, 0x84, -1,
	}

	g_str = 0xf
	g_dex = 0xf
	g_int = 0xf

	g_question_tbl = {0, 1, 2, 3, 4, 5, 6, 7}

local function shuffle_question_tbl(shuffle_len)
	local random = math.random
	local c = random(0, (shuffle_len * shuffle_len)-1) + shuffle_len

	for i=0,c,1 do
		local j = random(1, shuffle_len)
		local k = random(1, shuffle_len)

		local tmp = g_question_tbl[j]

		g_question_tbl[j] = g_question_tbl[k]
		g_question_tbl[k] = tmp
	end

end

local function gypsy_start_pouring(vial_num, vial_level)
	local pour_img_tbl =
	{
	 0x2B, 0x43, 0x56, 0x6B, 0x24, 0x74, 0x19, 0x2C,
	 0x2C, 0x44, 0x57, 0x6C, 0x23, 0x73, 0x18, 0x2B,
	 0x2E, 0x45, 0x58, 0x6D, 0x22, 0x72, 0x17, 0x2A,
	}

	if vial_level <= 3 and vial_level > 0 then
		g_gypsy_tbl["pour"].visible = true
		local img1 = pour_img_tbl[(3 - vial_level) * 8 + vial_num]
		--io.stderr:write("pour: "..vial_level.." img1="..img1.."vial_num="..vial_num.."\n")
		if vial_num > 6 then
			img1 = img1 + 9
		else
			img1 = img1 + 0x42
		end

		g_gypsy_tbl["pour"].image = g_gypsy_img_tbl[img1]

		local pour_y_tbl = {0x32, 0x37, 0x40}
		g_gypsy_tbl["pour"].y = pour_y_tbl[3 - vial_level + 1] - 20

		local pour_x_tbl =
		{
		 0x92, 0x92, 0x92, 0x92, 0x0A9, 0x0A9, 0x0A9, 0x0A9,
		 0x91, 0x91, 0x91, 0x91, 0x0A9, 0x0A9, 0x0A9, 0x0A9,
		 0x94, 0x94, 0x94, 0x94, 0x0AA, 0x0AA, 0x0AA, 0x0AA,
		}

		g_gypsy_tbl["pour"].x = pour_x_tbl[(3 - vial_level) * 8 + vial_num]
	end

	g_gypsy_tbl["jar_level"] = g_gypsy_tbl["jar_level"] + 1

	g_gypsy_tbl["jar_liquid"].visible = true
	g_gypsy_tbl["jar_liquid"].image = g_gypsy_img_tbl[8 + g_gypsy_tbl["jar_level"]]
	g_gypsy_tbl["jar_liquid"].y = g_gypsy_tbl["jar_liquid"].y - 1

	local vial_colors = {239, 14, 231, 103, 228, 5, 15, 219}

	image_bubble_effect_add_color(vial_colors[vial_num])
	g_gypsy_tbl["bubble_counter"] = 0

end

local function gypsy_stop_pouring()
	g_gypsy_tbl["pour"].visible = false
end

local function gypsy_vial_anim_liquid(img_num, vial_num, vial_level, hand_x, hand_y)

	--io.stderr:write(img_num..", "..vial_num..", "..vial_level..", "..hand_x..", "..hand_y.."\n")

	if vial_level == 3 then
		vial_level = 2
	end

	if vial_level <= 0 then
		g_gypsy_tbl["vial_liquid"][vial_num].visible = false
		return
	end

	local si = 0
	if img_num == 0xf then return end


	if img_num > 0xf then
		if img_num == 0x12 then
			si = 1
		else
			if img_num > 0x12 then
				if img_num == 0x25 then return end
				if img_num == 0x2e then
					si = 2
				end
			else
				if img_num == 0x10 then
					si = 2
				else
					if img_num == 0x11 then
						si = 0
					end
				end
			end
		end

	else

		img_num = img_num - 9
		if img_num == 0 then
			si = 0
		elseif img_num == 1 then
			si = 1
		elseif img_num >= 2 and img_num <= 5 then
			si = 3
		end

	end

	local vial_liquid_tbl = {
	0x3A, 0x4F, 0x64, 0x7C, 0x28, 0x7E, 0x20, 0x34,
	0x36, 0x4B, 0x60, 0x76, 0x26, 0x78, 0x1C, 0x30,
	0x36, 0x4B, 0x60, 0x76, 0x26, 0x78, 0x1C, 0x30,
	0x3C, 0x51, 0x66, 0x7F, 0x14, 0x5F, 0x22, 0x36,
	0x38, 0x4D, 0x62, 0x79, 0x27, 0x7B, 0x1E, 0x32,
	0x38, 0x4D, 0x62, 0x79, 0x27, 0x7B, 0x1E, 0x32,
	0x40, 0x53, 0x68, 0x81, 0x29, 0x83, 0x24, 0x38,
	0x40, 0x53, 0x68, 0x81, 0x29, 0x83, 0x24, 0x38,
	0x40, 0x53, 0x68, 0x81, 0x29, 0x83, 0x24, 0x38,
	0x0, 0x3, 0x6, 0x9, 0x16, 0x19, 0x1C, 0x1F,
	0x1, 0x4, 0x7, 0x0A, 0x17, 0x1A, 0x1D, 0x20,
	0x2, 0x5, 0x8, 0x0B, 0x18, 0x1B, 0x1E, 0x21
	}

	local img_offset

	if vial_num > 6 and si ~= 3 then
		img_offset = 9
	else
		img_offset = 0x42
	end

	--io.stderr:write("si ="..si.."\n")

	if vial_level > 0 and vial_level < 3 then
		--vial_liquid_tbl[vial_level * 2 + si * 24 + vial_num] + img_offset
		local img_idx = vial_liquid_tbl[(vial_level-1) * 8 + si * 24 + vial_num]
		g_gypsy_tbl["vial_liquid"][vial_num].image = g_gypsy_img_tbl[img_idx + img_offset]

		local hand_y_tbl = {0x1B, 0x13, 0x13, 0x16, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0B, 0x0B, 0x0B}
		g_gypsy_tbl["vial_liquid"][vial_num].y = hand_y + hand_y_tbl[si * 3 + vial_level] - 20

		local hand_x_tbl =
		{
		0x4, 0x4, 0x4, 0x4, 0x14, 0x14, 0x14, 0x14,
		0x4, 0x4, 0x4, 0x4, 0x13, 0x13, 0x13, 0x13,
		0x4, 0x4, 0x4, 0x4, 0x13, 0x13, 0x13, 0x13,
		0x4, 0x4, 0x4, 0x4, 0x17, 0x17, 0x17, 0x17,
		0x3, 0x3, 0x3, 0x3, 0x17, 0x17, 0x17, 0x17,
		0x3, 0x3, 0x3, 0x3, 0x17, 0x17, 0x17, 0x17,
		0x3, 0x3, 0x3, 0x3, 0x1B, 0x1B, 0x1B, 0x1B,
		0x3, 0x3, 0x3, 0x3, 0x1B, 0x1B, 0x1B, 0x1B,
		0x3, 0x3, 0x3, 0x3, 0x1B, 0x1B, 0x1B, 0x1B,
		0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
		0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
		0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
		}

		local hand_x_tbl1 = {10, 10, 10, 10, -13, -13, -13, -13}

		g_gypsy_tbl["vial_liquid"][vial_num].x = hand_x + hand_x_tbl[si * 24 + (vial_level-1) * 8 + vial_num] + hand_x_tbl1[vial_num]
	end

end

local function gypsy_vial_anim(vial)

	local vial_level = g_gypsy_tbl["vial_level"]
	local vial_img_off = {2, 5, 8, 0xB, 0x18, 0x1B, 0x1E, 0x21}

	local vial_x_offset = {41, 62, 83, 104, 200, 221, 242, 263}

	local idx

	--g_gypsy_tbl["vial"][vial].visible = false
	--g_gypsy_tbl["vial_liquid"][vial].visible = false


	local arm_img_tbl = {0, 0, 0, 0, 5, 5, 5, 5}
	local arm_img_tbl1 = {1, 1, 1, 1, 4, 4, 4, 4}
	local arm_img_tbl2 = {2, 2, 2, 2, 3, 3, 3, 3}

	local hand_img_tbl = {12, 12, 12, 12, 13, 13, 13, 13}
	local hand_img_tbl1 = {9, 9, 9, 9, 17, 17, 17, 17}
	local hand_img_tbl2 = {10, 10, 10, 10, 18, 18, 18, 18}
	local hand_img_tbl3 = {16, 16, 16, 16, 46, 46, 46, 46}
	local hand_img_tbl4 = {15, 15, 15, 15, 37, 37, 37, 37}

	local arm_x_offset = {93, 93, 93, 93, 172, 172, 172, 172}

	local hand_x_offset = {29, 50, 71, 92, 202, 223, 244, 264}
	local hand_x_offset1 = {107, 107, 107, 107, 170, 170, 170, 170}
	local hand_x_offset2 = {109, 109, 109, 109, 168, 168, 168, 168}
	local hand_x_offset3 = {112, 112, 112, 112, 165, 165, 165, 165}
	local hand_x_offset4 = {10, 10, 10, 10, -13, -13, -13, -13}

	local hand_img_num
	local should_update
	for idx=0,16 do
		should_update = true
		if idx == 0 then
			--
			hand_img_num = 0xf
			should_update = false
		elseif idx == 1 or idx == 15 then
			--
			g_gypsy_tbl["arm"].visible = true
			g_gypsy_tbl["hand"].visible = true
			g_gypsy_tbl["vial"][vial].visible = false
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl[vial]]
			g_gypsy_tbl["arm"].x = vial_x_offset[vial] - 6
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = vial_x_offset[vial]
			g_gypsy_tbl["hand"].y = 66
			g_gypsy_tbl["hand"].visible = true
		elseif idx == 2 or idx == 14 then
			--
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl1[vial]]
			g_gypsy_tbl["arm"].x = vial_x_offset[vial] - 6
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = vial_x_offset[vial]
			g_gypsy_tbl["hand"].y = 29

		elseif idx == 3 or idx == 13 then
			--
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl1[vial]]
			g_gypsy_tbl["arm"].x = vial_x_offset[vial] - 6
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl1[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = hand_x_offset[vial]
			g_gypsy_tbl["hand"].y = 25

		elseif idx == 4 or idx == 12 then
			--
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl2[vial]]
			g_gypsy_tbl["arm"].x = arm_x_offset[vial]
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl1[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = hand_x_offset1[vial]
			g_gypsy_tbl["hand"].y = 20

		elseif idx == 5 or idx == 11 then
			--
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl2[vial]]
			g_gypsy_tbl["arm"].x = arm_x_offset[vial]
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl2[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = hand_x_offset1[vial]
			g_gypsy_tbl["hand"].y = 20

			if idx == 11 then
				vial_level[vial] = vial_level[vial] - 1
				if vial_level[vial] == 2 then
					gypsy_stop_pouring()
				end
			end

			if vial_level[vial] == 3 then
				gypsy_start_pouring(vial, vial_level[vial])
			end
		elseif idx == 6 or idx == 10 then
			--
			if vial_level[vial] > 2 then
				should_update = false
			end
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl2[vial]]
			g_gypsy_tbl["arm"].x = arm_x_offset[vial]
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl3[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = hand_x_offset2[vial]
			g_gypsy_tbl["hand"].y = 21

			if vial_level[vial] == 2 then
				if idx == 6 then
					gypsy_start_pouring(vial, vial_level[vial])
				elseif idx == 10 then
					gypsy_stop_pouring()
				end
			end
		elseif idx == 7 or idx == 8 or idx == 9 then
			--
			if vial_level[vial] > 1 then
				should_update = false
			end
			g_gypsy_tbl["arm"].image = g_gypsy_img_tbl[1 + arm_img_tbl2[vial]]
			g_gypsy_tbl["arm"].x = arm_x_offset[vial]
			g_gypsy_tbl["arm"].y = 21

			hand_img_num = hand_img_tbl4[vial]
			g_gypsy_tbl["hand"].image = g_gypsy_img_tbl[9 + hand_img_num]
			g_gypsy_tbl["hand"].x = hand_x_offset3[vial]
			g_gypsy_tbl["hand"].y = 13

			if vial_level[vial] == 1 then
				if idx == 7 then
					gypsy_start_pouring(vial, vial_level[vial])
					g_gypsy_tbl["vial_liquid"][vial].visible = false
				elseif idx == 9 then
					gypsy_stop_pouring()
				end
			end
		end

		--if idx > 3 and idx < 9 or idx > 8 and idx < 13 then
		--	g_gypsy_tbl["hand"].x = g_gypsy_tbl["hand"].x - hand_x_offset4[vial]
		--end

		--io.stderr:write("idx ="..idx.."\n")
		if should_update == true then
			gypsy_vial_anim_liquid(hand_img_num, vial, vial_level[vial], g_gypsy_tbl["hand"].x - hand_x_offset4[vial], g_gypsy_tbl["hand"].y + 20)

			--g_gypsy_tbl["hand"].x = g_gypsy_tbl["hand"].x + hand_x_offset4[vial]

			local j
			for j = 1,8 do
				if g_gypsy_tbl["jar_level"] > 0 then
					gypsy_update_bubbles(g_gypsy_tbl["jar_liquid"].image)
				end
				canvas_update()
				input_poll()
			end
		end
	--[[
		local j
		for j = 1,8 do
			local img = g_gypsy_tbl["jar_liquid"].image
			if img ~= nil then
				gypsy_update_bubbles(g_gypsy_tbl["jar_liquid"].image)
			end
			canvas_update()
			input_poll()
		end
	--]]
	end

	g_gypsy_tbl["hand"].visible = false

	--io.stderr:write("vial #"..vial.." level="..vial_level[vial].."\n")



	g_gypsy_tbl["hand"].visible = false
	g_gypsy_tbl["arm"].visible = false

	g_gypsy_tbl["vial"][vial].visible = true
	--g_gypsy_tbl["vial_liquid"][vial].visible = true
end

local function gypsy_ask_questions(num_questions, scroll)

	local strength_adjustment_tbl = { 0, 0, 2, 0, 1, 1, 1, 0 }
	local dex_adjustment_tbl = { 0, 2, 0, 1, 1, 0, 1, 0 }
	local int_adjustment_tbl = { 2, 0, 0, 1, 0, 1, 1, 0 }

	for i=0,num_questions-1,1 do
		local q = gypsy_questions[g_question_tbl[i*2+1]*8 + g_question_tbl[i*2+2] + 1]

		local scroll_img = image_load("blocks.shp", 3)
		scroll.image = scroll_img
		image_print(scroll_img, gypsy_question_text[q - 104], 7, 303, 8, 9, 0x3e)

		local vial = gypsy_ab_select(q)

		gypsy_vial_anim(vial)

		g_str = g_str + strength_adjustment_tbl[vial]
		g_dex = g_dex + dex_adjustment_tbl[vial]
		g_int = g_int + int_adjustment_tbl[vial]

		g_question_tbl[i+1] = vial-1

		--io.stderr:write(q.." "..vial.."("..g_str..","..g_dex..","..g_int..")\n")
	end
end



g_keycode_tbl =
{
[32]=" ",
[39]="'",
[44]=",",
[45]="-",
[46]=".",
[48]="0",
[49]="1",
[50]="2",
[51]="3",
[52]="4",
[53]="5",
[54]="6",
[55]="7",
[56]="8",
[57]="9",
[65]="A",
[66]="B",
[67]="C",
[68]="D",
[69]="E",
[70]="F",
[71]="G",
[72]="H",
[73]="I",
[74]="J",
[75]="K",
[76]="L",
[77]="M",
[78]="N",
[79]="O",
[80]="P",
[81]="Q",
[82]="R",
[83]="S",
[84]="T",
[85]="U",
[86]="V",
[87]="W",
[88]="X",
[89]="Y",
[90]="Z",

[97]="a",
[98]="b",
[99]="c",
[100]="d",
[101]="e",
[102]="f",
[103]="g",
[104]="h",
[105]="i",
[106]="j",
[107]="k",
[108]="l",
[109]="m",
[110]="n",
[111]="o",
[112]="p",
[113]="q",
[114]="r",
[115]="s",
[116]="t",
[117]="u",
[118]="v",
[119]="w",
[120]="x",
[121]="y",
[122]="z",
}
local function create_character()
	music_play("create.m")

	local bubbles = sprite_new(image_new(100,100, 0), 110, 30, false)
	local bg = sprite_new(image_load("vellum1.shp", 0), 0x10, 0x50, true)
	image_print(bg.image, "By what name shalt thou be called?", 7, 303, 36, 24, 0x48)

	local name = sprite_new(nil, 0x34, 0x78, true)
	name.text = ""
	local char_index = 0
	local input = nil

	while input == nil do
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) == true then
				bg.visible = false
				name.visible = false
				return false
			end
			local name_text = name.text
			local len = string.len(name_text)
			if (input == 8 or input == SDLK_LEFT) and len > 0 then
				name.text = string.sub(name_text, 1, len - 1)
				if len == 1 then -- old len
					char_index = 0
				else
					char_index = string.byte(name_text, len -1)
				end
			elseif input == 13 and len > 0 then --return
				break;
			elseif g_keycode_tbl[input] ~= nil and len < 13 then
				char_index = input
				name.text = name_text..g_keycode_tbl[input]
			elseif input == SDLK_UP then --up
				if char_index == 0 then
					if len > 0 then
						char_index = 97 --a
					else
						char_index = 65 --A
					end
				elseif char_index <= 32 then --gap in characters
					char_index = 48
				elseif char_index >= 57 and  char_index < 65 then --gap in characters
					char_index = 65
				elseif char_index >= 90 and char_index < 97 then --gap in characters
					char_index = 97
				elseif char_index >= 122 then --last char
					char_index = 32
				else
					char_index = char_index + 1
				end

				if len > 0 then -- erase char
					name_text = string.sub(name_text, 1, len - 1)
				end
				name.text = name_text..g_keycode_tbl[char_index]
			elseif input == SDLK_DOWN then --down
				if char_index == 0 then
					if len > 0 then
						char_index = 122 --z
					else
						char_index = 90 --Z
					end
				elseif char_index == 65 then --gap in characters
					char_index = 57
				elseif char_index == 97 then --gap in characters
					char_index = 90
				elseif char_index <= 32 then --first char
					char_index = 122
				elseif char_index <= 48 then --gap in characters
					char_index = 32
				else
					char_index = char_index - 1
				end

				if len > 0 then -- erase char
					name_text = string.sub(name_text, 1, len - 1)
				end
				name.text = name_text..g_keycode_tbl[char_index]
			elseif input == SDLK_RIGHT and len < 13 then --right
				char_index = 97 --a
				name.text = name_text.."a"
			end
			input = nil
		end
	end

	name.x = 0x10 + (284 - canvas_string_length(name.text)) / 2

	image_print(bg.image, "And art thou Male, or Female?", 7, 303, 52, 56, 0x48)
	local gender_sprite = sprite_new(nil, 154, 152, true)
	gender_sprite.text = ""

	input = nil
	local gender = 0
	while input == nil do
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) == true then
				bg.visible = false
				name.visible = false
				gender_sprite.visible = false
				return false
			end
			if input == 77 or input == 109 then
				gender = 1 --male
				break
			elseif input == 70 or input == 102 then
				gender = 0 --female
				break
			elseif input == SDLK_UP or input == SDLK_DOWN then --up and down
				if gender == 0 then
					gender = 1 --male
					gender_sprite.text = "M"
				else
					gender = 0 --female
					gender_sprite.text = "F"
				end
			elseif input == 13 and gender_sprite.text ~= "" then --return
				break;
			end

			input = nil
		end
	end
	gender_sprite.visible = false
	load_images("vellum1.shp")
	bg.image = g_img_tbl[0]

	name.y = 0x59

	image_draw_line(bg.image, 14, 19, 277, 19, 0x48)

	local new_sex = sprite_new(g_img_tbl[3], 0x5e, 0x70, true)
	local new_portrait = sprite_new(g_img_tbl[0x12], 0x3e, 0x81, true)
	local continue = sprite_new(g_img_tbl[0x10], 0x56, 0x92, true)
	local esc_abort = sprite_new(g_img_tbl[4], 0x50, 0xa3, true)

	local montage_img_tbl = image_load_all("montage.shp")
	local portrait_num = 0
	local avatar = sprite_new(montage_img_tbl[gender*6+portrait_num], 0xc3, 0x76, true)
	local button_index = 0
	local old_button_index = 0

	local sex_highlight = sprite_new(image_new(58, 2), new_sex.x, new_sex.y +14, true)
	image_draw_line(sex_highlight.image, 0, 0, 59, 0, 248)
	image_draw_line(sex_highlight.image, 0, 1, 59, 1, 248)

	local portrait_highlight = sprite_new(image_new(90, 2), new_portrait.x, new_portrait.y +14, false)
	image_draw_line(portrait_highlight.image, 0, 0, 91, 0, 248)
	image_draw_line(portrait_highlight.image, 0, 1, 91, 1, 248)

	local continue_highlight = sprite_new(image_new(66, 2), continue.x, continue.y +14, false)
	image_draw_line(continue_highlight.image, 0, 0, 67, 0, 248)
	image_draw_line(continue_highlight.image, 0, 1, 67, 1, 248)

	local esc_abort_highlight = sprite_new(image_new(72, 2), esc_abort.x, esc_abort.y +14, false)
	image_draw_line(esc_abort_highlight.image, 0, 0, 73, 0, 248)
	image_draw_line(esc_abort_highlight.image, 0, 1, 73, 1, 248)

	input = nil
	local exiting = false

	while input == nil do
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) == true  or (input == 13 and button_index == 3) then
				exiting = true
			end
			if input == 112 or input == 80 or (input == 13 and button_index == 1) then
				if portrait_num == 5 then
					portrait_num = 0
				else
					portrait_num = portrait_num + 1
				end

				avatar.image = montage_img_tbl[gender*6+portrait_num]
			elseif input == 115 or input == 83 or (input == 13 and button_index == 0) then
				if gender == 0 then
					gender = 1
				else
					gender = 0
				end

				avatar.image = montage_img_tbl[gender*6+portrait_num]
			elseif input == 99 or input == 67 or (input == 13 and button_index == 2) then
				break
			elseif input == SDLK_UP and button_index > 0 then --up
				button_index = button_index -1;
			elseif input == SDLK_DOWN and button_index < 3 then --down
				button_index = button_index +1;
			elseif input == 0 then -- FIXME redundant stuff
				local x = get_mouse_x()
				if(x < 152) then
					local y = get_mouse_y()
					if x > 93 and y > 111 and y < 128 then
						if gender == 0 then
							gender = 1
						else
							gender = 0
						end
						avatar.image = montage_img_tbl[gender*6+portrait_num]
					elseif x > 61 and y > 128 and y < 145 then
						if portrait_num == 5 then
							portrait_num = 0
						else
							portrait_num = portrait_num + 1
						end
						avatar.image = montage_img_tbl[gender*6+portrait_num]
					elseif x > 85 and y > 145 and y < 162 then
						break
					elseif x > 79 and y > 162 and y < 179 then
						exiting = true
					end
				end
			end
			if button_index ~= old_button_index or exiting == true then

				if old_button_index == 0 then
					sex_highlight.visible = false
				elseif old_button_index == 1 then
					portrait_highlight.visible = false
				elseif old_button_index == 2 then
					continue_highlight.visible = false
				else
					esc_abort_highlight.visible = false
				end

				if exiting == true then
					bg.visible = false
					name.visible = false
					new_sex.visible = false
					new_portrait.visible = false
					continue.visible = false
					esc_abort.visible = false
					avatar.visible = false
					return false
				end

				if button_index == 0 then
					sex_highlight.visible = true
				elseif button_index == 1 then
					portrait_highlight.visible = true
				elseif button_index == 2 then
					continue_highlight.visible = true
				else
					esc_abort_highlight.visible = true
				end

				old_button_index = button_index
			end
			input = nil
		end
	end
	mouse_cursor_visible(false)
	fade_out()
	canvas_hide_all_sprites()

	canvas_set_palette("palettes.int", 4)
	local woods_img_tbl = image_load_all("woods.shp")
	local woods = sprite_new(woods_img_tbl[0], 0, 0, true)
	local woods1 = sprite_new(woods_img_tbl[1], 0x140, 0, true)

	fade_in()

	local scroll_img = image_load("blocks.shp", 1)
	local scroll = sprite_new(scroll_img, 1, 0xa0, true)

	local x, y = image_print(scroll_img, "\"Welcome, O Seeker!\127", 7, 303, 96, 14, 0x3e)

	wait_for_input()

	scroll.visible = false

	input = nil
	for i=0,0xbd,2 do
		woods.x = woods.x - 2
		woods1.x = woods1.x - 2
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	end

	woods.x = -190
	woods1.x = 130

	scroll_img = image_load("blocks.shp", 2)
	scroll.x = 1
	scroll.y = 0x98
	image_print(scroll_img, "A lonely stroll along an unfamiliar forest path brings you upon a curious gypsy wagon, its exotic colors dappled in the summer shade.", 7, 303, 8, 12, 0x3e)
	scroll.image = scroll_img
	scroll.visible = true

	wait_for_input()

	scroll_img = image_load("blocks.shp", 2)
	scroll.x = 1
	scroll.y = 0x98
	image_print(scroll_img, "A woman's voice rings out with friendship, beckoning you into across the wagon's threshold and, as it happens, into another life....", 7, 303, 8, 12, 0x3e)
	scroll.image = scroll_img
	scroll.visible = true

	wait_for_input()

	scroll.visible = false

	fade_out()
	canvas_hide_all_sprites()
	canvas_set_palette("palettes.int", 5)

	g_gypsy_img_tbl = image_load_all("gypsy.shp")

	bg.image = g_gypsy_img_tbl[0]
	bg.x = 0
	bg.y = -20
	bg.visible = true

	g_gypsy_tbl = {}
	g_gypsy_tbl["arm"] = sprite_new(nil, 0, 0, false)
	g_gypsy_tbl["pour"] = sprite_new(nil, 0, 0, false)


	g_gypsy_tbl["jar_level"] = 0
	g_gypsy_tbl["jar_liquid"] = sprite_new(nil, 0x92, 0x53, false)
	g_gypsy_tbl["jar_liquid"].clip_x = 0
	g_gypsy_tbl["jar_liquid"].clip_y = 0
	g_gypsy_tbl["jar_liquid"].clip_w = 320
	g_gypsy_tbl["jar_liquid"].clip_h = 0x6d

	g_gypsy_tbl["jar"] = sprite_new(g_gypsy_img_tbl[16], 0x8e, 0x38, true)

	g_gypsy_tbl["bubble_counter"] = 0

	g_gypsy_tbl["vial_level"] = {3,3,3,3,3,3,3,3}
	local vial_level = g_gypsy_tbl["vial_level"]

	local vial_img_off = {2, 5, 8, 0xB, 0x18, 0x1B, 0x1E, 0x21}

	g_gypsy_tbl["vial_liquid"] = {
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[1] + vial_level[1] - 3], 44, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[2] + vial_level[2] - 3], 65, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[3] + vial_level[3] - 3], 86, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[4] + vial_level[4] - 3], 107, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[5] + vial_level[5] - 3], 203, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[6] + vial_level[6] - 3], 224, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[7] + vial_level[7] - 3], 245, 0x4d, true),
		sprite_new(g_gypsy_img_tbl[0x42 + vial_img_off[8] + vial_level[8] - 3], 266, 0x4d, true),
	}

	g_gypsy_tbl["hand"] = sprite_new(nil, 0, 0, false)

	g_gypsy_tbl["vial"] = {
		sprite_new(g_gypsy_img_tbl[20], 41, 0x42, true),
		sprite_new(g_gypsy_img_tbl[20], 62, 0x42, true),
		sprite_new(g_gypsy_img_tbl[20], 83, 0x42, true),
		sprite_new(g_gypsy_img_tbl[20], 104, 0x42, true),
		sprite_new(g_gypsy_img_tbl[23], 200, 0x42, true),
		sprite_new(g_gypsy_img_tbl[23], 221, 0x42, true),
		sprite_new(g_gypsy_img_tbl[23], 242, 0x42, true),
		sprite_new(g_gypsy_img_tbl[23], 263, 0x42, true),
	}

	fade_in()

	scroll.x = 1
	scroll.y = 0x7c
	scroll.visible = true

	local scroll_img = image_load("blocks.shp", 3)
	scroll.image = scroll_img
	x, y = image_print(scroll_img, "\"At last thou hast come to fulfill thy destiny,\127 the gypsy says. She smiles, as if in great relief.", 7, 303, 8, 19, 0x3e)
	image_print(scroll_img, "\"Sit before me now, and I shall pour the light of Virtue into the shadows of thy future.\127", 7, 303, 8, y+16, 0x3e)

	wait_for_input()

	scroll_img = image_load("blocks.shp", 3)
	scroll.image = scroll_img
	x, y = image_print(scroll_img, "On a wooden table eight bottles stand, a rainbow of bubbling liquids.", 7, 303, 8, 19, 0x3e)
	image_print(scroll_img, "\"Behold the Virtues of the Avatar,\127 the woman says. \"Let us begin the casting!\127", 7, 303, 8, y+16, 0x3e)

	wait_for_input()
	canvas_set_palette_entry(19, 200, 200, 200) -- fix mouse cursor
	canvas_set_palette_entry(27, 68, 68, 68) -- fix mouse cursor

	mouse_cursor_visible(true)
	local a_button = sprite_new(g_gypsy_img_tbl[7], 0x117, 0xae, true)
	local b_button = sprite_new(g_gypsy_img_tbl[8], 0x128, 0xae, true)
	g_ab_highlight = sprite_new(image_new(16, 2), a_button.x, a_button.y + 13, true)
	image_draw_line(g_ab_highlight.image, 0, 0, 17, 0, 248)
	image_draw_line(g_ab_highlight.image, 0, 1, 17, 1, 248)

	g_str = 0xf
	g_dex = 0xf
	g_int = 0xf



	shuffle_question_tbl(8)
	gypsy_ask_questions(4, scroll)

	shuffle_question_tbl(4)

	gypsy_ask_questions(2, scroll)
	gypsy_ask_questions(1, scroll)

	a_button.visible = false
	b_button.visible = false
	g_ab_highlight.visible = false
	mouse_cursor_visible(false)

	scroll_img = image_load("blocks.shp", 3)
	scroll.image = scroll_img
	image_print(scroll_img, "\"The path of the Avatar lies beneath thy feet, worthy "..name.text..",\127 the gypsy intones. With a mysterious smile, she passes you the flask of shimmering liquids. \"Drink of these waters and go forth among our people, who shall receive thee in joy!\127", 7, 303, 8, 16, 0x3e)

	-- wait_for_input()

	input = nil
	while input == nil do
		gypsy_update_bubbles(g_gypsy_tbl["jar_liquid"].image)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	end

	fade_out()

	canvas_hide_all_sprites()
	canvas_set_palette("palettes.int", 6)


	--local big_flask = sprite_new(g_gypsy_img_tbl[198], 0, 0, true)
	bubbles.visible = true
	bg.x = 0
	bg.y = 0
	bg.visible = true
	bg.image = g_gypsy_img_tbl[198]
	g_gypsy_tbl["bubble_counter"] = 0
	gypsy_update_bubbles(bubbles.image)

	fade_in()

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.x = 1
	scroll.y = 0x98
	scroll.visible = true
	image_print(scroll_img, "As you drink from the flask, vertigo overwhelms you. A soothing mist obscures the gypsy's face, and you sink without fear into an untroubled sleep.", 7, 303, 8, 8, 0x3e)

	--wait_for_input()

	input = nil
	while input == nil do
		gypsy_update_bubbles(bubbles.image)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			break
		end
	end

	scroll.visible = false

	canvas_set_bg_color(0x75)
	fade_out()

	bubbles.visible = false

	bg.image = g_gypsy_img_tbl[199]
	bg.x = 0
	bg.y = 0
	bg.visible = true

	scroll.visible = false

	fade_in()

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.visible = true
	image_print(scroll_img, "You wake in a different time, upon another world's shore. Though the Avatar's quests bring you both triumph and tragedy, never do you stray from the path of the Eight Virtues.", 7, 303, 8, 8, 0x3e)

	wait_for_input()

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.visible = true
	image_print(scroll_img, "The sagas of Ultima IV and Ultima V chronicle your perilous travels, and your name and your deeds are written forever among Britannia's legends....", 7, 303, 8, 8, 0x3e)

	wait_for_input()

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.visible = true
	image_print(scroll_img, "Finally, tempered by your struggles against the enemies of Virtue, you are proven ready to answer the epic challenge of Ultima VI!", 7, 303, 8, 12, 0x3e)

	wait_for_input()
	scroll.visible = false

	canvas_set_bg_color(0)
	fade_out()

	config_set("config/newgame", true)

	config_set("config/newgamedata/name", name.text)
	config_set("config/newgamedata/gender", gender)
	config_set("config/newgamedata/portrait", portrait_num)
	config_set("config/newgamedata/str", g_str)
	config_set("config/newgamedata/dex", g_dex)
	config_set("config/newgamedata/int", g_int)

	g_gypsy_img_tbl = nil
	g_gypsy_tbl = nil

	return true
end

local function ack_header(scroll_img)
	image_print(scroll_img, "Ultima VI", 7, 303, 115, 9, 0x3d)
	image_print(scroll_img, "A Lord British Production", 7, 303, 63, 17, 0x3d)
	image_draw_line(scroll_img, 9, 26, 274, 26, 0x3d)
end

local function acknowledgements()

	local bg = sprite_new(image_load("vellum1.shp", 0), 0x10, 0x50, true)

	ack_header(bg.image)

	image_print(bg.image, "Produced by", 7, 303, 106, 32, 0xC)
	image_print(bg.image, "Richard Garriott and Warren Spector", 7, 303, 28, 40, 0x48)

	image_print(bg.image, "Executive Producer", 7, 303, 82, 56, 0xC)
	image_print(bg.image, "Dallas Snell", 7, 303, 106, 64, 0x48)

	image_print(bg.image, "Programming", 7, 303, 104, 80, 0xC)
	image_print(bg.image, "Cheryl Chen    John Miles", 7, 303, 67, 88, 0x48)
	image_print(bg.image, "Herman Miller    Gary Scott Smith", 7, 303, 40, 96, 0x48)

	fade_in_sprite(bg)
	local input

	wait_for_input()

	bg.image = image_load("vellum1.shp", 0)

	ack_header(bg.image)

	image_print(bg.image, "Writing", 7, 303, 120, 47, 0xC)
	image_print(bg.image, "Stephen Beeman    Dr. Cat    \39Manda Dee", 7, 303, 25, 55, 0x48)
	image_print(bg.image, "Richard Garriott    Greg Malone", 7, 303, 46, 63, 0x48)
	image_print(bg.image, "John Miles    Herman Miller", 7, 303, 61, 71, 0x48)
	image_print(bg.image, "Todd Porter    Warren Spector", 7, 303, 50, 79, 0x48)

	wait_for_input()

	bg.image = image_load("vellum1.shp", 0)

	ack_header(bg.image)

	image_print(bg.image, "Art", 7, 303, 132, 31, 0xC)
	image_print(bg.image, "Keith Berdak    Daniel Bourbonnais", 7, 303, 37, 39, 0x48)
	image_print(bg.image, "Jeff Dee    \39Manda Dee", 7, 303, 75, 47, 0x48)
	image_print(bg.image, "Glen Johnson    Denis Loubet", 7, 303, 56, 55, 0x48)

	image_print(bg.image, "Music", 7, 303, 126, 71, 0xC)
	image_print(bg.image, "Ken Arnold    Iolo Fitzowen", 7, 303, 61, 79, 0x48)
	image_print(bg.image, "Herman Miller    Todd Porter", 7, 303, 56, 87, 0x48)

	wait_for_input()

	bg.image = image_load("vellum1.shp", 0)

	ack_header(bg.image)

	image_print(bg.image, "Quality Assurance", 7, 303, 87, 31, 0xC)
	image_print(bg.image, "Paul Malone    Mike Romero", 7, 303, 62, 39, 0x48)
	image_print(bg.image, "", 7, 303, 49, 47, 0x48)

	image_print(bg.image, "Additional Support", 7, 303, 84, 63, 0xC)
	image_print(bg.image, "Michelle Caddel    Melanie Fleming", 7, 303, 39, 71, 0x48)
	image_print(bg.image, "Alan Gardner    Jeff Hillhouse", 7, 303, 51, 79, 0x48)
	image_print(bg.image, "Sherry Hunter    Steve Muchow", 7, 303, 49, 87, 0x48)
	image_print(bg.image, "Cheryl Neeld", 7, 303, 104, 95, 0x48)

	wait_for_input()

	fade_out_sprite(bg, 4)

	bg.visible = false

	return true
end


local function intro_sway_gargs(sprite, idx, angry_flag)
	if math.random(0, 3) == 0 then
		return idx
	end

	local movement_tbl = {1,1,1, -1,-1,-1,  -1,-1,-1,  1, 1, 1}

	if idx == 12 then
		idx = 1
	else
		idx = idx + 1
	end

	sprite.x = sprite.x + movement_tbl[idx]

	return idx
end

local function moongate_rotate_palette(idx, num)
	if g_pal_counter == 3 then
		canvas_rotate_palette(232, 8)
		g_pal_counter = 0
	else
		g_pal_counter = g_pal_counter + 1
	end
end

local function intro_exit()
	fade_out()
	canvas_hide_all_sprites()
	canvas_set_palette("palettes.int", 0)
end

local function intro_wait()
	if should_exit(wait_for_input()) == true then
		intro_exit()
		return false
	end

	return true
end

local function intro()
	local input
	local intro_img_tbl = image_load_all("intro.shp")

	local bg = sprite_new(intro_img_tbl[6], 0, 0, true)
	local moongate = sprite_new(intro_img_tbl[7], 0x78, 0x3a, false)
	local gargs_left = sprite_new(intro_img_tbl[3], -84, 0x6d, true)
	local gargs_right = sprite_new(intro_img_tbl[4], 326, 0xc7, true)
	local garg_body = sprite_new(intro_img_tbl[8], 0xd5, 0x8d, false)
	local garg_head = sprite_new(intro_img_tbl[11], 0x123, 0x9b, false)
	local iolo = sprite_new(intro_img_tbl[1], 0xa8, 0xca, false)
	local shamino = sprite_new(intro_img_tbl[2], 0x44, 0x7a, false)
	local dupre = sprite_new(intro_img_tbl[0], -0x20, 0x7a, false)

	local avatar = sprite_new(intro_img_tbl[9], 0x31, 0x44, false)
	local alter = sprite_new(intro_img_tbl[5], 0, 0x70, true)
	local ropes = sprite_new(intro_img_tbl[12], 0xd2, 0x84, false)

	canvas_set_palette("palettes.int", 7)
	music_play("intro.m")

	fade_in()

	local scroll_img = image_load("blocks.shp", 2)
	local scroll = sprite_new(scroll_img, 1, 0x98, true)

	image_print(scroll_img, "Dazed, you emerge from the portal to find yourself standing on a desolate plain. Nearby rests a massive rune-struck altar, shrouded in moonlit fog.", 7, 308, 8, 8, 0x3e)

	if should_exit(wait_for_input()) == true then intro_exit() return end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	image_print(scroll_img, "At first the plain is still. Then a hundred voices raise a slow, deathlike song, drawing closer and closer with each passing moment. You are seized by an urge to run...", 7, 308, 8, 8, 0x3e)

	if intro_wait() == false then return end

	local l_move_tbl_x = {1, 1, 0, 1, 1, 1}
	local l_move_tbl_y = {0, 0, -1, 0, 0, -1}

	local r_move_tbl_x = {-1, -1, -1, -1, -1, -1}
	local r_move_tbl_y = {-1, -1, 0, -1, -1, -1}

	scroll.visible = false

	local i
	for i=0,95,1 do
		gargs_left.x = gargs_left.x + l_move_tbl_x[(i%6)+1]
		gargs_left.y = gargs_left.y + l_move_tbl_y[(i%6)+1]

		if i > 23 then
			gargs_right.x = gargs_right.x + r_move_tbl_x[(i%6)+1] * 2
			gargs_right.y = gargs_right.y + r_move_tbl_y[(i%6)+1] * 2
		end

		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end

			gargs_left.x = -4
			gargs_left.y = 77
			gargs_right.x = 182
			gargs_right.y = 79

			break
		end
		canvas_update()
		canvas_update()
	end

	scroll_img = image_load("blocks.shp", 0)
	scroll.image = scroll_img
	scroll.x = 0x21
	scroll.y = 0x9d
	scroll.visible = true
	image_print(scroll_img, "...but you have no place to go.", 7, 308, 35, 8, 0x3e)

	if intro_wait() == false then return end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x98
	image_print(scroll_img, "Before you can offer a protest to the creatures who surround you, scaly claws grasp your body.", 7, 308, 8, 12, 0x3e)

	if intro_wait() == false then return end

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0
	image_print(scroll_img, "With unearthly strength, the monsters bind you to the altar stone!", 7, 308, 11, 10, 0x3e)

	avatar.visible = true
	ropes.visible = true

	if intro_wait() == false then return end

	gargs_left.y = gargs_left.y + 4
	gargs_right.y = gargs_right.y + 4
	scroll.visible = false

	garg_body.visible = true
	garg_head.visible = true

	for i=0,22,1 do
		garg_body.x = garg_body.x - 3
		garg_body.y = garg_body.y - 3
		garg_head.x = garg_head.x - 3
		garg_head.y = garg_head.y - 3

		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end

			break
		end
		canvas_update()
		canvas_update()
	end

	if input == nil then
		for i=0,13,1 do
			garg_body.y = garg_body.y - 3
			garg_head.y = garg_head.y - 3

			input = input_poll()
			if input ~= nil then
				if should_exit(input) ==  true then
					intro_exit()
					return
				end

				break
			end

			canvas_update()
			canvas_update()
		end
	end

	garg_body.x = 144
	garg_body.y = 30
	garg_head.x = 222
	garg_head.y = 44

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0
	scroll.visible = true
	image_print(scroll_img, "Kneeling, the hordes sway and chant as a stately winged nightmare steps forward.", 32, 262, 33, 10, 0x3e)

	local input = nil
	local left_idx = 1
	local right_idx = 6
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		canvas_update()
		input = input_poll()
			if input ~= nil then
				if should_exit(input) ==  true then
					intro_exit()
					return
				end
				break
			end
		canvas_update()
	end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x98
	scroll.visible = true
	image_print(scroll_img, "The leader unwraps a velvet covered, brass-bound book and recites from it in a formal, stilted tongue.", 7, 308, 8, 12, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end
			break
		end
		canvas_update()
	end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	image_print(scroll_img, "Shouts and jeers explode from the masses as the priest slams shut the book. In his hand a malignant dagger drips with moonlight.", 7, 308, 8, 12, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end
			break
		end
		canvas_update()
	end

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0
	image_print(scroll_img, "You close your eyes. A dying scream, certainly your own, curdles the air.", 80, 228, 16, 10, 0x3e)

	moongate.visible = true
	g_pal_counter = 0

	for i=0x78,0x3a,-1 do
		moongate.y = i
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			moongate.y = 0x3a
			break
		end
	end

	input = nil
	while input == nil do
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	for i=0xff,0,-3 do
		moongate_rotate_palette()
		canvas_set_opacity(i)
		canvas_update()
		input_poll()
	end

	canvas_hide_all_sprites()

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x50
	scroll.visible = true
	image_print(scroll_img, "Catcalls, the dagger, a scream, Death....", 7, 308, 39, 14, 0x3e)

	fade_in()

	if intro_wait() == false then return end

	fade_out()

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	image_print(scroll_img, "Pandemonium. Shrieks of rage, of terror.", 7, 308, 34, 14, 0x3e)

	fade_in()

	if intro_wait() == false then return end

	fade_out()

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	image_print(scroll_img, "From the inevitable, an impossibility emerges.", 7, 308, 16, 14, 0x3e)

	fade_in()

	if intro_wait() == false then return end

	fade_out()

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	image_print(scroll_img, "You are still alive.", 7, 308, 101, 14, 0x3e)

	fade_in()

	if intro_wait() == false then return end

	fade_out()

	bg.visible = true
	moongate.visible = true
	gargs_left.visible = true
	gargs_right.visible = true
	garg_body.visible = true
	garg_head.visible = true
	alter.visible = true
	avatar.visible = true
	ropes.visible = true

	scroll.visible = false

	for i=0,0xff,3 do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()
		canvas_set_opacity(i)
		canvas_update()
		input_poll()
	end

	garg_head.image = intro_img_tbl[10]

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x98
	scroll.visible = true
	image_print(scroll_img, "Silent red light fills the darkness. There is the wooden clack of a crossbow, and a violet- fletched rose blooms in the priest\39s barren forehead.", 7, 308, 8, 8, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	scroll.visible = false

	for i=0,42,1 do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()

		local x =  math.random(-1, 2)
		garg_body.x = garg_body.x + x
		garg_body.y = garg_body.y + 3
		garg_head.x = garg_head.x + x
		garg_head.y = garg_head.y + 3

		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end
			garg_body.visible = false
			garg_head.visible = false
			break
		end
		canvas_update()
	end

	garg_body.visible = false
	garg_head.visible = false

	iolo.visible = true
	dupre.visible = true
	shamino.visible = true

	for i=0,82,1 do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()


		if i > 33 then
			dupre.x = dupre.x + 2
		else
			dupre.x = dupre.x + 1
		end

		dupre.y = dupre.y - 1

		if i > 13 then
			if i > 36 then
				shamino.x = shamino.x + 2
			else
				shamino.x = shamino.x + 1
			end
			shamino.y = shamino.y - 1
		end

		if i > 10 then
			iolo.x = iolo.x + 1
			if i > 14 then
				iolo.y = iolo.y - 2
			else
				iolo.y = iolo.y - 1
			end
		end

		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end
			dupre.x = 99
			dupre.y = 40
			shamino.x = 183
			shamino.y = 53
			iolo.x = 240
			iolo.y = 62
			break
		end
		canvas_update()
	end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x98
	scroll.visible = true
	image_print(scroll_img, "Friendly faces vault from a newborn moongate, while a rain of quarrels holds the furious mob at bay. The knight Dupre\39s sword flashes twice in the darkness, slicing away your bonds!", 7, 308, 8, 8, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	scroll.visible = false
	ropes.visible = false

	for i=0,82,1 do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()

		dupre.x = dupre.x - 1
		dupre.y = dupre.y + 2


		shamino.x = shamino.x - 1
		shamino.y = shamino.y + 2

		iolo.x = iolo.x - 1
		iolo.y = iolo.y + 2

		avatar.y = avatar.y + 1

		input = input_poll()
		if input ~= nil then
			if should_exit(input) ==  true then
				intro_exit()
				return
			end
			dupre.visible = false
			shamino.visible = false
			iolo.visible = false
			avatar.visible = false
			break
		end
		canvas_update()
	end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.visible = true
	image_print(scroll_img, "\"Quickly, old friend! To the gate!\127 Accompanied by the swordsman Shamino and a grinning, crossbow-wielding Iolo the Bard, Dupre thrusts a spare sword into your hand.", 7, 308, 8, 8, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	image_print(scroll_img, "Snatching the fallen priest\39s book, Iolo dives into the redness with Shamino at his heels. The howling throng surges forward, all of one terrible mind.", 7, 308, 8, 8, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	scroll_img = image_load("blocks.shp", 1)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0xa0
	image_print(scroll_img, "The gate wanes rapidly as you and Dupre charge through...", 130, 178, 12, 10, 0x3e)

	for i=0,150,1 do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		if i % 2 == 0 then
			moongate.y = moongate.y + 1
		end
		moongate_rotate_palette()
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			moongate.visible = false
			break
		end
	end

	scroll_img = image_load("blocks.shp", 0)
	scroll.image = scroll_img
	scroll.x = 0x21
	scroll.y = 0x9d
	image_print(scroll_img, "...but not rapidly enough.", 7, 308, 50, 8, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	scroll_img = image_load("blocks.shp", 2)
	scroll.image = scroll_img
	scroll.x = 0x1
	scroll.y = 0x98
	image_print(scroll_img, "From the mob\39s vanguard, three of the abominations scramble toward the gate. Driven by fury, the creatures hurl their bodies into the portal\39s last handspan of light!", 7, 308, 8, 8, 0x3e)

	input = nil
	while input == nil do
		left_idx = intro_sway_gargs(gargs_left, left_idx, false)
		right_idx = intro_sway_gargs(gargs_right, right_idx, false)
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) then
				intro_exit()
				return
			end
			break
		end
	end

	scroll.visible = false

	intro_exit()
end

g_menu_pal =
{
	{232,96,0},
	{236,128,0},
	{244,164,0},
	{248,200,0},
	{252,252,84},
	{248,200,0},
	{244,164,0},
	{236,128,0},
	{232,96,0}
}

g_menu_pal_idx = { 14, 33, 34, 35, 36 }
local function main_menu_set_pal(idx)
	local i

	for i = 1,5,1 do
		local colour = g_menu_pal[5+(i-1)-idx]
		canvas_set_palette_entry(g_menu_pal_idx[i], colour[1], colour[2], colour[3])
	end
end

local function main_menu_load()
	g_menu = {}

	if engine_should_quit() == 1 then
		return
	end

	music_play("ultima.m")

	canvas_set_palette("palettes.int", 0)

	local title_img_tbl = image_load_all("titles.shp")
	g_menu["title"] = sprite_new(title_img_tbl[0], 0x13, 0, true)
	g_menu["subtitle"] = sprite_new(title_img_tbl[1], 0x3b, 0x2f, false)

	g_menu["menu"] = sprite_new(image_load("mainmenu.shp", 0), 0x31, 0x53, false)

	fade_in()

	g_menu["subtitle"].visible = true
	g_menu["menu"].visible = true

	fade_in_sprite(g_menu["menu"])
	mouse_cursor_visible(true)
end

	g_menu_idx = 0

local function selected_intro()
	mouse_cursor_visible(false)
	main_menu_set_pal(0)
	fade_out()
	canvas_hide_all_sprites()
	intro()
	music_play("ultima.m")
	g_menu["title"].visible = true
	fade_in()
	g_menu["subtitle"].visible = true
	g_menu["menu"].visible = true

	fade_in_sprite(g_menu["menu"])
	mouse_cursor_visible(true)
end

local function selected_create_character()
	main_menu_set_pal(1)
	fade_out_sprite(g_menu["menu"],6)
	if create_character() == true then
		return true
	end
	canvas_set_palette("palettes.int", 0)
	g_menu_idx=0
	main_menu_set_pal(g_menu_idx)
	music_play("ultima.m")
	fade_in_sprite(g_menu["menu"])
	return false
end

local function selected_acknowledgments()
	main_menu_set_pal(3)
	fade_out_sprite(g_menu["menu"],6)
	acknowledgements()
	canvas_set_palette("palettes.int", 0)
	g_menu_idx=0
	main_menu_set_pal(g_menu_idx)
	fade_in_sprite(g_menu["menu"])
end

local function main_menu()
	g_menu["title"].visible = true
	g_menu["subtitle"].visible = true
	g_menu["menu"].visible = true
	canvas_set_update_interval(10)

	local input

	while true do
		canvas_update()
		input = input_poll(true)

		if engine_should_quit() == 1 then
			return "Q"
		end

		if input ~= nil then
			canvas_set_update_interval(25)

			if input == 113 then     --q quit
				return "Q"
			elseif input == 105 or input == 13 and g_menu_idx == 0 then --i
				selected_intro()
			elseif input == 99 or input == 13 and g_menu_idx == 1 then  --c
				if selected_create_character()== true then
					return "J"
				end
			elseif input == 116 or input == 13 and g_menu_idx == 2 then --t
				--transfer a character
			elseif input == 97 or input == 13 and g_menu_idx == 3 then  --a
				selected_acknowledgments()
			elseif input == 106 or input == 13 and g_menu_idx == 4 then --j
				main_menu_set_pal(4)
				fade_out()
				return "J"
			elseif input == SDLK_DOWN then --down key
				if g_menu_idx < 4 then
					g_menu_idx = g_menu_idx + 1
					main_menu_set_pal(g_menu_idx)
				end
			elseif input == SDLK_UP then --up key
				if g_menu_idx > 0 then
					g_menu_idx = g_menu_idx - 1
					main_menu_set_pal(g_menu_idx)
				end
			elseif input >= 48 and input <= 57 or input == 45 or input == 61 then --play music play pressing number keys or '-' or '='
				if input == 45 then
					input = 11
				elseif input == 48 then
					input = 10
				elseif input == 61 then
					input = 12
				else
					input = input - 48
				end

				local song_names = {
					"ultima.m",
					"bootup.m",
					"intro.m",
					"create.m",
					"forest.m",
					"hornpipe.m",
					"engage.m",
					"stones.m",
					"dungeon.m",
					"brit.m",
					"gargoyle.m",
					"end.m"
				}
				music_play(song_names[input])
			elseif input == 0 then --mouse click
				local x = get_mouse_x()
				if x > 56 and x < 264 then
					local y = get_mouse_y()
					if y > 86 and y < 108 then
						selected_intro()
					elseif y > 107 and y < 128 then
						if selected_create_character() == true then
							return "J"
						end
					elseif y > 127 and y < 149 then
						--transfer a character
					elseif y > 148 and y < 170 then
						selected_acknowledgments()
					elseif y > 169 and y < 196 then
						main_menu_set_pal(4)
						fade_out()
						return "J" -- journey onward
					end
				end
			elseif input == 1 then --mouse movement
				local x = get_mouse_x()
				if x > 56 and x < 264 then
					local old_menu_idx = g_menu_idx
					local y = get_mouse_y()
					if y > 86 and y < 108 then
						g_menu_idx = 0
					elseif y > 107 and y < 128 then
						g_menu_idx = 1
					elseif y > 127 and y < 149 then
						g_menu_idx = 2
					elseif y > 148 and y < 170 then
						g_menu_idx = 3
					elseif y > 169 and y < 196 then
						g_menu_idx = 4
					end
					if g_menu_idx ~= old_menu_idx then
						main_menu_set_pal(g_menu_idx)
					end
				end
			end

			input = nil
			canvas_set_update_interval(10)
		end
	end
end


play()
fade_out()

g_img_tbl = nil
g_clock_tbl = nil
g_lounge_tbl = nil
g_window_tbl = nil
g_stones_tbl = nil

canvas_hide_all_sprites()

main_menu_load()

if main_menu() == "Q" then -- returns "Q" for quit or "J" for Journey Onward
	config_set("config/quit", true)
end

music_stop()
canvas_hide_all_sprites()
canvas_hide()
