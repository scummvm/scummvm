local caster = magic_get_caster()
local actor = select_actor()

magic_casting_fade_effect()

if actor == nil then magic_no_effect() return end

if spell_hit_actor(caster, actor, 50) == false then -- 50 = disable spell number
	magic_failed()
end
