local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x18b, caster)

if actor == nil then return end

if actor.visible == true and actor_can_turn_invisible(actor.obj_n) == true then
    fade_actor_blue(actor) --FIXME we should fade out at the same time as fading blue.
	actor.visible = false
	magic_success()
else
	magic_no_effect()
end
