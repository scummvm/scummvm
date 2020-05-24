--Actor stats table
--[objnum] = {str,dex,int,hp,dmg,alignment,can talk,drops blood,?,?,?,lives in water,flies-immune to tremor,repel undead (not used anymore),poisonous,strength_based,double dmg from fire,immune to magic (fire only),immune to poison,undead and immune to death spells,immune to sleep spell,{spell table},{weapon table},{armor table},{treasure table},exp_related see actor_hit()}
actor_tbl = {
--carnivorous orchid
[282] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--serpent woman
[283] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--baby apatosaurus
[284] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--baby apatosaurus
[285] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--apatosaurus
[286] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--apatosaurus
[287] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--pteranodon
[288] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--allosaurus
[289] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--allosaurus
[290] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--wisp
[291] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--carnivorous orchid
[292] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--tyrannosaurus
[293] = {30, 28, 255, 3, 255, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--tyrannosaurus
[294] = {30, 28, 255, 3, 255, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--tyrannosaurus
[295] = {30, 28, 255, 3, 255, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--gorilla
[296] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--robosaurus
[297] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--robosaurus
[298] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--baby pteranodon
[299] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--plesiosaur
[300] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--argent sergeant
[301] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--turtle
[302] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--stegosaurus
[303] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--stegosaurus
[304] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--triceratops
[305] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--triceratops
[306] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--party member on turtle
[307] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--snake
[308] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--alphadon
[309] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--modern man
[310] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--professor
[311] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--reporter
[312] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--woman
[313] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--shaman
[314] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--chieftain
[315] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--man
[316] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--parrot
[317] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Sakkhra
[318] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--automaton
[319] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--man
[320] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--woman
[321] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--noble
[322] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--myrmidex drone
[323] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--woman
[324] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--neanderthal
[325] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--man
[326] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--ankylosaurus
[327] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--Myrmidex queen
[328] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--dancer
[329] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--deinonychus
[330] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--sabretooth tiger
[331] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--sloth
[332] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--jaguar
[333] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--dimetrodon
[334] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--bear
[335] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--giant spider
[336] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0},
--eohippus
[337] = {1, 28, 4, 3, 1, ALIGNMENT_NEUTRAL, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}, {}, {}, 0}
}

function actor_map_dmg(actor, map_x, map_y, map_z)
	--FIXME
end

function actor_update_all()
	--FIXME
end

-- [objectnum] = range
   g_range_weapon_tbl = {
      [1] = 5, -- spear of shamap
      [21] = 4, --black staff --FIXME: only ranged weapon when Myrmidex are near
      [26] = 6, -- spear
      [27] = 4, -- throwing axe
      [31] = 5, -- bow
      [32] = 4, -- blowgun
      [37] = 7, -- atl atl better than spear (whole screen)
      [3] = 4, -- boomerang
      [44] = 4, -- knife
      [40] = 7, --modern rifle (whole screen)
      [50] = 7, --bamboo flintlock (whole screen)
      [138] = 6, --grenade
      [139] = 6, --lit grenade
      [240] = 6, --device
      [241] = 6 --activated device
   }

local projectile_weapon_tbl =
{
--obj_n = {tile_num, initial_tile_rotation, speed, rotation_amount}
[1] = {512, 45, 3, 0}, -- spear of shamap
[21] = {266, 90,4, 0}, --black staff --FIXME: rot, speed, amount
[26] = {547, 45,3, 0}, -- spear
[27] = {548, 0, 2, 10}, -- throwing axe
[31] = {566, 90,4, 0}, -- bow
[32] = {557, 90,4, 0}, --blowgun --CHECKME: rot, speed, amount
[37] = {547, 45,3, 0}, --atl atl
[39] = {560, 0, 4, 10}, -- boomerang
[44] = {565, 0, 2, 10}, -- knife
[40] = {562, 90,4, 0}, --modern rifle --FIXME: tile_num, rot, speed, amount
[50] = {562, 90,4, 0}, --bamboo flintlock --FIXME: tile_num, rot, speed, amount
[137] = {701, 90,4, 0}, --grenade --FIXME: tile_num, rot, speed, amount
[138] = {701, 90,4, 0}, --grenade --FIXME: tile_num, rot, speed, amount
[240] = {1029, 90,4, 0}, --device --FIXME: tile_num, rot, speed, amount
[241] = {1029, 90,4, 0} --activated device --FIXME: tile_num, rot, speed, amount
}

weapon_dmg_tbl = { --FIXME: all damage is made up
[1] = 15, -- spear of shamap (causes sleep)
[21] = 8, --black staff (extra damage vs Myrmidex)
[24] = 10, --club
[25] = 6, --obsidian knife
[26] = 15, --spear
[27] = 10, --throwing axe
[28] = 15, --axe
[29] = 15, --rock hammer
[31] = 15, --bow
[32] = 2, --blowgun (poisons)
[33] = 25, --obsidian sword
[34] = 20, --two-handed hammer
[37] = 25, --atl atl
[39] = 8, --boomerang
[40] = 20, --modern rifle
[44] = 10, --knife
[50] = 12, --bamboo flintlock
--[52] = 1, --scissors (says bare handed)
[94] = 15, --fire extingisher
[118] = 30, --fire axe
[119] = 15, --metal hammer
[128] = 4, --bamboo pole
--[131] = 1, --digging stick (says bare handed)
[137] = 30, --grenade
[138] = 30, --lit grenade
--[208] = 1, --torch (says bare handed)
[209] = 15, --lit torch
--[212] = 1, --fishing pole (says bare handed)
[240] = 30, --device (FIXME: explosive gas only hurts Myrmidex)
[241] = 30 --activated device (FIXME: explosive gas only hurts Myrmidex)
}

armour_tbl = --FIXME: all armor value is made up
{
[2] = 7, --shield of Krukk
[7] = 1, --cloth armor
[8] = 5, --leather armour
[9] = 2, --bark armor
[13] = 1, --bark shield
[14] = 3, --leather shield
[15] = 5, --stegosaurus shield
[67] = 10, --kotl shield (better vs Myrmidex?)
--[84] = 0, --tooth necklace
--[85] = 0, --jade necklace
--[135] = 0, --lei
--[600] = 0, --ring
}

function out_of_ammo(attacker, weapon, print_message)  -- untest function

	local weapon_obj_n = weapon.obj_n

	if weapon_obj_n == 32 and Actor.inv_has_obj_n(attacker, 36) == false then --blowgun, poisoned darts
		if(print_message) then
			print("Out of darts!\n")
		end
		return true
	end
	if weapon_obj_n == 31 and Actor.inv_has_obj_n(attacker, 45) == false then --bow, arrows
		if(print_message) then
			print("Out of arrows!\n")
		end
		return true
	end
	if weapon_obj_n == 37 and Actor.inv_has_obj_n(attacker, 26) == false then --atl atl, spear
		if(print_message) then
			print("Out of spears!\n")
		end
		return true
	end
	if weapon_obj_n == 40 and Actor.inv_has_obj_n(attacker, 41) == false then --modern rifle, rifle bullet
		if(print_message) then
			print("Out of ammunition!\n")
		end
		return true
	end
	if weapon_obj_n == 50 and weapon.frame_n == 0 then --unloaded bamboo flintlock (frame 1 is loaded)
		if(print_message) then
			print("Flintlock not loaded!\n")
		end
		return true
	end
	return false
end

function actor_update_all()
	dbg("actor_update_all()\n")
end

function advance_time(num_turns)
	--FIXME
	local minute = clock_get_minute()

	clock_inc(num_turns)

	if minute + num_turns >= 60 then
		update_actor_schedules()
	end
end

function can_get_obj_override(obj)
    return false
end

function actor_get_obj(actor, obj) -- FIXME need to limit inventory slots

	if obj.getable == false then
		print("\nNot possible.")
		return false
	end

	if Actor.can_carry_obj_weight(actor, obj) == false then
		print("\nThe total is too heavy.")
		return false
	end

--		print("\nYou are carrying too much already.");

	Obj.moveToInv(obj, actor.actor_num)
	subtract_movement_pts(actor, 3)

	return true
end

function player_post_move_action(did_move)
end
