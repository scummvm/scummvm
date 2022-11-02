local obj = select_actor_or_obj()

local obj_n = obj.obj_n
--          dead body         ghost             dead gargoyle     dead cyclops
if obj_n == 0x153 or obj_n == 0x160 or obj_n == 0x155 or obj_n == 0x154 then
	if obj.luatype == "actor" and obj.actor_num == 91 then
		Actor.set_talk_flag(obj, 7)
		actor_talk(obj)
	else
		Actor.talk(Actor.get(obj.quality))
	end
end
