local loc = player_get_location()
local caster = magic_get_caster()
local x = loc.x - 5
local y = loc.y - 5
local z = loc.z

magic_casting_fade_effect(caster)

for x = loc.x - 5,loc.x + 5 do
	for y = loc.y - 5,loc.y + 5 do
		local actor = map_get_actor(x, y, z)

		if actor ~= nil then
			local actor_type = actor_tbl[actor.obj_n]

			if actor.align == ALIGNMENT_EVIL and (actor_type == nil or actor_type[12] == 0) then
				if actor_int_check(actor, caster) == false then

					local old_tile_num = actor.tile_num
					actor.base_obj_n = 0x177 --convert actor into slime
					local new_tile_num = actor.tile_num
					actor.visible = false
					--FIXME need to fade all at once.
					fade_tile(x, y, z, old_tile_num, new_tile_num)
					actor.visible = true
					if actor.temp == true then
						--remove inventory
						local obj
						for obj in actor_inventory(actor) do
							Actor.inv_remove_obj(actor, obj)
						end
					end
				end
			end
		end
	end
end
slime_update_frames()
