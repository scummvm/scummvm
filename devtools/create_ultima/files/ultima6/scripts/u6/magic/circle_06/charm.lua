local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17f, caster)

if actor == nil then return end

if spell_charm_actor(caster, actor) == false then
	magic_failed()
end
