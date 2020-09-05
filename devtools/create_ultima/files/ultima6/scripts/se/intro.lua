local lua_file = nil

--load common functions
lua_file = nuvie_load("common/intro_common.lua"); lua_file();

-- TODO - ALL TRANSITIONS - FADE IN/OUT, STAR FADE
-- TODO - CURSORS
-- TODO - CONFIRM/CONFIGURE KEYS TO ADVANCE/BYPASS
-- TODO - MEMORY MANAGEMENT (prevent leaks)
-- TODO - CREATE NEW CHARACTER OVER EXISTING?
-- TODO - CHANGE MENU FOR NO CHARACTER YET CREATED? (new install)

local function should_exit(input)
	if input ~=nil and input == SDLK_ESCAPE then
		return true
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
	end

	return false
end

local function destroy_sprite(sprite)
-- TODO Correct Memory Management
	sprite.visible = false
--	sprite_gc(sprite)
end

local function destroy_sprites(sprites)
   local k,v
   for k,v in pairs(sprites) do
      destroy_sprite(v)
   end
	return false
end

local function opaque_sprites(sprites, number)
	for j=0,number-1,1 do
		-- TODO Opaque = 0x100 or 0xff???
		sprites[j+1].opacity = 0x100
		canvas_update()
	end
	return false
end

local function fade_out_sprites(sprites, speed, number)
	local i
	if speed ~= nil then
		speed = -speed
	else
		speed = -3
	end

	for i=0xff,0,speed do
		for j=0,number-1,1 do
			sprites[j+1].opacity = i
			canvas_update()
		end
	end

	-- Fully Transparent
	for j=0,number-1,1 do
		sprites[j+1].opacity = 0
		canvas_update()
	end

	return false
end

local function fade_in_sprites(sprites, speed, number)
	local i
	if speed ~= nil then
		speed = speed
	else
		speed = 3
	end

	for i=0,0xff,speed do
		for j=0,number-1,1 do
			sprites[j+1].opacity = i
			canvas_update()
		end
	end

	-- Fully Opaque
	for j=0,number-1,1 do
		-- TODO Opaque = 0x100 or 0xff???
		sprites[j+1].opacity = 0x100
		canvas_update()
	end

	return false
end

local function origin_fx_sequence()
	local g_img_tbl = image_load_all("title.lzc")

	canvas_set_palette("savage.pal", 2)


	local stars = sprite_new(g_img_tbl[0][0], 0, 24, true)
	local logo_image = image_new(282,82)
	image_blit(logo_image, g_img_tbl[0][1],0,16)
	image_blit(logo_image, g_img_tbl[0][2],g_img_tbl[0][1].w,14)
	image_blit(logo_image, g_img_tbl[0][3],g_img_tbl[0][1].w+g_img_tbl[0][2].w,0)

	local  logo = sprite_new(logo_image, 20, 70, false)

	local planet = sprite_new(g_img_tbl[12], 160, 48, true)
	planet.clip_x = 0
	planet.clip_y = 0
	planet.clip_w = 320
	planet.clip_h = 152
	local players = {}
	players[1] = create_player_sprite(g_img_tbl[1][0], 58, 118)
	players[2] = create_player_sprite(g_img_tbl[2][0], 186, 118)
	players[3] = create_player_sprite(g_img_tbl[3][0], 278, 118)

	players[4] = create_player_sprite(g_img_tbl[4][0], 58, 126)
	players[5] = create_player_sprite(g_img_tbl[5][0], 186, 126)
	players[6] = create_player_sprite(g_img_tbl[6][0], 278, 126)

	players[7] = create_player_sprite(g_img_tbl[7][0], 58, 134)
	players[8] = create_player_sprite(g_img_tbl[8][0], 186, 134)
	players[9] = create_player_sprite(g_img_tbl[9][0], 278, 134)

	local conductor = sprite_new(g_img_tbl[10][0], 158, 98, true)
	conductor.clip_x = 0
	conductor.clip_y = 24
	conductor.clip_w = 320
	conductor.clip_h = 128

	fade_in()

	music_play("music.lzc", 19)

	local i = 0
	for i=0,6,1 do
		conductor.image = g_img_tbl[10][i]

		update_players(players, g_img_tbl)
		if poll_for_key_or_button(4) == true then return end
	end
	for i=7,13,1 do
		conductor.image = g_img_tbl[10][i]

		update_players(players, g_img_tbl)
		if poll_for_key_or_button(4) == true then return end
	end
	for i=7,12,1 do
		conductor.image = g_img_tbl[10][i]

		update_players(players, g_img_tbl)
		if poll_for_key_or_button(4) == true then return end
	end
	local j
	for i=1,4,1 do
		for j=13,15,1 do
			conductor.image = g_img_tbl[10][j]
			if poll_for_key_or_button(1) == true then return end
		end

		conductor.image = g_img_tbl[10][14]
		if poll_for_key_or_button(2) == true then return end
		conductor.image = g_img_tbl[10][13]
		if poll_for_key_or_button(2) == true then return end
		conductor.image = g_img_tbl[10][16]

		if poll_for_key_or_button(1) == true then return end
			play_sfx(38, false)
	end

	for i=16,20,1 do
		conductor.image = g_img_tbl[10][i]
		if poll_for_key_or_button(4) == true then return end
	end
	if poll_for_key_or_button(200) == true then return end

	play_sfx(12, false)

	conductor.image = g_img_tbl[10][6]

	for i=1,21,1 do
		conductor.y = 98 + i * 12
		conductor.image.scale = 100 + i * 15

		for j=1,9,1 do
			players[j].y = players[j].y + 5
			players[j].image.scale = 100 + i * 5
			if j == 1 or j == 4 or j == 7 then
				players[j].x = players[j].x - 2
			end
			if j == 3 or j == 6 or j == 9 then
				players[j].x = players[j].x + 2
			end
		end

		if poll_for_esc(4) == true then return end
	end


	logo.visible = true
	logo.image.scale = 10


	for i=1,18,1 do
		planet.y = planet.y + 6

		logo.image.scale = logo.image.scale + 5
		logo.x = math.floor((320 - logo.image.w) / 2)
		if i < 10 then
			logo.y = logo.y - 4
		else
			logo.y = logo.y + 1
		end

		if poll_for_key_or_button(4) == true then return end
	end

	fireworks(g_img_tbl, logo)
end

local function intro_sequence(g_img_tbl)

	canvas_set_palette("savage.pal", 0)
	music_play("music.lzc", 18)

	local  logo = sprite_new(g_img_tbl[0][0], 0, 0, true)
	fade_in()
	if poll_for_key_or_button(175) == true then return end
	fade_out()
	logo.image = g_img_tbl[0][1]
	fade_in()
	if poll_for_key_or_button(175) == true then return end
	fade_out()

	canvas_set_palette("savage.pal", 0)
	local  bg = sprite_new(g_img_tbl[1][0], 0, 0, true) -- Background 0
	local  bg1 = sprite_new(g_img_tbl[1][1], 320, 0, true) -- Final Section
	local  bg2 = sprite_new(g_img_tbl[1][2], 0, 0, true) -- Background 1
	local  bg3 = sprite_new(g_img_tbl[1][3], 320, 0, true) -- Bushes w/ Triceratops
	local  bg4 = sprite_new(g_img_tbl[1][4], 640, 0, true) -- Left section of Tower
	local  t1 = sprite_new(g_img_tbl[2][0], 0, 240, true)
	local  bg5 = sprite_new(g_img_tbl[1][5], 0, 0, true) -- Dinosaurs & Bushes
	local  bg6 = sprite_new(g_img_tbl[1][6], 320, 0, true) -- Tree w/ Sleeping Dinosaur
	local  bg7 = sprite_new(g_img_tbl[1][7], 640, 0, true) -- Bushes on sides w/ flower on left
	canvas_set_opacity(0xff)
	i = 0
	while i < 240 do
	      canvas_update()
	      t1.y = t1.y - 3
	      if (t1.y < 0) then t1.y = 0 end
	      i = i + 3
	      if poll_for_key_or_button(1) == true then return end
	end
	if poll_for_key_or_button(30) == true then return end
	fade_out_sprite(t1, 4)
	i = 0
	current_credit = 1
	local  credit_sprite = sprite_new(g_img_tbl[2][current_credit], 0, 0, true)
	credit_time = 15
	credit_show = 1
	while i < 636 do
	      canvas_update()
	      bg.x = bg.x - 1
	      bg1.x = bg1.x - 1
	      bg2.x = bg2.x - 2
	      bg3.x = bg3.x - 2
	      bg4.x = bg4.x - 2
	      bg5.x = bg5.x - 3
	      bg6.x = bg6.x - 3
	      bg7.x = bg7.x - 3
	      i = i + 3
	      credit_time = credit_time - 1
	      if (credit_time == 0) then
	      	 if (credit_show == 1) then
		    credit_show = 0
		    credit_sprite.visible = false
		    credit_time = 20
		 else
		    credit_show = 1
		    current_credit = current_credit + 1
		    if (current_credit < 7) then
		       credit_sprite = sprite_new(g_img_tbl[2][current_credit], 0, 0, true)
		       credit_time = 15
		    end
		 end
	      end
	      if poll_for_key_or_button(8) == true then return end
	end
end

local story_so_far_text = {
"You had disturbing dreams of a faraway jungle,",
"an engangered princess, and the black",
"moonstone you carried away from Britannia in",
"your last adventure there.",
"The spirit of Lord British commanded you to",
"find out all you could about the stone.",
"",
"",
"So you took it to your old friend Dr. Rafkin,",
"curator of the local museum of natural",
"history, hoping that he could unravel the",
"mystery.",
"At Dr. Rafkin's, you met ace reporter and",
"\"Ultimate Adventures\" correspondent Jimmy",
"Malone.  Malone's a little too nosy to suit you...",
"",
"...but he kept his wits when everything went",
"haywire, when Dr. Rafkin's experiments on your",
"moonstone sent the whole museum lab through",
"a bizarre black moongate!",
"You were shocked to lift your head and see",
"jungle around you: Jungle, and this enormous",
"creature descending... descending upon--",
"",
"Her! The woman you dreamt of! A fiery tribal",
"she-warrior of some time-lost land... You and",
"your friends rescued her from the giant",
"pteranodon and befriended her.",
"She said her name was Aiela, princess of the",
"Kurak tribe, and seemed to be quite taken with",
"you, her most unusual rescuer.",
"",
"But Aiela's hated suitor, Darden the Huge,",
"prince of the Urali tribe, arrived with his men,",
"scattering your friends and smashing you into",
"the earth.",
"Now you lie broken and defeated, struggling to",
"regain consciousness...",
"",
""
}

local function story_so_far(img_tbl2)
	local prev_f1 = nil
	local prev_f2 = nil
	canvas_set_palette("savage.pal", 1)
	for i = 0, 9, 1 do
		-- Need to figure out if I need to get a new version
		-- of the Top level sprite or not
		local new_img = image_copy(img_tbl2[5][0])
		local  f1 = sprite_new(new_img, 0, 0, true)
		local  f2 = sprite_new(img_tbl2[5][i+1], 120, 45, true)
		if prev_f1 ~= nil then
			destroy_sprite(prev_f1)
			prev_f1 = nil
		end
		if prev_f2 ~= nil then
			destroy_sprite(prev_f2)
			prev_f2 = nil
		end
		prev_f1 = f1;
		prev_f2 = f2;
		image_print(new_img, story_so_far_text[i*4+1], 0, 300, 25, 136, 0)
		image_print(new_img, story_so_far_text[i*4+2], 0, 300, 25, 144, 0)
		image_print(new_img, story_so_far_text[i*4+3], 0, 300, 25, 152, 0)
		image_print(new_img, story_so_far_text[i*4+4], 0, 300, 25, 160, 0)
		wait_for_input()
	end
	if prev_f1 ~= nil then
		destroy_sprite(prev_f1)
	end
	if prev_f2 ~= nil then
		destroy_sprite(prev_f2)
	end
	canvas_set_palette("savage.pal", 0)
end

local function shaman_drop_anim(sprites, color)
	-- TODO - TIMING
	-- Arm Anim 4-9
	-- Drop Anim 10-18
	local prev_shaman = nil
	for i = 4,9,1 do
		sprites[i].visible = true
		if prev_shaman ~= nil then
			prev_shaman.visible = false
		end
		prev_shaman = sprites[i]
		canvas_update()
	end
	local prev_drop = nil
	for i = 10,26,1 do
		sprites[27+color].visible = true
		sprites[i].visible = true
		if prev_drop ~= nil then
			prev_drop.visible = false
		end
		prev_drop = sprites[i]
		canvas_update()
	end
	sprites[27+color].visible = false
	prev_drop.visible = false
	for i = 8,4,-1 do
		sprites[i].visible = true
		if prev_shaman ~= nil then
			prev_shaman.visible = false
		end
		prev_shaman = sprites[i]
		canvas_update()
	end
	prev_shaman.visible = false
	canvas_update()
end

-- Organization of questions (Red, Yellow, Blue)
-- Int (Red) vs Str (Yellow), Int (Red) vs Dex (Blue), Str (Yellow) vs. Dex (Blue)
-- 4 Lines for Each Question (nil on final question means no first screen)
-- Two sets of questions for each type, reversing answer order (Red vs. Yellow, then Yellow vs. Red)
local create_questions_text = {
"Your chief has told you to guard the village while a battle rages nearby. Your chief fights in the battle",
"Will you (a) obey the word of your chief, or (b) join your fellow warriors in the fight?",
nil,
nil,
"You fight a warrior you hate, and knock his spear from his hands.",
"Another blow and he will be dead.",
"Will you (a) let him surrender, and spare him if he does, or (b) slay him where he stands?",
nil,
"One warrior borrows another's spear and fails to return it. Days later, he mislays his own spear and you find it.",
"Do you (a) return it to him, or (b) give it to the warrior who is owed the spear?",
nil,
nil,
"You have sworn to protect your chief at any cost. Yet you see him foully murder a warrior of his own tribe.",
"The murdered man's brother asks you what you saw.",
"Will you (a) keep your oath and protect your king, or (b) tell the truth of the matter?",
nil,
"A huge, powerful warrior stands against you and demands you give him your food.",
"Will you (a) throw his demand in his teeth and attack him, or (b) give him your food, since it is clear he is hungry?",
nil,
nil,
"Your chief gives you a pouch of gems to take to another; he has not counted them.",
"On the path to the other village, you find a lamed warrior who cannot hunt.",
"He could trade you a few of your gems for food, and they will not be missed.",
"Will you (a) deliver the gems as you were charged, or (b) give the warrior a gem?",
}

local create_questions_A_tbl = {
2,
1,
1,
0,
0,
2
}

local create_questions_B_tbl = {
0,
0,
2,
2,
1,
1
}

local function process_answer(q_num, input, sprites)
	local stat_type
	if input == SDLK_a then
		stat_type = create_questions_A_tbl[q_num]
	else
		stat_type = create_questions_B_tbl[q_num]
	end

	g_stats[stat_type] = g_stats[stat_type] + 3 + math.random(0, 2)

	local color = 0 -- stat_type == 2
	if stat_type == 0 then color = 1 end
	if stat_type == 1 then color = 2 end

	shaman_drop_anim(sprites, color)
	return stat_type
end

local function ask_question(q_num, q_ord, sprites, text_image, a_b_border_img, images, text_width)
	-- Do I need to display two pages?
	local q_index = q_num -- * 2 + q_ord
	-- Do I need to display two pages?
	local two_questions = false
	if create_questions_text[q_index*4+3+1] ~= nil then two_questions = true end
	local txt_ind = 1
	local x,y
	if (two_questions) then
		image_blit(text_image, images[4][2], 0, 0)
		x, y = image_print(text_image, create_questions_text[q_index*4+txt_ind], 8, text_width, 8, 10, 0x00)
		x, y = image_print(text_image, create_questions_text[q_index*4+txt_ind+1], 8, text_width, 8, y+20, 0x00)
		canvas_update()
		wait_for_input()
		txt_ind = 3
	end
	image_blit(text_image, images[4][2], 0, 0)
	x, y = image_print(text_image, create_questions_text[q_index*4+txt_ind], 8, text_width, 8, 10, 0x00)
	x, y = image_print(text_image, create_questions_text[q_index*4+txt_ind+1], 8, text_width, 8, y+20, 0x00)
	if txt_ind ~= 3 then
		if create_questions_text[q_index*4+txt_ind+2] ~= nil then
			x, y = image_print(text_image, create_questions_text[q_index*4+txt_ind+2], 8, text_width, 8, y+20, 0x00)
		end
	end

	local red = 0x04
	local black = 0x00
	local answer = SDLK_a
	local old_answer = 0
	local stat_type = -1
	local need_to_process_answer = false
	local a_sprite = sprite_new(nil, 56 + 170, 129 + 45, true)
	local b_sprite = sprite_new(nil, 72 + 170, 129 + 45, true)
	a_sprite.text = "a"
	b_sprite.text = "b"
	a_sprite.text_color = red
	b_sprite.text_color = black

	while stat_type == -1 do
		canvas_update()
		input = input_poll(true)
		if input ~= nil then
			if input == SDLK_a or input == SDLK_b then
				answer = input
				need_to_process_answer = true
			elseif input == SDLK_LEFT or input == SDLK_KP4 then
				answer = SDLK_a
			elseif input == SDLK_RIGHT or input == SDLK_KP6 then
				answer = SDLK_b
			elseif input == SDLK_KP_ENTER or input == SDLK_RETURN or input == SDLK_SPACE then
				need_to_process_answer = true
			elseif input == MOUSE_CLICK or input == MOUSE_MOTION then
				local mouse_y = get_mouse_y()
				if mouse_y >= b_sprite.y and mouse_y <= b_sprite.y + 7 then
					local mouse_x = get_mouse_x()
					if mouse_x > a_sprite.x and mouse_x < a_sprite.x + 7 then
						answer = SDLK_a
						if input == MOUSE_CLICK then
							need_to_process_answer = true
						end
					elseif mouse_x > b_sprite.x and mouse_x < b_sprite.x + 6 then
						answer = SDLK_b
						if input == MOUSE_CLICK then
							need_to_process_answer = true
						end
					end
				end
			end
			if answer ~= old_answer then
				old_answer = answer
				if answer == SDLK_a then
					a_sprite.text_color = red
					b_sprite.text_color = black
				else
					a_sprite.text_color = black
					b_sprite.text_color = red
				end
			end
			if need_to_process_answer then
				stat_type = process_answer(q_index+1, answer, sprites)
			end
		end
	end

	destroy_sprite(a_sprite)
	destroy_sprite(b_sprite)

	local next_q
	if stat_type == 0 then
		if q_ord == 0 then
			next_q = 2
		elseif q_num == 0 then
			next_q = 4
		else
			next_q = 3
		end
	elseif stat_type == 1 then
		if q_ord == 0 then
			next_q = 0
		elseif q_num == 2 then
			next_q = 4
		else
			next_q = 5
		end
	elseif stat_type == 2 then
		if q_ord == 0 then
			next_q = 1
		elseif q_num == 0 then
			next_q = 5
		else
			next_q = 3
		end
	end

	return next_q
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

	g_stats = {}	-- Int/Str/Dex

local function create_new_character(img_tbl2)
	local input
	canvas_set_palette("savage.pal", 1)
	local create_char_sprites = {}
	for i=0,2,1 do
		g_stats[i] = 16 + math.random(0,2)
	end
	create_char_sprites[1] = sprite_new(img_tbl2[1][2], 0, 0, true) -- Full Screen Border Around Shaman
	create_char_sprites[2] = sprite_new(img_tbl2[1][0], 8, 28, true) -- Create Character Shaman
	create_char_sprites[4] = sprite_new(img_tbl2[2][0], 8, 28, false) -- Shaman Arm Anim1
	create_char_sprites[5] = sprite_new(img_tbl2[2][1], 8, 28, false) -- Shaman Arm Anim2
	create_char_sprites[6] = sprite_new(img_tbl2[2][2], 8, 28, false) -- Shaman Arm Anim3
	create_char_sprites[7] = sprite_new(img_tbl2[2][3], 8, 28, false) -- Shaman Arm Anim4
	create_char_sprites[8] = sprite_new(img_tbl2[2][4], 8, 28, false) -- Shaman Arm Anim5
	create_char_sprites[9] = sprite_new(img_tbl2[2][5], 8, 28, false) -- Shaman Arm Anim6
	create_char_sprites[27] = sprite_new(img_tbl2[3][17], 8, 28, false) -- Red
	create_char_sprites[28] = sprite_new(img_tbl2[3][18], 8, 28, false) -- Yellow
	create_char_sprites[29] = sprite_new(img_tbl2[3][19], 8, 28, false) -- Blue
	create_char_sprites[10] = sprite_new(img_tbl2[3][0], 8, 28, false) -- Drop Anim
	create_char_sprites[11] = sprite_new(img_tbl2[3][1], 8, 28, false) --
	create_char_sprites[12] = sprite_new(img_tbl2[3][2], 8, 28, false) --
	create_char_sprites[13] = sprite_new(img_tbl2[3][3], 8, 28, false) --
	create_char_sprites[14] = sprite_new(img_tbl2[3][4], 8, 28, false) --
	create_char_sprites[15] = sprite_new(img_tbl2[3][5], 8, 28, false) --
	create_char_sprites[16] = sprite_new(img_tbl2[3][6], 8, 28, false) --
	create_char_sprites[17] = sprite_new(img_tbl2[3][7], 8, 28, false) --
	create_char_sprites[18] = sprite_new(img_tbl2[3][8], 8, 28, false) --
	create_char_sprites[19] = sprite_new(img_tbl2[3][9], 8, 28, false) --
	create_char_sprites[20] = sprite_new(img_tbl2[3][10], 8, 28, false) --
	create_char_sprites[21] = sprite_new(img_tbl2[3][11], 8, 28, false) --
	create_char_sprites[22] = sprite_new(img_tbl2[3][12], 8, 28, false) --
	create_char_sprites[23] = sprite_new(img_tbl2[3][13], 8, 28, false) --
	create_char_sprites[24] = sprite_new(img_tbl2[3][14], 8, 28, false) --
	create_char_sprites[25] = sprite_new(img_tbl2[3][15], 8, 28, false) --
	create_char_sprites[26] = sprite_new(img_tbl2[3][16], 8, 28, false) --
	create_char_sprites[3] = sprite_new(img_tbl2[1][1], 8, 28, true) -- Weird Border Around Shaman
	-- TODO BORDER SHADOW (Solid Black)
	-- TODO Memory Management on Image?
	-- Intro Screen1
	local scroll_img = image_copy(img_tbl2[4][2])
	local a_b_border = image_copy(img_tbl2[4][2])
	local text_width = 130
	create_char_sprites[32] = sprite_new(a_b_border, 170, 45, false) -- Border for a and b answers (overlapped by next border)
	create_char_sprites[30] = sprite_new(scroll_img, 170, 25, true) -- Border
	local x, y = image_print(scroll_img, "You are in a dirt-floored hut... and a tribesman with wise eyes stares down at you.", 8, text_width, 8, 10, 0x00)
	x, y = image_print(scroll_img, "You feel as though you are floating. You cannot move your limbs. It is like a dream... yet you sense it is not a dream.", 8, text_width, 8, y+20, 0x00)
	canvas_update()
	wait_for_input()
	-- Intro Screen2
	image_blit(scroll_img, img_tbl2[4][2], 0, 0)
	local x, y = image_print(scroll_img, "The man speaks: \"Yes, warrior, you can hear. You see Intanya, shaman and healer. Intanya will heal you... but to concoct his healing potion, he must ask you questions.\"", 8, text_width, 8, 10, 0x00)
	canvas_update()
	wait_for_input()
	-- Intro Screen3
	image_blit(scroll_img, img_tbl2[4][2], 0, 0)
	x, y = image_print(scroll_img, "\"In order to heal your spirit, Intanya must know your spirit, so you must answer with truthful words.\"", 8, text_width, 8, 10, 0x00)
	x, y = image_print(scroll_img, "\"Intanya will speak of deeds and choices; you must answer with the choices you would make.\"", 8, text_width, 8, y+20, 0x00)
	canvas_update()
	wait_for_input()
	-- Name Input
	image_blit(scroll_img, img_tbl2[4][2], 0, 0)
	x, y = image_print(scroll_img, "\"First, though, Intanya must know the warrior's name, for there is much power in names.", 8, text_width, 8, 10, 0x00)
	x, y = image_print(scroll_img, "\"What does the warrior call himself?\"", 8, text_width, 8, y+20, 0x00)
	canvas_update()
	local name = sprite_new(nil, 178, 25+y+10, true)
	create_char_sprites[31] = name
	local num_sprites = 32
	name.text = ""
	name.text_color = 0x00
	local char_index = 0
	while input == nil do
		canvas_update()
		input = input_poll()
		if input ~= nil then
			if should_exit(input) == true then
				destroy_sprites(create_char_sprites)
				canvas_set_palette("savage.pal", 0)
				return false -- back to main menu
			end
			local name_text = name.text
			local len = string.len(name_text)
			if (input == SDLK_BACKSPACE or input == SDLK_LEFT) and len > 0 then
				name.text = string.sub(name_text, 1, len - 1)
				if len == 1 then -- old len
					char_index = 0
				else
					char_index = string.byte(name_text, len -1)
				end
			elseif (input == SDLK_RETURN or input == SDLK_KP_ENTER) and len > 0 then --return
				break;
			elseif g_keycode_tbl[input] ~= nil and len < 13 then
				char_index = input
				name.text = name_text..g_keycode_tbl[input]
			elseif input == SDLK_UP then --up
				if char_index == 0 then
					if len > 0 then
						char_index = SDLK_a
					else
						char_index = 65 --A
					end
				elseif char_index == 32 then --gap in characters
					char_index = 39
				elseif char_index == 39 then --gap in characters
					char_index = 44
				elseif char_index == 46 then --gap in characters
					char_index = 48
				elseif char_index == 57 then --gap in characters
					char_index = 65
				elseif char_index == 90 then --gap in characters
					char_index = 97
				elseif char_index == 122 then --last char
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
				elseif char_index == 39 then --gap in characters
					char_index = 32
				elseif char_index == 44 then --gap in characters
					char_index = 39
				elseif char_index == 48 then --gap in characters
					char_index = 46
				elseif char_index == 65 then --gap in characters
					char_index = 57
				elseif char_index == 97 then --gap in characters
					char_index = 90
				elseif char_index == 32 then --first char
					char_index = 122
				else
					char_index = char_index - 1
				end

				if len > 0 then -- erase char
					name_text = string.sub(name_text, 1, len - 1)
				end
				name.text = name_text..g_keycode_tbl[char_index]
			elseif input == SDLK_RIGHT and len < 13 then --right
				char_index = SDLK_a --a
				name.text = name_text.."a"
			end
			input = nil
		end
	end

--	name.x = 0x10 + (284 - canvas_string_length(name.text)) / 2
	name.visible = false

	-- Questions
	local next_q = math.random(0,2)

	mouse_cursor_visible(true)
	create_char_sprites[32].visible = true -- a b border
	-- Answers are 0-Red, 1-Yellow, 2-Blue (order of sprites in file)
	next_q = ask_question(next_q, 0, create_char_sprites, scroll_img, a_b_border, img_tbl2, text_width)
	next_q = ask_question(next_q, 1, create_char_sprites, scroll_img, a_b_border, img_tbl2, text_width)
	ask_question(next_q, 2, create_char_sprites, scroll_img, a_b_border, img_tbl2, text_width)

	config_set("config/newgame", true)
	config_set("config/newgamedata/name", name.text)
	config_set("config/newgamedata/str", g_stats[0])
	config_set("config/newgamedata/dex", g_stats[1])
	config_set("config/newgamedata/int", g_stats[2])

	--io.stderr:write("str "..g_stats[0].." dex "..g_stats[1].." int "..g_stats[2].."\n")

	destroy_sprites(create_char_sprites)
	canvas_set_palette("savage.pal", 0)
	return true -- Journey onward
end

-- Could/should get heights from sprite, but was getting error when trying
local credit_heights = {
      182, 157, 100, 188, 136, 193, 183, 176, 97, 190, 190, 50
}

function create_about_sprite(image, x, y)
   local sprite = sprite_new(image, x, y, true)
   sprite.clip_x = 0
   sprite.clip_y = 100
   sprite.clip_w = 320
   sprite.clip_h = 100
   return sprite
end

local function about_the_savage_empire(img_tbl2)
	local ypos = 200
	local about_sprites = {}

	local speed = 1
	local i
	for i=0,11 do
	  table.insert(about_sprites, create_about_sprite(img_tbl2[6][i],   100, ypos))
	  ypos = ypos + credit_heights[i+1]
	end

	local done = 0

   local k,v
	while done < ypos do

		for k,v in pairs(about_sprites) do
		 v.y = v.y - speed
		end

		done = done + speed
		if poll_for_key_or_button(1) == true then
			break
		end
	end

	destroy_sprites(about_sprites)
end

	g_menu_idx = 0
	old_g_menu_idx = 0
	g_menu1 = nil -- The Story So far...
	g_menu2 = nil -- Create New Character
	g_menu3 = nil -- About the Savage Empire
	g_menu4 = nil -- Journey Onward
	g_m1_focus = nil -- The Story So far...
	g_m2_focus = nil -- Create New Character
	g_m3_focus = nil -- About the Savage Empire
	g_m4_focus = nil -- Journey Onward
	g_m1_highlight = nil -- The Story So far...
	g_m2_highlight = nil -- Create New Character
	g_m3_highlight = nil -- About the Savage Empire
	g_m4_highlight = nil -- Journey Onward
	g_menu_sprites = {}

local function selected_story_so_far(img_tbl2)
	mouse_cursor_visible(false)
	g_menu1.visible = false
	g_m1_focus.visible = false
	g_m1_highlight.visible = true
	canvas_update()
	story_so_far(img_tbl2)
	g_menu1.visible = true
	g_m1_focus.visible = true
	g_m1_highlight.visible = false
	mouse_cursor_visible(true)
end

local function selected_create_character(img_tbl2)
	mouse_cursor_visible(false)
	local start_new_game = create_new_character(img_tbl2)
	mouse_cursor_visible(true)
	return start_new_game
end

local function selected_about_the_savage_empire(img_tbl2)
	mouse_cursor_visible(false)
	g_menu3.visible = false
	g_m3_focus.visible = false
	g_m3_highlight.visible = true
	canvas_update()
	fade_out_sprites(g_menu_sprites, 50, 12)
	about_the_savage_empire(img_tbl2)
	g_menu3.visible = true
	g_m3_focus.visible = true
	g_m3_highlight.visible = false
	fade_in_sprites(g_menu_sprites, 50, 12)
	mouse_cursor_visible(true)
end

local function journey_onward()
	mouse_cursor_visible(false)
	g_menu4.visible = false
	g_m4_focus.visible = false
	g_m4_highlight.visible = true
	canvas_update()
end

local function initialize_main_g_menu_sprites(img_tbl2)
	g_menu1 = sprite_new(img_tbl2[0][5], 85, 117, true) -- The Story So far...
	g_menu2 = sprite_new(img_tbl2[0][1], 85, 135, true) -- Create New Character
	g_menu3 = sprite_new(img_tbl2[0][9], 85, 153, true) -- About the Savage Empire
	g_menu4 = sprite_new(img_tbl2[0][3], 85, 171, true) -- Journey Onward
	g_m1_focus = sprite_new(img_tbl2[0][6], 85, 117, true) -- The Story So far...
	g_m2_focus = sprite_new(img_tbl2[0][2], 85, 135, false) -- Create New Character
	g_m3_focus = sprite_new(img_tbl2[0][10], 85, 153, false) -- About the Savage Empire
	g_m4_focus = sprite_new(img_tbl2[0][4], 85, 171, false) -- Journey Onward
	g_m1_highlight = sprite_new(img_tbl2[0][13], 85, 117, false) -- The Story So far...
	g_m2_highlight = sprite_new(img_tbl2[0][11], 85, 135, false) -- Create New Character
	g_m3_highlight = sprite_new(img_tbl2[0][15], 85, 153, false) -- About the Savage Empire
	g_m4_highlight = sprite_new(img_tbl2[0][12], 85, 171, false) -- Journey Onward
	g_menu_sprites[1] = g_menu1
	g_menu_sprites[2] = g_menu2
	g_menu_sprites[3] = g_menu3
	g_menu_sprites[4] = g_menu4
	g_menu_sprites[5] = g_m1_highlight
	g_menu_sprites[6] = g_m2_highlight
	g_menu_sprites[7] = g_m3_highlight
	g_menu_sprites[8] = g_m4_highlight
	g_menu_sprites[9] = g_m1_focus
	g_menu_sprites[10] = g_m2_focus
	g_menu_sprites[11] = g_m3_focus
	g_menu_sprites[12] = g_m4_focus

	g_m2_focus.visible = false
	g_m3_focus.visible = false
	g_m4_focus.visible = false
end

local function set_main_menu_highlight()
	if g_menu_idx == old_g_menu_idx then -- no change
		return
	end
	g_menu_sprites[g_menu_idx + 9].visible = true
	g_menu_sprites[old_g_menu_idx + 9].visible = false
	old_g_menu_idx = g_menu_idx
end

local function main_menu(img_tbl2)
	local input
	local seTitle = sprite_new(img_tbl2[0][0], 0, 0, true)
	mouse_cursor_visible(true)
	initialize_main_g_menu_sprites(img_tbl2)
   canvas_set_update_interval(10)

	while true do
		canvas_update()
		input = input_poll(true)

		if engine_should_quit() == 1 then
			return "Q"
		end

		if input ~= nil then
			canvas_set_update_interval(25)

			if input == SDLK_q then -- q
				return "Q"
			elseif input == SDLK_RETURN or input == SDLK_SPACE or input == KP_ENTER then -- space or return
				if g_menu_idx == 0 then -- story so far
					selected_story_so_far(img_tbl2)
				elseif g_menu_idx == 1 then -- create char
					if selected_create_character(img_tbl2) == true then
						return "J" -- starting new game
					end
				elseif g_menu_idx == 2 then -- about SE
					selected_about_the_savage_empire(img_tbl2)
				elseif g_menu_idx == 3 then -- journey onward
					journey_onward()
					return "J"
				end
			elseif input == SDLK_s or input == SDLK_t then -- s or t (story so far)
				g_menu_idx = 0
				set_main_menu_highlight()
				selected_story_so_far(img_tbl2)
			elseif input == SDLK_c then -- c (create char)
				g_menu_idx = 1
				set_main_menu_highlight()
				if selected_create_character(img_tbl2) == true then
					return "J" -- starting new game
				end
			elseif input == SDLK_a then -- a (about SE)
				g_menu_idx = 2
				set_main_menu_highlight()
				selected_about_the_savage_empire(img_tbl2)
			elseif input == SDLK_j then -- j (journey onward)
				g_menu_idx = 3
				set_main_menu_highlight()
				journey_onward()
				return "J"
			elseif input == SDLK_DOWN or input == SDL_KP2 then -- down key
				g_menu_idx = g_menu_idx + 1
				if (g_menu_idx == 4) then g_menu_idx = 0 end
				set_main_menu_highlight()
			elseif input == SDLK_UP or input == SDL_KP8 then -- up key
				g_menu_idx = g_menu_idx - 1
				if (g_menu_idx == -1) then g_menu_idx = 3 end
				set_main_menu_highlight()
			elseif input == MOUSE_CLICK then --mouse click
				local x = get_mouse_x()
				if x > 84 and x < 242 then
					local y = get_mouse_y()
					if y > 118 and y < 134 then -- story so far
						selected_story_so_far(img_tbl2)
					elseif y > 133 and y < 152 then -- create new char
						if selected_create_character(img_tbl2) == true then
							return "J" -- starting new game
						end
					elseif y > 151 and y < 170 then -- about SE
						selected_about_the_savage_empire(img_tbl2)
					elseif y > 169 and y < 185 then -- journey onward
						journey_onward()
						return "J"
					end
				end
			elseif input == MOUSE_MOTION then --mouse movement
				local x = get_mouse_x()
				if x > 84 and x < 242 then
					local y = get_mouse_y()
					if y > 118 and y < 134 then -- story so far
						g_menu_idx = 0
					elseif y > 133 and y < 152 then -- create new char
						g_menu_idx = 1
					elseif y > 151 and y < 170 then -- about SE
						g_menu_idx = 2
					elseif y > 169 and y < 185 then -- journey onward
						g_menu_idx = 3
					end
					set_main_menu_highlight()
				end
			end

			canvas_set_update_interval(10)
		end
	end
--[[		This code should never execute because there is no break in the loop
	wait_for_input()
	g_menu1.visible = false
	g_menu2.visible = false
	g_menu3.visible = false
	g_menu4.visible = false]]--
--	canvas_set_palette("savage.pal", 1)
--	local  e4 = sprite_new(img_tbl2[4][3], 0, 0, true) -- Small Head/Eyes
--	local  e5 = sprite_new(img_tbl2[4][4], 50, 0, true) -- Truth
--	local  e6 = sprite_new(img_tbl2[4][5], 100, 0, true) -- Small Head/Eyes
--	local  e7 = sprite_new(img_tbl2[4][6], 150, 0, true) -- Love
--	local  e8 = sprite_new(img_tbl2[4][7], 100, 0, true) -- Small Head/Eyes
--	local  e9 = sprite_new(img_tbl2[4][8], 100, 0, true) -- Full Top
--	local  e10 = sprite_new(img_tbl2[4][9], 100, 0, true) -- Full Top2
--	local  h1 = sprite_new(img_tbl2[7][0], 0, 0, true) -- Small Top1 (Correct Palette?)
--	local  h2 = sprite_new(img_tbl2[7][1], 50, 50, true) -- Small Top2
end

mouse_cursor_visible(false)
canvas_set_update_interval(25)
canvas_set_bg_color(0)
canvas_set_opacity(0)

origin_fx_sequence()

--canvas_hide_all_sprites()

-- Load Graphics for Intro & Main Menu
local g_img_tbl = image_load_all("intro.lzc")
local img_tbl2 = image_load_all("create.lzc")
intro_sequence(g_img_tbl)

if main_menu(img_tbl2) == "Q" then -- returns "Q" for quit or "J" for Journey Onward
   fade_out(6)
	config_set("config/quit", true)
end

music_stop()
canvas_hide_all_sprites()
canvas_hide()
