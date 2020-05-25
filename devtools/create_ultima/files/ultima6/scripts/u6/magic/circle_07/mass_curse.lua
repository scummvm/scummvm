local caster = magic_get_caster()
local loc = select_location_with_projectile(0x18b, caster)

if loc == nil then return end

local hit_items = explosion(0x17f,loc.x,loc.y)

local k,v

for k,v in pairs(hit_items) do
	if v.luatype == "actor" then
		if is_god_mode_enabled() == false or v.in_party == false then
			v.cursed = true
		end
	end
end

magic_success()
