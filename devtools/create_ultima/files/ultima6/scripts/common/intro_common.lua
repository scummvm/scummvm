-- input value constants
MOUSE_CLICK       = 0
MOUSE_MOTION      = 1
SDLK_BACKSPACE    = 8
SDLK_RETURN       = 13
SDLK_ESCAPE       = 27
SDLK_SPACE        = 32
SDLK_a            = 97
SDLK_b            = 98
SDLK_c            = 99
SDLK_g            = 103
SDLK_i            = 105
SDLK_j            = 106
SDLK_q            = 113
SDLK_r            = 114
SDLK_s            = 115
SDLK_t            = 116
SDLK_KP2          = 258
SDLK_KP4          = 260
SDLK_KP6          = 262
SDLK_KP8          = 264
SDLK_KP_ENTER     = 271
SDLK_UP           = 82 + 1073741824
SDLK_DOWN         = 81 + 1073741824
SDLK_RIGHT        = 79 + 1073741824
SDLK_LEFT         = 80 + 1073741824


function trim_string(s)
   return s:match'^%s*(.*%S)' or ''
end

function wait_for_input()
   local input = nil
   while input == nil do
      canvas_update()
      input = input_poll()
      if input ~= nil then
         break
      end
   end

   return input
end

local g_should_exit = false

function clear_should_exit_flag()
   g_should_exit = false
end

function should_exit()
   return g_should_exit
end

function poll_for_input()
   input = input_poll()
   if input ~= nil then
      if input == SDLK_ESCAPE then
         g_should_exit = true
      end
   end

   if engine_should_quit() == 1 then
	   g_should_exit = true
   end

   return input
end

function poll_for_key_or_button(cycles)
   local input

   if engine_should_quit() == 1 then
	   g_should_exit = true
	   return true
   end

   if cycles == nil then
      input = input_poll()
      if input ~= nil then
         if input == SDLK_ESCAPE then
            g_should_exit = true
         end
         return true
      end
   else
      local i
      for i=0,cycles,1 do
         local input = input_poll()
         if input ~= nil then
            if input == SDLK_ESCAPE then
               g_should_exit = true
            end
            return true
         end
         if engine_should_quit() == 1 then
            g_should_exit = true
            return true
         end
         canvas_update()
      end
   end
   return false
end

function poll_for_esc(cycles)
   local input
   if cycles == nil then
      input = input_poll()
      if input ~= nil and input == SDLK_ESCAPE then
         return true
      end
      if engine_should_quit() == 1 then
         return true
      end
   else
      local i
      for i=0,cycles,1 do
         local input = input_poll()
         if input ~= nil and input == SDLK_ESCAPE then
            return true
         end
         if engine_should_quit() == 1 then
            return true
         end
         canvas_update()
      end
   end
   return false
end

function fade_in(speed)
   if speed == nil then
      speed = 3
   end

   local i
   for i=0x0,0xff,speed do
      canvas_set_opacity(i)
      canvas_update()

      if engine_should_quit() == 1 then
         return false
      end
   end

   return false
end

function fade_out(speed)
   if speed == nil then
      speed = 3
   end

   local i
   for i=0xff,0,-speed do
      canvas_set_opacity(i)
      canvas_update()

      if engine_should_quit() == 1 then
         return false
      end
   end

   return false
end

function update_players(players, img_tbl)
local rand = math.random

   players[1].image = img_tbl[1][rand(0,12)]
   players[2].image = img_tbl[2][rand(0,8)]
   players[3].image = img_tbl[3][rand(0,2)]
   players[4].image = img_tbl[4][rand(0,6)]
   players[5].image = img_tbl[5][rand(0,4)]
   players[6].image = img_tbl[6][rand(0,2)]
   players[7].image = img_tbl[7][rand(0,4)]
   players[8].image = img_tbl[8][rand(0,4)]
   players[9].image = img_tbl[9][rand(0,3)]

end

function create_player_sprite(image, x, y)
   local sprite = sprite_new(image, x, y, true)
   sprite.clip_x = 0
   sprite.clip_y = 0
   sprite.clip_w = 320
   sprite.clip_h = 152

   return sprite
end

function create_sprite(image, x, y)
   local sprite = sprite_new(image, x, y, true)
   sprite.clip_x = 0
   sprite.clip_y = 24
   sprite.clip_w = 320
   sprite.clip_h = 128
   return sprite
end

function create_firework(img_tbl)
   local rand = math.random
   local colour = rand(0,2)
   local exp = {create_sprite(img_tbl[11][8*colour], rand(0,319), rand(0,127)), colour, 0}
   return exp
end

function fireworks_update(exp_tbl, img_tbl)
   local exp_finished = 0
   local k,v
   for k,v in pairs(exp_tbl) do
      if v[3] == 7 then
         v[1].visible = false
         table.remove(exp_tbl, k)
         exp_finished = exp_finished + 1
      else
         v[3] = v[3] + 1
         v[1].image = img_tbl[11][8*v[2]+v[3]]
      end
   end

   return exp_finished
end

function fireworks(img_tbl, logo)
   local rand = math.random
   local exp_tbl = {}
   local exp_count = 0
   local i
   for i=0,125 do
      if exp_count < 5 then
         if rand(0,5) == 0 then
            local exp = create_firework(img_tbl)
            table.insert(exp_tbl, exp)
            exp_count = exp_count + 1
            sprite_move_to_front(logo)
            play_sfx(12, false)
         end
      end
      local num_finished = fireworks_update(exp_tbl, img_tbl)
      exp_count = exp_count - num_finished
      poll_for_esc(1)
   end

   --wait for remaining explosions to finish
   while exp_count > 0 do
      local num_finished = fireworks_update(exp_tbl, img_tbl)
      exp_count = exp_count - num_finished
      poll_for_esc(1)
   end

   poll_for_esc(10)

   -- create final 30 explosions.
   for i=1,30 do
      local exp = create_firework(img_tbl)
      table.insert(exp_tbl, exp)
      exp_count = exp_count + 1
   end

   while exp_count > 0 do
      local num_finished = fireworks_update(exp_tbl, img_tbl)
      exp_count = exp_count - num_finished
      poll_for_esc(1)
   end

end

function display_image_table(img_tbl, x, y)
   if x == nil then
      x = 160
   end

   if y == nil then
      y = 100
   end

   local sprite = sprite_new(nil, x, y, true)

   local text_sprite = sprite_new(nil, 100, 180, true)

   local i = 0
   for k,v in pairs(img_tbl) do
      if type(v) == "table" then
         local j = 0
         for l,m in pairs(v) do

            local img = image_new(50,20)
            text_sprite.image = img
            image_print(img, "("..k..","..l..")", 0, 50, 0, 8, 0x6)
            sprite.image = m
            wait_for_input()
            j = j + 1
         end
      else
         local img = image_new(50,20)
         text_sprite.image = img
         image_print(img, "("..k..",x)", 0, 50, 0, 8, 0x6)
         sprite.image = v
         wait_for_input()
      end
      i = i + 1
   end
end

function about_martian_dreams()
   canvas_hide_all_sprites()
   local bg = sprite_new(image_load("mars.lzc", 0), 0, 24, true)

   local text_tbl = text_load("scenetxt.lzc", 4)
   music_play("mdd_mus.lzc", 8)

   local sprites = {}
   local i
   for i=0,81 do
      local s = sprite_new(nil, 11, 153 + i * 14, true)
      s.text_color = 6
      s.text = text_tbl[i]
      table.insert(sprites, s)

      s = sprite_new(nil, 12, 152 + i * 14, true)
      s.text_color = 14
      s.text = text_tbl[i]
      table.insert(sprites, s)
   end

   --black bars for the top and bottom of the screen.
   --These hide the text as it is scrolling in and out.
   sprite_new(image_new(220, 24, 0), 0, 0, true)
   sprite_new(image_new(220, 48, 0), 0, 152, true)

   --scroll the text up the screen
   for i=0,90*14 do
      local j
      for j=1,82*2 do
         sprites[j].y = sprites[j].y - 1
      end
      poll_for_key_or_button(2)
      if should_exit() then
         fade_out()
         return
      end
   end
   music_stop()
   fade_out()
end
