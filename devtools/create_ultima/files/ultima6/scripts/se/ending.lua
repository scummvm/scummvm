local lua_file = nil

lua_file = nuvie_load("common/intro_common.lua"); lua_file();

function play()
   local g_img_tbl = image_load_all("endgame.lzc")
   -- 0,0 Party
   -- 0,1 Lord British
   -- 0,2 Aiela
   -- 0,3 Tristia
   -- 0,4 Leaving Savage Empire

   local bg = sprite_new(g_img_tbl[0][0], 0, 0, true)
   canvas_set_palette("savage.pal", 1)
   canvas_set_opacity(0xff);
   mouse_cursor_visible(false)
   canvas_set_update_interval(25)

   local txt = sprite_new(nil, 28, 135, true)
   txt.text_color = 0
   txt.text = i18n("IMG1_TXT1_END")
   txt.text_align = 1
   fade_in()
   wait_for_input()
   txt.text = i18n("IMG1_TXT2_END")
   wait_for_input()
   txt.text = i18n("IMG1_TXT3_END")
   wait_for_input()

   bg.image = g_img_tbl[0][1]
   txt.text = i18n("IMG2_TXT1_END")
   wait_for_input()
   txt.text = i18n("IMG2_TXT2_END")
   wait_for_input()

-- TODO: Add Check for Love with Aiela
   bg.image = g_img_tbl[0][2]
   txt.text = i18n("IMG3_TXT1_END")
   wait_for_input()
   txt.text = i18n("IMG3_TXT2_END")
   wait_for_input()
   txt.text = i18n("IMG3_TXT3_END")
   wait_for_input()

-- TODO: Add Check for Love with Tristia
   bg.image = g_img_tbl[0][3]
   txt.text = i18n("IMG4_TXT1_END")
   wait_for_input()
   txt.text = i18n("IMG4_TXT2_END")
   wait_for_input()

   bg.image = g_img_tbl[0][4]
   txt.text = i18n("IMG5_TXT1_END")
   wait_for_input()
   txt.text = i18n("IMG5_TXT2_END")
   wait_for_input()
   txt.text = i18n("IMG5_TXT3_END")
   wait_for_input()
   txt.text = i18n("IMG5_TXT4_END")
   wait_for_input()

   bg.image = g_img_tbl[0][1]
   txt.text = i18n("IMG2_TXT3_END")
-- TODO - Add Years, Months, Days to String
   wait_for_input()
end

play()
