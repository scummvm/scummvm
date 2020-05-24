local PLAYER_CAN_MOVE = 0
local PLAYER_BLOCKED = 1
local PLAYER_FORCE_MOVE = 2

local map_entrance_tbl = {
    {x=0x43,  y=0x51,  z=0x1},
    {x=0x80,  y=0x8D,  z=0x0},
    {x=0x76,  y=0x0F2, z=0x3},
    {x=0x7A,  y=0x0C,  z=0x3},
    {x=0x9A,  y=0x41,  z=0x4},
    {x=0x2B2, y=0x1CC, z=0x0},
    {x=0x73,  y=0x40,  z=0x4},
    {x=0x29D, y=0x1CE, z=0x0},
    {x=0x0B3, y=0x0E1, z=0x4},
    {x=0x27B, y=0x1F1, z=0x0},
    {x=0x0C3, y=0x70,  z=0x1},
    {x=0x0CB, y=0x1D1, z=0x0},
    {x=0x24,  y=0x0F1, z=0x1},
    {x=0x31A, y=0x232, z=0x0},
    {x=0x5C,  y=0x0F1, z=0x1},
    {x=0x34C, y=0x25A, z=0x0},
    {x=0x7C,  y=0x28,  z=0x4},
    {x=0x13E, y=0x118, z=0x0},
    {x=0x0A,  y=0x61,  z=0x5},
    {x=0x3B7, y=0x1C4, z=0x0},
    {x=0x5C,  y=0x0B1, z=0x5},
    {x=0x3DC, y=0x1DD, z=0x0},
    {x=0x5E,  y=0x29,  z=0x4},
    {x=0x227, y=0x1ab, z=0x0}
}

local pod_teleport_tbl = {
    {x=0xCA,  y=0x295},
    {x=0x34D, y=0x295},
    {x=0x13A, y=0x17A},
    {x=0x2CD, y=0x1FA}
}

function player_pass()
    printl("PASS")
    update_conveyor_belt(true)
end

--returns true if the player can move to rel_x, rel_y
function player_before_move_action(rel_x, rel_y)
    if rel_x ~= 0 and rel_y ~= 0 then
        return PLAYER_CAN_MOVE
    end

    local player_loc = player_get_location()

    if rel_x == 0 and rel_y == 1 then
        local tile_num = map_get_tile_num(player_loc)
        -- Fall off cliff logic
        if tile_num >= 384 and tile_num <= 387 then
            return PLAYER_FORCE_MOVE
        end
    end

    local z = player_loc.z
    local x = wrap_coord(player_loc.x+rel_x,z)
    local y = wrap_coord(player_loc.y+rel_y,z)

    for obj in objs_at_loc(x, y, z) do
        local obj_n = obj.obj_n
        if obj_n == 268 then --wheelbarrow
        move_wheelbarrow(obj, rel_x, rel_y)
        if can_move_obj(obj, rel_x, rel_y) then
            obj.x = obj.x + rel_x
            obj.y = obj.y + rel_y
        end
        elseif obj_n == 410 and can_move_obj(obj, rel_x, rel_y) then --railcar
        move_rail_cart(obj, rel_x, rel_y)
        elseif obj_n == 441 then --assembled drill
        move_drill(obj, rel_x, rel_y) --update drill direction
        if can_move_drill(obj, rel_x, rel_y) then
            obj.x = obj.x + rel_x
            obj.y = obj.y + rel_y
        end
        end
    end

    return PLAYER_CAN_MOVE
end

function update_objects_around_party()
    local loc = player_get_location()
    for obj in find_obj_from_area(wrap_coord(loc.x - 5, loc.z), wrap_coord(loc.y - 5, loc.z), loc.z, 11, 11) do
        local obj_n = obj.obj_n
        if (obj_n == 227 and Actor.get_talk_flag(0x73, 2)) or --OBJ_DOOR3
                (obj_n == 179 and Actor.get_talk_flag(0x73, 4)) then --OBJ_CLOSED_DOOR
            if bit32.band(obj.quality, 0x80) == 0 then -- check if the door is stuck
                local base_frame = bit32.band(obj.frame_n, 2)
                local actor = map_get_actor(obj.xyz)
                if actor ~= nil then
                    if map_is_on_screen(obj.xyz) then
                        play_door_sfx()
                    end
                    obj.frame_n = base_frame + 9
                else
                    actor = map_get_actor(obj.x + movement_offset_x_tbl[base_frame + 1], obj.y + movement_offset_y_tbl[base_frame + 1], obj.z)
                    if actor == nil then
                        actor = map_get_actor(obj.x + movement_offset_x_tbl[base_frame + 4 + 1], obj.y + movement_offset_y_tbl[base_frame + 4 + 1], obj.z)
                    end

                    if actor ~= nil and map_is_on_screen(obj.xyz) then
                        play_door_sfx()
                        if obj.frame_n < 4 then
                            obj.frame_n = (base_frame) + 5
                        elseif obj.frame_n < 8 then
                            obj.frame_n = (base_frame) + 1
                        else
                            obj.frame_n = (base_frame) + 5
                        end
                    else
                        if obj.frame_n == 5 then
                            obj.frame_n = 1
                        elseif obj.frame_n == 7 then
                            obj.frame_n = 3
                        end

                    end
                end
            end
        elseif obj_n == 301 then --OBJ_REFLECTIVE_SURFACE
            if obj.frame_n < 3 then
                local actor = map_get_actor(obj.x, obj.y + 1, obj.z)
                if actor ~= nil then
                    local actor_num = actor.actor_num
                    if actor_num >= 0x70 or actor_num == 0x5d or (actor_num >= 0x6a and actor_num <= 0x6c) then
                        obj.frame_n = 0
                    elseif (actor_num < 0x20 and actor_num ~= 6)
                            or (actor_num >= 0x2a and actor_num <= 0x5c)
                            or actor_num == 0x60 or actor_num == 0x69 or actor_num == 0x6d then
                        obj.frame_n = 1
                    else
                        obj.frame_n = 2
                    end
                else
                    obj.frame_n = 0
                end
            end
        end

    end

end

function player_post_move_action(did_move)
    local player_loc = player_get_location()

    if did_move then
        if map_get_tile_num(player_loc, true) == 0x6f then
            printl("FASTER")
            --FIXME update falling brick animation speed here.
        end

        update_conveyor_belt(true)
        update_objects_around_party()

        for obj in objs_at_loc(player_loc) do
            local obj_n = obj.obj_n
            if (obj_n == 175 or obj_n == 163 or obj_n == 180 or obj_n == 178 or (obj_n == 197 and obj.frame_n == 3) or obj_n == 210) then
                if  player_is_in_solo_mode() then
                    if obj_n == 163 or obj_n == 178 then
                        printl("YOU_MUST_BE_IN_PARTY_MODE_TO_LEAVE")
                    else
                        printl("YOU_MUST_BE_IN_PARTY_MODE_TO_ENTER")
                    end
                    return
                end
                if obj_n == 175 then --Mine entry
                    for transfer_obj in objs_at_loc(player_loc.x, player_loc.y-1, player_loc.z) do
                        transfer_obj.x = map_entrance_tbl[obj.quality].x
                        transfer_obj.y = map_entrance_tbl[obj.quality].y-1
                        transfer_obj.z = map_entrance_tbl[obj.quality].z
                    end
                elseif obj_n == 163 then --Mine exit
                    for transfer_obj in objs_at_loc(player_loc.x, player_loc.y+1, player_loc.z) do
                        transfer_obj.x = map_entrance_tbl[obj.quality].x
                        transfer_obj.y = map_entrance_tbl[obj.quality].y+2
                        transfer_obj.z = map_entrance_tbl[obj.quality].z
                    end
                elseif obj_n == 197 then
                    if Actor.get_talk_flag(0x73, 4) then
                        if obj.quality == 0 then
                            printl("NOTHING_APPEARS_TO_HAPPEN")
                        elseif obj.quality <= 4 then
                            local pod_exit = pod_teleport_tbl[obj.quality]
                            advance_time(0)
                            party_use_entrance(player_loc.x, player_loc.y, player_loc.z, pod_exit.x, pod_exit.y, 0)
                        end
                    else
                        printl("THE_ALIEN_DEVICE_DOESNT_SEEM_TO_BE_FUNCTIONING")
                    end
                    return
                end
                advance_time(0)
                party_use_entrance(player_loc.x, player_loc.y, player_loc.z, map_entrance_tbl[obj.quality])
            elseif obj_n == 461 then --OBJ_DREAM_TELEPORTER
                --FIXME add logic here.
                local obelisk = map_get_obj(player_loc.x, player_loc.y - 1, player_loc.z, 292, true)
                if obelisk ~= nil then
                    --FIXME fade here.
                end
                local dream_actor = Actor.get(0)
                if bit32.band(obj.status, 0xe5) ~= 0xa5 or dream_actor.hp > 4 then
                    if bit32.band(obj.status, 0xe5) == 0 then
                        player_move(obj.quality, obj.qty, player_loc.z)
                        advance_time(0)
                    else
                        dreamworld_cleanup_state(obj)
                    end

                end
            elseif obj_n == 462 then --OBJ_DREAM_TELEPORTER1 Walk on walls object
                local dream_actor = Actor.get(0)
                if obj.quality == 0 then
                    dream_actor.frame_n = dream_actor.old_frame_n
                    dream_actor.obj_n = dream_actor.base_obj_n
                else
                    dream_actor.frame_n = 0
                    if player_get_gender() == 0 then
                        dream_actor.obj_n = 318
                    else
                        dream_actor.obj_n = 319
                    end
                end
            elseif obj_n == 465 then --direction control modifier
                --FIXME
            end
        end
    else
        actor_map_dmg(Actor.get_player_actor(), player_loc.x, player_loc.y, player_loc.z)
        play_md_sfx(0)
    end

end

function can_move_drill(drill, rel_x, rel_y)
    if can_move_obj(drill, rel_x, rel_y) then
        return true
    end

    local z = drill.z
    local x = wrap_coord(drill.x+rel_x,z)
    local y = wrap_coord(drill.y+rel_y,z)

    if map_get_obj(x, y, z, 175, true) == nil then --mine entrance
    return false
    end

    for obj in objs_at_loc(x, y, z) do
        if is_blood(obj.obj_n) == false then
            return false
        end
    end

    return true
end

local player_readied_weapons
function weapon_select()
    player_readied_weapons = {}
    local player = Actor.get_player_actor()
    for obj in actor_inventory(player) do
        if obj.readied and get_weapon_damage(obj) > 0 then
            player_readied_weapons[#player_readied_weapons+1] = obj
        end
    end

    if #player_readied_weapons == 0 then
        printl("ATTACK_WITH_BARE_HANDS")
        return player
    else
        local weapon = player_readied_weapons[1]
        table.remove(player_readied_weapons, 1)
        printfl("ATTACK_WITH_WEAPON", weapon.name)
        return weapon
    end
end

function select_next_weapon()
    if #player_readied_weapons == 0 then
        return nil
    end

    local weapon = player_readied_weapons[1]
    table.remove(player_readied_weapons, 1)
    if weapon ~= nil then
        print("\n")
        printfl("ATTACK_WITH_WEAPON", weapon.name)
    end
    return weapon
end

function select_attack_target()
    local target_loc = get_target()
    g_selected_obj = get_actor_or_obj_from_loc(target_loc)

    local name
    if g_selected_obj ~= nil then
        name = g_selected_obj.name
    else
        name = tile_get_description(map_get_tile_num(target_loc))
    end
    print(name..".\n")

    return target_loc
end

function player_attack()
    local weapon = weapon_select()

    repeat
        local target_loc = select_attack_target()
        if target_loc == nil then
            printl("WHAT")
            return
        end

        player_attack_with_weapon(weapon, target_loc)
        weapon = select_next_weapon()
    until weapon == nil
end

function player_attack_with_weapon(weapon, target_loc)
    local player = Actor.get_player_actor()
    local obj_n = weapon.obj_n

    if out_of_ammo(player, weapon, true) then
        return
    end

    if obj_n == 313 then --OBJ_M60_MACHINE_GUN
        --FIXME MACHINE GUN LOGIC HERE
    else
        local result = attack_target_with_weapon(player, target_loc.x, target_loc.y, weapon)
        if result == 2 then
            printl("OUT_OF_RANGE")
            play_md_sfx(5)
        elseif result == 3 then
            printl("THAT_IS_NOT_POSSIBLE")
            play_md_sfx(5)
        end

    end

end
