local caster = magic_get_caster()
local obj = select_obj()
if obj == nil then return end

if obj.obj_n ~= 0x4e then --staff
	magic_no_effect()
	return
end

print("with: ")
local spell_num = select_spell()

if spell_num ~= nil then
	if spell_num ~= 0x61 then --0x61 = enchant. Don't allow recursion on enchant spell. ;-)
		magic_print_invocation_string(spell_num)
		print("\n")
		local i = 0
		for child in container_objs(obj) do
			i = i + 1
		end

		if i >= 10 then
			print("\nIt's full\n")
			play_sfx(SFX_FAILURE)
			return
		end

		magic_casting_fade_effect(caster)
		local charge = Obj.new(336,0, spell_num)
		charge.invisible = true
		Obj.moveToCont(charge, obj)
	end
else
	print("none\n\n")
end
