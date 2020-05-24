local obj = select_obj()

magic_casting_fade_effect()

if obj == nil then magic_no_effect() return end

local magical_tiles = {
[0x207] = 1,[0x20F] = 1,[0x216] = 1,[0x22F] = 1,[0x232] = 1,[0x235] = 1,[0x238] = 1,[0x239] = 1,[0x23D] = 1,
[0x248] = 1,[0x249] = 1,[0x24A] = 1,[0x24B] = 1,[0x24C] = 1,[0x24D] = 1,[0x24E] = 1,[0x24F] = 1,[0x251] = 1,
[0x252] = 1,[0x253] = 1,[0x255] = 1,[0x256] = 1,[0x257] = 1,[0x25A] = 1,[0x25B] = 1,[0x25C] = 1,[0x25D] = 1,
[0x25F] = 1,[0x26E] = 1,[0x2B6] = 1,[0x31E] = 1,[0x31F] = 1,[0x326] = 1,[0x368] = 1,[0x369] = 1,[0x36A] = 1,
[0x36B] = 1,[0x36C] = 1,[0x36D] = 1,[0x36E] = 1,[0x36F] = 1,[0x37D] = 1,[0x37E] = 1,[0x37F] = 1,[0x3A8] = 1,
[0x3A9] = 1,[0x3AA] = 1,[0x3AB] = 1,[0x3AC] = 1,[0x3AD] = 1,[0x3AE] = 1,[0x3AF] = 1,[0x3FD] = 1,[0x3FE] = 1,
[0x40C] = 1,[0x40D] = 1,[0x40E] = 1,[0x40F] = 1,[0x41C] = 1,[0x41D] = 1,[0x41E] = 1,[0x41F] = 1,[0x42C] = 1,
[0x42D] = 1,[0x42E] = 1,[0x42F] = 1,[0x43C] = 1,[0x43D] = 1,[0x43E] = 1,[0x43F] = 1,[0x484] = 1,[0x485] = 1,
[0x48E] = 1,[0x48F] = 1,[0x6D0] = 1,[0x6D1] = 1,[0x6D2] = 1,[0x6D3] = 1}

local potion_name = {
[0] = "an awaken",
[1] = "a cure",
[2] = "a heal",
[3] = "a poison",
[4] = "a sleep",
[5] = "a protection",
[6] = "an invisibility",
[7] = "an xray vision",
[8] = "an unknown"
}

fade_obj_blue(obj)

if magical_tiles[obj.tile_num] ~= nil then
	--magical object.
	if obj.obj_n == 0x113 then
		local frame_n = obj.frame_n
		if frame_n > 8 then
			frame_n = 8
		end
		print("\nIt's " .. potion_name[frame_n] .. " potion.\n")
	else
		print("\nIt's magical.\n")
	end
else
	--not magical
	local has_charge = false
	local child
	for child in container_objs(obj) do  -- look through container for effect object.
		if child.obj_n == 0x150 then --charge
			has_charge = true
			print("\nIt shows a charge of " .. magic_spell_name(child.quality) .. ".\n")
			break
	  	end
	end

	if has_charge == false then
		print("\nIt's not magical.\n")
	end
end
