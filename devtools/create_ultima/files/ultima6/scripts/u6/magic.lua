--io.stderr:write("Magic init\n");
magic_syllable_tbl = {a = "An", b = "Bet", c = "Corp", d = "Des", e = "Ex", f = "Flam", g = "Grav", h = "Hur",
                      i = "In", j = "Jux", k = "Kal", l = "Lor", m = "Mani", n = "Nox", o = "Ort", p = "Por",
                      q = "Quas", r = "Rel", s = "Sanct", t = "Tym", u = "Uus", v = "Vas", w = "Wis", x = "Xen",
                      y = "Ylem", z = "Zu"}
magic = {}
magic_invocations = {}

function magic_print_invocation_string(spell_num)
	local i
	local invocation = magic_spell_invocation(spell_num)

	for i = 1,#invocation do
		if i ~= 1 then
			print(" ")
		end
		print(magic_syllable_tbl[string.sub(invocation, i, i)])
	end
end

run_magic_script = function(invocation)
	local spell_num = magic_invocations[invocation]
    if spell_num == nil then
      --io.stderr:write("No magic script found for invocation \"" .. invocation .. "\"\n");
      return
    end

--io.stderr:write("Running script \"" .. magic_invocations[invocation].script .."\"\n");

    --run_script(magic_invocations[invocation].script)
    magic_cast_spell(spell_num, nil, nil)
    return
end

g_magic_target = nil
g_magic_caster = nil
g_magic_spell_num = nil

function magic_cast_spell(spell_num, caster, target)
	g_magic_target = target
	g_magic_caster = caster
	g_magic_spell_num = spell_num
	if magic[spell_num+1] ~= nil then
		run_script(magic[spell_num+1].script)
	end
	g_magic_caster = nil
	g_magic_target = nil
	if g_avatar_died == true then
		actor_avatar_death()
	end
end

function magic_spell_name(spell_num)
	if magic[spell_num+1] ~= nil then
		return magic[spell_num+1].name
	end

	return "Unknown"
end

function magic_spell_invocation(spell_num)
	if magic[spell_num+1] ~= nil then
		return magic[spell_num+1].invocation
	end

	return ""
end

function magic_get_spell_list()
	local list = {}
	local insert = table.insert
	local k,v
	for k,v in pairs(magic) do
			insert(list, v)
	end

	return list
end

magic_init = function(name, invocation, reagents, circle, num, script)
	local spell_num = (circle-1) * 16 + (num-1);
    local spell = {name=name,invocation=invocation,reagents=reagents,circle=circle,spell_num=spell_num,script=script}

    magic[spell_num+1] = spell
    magic_invocations[string.lower(invocation)] = spell_num

    --io.stderr:write("Init Magic: " .. name .. " I: " .. invocation .. "\n")
end

function select_location_with_prompt(prompt)
if g_magic_target ~= nil then return g_magic_target end

print(prompt)
return get_target()
end

function select_location()
	return select_location_with_prompt("Location: ")
end

select_actor = function()
	if g_magic_target ~= nil then return map_get_actor(g_magic_target) end

	print("On whom: ");

	local loc = get_target()
	local actor

	if loc ~= nil then
		actor = map_get_actor(loc)
	end

	if actor == nil then
		print("nothing\n");
	else
		print(actor.name.."\n");
		if out_of_spell_range(actor.x, actor.y) then return end
	end

	return actor
end

select_obj = function()
	if g_magic_target ~= nil then return map_get_obj(g_magic_target) end

	print("On what: ");

	local obj = get_obj()

	if obj == nil then
		print("nothing\n");
	else
		print(obj.name .. "\n");
		if obj.on_map and out_of_spell_range(obj.x, obj.y) then return end
	end

	return obj
end

function select_actor_or_obj()
	if g_magic_target ~= nil then return map_get_obj(g_magic_target) end

	local is_player = caster_is_player()

	local loc = select_location_with_prompt("On Whom: ")
	local actor = map_get_actor(loc)
	local obj
	if actor == nil then
		obj = map_get_obj(loc)
		actor = obj
		if is_player == true then
			if obj ~= nil then
				print(obj.name.."\n")
			else
				print("nothing\n")
			end
		end
	elseif is_player == true then
		print(actor.name.."\n")
	end

	magic_casting_fade_effect(caster)
	if loc == nil or actor == nil then magic_no_effect() return end
	if (obj == nil or obj.on_map) and out_of_spell_range(loc.x, loc.y) then return end

	return actor
end

function select_actor_with_projectile(projectile_tile, caster)

	if caster == nil then caster = magic_get_caster() end

	local is_player = caster_is_player()

	local loc = select_location_with_prompt("On Whom: ")
	local actor = map_get_actor(loc)
	if actor == nil then
		local obj = map_get_obj(loc)
		if is_player == true then
			if obj ~= nil then
				print(obj.name.."\n")
			else
				print("nothing\n")
			end
		end
	elseif is_player == true then
		print(actor.name.."\n")
	end

	magic_casting_fade_effect(caster)
	if loc == nil then magic_no_effect() return end
	if out_of_spell_range(loc.x, loc.y) then return end

	local hit_x, hit_y =  map_line_hit_check(caster.x, caster.y, loc.x, loc.y, loc.z)
	projectile(projectile_tile, caster.x, caster.y, hit_x, hit_y, 4)

	if hit_x ~= loc.x or hit_y ~= loc.y then magic_blocked() return end

	if actor == nil then magic_no_effect() return end

	return actor
end

function select_actor_or_obj_with_projectile(projectile_tile, caster)

	if caster == nil then caster = magic_get_caster() end

	local loc = select_location_with_prompt("On what: ")
	local item = map_get_actor(loc)
	if item == nil then
		item = map_get_obj(loc)
	end

	if item ~= nil then
		print(item.name)
	else
		print("nothing")
	end

	print("\n")

	magic_casting_fade_effect(caster)

	if loc == nil then magic_no_effect() return end
	if out_of_spell_range(loc.x, loc.y) then return end

	local hit_x, hit_y =  map_line_hit_check(caster.x, caster.y, loc.x, loc.y, loc.z)
	projectile(projectile_tile, caster.x, caster.y, hit_x, hit_y, 4)

	if hit_x ~= loc.x or hit_y ~= loc.y then magic_blocked() return end

	if item == nil then magic_no_effect() return end

	return item
end

function select_obj_with_projectile(projectile_tile, caster)

	if caster == nil then caster = magic_get_caster() end

	local loc = select_location_with_prompt("On What: ")
	local obj = map_get_obj(loc)
	if obj == nil then
		print("nothing\n");
	else
		print(obj.name.."\n");
	end

	magic_casting_fade_effect(caster)
	if loc == nil then magic_no_effect() return end
	if out_of_spell_range(loc.x, loc.y) then return end

	local hit_x, hit_y =  map_line_hit_check(caster.x, caster.y, loc.x, loc.y, loc.z)
	projectile(projectile_tile, caster.x, caster.y, hit_x, hit_y, 4)

	if hit_x ~= loc.x or hit_y ~= loc.y then magic_blocked() return end

	if obj == nil then magic_no_effect() return end

	return obj
end

function select_location_with_projectile(projectile_tile, caster)

if caster == nil then caster = magic_get_caster() end

local loc = select_location_with_prompt("Location: ")

magic_casting_fade_effect(caster)
if loc == nil then magic_no_effect() return end
if out_of_spell_range(loc.x, loc.y) then return end

local hit_x, hit_y =  map_line_hit_check(caster.x, caster.y, loc.x, loc.y, loc.z)
projectile(projectile_tile, caster.x, caster.y, hit_x, hit_y, 4)

if hit_x ~= loc.x or hit_y ~= loc.y then magic_blocked() return end

return loc
end

function select_spell()
	return get_spell()
end

function out_of_spell_range(target_x, target_y)
	if Actor.get_range(magic_get_caster(), target_x, target_y) > 7 then
		print("\nout of range\n")
		return true
	else
		return false
	end
end

function caster_get_location()
	 if g_magic_caster ~= nil then
	  	return {x = g_magic_caster.x, y = g_magic_caster.y, z = g_magic_caster.z}
	  end

	  return player_get_location()
end

function magic_get_caster()
	if g_magic_caster ~= nil then return g_magic_caster end

	return Actor.get_player_actor()
end

function caster_is_player()
	if g_magic_caster == nil then return true end

	return false
end

function magic_casting_effect()
	local magic_level = math.floor(g_magic_spell_num / 0x10);

	play_sfx(SFX_CASTING_MAGIC_P1 + magic_level, true)
	play_sfx(SFX_CASTING_MAGIC_P2 + magic_level)
	xor_effect(1700)
end

function magic_casting_fade_effect(caster)
	if caster == nil then caster = magic_get_caster() end

	Actor.black_fade_effect(caster, 12, 20) -- 12 = colour red. 20 = fade_speed
end

function magic_remove_actor_enchantments(actor)
	local success = false
	fade_actor_blue(actor)
	if actor.asleep == true then
		actor.asleep = false
		success = true
	end
	if actor.poisoned == true then
		actor.poisoned = false
		success = true
	end
	if actor.paralysed == true then
		actor.paralyzed = false
		success = true
	end

	if actor_remove_charm(actor) == true then
		success = true
	end

	return success
end


function magic_wind(tile_num, caster, target_x, target_y)

	local target_z = caster.z

	local index = ((caster.y - target_y + 5) * 11) + (caster.x - target_x + 5) + 1

	local offset_x_low = movement_offset_x_tbl[g_projectile_offset_tbl[1][index]+1]
	local offset_x_high = movement_offset_x_tbl[g_projectile_offset_tbl[2][index]+1]

	local offset_y_low = movement_offset_y_tbl[g_projectile_offset_tbl[1][index]+1]
	local offset_y_high = movement_offset_y_tbl[g_projectile_offset_tbl[2][index]+1]

	local targets = {
		{x=target_x,
		y=target_y,
		z=target_z},
		{x=target_x + offset_x_low,
		y=target_y + offset_y_low,
		z=target_z},
		{x=target_x + offset_x_low + offset_x_low,
		y=target_y + offset_y_low + offset_y_low,
		z=target_z},
		{x=target_x + offset_x_high,
		y=target_y + offset_y_high,
		z=target_z},
		{x=target_x + offset_x_high + offset_x_high,
		y=target_y + offset_y_high + offset_y_high,
		z=target_z}
	}

	return projectile_anim_multi(tile_num, caster.x, caster.y, targets, 3, 1, 0)
end

function magic_wind_spell(spell_num, tile_num)

	local caster = magic_get_caster()
	local loc = select_location()

	if loc == nil then return end

	print("\n")

	if loc.x == caster.x and loc.y == caster.y then return magic_no_effect() end

	local hit_items = magic_wind(tile_num, caster, loc.x, loc.y)

	local k, v
	for k,v in pairs(hit_items) do
		if v.luatype == "actor" and v.actor_num ~= caster.actor_num then
			if spell_num == 83 then --flame wind
				spell_take_fire_dmg(caster, v)

			elseif spell_num == 87 then --poison wind
				spell_poison_actor(caster, v)

			elseif spell_num == 98 then --energy wind
				local exp = actor_hit(v, math.random(1, 0x1e))
				if exp ~= 0 then
					caster.exp = caster.exp + exp
				end

				actor_yell_for_help(caster, v, 1)
				actor_hit_msg(v)

			elseif spell_num == 113 then --death wind
				spell_kill_actor(caster, v)
			end
			if g_avatar_died == true then
				break -- don't keep casting once Avatar is dead
			end
		end
	end
end

function magic_success()
	if caster_is_player() then
		print("\nSuccess\n")
		play_sfx(SFX_SUCCESS)
	end
end

function magic_no_effect()
	if caster_is_player() then
		print("\nNo effect\n")
		play_sfx(SFX_FAILURE)
	end
end

function magic_blocked()
	if caster_is_player() then
		print("\nBlocked!\n")
		play_sfx(SFX_FAILURE)
	end
end

function magic_failed()
	if caster_is_player() then
		print("\nFailed\n")
		play_sfx(SFX_FAILURE)
	end
end

function magic_not_possible()
	if caster_is_player() then
		print("\nNot possible\n")
		play_sfx(SFX_FAILURE)
	end
end

do
local init
--name, cast directly keys, reagents, level, level index, spell file
--MANDRAKE_ROOT 0x01, NIGHTSHADE 0x02, BLACK_PEARL 0x04, BLOOD_MOSS 0x08, SPIDER_SILK 0x10, GARLIC 0x20, GINSENG 0x40, SULFUROUS_ASH 0x80
--add reagent numbers together for reagents argument
magic_init("Create Food", "imy", 0x61, 1, 1, "u6/magic/circle_01/create_food.lua");
magic_init("Detect Magic", "wo", 0x82, 1, 2, "u6/magic/circle_01/detect_magic.lua");
magic_init("Detect Trap", "wj", 0x82, 1, 3, "u6/magic/circle_01/detect_trap.lua");
magic_init("Dispel Magic", "ajo", 0x60, 1, 4, "u6/magic/circle_01/dispel_magic.lua");
magic_init("Douse", "af", 0x24, 1, 5, "u6/magic/circle_01/douse.lua");
magic_init("Harm", "am", 0x12, 1, 6, "u6/magic/circle_01/harm.lua");
magic_init("Heal", "im", 0x50, 1, 7, "u6/magic/circle_01/heal.lua");
magic_init("Help", "kl", 0x00, 1, 8, "u6/magic/circle_01/help.lua");
magic_init("Ignite", "if", 0x84, 1, 9, "u6/magic/circle_01/ignite.lua");
magic_init("Light", "il", 0x80, 1, 10, "u6/magic/circle_01/light.lua");

magic_init("Infravision", "ql", 0x82, 2, 1, "u6/magic/circle_02/infravision.lua");
magic_init("Magic Arrow", "oj", 0x84, 2, 2, "u6/magic/circle_02/magic_arrow.lua");
magic_init("Poison", "inp", 0x0e, 2, 3, "u6/magic/circle_02/poison.lua");
magic_init("Reappear", "iy", 0x1c, 2, 4, "u6/magic/circle_02/reappear.lua");
magic_init("Sleep", "iz", 0x16, 2, 5, "u6/magic/circle_02/sleep.lua");
magic_init("Telekinesis", "opy", 0x0d, 2, 6, "u6/magic/circle_02/telekinesis.lua");
magic_init("Trap", "ij", 0x12, 2, 7, "u6/magic/circle_02/trap.lua");
magic_init("Unlock Magic", "ep", 0x88, 2, 8, "u6/magic/circle_02/unlock_magic.lua");
magic_init("Untrap", "aj", 0x88, 2, 9, "u6/magic/circle_02/untrap.lua");
magic_init("Vanish", "ay", 0x2c, 2, 10, "u6/magic/circle_02/vanish.lua");

magic_init("Curse", "as", 0xa2, 3, 1, "u6/magic/circle_03/curse.lua");
magic_init("Dispel Field", "ag", 0x84, 3, 2, "u6/magic/circle_03/dispel_field.lua");
magic_init("Fireball", "pf", 0x84, 3, 3, "u6/magic/circle_03/fireball.lua");
magic_init("Great Light", "vl", 0x81, 3, 4, "u6/magic/circle_03/great_light.lua");
magic_init("Lock", "ap", 0xa8, 3, 5, "u6/magic/circle_03/magic_lock.lua");
magic_init("Mass Awaken", "avz", 0x60, 3, 6, "u6/magic/circle_03/mass_awaken.lua");
magic_init("Mass Sleep", "vz", 0x52, 3, 7, "u6/magic/circle_03/mass_sleep.lua");
magic_init("Peer", "vwy", 0x03, 3, 8, "u6/magic/circle_03/peer.lua");
magic_init("Protection", "is", 0xe0, 3, 9, "u6/magic/circle_03/protection.lua");
magic_init("Repel Undead", "axc", 0xa0, 3, 10, "u6/magic/circle_03/repel_undead.lua");

magic_init("Animate", "oy", 0x89, 4, 1, "u6/magic/circle_04/animate.lua");
magic_init("Conjure", "kx", 0x11, 4, 2, "u6/magic/circle_04/conjure.lua");
magic_init("Disable", "avm", 0x13, 4, 3, "u6/magic/circle_04/disable.lua");
magic_init("Fire Field", "ifg", 0x94, 4, 4, "u6/magic/circle_04/fire_field.lua");
magic_init("Great Heal", "vm", 0x51, 4, 5, "u6/magic/circle_04/great_heal.lua");
magic_init("Locate", "iw", 0x02, 4, 6, "u6/magic/circle_04/locate.lua");
magic_init("Mass Dispel", "vajo", 0x60, 4, 7, "u6/magic/circle_04/mass_dispel.lua"); -- The original engine uses 0x16
magic_init("Poison Field", "ing", 0x16, 4, 8, "u6/magic/circle_04/poison_field.lua"); -- The original engine uses 0x54
magic_init("Sleep Field", "izg", 0x54, 4, 9, "u6/magic/circle_04/sleep_field.lua"); -- The original engine uses 0x60
magic_init("Wind Change", "rh", 0x88, 4, 10, "u6/magic/circle_04/wind_change.lua");

magic_init("Energy Field", "isg", 0x15, 5, 1, "u6/magic/circle_05/energy_field.lua");
magic_init("Explosion", "vpf", 0x8d, 5, 2, "u6/magic/circle_05/explosion.lua");
magic_init("Insect Swarm", "kbx", 0x98, 5, 3, "u6/magic/circle_05/insect_swarm.lua");
magic_init("Invisibility", "sl", 0x0a, 5, 4, "u6/magic/circle_05/invisibility.lua");
magic_init("Lightning", "og", 0x85, 5, 5, "u6/magic/circle_05/lightning.lua");
magic_init("Paralyze", "axp", 0x96, 5, 6, "u6/magic/circle_05/paralyze.lua");
magic_init("Pickpocket", "py", 0x1a, 5, 7, "u6/magic/circle_05/pickpocket.lua");
magic_init("Reveal", "asl", 0x13, 5, 8, "u6/magic/circle_05/reveal.lua");
magic_init("Seance", "kmc", 0x9b, 5, 9, "u6/magic/circle_05/seance.lua");
magic_init("X-ray", "wy", 0x81, 5, 10, "u6/magic/circle_05/xray.lua");

magic_init("Charm", "axe", 0x16, 6, 1, "u6/magic/circle_06/charm.lua");
magic_init("Clone", "iqx", 0xdb, 6, 2, "u6/magic/circle_06/clone.lua");
magic_init("Confuse", "vq", 0x03, 6, 3, "u6/magic/circle_06/confuse.lua");
magic_init("Flame Wind", "fh", 0x89, 6, 4, "u6/magic/circle_06/flame_wind.lua");
magic_init("Hail Storm", "kdy", 0x0d, 6, 5, "u6/magic/circle_06/hail_storm.lua");
magic_init("Mass Protect", "vis", 0xe1, 6, 6, "u6/magic/circle_06/mass_protect.lua");
magic_init("Negate Magic", "ao", 0xa1, 6, 7, "u6/magic/circle_06/negate_magic.lua");
magic_init("Poison Wind", "nh", 0x8a, 6, 8, "u6/magic/circle_06/poison_wind.lua");
magic_init("Replicate", "iqy", 0xda, 6, 9, "u6/magic/circle_06/replicate.lua");
magic_init("Web", "idp", 0x10, 6, 10, "u6/magic/circle_06/web.lua");

magic_init("Chain Bolt", "vog", 0x8d, 7, 1, "u6/magic/circle_07/chain_bolt.lua");
magic_init("Enchant", "ioy", 0x91, 7, 2, "u6/magic/circle_07/enchant.lua");
magic_init("Energy Wind", "gh", 0x8b, 7, 3, "u6/magic/circle_07/energy_wind.lua");
magic_init("Fear", "qc", 0x23, 7, 4, "u6/magic/circle_07/fear.lua");
magic_init("Gate Travel", "vrp", 0x85, 7, 5, "u6/magic/circle_07/gate_travel.lua");
magic_init("Kill", "ic", 0x86, 7, 6, "u6/magic/circle_07/kill.lua");
magic_init("Mass Curse", "vas", 0xa3, 7, 7, "u6/magic/circle_07/mass_curse.lua");
magic_init("Mass Invis", "vsl", 0x0f, 7, 8, "u6/magic/circle_07/mass_invisibility.lua");
magic_init("Wing Strike", "kox", 0x99, 7, 9, "u6/magic/circle_07/wing_strike.lua");
magic_init("Wizard Eye", "pow", 0x9f, 7, 10, "u6/magic/circle_07/wizard_eye.lua");

magic_init("Armageddon", "vcbm", 0x00, 8, 1, "u6/magic/circle_08/armageddon.lua");
magic_init("Death Wind", "ch", 0x8b, 8, 2, "u6/magic/circle_08/death_wind.lua");
magic_init("Eclipse", "val", 0xab, 8, 3, "u6/magic/circle_08/eclipse.lua");
magic_init("Mass Charm", "vaxe", 0x17, 8, 4, "u6/magic/circle_08/mass_charm.lua");
magic_init("Mass Kill", "vc", 0x87, 8, 5, "u6/magic/circle_08/mass_kill.lua");
magic_init("Resurrect", "imc", 0xf9, 8, 6, "u6/magic/circle_08/resurrect.lua");
magic_init("Slime", "vrx", 0xb, 8, 7, "u6/magic/circle_08/slime.lua");
magic_init("Summon", "kxc", 0x39, 8, 8, "u6/magic/circle_08/summon.lua");
magic_init("Time Stop", "at", 0x29, 8, 9, "u6/magic/circle_08/time_stop.lua");
magic_init("Tremor", "vpy", 0x89, 8, 10, "u6/magic/circle_08/tremor.lua");

magic_init("Monster Invisible", "", 0x00, 8, 17, "u6/magic/circle_08/monster_invisible.lua");

end

magic_load = nil
