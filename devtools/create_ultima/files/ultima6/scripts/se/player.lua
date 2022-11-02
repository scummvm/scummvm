function player_pass()
   printl("PASS")
end


--returns true if the player can move to rel_x, rel_y
function player_before_move_action(rel_x, rel_y)
   return true
end

function player_post_move_action(did_move)
end

function player_attack()
   --FIXME
end
