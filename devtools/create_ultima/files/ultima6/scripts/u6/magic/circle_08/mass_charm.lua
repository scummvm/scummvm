local caster = magic_get_caster()
local loc = select_location_with_projectile(0x188, caster)

if loc == nil then return end

local hit_items = explosion(0x18a,loc.x,loc.y)

local k,v

for k,v in pairs(hit_items) do
	if v.luatype == "actor" then
		spell_charm_actor(caster, v)
	end
end

magic_success()
