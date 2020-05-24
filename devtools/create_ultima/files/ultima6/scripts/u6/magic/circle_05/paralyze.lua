local caster = magic_get_caster()
local actor = select_actor_with_projectile(0x17f, caster)

if actor == nil then return end
if is_god_mode_enabled() and actor.in_party then
	return
end
print("\n")

hit_anim(actor.x, actor.y)
actor.paralyzed = true
print(actor.name.." is paralyzed.\n")

if actor.in_party == true then
	party_update_leader()
end
