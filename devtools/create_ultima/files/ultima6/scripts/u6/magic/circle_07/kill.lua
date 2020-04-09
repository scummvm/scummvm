local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x18c, caster)

if actor == nil then return end

local ret = spell_kill_actor(caster, actor)

if ret == 2 then
	magic_no_effect()
elseif ret == 1 then
	magic_failed()
end
