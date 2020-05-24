local caster = magic_get_caster()

local x = caster.x
local y = caster.y
local z = caster.z
local loc_x = x
local loc_y = y

magic_casting_fade_effect(caster)

play_sfx(SFX_EARTH_QUAKE, false)
quake_start(1,1024)

local random = math.random

for x = x - 5,loc_x + 5 do
	for y = y - 5,loc_y + 5 do
		local actor = map_get_actor(x, y, z)

		if actor ~= nil then
			if actor.align == ALIGNMENT_EVIL then
				local actor_base = actor_tbl[actor.obj_n]
				if actor_base == nil or actor_base[13] == 0 then -- 13 is immune to tremor
					local exp = actor_hit(actor, random(1, 0x1e))
					if exp ~= 0 then
						caster.exp = caster.exp + exp
					end

					actor_hit_msg(actor)
					if g_avatar_died == true then
						break -- don't keep casting once Avatar is dead
					end
				end
			end
		end
	end
end

print("\nSuccess\n")
