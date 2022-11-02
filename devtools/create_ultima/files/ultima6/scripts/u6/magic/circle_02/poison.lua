local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17c, caster)

if actor == nil then return end

local return_val = spell_poison_actor(caster, actor)

if return_val == 2 then
	magic_no_effect()
elseif return_val == 1 then
	magic_failed()
end
