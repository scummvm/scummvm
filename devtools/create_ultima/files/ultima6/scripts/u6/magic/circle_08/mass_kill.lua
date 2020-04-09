local caster = magic_get_caster()
local loc = select_location_with_projectile(0x188, caster)

if loc == nil then return end

local hit_items = explosion(0x18c,loc.x,loc.y)

local k,v

for k,v in pairs(hit_items) do
	if v.luatype == "actor" then
		spell_kill_actor(caster, v)
	end
	if g_avatar_died == true then
		break -- don't keep casting once Avatar is dead
	end
end

explode_surrounding_objects(loc.x, loc.y, loc.z)
magic_success()
