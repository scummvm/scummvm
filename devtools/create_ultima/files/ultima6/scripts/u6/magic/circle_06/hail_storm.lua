local loc = select_location_with_prompt("Location: ")

magic_casting_fade_effect()

if loc == nil then magic_no_effect() return end

print("\n")

hail_storm_effect(loc)

print("\nSuccess\n")
