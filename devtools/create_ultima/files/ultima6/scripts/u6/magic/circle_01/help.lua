magic_casting_effect()
play_sfx(SFX_KAL_LOR, true)
if g_armageddon == false then
	party_resurrect_dead_members()
end
party_heal()
player_move(0x133, 0x160, 0, true)
get_LB_to_throne()

fade_in() --FIXME the original fades from current location. We fade from black.
