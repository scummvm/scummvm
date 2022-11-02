magic_casting_effect()
magic_casting_fade_effect()

local qty = math.random(1,10)

local obj = Obj.new(129,0,0,qty)

Obj.moveToInv(obj, magic_get_caster())

print("\nCreate " .. obj.qty .. " food.\n")
