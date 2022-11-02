local actor = select_actor_with_projectile(0x17f)

if actor == nil then return end

if magic_remove_actor_enchantments(actor) == true then
	magic_success()
else
	magic_no_effect()
end
