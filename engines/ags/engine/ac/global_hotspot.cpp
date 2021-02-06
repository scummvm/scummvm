//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "ac/global_hotspot.h"
#include "ac/common.h"
#include "ac/common_defines.h"
#include "ac/characterinfo.h"
#include "ac/draw.h"
#include "ac/event.h"
#include "ac/gamesetupstruct.h"
#include "ac/global_character.h"
#include "ac/global_translation.h"
#include "ac/hotspot.h"
#include "ac/properties.h"
#include "ac/roomstatus.h"
#include "ac/string.h"
#include "debug/debug_log.h"
#include "game/roomstruct.h"
#include "script/script.h"

using namespace AGS::Common;

extern RoomStruct thisroom;
extern RoomStatus*croom;
extern CharacterInfo*playerchar;
extern GameSetupStruct game;


void DisableHotspot(int hsnum) {
    if ((hsnum<1) | (hsnum>=MAX_ROOM_HOTSPOTS))
        quit("!DisableHotspot: invalid hotspot specified");
    croom->hotspot_enabled[hsnum]=0;
    debug_script_log("Hotspot %d disabled", hsnum);
}

void EnableHotspot(int hsnum) {
    if ((hsnum<1) | (hsnum>=MAX_ROOM_HOTSPOTS))
        quit("!EnableHotspot: invalid hotspot specified");
    croom->hotspot_enabled[hsnum]=1;
    debug_script_log("Hotspot %d re-enabled", hsnum);
}

int GetHotspotPointX (int hotspot) {
    if ((hotspot < 0) || (hotspot >= MAX_ROOM_HOTSPOTS))
        quit("!GetHotspotPointX: invalid hotspot");

    if (thisroom.Hotspots[hotspot].WalkTo.X < 1)
        return -1;

    return thisroom.Hotspots[hotspot].WalkTo.X;
}

int GetHotspotPointY (int hotspot) {
    if ((hotspot < 0) || (hotspot >= MAX_ROOM_HOTSPOTS))
        quit("!GetHotspotPointY: invalid hotspot");

    if (thisroom.Hotspots[hotspot].WalkTo.X < 1) // TODO: there was "x" here, why?
        return -1;

    return thisroom.Hotspots[hotspot].WalkTo.Y;
}

int GetHotspotIDAtScreen(int scrx, int scry) {
    VpPoint vpt = play.ScreenToRoomDivDown(scrx, scry);
    if (vpt.second < 0) return 0;
    return get_hotspot_at(vpt.first.X, vpt.first.Y);
}

void GetHotspotName(int hotspot, char *buffer) {
    VALIDATE_STRING(buffer);
    if ((hotspot < 0) || (hotspot >= MAX_ROOM_HOTSPOTS))
        quit("!GetHotspotName: invalid hotspot number");

    strcpy(buffer, get_translation(thisroom.Hotspots[hotspot].Name));
}

void RunHotspotInteraction (int hotspothere, int mood) {

    int passon=-1,cdata=-1;
    if (mood==MODE_TALK) passon=4;
    else if (mood==MODE_WALK) passon=0;
    else if (mood==MODE_LOOK) passon=1;
    else if (mood==MODE_HAND) passon=2;
    else if (mood==MODE_PICKUP) passon=7;
    else if (mood==MODE_CUSTOM1) passon = 8;
    else if (mood==MODE_CUSTOM2) passon = 9;
    else if (mood==MODE_USE) { passon=3;
    cdata=playerchar->activeinv;
    play.usedinv=cdata;
    }

    if ((game.options[OPT_WALKONLOOK]==0) & (mood==MODE_LOOK)) ;
    else if (play.auto_use_walkto_points == 0) ;
    else if ((mood!=MODE_WALK) && (play.check_interaction_only == 0))
        MoveCharacterToHotspot(game.playercharacter,hotspothere);

    // can't use the setevent functions because this ProcessClick is only
    // executed once in a eventlist
    const char *oldbasename = evblockbasename;
    int   oldblocknum = evblocknum;

    evblockbasename="hotspot%d";
    evblocknum=hotspothere;

    if (thisroom.Hotspots[hotspothere].EventHandlers != nullptr)
    {
        if (passon>=0)
            run_interaction_script(thisroom.Hotspots[hotspothere].EventHandlers.get(), passon, 5, (passon == 3));
        run_interaction_script(thisroom.Hotspots[hotspothere].EventHandlers.get(), 5);  // any click on hotspot
    }
    else
    {
        if (passon>=0) {
            if (run_interaction_event(&croom->intrHotspot[hotspothere],passon, 5, (passon == 3))) {
                evblockbasename = oldbasename;
                evblocknum = oldblocknum;
                return;
            }
        }
        // run the 'any click on hs' event
        run_interaction_event(&croom->intrHotspot[hotspothere],5);
    }

    evblockbasename = oldbasename;
    evblocknum = oldblocknum;
}

int GetHotspotProperty (int hss, const char *property)
{
    return get_int_property(thisroom.Hotspots[hss].Properties, croom->hsProps[hss], property);
}

void GetHotspotPropertyText (int item, const char *property, char *bufer)
{
    get_text_property(thisroom.Hotspots[item].Properties, croom->hsProps[item], property, bufer);
}
