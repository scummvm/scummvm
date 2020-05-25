local lua_file = nil

--load common functions
lua_file = nuvie_load("common/intro_common.lua"); lua_file();

function play()
   local gender = player_get_gender()
   local g_img_tbl = image_load_all("endgame.lzc")
   local text_tbl = text_load("scenetxt.lzc", 3)
   local text = sprite_new(nil, 0, 160, true)
   text.text_color = 4
   text.text = text_tbl[0]
   text.text_align = 2

   local bg = sprite_new(g_img_tbl[0][0], 0, 24, true)
   local jack = sprite_new(g_img_tbl[0][gender+1], 155, 151, true)

   canvas_set_palette("md_title.pal", 0)
   canvas_set_opacity(0xff);
   mouse_cursor_visible(false)
   canvas_set_update_interval(25)

   music_play("mdd_mus.lzc", 11)
   fade_in()

   poll_for_key_or_button(150)
   if should_exit() then return end

   text.text = text_tbl[1]

   poll_for_key_or_button(150)
   if should_exit() then return end

   --space capsule rockets away from mars

   jack.visible = false
   text.text = text_tbl[2]

   bg.image = g_img_tbl[1][0]

   local capsule = create_sprite(g_img_tbl[1][1], 105, 112)
   capsule.visible = false
   local smoke = sprite_new(g_img_tbl[1][2], 105, 112, false)

   local i
   for i=-1,39 do
      bg.y = 23 + ((i + 3) % 4)
      bg.x = ((i + 2) % 5) - 2

      if i > -1 and i < 13 then
         capsule.visible = true
         capsule.x = 105 + (i * i * 2) / 3
         capsule.y = 112 - i * i
         capsule.image.scale = (i * i) + 16

         smoke.image = g_img_tbl[1][2 + math.floor(i/2)]
         smoke.visible = true
      else
         capsule.visible = false
         smoke.visible = false
      end

      poll_for_key_or_button(3)
      if should_exit() then return end
   end

   --capsule lands in the ocean.
   bg.image = g_img_tbl[2][0]
   bg.x = 0
   bg.y = 24

   capsule.visible = true
   capsule.x = 170
   capsule.y = 150

   text.text = text_tbl[3]

   for i=0,9 do
      local j
      for j=0,3 do
         capsule.image = g_img_tbl[2][math.abs(2 - j) + 1]
         if i == 5 then
            text.text = text_tbl[4]
         end
         poll_for_key_or_button(8)
         if should_exit() then return end
      end
   end

   --ticket tape parade
   canvas_set_opacity(0);
   capsule.visible = false

   bg.image = g_img_tbl[3][0]
   local spector = sprite_new(g_img_tbl[3][gender+1], 190, 151, true)
   text.text = text_tbl[5]

   fade_in(6)

   poll_for_key_or_button(150)
   if should_exit() then return end


   --group photo.
   canvas_set_opacity(0);
   spector.visible = false

   bg.image = g_img_tbl[4][0]

   local moongate_tbl = image_load_all("moongate.lzc")
   local moongate = sprite_new(moongate_tbl[1][0], 35, 135, true)
   local group = sprite_new(g_img_tbl[4][gender+1], 195, 151, true)

   local photographer = sprite_new(g_img_tbl[4][3], 75, 151, true)

   text.text = text_tbl[6]

   fade_in(6)

   for i=0,79 do
      if i == 40 then
         moongate.visible = false
         group.visible = false
         photographer.visible = false
         bg.image = g_img_tbl[4][5]
         poll_for_key_or_button(2)
         fade_out(6)
         bg.image = g_img_tbl[4][0]
         moongate.visible = true
         group.visible = true
         photographer.visible = true
         canvas_set_opacity(0xff);
      else
         moongate.image = moongate_tbl[1][i % 8]
         if i >= 38 and i <= 42 then
            photographer.image = g_img_tbl[4][4]
         else
            photographer.image = g_img_tbl[4][3]
         end
         if i == 41 then
            text.text = text_tbl[7]
         end

         poll_for_key_or_button(3)
         if should_exit() then return end
      end
   end

   --mars dust storm

   moongate.visible = false
   group.visible = false
   photographer.visible = false

   bg.image = g_img_tbl[5]

   local sand = sprite_new(nil, 240, 140, true)
   local tree = sprite_new(nil, 0, 140, true)

   local dust = create_sprite(g_img_tbl[6], 0, 24)
   local dust1 = create_sprite(g_img_tbl[6], 0, 24)
   local dust2 = create_sprite(g_img_tbl[6], 0, 24)
   local dust3 = create_sprite(g_img_tbl[6], 0, 24)
   local dust4 = create_sprite(g_img_tbl[6], 0, 24)

   dust.visible = true
   dust1.visible = true
   dust2.visible = true
   dust3.visible = true
   dust4.visible = true

   text.text = text_tbl[8]

   for i=0,63 do
      if math.floor(i / 8) > 4 then
         sand.image = g_img_tbl[7][4]
      else
         sand.image = g_img_tbl[7][math.floor(i / 8)]
      end
      if i < 56 then
         if i - 50 < 0 then
            tree.image = g_img_tbl[8][0]
         else
            tree.image = g_img_tbl[8][i - 50]
         end
      else
         tree.visible = false
      end

      dust.x = 340 - (i * 10) - math.random(0, 19)
      dust.y = 24 - math.random(0, 9)

      dust1.x = 420 - (i * 20) - math.random(0, 19)
      dust1.y = 24 - math.random(0, 9)

      dust2.x = 500 - (i * 30) - math.random(0, 19)
      dust2.y = 24 - math.random(0, 9)

      dust3.x = 580 - (i * 30) - math.random(0, 19)
      dust3.y = 24 - math.random(0, 9)

      dust4.x = 660 - (i * 20) - math.random(0, 19)
      dust4.y = 24 - math.random(0, 9)

      if i == 62 then
         text.text = text_tbl[9]
      end

      poll_for_key_or_button(2)
      if should_exit() then return end
   end

   poll_for_key_or_button(150)
   if should_exit() then return end

   about_martian_dreams()
end


play()
