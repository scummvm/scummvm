local caster = magic_get_caster()
if caster.visible == true then
	fade_actor_blue(caster)
	caster.visible = false
	magic_success()
else
	magic_no_effect()
end
