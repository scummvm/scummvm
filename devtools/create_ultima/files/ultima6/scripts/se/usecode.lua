local USE_EVENT_USE = 0x01

-- Returns true if the actor has the item or a container with the item
function actor_has_item(actor, obj)
    local tobj = obj
    while tobj.in_container do
        tobj = tobj.parent
    end
    if tobj.on_map == false and
       tobj.parent.luatype == "actor" and
       tobj.parent.actor_num == actor.actor_num then
           return true
    end
    return false
end

-- Check Reachable
--   A) Next to actor passed in
--   B) In inventory of any party member
-- Return actor that can reach object or nil
-- Optionally automatically print "Out of Range!" message
function check_can_reach(obj, actor, print_msg)
    if math.abs(obj.x - actor.x) <= 1 and
       math.abs(obj.y - actor.y) <= 1 and
       obj.z == actor.z then
        return actor
    end

    for party_member in party_members() do
        if actor_has_item(party_member, obj) == true then
            return party_member
        end
    end

    if print_msg then
        printl("OUT_OF_RANGE")
    end
    return nil
end

-- delete an object from inventory or remove it from the map
function delete_or_remove_object(actor, obj, qty)
    -- FIXME: Remove the exact object from the Actor
    if Actor.inv_has_obj_n(actor, obj.obj_n) then
        if qty ~= nil then
            Actor.inv_remove_obj_qty(actor, obj.obj_n, qty)
        else
            Actor.inv_remove_obj(actor, obj)
        end
    -- Remove the object from the map
    else
        if qty ~= nil then
            map_remove_obj(obj, qty)
        else
            map_remove_obj(obj)
        end
    end
end

-- create and add item to actor, qty if stackable, drop if required to create the item,
--    obj_n: Item Code to Create
--    actor: actor to get the object
--    qty: how many (or nil)
--    drop_if_fail: if true, object gets put on the map on failure
--    print_msg: if true, prints the carry too much message on failure
function add_item_to_actor(obj_n, actor, qty, drop_if_fail, print_msg)
    local new_obj = Obj.new(obj_n)
    if qty ~= nil then
        new_obj.qty = qty
    else
        new_obj.qty = 1
    end

    if Actor.can_carry_obj(actor, new_obj) then
        if qty ~= nil then
            Actor.inv_add_obj(actor, new_obj, STACK_OBJECT_QTY)
        else
            Actor.inv_add_obj(actor, new_obj, false)
        end
    else
        if print_msg then
            printl("CARRY_TOO_MUCH")
        end
        return false
    end
    return true
end

function use_corn_stalk(obj, actor)
    local use_actor = check_can_reach(obj, actor, true)
    if use_actor == nil then
        return
    end

    if add_item_to_actor(108, use_actor, 1, false, true) == true then
        printl("GOT_CORN")
    end
end

function use_tree(obj, actor)
    local use_actor = check_can_reach(obj, actor, true)
    if use_actor == nil then
        return
    end

    if add_item_to_actor(206, use_actor, 1, false, true) == true then
        printl("USE_TREE")
    end
end

function use_yucca_plant(obj, actor)
    local use_actor = check_can_reach(obj, actor, true)
    if use_actor == nil then
        return
    end

    if add_item_to_actor(210, use_actor, 1, false, true) == true then
        printl("USE_YUCCA_PLANT")
    end
end

function use_clay(obj, actor)
    local use_actor = check_can_reach(obj, actor, true)
    if use_actor == nil then
        return
    end

    -- delete clay
    local location_code = delete_or_remove_object(use_actor, obj, nil)
    if add_item_to_actor(132, use_actor, nil, false, false) == true then
        printl("USE_CLAY")
    end
    -- TODO: IF ADD FAILED, PUT CLAY BACK WHERE IT WAS
end

function use_fishing_pole(obj, actor)
    local use_actor = check_can_reach(obj, actor, true)
    if use_actor == nil then
        return
    end

    -- Check Deep Water
--    if <<Standing Next to Deep Water>> then
    printl("USE_FISHING_POLE_NO_WATER")
--    end
    -- Check Success
--    if random() == SUCCESS then
        if add_item_to_actor(192, use_actor, nil, false, false) == true then
--        add_item without print
--        if SUCCESS then
              printl("USE_FISHING_POLE_SUCCESS")
        end
--    end
    printl("USE_FISHING_POLE_FAIL")
end

function use_digging_stick(obj, actor)
    local use_actor = check_can_reach(obj, actor, true)
    if use_actor == nil then
        return
    end

    -- Check Water
--    if <<Standing Next to Water>> then
            printl("USE_DIGGING_STICK_NO_WATER")
--    end
    if add_item_to_actor(192, use_actor, 1, false, true) == true then
        printl("USE_DIGGING_STICK")
    end
end

local usecode_table = {
[61]=use_corn_stalk,
[131]=use_digging_stick,
[192]=use_clay,
[212]=use_fishing_pole,
[5000]=use_tree,
[108]=use_tree,
[5001]=use_oven_or_fire,
[5002]=use_yucca_plant,
--[[
[62]=use_bean_stalk,
--[127]=use_bamboo_plant,			 -- confirmed
[132]=use_soft_clay_pot,         -- confirmed
[133]=use_fired_clay_pot,		 -- confirmed
[134]=use_cloth_strip,			 -- confirmed
[191]=use_tarred_cloth_strip,	 -- confirmed
[210]=use_flax,					 -- confirmed
]]--
--[[
[10]=use_magnesium_ribbon,		 -- confirmed
[12]=use_paddle,				 -- confirmed
[20]={["on"]=use_rope},
[25]={["on"]=use_cutting_tool},  -- confirmed (obsidian knife)
[29]=use_rock_hammer,			 -- confirmed (rock hammer)
[42]=use_vine,					 -- confirmed (vine)
[44]={["on"]=use_cutting_tool},  -- confirmed (knife)
[47]=use_turtle_bait,
[51]=use_camera,
[52]={["on"]=use_cutting_tool},  -- confirmed (scissors)
[54]={["on"]=use_chocolatl}, --Original required using totem on reagent
[55]={["on"]=use_pinde},	 	 -- confirmed
[56]={["on"]=use_yopo},			 -- confirmed
[59]=use_grinding_stone,		 -- confirmed (mortar)
[60]=use_grinding_stone,		 -- confirmed (grinding stone)
[63]=use_jug_of_platcha,
[64]=use_torch,
[75]=standing_torch,			 -- confirmed (almost certainly can't use)
[86]=use_heluzz,				 -- confirmed
[87]=use_motazz,				 -- confirmed
[88]=use_aphazz,				 -- confirmed
[93]=use_corn_meal,
[94]=use_fire_extinguisher,		 -- confirmed
[102]=use_tortilla,				 -- confirmed
[108]=use_corn,					 -- confirmed
[118]=use_fire_axe,				 -- confirmed
[119]=use_metal_hammer,			 -- confirmed
[128]=use_bamboo_pole,			 -- confirmed
[136]=use_metal_bucket,			 -- confirmed
[137]=use_grenade,				 -- confirmed
[153]=use_vine(corn_stalk)???,			 -- confirmed
[206]=use_tree_branch,			 -- confirmed
[208]=use_torch,
[240]=use_device,				 -- confirmed Explosive Device
[267]=use_device2				 -- confirmed Compass Device?  USE?
]]--
}


function has_usecode(obj, usecode_type)
    if usecode_type == USE_EVENT_USE and usecode_table[obj.obj_n] ~= nil then
        return true
    end

    return false
end

function use_obj(obj, actor)
    if type(usecode_table[obj.obj_n]) == "function" then
        local func = usecode_table[obj.obj_n]
        if func ~= nil then
            print("\n")
            func(obj, actor)
        end
    else
        use_obj_on(obj, actor, usecode_table[obj.obj_n])
    end
end

function move_obj(obj, rel_x, rel_y)
   return false
end

function is_ranged_select(operation)
   return false
end
