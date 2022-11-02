set_g_armageddon(true)
magic_casting_effect()
local i
for i=1,0xff do
   local actor = Actor.get(i)
   if i ~= 1 and i ~= 5 and actor ~= nil and actor.alive then --don't kill the Avatar or Lord British.
   	Actor.kill(actor, false) --false = don't create body
   end
end
