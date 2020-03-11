print("To phase ")
local phase = input_select_integer(nil,true)


print("\n")

magic_casting_fade_effect()

if phase > 0 and phase < 9 and g_moonstone_loc_tbl[phase].x ~= 0 then
	--FIXME the original walks the party into an invisible gate to teleport.
	player_move(g_moonstone_loc_tbl[phase].x, g_moonstone_loc_tbl[phase].y, g_moonstone_loc_tbl[phase].z, true)
	fade_in()
else
	play_sfx(SFX_FAILURE)
end
