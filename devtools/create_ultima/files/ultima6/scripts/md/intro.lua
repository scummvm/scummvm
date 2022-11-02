local lua_file = nil

--load common functions
lua_file = nuvie_load("common/intro_common.lua"); lua_file();

local FREUD_STATE_STARING  = 0
local FREUD_STATE_WRITING  = 1

local function origin_fx_sequence()
   local g_img_tbl = image_load_all("title.lzc")

   canvas_set_palette("strax.pal", 0)


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

   music_play("strx_mus.lzc", 0)

   fade_in()

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
   if poll_for_key_or_button(135) == true then return end

   --play_sfx(12, false)

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

   if poll_for_esc(45) == true then return end

   fireworks(g_img_tbl, logo)
   fade_out()
end

function show_logos()
   local g_img_tbl = image_load_all("logos.lzc")
   local sprite = sprite_new(g_img_tbl[0], 0, 0, true)
   music_play("mdd_mus.lzc", 0)
   fade_in()

   poll_for_key_or_button(195)
   if should_exit() then return end
   --fade_out()
   sprite.image = g_img_tbl[1]
   fade_in()
   poll_for_key_or_button(195)
   if should_exit() then return end

   sprite.image = g_img_tbl[2]
   sprite.x = 15
   music_play("mdd_mus.lzc", 1)
   fade_in()
   poll_for_key_or_button(175)
   sprite.visible = false
end

function flash_effect(image, text_delay)
   if text_delay == nil then
      text_delay = 300
   end

   canvas_hide_all_sprites()

   local s = sprite_new(image, 0, 0, true)
   local bg = sprite_new(image_new(320,200,15), 0, 0, true)

   if poll_for_key_or_button(6) == true then return end

   local speed = 10
   local i
   for i=0x0,0xff,speed do
      bg.opacity = 0xff - i
      canvas_update()
   end

   return poll_for_key_or_button(text_delay)
end

function show_fair_ground()
   local g_img_tbl = image_load_all("fair.lzc")

   canvas_hide_all_sprites()

   local fair_bg = sprite_new(g_img_tbl[0], 0, 24, true)
   local buildings = sprite_new(g_img_tbl[1][0], 0, 24, true)
   local buildings1 = sprite_new(g_img_tbl[1][1], 320, 24, true)
   local wheel = sprite_new(g_img_tbl[2][0], 0, 24, true)
   local column = sprite_new(g_img_tbl[3], 190, 24, true)
   local podium = sprite_new(g_img_tbl[4], 240, 24, true)
   local people = sprite_new(g_img_tbl[5][0], 0, 24, true)
   local people1 = sprite_new(g_img_tbl[5][1], 320, 24, true)

   local i
   for i=0,9 do
      wheel.image = g_img_tbl[2][i%5]
      if poll_for_key_or_button(7) == true then return end
   end

   for i=0,79 do
      buildings.x = 0 - i
      buildings1.x = 320 - i
      wheel.x = 0 - i * 2
      wheel.image = g_img_tbl[2][i%5]
      column.x = 190 - i * 2
      podium.x = 240 - i * 2
      people.x = 0 - i * 3
      people1.x = 320 - i * 3
      if poll_for_key_or_button(7) == true then return end
   end

   if poll_for_key_or_button(15) == true then return end

end

function show_lowell()

   flash_effect(image_load("credits.lzc", 0))

   canvas_hide_all_sprites()

   local g_img_tbl = image_load_all("lowell.lzc")
   local bg = sprite_new(g_img_tbl[0][0], 0, 24, true)
   local lowell = sprite_new(g_img_tbl[0][1], 0, 24, true)

   fade_in()

   for i=1,20 do
      lowell.image = g_img_tbl[0][i]
      if poll_for_key_or_button(5) == true then return end
   end

   flash_effect(image_load("credits.lzc", 1))

   if poll_for_key_or_button(70) == true then return end
end

function show_fuse()

   canvas_hide_all_sprites()

   local g_img_tbl = image_load_all("fuse.lzc")
   local bg = sprite_new(g_img_tbl[0][0], 0, 24, true)
   local fuse = sprite_new(g_img_tbl[0][1], 0, 24, true)

   music_play("mdd_mus.lzc", 2)

   fade_in(4)

   for i=1,28 do
      fuse.image = g_img_tbl[0][i]
      if poll_for_key_or_button(2) == true then return end
   end

      flash_effect(image_load("credits.lzc", 2))
end

function show_flash()

   canvas_hide_all_sprites()

      local g_img_tbl = image_load_all("fair.lzc")

      canvas_hide_all_sprites()

      local fair_bg = sprite_new(g_img_tbl[0], 0, 24, true)
      local buildings = sprite_new(g_img_tbl[1][0], -80, 24, true)
      local buildings1 = sprite_new(g_img_tbl[1][1], 240, 24, true)
      local column = sprite_new(g_img_tbl[3], 30, 24, true)
      local podium = sprite_new(g_img_tbl[4], 80, 24, true)

   music_play("mdd_mus.lzc", 3)

   g_img_tbl = image_load_all("flash.lzc")

   fade_in(8)
   play_sfx(12, false) --explosion FIXME this might not be the correct sfx
   local s = sprite_new(g_img_tbl[0][0], 80, 24, true)
   local i
   for i=0,9 do
      s.image = g_img_tbl[0][i]
      if poll_for_key_or_button(7) == true then return end
   end
   for i=0,8 do
      s.image = g_img_tbl[1][i]
      if poll_for_key_or_button(6) == true then return end
   end

   flash_effect(image_load("credits.lzc", 3), 215)
end

function show_flight()
   canvas_hide_all_sprites()

   local g_img_tbl = image_load_all("flight.lzc")
   local bg = sprite_new(g_img_tbl[0], 0, 24, true)
   music_play("mdd_mus.lzc", 4)
   fade_in(12)
   local capsule = create_sprite(g_img_tbl[1], 270, 140)

   local i
   for i=0,12 do
      capsule.image.scale = 1 + i * i + (i * 2)
      capsule.x = 270 - i * 4 * i
      capsule.y = 140 - i * 2 * i
      if poll_for_key_or_button(6) == true then return end
   end

   if poll_for_key_or_button(98) == true then return end
   music_stop()
   if poll_for_key_or_button(8) == true then return end

end

function show_cabin()
   canvas_hide_all_sprites()

   local g_img_tbl = image_load_all("cabin.lzc")

   local bg = sprite_new(g_img_tbl[0], 0, 24, true)
   local cloud = sprite_new(g_img_tbl[1][0], 116, 28, true)
   local cloud1 = sprite_new(g_img_tbl[1][0], 222, 53, true)
   local man = create_sprite(g_img_tbl[2], 0, 50)
   local silhouette = create_sprite(g_img_tbl[3], -900, -150)
   local hat1 = create_sprite(g_img_tbl[4], 62, -150)
   local hat2 = create_sprite(g_img_tbl[5], 62, 0)
   local hat3 = create_sprite(g_img_tbl[6], 62, 0)

   music_play("mdd_mus.lzc", 5)

   --FIXME rotate sprites.

   local i
   for i=0,30 do
      cloud.image = g_img_tbl[1][(i + 2) % 15]
      cloud1.image = g_img_tbl[1][i % 15]
      silhouette.x = i * 64 - 900
      silhouette.y = i * 16 - 150
      man.x = 0 - (i * 4)
      man.y = i * i * 2 + 50
      man.image.scale = 100 + i * 32
      hat1.x = i * 32 + 62
      if i > 6 then
         hat1.y = i * 4 + 15
      else
         hat1.y = i * 8 + 15
      end

      hat2.x = i * 32 - 150
      hat2.y = i * 8

      hat3.x = i * 32 - 500
      hat3.y = i * 4

      if poll_for_key_or_button(8) == true then return end
   end

   flash_effect(image_load("credits.lzc", 4),250)
end

function show_mars_flash()
   canvas_hide_all_sprites()

   local g_img_tbl = image_load_all("mars.lzc")

   music_play("mdd_mus.lzc", 6)

   local bg = sprite_new(g_img_tbl[0][0], 0, 24, true)
   local flash = sprite_new(g_img_tbl[0][1], 0, 24, true)

   fade_in(12)

   if poll_for_key_or_button(60) == false then

   local i
   for i=-1,1 do
      local j
      for j=0,2 do
      local k
         for k=-1,2 do
            flash.image = g_img_tbl[0][3-math.abs(k)]

            local delay = 3
            if i == 0 and k == 0 then
               delay = 10
            end
            if poll_for_key_or_button(delay) == true then
               flash_effect(image_load("credits.lzc", 5))
               return
            end
         end

         if poll_for_key_or_button(5) == true then
            flash_effect(image_load("credits.lzc", 5))
            return
         end
      end

      if poll_for_key_or_button(30) == true then
         flash_effect(image_load("credits.lzc", 5))
         return
      end
   end

   end

   poll_for_key_or_button(40)

   flash_effect(image_load("credits.lzc", 5))
end

function play_intro()

   show_logos()
   if should_exit() then return end

   show_fair_ground()
   if should_exit() then return end

   show_lowell()
   if should_exit() then return end

   show_fuse()
   if should_exit() then return end

   show_flash()
   if should_exit() then return end

   show_flight()
   if should_exit() then return end

   show_cabin()
   if should_exit() then return end

   show_mars_flash()
   if should_exit() then return end

end

function show_home()
   canvas_hide_all_sprites()

   local text_tbl = text_load("scenetxt.lzc", 0)

   local g_img_tbl = image_load_all("scene1.lzc")

   local bg = sprite_new(g_img_tbl[0][0], 0, 24, true)
   local door = sprite_new(g_img_tbl[0][1], 0, 24, true)
   local avatar = sprite_new(g_img_tbl[1][0], 240, 151, true)
   local text = sprite_new(nil, 0, 160, true)
   text.text = text_tbl[1]
   text.text_color = 14
   text.text_align = 2
   fade_in()
   music_play("mdd_mus.lzc", 8)

   poll_for_key_or_button(75)
   if should_exit() then return end

   local woman = sprite_new(g_img_tbl[8][0], 0, 151, true)
   local spector = create_sprite(g_img_tbl[2][0], 60, 151)

   door.visible = false
   avatar.image = g_img_tbl[1][1]
   text.text = text_tbl[2]

   poll_for_key_or_button(100)
   if should_exit() then return end

   avatar.image = g_img_tbl[1][3]
   spector.image = g_img_tbl[2][1]
   spector.x = 80
   text.text = text_tbl[3]
   text.text_color = 2

   poll_for_key_or_button(100)
   if should_exit() then return end

   woman.image = g_img_tbl[8][1]
   text.text = text_tbl[4]
   text.text_color = 14

   poll_for_key_or_button(100)
   if should_exit() then return end

   woman.image = g_img_tbl[8][2]
   spector.image = g_img_tbl[2][2]
   spector.x = 100
   text.text = text_tbl[5]
   text.text_color = 2

   poll_for_key_or_button(75)
   if should_exit() then return end

   woman.image = g_img_tbl[3][0]
   woman.x = -10
   woman.y = 155
   spector.image = g_img_tbl[2][3]
   spector.x = 145
   text.text = text_tbl[6]
   text.text_color = 9

   poll_for_key_or_button(75)
   if should_exit() then return end

   --woman hands spector note
   text.text = text_tbl[7]
   local i
   for i=0,10 do
      if i <= 9 then
         woman.image = g_img_tbl[3][i]
      end
      spector.image = g_img_tbl[2][i + 5]
      poll_for_key_or_button(5)
   end

   poll_for_key_or_button(50)
   if should_exit() then return end

   text.text = text_tbl[8]
   text.text_color = 2

   poll_for_key_or_button(100)
   if should_exit() then return end


   --close up on woman
   canvas_hide_all_sprites()

   bg.image = g_img_tbl[7]
   bg.visible = true
   text.text = text_tbl[9]
   text.text_color = 9
   text.visible = true

   poll_for_key_or_button(100)
   if should_exit() then return end

   --spector, avatar review papers

   bg.image = g_img_tbl[0][0]
   avatar.image = g_img_tbl[1][1]
   avatar.x = 270
   avatar.visible = true

   spector.image = g_img_tbl[2][16]
   spector.x = 193
   spector.y = 161
   spector.visible = true

   text.text = text_tbl[10]
   text.text_color = 2

   poll_for_key_or_button(75)
   if should_exit() then return end

   --close up on papers
   canvas_hide_all_sprites()
   bg.image = g_img_tbl[5][0]
   bg.visible = true

   local letter = sprite_new(g_img_tbl[5][1], 50, 24, true)
   local hand = sprite_new(g_img_tbl[4], 235, 78, true)

   text.text = text_tbl[11]
   text.text_color = 14
   text.visible = true

   poll_for_key_or_button(100)
   if should_exit() then return end

   hand.visible = false
   text.text = text_tbl[12]
   text.text_color = 2

   poll_for_key_or_button(100)
   if should_exit() then return end

   --pickup letter
   letter.image = g_img_tbl[6]
   letter.x = 0

   text.text = text_tbl[13]
   --FIXME need to rotate the letter sprite.
   for i=0,19 do
      letter.y = -20 - i
      poll_for_key_or_button(1)
   end

   letter.visible = false
   text.text = text_tbl[14]
   text.text_color = 14

   poll_for_key_or_button(100)
   if should_exit() then return end

   fade_out(6)
end

function show_map()

   local xhair_tbl = {
      {["x"]=190, ["y"]=80},
      {["x"]=180, ["y"]=82},
      {["x"]=170, ["y"]=84},
      {["x"]=162, ["y"]=81},
      {["x"]=154, ["y"]=75},
      {["x"]=145, ["y"]=73},
      {["x"]=135, ["y"]=73},
      {["x"]=125, ["y"]=74},
      {["x"]=115, ["y"]=76},
      {["x"]=105, ["y"]=78},
      {["x"]=95, ["y"]=81},
      {["x"]=86, ["y"]=86}
   }

   canvas_hide_all_sprites()

   local bg = sprite_new(image_load("scene2.lzc", 0), 0, 24, true)
   fade_in(6)

   poll_for_key_or_button(50)
   if should_exit() then return end

   local i
   for i=1,12 do
      image_draw_line(bg.image, xhair_tbl[i].x-1, xhair_tbl[i].y-24-5, xhair_tbl[i].x+3, xhair_tbl[i].y-24-5, 9)
      image_draw_line(bg.image, xhair_tbl[i].x+1, xhair_tbl[i].y-1-24-5, xhair_tbl[i].x+1, xhair_tbl[i].y+1-24-5, 9)
      poll_for_key_or_button(20)
      if should_exit() then return end
   end

   poll_for_key_or_button(50)

   fade_out(6)

end

function show_hike()
   canvas_hide_all_sprites()
   local g_img_tbl = image_load_all("scene2.lzc")

   local bg = sprite_new(g_img_tbl[1], 0, 24, true)
   local avatar = sprite_new(g_img_tbl[3][0], -15, 148, true)
   local spector = sprite_new(g_img_tbl[4][0], -45, 149, true)
   local fg = sprite_new(g_img_tbl[2], 0, 24, true)

   fade_in(6)

   local i
   for i=0,94 do
      avatar.image = g_img_tbl[3][i % 12]
      avatar.x = i * 4 + -15
      spector.image = g_img_tbl[4][(i+4) % 12]
      spector.x = i * 4 + -45
      poll_for_key_or_button(3)
      if should_exit() then return end
   end

   fade_out(6)
end

function show_lab_present_day()
   canvas_hide_all_sprites()
   local g_img_tbl = image_load_all("scene3.lzc")
   local text_tbl = text_load("scenetxt.lzc", 1)

   local text = sprite_new(nil, 0, 160, true)
   text.text = text_tbl[1]
   text.text_color = 2
   text.text_align = 2

   local moongate_tbl = image_load_all("moongate.lzc")

   local bg = sprite_new(g_img_tbl[0], 0, 24, true)

   local moongate = create_sprite(moongate_tbl[0][0], 125, 130)
   moongate.visible = false

   local spector = sprite_new(g_img_tbl[3][0], 165, 137,true)
   local table = create_sprite(g_img_tbl[1][0], 151, 82)
   local avatar = create_sprite(g_img_tbl[2][0], 65, 150)

   --local s = sprite_new(g_img_tbl[4][0], 0, 24,true)
   music_play("mdd_mus.lzc", 9)

   fade_in(6)

   local lab_tbl = {
      {1, 2,  0, 0, 50},
      {1, 2,  0, 1, 100},
      {2, 14, 0, 1, 100},
      {3, 2,  0, 2, 100},
      {4, 2, -1, 0, 100},
      {5, 14,-2, 0, 100},
      {6, 2, -1, 0, 100},
      {7, 14, 1, 3, 100},
      {8, 2,  1, 2, 100},
      {9, 14, 0, 2, 100}

   }

   local i
   for i=1,7 do
      if lab_tbl[i][3] >= 0 then
         spector.image = g_img_tbl[3][lab_tbl[i][4]]
      else
         spector.visible = false
         table.visible = false
         avatar.visible = false
         bg.image = g_img_tbl[4][math.abs(lab_tbl[i][3])-1]
      end

      text.text = text_tbl[lab_tbl[i][1]]
      text.text_color = lab_tbl[i][2]
      poll_for_key_or_button(lab_tbl[i][5])
      if should_exit() then return end
   end

   bg.image = g_img_tbl[0]
   spector.visible = true
   avatar.visible = true
   table.visible = true
   table.image = g_img_tbl[1][1]

   spector.image = g_img_tbl[3][3]
   avatar.image = g_img_tbl[2][1]

   moongate.visible = true

   --moongate rises up from floor
   for i=0,8 do
      moongate.image = moongate_tbl[0][i]
      poll_for_key_or_button(4)
      if should_exit() then return end
   end

   --avatar, spector discuss moongate
   moongate.image = moongate_tbl[1][0]
   for i=8,10 do
      local j
      for j=0,39 do
         moongate.image = moongate_tbl[1][j % 8]
         avatar.image = g_img_tbl[2][lab_tbl[i][3]]
         spector.image = g_img_tbl[3][lab_tbl[i][4]]
         text.text = text_tbl[lab_tbl[i][1]]
         text.text_color = lab_tbl[i][2]

         poll_for_key_or_button(4)
         if should_exit() then return end
      end
   end


   spector.image = moongate_tbl[4][0]
   for i=0,64 do
      moongate.image = moongate_tbl[1][i % 8]
      avatar.image = moongate_tbl[2][math.floor(i/2)]

      poll_for_key_or_button(4)
      if should_exit() then return end
   end

   avatar.visible = false

   for i=0,39 do
      moongate.image = moongate_tbl[1][i % 8]
      if i ~= 39 then
         spector.image = moongate_tbl[4][math.floor(i/2)]
      end

      poll_for_key_or_button(4)
      if should_exit() then return end
   end

   fade_out(6)
end

function show_lab_1895()
   canvas_hide_all_sprites()
   local scene4a_tbl = image_load_all("scene4a.lzc")
   local scene4b_tbl = image_load_all("scene4b.lzc")
   local moongate_tbl = image_load_all("moongate.lzc")

   local text_tbl = text_load("scenetxt.lzc", 2)

   local text = sprite_new(nil, 0, 160, true)
   text.text = text_tbl[1]
   text.text_color = 6
   text.text_align = 2

   local bg = sprite_new(scene4a_tbl[0], 0, 24, true)

   local moongate = create_sprite(moongate_tbl[0][0], 140, 125)
   moongate.visible = false

   local tesla = sprite_new(scene4a_tbl[6][0], 187, 125, true)
   local spark = sprite_new(scene4b_tbl[2][0], 32, 24, false)

   local freud_body = create_sprite(scene4a_tbl[4][0], 0, 74)
   local freud_head = create_sprite(scene4a_tbl[4][2], 23, 100)

   local nellie = sprite_new(scene4b_tbl[1][0], 242, 24, false)

   local bookcase = sprite_new(scene4a_tbl[1], 242, 24, false)

   local blood = sprite_new(scene4a_tbl[5][0], 240, 40, true)
   local garrett = sprite_new(scene4b_tbl[0][0], 270, 45, false)

   fade_in(6)

   music_play("mdd_mus.lzc", 10)

   local i
   for i=1,2 do
      text.text = text_tbl[i]
      local j
      for j=0,20 do
         if 10 - math.abs(j-10) > 5 then
            tesla.image = scene4a_tbl[6][5]
         else
            tesla.image = scene4a_tbl[6][10 - math.abs(j-10)]
         end

         if j > 4 and j < 16 then
            --FIXME need spark sfx
            spark.visible = true
            spark.image = scene4b_tbl[2][j % 10]
         else
            spark.visible = false
         end

         poll_for_key_or_button(4)
         if should_exit() then return end
      end
      text.text_color = 7
   end

   moongate.visible = true

   --moongate rises up from floor
   tesla.x = 188
   tesla.y = 128
   tesla.image = scene4a_tbl[6][6]

   for i=0,8 do
      moongate.image = moongate_tbl[0][i]
      if i == 5 then
         tesla.image = scene4a_tbl[6][7]
      end

      if i == 2 then
         blood.image = scene4a_tbl[5][1]
      end

      poll_for_key_or_button(4)
      if should_exit() then return end
   end

   --avatar walks out of the newly risen moongate
   local avatar = create_sprite(moongate_tbl[3][0], 80, 150)

   tesla.x = 180
   tesla.y = 133
   tesla.image = scene4a_tbl[6][8]

   for i=0,68 do
      moongate.image = moongate_tbl[1][i % 8]
      avatar.image = moongate_tbl[3][math.floor(i/2)]
      if i == 3 then
         blood.image = scene4a_tbl[5][2]
      end

      if i == 4 then
         freud_head.image = scene4a_tbl[4][3]
      end

      if i == 11 then
         text.text = text_tbl[3]
         text.text_color = 7
      end

      poll_for_key_or_button(3)
      if should_exit() then return end
   end

   --spector walks out of moongate
   local spector = create_sprite(moongate_tbl[5][0], 140, 130)

   text.text = text_tbl[4]
   text.text_color = 14

   for i=0,24 do
      moongate.image = moongate_tbl[1][i % 8]
      spector.image = moongate_tbl[5][math.floor(i/2)]

      poll_for_key_or_button(3)
      if should_exit() then return end
   end

   --spector face closeup shot
   canvas_hide_all_sprites()

   bg.image = scene4a_tbl[9][0]
   bg.visible = true

   local face = sprite_new(scene4a_tbl[9][1], 0, 24, true)

   text.text = text_tbl[5]
   text.text_color = 2
   text.visible = true

   poll_for_key_or_button(200)
   if should_exit() then return end

   --spector shows note to tesla
   text.text = text_tbl[6]
   face.visible = false
   bg.image = scene4a_tbl[0]

   tesla.visible = true
   tesla.image = scene4a_tbl[6][9]

   blood.visible = true

   freud_head.image = scene4a_tbl[4][2]
   freud_head.visible = true

   freud_body.visible = true

   avatar.visible = true
   avatar.x = 80
   avatar.y = 150
   avatar.image = scene4a_tbl[2][1]

   spector.visible = true
   spector.x = 145
   spector.y = 140
   spector.image = scene4a_tbl[3][1]

   poll_for_key_or_button(200)
   if should_exit() then return end

   --tesla face closeup shot
   canvas_hide_all_sprites()

   bg.image = scene4a_tbl[7][0]
   bg.visible = true

   face.image = scene4a_tbl[7][1]
   face.visible = true

   text.text = text_tbl[7]
   text.text_color = 7
   text.visible = true

   poll_for_key_or_button(200)
   if should_exit() then return end

   --blood closeup.
   canvas_hide_all_sprites()

   bg.image = scene4a_tbl[8][0]
   bg.visible = true

   face.visible = true
   text.visible = true
   text.text_color = 4

   for i=8,9 do
      face.image = scene4a_tbl[8][i - 7]
      text.text = text_tbl[i]

      poll_for_key_or_button(200)
      if should_exit() then return end
   end

   --Tesla calls Mr Garrett.
   text.text = text_tbl[10]
   text.text_color = 7
   face.visible = false
   bg.image = scene4a_tbl[0]

   tesla.visible = true
   tesla.image = scene4a_tbl[6][10]

   blood.visible = true

   freud_head.visible = true
   freud_body.visible = true

   avatar.visible = true
   avatar.image = scene4a_tbl[2][0]

   spector.visible = true
   spector.image = scene4a_tbl[3][0]

   poll_for_key_or_button(200)
   if should_exit() then return end

   --Mr Garrett entrance
   garrett.visible = true
   spector.x = 150
   spector.y = 140
   text.text = text_tbl[11]
   text.text_color = 9
   for i=0,31 do
      if i == 5 then
         spector.image = scene4a_tbl[3][2]
      end
      if i <= 10 then
         avatar.image = scene4a_tbl[2][i]
      end

      garrett.image = scene4b_tbl[0][i]

      poll_for_key_or_button(3)
      if should_exit() then return end
   end

   --Tesla checks note, talks about mission.

   spector.image = scene4a_tbl[3][1]
   spector.x = 155
   avatar.image = scene4a_tbl[2][10]
   freud_head.image = scene4a_tbl[4][1]

   local text_color_tbl = {4, 6, 7, 7, 14, 2, 2, 7, 4, 4, 7, 9, 7, 4, 7, 4, 7, 2, 7, 7, 7, 7, 7, 11, 2, 7, 7}

   for i=12,20 do
      if i==13 or i==16 or i==20 then
         face.visible = true
         tesla.visible = false
         blood.visible = false
         freud_head.visible = false
         freud_body.visible = false
         avatar.visible = false
         spector.visible = false
         garrett.visible = false
         local idx
         if i==13 then
            idx = 8
         else
            idx = 7
         end
         bg.image = scene4a_tbl[idx][0]

         if i==16 then
            face.image = scene4a_tbl[idx][2]
         else
            face.image = scene4a_tbl[idx][1]
         end
      else
         local idx
         if i == 14 then
            spector.image = scene4a_tbl[3][0]
         end
         if i == 12 or i == 17 then
            idx = 9
         elseif i == 14 or i == 15 then
            idx = 10
         else
            idx = 11
         end
         tesla.image = scene4a_tbl[6][idx]
      end

      text.text = text_tbl[i]
      text.text_color = text_color_tbl[i+1]

      poll_for_key_or_button(200)
      if should_exit() then return end

      face.visible = false
      tesla.visible = true
      blood.visible = true
      freud_head.visible = true
      freud_body.visible = true
      avatar.visible = true
      spector.visible = true
      garrett.visible = true
      bg.image = scene4a_tbl[0]
   end


   --Nellie walks in

   tesla.image = scene4a_tbl[6][10]

   text.text = text_tbl[21]
   text.text_color = text_color_tbl[22]

   nellie.visible = true
   nellie.y = 125
   bookcase.visible = true

   for i=0,31 do
      if i == 7 then
         freud_head.image = scene4a_tbl[4][2]
      end
      nellie.image = scene4b_tbl[1][i%4]
      nellie.x = 320 - i * 3
      if i > 25 then
         nellie.y = 125 + (i - 26) * 2
      end

      local avatar_idx = 20 - i
      if avatar_idx > 10 then
      avatar_idx = 10
      end

      if avatar_idx < 4 then
      avatar_idx = 4
      end
      avatar.image = scene4a_tbl[2][avatar_idx]

      local garrett_idx = 45 - i
      if garrett_idx > 31 then
         garrett_idx = 31
      end

      if garrett_idx < 18 then
         garrett_idx = 18
      end
      garrett.image = scene4b_tbl[0][garrett_idx]

      poll_for_key_or_button(3)
      if should_exit() then return end
   end

   --Nellie shakes hands with Spector, Avatar

   spector.x = 165
   nellie.x = 227
   nellie.y = 135

   for i=22,26 do
      if i == 24 then
         nellie.image = scene4b_tbl[1][7]
         spector.image = scene4a_tbl[3][3]
      else
         if i == 23 then
            nellie.image = scene4b_tbl[1][6]
         else
            nellie.image = scene4b_tbl[1][5]
         end
         spector.image = scene4a_tbl[3][0]
      end

      if i == 25 then
         tesla.image = scene4a_tbl[6][9]
      elseif i == 26 then
         tesla.image = scene4a_tbl[6][11]
      else
         tesla.image = scene4a_tbl[6][12]
      end

      text.text = text_tbl[i]
      text.text_color = text_color_tbl[i+1]

      poll_for_key_or_button(200)
      if should_exit() then return end
   end

end

function run_introduction()

   show_home()
   if should_exit() then return end

   show_map()
   if should_exit() then return end

   show_hike()
   if should_exit() then return end

   show_lab_present_day()
   if should_exit() then return end

   show_lab_1895()
   if should_exit() then return end

   fade_out(6)
end

local char_creation_tbl = {
   {["text"]=-1, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=-2, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=3,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=4,  ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=5,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -4},
   {["text"]=6,  ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=7,  ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=8,  ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=9,  ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=10, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=11, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=12, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=13, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=14, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -3},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=15, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=16, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=17, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=18, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=19, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=20, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=21, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=22, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=23, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=24, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=25, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=26, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=27, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=28, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=29, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=30, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=31, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=32, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=33, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=34, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=35, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=36, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=37, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=38, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=39, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=40, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=41, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=42, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=43, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=44, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=45, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=46, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=47, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=48, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=49, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=50, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=51, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=52, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=53, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=54, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=55, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=56, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=57, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=58, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=59, ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -2},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=60, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=61, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=62, ["eye_sprite"]=3, ["can_move_pen"]=true, ["action_code"]= -1},
   {["text"]=63, ["eye_sprite"]=1, ["can_move_pen"]=false, ["action_code"]= -1},
   {["text"]=0,  ["eye_sprite"]=2, ["can_move_pen"]=false, ["action_code"]= -1}
}

local g_player_name = ""
local gender_answer = ""
local avatar_str
local avatar_dex
local avatar_int

function insert_player_name(text)
   local output = ""
   local i = string.find(text,"$P")
   while i ~= nil do
      output = output .. string.sub(text,1,i-1) .. g_player_name
      text = string.sub(text,i+2)
      i = string.find(text, "$P")
   end
   output = output .. text

   return output
end

local g_keycode_tbl =
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

local g_name_sprite = sprite_new(nil, 120, 184, false)
g_name_sprite.text_color = 14
local g_cursor_sprite = sprite_new(nil, 120, 184, false)
g_cursor_sprite.text = "_"
g_cursor_sprite.text_color = 14
local g_cursor_timer = 0
g_char_index = 0


function collect_player_name()
   local name_text = g_name_sprite.text
   local len = string.len(name_text)

   g_name_sprite.visible = true

   local input = poll_for_input()
   if input ~= nil then
      if should_exit() then
         return false
      end

      if (input == SDLK_BACKSPACE or input == SDLK_LEFT) and len > 0 then
         g_name_sprite.text = string.sub(name_text, 1, len - 1)
         len = len - 1
         if len == 1 then -- old len
           g_char_index = 0
         else
           g_char_index = string.byte(name_text, len -1)
         end
      elseif (input == SDLK_RETURN or input == SDLK_KP_ENTER) and string.len(trim_string(name_text)) > 0 then --return
         g_name_sprite.visible = false
         g_cursor_sprite.visible = false
         g_player_name = trim_string(name_text)
         return true
      elseif input == SDLK_UP then --up
         if g_char_index == 0 then
           if len > 0 then
             g_char_index = SDLK_a
           else
             g_char_index = 65 --A
         end
         elseif g_char_index <= 32 then --gap in characters
           g_char_index = 48
         elseif g_char_index >= 57 and  g_char_index < 65 then --gap in characters
           g_char_index = 65
         elseif g_char_index >= 90 and g_char_index < 97 then --gap in characters
           g_char_index = 97
         elseif g_char_index >= 122 then --last char
           g_char_index = 32
         else
           g_char_index = g_char_index + 1
         end

         if len > 0 then -- erase char
           name_text = string.sub(name_text, 1, len - 1)
         end
         g_name_sprite.text = name_text..g_keycode_tbl[g_char_index]
      elseif input == SDLK_DOWN then --down
         if g_char_index == 0 then
           if len > 0 then
             g_char_index = 122 --z
           else
             g_char_index = 90 --Z
           end
         elseif g_char_index == 65 then --gap in characters
           g_char_index = 57
         elseif g_char_index == 97 then --gap in characters
           g_char_index = 90
         elseif g_char_index <= 32 then --first char
           g_char_index = 122
         elseif g_char_index <= 48 then --gap in characters
           g_char_index = 32
         else
           g_char_index = g_char_index - 1
         end

         if len > 0 then -- erase char
           name_text = string.sub(name_text, 1, len - 1)
         end
         g_name_sprite.text = name_text..g_keycode_tbl[g_char_index]
      elseif input == SDLK_RIGHT and len < 13 then --right
         g_char_index = SDLK_a --a
         g_name_sprite.text = name_text.."a"
      elseif g_keycode_tbl[input] ~= nil and len < 13 then
         g_name_sprite.text = name_text..g_keycode_tbl[input]
         len = len + 1
      end

   end

   if len == 13 then
      g_cursor_sprite.visible = false
   else
      g_cursor_sprite.x = 120 + g_name_sprite.text_width
      if g_cursor_timer == 0 then
         g_cursor_timer = 8
         if g_cursor_sprite.visible == true then
            g_cursor_sprite.visible = false
         else
            g_cursor_sprite.visible = true
         end
      else
         g_cursor_timer = g_cursor_timer - 1
      end
   end

   return false
end

function update_freud(freud)
   if freud.eyes.blink_timer < 200 then
      if freud.timer == 0 then
         freud.timer = math.random(100, 350)
         if freud.state == FREUD_STATE_STARING then
            freud.state = FREUD_STATE_WRITING
         else
            freud.state = FREUD_STATE_STARING
         end
      else
         freud.timer = freud.timer - 1
      end

      local state = freud.state
      if state == FREUD_STATE_STARING then
         if freud.can_move_pen == true then
            freud.eyes.sprite.image = freud.images[0][1]
         else
            freud.eyes.sprite.image = freud.images[0][freud.eyes.sprite_idx]
         end
      elseif state == FREUD_STATE_WRITING and freud.can_move_pen == true then
         freud.eyes.sprite.image = freud.images[0][freud.eyes.sprite_idx]
         if freud.timer % 4 == 0 then
            freud.pen.sprite.image = freud.images[2][math.random(0,6)]
            freud.pen.x_off = freud.pen.x_off + math.random(0, 3)
            if freud.pen.x_off >= 30 then
               freud.pen.x_off = freud.pen.x_off % 30
               freud.pen.y_off = (freud.pen.y_off + 2) % 20
            end

            freud.pen.sprite.x = 128 - freud.pen.x_off
            freud.pen.sprite.y = 83 + freud.pen.y_off
         end
      end
   end

   if freud.eyes.blink_timer == 200 then
      if freud.eyes.sprite_idx == 2 then
         freud.eyes.sprite.image = freud.images[0][5]
      else
         freud.eyes.sprite.image = freud.images[0][4]
      end
   elseif freud.eyes.blink_timer == 215 then
      if state == FREUD_STATE_WRITING or freud.can_move_pen == false then
         freud.eyes.sprite.image = freud.images[0][freud.eyes.sprite_idx]
      else
         freud.eyes.sprite.image = freud.images[0][1]
      end
      freud.eyes.blink_timer = -1
   end
   freud.eyes.blink_timer = freud.eyes.blink_timer + 1
end

function ask_question(question_idx, text, freud)
   question_idx = question_idx + 1

   local key_input = nil

   local text_offset = char_creation_tbl[question_idx].text

   while text_offset ~= 0 do

      if text_offset < 0 then
         text.text_color = 4
      else
         text.text_color = 6
      end

      text.text = insert_player_name(freud.text_tbl[math.abs(text_offset)])

      freud.eyes.sprite_idx = char_creation_tbl[question_idx].eye_sprite
      freud.eyes.sprite.image = freud.images[0][freud.eyes.sprite_idx]
      freud.eyes.blink_timer = 0
      freud.state = FREUD_STATE_STARING
      freud.can_move_pen = char_creation_tbl[question_idx].can_move_pen
      local action = char_creation_tbl[question_idx].action_code

      local continue_loop = true
      while continue_loop do
         if action == -1 then
            local input = poll_for_input()
            if input ~= nil then
               continue_loop = false
            end
         elseif action == -2 then
            -- A, B input
            local input = poll_for_input()
            if input ~= nil and (input == SDLK_a or input == SDLK_b) then
               key_input = input
               continue_loop = false
            end
         elseif action == -3 then
            local input = poll_for_input()
            if input ~= nil and (input == SDLK_a or input == SDLK_b or input == SDLK_c) then
               key_input = input
               continue_loop = false
            end
         elseif action == -4 then
            if collect_player_name() == true then
               continue_loop = false
            end
         end

         if should_exit() then
            return nil
         end

         update_freud(freud)
         canvas_update()
      end

      question_idx = question_idx + 1
      text_offset = char_creation_tbl[question_idx].text
   end

   return key_input
end

function question_1_answer_a(text, freud, rand_high, rand_low)
      ask_question(18, text, freud)
      local var_14 = ask_question(20, text, freud)
      if var_14 == SDLK_a then
         local var_16 = ask_question(23, text, freud)
         if var_16 == SDLK_a then
            local answer = ask_question(29, text, freud)
            if answer == SDLK_a then
               avatar_str = rand_high
               avatar_int = rand_low
               if gender_answer == SDLK_a then
                  ask_question(31, text, freud)
               else
                  ask_question(34, text, freud)
               end
            elseif answer == SDLK_b then
               avatar_int = rand_high
               avatar_str = rand_low
            end
         elseif var_16 == SDLK_b then
            local answer = ask_question(37, text, freud)
            if answer == SDLK_a then
               avatar_int = rand_high
               avatar_dex = rand_low
               ask_question(40, text, freud)
            elseif answer == SDLK_b then
               avatar_dex = rand_high
               avatar_int = rand_low
               ask_question(43, text, freud)
            end
         end
      elseif var_14 == SDLK_b then
         local var_16 = ask_question(26, text, freud)
         if var_16 == SDLK_a then
            local answer = ask_question(46, text, freud)
            if answer == SDLK_a then
               avatar_str = rand_high
               avatar_dex = rand_low
               if gender_answer == SDLK_a then
                  ask_question(49, text, freud)
               else
                  ask_question(52, text, freud)
               end
            elseif answer == SDLK_b then
               avatar_dex = rand_high
               avatar_str = rand_low
               if gender_answer == SDLK_a then
                  ask_question(58, text, freud)
               else
                  ask_question(55, text, freud)
               end
            end
         elseif var_16 == SDLK_b then
            local answer = ask_question(61, text, freud)
            if answer == SDLK_a then
               avatar_int = rand_high
               avatar_dex = rand_low
               ask_question(64, text, freud)
            elseif answer == SDLK_b then
               avatar_dex = rand_high
               avatar_int = rand_low
               ask_question(66, text, freud)
            end
         end
      end
end

function question_1_answer_b(text, freud, rand_high, rand_low)
   local var_14 = ask_question(26, text, freud)
   if var_14 == SDLK_a then
      local var_16 = ask_question(23, text, freud)
      if var_16 == SDLK_a then
         local answer = ask_question(29, text, freud)
         if answer == SDLK_a then
            avatar_str = rand_high
            avatar_int = rand_low
            if gender_answer == SDLK_a then
               ask_question(31, text, freud)
            else
               ask_question(34, text, freud)
            end
         elseif answer == SDLK_b then
            avatar_int = rand_high
            avatar_str = rand_low
         end
      elseif var_16 == SDLK_b then
         local answer = ask_question(37, text, freud)
         if answer == SDLK_a then
            avatar_int = rand_high
            avatar_dex = rand_low
            ask_question(40, text, freud)
         elseif answer == SDLK_b then
            avatar_dex = rand_high
            avatar_int = rand_low
            ask_question(43, text, freud)
         end
      end
   elseif var_14 == SDLK_b then
      local var_16 = ask_question(20, text, freud)
      if var_16 == SDLK_a then
         local answer = ask_question(72, text, freud)
         if answer == SDLK_a then
            avatar_int = rand_high
            avatar_str = rand_low
            ask_question(75, text, freud)
         elseif answer == SDLK_b then
            avatar_str = rand_high
            avatar_int = rand_low
            ask_question(78, text, freud)
         end
      elseif var_16 == SDLK_b then
         local answer = ask_question(46, text, freud)
         if answer == SDLK_a then
            avatar_str = rand_high
            avatar_dex = rand_low
            if gender_answer == SDLK_a then
               ask_question(49, text, freud)
            else
               ask_question(52, text, freud)
            end
         elseif answer == SDLK_b then
            avatar_dex = rand_high
            avatar_str = rand_low
            if gender_answer == SDLK_a then
               ask_question(58, text, freud)
            else
               ask_question(55, text, freud)
            end
         end
      end
   end
end

function question_1_answer_c(text, freud, rand_high, rand_low)
   ask_question(81, text, freud)
   local var_14 = ask_question(23, text, freud)
   if var_14 == SDLK_a then
      local var_16 = ask_question(26, text, freud)
      if var_16 == SDLK_a then
         local answer = ask_question(46, text, freud)
         if answer == SDLK_a then
            avatar_str = rand_high
            avatar_dex = rand_low
            if gender_answer == SDLK_a then
               ask_question(49, text, freud)
            else
               ask_question(52, text, freud)
            end
         elseif answer == SDLK_b then
            avatar_dex = rand_high
            avatar_str = rand_low
            if gender_answer == SDLK_a then
               ask_question(58, text, freud)
            else
               ask_question(55, text, freud)
            end
         end
      elseif var_16 == SDLK_b then
         local answer = ask_question(37, text, freud)
         if answer == SDLK_a then
            avatar_int = rand_high
            avatar_dex = rand_low
            ask_question(40, text, freud)
         elseif answer == SDLK_b then
            avatar_dex = rand_high
            avatar_int = rand_low
            ask_question(43, text, freud)
         end
      end
   elseif var_14 == SDLK_b then
      local var_16 = ask_question(20, text, freud)
      if var_16 == SDLK_a then
         local answer = ask_question(29, text, freud)
         if answer == SDLK_a then
            avatar_str = rand_high
            avatar_int = rand_low
            if gender_answer == SDLK_a then
               ask_question(31, text, freud)
            else
               ask_question(34, text, freud)
            end
         elseif answer == SDLK_b then
            avatar_int = rand_high
            avatar_str = rand_low
         end
      elseif var_16 == SDLK_b then
         local answer = ask_question(84, text, freud)
         if answer == SDLK_a then
            avatar_dex = rand_high
            avatar_str = rand_low
            ask_question(87, text, freud)
         elseif answer == SDLK_b then
            avatar_str = rand_high
            avatar_dex = rand_low
            ask_question(90, text, freud)
         end
      end
   end
end

function create_character()
   canvas_hide_all_sprites()
   local create_tbl = image_load_all("create.lzc")

   local text_tbl = text_load("scenetxt.lzc", 5)

   local text = sprite_new(nil, 0, 160, true)
   text.text = text_tbl[0]
   text.text_color = 2
   text.text_align = 2

   local bg = sprite_new(create_tbl[0][0], 0, 24, true)
   local eyes_sprite = sprite_new(create_tbl[0][4], 0, 24, true)
   local pen_sprite = sprite_new(create_tbl[2][0], 128, 83, true)
   local clipboard = sprite_new(create_tbl[1], 89, 109, true)

   local eyes ={["sprite"]=eyes_sprite, ["sprite_idx"]=4, ["blink_timer"]=0}
   local pen = {["sprite"]=pen_sprite,["x_off"]=0,["y_off"]=0,["sprite_idx"]=0,["timer"]=0,}
   local freud = {["eyes"]=eyes, ["pen"]=pen, ["state"]=FREUD_STATE_STARING, ["timer"] = 0, ["can_move_pen"] = false, ["images"]=create_tbl, ["text_tbl"]=text_tbl}

   music_play("mdd_mus.lzc", 7)

   local rand_high = math.random(24,26)
   local rand_low = math.random(22,24)
   local stat_base = math.random(18,22)

   avatar_str = stat_base
   avatar_dex = stat_base
   avatar_int = stat_base

   local gender

   gender_answer = ask_question(0, text, freud)
   if gender_answer == SDLK_a then
      gender = 0 --male
      ask_question(8, text, freud)
   elseif gender_answer == SDLK_b then
      gender = 1 --female
      ask_question(11, text, freud)
   end

   local answer = ask_question(14, text, freud)

   if answer == SDLK_a then
      question_1_answer_a(text, freud, rand_high, rand_low)
   elseif answer == SDLK_b then
      question_1_answer_b(text, freud, rand_high, rand_low)
   elseif answer == SDLK_c then
      question_1_answer_c(text, freud, rand_high, rand_low)
   end

   if should_exit() then
      return false
   end

   ask_question(69, text, freud)

   config_set("config/newgame", true)
   config_set("config/newgamedata/name", g_player_name)
   config_set("config/newgamedata/gender", gender)
   config_set("config/newgamedata/str", avatar_str)
   config_set("config/newgamedata/dex", avatar_dex)
   config_set("config/newgamedata/int", avatar_int)

   --wait_for_input()
   fade_out(6)

   return true
end

function journey_onward()
end

local g_menu_idx = 0
local g_menu_cursor_sprite = nil
local g_menu_bg_sprite = nil

function execute_menu_item(cursor_pos)
   if cursor_pos ~= nil then
      set_menu_cursor_pos(cursor_pos)
   end

   hide_mouse_cursor()

   if g_menu_idx == 0 then -- story so far
      run_introduction()
   elseif g_menu_idx == 1 then -- create char
      if create_character() == true then
         return "J" -- starting new game
      end
   elseif g_menu_idx == 2 then -- journey onward
      journey_onward()
      return "J"
   elseif g_menu_idx == 3 then -- about md
      about_martian_dreams()
   end

   clear_should_exit_flag()
   canvas_hide_all_sprites()
   canvas_set_opacity(0xff)
   music_stop()
   g_menu_bg_sprite.visible = true
   g_menu_cursor_sprite.visible = true

   return "";
end

function update_menu_cursor()
   local box_y_tbl = {[0]=130,[1]=147,[2]=164,[3]=181}
   g_menu_cursor_sprite.y = box_y_tbl[g_menu_idx]
end

function set_menu_cursor_pos(new_pos)
   g_menu_idx = new_pos
   update_menu_cursor()
end

function menu_cursor_down()
   g_menu_idx = (g_menu_idx + 1) % 4
   update_menu_cursor()
end

function menu_cursor_up()
   g_menu_idx = g_menu_idx - 1
   if g_menu_idx < 0 then
      g_menu_idx = 3
   end
   update_menu_cursor()
end

local g_mouse_cursor_visible = false

function show_mouse_cursor()
   if g_mouse_cursor_visible == false then
      mouse_cursor_set_pointer(9)
      mouse_cursor_visible(true)
      g_mouse_cursor_visible = true
   end
end

function hide_mouse_cursor()
   if g_mouse_cursor_visible == true then
      mouse_cursor_visible(false)
      g_mouse_cursor_visible = false
   end
end

function main_menu()
   canvas_hide_all_sprites()
   music_stop()
   clear_should_exit_flag()
   local g_img_tbl = image_load_all("mdmenu.lzc")

   g_menu_bg_sprite = sprite_new(g_img_tbl[0][0], 0, 0, true)

   fade_in()

   g_menu_cursor_sprite = sprite_new(g_img_tbl[0][2], 26, 0, true)
   update_menu_cursor()
   canvas_set_update_interval(10)

   while true do
      canvas_update()
      local input = input_poll(true)

      if engine_should_quit() == 1 then
         return "Q"
      end

      if input ~= nil then
         canvas_set_update_interval(25)

         if input == SDLK_q then -- q
            return "Q"
         elseif input == SDLK_RETURN or input == SDLK_SPACE or input == KP_ENTER then -- space or return
            if execute_menu_item() == "J" then
               return "J"
            end
         elseif input == SDLK_r or input == SDLK_i then -- Run Introduction
            execute_menu_item(0)
         elseif input == SDLK_c then -- c (create char)
            if execute_menu_item(1) == "J" then
               return "J" -- starting new game
            end
         elseif input == SDLK_j or input == SDLK_g then -- j, g (journey onward, continue Game)
            execute_menu_item(2)
            return "J"
         elseif input == SDLK_a then -- a (about MD)
            execute_menu_item(3)
         elseif input == SDLK_DOWN or input == SDL_KP2 then -- down key
            menu_cursor_down()
         elseif input == SDLK_UP or input == SDL_KP8 then -- up key
            menu_cursor_up()
         elseif input == MOUSE_MOTION or input == MOUSE_CLICK then --mouse movement
            show_mouse_cursor()
            local x = get_mouse_x()
            local y = get_mouse_y()
            if x > 57 and x < 260 and y > 130 then
               if y > 130 and y < 148 then -- run introduction
                  set_menu_cursor_pos(0)
               elseif y > 147 and y < 164 then -- create new char
                  set_menu_cursor_pos(1)
               elseif y > 163 and y < 181 then -- continue game
                  set_menu_cursor_pos(2)
               elseif y > 180 then -- about MD
                  set_menu_cursor_pos(3)
               end
               if input == MOUSE_CLICK then
                  if execute_menu_item() == "J" then
                     return "J"
                  end
               end
            end
         end

         canvas_set_update_interval(10)
      end
   end

end


mouse_cursor_visible(false)
canvas_set_update_interval(25)
canvas_set_bg_color(0)
canvas_set_opacity(0)

origin_fx_sequence()

canvas_set_palette("md_title.pal", 0)
canvas_hide_all_sprites()
play_intro()

if main_menu() == "Q" then -- returns "Q" for quit or "J" for Journey Onward
   hide_mouse_cursor()
   fade_out(6)
   config_set("config/quit", true)
end

music_stop()
canvas_hide()
