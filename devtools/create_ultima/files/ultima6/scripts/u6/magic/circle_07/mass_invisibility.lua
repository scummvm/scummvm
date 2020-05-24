local caster = magic_get_caster()
local loc = select_location_with_projectile(0x188, caster)

if loc == nil then return end

local hit_items = explosion(0x18a,loc.x,loc.y)

local k,v

for k,v in pairs(hit_items) do
	if v.luatype == "actor" then
		if v.visible == true and actor_can_turn_invisible(v.obj_n) == true then
			fade_actor_blue(v) --FIXME we should fade out at the same time as fading blue.
			v.visible = false
		end
	end
end

magic_success()
