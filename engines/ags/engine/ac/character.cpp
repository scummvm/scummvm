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
//
// AGS Character functions
//
//=============================================================================

#include "ac/character.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/view.h"
#include "ac/display.h"
#include "ac/draw.h"
#include "ac/event.h"
#include "ac/game.h"
#include "ac/global_audio.h"
#include "ac/global_character.h"
#include "ac/global_game.h"
#include "ac/global_object.h"
#include "ac/global_region.h"
#include "ac/global_room.h"
#include "ac/global_translation.h"
#include "ac/gui.h"
#include "ac/lipsync.h"
#include "ac/mouse.h"
#include "ac/object.h"
#include "ac/overlay.h"
#include "ac/properties.h"
#include "ac/room.h"
#include "ac/screenoverlay.h"
#include "ac/string.h"
#include "ac/system.h"
#include "ac/viewframe.h"
#include "ac/walkablearea.h"
#include "gui/guimain.h"
#include "ac/route_finder.h"
#include "ac/gamestate.h"
#include "debug/debug_log.h"
#include "main/game_run.h"
#include "main/update.h"
#include "ac/spritecache.h"
#include "util/string_compat.h"
#include <math.h>
#include "gfx/graphicsdriver.h"
#include "script/runtimescriptvalue.h"
#include "ac/dynobj/cc_character.h"
#include "ac/dynobj/cc_inventory.h"
#include "script/script_runtime.h"
#include "gfx/gfx_def.h"
#include "media/audio/audio_system.h"
#include "ac/movelist.h"

using namespace AGS::Common;

extern GameSetupStruct game;
extern int displayed_room,starting_room;
extern RoomStruct thisroom;
extern MoveList *mls;
extern ViewStruct*views;
extern RoomObject*objs;
extern ScriptInvItem scrInv[MAX_INV];
extern SpriteCache spriteset;
extern Bitmap *walkable_areas_temp;
extern IGraphicsDriver *gfxDriver;
extern Bitmap **actsps;
extern int is_text_overlay;
extern int said_speech_line;
extern int said_text;
extern int our_eip;
extern CCCharacter ccDynamicCharacter;
extern CCInventory ccDynamicInv;

//--------------------------------


CharacterExtras *charextra;
CharacterInfo*playerchar;
int32_t _sc_PlayerCharPtr = 0;
int char_lowest_yp;

// Sierra-style speech settings
int face_talking=-1,facetalkview=0,facetalkwait=0,facetalkframe=0;
int facetalkloop=0, facetalkrepeat = 0, facetalkAllowBlink = 1;
int facetalkBlinkLoop = 0;
CharacterInfo *facetalkchar = nullptr;
// Do override default portrait position during QFG4-style speech overlay update
bool facetalk_qfg4_override_placement_x = false;
bool facetalk_qfg4_override_placement_y = false;

// lip-sync speech settings
int loops_per_character, text_lips_offset, char_speaking = -1;
int char_thinking = -1;
const char *text_lips_text = nullptr;
SpeechLipSyncLine *splipsync = nullptr;
int numLipLines = 0, curLipLine = -1, curLipLinePhoneme = 0;

// **** CHARACTER: FUNCTIONS ****

void Character_AddInventory(CharacterInfo *chaa, ScriptInvItem *invi, int addIndex) {
    int ee;

    if (invi == nullptr)
        quit("!AddInventoryToCharacter: invalid invnetory number");

    int inum = invi->id;

    if (chaa->inv[inum] >= 32000)
        quit("!AddInventory: cannot carry more than 32000 of one inventory item");

    chaa->inv[inum]++;

    int charid = chaa->index_id;

    if (game.options[OPT_DUPLICATEINV] == 0) {
        // Ensure it is only in the list once
        for (ee = 0; ee < charextra[charid].invorder_count; ee++) {
            if (charextra[charid].invorder[ee] == inum) {
                // They already have the item, so don't add it to the list
                if (chaa == playerchar)
                    run_on_event (GE_ADD_INV, RuntimeScriptValue().SetInt32(inum));
                return;
            }
        }
    }
    if (charextra[charid].invorder_count >= MAX_INVORDER)
        quit("!Too many inventory items added, max 500 display at one time");

    if ((addIndex == SCR_NO_VALUE) ||
        (addIndex >= charextra[charid].invorder_count) ||
        (addIndex < 0)) {
            // add new item at end of list
            charextra[charid].invorder[charextra[charid].invorder_count] = inum;
    }
    else {
        // insert new item at index
        for (ee = charextra[charid].invorder_count - 1; ee >= addIndex; ee--)
            charextra[charid].invorder[ee + 1] = charextra[charid].invorder[ee];

        charextra[charid].invorder[addIndex] = inum;
    }
    charextra[charid].invorder_count++;
    guis_need_update = 1;
    if (chaa == playerchar)
        run_on_event (GE_ADD_INV, RuntimeScriptValue().SetInt32(inum));

}

void Character_AddWaypoint(CharacterInfo *chaa, int x, int y) {

    if (chaa->room != displayed_room)
        quit("!MoveCharacterPath: specified character not in current room");

    // not already walking, so just do a normal move
    if (chaa->walking <= 0) {
        Character_Walk(chaa, x, y, IN_BACKGROUND, ANYWHERE);
        return;
    }

    MoveList *cmls = &mls[chaa->walking % TURNING_AROUND];
    if (cmls->numstage >= MAXNEEDSTAGES)
    {
        debug_script_warn("Character_AddWaypoint: move is too complex, cannot add any further paths");
        return;
    }

    cmls->pos[cmls->numstage] = (x << 16) + y;
    // They're already walking there anyway
    if (cmls->pos[cmls->numstage] == cmls->pos[cmls->numstage - 1])
        return;

    calculate_move_stage (cmls, cmls->numstage-1);
    cmls->numstage ++;

}

void Character_AnimateFrom(CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction, int sframe) {

    if (direction == FORWARDS)
        direction = 0;
    else if (direction == BACKWARDS)
        direction = 1;
    else
        quit("!Character.Animate: Invalid DIRECTION parameter");

    animate_character(chaa, loop, delay, repeat, 0, direction, sframe);

    if ((blocking == BLOCKING) || (blocking == 1))
        GameLoopUntilValueIsZero(&chaa->animating);
    else if ((blocking != IN_BACKGROUND) && (blocking != 0))
        quit("!Character.Animate: Invalid BLOCKING parameter");
}

void Character_Animate(CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction) {
    Character_AnimateFrom(chaa, loop, delay, repeat, blocking, direction, 0);
}

void Character_ChangeRoomAutoPosition(CharacterInfo *chaa, int room, int newPos) 
{
    if (chaa->index_id != game.playercharacter) 
    {
        quit("!Character.ChangeRoomAutoPosition can only be used with the player character.");
    }

    new_room_pos = newPos;

    if (new_room_pos == 0) {
        // auto place on other side of screen
        if (chaa->x <= thisroom.Edges.Left + 10)
            new_room_pos = 2000;
        else if (chaa->x >= thisroom.Edges.Right - 10)
            new_room_pos = 1000;
        else if (chaa->y <= thisroom.Edges.Top + 10)
            new_room_pos = 3000;
        else if (chaa->y >= thisroom.Edges.Bottom - 10)
            new_room_pos = 4000;

        if (new_room_pos < 3000)
            new_room_pos += chaa->y;
        else
            new_room_pos += chaa->x;
    }
    NewRoom(room);
}

void Character_ChangeRoom(CharacterInfo *chaa, int room, int x, int y) {
    Character_ChangeRoomSetLoop(chaa, room, x, y, SCR_NO_VALUE);
}

void Character_ChangeRoomSetLoop(CharacterInfo *chaa, int room, int x, int y, int direction) {

    if (chaa->index_id != game.playercharacter) {
        // NewRoomNPC
        if ((x != SCR_NO_VALUE) && (y != SCR_NO_VALUE)) {
            chaa->x = x;
            chaa->y = y;
			if (direction != SCR_NO_VALUE && direction>=0) chaa->loop = direction;
        }
        chaa->prevroom = chaa->room;
        chaa->room = room;

		debug_script_log("%s moved to room %d, location %d,%d, loop %d",
			chaa->scrname, room, chaa->x, chaa->y, chaa->loop);

        return;
    }

    if ((x != SCR_NO_VALUE) && (y != SCR_NO_VALUE)) {
        new_room_pos = 0;

        if (loaded_game_file_version <= kGameVersion_272)
        {
            // Set position immediately on 2.x.
            chaa->x = x;
            chaa->y = y;
        }
        else
        {
            // don't check X or Y bounds, so that they can do a
            // walk-in animation if they want
            new_room_x = x;
            new_room_y = y;
			if (direction != SCR_NO_VALUE) new_room_loop = direction;
        }
    }

    NewRoom(room);
}


void Character_ChangeView(CharacterInfo *chap, int vii) {
    vii--;

    if ((vii < 0) || (vii >= game.numviews))
        quit("!ChangeCharacterView: invalid view number specified");

    // if animating, but not idle view, give warning message
    if ((chap->flags & CHF_FIXVIEW) && (chap->idleleft >= 0))
        debug_script_warn("Warning: ChangeCharacterView was used while the view was fixed - call ReleaseCharView first");

    // if the idle animation is playing we should release the view
    if ( chap->idleleft < 0) {
      Character_UnlockView(chap);
      chap->idleleft = chap->idletime;
    }

    debug_script_log("%s: Change view to %d", chap->scrname, vii+1);
    chap->defview = vii;
    chap->view = vii;
    chap->animating = 0;
    chap->frame = 0;
    chap->wait = 0;
    chap->walkwait = 0;
    charextra[chap->index_id].animwait = 0;
    FindReasonableLoopForCharacter(chap);
}

enum DirectionalLoop
{
    kDirLoop_Down      = 0,
    kDirLoop_Left      = 1,
    kDirLoop_Right     = 2,
    kDirLoop_Up        = 3,
    kDirLoop_DownRight = 4,
    kDirLoop_UpRight   = 5,
    kDirLoop_DownLeft  = 6,
    kDirLoop_UpLeft    = 7,

    kDirLoop_Default        = kDirLoop_Down,
    kDirLoop_LastOrthogonal = kDirLoop_Up,
    kDirLoop_Last           = kDirLoop_UpLeft,
};

// Internal direction-facing functions

DirectionalLoop GetDirectionalLoop(CharacterInfo *chinfo, int x_diff, int y_diff)
{
    DirectionalLoop next_loop = kDirLoop_Left; // NOTE: default loop was Left for some reason

    const ViewStruct &chview  = views[chinfo->view];
    const bool new_version    = loaded_game_file_version > kGameVersion_272;
    const bool has_down_loop  = ((chview.numLoops > kDirLoop_Down)  && (chview.loops[kDirLoop_Down].numFrames > 0));
    const bool has_up_loop    = ((chview.numLoops > kDirLoop_Up)    && (chview.loops[kDirLoop_Up].numFrames > 0));
    // NOTE: 3.+ games required left & right loops to be present at all times
    const bool has_left_loop  = new_version ||
                                ((chview.numLoops > kDirLoop_Left)  && (chview.loops[kDirLoop_Left].numFrames > 0));
    const bool has_right_loop = new_version ||
                                ((chview.numLoops > kDirLoop_Right) && (chview.loops[kDirLoop_Right].numFrames > 0));
    const bool has_diagonal_loops = useDiagonal(chinfo) == 0; // NOTE: useDiagonal returns 0 for "true"

    const bool want_horizontal = (abs(y_diff) < abs(x_diff)) ||
        (new_version && (!has_down_loop || !has_up_loop) )||
        // NOTE: <= 2.72 games switch to horizontal loops only if both vertical ones are missing
        (!new_version && (!has_down_loop && !has_up_loop));
    if (want_horizontal)
    {
        const bool want_diagonal = has_diagonal_loops && (abs(y_diff) > abs(x_diff) / 2);
        if (!has_left_loop && !has_right_loop)
        {
            next_loop = kDirLoop_Down;
        }
        else if (has_right_loop && (x_diff > 0))
        {
            next_loop = want_diagonal ? (y_diff < 0 ? kDirLoop_UpRight : kDirLoop_DownRight) :
                kDirLoop_Right;
        }
        else if (has_left_loop && (x_diff <= 0))
        {
            next_loop = want_diagonal ? (y_diff < 0 ? kDirLoop_UpLeft : kDirLoop_DownLeft) :
                kDirLoop_Left;
        }
    }
    else
    {
        const bool want_diagonal = has_diagonal_loops && (abs(x_diff) > abs(y_diff) / 2);
        if (y_diff > 0 || !has_up_loop)
        {
            next_loop = want_diagonal ? (x_diff < 0 ? kDirLoop_DownLeft : kDirLoop_DownRight) :
                kDirLoop_Down;
        }
        else
        {
            next_loop = want_diagonal ? (x_diff < 0 ? kDirLoop_UpLeft : kDirLoop_UpRight) :
                kDirLoop_Up;
        }
    }
    return next_loop;
}

void FaceDirectionalLoop(CharacterInfo *char1, int direction, int blockingStyle)
{
    // Change facing only if the desired direction is different
    if (direction != char1->loop)
    {
        if ((game.options[OPT_TURNTOFACELOC] != 0) &&
            (in_enters_screen == 0))
        {
            const int no_diagonal = useDiagonal (char1);
            const int highestLoopForTurning = no_diagonal != 1 ? kDirLoop_Last : kDirLoop_LastOrthogonal;
            if ((char1->loop <= highestLoopForTurning))
            {
                // Turn to face new direction
                Character_StopMoving(char1);
                if (char1->on == 1)
                {
                    // only do the turning if the character is not hidden
                    // (otherwise GameLoopUntilNotMoving will never return)
                    start_character_turning (char1, direction, no_diagonal);

                    if ((blockingStyle == BLOCKING) || (blockingStyle == 1))
                        GameLoopUntilNotMoving(&char1->walking);
                }
                else
                    char1->loop = direction;
            }
            else
                char1->loop = direction;
        }
        else
            char1->loop = direction;
    }

    char1->frame = 0;
}

void FaceLocationXY(CharacterInfo *char1, int xx, int yy, int blockingStyle)
{
    debug_script_log("%s: Face location %d,%d", char1->scrname, xx, yy);

    const int diffrx = xx - char1->x;
    const int diffry = yy - char1->y;

    if ((diffrx == 0) && (diffry == 0)) {
        // FaceLocation called on their current position - do nothing
        return;
    }

    FaceDirectionalLoop(char1, GetDirectionalLoop(char1, diffrx, diffry), blockingStyle);
}

// External direction-facing functions with validation

void Character_FaceDirection(CharacterInfo *char1, int direction, int blockingStyle)
{
    if (char1 == nullptr)
        quit("!FaceDirection: invalid character specified");

    if (direction != SCR_NO_VALUE)
    {
        if (direction < 0 || direction > kDirLoop_Last)
            quit("!FaceDirection: invalid direction specified");

        FaceDirectionalLoop(char1, direction, blockingStyle);
    }
}

void Character_FaceLocation(CharacterInfo *char1, int xx, int yy, int blockingStyle)
{
    if (char1 == nullptr)
        quit("!FaceLocation: invalid character specified");

    FaceLocationXY(char1, xx, yy, blockingStyle);
}

void Character_FaceObject(CharacterInfo *char1, ScriptObject *obj, int blockingStyle) {
    if (obj == nullptr) 
        quit("!FaceObject: invalid object specified");

    FaceLocationXY(char1, objs[obj->id].x, objs[obj->id].y, blockingStyle);
}

void Character_FaceCharacter(CharacterInfo *char1, CharacterInfo *char2, int blockingStyle) {
    if (char2 == nullptr) 
        quit("!FaceCharacter: invalid character specified");

    if (char1->room != char2->room)
        quit("!FaceCharacter: characters are in different rooms");

    FaceLocationXY(char1, char2->x, char2->y, blockingStyle);
}

void Character_FollowCharacter(CharacterInfo *chaa, CharacterInfo *tofollow, int distaway, int eagerness) {

    if ((eagerness < 0) || (eagerness > 250))
        quit("!FollowCharacterEx: invalid eagerness: must be 0-250");

    if ((chaa->index_id == game.playercharacter) && (tofollow != nullptr) && 
        (tofollow->room != chaa->room))
        quit("!FollowCharacterEx: you cannot tell the player character to follow a character in another room");

    if (tofollow != nullptr) {
        debug_script_log("%s: Start following %s (dist %d, eager %d)", chaa->scrname, tofollow->scrname, distaway, eagerness);
    }
    else {
        debug_script_log("%s: Stop following other character", chaa->scrname);
    }

    if ((chaa->following >= 0) &&
        (chaa->followinfo == FOLLOW_ALWAYSONTOP)) {
            // if this character was following always-on-top, its baseline will
            // have been changed, so release it.
            chaa->baseline = -1;
    }

    if (tofollow == nullptr)
        chaa->following = -1;
    else
        chaa->following = tofollow->index_id;

    chaa->followinfo=(distaway << 8) | eagerness;

    chaa->flags &= ~CHF_BEHINDSHEPHERD;

    // special case for Always On Other Character
    if (distaway == FOLLOW_ALWAYSONTOP) {
        chaa->followinfo = FOLLOW_ALWAYSONTOP;
        if (eagerness == 1)
            chaa->flags |= CHF_BEHINDSHEPHERD;
    }

    if (chaa->animating & CHANIM_REPEAT)
        debug_script_warn("Warning: FollowCharacter called but the sheep is currently animating looped. It may never start to follow.");

}

int Character_IsCollidingWithChar(CharacterInfo *char1, CharacterInfo *char2) {
    if (char2 == nullptr)
        quit("!AreCharactersColliding: invalid char2");

    if (char1->room != char2->room) return 0; // not colliding

    if ((char1->y > char2->y - 5) && (char1->y < char2->y + 5)) ;
    else return 0;

    int w1 = game_to_data_coord(GetCharacterWidth(char1->index_id));
    int w2 = game_to_data_coord(GetCharacterWidth(char2->index_id));

    int xps1=char1->x - w1/2;
    int xps2=char2->x - w2/2;

    if ((xps1 >= xps2 - w1) & (xps1 <= xps2 + w2)) return 1;
    return 0;
}

int Character_IsCollidingWithObject(CharacterInfo *chin, ScriptObject *objid) {
    if (objid == nullptr)
        quit("!AreCharObjColliding: invalid object number");

    if (chin->room != displayed_room)
        return 0;
    if (objs[objid->id].on != 1)
        return 0;

    Bitmap *checkblk = GetObjectImage(objid->id, nullptr);
    int objWidth = checkblk->GetWidth();
    int objHeight = checkblk->GetHeight();
    int o1x = objs[objid->id].x;
    int o1y = objs[objid->id].y - game_to_data_coord(objHeight);

    Bitmap *charpic = GetCharacterImage(chin->index_id, nullptr);

    int charWidth = charpic->GetWidth();
    int charHeight = charpic->GetHeight();
    int o2x = chin->x - game_to_data_coord(charWidth) / 2;
    int o2y = chin->get_effective_y() - 5;  // only check feet

    if ((o2x >= o1x - game_to_data_coord(charWidth)) &&
        (o2x <= o1x + game_to_data_coord(objWidth)) &&
        (o2y >= o1y - 8) &&
        (o2y <= o1y + game_to_data_coord(objHeight))) {
            // the character's feet are on the object
            if (game.options[OPT_PIXPERFECT] == 0)
                return 1;
            // check if they're on a transparent bit of the object
            int stxp = data_to_game_coord(o2x - o1x);
            int styp = data_to_game_coord(o2y - o1y);
            int maskcol = checkblk->GetMaskColor ();
            int maskcolc = charpic->GetMaskColor ();
            int thispix, thispixc;
            // check each pixel of the object along the char's feet
            for (int i = 0; i < charWidth; i += get_fixed_pixel_size(1)) {
                for (int j = 0; j < get_fixed_pixel_size(6); j += get_fixed_pixel_size(1)) {
                    thispix = my_getpixel(checkblk, i + stxp, j + styp);
                    thispixc = my_getpixel(charpic, i, j + (charHeight - get_fixed_pixel_size(5)));

                    if ((thispix != -1) && (thispix != maskcol) &&
                        (thispixc != -1) && (thispixc != maskcolc))
                        return 1;
                }
            }

    }
    return 0;
}

bool Character_IsInteractionAvailable(CharacterInfo *cchar, int mood) {

    play.check_interaction_only = 1;
    RunCharacterInteraction(cchar->index_id, mood);
    int ciwas = play.check_interaction_only;
    play.check_interaction_only = 0;
    return (ciwas == 2);
}

void Character_LockView(CharacterInfo *chap, int vii) {
    Character_LockViewEx(chap, vii, STOP_MOVING);
}

void Character_LockViewEx(CharacterInfo *chap, int vii, int stopMoving) {

    if ((vii < 1) || (vii > game.numviews)) {
        quitprintf("!SetCharacterView: invalid view number (You said %d, max is %d)", vii, game.numviews);
    }
    vii--;

    debug_script_log("%s: View locked to %d", chap->scrname, vii+1);
    if (chap->idleleft < 0) {
        Character_UnlockView(chap);
        chap->idleleft = chap->idletime;
    }
    if (stopMoving != KEEP_MOVING)
    {
        Character_StopMoving(chap);
    }
    chap->view=vii;
    chap->animating=0;
    FindReasonableLoopForCharacter(chap);
    chap->frame=0;
    chap->wait=0;
    chap->flags|=CHF_FIXVIEW;
    chap->pic_xoffs = 0;
    chap->pic_yoffs = 0;
}

void Character_LockViewAligned_Old(CharacterInfo *chap, int vii, int loop, int align) {
    Character_LockViewAlignedEx(chap, vii, loop, ConvertLegacyScriptAlignment((LegacyScriptAlignment)align), STOP_MOVING);
}

void Character_LockViewAlignedEx_Old(CharacterInfo *chap, int vii, int loop, int align, int stopMoving) {
    Character_LockViewAlignedEx(chap, vii, loop, ConvertLegacyScriptAlignment((LegacyScriptAlignment)align), stopMoving);
}

void Character_LockViewAligned(CharacterInfo *chap, int vii, int loop, int align) {
    Character_LockViewAlignedEx(chap, vii, loop, align, STOP_MOVING);
}

void Character_LockViewAlignedEx(CharacterInfo *chap, int vii, int loop, int align, int stopMoving) {
    if (chap->view < 0)
        quit("!SetCharacterLoop: character has invalid old view number");

    int sppic = views[chap->view].loops[chap->loop].frames[chap->frame].pic;
    int leftSide = data_to_game_coord(chap->x) - game.SpriteInfos[sppic].Width / 2;

    Character_LockViewEx(chap, vii, stopMoving);

    if ((loop < 0) || (loop >= views[chap->view].numLoops))
        quit("!SetCharacterViewEx: invalid loop specified");

    chap->loop = loop;
    chap->frame = 0;
    int newpic = views[chap->view].loops[chap->loop].frames[chap->frame].pic;
    int newLeft = data_to_game_coord(chap->x) - game.SpriteInfos[newpic].Width / 2;
    int xdiff = 0;

    if (align & kMAlignLeft)
        xdiff = leftSide - newLeft;
    else if (align & kMAlignHCenter)
        xdiff = 0;
    else if (align & kMAlignRight)
        xdiff = (leftSide + game.SpriteInfos[sppic].Width) - (newLeft + game.SpriteInfos[newpic].Width);
    else
        quit("!SetCharacterViewEx: invalid alignment type specified");

    chap->pic_xoffs = xdiff;
    chap->pic_yoffs = 0;
}

void Character_LockViewFrame(CharacterInfo *chaa, int view, int loop, int frame) {
    Character_LockViewFrameEx(chaa, view, loop, frame, STOP_MOVING);
}

void Character_LockViewFrameEx(CharacterInfo *chaa, int view, int loop, int frame, int stopMoving) {

    Character_LockViewEx(chaa, view, stopMoving);

    view--;
    if ((loop < 0) || (loop >= views[view].numLoops))
        quit("!SetCharacterFrame: invalid loop specified");
    if ((frame < 0) || (frame >= views[view].loops[loop].numFrames))
        quit("!SetCharacterFrame: invalid frame specified");

    chaa->loop = loop;
    chaa->frame = frame;
}

void Character_LockViewOffset(CharacterInfo *chap, int vii, int xoffs, int yoffs) {
    Character_LockViewOffsetEx(chap, vii, xoffs, yoffs, STOP_MOVING);
}

void Character_LockViewOffsetEx(CharacterInfo *chap, int vii, int xoffs, int yoffs, int stopMoving) {
    Character_LockViewEx(chap, vii, stopMoving);

    // This function takes offsets in real game coordinates as opposed to script coordinates
    defgame_to_finalgame_coords(xoffs, yoffs);
    chap->pic_xoffs = xoffs;
    chap->pic_yoffs = yoffs;
}

void Character_LoseInventory(CharacterInfo *chap, ScriptInvItem *invi) {

    if (invi == nullptr)
        quit("!LoseInventoryFromCharacter: invalid invnetory number");

    int inum = invi->id;

    if (chap->inv[inum] > 0)
        chap->inv[inum]--;

    if ((chap->activeinv == inum) & (chap->inv[inum] < 1)) {
        chap->activeinv = -1;
        if ((chap == playerchar) && (GetCursorMode() == MODE_USE))
            set_cursor_mode(0);
    }

    int charid = chap->index_id;

    if ((chap->inv[inum] == 0) || (game.options[OPT_DUPLICATEINV] > 0)) {
        int xx,tt;
        for (xx = 0; xx < charextra[charid].invorder_count; xx++) {
            if (charextra[charid].invorder[xx] == inum) {
                charextra[charid].invorder_count--;
                for (tt = xx; tt < charextra[charid].invorder_count; tt++)
                    charextra[charid].invorder[tt] = charextra[charid].invorder[tt+1];
                break;
            }
        }
    }
    guis_need_update = 1;

    if (chap == playerchar)
        run_on_event (GE_LOSE_INV, RuntimeScriptValue().SetInt32(inum));
}

void Character_PlaceOnWalkableArea(CharacterInfo *chap) 
{
    if (displayed_room < 0)
        quit("!Character.PlaceOnWalkableArea: no room is currently loaded");

    find_nearest_walkable_area(&chap->x, &chap->y);
}

void Character_RemoveTint(CharacterInfo *chaa) {

    if (chaa->flags & (CHF_HASTINT | CHF_HASLIGHT)) {
        debug_script_log("Un-tint %s", chaa->scrname);
        chaa->flags &= ~(CHF_HASTINT | CHF_HASLIGHT);
    }
    else {
        debug_script_warn("Character.RemoveTint called but character was not tinted");
    }
}

int Character_GetHasExplicitTint_Old(CharacterInfo *ch)
{
    return ch->has_explicit_tint() || ch->has_explicit_light();
}

int Character_GetHasExplicitTint(CharacterInfo *ch)
{
    return ch->has_explicit_tint();
}

void Character_Say(CharacterInfo *chaa, const char *text) {
    _DisplaySpeechCore(chaa->index_id, text);
}

void Character_SayAt(CharacterInfo *chaa, int x, int y, int width, const char *texx) {

    DisplaySpeechAt(x, y, width, chaa->index_id, (char*)texx);
}

ScriptOverlay* Character_SayBackground(CharacterInfo *chaa, const char *texx) {

    int ovltype = DisplaySpeechBackground(chaa->index_id, (char*)texx);
    int ovri = find_overlay_of_type(ovltype);
    if (ovri<0)
        quit("!SayBackground internal error: no overlay");

    // Convert the overlay ID to an Overlay object
    ScriptOverlay *scOver = new ScriptOverlay();
    scOver->overlayId = ovltype;
    scOver->borderHeight = 0;
    scOver->borderWidth = 0;
    scOver->isBackgroundSpeech = 1;
    int handl = ccRegisterManagedObject(scOver, scOver);
    screenover[ovri].associatedOverlayHandle = handl;

    return scOver;
}

void Character_SetAsPlayer(CharacterInfo *chaa) {

    // Set to same character, so ignore.
    // But only on versions > 2.61. The relevant entry in the 2.62 changelog is:
    //  - Fixed SetPlayerCharacter to do nothing at all if you pass the current
    //    player character to it (previously it was resetting the inventory layout)
    if ((loaded_game_file_version > kGameVersion_261) && (game.playercharacter == chaa->index_id))
        return;

    setup_player_character(chaa->index_id);

    //update_invorder();

    debug_script_log("%s is new player character", playerchar->scrname);

    // Within game_start, return now
    if (displayed_room < 0)
        return;

    // Ignore invalid room numbers for the character and just place him in
    // the current room for 2.x. Following script calls to NewRoom() will
    // make sure this still works as intended.
    if ((loaded_game_file_version <= kGameVersion_272) && (playerchar->room < 0))
        playerchar->room = displayed_room;

    if (displayed_room != playerchar->room)
        NewRoom(playerchar->room);
    else   // make sure it doesn't run the region interactions
        play.player_on_region = GetRegionIDAtRoom(playerchar->x, playerchar->y);

    if ((playerchar->activeinv >= 0) && (playerchar->inv[playerchar->activeinv] < 1))
        playerchar->activeinv = -1;

    // They had inv selected, so change the cursor
    if (cur_mode == MODE_USE) {
        if (playerchar->activeinv < 0)
            SetNextCursor ();
        else
            SetActiveInventory (playerchar->activeinv);
    }

}


void Character_SetIdleView(CharacterInfo *chaa, int iview, int itime) {

    if (iview == 1) 
        quit("!SetCharacterIdle: view 1 cannot be used as an idle view, sorry.");

    // if an idle anim is currently playing, release it
    if (chaa->idleleft < 0)
        Character_UnlockView(chaa);

    chaa->idleview = iview - 1;
    // make sure they don't appear idle while idle anim is disabled
    if (iview < 1)
        itime = 10;
    chaa->idletime = itime;
    chaa->idleleft = itime;

    // if not currently animating, reset the wait counter
    if ((chaa->animating == 0) && (chaa->walking == 0))
        chaa->wait = 0;

    if (iview >= 1) {
        debug_script_log("Set %s idle view to %d (time %d)", chaa->scrname, iview, itime);
    }
    else {
        debug_script_log("%s idle view disabled", chaa->scrname);
    }
    if (chaa->flags & CHF_FIXVIEW) {
        debug_script_warn("SetCharacterIdle called while character view locked with SetCharacterView; idle ignored");
        debug_script_log("View locked, idle will not kick in until Released");
    }
    // if they switch to a swimming animation, kick it off immediately
    if (itime == 0)
        charextra[chaa->index_id].process_idle_this_time = 1;

}

bool Character_GetHasExplicitLight(CharacterInfo *ch)
{
    return ch->has_explicit_light();
}

int Character_GetLightLevel(CharacterInfo *ch)
{
    return ch->has_explicit_light() ? charextra[ch->index_id].tint_light : 0;
}

void Character_SetLightLevel(CharacterInfo *chaa, int light_level)
{
    light_level = Math::Clamp(light_level, -100, 100);
    
    charextra[chaa->index_id].tint_light = light_level;
    chaa->flags &= ~CHF_HASTINT;
    chaa->flags |= CHF_HASLIGHT;
}

int Character_GetTintRed(CharacterInfo *ch)
{
    return ch->has_explicit_tint() ? charextra[ch->index_id].tint_r : 0;
}

int Character_GetTintGreen(CharacterInfo *ch)
{
    return ch->has_explicit_tint() ? charextra[ch->index_id].tint_g : 0;
}

int Character_GetTintBlue(CharacterInfo *ch)
{
    return ch->has_explicit_tint() ? charextra[ch->index_id].tint_b : 0;
}

int Character_GetTintSaturation(CharacterInfo *ch)
{
    return ch->has_explicit_tint() ? charextra[ch->index_id].tint_level : 0;
}

int Character_GetTintLuminance(CharacterInfo *ch)
{
    return ch->has_explicit_tint() ? ((charextra[ch->index_id].tint_light * 10) / 25) : 0;
}

void Character_SetOption(CharacterInfo *chaa, int flag, int yesorno) {

    if ((yesorno < 0) || (yesorno > 1))
        quit("!SetCharacterProperty: last parameter must be 0 or 1");

    if (flag & CHF_MANUALSCALING) {
        // backwards compatibility fix
        Character_SetIgnoreScaling(chaa, yesorno);
    }
    else {
        chaa->flags &= ~flag;
        if (yesorno)
            chaa->flags |= flag;
    }

}

void Character_SetSpeed(CharacterInfo *chaa, int xspeed, int yspeed) {

    if ((xspeed == 0) || (xspeed > 50) || (yspeed == 0) || (yspeed > 50))
        quit("!SetCharacterSpeedEx: invalid speed value");
    if (chaa->walking)
    {
        debug_script_warn("Character_SetSpeed: cannot change speed while walking");
        return;
    }

    chaa->walkspeed = xspeed;

    if (yspeed == xspeed) 
        chaa->walkspeed_y = UNIFORM_WALK_SPEED;
    else
        chaa->walkspeed_y = yspeed;
}


void Character_StopMoving(CharacterInfo *charp) {

    int chaa = charp->index_id;
    if (chaa == play.skip_until_char_stops)
        EndSkippingUntilCharStops();

    if (charextra[chaa].xwas != INVALID_X) {
        charp->x = charextra[chaa].xwas;
        charp->y = charextra[chaa].ywas;
        charextra[chaa].xwas = INVALID_X;
    }
    if ((charp->walking > 0) && (charp->walking < TURNING_AROUND)) {
        // if it's not a MoveCharDirect, make sure they end up on a walkable area
        if ((mls[charp->walking].direct == 0) && (charp->room == displayed_room))
            Character_PlaceOnWalkableArea(charp);

        debug_script_log("%s: stop moving", charp->scrname);

        charp->idleleft = charp->idletime;
        // restart the idle animation straight away
        charextra[chaa].process_idle_this_time = 1;
    }
    if (charp->walking) {
        // If the character is currently moving, stop them and reset their frame
        charp->walking = 0;
        if ((charp->flags & CHF_MOVENOTWALK) == 0)
            charp->frame = 0;
    }
}

void Character_Tint(CharacterInfo *chaa, int red, int green, int blue, int opacity, int luminance) {
    if ((red < 0) || (green < 0) || (blue < 0) ||
        (red > 255) || (green > 255) || (blue > 255) ||
        (opacity < 0) || (opacity > 100) ||
        (luminance < 0) || (luminance > 100))
        quit("!Character.Tint: invalid parameter. R,G,B must be 0-255, opacity & luminance 0-100");

    debug_script_log("Set %s tint RGB(%d,%d,%d) %d%%", chaa->scrname, red, green, blue, opacity);

    charextra[chaa->index_id].tint_r = red;
    charextra[chaa->index_id].tint_g = green;
    charextra[chaa->index_id].tint_b = blue;
    charextra[chaa->index_id].tint_level = opacity;
    charextra[chaa->index_id].tint_light = (luminance * 25) / 10;
    chaa->flags &= ~CHF_HASLIGHT;
    chaa->flags |= CHF_HASTINT;
}

void Character_Think(CharacterInfo *chaa, const char *text) {
    _DisplayThoughtCore(chaa->index_id, text);
}

void Character_UnlockView(CharacterInfo *chaa) {
    Character_UnlockViewEx(chaa, STOP_MOVING);
}

void Character_UnlockViewEx(CharacterInfo *chaa, int stopMoving) {
    if (chaa->flags & CHF_FIXVIEW) {
        debug_script_log("%s: Released view back to default", chaa->scrname);
    }
    chaa->flags &= ~CHF_FIXVIEW;
    chaa->view = chaa->defview;
    chaa->frame = 0;
    if (stopMoving != KEEP_MOVING)
    {
        Character_StopMoving(chaa);
    }
    if (chaa->view >= 0) {
        int maxloop = views[chaa->view].numLoops;
        if (((chaa->flags & CHF_NODIAGONAL)!=0) && (maxloop > 4))
            maxloop = 4;
        FindReasonableLoopForCharacter(chaa);
    }
    chaa->animating = 0;
    chaa->idleleft = chaa->idletime;
    chaa->pic_xoffs = 0;
    chaa->pic_yoffs = 0;
    // restart the idle animation straight away
    charextra[chaa->index_id].process_idle_this_time = 1;

}


void Character_Walk(CharacterInfo *chaa, int x, int y, int blocking, int direct) 
{
    walk_or_move_character(chaa, x, y, blocking, direct, true);
}

void Character_Move(CharacterInfo *chaa, int x, int y, int blocking, int direct) 
{
    walk_or_move_character(chaa, x, y, blocking, direct, false);
}

void Character_WalkStraight(CharacterInfo *chaa, int xx, int yy, int blocking) {

    if (chaa->room != displayed_room)
        quit("!MoveCharacterStraight: specified character not in current room");

    Character_StopMoving(chaa);
    int movetox = xx, movetoy = yy;

    set_wallscreen(prepare_walkable_areas(chaa->index_id));

    int fromXLowres = room_to_mask_coord(chaa->x);
    int fromYLowres = room_to_mask_coord(chaa->y);
    int toXLowres = room_to_mask_coord(xx);
    int toYLowres = room_to_mask_coord(yy);

    if (!can_see_from(fromXLowres, fromYLowres, toXLowres, toYLowres)) {
        int lastcx, lastcy;
        get_lastcpos(lastcx, lastcy);
        movetox = mask_to_room_coord(lastcx);
        movetoy = mask_to_room_coord(lastcy);
    }

    walk_character(chaa->index_id, movetox, movetoy, 1, true);

    if ((blocking == BLOCKING) || (blocking == 1))
        GameLoopUntilNotMoving(&chaa->walking);
    else if ((blocking != IN_BACKGROUND) && (blocking != 0))
        quit("!Character.Walk: Blocking must be BLOCKING or IN_BACKGRUOND");

}

void Character_RunInteraction(CharacterInfo *chaa, int mood) {

    RunCharacterInteraction(chaa->index_id, mood);
}



// **** CHARACTER: PROPERTIES ****

int Character_GetProperty(CharacterInfo *chaa, const char *property) {

    return get_int_property(game.charProps[chaa->index_id], play.charProps[chaa->index_id], property);

}
void Character_GetPropertyText(CharacterInfo *chaa, const char *property, char *bufer) {
    get_text_property(game.charProps[chaa->index_id], play.charProps[chaa->index_id], property, bufer);
}
const char* Character_GetTextProperty(CharacterInfo *chaa, const char *property) {
    return get_text_property_dynamic_string(game.charProps[chaa->index_id], play.charProps[chaa->index_id], property);
}

bool Character_SetProperty(CharacterInfo *chaa, const char *property, int value)
{
    return set_int_property(play.charProps[chaa->index_id], property, value);
}

bool Character_SetTextProperty(CharacterInfo *chaa, const char *property, const char *value)
{
    return set_text_property(play.charProps[chaa->index_id], property, value);
}

ScriptInvItem* Character_GetActiveInventory(CharacterInfo *chaa) {

    if (chaa->activeinv <= 0)
        return nullptr;

    return &scrInv[chaa->activeinv];
}

void Character_SetActiveInventory(CharacterInfo *chaa, ScriptInvItem* iit) {
    guis_need_update = 1;

    if (iit == nullptr) {
        chaa->activeinv = -1;

        if (chaa->index_id == game.playercharacter) {

            if (GetCursorMode()==MODE_USE)
                set_cursor_mode(0);
        }
        return;
    }

    if (chaa->inv[iit->id] < 1)
    {
        debug_script_warn("SetActiveInventory: character doesn't have any of that inventory");
        return;
    }

    chaa->activeinv = iit->id;

    if (chaa->index_id == game.playercharacter) {
        // if it's the player character, update mouse cursor
        update_inv_cursor(iit->id);
        set_cursor_mode(MODE_USE);
    }
}

int Character_GetAnimating(CharacterInfo *chaa) {
    if (chaa->animating)
        return 1;
    return 0;
}

int Character_GetAnimationSpeed(CharacterInfo *chaa) {
    return chaa->animspeed;
}

void Character_SetAnimationSpeed(CharacterInfo *chaa, int newval) {

    chaa->animspeed = newval;
}

int Character_GetBaseline(CharacterInfo *chaa) {

    if (chaa->baseline < 1)
        return 0;

    return chaa->baseline;
}

void Character_SetBaseline(CharacterInfo *chaa, int basel) {

    chaa->baseline = basel;
}

int Character_GetBlinkInterval(CharacterInfo *chaa) {

    return chaa->blinkinterval;
}

void Character_SetBlinkInterval(CharacterInfo *chaa, int interval) {

    if (interval < 0)
        quit("!SetCharacterBlinkView: invalid blink interval");

    chaa->blinkinterval = interval;

    if (chaa->blinktimer > 0)
        chaa->blinktimer = chaa->blinkinterval;
}

int Character_GetBlinkView(CharacterInfo *chaa) {

    return chaa->blinkview + 1;
}

void Character_SetBlinkView(CharacterInfo *chaa, int vii) {

    if (((vii < 2) || (vii > game.numviews)) && (vii != -1))
        quit("!SetCharacterBlinkView: invalid view number");

    chaa->blinkview = vii - 1;
}

int Character_GetBlinkWhileThinking(CharacterInfo *chaa) {
    if (chaa->flags & CHF_NOBLINKANDTHINK)
        return 0;
    return 1;
}

void Character_SetBlinkWhileThinking(CharacterInfo *chaa, int yesOrNo) {
    chaa->flags &= ~CHF_NOBLINKANDTHINK;
    if (yesOrNo == 0)
        chaa->flags |= CHF_NOBLINKANDTHINK;
}

int Character_GetBlockingHeight(CharacterInfo *chaa) {

    return chaa->blocking_height;
}

void Character_SetBlockingHeight(CharacterInfo *chaa, int hit) {

    chaa->blocking_height = hit;
}

int Character_GetBlockingWidth(CharacterInfo *chaa) {

    return chaa->blocking_width;
}

void Character_SetBlockingWidth(CharacterInfo *chaa, int wid) {

    chaa->blocking_width = wid;
}

int Character_GetDiagonalWalking(CharacterInfo *chaa) {

    if (chaa->flags & CHF_NODIAGONAL)
        return 0;
    return 1;  
}

void Character_SetDiagonalWalking(CharacterInfo *chaa, int yesorno) {

    chaa->flags &= ~CHF_NODIAGONAL;
    if (!yesorno)
        chaa->flags |= CHF_NODIAGONAL;
}

int Character_GetClickable(CharacterInfo *chaa) {

    if (chaa->flags & CHF_NOINTERACT)
        return 0;
    return 1;
}

void Character_SetClickable(CharacterInfo *chaa, int clik) {

    chaa->flags &= ~CHF_NOINTERACT;
    // if they don't want it clickable, set the relevant bit
    if (clik == 0)
        chaa->flags |= CHF_NOINTERACT;
}

int Character_GetID(CharacterInfo *chaa) {

    return chaa->index_id;

}

int Character_GetFrame(CharacterInfo *chaa) {
    return chaa->frame;
}

void Character_SetFrame(CharacterInfo *chaa, int newval) {
    chaa->frame = newval;
}

int Character_GetIdleView(CharacterInfo *chaa) {

    if (chaa->idleview < 1)
        return -1;

    return chaa->idleview + 1;
}

int Character_GetIInventoryQuantity(CharacterInfo *chaa, int index) {
    if ((index < 1) || (index >= game.numinvitems))
        quitprintf("!Character.InventoryQuantity: invalid inventory index %d", index);

    return chaa->inv[index];
}

int Character_HasInventory(CharacterInfo *chaa, ScriptInvItem *invi)
{
    if (invi == nullptr)
        quit("!Character.HasInventory: NULL inventory item supplied");

    return (chaa->inv[invi->id] > 0) ? 1 : 0;
}

void Character_SetIInventoryQuantity(CharacterInfo *chaa, int index, int quant) {
    if ((index < 1) || (index >= game.numinvitems))
        quitprintf("!Character.InventoryQuantity: invalid inventory index %d", index);

    if ((quant < 0) || (quant > 32000))
        quitprintf("!Character.InventoryQuantity: invalid quantity %d", quant);

    chaa->inv[index] = quant;
}

int Character_GetIgnoreLighting(CharacterInfo *chaa) {

    if (chaa->flags & CHF_NOLIGHTING)
        return 1;
    return 0;
}

void Character_SetIgnoreLighting(CharacterInfo *chaa, int yesorno) {

    chaa->flags &= ~CHF_NOLIGHTING;
    if (yesorno)
        chaa->flags |= CHF_NOLIGHTING;
}

int Character_GetIgnoreScaling(CharacterInfo *chaa) {

    if (chaa->flags & CHF_MANUALSCALING)
        return 1;
    return 0;  
}

void Character_SetIgnoreScaling(CharacterInfo *chaa, int yesorno) {

    if (yesorno) {
        // when setting IgnoreScaling to 1, should reset zoom level
        // like it used to in pre-2.71
        charextra[chaa->index_id].zoom = 100;
    }
    Character_SetManualScaling(chaa, yesorno);
}

void Character_SetManualScaling(CharacterInfo *chaa, int yesorno) {

    chaa->flags &= ~CHF_MANUALSCALING;
    if (yesorno)
        chaa->flags |= CHF_MANUALSCALING;
}

int Character_GetIgnoreWalkbehinds(CharacterInfo *chaa) {

    if (chaa->flags & CHF_NOWALKBEHINDS)
        return 1;
    return 0;
}

void Character_SetIgnoreWalkbehinds(CharacterInfo *chaa, int yesorno) {
    if (game.options[OPT_BASESCRIPTAPI] >= kScriptAPI_v350)
        debug_script_warn("IgnoreWalkbehinds is not recommended for use, consider other solutions");
    chaa->flags &= ~CHF_NOWALKBEHINDS;
    if (yesorno)
        chaa->flags |= CHF_NOWALKBEHINDS;
}

int Character_GetMovementLinkedToAnimation(CharacterInfo *chaa) {

    if (chaa->flags & CHF_ANTIGLIDE)
        return 1;
    return 0;
}

void Character_SetMovementLinkedToAnimation(CharacterInfo *chaa, int yesorno) {

    chaa->flags &= ~CHF_ANTIGLIDE;
    if (yesorno)
        chaa->flags |= CHF_ANTIGLIDE;
}

int Character_GetLoop(CharacterInfo *chaa) {
    return chaa->loop;
}

void Character_SetLoop(CharacterInfo *chaa, int newval) {
    if ((newval < 0) || (newval >= views[chaa->view].numLoops))
        quit("!Character.Loop: invalid loop number for this view");

    chaa->loop = newval;

    if (chaa->frame >= views[chaa->view].loops[chaa->loop].numFrames)
        chaa->frame = 0;
}

int Character_GetMoving(CharacterInfo *chaa) {
    if (chaa->walking)
        return 1;
    return 0;
}

int Character_GetDestinationX(CharacterInfo *chaa) {
    if (chaa->walking) {
        MoveList *cmls = &mls[chaa->walking % TURNING_AROUND];
        return cmls->pos[cmls->numstage - 1] >> 16;
    }
    else
        return chaa->x;
}

int Character_GetDestinationY(CharacterInfo *chaa) {
    if (chaa->walking) {
        MoveList *cmls = &mls[chaa->walking % TURNING_AROUND];
        return cmls->pos[cmls->numstage - 1] & 0xFFFF;
    }
    else
        return chaa->y;
}

const char* Character_GetName(CharacterInfo *chaa) {
    return CreateNewScriptString(chaa->name);
}

void Character_SetName(CharacterInfo *chaa, const char *newName) {
    strncpy(chaa->name, newName, 40);
    chaa->name[39] = 0;
}

int Character_GetNormalView(CharacterInfo *chaa) {
    return chaa->defview + 1;
}

int Character_GetPreviousRoom(CharacterInfo *chaa) {
    return chaa->prevroom;
}

int Character_GetRoom(CharacterInfo *chaa) {
    return chaa->room;
}


int Character_GetScaleMoveSpeed(CharacterInfo *chaa) {

    if (chaa->flags & CHF_SCALEMOVESPEED)
        return 1;
    return 0;  
}

void Character_SetScaleMoveSpeed(CharacterInfo *chaa, int yesorno) {

    if ((yesorno < 0) || (yesorno > 1))
        quit("Character.ScaleMoveSpeed: value must be true or false (1 or 0)");

    chaa->flags &= ~CHF_SCALEMOVESPEED;
    if (yesorno)
        chaa->flags |= CHF_SCALEMOVESPEED;
}

int Character_GetScaleVolume(CharacterInfo *chaa) {

    if (chaa->flags & CHF_SCALEVOLUME)
        return 1;
    return 0;  
}

void Character_SetScaleVolume(CharacterInfo *chaa, int yesorno) {

    if ((yesorno < 0) || (yesorno > 1))
        quit("Character.ScaleVolume: value must be true or false (1 or 0)");

    chaa->flags &= ~CHF_SCALEVOLUME;
    if (yesorno)
        chaa->flags |= CHF_SCALEVOLUME;
}

int Character_GetScaling(CharacterInfo *chaa) {
    return charextra[chaa->index_id].zoom;
}

void Character_SetScaling(CharacterInfo *chaa, int zoomlevel) {

    if ((chaa->flags & CHF_MANUALSCALING) == 0)
    {
        debug_script_warn("Character.Scaling: cannot set property unless ManualScaling is enabled");
        return;
    }
    if ((zoomlevel < 5) || (zoomlevel > 200))
        quit("!Character.Scaling: scaling level must be between 5 and 200%");

    charextra[chaa->index_id].zoom = zoomlevel;
}

int Character_GetSolid(CharacterInfo *chaa) {

    if (chaa->flags & CHF_NOBLOCKING)
        return 0;
    return 1;
}

void Character_SetSolid(CharacterInfo *chaa, int yesorno) {

    chaa->flags &= ~CHF_NOBLOCKING;
    if (!yesorno)
        chaa->flags |= CHF_NOBLOCKING;
}

int Character_GetSpeaking(CharacterInfo *chaa) {
    if (get_character_currently_talking() == chaa->index_id)
        return 1;

    return 0;
}

int Character_GetSpeechColor(CharacterInfo *chaa) {

    return chaa->talkcolor;
}

void Character_SetSpeechColor(CharacterInfo *chaa, int ncol) {

    chaa->talkcolor = ncol;
}

void Character_SetSpeechAnimationDelay(CharacterInfo *chaa, int newDelay)
{
    if (game.options[OPT_GLOBALTALKANIMSPD] != 0)
    {
        debug_script_warn("Character.SpeechAnimationDelay cannot be set when global speech animation speed is enabled");
        return;
    }

    chaa->speech_anim_speed = newDelay;
}

int Character_GetSpeechView(CharacterInfo *chaa) {

    return chaa->talkview + 1;
}

void Character_SetSpeechView(CharacterInfo *chaa, int vii) {
    if (vii == -1) {
        chaa->talkview = -1;
        return;
    }

    if ((vii < 1) || (vii > game.numviews))
        quit("!SetCharacterSpeechView: invalid view number");

    chaa->talkview = vii - 1;
}

bool Character_GetThinking(CharacterInfo *chaa)
{
    return char_thinking == chaa->index_id;
}

int Character_GetThinkingFrame(CharacterInfo *chaa)
{
    if (char_thinking == chaa->index_id)
        return chaa->thinkview > 0 ? chaa->frame : -1;

    debug_script_warn("Character.ThinkingFrame: character is not currently thinking");
    return -1;
}

int Character_GetThinkView(CharacterInfo *chaa) {

    return chaa->thinkview + 1;
}

void Character_SetThinkView(CharacterInfo *chaa, int vii) {
    if (((vii < 2) || (vii > game.numviews)) && (vii != -1))
        quit("!SetCharacterThinkView: invalid view number");

    chaa->thinkview = vii - 1;
}

int Character_GetTransparency(CharacterInfo *chaa) {

    return GfxDef::LegacyTrans255ToTrans100(chaa->transparency);
}

void Character_SetTransparency(CharacterInfo *chaa, int trans) {

    if ((trans < 0) || (trans > 100))
        quit("!SetCharTransparent: transparency value must be between 0 and 100");

    chaa->transparency = GfxDef::Trans100ToLegacyTrans255(trans);
}

int Character_GetTurnBeforeWalking(CharacterInfo *chaa) {

    if (chaa->flags & CHF_NOTURNING)
        return 0;
    return 1;  
}

void Character_SetTurnBeforeWalking(CharacterInfo *chaa, int yesorno) {

    chaa->flags &= ~CHF_NOTURNING;
    if (!yesorno)
        chaa->flags |= CHF_NOTURNING;
}

int Character_GetView(CharacterInfo *chaa) {
    return chaa->view + 1;
}

int Character_GetWalkSpeedX(CharacterInfo *chaa) {
    return chaa->walkspeed;
}

int Character_GetWalkSpeedY(CharacterInfo *chaa) {
    if (chaa->walkspeed_y != UNIFORM_WALK_SPEED)
        return chaa->walkspeed_y;

    return chaa->walkspeed;
}

int Character_GetX(CharacterInfo *chaa) {
    return chaa->x;
}

void Character_SetX(CharacterInfo *chaa, int newval) {
    chaa->x = newval;
}

int Character_GetY(CharacterInfo *chaa) {
    return chaa->y;
}

void Character_SetY(CharacterInfo *chaa, int newval) {
    chaa->y = newval;
}

int Character_GetZ(CharacterInfo *chaa) {
    return chaa->z;
}

void Character_SetZ(CharacterInfo *chaa, int newval) {
    chaa->z = newval;
}

extern int char_speaking;

int Character_GetSpeakingFrame(CharacterInfo *chaa) {

    if ((face_talking >= 0) && (facetalkrepeat))
    {
        if (facetalkchar->index_id == chaa->index_id)
        {
            return facetalkframe;
        }
    }
    else if (char_speaking >= 0)
    {
        if (char_speaking == chaa->index_id)
        {
            return chaa->frame;
        }
    }

    debug_script_warn("Character.SpeakingFrame: character is not currently speaking");
    return -1;
}

//=============================================================================

// order of loops to turn character in circle from down to down
int turnlooporder[8] = {0, 6, 1, 7, 3, 5, 2, 4};

void walk_character(int chac,int tox,int toy,int ignwal, bool autoWalkAnims) {
    CharacterInfo*chin=&game.chars[chac];
    if (chin->room!=displayed_room)
        quit("!MoveCharacter: character not in current room");

    chin->flags &= ~CHF_MOVENOTWALK;

    int toxPassedIn = tox, toyPassedIn = toy;
    int charX = room_to_mask_coord(chin->x);
    int charY = room_to_mask_coord(chin->y);
    tox = room_to_mask_coord(tox);
    toy = room_to_mask_coord(toy);

    if ((tox == charX) && (toy == charY)) {
        StopMoving(chac);
        debug_script_log("%s already at destination, not moving", chin->scrname);
        return;
    }

    if ((chin->animating) && (autoWalkAnims))
        chin->animating = 0;

    if (chin->idleleft < 0) {
        ReleaseCharacterView(chac);
        chin->idleleft=chin->idletime;
    }
    // stop them to make sure they're on a walkable area
    // but save their frame first so that if they're already
    // moving it looks smoother
    int oldframe = chin->frame;
    int waitWas = 0, animWaitWas = 0;
    // if they are currently walking, save the current Wait
    if (chin->walking)
    {
        waitWas = chin->walkwait;
        animWaitWas = charextra[chac].animwait;
    }

    StopMoving (chac);
    chin->frame = oldframe;
    // use toxPassedIn cached variable so the hi-res co-ordinates
    // are still displayed as such
    debug_script_log("%s: Start move to %d,%d", chin->scrname, toxPassedIn, toyPassedIn);

    int move_speed_x = chin->walkspeed;
    int move_speed_y = chin->walkspeed;

    if (chin->walkspeed_y != UNIFORM_WALK_SPEED)
        move_speed_y = chin->walkspeed_y;

    if ((move_speed_x == 0) && (move_speed_y == 0)) {
        debug_script_warn("Warning: MoveCharacter called for '%s' with walk speed 0", chin->name);
    }

    set_route_move_speed(move_speed_x, move_speed_y);
    set_color_depth(8);
    int mslot=find_route(charX, charY, tox, toy, prepare_walkable_areas(chac), chac+CHMLSOFFS, 1, ignwal);
    set_color_depth(game.GetColorDepth());
    if (mslot>0) {
        chin->walking = mslot;
        mls[mslot].direct = ignwal;
        convert_move_path_to_room_resolution(&mls[mslot]);

        // cancel any pending waits on current animations
        // or if they were already moving, keep the current wait - 
        // this prevents a glitch if MoveCharacter is called when they
        // are already moving
        if (autoWalkAnims)
        {
            chin->walkwait = waitWas;
            charextra[chac].animwait = animWaitWas;

            if (mls[mslot].pos[0] != mls[mslot].pos[1]) {
                fix_player_sprite(&mls[mslot],chin);
            }
        }
        else
            chin->flags |= CHF_MOVENOTWALK;
    }
    else if (autoWalkAnims) // pathfinder couldn't get a route, stand them still
        chin->frame = 0;
}

int find_looporder_index (int curloop) {
    int rr;
    for (rr = 0; rr < 8; rr++) {
        if (turnlooporder[rr] == curloop)
            return rr;
    }
    return 0;
}

// returns 0 to use diagonal, 1 to not
int useDiagonal (CharacterInfo *char1) {
    if ((views[char1->view].numLoops < 8) || ((char1->flags & CHF_NODIAGONAL)!=0))
        return 1;
    // If they have just provided standing frames for loops 4-7, to
    // provide smoother turning
    if (views[char1->view].loops[4].numFrames < 2)
        return 2;
    return 0;
}

// returns 1 normally, or 0 if they only have horizontal animations
int hasUpDownLoops(CharacterInfo *char1) {
    // if no loops in the Down animation
    // or no loops in the Up animation
    if ((views[char1->view].loops[0].numFrames < 1) ||
        (views[char1->view].numLoops < 4) ||
        (views[char1->view].loops[3].numFrames < 1))
    {
        return 0;
    }

    return 1;
}

void start_character_turning (CharacterInfo *chinf, int useloop, int no_diagonal) {
    // work out how far round they have to turn 
    int fromidx = find_looporder_index (chinf->loop);
    int toidx = find_looporder_index (useloop);
    //Display("Curloop: %d, needloop: %d",chinf->loop, useloop);
    int ii, go_anticlock = 0;
    // work out whether anticlockwise is quicker or not
    if ((toidx > fromidx) && ((toidx - fromidx) > 4))
        go_anticlock = 1;
    if ((toidx < fromidx) && ((fromidx - toidx) < 4))
        go_anticlock = 1;
    // strip any current turning_around stages
    chinf->walking = chinf->walking % TURNING_AROUND;
    if (go_anticlock)
        chinf->walking += TURNING_BACKWARDS;
    else
        go_anticlock = -1;

    // Allow the diagonal frames just for turning
    if (no_diagonal == 2)
        no_diagonal = 0;

    for (ii = fromidx; ii != toidx; ii -= go_anticlock) {
        if (ii < 0)
            ii = 7;
        if (ii >= 8)
            ii = 0;
        if (ii == toidx)
            break;
        if ((turnlooporder[ii] >= 4) && (no_diagonal > 0))
            continue;
        if (views[chinf->view].loops[turnlooporder[ii]].numFrames < 1)
            continue;
        if (turnlooporder[ii] < views[chinf->view].numLoops)
            chinf->walking += TURNING_AROUND;
    }

}

void fix_player_sprite(MoveList*cmls,CharacterInfo*chinf) {
    const fixed xpmove = cmls->xpermove[cmls->onstage];
    const fixed ypmove = cmls->ypermove[cmls->onstage];

    // if not moving, do nothing
    if ((xpmove == 0) && (ypmove == 0))
        return;

    const int useloop = GetDirectionalLoop(chinf, xpmove, ypmove);

    if ((game.options[OPT_ROTATECHARS] == 0) || ((chinf->flags & CHF_NOTURNING) != 0)) {
        chinf->loop = useloop;
        return;
    }
    if ((chinf->loop > kDirLoop_LastOrthogonal) && ((chinf->flags & CHF_NODIAGONAL)!=0)) {
        // They've just been playing an animation with an extended loop number,
        // so don't try and rotate using it
        chinf->loop = useloop;
        return;
    }
    if ((chinf->loop >= views[chinf->view].numLoops) ||
        (views[chinf->view].loops[chinf->loop].numFrames < 1) ||
        (hasUpDownLoops(chinf) == 0)) {
            // Character is not currently on a valid loop, so don't try to rotate
            // eg. left/right only view, but current loop 0
            chinf->loop = useloop;
            return;
    }
    const int no_diagonal = useDiagonal (chinf);
    start_character_turning (chinf, useloop, no_diagonal);
}

// Check whether two characters have walked into each other
int has_hit_another_character(int sourceChar) {

    // if the character who's moving doesn't Bitmap *, don't bother checking
    if (game.chars[sourceChar].flags & CHF_NOBLOCKING)
        return -1;

    for (int ww = 0; ww < game.numcharacters; ww++) {
        if (game.chars[ww].on != 1) continue;
        if (game.chars[ww].room != displayed_room) continue;
        if (ww == sourceChar) continue;
        if (game.chars[ww].flags & CHF_NOBLOCKING) continue;

        if (is_char_on_another (sourceChar, ww, nullptr, nullptr)) {
            // we are now overlapping character 'ww'
            if ((game.chars[ww].walking) && 
                ((game.chars[ww].flags & CHF_AWAITINGMOVE) == 0))
                return ww;
        }

    }
    return -1;
}

// Does the next move from the character's movelist.
// Returns 1 if they are now waiting for another char to move,
// otherwise returns 0
int doNextCharMoveStep (CharacterInfo *chi, int &char_index, CharacterExtras *chex) {
    int ntf=0, xwas = chi->x, ywas = chi->y;

    if (do_movelist_move(&chi->walking,&chi->x,&chi->y) == 2) 
    {
        if ((chi->flags & CHF_MOVENOTWALK) == 0)
            fix_player_sprite(&mls[chi->walking], chi);
    }

    ntf = has_hit_another_character(char_index);
    if (ntf >= 0) {
        chi->walkwait = 30;
        if (game.chars[ntf].walkspeed < 5)
            chi->walkwait += (5 - game.chars[ntf].walkspeed) * 5;
        // we are now waiting for the other char to move, so
        // make sure he doesn't stop for us too

        chi->flags |= CHF_AWAITINGMOVE;

        if ((chi->flags & CHF_MOVENOTWALK) == 0)
        {
            chi->frame = 0;
            chex->animwait = chi->walkwait;
        }

        if ((chi->walking < 1) || (chi->walking >= TURNING_AROUND)) ;
        else if (mls[chi->walking].onpart > 0) {
            mls[chi->walking].onpart --;
            chi->x = xwas;
            chi->y = ywas;
        }
        debug_script_log("%s: Bumped into %s, waiting for them to move", chi->scrname, game.chars[ntf].scrname);
        return 1;
    }
    return 0;
}

int find_nearest_walkable_area_within(int *xx, int *yy, int range, int step)
{
    int ex, ey, nearest = 99999, thisis, nearx = 0, neary = 0;
    int startx = 0, starty = 14;
    int roomWidthLowRes = room_to_mask_coord(thisroom.Width);
    int roomHeightLowRes = room_to_mask_coord(thisroom.Height);
    int xwidth = roomWidthLowRes, yheight = roomHeightLowRes;

    int xLowRes = room_to_mask_coord(xx[0]);
    int yLowRes = room_to_mask_coord(yy[0]);
    int rightEdge = room_to_mask_coord(thisroom.Edges.Right);
    int leftEdge = room_to_mask_coord(thisroom.Edges.Left);
    int topEdge = room_to_mask_coord(thisroom.Edges.Top);
    int bottomEdge = room_to_mask_coord(thisroom.Edges.Bottom);

    // tweak because people forget to move the edges sometimes
    // if the player is already over the edge, ignore it
    if (xLowRes >= rightEdge) rightEdge = roomWidthLowRes;
    if (xLowRes <= leftEdge) leftEdge = 0;
    if (yLowRes >= bottomEdge) bottomEdge = roomHeightLowRes;
    if (yLowRes <= topEdge) topEdge = 0;

    if (range > 0) 
    {
        startx = xLowRes - range;
        starty = yLowRes - range;
        xwidth = startx + range * 2;
        yheight = starty + range * 2;
        if (startx < 0) startx = 0;
        if (starty < 10) starty = 10;
        if (xwidth > roomWidthLowRes) xwidth = roomWidthLowRes;
        if (yheight > roomHeightLowRes) yheight = roomHeightLowRes;
    }

    for (ex = startx; ex < xwidth; ex += step) {
        for (ey = starty; ey < yheight; ey += step) {
            // non-walkalbe, so don't go here
            if (thisroom.WalkAreaMask->GetPixel(ex,ey) == 0) continue;
            // off a screen edge, don't move them there
            if ((ex <= leftEdge) || (ex >= rightEdge) ||
                (ey <= topEdge) || (ey >= bottomEdge))
                continue;
            // otherwise, calculate distance from target
            thisis=(int) ::sqrt((double)((ex - xLowRes) * (ex - xLowRes) + (ey - yLowRes) * (ey - yLowRes)));
            if (thisis<nearest) { nearest=thisis; nearx=ex; neary=ey; }
        }
    }
    if (nearest < 90000) 
    {
        xx[0] = mask_to_room_coord(nearx);
        yy[0] = mask_to_room_coord(neary);
        return 1;
    }

    return 0;
}

void find_nearest_walkable_area (int *xx, int *yy) {

    int pixValue = thisroom.WalkAreaMask->GetPixel(room_to_mask_coord(xx[0]), room_to_mask_coord(yy[0]));
    // only fix this code if the game was built with 2.61 or above
    if (pixValue == 0 || (loaded_game_file_version >= kGameVersion_261 && pixValue < 1))
    {
        // First, check every 2 pixels within immediate area
        if (!find_nearest_walkable_area_within(xx, yy, 20, 2))
        {
            // If not, check whole screen at 5 pixel intervals
            find_nearest_walkable_area_within(xx, yy, -1, 5);
        }
    }

}

void FindReasonableLoopForCharacter(CharacterInfo *chap) {

    if (chap->loop >= views[chap->view].numLoops)
        chap->loop=kDirLoop_Default;
    if (views[chap->view].numLoops < 1)
        quitprintf("!View %d does not have any loops", chap->view + 1);

    // if the current loop has no frames, find one that does
    if (views[chap->view].loops[chap->loop].numFrames < 1) 
    {
        for (int i = 0; i < views[chap->view].numLoops; i++) 
        {
            if (views[chap->view].loops[i].numFrames > 0) {
                chap->loop = i;
                break;
            }
        }
    }

}

void walk_or_move_character(CharacterInfo *chaa, int x, int y, int blocking, int direct, bool isWalk)
{
    if (chaa->on != 1)
    {
        debug_script_warn("MoveCharacterBlocking: character is turned off and cannot be moved");
        return;
    }

    if ((direct == ANYWHERE) || (direct == 1))
        walk_character(chaa->index_id, x, y, 1, isWalk);
    else if ((direct == WALKABLE_AREAS) || (direct == 0))
        walk_character(chaa->index_id, x, y, 0, isWalk);
    else
        quit("!Character.Walk: Direct must be ANYWHERE or WALKABLE_AREAS");

    if ((blocking == BLOCKING) || (blocking == 1))
        GameLoopUntilNotMoving(&chaa->walking);
    else if ((blocking != IN_BACKGROUND) && (blocking != 0))
        quit("!Character.Walk: Blocking must be BLOCKING or IN_BACKGRUOND");

}

int is_valid_character(int newchar) {
    if ((newchar < 0) || (newchar >= game.numcharacters)) return 0;
    return 1;
}

int wantMoveNow (CharacterInfo *chi, CharacterExtras *chex) {
    // check most likely case first
    if ((chex->zoom == 100) || ((chi->flags & CHF_SCALEMOVESPEED) == 0))
        return 1;

    // the % checks don't work when the counter is negative, so once
    // it wraps round, correct it
    while (chi->walkwaitcounter < 0) {
        chi->walkwaitcounter += 12000;
    }

    // scaling 170-200%, move 175% speed
    if (chex->zoom >= 170) {
        if ((chi->walkwaitcounter % 4) >= 1)
            return 2;
        else
            return 1;
    }
    // scaling 140-170%, move 150% speed
    else if (chex->zoom >= 140) {
        if ((chi->walkwaitcounter % 2) == 1)
            return 2;
        else
            return 1;
    }
    // scaling 115-140%, move 125% speed
    else if (chex->zoom >= 115) {
        if ((chi->walkwaitcounter % 4) >= 3)
            return 2;
        else
            return 1;
    }
    // scaling 80-120%, normal speed
    else if (chex->zoom >= 80)
        return 1;
    // scaling 60-80%, move 75% speed
    if (chex->zoom >= 60) {
        if ((chi->walkwaitcounter % 4) >= 1)
            return -1;
        else if (chex->xwas != INVALID_X) {
            // move the second half of the movement to make it smoother
            chi->x = chex->xwas;
            chi->y = chex->ywas;
            chex->xwas = INVALID_X;
        }
    }
    // scaling 30-60%, move 50% speed
    else if (chex->zoom >= 30) {
        if ((chi->walkwaitcounter % 2) == 1)
            return -1;
        else if (chex->xwas != INVALID_X) {
            // move the second half of the movement to make it smoother
            chi->x = chex->xwas;
            chi->y = chex->ywas;
            chex->xwas = INVALID_X;
        }
    }
    // scaling 0-30%, move 25% speed
    else {
        if ((chi->walkwaitcounter % 4) >= 3)
            return -1;
        if (((chi->walkwaitcounter % 4) == 1) && (chex->xwas != INVALID_X)) {
            // move the second half of the movement to make it smoother
            chi->x = chex->xwas;
            chi->y = chex->ywas;
            chex->xwas = INVALID_X;
        }

    }

    return 0;
}

void setup_player_character(int charid) {
    game.playercharacter = charid;
    playerchar = &game.chars[charid];
    _sc_PlayerCharPtr = ccGetObjectHandleFromAddress((char*)playerchar);
    if (loaded_game_file_version < kGameVersion_270) {
        ccAddExternalDynamicObject("player", playerchar, &ccDynamicCharacter);
    }
}

void animate_character(CharacterInfo *chap, int loopn,int sppd,int rept, int noidleoverride, int direction, int sframe) {

    if ((chap->view < 0) || (chap->view > game.numviews)) {
        quitprintf("!AnimateCharacter: you need to set the view number first\n"
            "(trying to animate '%s' using loop %d. View is currently %d).",chap->name,loopn,chap->view+1);
    }
    debug_script_log("%s: Start anim view %d loop %d, spd %d, repeat %d, frame: %d", chap->scrname, chap->view+1, loopn, sppd, rept, sframe);
    if ((chap->idleleft < 0) && (noidleoverride == 0)) {
        // if idle view in progress for the character (and this is not the
        // "start idle animation" animate_character call), stop the idle anim
        Character_UnlockView(chap);
        chap->idleleft=chap->idletime;
    }
    if ((loopn < 0) || (loopn >= views[chap->view].numLoops))
        quit("!AnimateCharacter: invalid loop number specified");
    if ((sframe < 0) || (sframe >= views[chap->view].loops[loopn].numFrames))
        quit("!AnimateCharacter: invalid starting frame number specified");
    Character_StopMoving(chap);
    chap->animating=1;
    if (rept) chap->animating |= CHANIM_REPEAT;
    if (direction) chap->animating |= CHANIM_BACKWARDS;

    chap->animating|=((sppd << 8) & 0xff00);
    chap->loop=loopn;
    // reverse animation starts at the *previous frame*
    if (direction) {
        sframe--;
        if (sframe < 0)
            sframe = views[chap->view].loops[loopn].numFrames - (-sframe);
    }
    chap->frame = sframe;

    chap->wait = sppd + views[chap->view].loops[loopn].frames[chap->frame].speed;
    CheckViewFrameForCharacter(chap);
}

void CheckViewFrameForCharacter(CharacterInfo *chi) {

    int soundVolume = SCR_NO_VALUE;

    if (chi->flags & CHF_SCALEVOLUME) {
        // adjust the sound volume using the character's zoom level
        int zoom_level = charextra[chi->index_id].zoom;
        if (zoom_level == 0)
            zoom_level = 100;

        soundVolume = zoom_level;

        if (soundVolume < 0)
            soundVolume = 0;
        if (soundVolume > 100)
            soundVolume = 100;
    }

    CheckViewFrame(chi->view, chi->loop, chi->frame, soundVolume);
}

Bitmap *GetCharacterImage(int charid, int *isFlipped) 
{
    if (!gfxDriver->HasAcceleratedTransform())
    {
        if (actsps[charid + MAX_ROOM_OBJECTS] != nullptr) 
        {
            // the actsps image is pre-flipped, so no longer register the image as such
            if (isFlipped)
                *isFlipped = 0;
            return actsps[charid + MAX_ROOM_OBJECTS];
        }
    }
    CharacterInfo*chin=&game.chars[charid];
    int sppic = views[chin->view].loops[chin->loop].frames[chin->frame].pic;
    return spriteset[sppic];
}

CharacterInfo *GetCharacterAtScreen(int xx, int yy) {
    int hsnum = GetCharIDAtScreen(xx, yy);
    if (hsnum < 0)
        return nullptr;
    return &game.chars[hsnum];
}

CharacterInfo *GetCharacterAtRoom(int x, int y)
{
    int hsnum = is_pos_on_character(x, y);
    if (hsnum < 0)
        return nullptr;
    return &game.chars[hsnum];
}

extern int char_lowest_yp, obj_lowest_yp;

int is_pos_on_character(int xx,int yy) {
    int cc,sppic,lowestyp=0,lowestwas=-1;
    for (cc=0;cc<game.numcharacters;cc++) {
        if (game.chars[cc].room!=displayed_room) continue;
        if (game.chars[cc].on==0) continue;
        if (game.chars[cc].flags & CHF_NOINTERACT) continue;
        if (game.chars[cc].view < 0) continue;
        CharacterInfo*chin=&game.chars[cc];

        if ((chin->view < 0) || 
            (chin->loop >= views[chin->view].numLoops) ||
            (chin->frame >= views[chin->view].loops[chin->loop].numFrames))
        {
            continue;
        }

        sppic=views[chin->view].loops[chin->loop].frames[chin->frame].pic;
        int usewid = charextra[cc].width;
        int usehit = charextra[cc].height;
        if (usewid==0) usewid=game.SpriteInfos[sppic].Width;
        if (usehit==0) usehit= game.SpriteInfos[sppic].Height;
        int xxx = chin->x - game_to_data_coord(usewid) / 2;
        int yyy = chin->get_effective_y() - game_to_data_coord(usehit);

        int mirrored = views[chin->view].loops[chin->loop].frames[chin->frame].flags & VFLG_FLIPSPRITE;
        Bitmap *theImage = GetCharacterImage(cc, &mirrored);

        if (is_pos_in_sprite(xx,yy,xxx,yyy, theImage,
            game_to_data_coord(usewid),
            game_to_data_coord(usehit), mirrored) == FALSE)
            continue;

        int use_base = chin->get_baseline();
        if (use_base < lowestyp) continue;
        lowestyp=use_base;
        lowestwas=cc;
    }
    char_lowest_yp = lowestyp;
    return lowestwas;
}

void get_char_blocking_rect(int charid, int *x1, int *y1, int *width, int *y2) {
    CharacterInfo *char1 = &game.chars[charid];
    int cwidth, fromx;

    if (char1->blocking_width < 1)
        cwidth = game_to_data_coord(GetCharacterWidth(charid)) - 4;
    else
        cwidth = char1->blocking_width;

    fromx = char1->x - cwidth/2;
    if (fromx < 0) {
        cwidth += fromx;
        fromx = 0;
    }
    if (fromx + cwidth >= mask_to_room_coord(walkable_areas_temp->GetWidth()))
        cwidth = mask_to_room_coord(walkable_areas_temp->GetWidth()) - fromx;

    if (x1)
        *x1 = fromx;
    if (width)
        *width = cwidth;
    if (y1)
        *y1 = char1->get_blocking_top();
    if (y2)
        *y2 = char1->get_blocking_bottom();
}

// Check whether the source char has walked onto character ww
int is_char_on_another (int sourceChar, int ww, int*fromxptr, int*cwidptr) {

    int fromx, cwidth;
    int y1, y2;
    get_char_blocking_rect(ww, &fromx, &y1, &cwidth, &y2);

    if (fromxptr)
        fromxptr[0] = fromx;
    if (cwidptr)
        cwidptr[0] = cwidth;

    // if the character trying to move is already on top of
    // this char somehow, allow them through
    if ((sourceChar >= 0) &&
        // x/width are left and width co-ords, so they need >= and <
        (game.chars[sourceChar].x >= fromx) &&
        (game.chars[sourceChar].x < fromx + cwidth) &&
        // y1/y2 are the top/bottom co-ords, so they need >= / <=
        (game.chars[sourceChar].y >= y1 ) &&
        (game.chars[sourceChar].y <= y2 ))
        return 1;

    return 0;
}

int my_getpixel(Bitmap *blk, int x, int y) {
    if ((x < 0) || (y < 0) || (x >= blk->GetWidth()) || (y >= blk->GetHeight()))
        return -1;

    // strip the alpha channel
	// TODO: is there a way to do this vtable thing with Bitmap?
	BITMAP *al_bmp = (BITMAP*)blk->GetAllegroBitmap();
    return al_bmp->vtable->getpixel(al_bmp, x, y) & 0x00ffffff;
}

int check_click_on_character(int xx,int yy,int mood) {
    int lowestwas=is_pos_on_character(xx,yy);
    if (lowestwas>=0) {
        RunCharacterInteraction (lowestwas, mood);
        return 1;
    }
    return 0;
}

void _DisplaySpeechCore(int chid, const char *displbuf) {
    if (displbuf[0] == 0) {
        // no text, just update the current character who's speaking
        // this allows the portrait side to be switched with an empty
        // speech line
        play.swap_portrait_lastchar = chid;
        return;
    }

    // adjust timing of text (so that DisplaySpeech("%s", str) pauses
    // for the length of the string not 2 frames)
    int len = (int)strlen(displbuf);
    if (len > source_text_length + 3)
        source_text_length = len;

    DisplaySpeech(displbuf, chid);
}

void _DisplayThoughtCore(int chid, const char *displbuf) {
    // adjust timing of text (so that DisplayThought("%s", str) pauses
    // for the length of the string not 2 frames)
    int len = (int)strlen(displbuf);
    if (len > source_text_length + 3)
        source_text_length = len;

    int xpp = -1, ypp = -1, width = -1;

    if ((game.options[OPT_SPEECHTYPE] == 0) || (game.chars[chid].thinkview <= 0)) {
        // lucasarts-style, so we want a speech bubble actually above
        // their head (or if they have no think anim in Sierra-style)
        width = data_to_game_coord(play.speech_bubble_width);
        xpp = play.RoomToScreenX(data_to_game_coord(game.chars[chid].x)) - width / 2;
        if (xpp < 0)
            xpp = 0;
        // -1 will automatically put it above the char's head
        ypp = -1;
    }

    _displayspeech ((char*)displbuf, chid, xpp, ypp, width, 1);
}

void _displayspeech(const char*texx, int aschar, int xx, int yy, int widd, int isThought) {
    if (!is_valid_character(aschar))
        quit("!DisplaySpeech: invalid character");

    CharacterInfo *speakingChar = &game.chars[aschar];
    if ((speakingChar->view < 0) || (speakingChar->view >= game.numviews))
        quit("!DisplaySpeech: character has invalid view");

    if (is_text_overlay > 0)
    {
        debug_script_warn("DisplaySpeech: speech was already displayed (nested DisplaySpeech, perhaps room script and global script conflict?)");
        return;
    }

    EndSkippingUntilCharStops();

    said_speech_line = 1;

    if (play.bgspeech_stay_on_display == 0) {
        // remove any background speech
        for (size_t i = 0; i < screenover.size();) {
            if (screenover[i].timeout > 0)
                remove_screen_overlay(screenover[i].type);
            else
                i++;
        }
    }
    said_text = 1;

    // the strings are pre-translated
    //texx = get_translation(texx);
    our_eip=150;

    int isPause = 1;
    // if the message is all .'s, don't display anything
    for (int aa = 0; texx[aa] != 0; aa++) {
        if (texx[aa] != '.') {
            isPause = 0;
            break;
        }
    }

    play.messagetime = GetTextDisplayTime(texx);
    play.speech_in_post_state = false;

    if (isPause) {
        postpone_scheduled_music_update_by(std::chrono::milliseconds(play.messagetime * 1000 / frames_per_second));
        GameLoopUntilValueIsNegative(&play.messagetime);
        return;
    }

    int textcol = speakingChar->talkcolor;

    // if it's 0, it won't be recognised as speech
    if (textcol == 0)
        textcol = 16;

    Rect ui_view = play.GetUIViewport();
    int allowShrink = 0;
    int bwidth = widd;
    if (bwidth < 0)
        bwidth = ui_view.GetWidth()/2 + ui_view.GetWidth()/4;

    our_eip=151;

    int useview = speakingChar->talkview;
    if (isThought) {
        useview = speakingChar->thinkview;
        // view 0 is not valid for think views
        if (useview == 0)
            useview = -1;
        // speech bubble can shrink to fit
        allowShrink = 1;
        if (speakingChar->room != displayed_room) {
            // not in room, centre it
            xx = -1;
            yy = -1;
        }
    }

    if (useview >= game.numviews)
        quitprintf("!Character.Say: attempted to use view %d for animation, but it does not exist", useview + 1);

    int tdxp = xx,tdyp = yy;
    int oldview=-1, oldloop = -1;
    int ovr_type = 0;

    text_lips_offset = 0;
    text_lips_text = texx;

    Bitmap *closeupface=nullptr;
    // TODO: we always call _display_at later which may also start voice-over;
    // find out if this may be refactored and voice started only in one place.
    try_auto_play_speech(texx, texx, aschar, true);

    if (game.options[OPT_SPEECHTYPE] == 3)
        remove_screen_overlay(OVER_COMPLETE);
    our_eip=1500;

    if (game.options[OPT_SPEECHTYPE] == 0)
        allowShrink = 1;

    if (speakingChar->idleleft < 0)  {
        // if idle anim in progress for the character, stop it
        ReleaseCharacterView(aschar);
        //    speakingChar->idleleft = speakingChar->idletime;
    }

    bool overlayPositionFixed = false;
    int charFrameWas = 0;
    int viewWasLocked = 0;
    if (speakingChar->flags & CHF_FIXVIEW)
        viewWasLocked = 1;

    /*if ((speakingChar->room == displayed_room) ||
    ((useview >= 0) && (game.options[OPT_SPEECHTYPE] > 0)) ) {*/

    if (speakingChar->room == displayed_room) {
        // If the character is in this room, go for it - otherwise
        // run the "else" clause which  does text in the middle of
        // the screen.
        our_eip=1501;

        if (speakingChar->walking)
            StopMoving(aschar);

        // save the frame we need to go back to
        // if they were moving, this will be 0 (because we just called
        // StopMoving); otherwise, it might be a specific animation 
        // frame which we should return to
        if (viewWasLocked)
            charFrameWas = speakingChar->frame;

        // if the current loop doesn't exist in talking view, use loop 0
        if (speakingChar->loop >= views[speakingChar->view].numLoops)
            speakingChar->loop = 0;

        if ((speakingChar->view < 0) || 
            (speakingChar->loop >= views[speakingChar->view].numLoops) ||
            (views[speakingChar->view].loops[speakingChar->loop].numFrames < 1))
        {
            quitprintf("Unable to display speech because the character %s has an invalid view frame (View %d, loop %d, frame %d)", speakingChar->scrname, speakingChar->view + 1, speakingChar->loop, speakingChar->frame);
        }

        our_eip=1504;

        // Calculate speech position based on character's position on screen
        auto view = FindNearestViewport(aschar);
        if (tdxp < 0)
            tdxp = view->RoomToScreen(data_to_game_coord(speakingChar->x), 0).first.X;
        if (tdxp < 2)
            tdxp = 2;
        tdxp = -tdxp;  // tell it to centre it ([ikm] not sure what's going on here... wrong comment?)

        if (tdyp < 0)
        {
            int sppic = views[speakingChar->view].loops[speakingChar->loop].frames[0].pic;
            int height = (charextra[aschar].height < 1) ? game.SpriteInfos[sppic].Height : height = charextra[aschar].height;
            tdyp = view->RoomToScreen(0, data_to_game_coord(game.chars[aschar].get_effective_y()) - height).first.Y
                    - get_fixed_pixel_size(5);
            if (isThought) // if it's a thought, lift it a bit further up
                tdyp -= get_fixed_pixel_size(10);
        }
        if (tdyp < 5)
            tdyp = 5;

        our_eip=152;

        if ((useview >= 0) && (game.options[OPT_SPEECHTYPE] > 0)) {
            // Sierra-style close-up portrait

            if (play.swap_portrait_lastchar != aschar) {
                // if the portraits are set to Alternate, OR they are
                // set to Left but swap_portrait has been set to 1 (the old
                // method for enabling it), then swap them round
                if ((game.options[OPT_PORTRAITSIDE] == PORTRAIT_ALTERNATE) ||
                    ((game.options[OPT_PORTRAITSIDE] == 0) &&
                    (play.swap_portrait_side > 0))) {

                        if (play.swap_portrait_side == 2)
                            play.swap_portrait_side = 1;
                        else
                            play.swap_portrait_side = 2;
                }

                if (game.options[OPT_PORTRAITSIDE] == PORTRAIT_XPOSITION) {
                    // Portrait side based on character X-positions
                    if (play.swap_portrait_lastchar < 0) {
                        // No previous character been spoken to
                        // therefore, assume it's the player
                        if(game.playercharacter != aschar && game.chars[game.playercharacter].room == speakingChar->room && game.chars[game.playercharacter].on == 1)
                            play.swap_portrait_lastchar = game.playercharacter;
                        else
                            // The player's not here. Find another character in this room
                            // that it could be
                            for (int ce = 0; ce < game.numcharacters; ce++) {
                                if ((game.chars[ce].room == speakingChar->room) &&
                                    (game.chars[ce].on == 1) &&
                                    (ce != aschar)) {
                                        play.swap_portrait_lastchar = ce;
                                        break;
                                }
                            }
                    }

                    if (play.swap_portrait_lastchar >= 0) {
                        // if this character is right of the one before, put the
                        // portrait on the right
                        if (speakingChar->x > game.chars[play.swap_portrait_lastchar].x)
                            play.swap_portrait_side = -1;
                        else
                            play.swap_portrait_side = 0;
                    }
                }
                play.swap_portrait_lastlastchar = play.swap_portrait_lastchar;
                play.swap_portrait_lastchar = aschar;
            }
            else
                // If the portrait side is based on the character's X position and the same character is
                // speaking, compare against the previous *previous* character to see where the speech should be
                if (game.options[OPT_PORTRAITSIDE] == PORTRAIT_XPOSITION && play.swap_portrait_lastlastchar >= 0) {
                    if (speakingChar->x > game.chars[play.swap_portrait_lastlastchar].x)
                        play.swap_portrait_side = -1;
                    else
                        play.swap_portrait_side = 0;
                }

            // Determine whether to display the portrait on the left or right
            int portrait_on_right = 0;

            if (game.options[OPT_SPEECHTYPE] == 3) 
            { }  // always on left with QFG-style speech
            else if ((play.swap_portrait_side == 1) ||
                (play.swap_portrait_side == -1) ||
                (game.options[OPT_PORTRAITSIDE] == PORTRAIT_RIGHT))
                portrait_on_right = 1;


            int bigx=0,bigy=0,kk;
            ViewStruct*viptr=&views[useview];
            for (kk = 0; kk < viptr->loops[0].numFrames; kk++) 
            {
                int tw = game.SpriteInfos[viptr->loops[0].frames[kk].pic].Width;
                if (tw > bigx) bigx=tw;
                tw = game.SpriteInfos[viptr->loops[0].frames[kk].pic].Height;
                if (tw > bigy) bigy=tw;
            }

            // if they accidentally used a large full-screen image as the sierra-style
            // talk view, correct it
            if ((game.options[OPT_SPEECHTYPE] != 3) && (bigx > ui_view.GetWidth() - get_fixed_pixel_size(50)))
                bigx = ui_view.GetWidth() - get_fixed_pixel_size(50);

            if (widd > 0)
                bwidth = widd - bigx;

            our_eip=153;
            int ovr_yp = get_fixed_pixel_size(20);
            int view_frame_x = 0;
            int view_frame_y = 0;
            facetalk_qfg4_override_placement_x = false;
            facetalk_qfg4_override_placement_y = false;

            if (game.options[OPT_SPEECHTYPE] == 3) {
                // QFG4-style whole screen picture
                closeupface = BitmapHelper::CreateBitmap(ui_view.GetWidth(), ui_view.GetHeight(), spriteset[viptr->loops[0].frames[0].pic]->GetColorDepth());
                closeupface->Clear(0);
                if (xx < 0 && play.speech_portrait_placement)
                {
                    facetalk_qfg4_override_placement_x = true;
                    view_frame_x = play.speech_portrait_x;
                }
                if (yy < 0 && play.speech_portrait_placement)
                {
                    facetalk_qfg4_override_placement_y = true;
                    view_frame_y = play.speech_portrait_y;
                }
                else
                {
                    view_frame_y = ui_view.GetHeight()/2 - game.SpriteInfos[viptr->loops[0].frames[0].pic].Height/2;
                }
                bigx = ui_view.GetWidth()/2 - get_fixed_pixel_size(20);
                ovr_type = OVER_COMPLETE;
                ovr_yp = 0;
                tdyp = -1;  // center vertically
            }
            else {
                // KQ6-style close-up face picture
                if (yy < 0 && play.speech_portrait_placement)
                {
                    ovr_yp = play.speech_portrait_y;
                }
                else if (yy < 0)
                    ovr_yp = adjust_y_for_guis (ovr_yp);
                else
                    ovr_yp = yy;

                closeupface = BitmapHelper::CreateTransparentBitmap(bigx+1,bigy+1,spriteset[viptr->loops[0].frames[0].pic]->GetColorDepth());
                ovr_type = OVER_PICTURE;

                if (yy < 0)
                    tdyp = ovr_yp + get_textwindow_top_border_height(play.speech_textwindow_gui);
            }
            const ViewFrame *vf = &viptr->loops[0].frames[0];
            const bool closeupface_has_alpha = (game.SpriteInfos[vf->pic].Flags & SPF_ALPHACHANNEL) != 0;
            DrawViewFrame(closeupface, vf, view_frame_x, view_frame_y);

            int overlay_x = get_fixed_pixel_size(10);
            if (xx < 0) {
                tdxp = bigx + get_textwindow_border_width(play.speech_textwindow_gui) / 2;
                if (play.speech_portrait_placement)
                {
                    overlay_x = play.speech_portrait_x;
                    tdxp += overlay_x + get_fixed_pixel_size(6);
                }
                else
                {
                    tdxp += get_fixed_pixel_size(16);
                }

                int maxWidth = (ui_view.GetWidth() - tdxp) - get_fixed_pixel_size(5) -
                    get_textwindow_border_width (play.speech_textwindow_gui) / 2;

                if (bwidth > maxWidth)
                    bwidth = maxWidth;
            }
            else {
                tdxp = xx + bigx + get_fixed_pixel_size(8);
                overlay_x = xx;
            }

            // allow the text box to be shrunk to fit the text
            allowShrink = 1;

            // if the portrait's on the right, swap it round
            if (portrait_on_right) {
                if ((xx < 0) || (widd < 0)) {
                    tdxp = get_fixed_pixel_size(9);
                    if (play.speech_portrait_placement)
                    {
                        overlay_x = (ui_view.GetWidth() - bigx) - play.speech_portrait_x;
                        int maxWidth = overlay_x - tdxp - get_fixed_pixel_size(9) - 
                            get_textwindow_border_width (play.speech_textwindow_gui) / 2;
                        if (bwidth > maxWidth)
                            bwidth = maxWidth;
                    }
                    else
                    {
                        overlay_x = (ui_view.GetWidth() - bigx) - get_fixed_pixel_size(5);
                    }
                }
                else {
                    overlay_x = (xx + widd - bigx) - get_fixed_pixel_size(5);
                    tdxp = xx;
                }
                tdxp += get_textwindow_border_width(play.speech_textwindow_gui) / 2;
                allowShrink = 2;
            }
            if (game.options[OPT_SPEECHTYPE] == 3)
                overlay_x = 0;
            face_talking=add_screen_overlay(overlay_x,ovr_yp,ovr_type,closeupface, closeupface_has_alpha);
            facetalkframe = 0;
            facetalkwait = viptr->loops[0].frames[0].speed + GetCharacterSpeechAnimationDelay(speakingChar);
            facetalkloop = 0;
            facetalkview = useview;
            facetalkrepeat = (isThought) ? 0 : 1;
            facetalkBlinkLoop = 0;
            facetalkAllowBlink = 1;
            if ((isThought) && (speakingChar->flags & CHF_NOBLINKANDTHINK))
                facetalkAllowBlink = 0;
            facetalkchar = &game.chars[aschar];
            if (facetalkchar->blinktimer < 0)
                facetalkchar->blinktimer = facetalkchar->blinkinterval;
            textcol=-textcol;
            overlayPositionFixed = true;
        }
        else if (useview >= 0) {
            // Lucasarts-style speech
            our_eip=154;

            oldview = speakingChar->view;
            oldloop = speakingChar->loop;
            speakingChar->animating = 1 | (GetCharacterSpeechAnimationDelay(speakingChar) << 8);
            // only repeat if speech, not thought
            if (!isThought)
                speakingChar->animating |= CHANIM_REPEAT;

            speakingChar->view = useview;
            speakingChar->frame=0;
            speakingChar->flags|=CHF_FIXVIEW;

            if (speakingChar->loop >= views[speakingChar->view].numLoops)
            {
                // current character loop is outside the normal talking directions
                speakingChar->loop = 0;
            }

            facetalkBlinkLoop = speakingChar->loop;

            if (speakingChar->on && // don't bother checking if character is not visible (also fixes 'Trilby's Notes' legacy game)
                ((speakingChar->loop >= views[speakingChar->view].numLoops) ||
                (views[speakingChar->view].loops[speakingChar->loop].numFrames < 1)))
            {
                quitprintf("!Unable to display speech because the character %s has an invalid speech view (View %d, loop %d, frame %d)", speakingChar->scrname, speakingChar->view + 1, speakingChar->loop, speakingChar->frame);
            }

            // set up the speed of the first frame
            speakingChar->wait = GetCharacterSpeechAnimationDelay(speakingChar) + 
                views[speakingChar->view].loops[speakingChar->loop].frames[0].speed;

            if (widd < 0) {
                bwidth = ui_view.GetWidth()/2 + ui_view.GetWidth()/6;
                // If they are close to the screen edge, make the text narrower
                int relx = play.RoomToScreenX(data_to_game_coord(speakingChar->x));
                if ((relx < ui_view.GetWidth() / 4) || (relx > ui_view.GetWidth() - (ui_view.GetWidth() / 4)))
                    bwidth -= ui_view.GetWidth() / 5;
            }
            /*   this causes the text to bob up and down as they talk
            tdxp = OVR_AUTOPLACE;
            tdyp = aschar;*/
            if (!isThought)  // set up the lip sync if not thinking
                char_speaking = aschar;

        }
    }
    else
        allowShrink = 1;

    // it wants the centred position, so make it so
    if ((xx >= 0) && (tdxp < 0))
        tdxp -= widd / 2;

    // if they used DisplaySpeechAt, then use the supplied width
    if ((widd > 0) && (isThought == 0))
        allowShrink = 0;

    if (isThought)
        char_thinking = aschar;

    our_eip=155;
    _display_at(tdxp, tdyp, bwidth, texx, DISPLAYTEXT_SPEECH, textcol, isThought, allowShrink, overlayPositionFixed);
    our_eip=156;
    if ((play.in_conversation > 0) && (game.options[OPT_SPEECHTYPE] == 3))
        closeupface = nullptr;
    if (closeupface!=nullptr)
        remove_screen_overlay(ovr_type);
    mark_screen_dirty();
    face_talking = -1;
    facetalkchar = nullptr;
    our_eip=157;
    if (oldview>=0) {
        speakingChar->flags &= ~CHF_FIXVIEW;
        if (viewWasLocked)
            speakingChar->flags |= CHF_FIXVIEW;
        speakingChar->view=oldview;

        // Don't reset the loop in 2.x games
        if (loaded_game_file_version > kGameVersion_272)
            speakingChar->loop = oldloop;

        speakingChar->animating=0;
        speakingChar->frame = charFrameWas;
        speakingChar->wait=0;
        speakingChar->idleleft = speakingChar->idletime;
        // restart the idle animation straight away
        charextra[aschar].process_idle_this_time = 1;
    }
    char_speaking = -1;
    char_thinking = -1;
    if (play.IsBlockingVoiceSpeech())
        stop_voice_speech();
}

int get_character_currently_talking() {
    if ((face_talking >= 0) && (facetalkrepeat))
        return facetalkchar->index_id;
    else if (char_speaking >= 0)
        return char_speaking;

    return -1;
}

void DisplaySpeech(const char*texx, int aschar) {
    _displayspeech (texx, aschar, -1, -1, -1, 0);
}

// Calculate which frame of the loop to use for this character of
// speech
int GetLipSyncFrame (const char *curtex, int *stroffs) {
    /*char *frameletters[MAXLIPSYNCFRAMES] =
    {"./,/ ", "A", "O", "F/V", "D/N/G/L/R", "B/P/M",
    "Y/H/K/Q/C", "I/T/E/X/th", "U/W", "S/Z/J/ch", NULL,
    NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};*/

    int bestfit_len = 0, bestfit = game.default_lipsync_frame;
    for (int aa = 0; aa < MAXLIPSYNCFRAMES; aa++) {
        char *tptr = game.lipSyncFrameLetters[aa];
        while (tptr[0] != 0) {
            int lenthisbit = strlen(tptr);
            if (strchr(tptr, '/'))
                lenthisbit = strchr(tptr, '/') - tptr;

            if ((ags_strnicmp (curtex, tptr, lenthisbit) == 0) && (lenthisbit > bestfit_len)) {
                bestfit = aa;
                bestfit_len = lenthisbit;
            }
            tptr += lenthisbit;
            while (tptr[0] == '/')
                tptr++;
        }
    }
    // If it's an unknown character, use the default frame
    if (bestfit_len == 0)
        bestfit_len = 1;
    *stroffs += bestfit_len;
    return bestfit;
}

int update_lip_sync(int talkview, int talkloop, int *talkframeptr) {
    int talkframe = talkframeptr[0];
    int talkwait = 0;

    // lip-sync speech
    const char *nowsaying = &text_lips_text[text_lips_offset];
    // if it's an apostraphe, skip it (we'll, I'll, etc)
    if (nowsaying[0] == '\'') {
        text_lips_offset++;
        nowsaying++;
    }

    if (text_lips_offset >= (int)strlen(text_lips_text))
        talkframe = 0;
    else {
        talkframe = GetLipSyncFrame (nowsaying, &text_lips_offset);
        if (talkframe >= views[talkview].loops[talkloop].numFrames)
            talkframe = 0;
    }

    talkwait = loops_per_character + views[talkview].loops[talkloop].frames[talkframe].speed;

    talkframeptr[0] = talkframe;
    return talkwait;
}

Rect GetCharacterRoomBBox(int charid, bool use_frame_0)
{
    int width, height;
    const CharacterExtras& chex = charextra[charid];
    const CharacterInfo& chin = game.chars[charid];
    int frame = use_frame_0 ? 0 : chin.frame;
    int pic = views[chin.view].loops[chin.loop].frames[frame].pic;
    scale_sprite_size(pic, chex.zoom, &width, &height);
    return RectWH(chin.x - width / 2, chin.y - height, width, height);
}

PViewport FindNearestViewport(int charid)
{
    Rect bbox = GetCharacterRoomBBox(charid, true);
    float min_dist = -1.f;
    PViewport nearest_view;
    for (int i = 0; i < play.GetRoomViewportCount(); ++i)
    {
        auto view = play.GetRoomViewport(i);
        if (!view->IsVisible())
            continue;
        auto cam = view->GetCamera();
        if (!cam)
            continue;
        Rect camr = cam->GetRect();
        float dist = DistanceBetween(bbox, camr);
        if (dist == 0.f)
            return view;
        if (min_dist < 0.f || dist < min_dist)
        {
            min_dist = dist;
            nearest_view = view;
        }
    }
    return nearest_view ? nearest_view : play.GetRoomViewport(0);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "debug/out.h"
#include "script/script_api.h"
#include "script/script_runtime.h"
#include "ac/dynobj/scriptstring.h"

extern ScriptString myScriptStringImpl;

// void | CharacterInfo *chaa, ScriptInvItem *invi, int addIndex
RuntimeScriptValue Sc_Character_AddInventory(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ_PINT(CharacterInfo, Character_AddInventory, ScriptInvItem);
}

// void | CharacterInfo *chaa, int x, int y
RuntimeScriptValue Sc_Character_AddWaypoint(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_AddWaypoint);
}

// void | CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction
RuntimeScriptValue Sc_Character_Animate(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT5(CharacterInfo, Character_Animate);
}

RuntimeScriptValue Sc_Character_AnimateFrom(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT6(CharacterInfo, Character_AnimateFrom);
}

// void | CharacterInfo *chaa, int room, int x, int y
RuntimeScriptValue Sc_Character_ChangeRoom(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_ChangeRoom);
}

RuntimeScriptValue Sc_Character_ChangeRoomSetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_ChangeRoomSetLoop);
}

// void | CharacterInfo *chaa, int room, int newPos
RuntimeScriptValue Sc_Character_ChangeRoomAutoPosition(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_ChangeRoomAutoPosition);
}

// void | CharacterInfo *chap, int vii
RuntimeScriptValue Sc_Character_ChangeView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_ChangeView);
}

// void | CharacterInfo *char1, CharacterInfo *char2, int blockingStyle
RuntimeScriptValue Sc_Character_FaceCharacter(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ_PINT(CharacterInfo, Character_FaceCharacter, CharacterInfo);
}

// void | CharacterInfo *char1, int direction, int blockingStyle
RuntimeScriptValue Sc_Character_FaceDirection(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_FaceDirection);
}

// void | CharacterInfo *char1, int xx, int yy, int blockingStyle
RuntimeScriptValue Sc_Character_FaceLocation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_FaceLocation);
}

// void | CharacterInfo *char1, ScriptObject *obj, int blockingStyle
RuntimeScriptValue Sc_Character_FaceObject(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ_PINT(CharacterInfo, Character_FaceObject, ScriptObject);
}

// void | CharacterInfo *chaa, CharacterInfo *tofollow, int distaway, int eagerness
RuntimeScriptValue Sc_Character_FollowCharacter(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ_PINT2(CharacterInfo, Character_FollowCharacter, CharacterInfo);
}

// int (CharacterInfo *chaa, const char *property)
RuntimeScriptValue Sc_Character_GetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT_POBJ(CharacterInfo, Character_GetProperty, const char);
}

// void (CharacterInfo *chaa, const char *property, char *bufer)
RuntimeScriptValue Sc_Character_GetPropertyText(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ2(CharacterInfo, Character_GetPropertyText, const char, char);
}

// const char* (CharacterInfo *chaa, const char *property)
RuntimeScriptValue Sc_Character_GetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJ_POBJ(CharacterInfo, const char, myScriptStringImpl, Character_GetTextProperty, const char);
}

RuntimeScriptValue Sc_Character_SetProperty(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_BOOL_POBJ_PINT(CharacterInfo, Character_SetProperty, const char);
}

RuntimeScriptValue Sc_Character_SetTextProperty(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_BOOL_POBJ2(CharacterInfo, Character_SetTextProperty, const char, const char);
}

// int (CharacterInfo *chaa, ScriptInvItem *invi)
RuntimeScriptValue Sc_Character_HasInventory(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT_POBJ(CharacterInfo, Character_HasInventory, ScriptInvItem);
}

// int (CharacterInfo *char1, CharacterInfo *char2)
RuntimeScriptValue Sc_Character_IsCollidingWithChar(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT_POBJ(CharacterInfo, Character_IsCollidingWithChar, CharacterInfo);
}

// int (CharacterInfo *chin, ScriptObject *objid)
RuntimeScriptValue Sc_Character_IsCollidingWithObject(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT_POBJ(CharacterInfo, Character_IsCollidingWithObject, ScriptObject);
}

RuntimeScriptValue Sc_Character_IsInteractionAvailable(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_BOOL_PINT(CharacterInfo, Character_IsInteractionAvailable);
}

// void (CharacterInfo *chap, int vii)
RuntimeScriptValue Sc_Character_LockView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_LockView);
}

// void (CharacterInfo *chap, int vii, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewEx(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_LockViewEx);
}

// void (CharacterInfo *chap, int vii, int loop, int align)
RuntimeScriptValue Sc_Character_LockViewAligned_Old(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewAligned_Old);
}

// void (CharacterInfo *chap, int vii, int loop, int align, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewAlignedEx_Old(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewAlignedEx_Old);
}

RuntimeScriptValue Sc_Character_LockViewAligned(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewAligned);
}

RuntimeScriptValue Sc_Character_LockViewAlignedEx(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewAlignedEx);
}

// void (CharacterInfo *chaa, int view, int loop, int frame)
RuntimeScriptValue Sc_Character_LockViewFrame(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewFrame);
}

// void (CharacterInfo *chaa, int view, int loop, int frame, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewFrameEx(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewFrameEx);
}

// void (CharacterInfo *chap, int vii, int xoffs, int yoffs)
RuntimeScriptValue Sc_Character_LockViewOffset(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_LockViewOffset);
}

// void (CharacterInfo *chap, int vii, int xoffs, int yoffs, int stopMoving)
RuntimeScriptValue Sc_Character_LockViewOffsetEx(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_LockViewOffsetEx);
}

// void (CharacterInfo *chap, ScriptInvItem *invi)
RuntimeScriptValue Sc_Character_LoseInventory(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ(CharacterInfo, Character_LoseInventory, ScriptInvItem);
}

// void (CharacterInfo *chaa, int x, int y, int blocking, int direct) 
RuntimeScriptValue Sc_Character_Move(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_Move);
}

// void (CharacterInfo *chap) 
RuntimeScriptValue Sc_Character_PlaceOnWalkableArea(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(CharacterInfo, Character_PlaceOnWalkableArea);
}

// void (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_RemoveTint(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(CharacterInfo, Character_RemoveTint);
}

// void (CharacterInfo *chaa, int mood)
RuntimeScriptValue Sc_Character_RunInteraction(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_RunInteraction);
}

// void (CharacterInfo *chaa, const char *texx, ...)
RuntimeScriptValue Sc_Character_Say(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_SCRIPT_SPRINTF(Character_Say, 1);
    Character_Say((CharacterInfo*)self, scsf_buffer);
    return RuntimeScriptValue((int32_t)0);
}

// void (CharacterInfo *chaa, int x, int y, int width, const char *texx)
RuntimeScriptValue Sc_Character_SayAt(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3_POBJ(CharacterInfo, Character_SayAt, const char);
}

// ScriptOverlay* (CharacterInfo *chaa, const char *texx)
RuntimeScriptValue Sc_Character_SayBackground(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJAUTO_POBJ(CharacterInfo, ScriptOverlay, Character_SayBackground, const char);
}

// void (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_SetAsPlayer(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(CharacterInfo, Character_SetAsPlayer);
}

// void (CharacterInfo *chaa, int iview, int itime)
RuntimeScriptValue Sc_Character_SetIdleView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_SetIdleView);
}

RuntimeScriptValue Sc_Character_HasExplicitLight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_BOOL(CharacterInfo, Character_GetHasExplicitLight);
}

RuntimeScriptValue Sc_Character_GetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetLightLevel);
}

RuntimeScriptValue Sc_Character_SetLightLevel(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetLightLevel);
}

RuntimeScriptValue Sc_Character_GetTintBlue(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTintBlue);
}

RuntimeScriptValue Sc_Character_GetTintGreen(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTintGreen);
}

RuntimeScriptValue Sc_Character_GetTintRed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTintRed);
}

RuntimeScriptValue Sc_Character_GetTintSaturation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTintSaturation);
}

RuntimeScriptValue Sc_Character_GetTintLuminance(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTintLuminance);
}

/*
RuntimeScriptValue Sc_Character_SetOption(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
}
*/

// void (CharacterInfo *chaa, int xspeed, int yspeed)
RuntimeScriptValue Sc_Character_SetSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_SetSpeed);
}

// void (CharacterInfo *charp)
RuntimeScriptValue Sc_Character_StopMoving(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(CharacterInfo, Character_StopMoving);
}

// void (CharacterInfo *chaa, const char *texx, ...)
RuntimeScriptValue Sc_Character_Think(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_SCRIPT_SPRINTF(Character_Think, 1);
    Character_Think((CharacterInfo*)self, scsf_buffer);
    return RuntimeScriptValue((int32_t)0);
}

//void (CharacterInfo *chaa, int red, int green, int blue, int opacity, int luminance)
RuntimeScriptValue Sc_Character_Tint(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT5(CharacterInfo, Character_Tint);
}

// void (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_UnlockView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID(CharacterInfo, Character_UnlockView);
}

// void (CharacterInfo *chaa, int stopMoving)
RuntimeScriptValue Sc_Character_UnlockViewEx(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_UnlockViewEx);
}

// void (CharacterInfo *chaa, int x, int y, int blocking, int direct)
RuntimeScriptValue Sc_Character_Walk(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT4(CharacterInfo, Character_Walk);
}

// void (CharacterInfo *chaa, int xx, int yy, int blocking)
RuntimeScriptValue Sc_Character_WalkStraight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT3(CharacterInfo, Character_WalkStraight);
}

RuntimeScriptValue Sc_GetCharacterAtRoom(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJ_PINT2(CharacterInfo, ccDynamicCharacter, GetCharacterAtRoom);
}

// CharacterInfo *(int xx, int yy)
RuntimeScriptValue Sc_GetCharacterAtScreen(const RuntimeScriptValue *params, int32_t param_count)
{
    API_SCALL_OBJ_PINT2(CharacterInfo, ccDynamicCharacter, GetCharacterAtScreen);
}

// ScriptInvItem* (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetActiveInventory(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJ(CharacterInfo, ScriptInvItem, ccDynamicInv, Character_GetActiveInventory);
}

// void (CharacterInfo *chaa, ScriptInvItem* iit)
RuntimeScriptValue Sc_Character_SetActiveInventory(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ(CharacterInfo, Character_SetActiveInventory, ScriptInvItem);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetAnimating(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetAnimating);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetAnimationSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetAnimationSpeed);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetAnimationSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetAnimationSpeed);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBaseline(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetBaseline);
}

// void (CharacterInfo *chaa, int basel)
RuntimeScriptValue Sc_Character_SetBaseline(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBaseline);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlinkInterval(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetBlinkInterval);
}

// void (CharacterInfo *chaa, int interval)
RuntimeScriptValue Sc_Character_SetBlinkInterval(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlinkInterval);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlinkView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetBlinkView);
}

// void (CharacterInfo *chaa, int vii)
RuntimeScriptValue Sc_Character_SetBlinkView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlinkView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlinkWhileThinking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetBlinkWhileThinking);
}

// void (CharacterInfo *chaa, int yesOrNo)
RuntimeScriptValue Sc_Character_SetBlinkWhileThinking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlinkWhileThinking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlockingHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetBlockingHeight);
}

// void (CharacterInfo *chaa, int hit)
RuntimeScriptValue Sc_Character_SetBlockingHeight(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlockingHeight);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetBlockingWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetBlockingWidth);
}

// void (CharacterInfo *chaa, int wid)
RuntimeScriptValue Sc_Character_SetBlockingWidth(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetBlockingWidth);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetClickable);
}

// void (CharacterInfo *chaa, int clik)
RuntimeScriptValue Sc_Character_SetClickable(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetClickable);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetDiagonalWalking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetDiagonalWalking);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetDiagonalWalking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetDiagonalWalking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetFrame);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetFrame(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetFrame);
}

RuntimeScriptValue Sc_Character_GetHasExplicitTint_Old(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetHasExplicitTint_Old);
}

RuntimeScriptValue Sc_Character_GetHasExplicitTint(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetHasExplicitTint);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetID);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIdleView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetIdleView);
}

// int (CharacterInfo *chaa, int index)
RuntimeScriptValue Sc_Character_GetIInventoryQuantity(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT_PINT(CharacterInfo, Character_GetIInventoryQuantity);
}

// void (CharacterInfo *chaa, int index, int quant)
RuntimeScriptValue Sc_Character_SetIInventoryQuantity(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT2(CharacterInfo, Character_SetIInventoryQuantity);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIgnoreLighting(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetIgnoreLighting);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetIgnoreLighting(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIgnoreLighting);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIgnoreScaling(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetIgnoreScaling);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetIgnoreScaling(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIgnoreScaling);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetIgnoreWalkbehinds(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetIgnoreWalkbehinds);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetIgnoreWalkbehinds(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetIgnoreWalkbehinds);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetLoop);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetLoop(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetLoop);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetManualScaling(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetManualScaling);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetMovementLinkedToAnimation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetMovementLinkedToAnimation);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetMovementLinkedToAnimation(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetMovementLinkedToAnimation);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetMoving(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetMoving);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetDestinationX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetDestinationX);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetDestinationY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetDestinationY);
}

// const char* (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetName(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_OBJ(CharacterInfo, const char, myScriptStringImpl, Character_GetName);
}

// void (CharacterInfo *chaa, const char *newName)
RuntimeScriptValue Sc_Character_SetName(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_POBJ(CharacterInfo, Character_SetName, const char);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetNormalView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetNormalView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetPreviousRoom(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetPreviousRoom);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetRoom(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetRoom);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetScaleMoveSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetScaleMoveSpeed);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetScaleMoveSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetScaleMoveSpeed);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetScaleVolume(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetScaleVolume);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetScaleVolume(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetScaleVolume);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetScaling(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetScaling);
}

// void (CharacterInfo *chaa, int zoomlevel)
RuntimeScriptValue Sc_Character_SetScaling(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetScaling);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSolid(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetSolid);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetSolid(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSolid);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeaking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetSpeaking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeakingFrame(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetSpeakingFrame);
}

// int (CharacterInfo *cha)
RuntimeScriptValue Sc_GetCharacterSpeechAnimationDelay(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, GetCharacterSpeechAnimationDelay);
}

// void (CharacterInfo *chaa, int newDelay)
RuntimeScriptValue Sc_Character_SetSpeechAnimationDelay(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSpeechAnimationDelay);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeechColor(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetSpeechColor);
}

// void (CharacterInfo *chaa, int ncol)
RuntimeScriptValue Sc_Character_SetSpeechColor(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSpeechColor);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetSpeechView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetSpeechView);
}

// void (CharacterInfo *chaa, int vii)
RuntimeScriptValue Sc_Character_SetSpeechView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetSpeechView);
}

RuntimeScriptValue Sc_Character_GetThinking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_BOOL(CharacterInfo, Character_GetThinking);
}

RuntimeScriptValue Sc_Character_GetThinkingFrame(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetThinkingFrame);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetThinkView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetThinkView);
}

// void (CharacterInfo *chaa, int vii)
RuntimeScriptValue Sc_Character_SetThinkView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetThinkView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTransparency);
}

// void (CharacterInfo *chaa, int trans)
RuntimeScriptValue Sc_Character_SetTransparency(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetTransparency);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetTurnBeforeWalking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetTurnBeforeWalking);
}

// void (CharacterInfo *chaa, int yesorno)
RuntimeScriptValue Sc_Character_SetTurnBeforeWalking(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetTurnBeforeWalking);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetView(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetView);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetWalkSpeedX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetWalkSpeedX);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetWalkSpeedY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetWalkSpeedY);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetX);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetX(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetX);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetY);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetY(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetY);
}

// int (CharacterInfo *chaa)
RuntimeScriptValue Sc_Character_GetZ(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_INT(CharacterInfo, Character_GetZ);
}

// void (CharacterInfo *chaa, int newval)
RuntimeScriptValue Sc_Character_SetZ(void *self, const RuntimeScriptValue *params, int32_t param_count)
{
    API_OBJCALL_VOID_PINT(CharacterInfo, Character_SetZ);
}

//=============================================================================
//
// Exclusive API for Plugins
//
//=============================================================================

// void (CharacterInfo *chaa, const char *texx, ...)
void ScPl_Character_Say(CharacterInfo *chaa, const char *texx, ...)
{
    API_PLUGIN_SCRIPT_SPRINTF(texx);
    Character_Say(chaa, scsf_buffer);
}

// void (CharacterInfo *chaa, const char *texx, ...)
void ScPl_Character_Think(CharacterInfo *chaa, const char *texx, ...)
{
    API_PLUGIN_SCRIPT_SPRINTF(texx);
    Character_Think(chaa, scsf_buffer);
}

void RegisterCharacterAPI(ScriptAPIVersion base_api, ScriptAPIVersion compat_api)
{
    ccAddExternalObjectFunction("Character::AddInventory^2",            Sc_Character_AddInventory);
	ccAddExternalObjectFunction("Character::AddWaypoint^2",             Sc_Character_AddWaypoint);
	ccAddExternalObjectFunction("Character::Animate^5",                 Sc_Character_Animate);
    ccAddExternalObjectFunction("Character::Animate^6",                 Sc_Character_AnimateFrom);
	ccAddExternalObjectFunction("Character::ChangeRoom^3",              Sc_Character_ChangeRoom);
    ccAddExternalObjectFunction("Character::ChangeRoom^4",              Sc_Character_ChangeRoomSetLoop);
	ccAddExternalObjectFunction("Character::ChangeRoomAutoPosition^2",  Sc_Character_ChangeRoomAutoPosition);
	ccAddExternalObjectFunction("Character::ChangeView^1",              Sc_Character_ChangeView);
	ccAddExternalObjectFunction("Character::FaceCharacter^2",           Sc_Character_FaceCharacter);
	ccAddExternalObjectFunction("Character::FaceDirection^2",           Sc_Character_FaceDirection);
	ccAddExternalObjectFunction("Character::FaceLocation^3",            Sc_Character_FaceLocation);
	ccAddExternalObjectFunction("Character::FaceObject^2",              Sc_Character_FaceObject);
	ccAddExternalObjectFunction("Character::FollowCharacter^3",         Sc_Character_FollowCharacter);
	ccAddExternalObjectFunction("Character::GetProperty^1",             Sc_Character_GetProperty);
	ccAddExternalObjectFunction("Character::GetPropertyText^2",         Sc_Character_GetPropertyText);
	ccAddExternalObjectFunction("Character::GetTextProperty^1",         Sc_Character_GetTextProperty);
    ccAddExternalObjectFunction("Character::SetProperty^2",             Sc_Character_SetProperty);
    ccAddExternalObjectFunction("Character::SetTextProperty^2",         Sc_Character_SetTextProperty);
	ccAddExternalObjectFunction("Character::HasInventory^1",            Sc_Character_HasInventory);
	ccAddExternalObjectFunction("Character::IsCollidingWithChar^1",     Sc_Character_IsCollidingWithChar);
	ccAddExternalObjectFunction("Character::IsCollidingWithObject^1",   Sc_Character_IsCollidingWithObject);
    ccAddExternalObjectFunction("Character::IsInteractionAvailable^1",  Sc_Character_IsInteractionAvailable);
	ccAddExternalObjectFunction("Character::LockView^1",                Sc_Character_LockView);
	ccAddExternalObjectFunction("Character::LockView^2",                Sc_Character_LockViewEx);
    if (base_api < kScriptAPI_v350)
    {
        ccAddExternalObjectFunction("Character::LockViewAligned^3", Sc_Character_LockViewAligned_Old);
        ccAddExternalObjectFunction("Character::LockViewAligned^4", Sc_Character_LockViewAlignedEx_Old);
    }
    else
    {
        ccAddExternalObjectFunction("Character::LockViewAligned^3", Sc_Character_LockViewAligned);
        ccAddExternalObjectFunction("Character::LockViewAligned^4", Sc_Character_LockViewAlignedEx);
    }
	ccAddExternalObjectFunction("Character::LockViewFrame^3",           Sc_Character_LockViewFrame);
	ccAddExternalObjectFunction("Character::LockViewFrame^4",           Sc_Character_LockViewFrameEx);
	ccAddExternalObjectFunction("Character::LockViewOffset^3",          Sc_Character_LockViewOffset);
	ccAddExternalObjectFunction("Character::LockViewOffset^4",          Sc_Character_LockViewOffsetEx);
	ccAddExternalObjectFunction("Character::LoseInventory^1",           Sc_Character_LoseInventory);
	ccAddExternalObjectFunction("Character::Move^4",                    Sc_Character_Move);
	ccAddExternalObjectFunction("Character::PlaceOnWalkableArea^0",     Sc_Character_PlaceOnWalkableArea);
	ccAddExternalObjectFunction("Character::RemoveTint^0",              Sc_Character_RemoveTint);
	ccAddExternalObjectFunction("Character::RunInteraction^1",          Sc_Character_RunInteraction);
	ccAddExternalObjectFunction("Character::Say^101",                   Sc_Character_Say);
	ccAddExternalObjectFunction("Character::SayAt^4",                   Sc_Character_SayAt);
	ccAddExternalObjectFunction("Character::SayBackground^1",           Sc_Character_SayBackground);
	ccAddExternalObjectFunction("Character::SetAsPlayer^0",             Sc_Character_SetAsPlayer);
	ccAddExternalObjectFunction("Character::SetIdleView^2",             Sc_Character_SetIdleView);
    ccAddExternalObjectFunction("Character::SetLightLevel^1",           Sc_Character_SetLightLevel);
	//ccAddExternalObjectFunction("Character::SetOption^2",             Sc_Character_SetOption);
	ccAddExternalObjectFunction("Character::SetWalkSpeed^2",            Sc_Character_SetSpeed);
	ccAddExternalObjectFunction("Character::StopMoving^0",              Sc_Character_StopMoving);
	ccAddExternalObjectFunction("Character::Think^101",                 Sc_Character_Think);
	ccAddExternalObjectFunction("Character::Tint^5",                    Sc_Character_Tint);
	ccAddExternalObjectFunction("Character::UnlockView^0",              Sc_Character_UnlockView);
	ccAddExternalObjectFunction("Character::UnlockView^1",              Sc_Character_UnlockViewEx);
	ccAddExternalObjectFunction("Character::Walk^4",                    Sc_Character_Walk);
	ccAddExternalObjectFunction("Character::WalkStraight^3",            Sc_Character_WalkStraight);

    ccAddExternalStaticFunction("Character::GetAtRoomXY^2",             Sc_GetCharacterAtRoom);
	ccAddExternalStaticFunction("Character::GetAtScreenXY^2",           Sc_GetCharacterAtScreen);

	ccAddExternalObjectFunction("Character::get_ActiveInventory",       Sc_Character_GetActiveInventory);
	ccAddExternalObjectFunction("Character::set_ActiveInventory",       Sc_Character_SetActiveInventory);
	ccAddExternalObjectFunction("Character::get_Animating",             Sc_Character_GetAnimating);
	ccAddExternalObjectFunction("Character::get_AnimationSpeed",        Sc_Character_GetAnimationSpeed);
	ccAddExternalObjectFunction("Character::set_AnimationSpeed",        Sc_Character_SetAnimationSpeed);
	ccAddExternalObjectFunction("Character::get_Baseline",              Sc_Character_GetBaseline);
	ccAddExternalObjectFunction("Character::set_Baseline",              Sc_Character_SetBaseline);
	ccAddExternalObjectFunction("Character::get_BlinkInterval",         Sc_Character_GetBlinkInterval);
	ccAddExternalObjectFunction("Character::set_BlinkInterval",         Sc_Character_SetBlinkInterval);
	ccAddExternalObjectFunction("Character::get_BlinkView",             Sc_Character_GetBlinkView);
	ccAddExternalObjectFunction("Character::set_BlinkView",             Sc_Character_SetBlinkView);
	ccAddExternalObjectFunction("Character::get_BlinkWhileThinking",    Sc_Character_GetBlinkWhileThinking);
	ccAddExternalObjectFunction("Character::set_BlinkWhileThinking",    Sc_Character_SetBlinkWhileThinking);
	ccAddExternalObjectFunction("Character::get_BlockingHeight",        Sc_Character_GetBlockingHeight);
	ccAddExternalObjectFunction("Character::set_BlockingHeight",        Sc_Character_SetBlockingHeight);
	ccAddExternalObjectFunction("Character::get_BlockingWidth",         Sc_Character_GetBlockingWidth);
	ccAddExternalObjectFunction("Character::set_BlockingWidth",         Sc_Character_SetBlockingWidth);
	ccAddExternalObjectFunction("Character::get_Clickable",             Sc_Character_GetClickable);
	ccAddExternalObjectFunction("Character::set_Clickable",             Sc_Character_SetClickable);
	ccAddExternalObjectFunction("Character::get_DestinationX",          Sc_Character_GetDestinationX);
	ccAddExternalObjectFunction("Character::get_DestinationY",          Sc_Character_GetDestinationY);
	ccAddExternalObjectFunction("Character::get_DiagonalLoops",         Sc_Character_GetDiagonalWalking);
	ccAddExternalObjectFunction("Character::set_DiagonalLoops",         Sc_Character_SetDiagonalWalking);
	ccAddExternalObjectFunction("Character::get_Frame",                 Sc_Character_GetFrame);
	ccAddExternalObjectFunction("Character::set_Frame",                 Sc_Character_SetFrame);
    if (base_api < kScriptAPI_v341)
        ccAddExternalObjectFunction("Character::get_HasExplicitTint",       Sc_Character_GetHasExplicitTint_Old);
    else
	    ccAddExternalObjectFunction("Character::get_HasExplicitTint",       Sc_Character_GetHasExplicitTint);
	ccAddExternalObjectFunction("Character::get_ID",                    Sc_Character_GetID);
	ccAddExternalObjectFunction("Character::get_IdleView",              Sc_Character_GetIdleView);
	ccAddExternalObjectFunction("Character::geti_InventoryQuantity",    Sc_Character_GetIInventoryQuantity);
	ccAddExternalObjectFunction("Character::seti_InventoryQuantity",    Sc_Character_SetIInventoryQuantity);
	ccAddExternalObjectFunction("Character::get_IgnoreLighting",        Sc_Character_GetIgnoreLighting);
	ccAddExternalObjectFunction("Character::set_IgnoreLighting",        Sc_Character_SetIgnoreLighting);
	ccAddExternalObjectFunction("Character::get_IgnoreScaling",         Sc_Character_GetIgnoreScaling);
	ccAddExternalObjectFunction("Character::set_IgnoreScaling",         Sc_Character_SetIgnoreScaling);
	ccAddExternalObjectFunction("Character::get_IgnoreWalkbehinds",     Sc_Character_GetIgnoreWalkbehinds);
	ccAddExternalObjectFunction("Character::set_IgnoreWalkbehinds",     Sc_Character_SetIgnoreWalkbehinds);
	ccAddExternalObjectFunction("Character::get_Loop",                  Sc_Character_GetLoop);
	ccAddExternalObjectFunction("Character::set_Loop",                  Sc_Character_SetLoop);
	ccAddExternalObjectFunction("Character::get_ManualScaling",         Sc_Character_GetIgnoreScaling);
	ccAddExternalObjectFunction("Character::set_ManualScaling",         Sc_Character_SetManualScaling);
	ccAddExternalObjectFunction("Character::get_MovementLinkedToAnimation",Sc_Character_GetMovementLinkedToAnimation);
	ccAddExternalObjectFunction("Character::set_MovementLinkedToAnimation",Sc_Character_SetMovementLinkedToAnimation);
	ccAddExternalObjectFunction("Character::get_Moving",                Sc_Character_GetMoving);
	ccAddExternalObjectFunction("Character::get_Name",                  Sc_Character_GetName);
	ccAddExternalObjectFunction("Character::set_Name",                  Sc_Character_SetName);
	ccAddExternalObjectFunction("Character::get_NormalView",            Sc_Character_GetNormalView);
	ccAddExternalObjectFunction("Character::get_PreviousRoom",          Sc_Character_GetPreviousRoom);
	ccAddExternalObjectFunction("Character::get_Room",                  Sc_Character_GetRoom);
	ccAddExternalObjectFunction("Character::get_ScaleMoveSpeed",        Sc_Character_GetScaleMoveSpeed);
	ccAddExternalObjectFunction("Character::set_ScaleMoveSpeed",        Sc_Character_SetScaleMoveSpeed);
	ccAddExternalObjectFunction("Character::get_ScaleVolume",           Sc_Character_GetScaleVolume);
	ccAddExternalObjectFunction("Character::set_ScaleVolume",           Sc_Character_SetScaleVolume);
	ccAddExternalObjectFunction("Character::get_Scaling",               Sc_Character_GetScaling);
	ccAddExternalObjectFunction("Character::set_Scaling",               Sc_Character_SetScaling);
	ccAddExternalObjectFunction("Character::get_Solid",                 Sc_Character_GetSolid);
	ccAddExternalObjectFunction("Character::set_Solid",                 Sc_Character_SetSolid);
	ccAddExternalObjectFunction("Character::get_Speaking",              Sc_Character_GetSpeaking);
	ccAddExternalObjectFunction("Character::get_SpeakingFrame",         Sc_Character_GetSpeakingFrame);
	ccAddExternalObjectFunction("Character::get_SpeechAnimationDelay",  Sc_GetCharacterSpeechAnimationDelay);
	ccAddExternalObjectFunction("Character::set_SpeechAnimationDelay",  Sc_Character_SetSpeechAnimationDelay);
	ccAddExternalObjectFunction("Character::get_SpeechColor",           Sc_Character_GetSpeechColor);
	ccAddExternalObjectFunction("Character::set_SpeechColor",           Sc_Character_SetSpeechColor);
	ccAddExternalObjectFunction("Character::get_SpeechView",            Sc_Character_GetSpeechView);
	ccAddExternalObjectFunction("Character::set_SpeechView",            Sc_Character_SetSpeechView);
    ccAddExternalObjectFunction("Character::get_Thinking",              Sc_Character_GetThinking);
    ccAddExternalObjectFunction("Character::get_ThinkingFrame",         Sc_Character_GetThinkingFrame);
	ccAddExternalObjectFunction("Character::get_ThinkView",             Sc_Character_GetThinkView);
	ccAddExternalObjectFunction("Character::set_ThinkView",             Sc_Character_SetThinkView);
	ccAddExternalObjectFunction("Character::get_Transparency",          Sc_Character_GetTransparency);
	ccAddExternalObjectFunction("Character::set_Transparency",          Sc_Character_SetTransparency);
	ccAddExternalObjectFunction("Character::get_TurnBeforeWalking",     Sc_Character_GetTurnBeforeWalking);
	ccAddExternalObjectFunction("Character::set_TurnBeforeWalking",     Sc_Character_SetTurnBeforeWalking);
	ccAddExternalObjectFunction("Character::get_View",                  Sc_Character_GetView);
	ccAddExternalObjectFunction("Character::get_WalkSpeedX",            Sc_Character_GetWalkSpeedX);
	ccAddExternalObjectFunction("Character::get_WalkSpeedY",            Sc_Character_GetWalkSpeedY);
	ccAddExternalObjectFunction("Character::get_X",                     Sc_Character_GetX);
	ccAddExternalObjectFunction("Character::set_X",                     Sc_Character_SetX);
	ccAddExternalObjectFunction("Character::get_x",                     Sc_Character_GetX);
	ccAddExternalObjectFunction("Character::set_x",                     Sc_Character_SetX);
	ccAddExternalObjectFunction("Character::get_Y",                     Sc_Character_GetY);
	ccAddExternalObjectFunction("Character::set_Y",                     Sc_Character_SetY);
	ccAddExternalObjectFunction("Character::get_y",                     Sc_Character_GetY);
	ccAddExternalObjectFunction("Character::set_y",                     Sc_Character_SetY);
	ccAddExternalObjectFunction("Character::get_Z",                     Sc_Character_GetZ);
	ccAddExternalObjectFunction("Character::set_Z",                     Sc_Character_SetZ);
	ccAddExternalObjectFunction("Character::get_z",                     Sc_Character_GetZ);
	ccAddExternalObjectFunction("Character::set_z",                     Sc_Character_SetZ);

    ccAddExternalObjectFunction("Character::get_HasExplicitLight",      Sc_Character_HasExplicitLight);
    ccAddExternalObjectFunction("Character::get_LightLevel",            Sc_Character_GetLightLevel);
    ccAddExternalObjectFunction("Character::get_TintBlue",              Sc_Character_GetTintBlue);
    ccAddExternalObjectFunction("Character::get_TintGreen",             Sc_Character_GetTintGreen);
    ccAddExternalObjectFunction("Character::get_TintRed",               Sc_Character_GetTintRed);
    ccAddExternalObjectFunction("Character::get_TintSaturation",        Sc_Character_GetTintSaturation);
    ccAddExternalObjectFunction("Character::get_TintLuminance",         Sc_Character_GetTintLuminance);

    /* ----------------------- Registering unsafe exports for plugins -----------------------*/

    ccAddExternalFunctionForPlugin("Character::AddInventory^2",            (void*)Character_AddInventory);
    ccAddExternalFunctionForPlugin("Character::AddWaypoint^2",             (void*)Character_AddWaypoint);
    ccAddExternalFunctionForPlugin("Character::Animate^5",                 (void*)Character_Animate);
    ccAddExternalFunctionForPlugin("Character::ChangeRoom^3",              (void*)Character_ChangeRoom);
    ccAddExternalFunctionForPlugin("Character::ChangeRoomAutoPosition^2",  (void*)Character_ChangeRoomAutoPosition);
    ccAddExternalFunctionForPlugin("Character::ChangeView^1",              (void*)Character_ChangeView);
    ccAddExternalFunctionForPlugin("Character::FaceCharacter^2",           (void*)Character_FaceCharacter);
    ccAddExternalFunctionForPlugin("Character::FaceDirection^2",           (void*)Character_FaceDirection);
    ccAddExternalFunctionForPlugin("Character::FaceLocation^3",            (void*)Character_FaceLocation);
    ccAddExternalFunctionForPlugin("Character::FaceObject^2",              (void*)Character_FaceObject);
    ccAddExternalFunctionForPlugin("Character::FollowCharacter^3",         (void*)Character_FollowCharacter);
    ccAddExternalFunctionForPlugin("Character::GetProperty^1",             (void*)Character_GetProperty);
    ccAddExternalFunctionForPlugin("Character::GetPropertyText^2",         (void*)Character_GetPropertyText);
    ccAddExternalFunctionForPlugin("Character::GetTextProperty^1",         (void*)Character_GetTextProperty);
    ccAddExternalFunctionForPlugin("Character::HasInventory^1",            (void*)Character_HasInventory);
    ccAddExternalFunctionForPlugin("Character::IsCollidingWithChar^1",     (void*)Character_IsCollidingWithChar);
    ccAddExternalFunctionForPlugin("Character::IsCollidingWithObject^1",   (void*)Character_IsCollidingWithObject);
    ccAddExternalFunctionForPlugin("Character::LockView^1",                (void*)Character_LockView);
    ccAddExternalFunctionForPlugin("Character::LockView^2",                (void*)Character_LockViewEx);
    if (base_api < kScriptAPI_v341)
    {
        ccAddExternalFunctionForPlugin("Character::LockViewAligned^3", (void*)Character_LockViewAligned_Old);
        ccAddExternalFunctionForPlugin("Character::LockViewAligned^4", (void*)Character_LockViewAlignedEx_Old);
    }
    else
    {
        ccAddExternalFunctionForPlugin("Character::LockViewAligned^3", (void*)Character_LockViewAligned);
        ccAddExternalFunctionForPlugin("Character::LockViewAligned^4", (void*)Character_LockViewAlignedEx);
    }
    ccAddExternalFunctionForPlugin("Character::LockViewFrame^3",           (void*)Character_LockViewFrame);
    ccAddExternalFunctionForPlugin("Character::LockViewFrame^4",           (void*)Character_LockViewFrameEx);
    ccAddExternalFunctionForPlugin("Character::LockViewOffset^3",          (void*)Character_LockViewOffset);
    ccAddExternalFunctionForPlugin("Character::LockViewOffset^4",          (void*)Character_LockViewOffset);
    ccAddExternalFunctionForPlugin("Character::LoseInventory^1",           (void*)Character_LoseInventory);
    ccAddExternalFunctionForPlugin("Character::Move^4",                    (void*)Character_Move);
    ccAddExternalFunctionForPlugin("Character::PlaceOnWalkableArea^0",     (void*)Character_PlaceOnWalkableArea);
    ccAddExternalFunctionForPlugin("Character::RemoveTint^0",              (void*)Character_RemoveTint);
    ccAddExternalFunctionForPlugin("Character::RunInteraction^1",          (void*)Character_RunInteraction);
    ccAddExternalFunctionForPlugin("Character::Say^101",                   (void*)ScPl_Character_Say);
    ccAddExternalFunctionForPlugin("Character::SayAt^4",                   (void*)Character_SayAt);
    ccAddExternalFunctionForPlugin("Character::SayBackground^1",           (void*)Character_SayBackground);
    ccAddExternalFunctionForPlugin("Character::SetAsPlayer^0",             (void*)Character_SetAsPlayer);
    ccAddExternalFunctionForPlugin("Character::SetIdleView^2",             (void*)Character_SetIdleView);
    //ccAddExternalFunctionForPlugin("Character::SetOption^2",             (void*)Character_SetOption);
    ccAddExternalFunctionForPlugin("Character::SetWalkSpeed^2",            (void*)Character_SetSpeed);
    ccAddExternalFunctionForPlugin("Character::StopMoving^0",              (void*)Character_StopMoving);
    ccAddExternalFunctionForPlugin("Character::Think^101",                 (void*)ScPl_Character_Think);
    ccAddExternalFunctionForPlugin("Character::Tint^5",                    (void*)Character_Tint);
    ccAddExternalFunctionForPlugin("Character::UnlockView^0",              (void*)Character_UnlockView);
    ccAddExternalFunctionForPlugin("Character::UnlockView^1",              (void*)Character_UnlockViewEx);
    ccAddExternalFunctionForPlugin("Character::Walk^4",                    (void*)Character_Walk);
    ccAddExternalFunctionForPlugin("Character::WalkStraight^3",            (void*)Character_WalkStraight);
    ccAddExternalFunctionForPlugin("Character::GetAtRoomXY^2",             (void*)GetCharacterAtRoom);
    ccAddExternalFunctionForPlugin("Character::GetAtScreenXY^2",           (void*)GetCharacterAtScreen);
    ccAddExternalFunctionForPlugin("Character::get_ActiveInventory",       (void*)Character_GetActiveInventory);
    ccAddExternalFunctionForPlugin("Character::set_ActiveInventory",       (void*)Character_SetActiveInventory);
    ccAddExternalFunctionForPlugin("Character::get_Animating",             (void*)Character_GetAnimating);
    ccAddExternalFunctionForPlugin("Character::get_AnimationSpeed",        (void*)Character_GetAnimationSpeed);
    ccAddExternalFunctionForPlugin("Character::set_AnimationSpeed",        (void*)Character_SetAnimationSpeed);
    ccAddExternalFunctionForPlugin("Character::get_Baseline",              (void*)Character_GetBaseline);
    ccAddExternalFunctionForPlugin("Character::set_Baseline",              (void*)Character_SetBaseline);
    ccAddExternalFunctionForPlugin("Character::get_BlinkInterval",         (void*)Character_GetBlinkInterval);
    ccAddExternalFunctionForPlugin("Character::set_BlinkInterval",         (void*)Character_SetBlinkInterval);
    ccAddExternalFunctionForPlugin("Character::get_BlinkView",             (void*)Character_GetBlinkView);
    ccAddExternalFunctionForPlugin("Character::set_BlinkView",             (void*)Character_SetBlinkView);
    ccAddExternalFunctionForPlugin("Character::get_BlinkWhileThinking",    (void*)Character_GetBlinkWhileThinking);
    ccAddExternalFunctionForPlugin("Character::set_BlinkWhileThinking",    (void*)Character_SetBlinkWhileThinking);
    ccAddExternalFunctionForPlugin("Character::get_BlockingHeight",        (void*)Character_GetBlockingHeight);
    ccAddExternalFunctionForPlugin("Character::set_BlockingHeight",        (void*)Character_SetBlockingHeight);
    ccAddExternalFunctionForPlugin("Character::get_BlockingWidth",         (void*)Character_GetBlockingWidth);
    ccAddExternalFunctionForPlugin("Character::set_BlockingWidth",         (void*)Character_SetBlockingWidth);
    ccAddExternalFunctionForPlugin("Character::get_Clickable",             (void*)Character_GetClickable);
    ccAddExternalFunctionForPlugin("Character::set_Clickable",             (void*)Character_SetClickable);
    ccAddExternalFunctionForPlugin("Character::get_DestinationX",          (void*)Character_GetDestinationX);
    ccAddExternalFunctionForPlugin("Character::get_DestinationY",          (void*)Character_GetDestinationY);
    ccAddExternalFunctionForPlugin("Character::get_DiagonalLoops",         (void*)Character_GetDiagonalWalking);
    ccAddExternalFunctionForPlugin("Character::set_DiagonalLoops",         (void*)Character_SetDiagonalWalking);
    ccAddExternalFunctionForPlugin("Character::get_Frame",                 (void*)Character_GetFrame);
    ccAddExternalFunctionForPlugin("Character::set_Frame",                 (void*)Character_SetFrame);
    if (base_api < kScriptAPI_v341)
        ccAddExternalFunctionForPlugin("Character::get_HasExplicitTint",       (void*)Character_GetHasExplicitTint_Old);
    else
        ccAddExternalFunctionForPlugin("Character::get_HasExplicitTint",       (void*)Character_GetHasExplicitTint);
    ccAddExternalFunctionForPlugin("Character::get_ID",                    (void*)Character_GetID);
    ccAddExternalFunctionForPlugin("Character::get_IdleView",              (void*)Character_GetIdleView);
    ccAddExternalFunctionForPlugin("Character::geti_InventoryQuantity",    (void*)Character_GetIInventoryQuantity);
    ccAddExternalFunctionForPlugin("Character::seti_InventoryQuantity",    (void*)Character_SetIInventoryQuantity);
    ccAddExternalFunctionForPlugin("Character::get_IgnoreLighting",        (void*)Character_GetIgnoreLighting);
    ccAddExternalFunctionForPlugin("Character::set_IgnoreLighting",        (void*)Character_SetIgnoreLighting);
    ccAddExternalFunctionForPlugin("Character::get_IgnoreScaling",         (void*)Character_GetIgnoreScaling);
    ccAddExternalFunctionForPlugin("Character::set_IgnoreScaling",         (void*)Character_SetIgnoreScaling);
    ccAddExternalFunctionForPlugin("Character::get_IgnoreWalkbehinds",     (void*)Character_GetIgnoreWalkbehinds);
    ccAddExternalFunctionForPlugin("Character::set_IgnoreWalkbehinds",     (void*)Character_SetIgnoreWalkbehinds);
    ccAddExternalFunctionForPlugin("Character::get_Loop",                  (void*)Character_GetLoop);
    ccAddExternalFunctionForPlugin("Character::set_Loop",                  (void*)Character_SetLoop);
    ccAddExternalFunctionForPlugin("Character::get_ManualScaling",         (void*)Character_GetIgnoreScaling);
    ccAddExternalFunctionForPlugin("Character::set_ManualScaling",         (void*)Character_SetManualScaling);
    ccAddExternalFunctionForPlugin("Character::get_MovementLinkedToAnimation",(void*)Character_GetMovementLinkedToAnimation);
    ccAddExternalFunctionForPlugin("Character::set_MovementLinkedToAnimation",(void*)Character_SetMovementLinkedToAnimation);
    ccAddExternalFunctionForPlugin("Character::get_Moving",                (void*)Character_GetMoving);
    ccAddExternalFunctionForPlugin("Character::get_Name",                  (void*)Character_GetName);
    ccAddExternalFunctionForPlugin("Character::set_Name",                  (void*)Character_SetName);
    ccAddExternalFunctionForPlugin("Character::get_NormalView",            (void*)Character_GetNormalView);
    ccAddExternalFunctionForPlugin("Character::get_PreviousRoom",          (void*)Character_GetPreviousRoom);
    ccAddExternalFunctionForPlugin("Character::get_Room",                  (void*)Character_GetRoom);
    ccAddExternalFunctionForPlugin("Character::get_ScaleMoveSpeed",        (void*)Character_GetScaleMoveSpeed);
    ccAddExternalFunctionForPlugin("Character::set_ScaleMoveSpeed",        (void*)Character_SetScaleMoveSpeed);
    ccAddExternalFunctionForPlugin("Character::get_ScaleVolume",           (void*)Character_GetScaleVolume);
    ccAddExternalFunctionForPlugin("Character::set_ScaleVolume",           (void*)Character_SetScaleVolume);
    ccAddExternalFunctionForPlugin("Character::get_Scaling",               (void*)Character_GetScaling);
    ccAddExternalFunctionForPlugin("Character::set_Scaling",               (void*)Character_SetScaling);
    ccAddExternalFunctionForPlugin("Character::get_Solid",                 (void*)Character_GetSolid);
    ccAddExternalFunctionForPlugin("Character::set_Solid",                 (void*)Character_SetSolid);
    ccAddExternalFunctionForPlugin("Character::get_Speaking",              (void*)Character_GetSpeaking);
    ccAddExternalFunctionForPlugin("Character::get_SpeakingFrame",         (void*)Character_GetSpeakingFrame);
    ccAddExternalFunctionForPlugin("Character::get_SpeechAnimationDelay",  (void*)GetCharacterSpeechAnimationDelay);
    ccAddExternalFunctionForPlugin("Character::set_SpeechAnimationDelay",  (void*)Character_SetSpeechAnimationDelay);
    ccAddExternalFunctionForPlugin("Character::get_SpeechColor",           (void*)Character_GetSpeechColor);
    ccAddExternalFunctionForPlugin("Character::set_SpeechColor",           (void*)Character_SetSpeechColor);
    ccAddExternalFunctionForPlugin("Character::get_SpeechView",            (void*)Character_GetSpeechView);
    ccAddExternalFunctionForPlugin("Character::set_SpeechView",            (void*)Character_SetSpeechView);
    ccAddExternalFunctionForPlugin("Character::get_ThinkView",             (void*)Character_GetThinkView);
    ccAddExternalFunctionForPlugin("Character::set_ThinkView",             (void*)Character_SetThinkView);
    ccAddExternalFunctionForPlugin("Character::get_Transparency",          (void*)Character_GetTransparency);
    ccAddExternalFunctionForPlugin("Character::set_Transparency",          (void*)Character_SetTransparency);
    ccAddExternalFunctionForPlugin("Character::get_TurnBeforeWalking",     (void*)Character_GetTurnBeforeWalking);
    ccAddExternalFunctionForPlugin("Character::set_TurnBeforeWalking",     (void*)Character_SetTurnBeforeWalking);
    ccAddExternalFunctionForPlugin("Character::get_View",                  (void*)Character_GetView);
    ccAddExternalFunctionForPlugin("Character::get_WalkSpeedX",            (void*)Character_GetWalkSpeedX);
    ccAddExternalFunctionForPlugin("Character::get_WalkSpeedY",            (void*)Character_GetWalkSpeedY);
    ccAddExternalFunctionForPlugin("Character::get_X",                     (void*)Character_GetX);
    ccAddExternalFunctionForPlugin("Character::set_X",                     (void*)Character_SetX);
    ccAddExternalFunctionForPlugin("Character::get_x",                     (void*)Character_GetX);
    ccAddExternalFunctionForPlugin("Character::set_x",                     (void*)Character_SetX);
    ccAddExternalFunctionForPlugin("Character::get_Y",                     (void*)Character_GetY);
    ccAddExternalFunctionForPlugin("Character::set_Y",                     (void*)Character_SetY);
    ccAddExternalFunctionForPlugin("Character::get_y",                     (void*)Character_GetY);
    ccAddExternalFunctionForPlugin("Character::set_y",                     (void*)Character_SetY);
    ccAddExternalFunctionForPlugin("Character::get_Z",                     (void*)Character_GetZ);
    ccAddExternalFunctionForPlugin("Character::set_Z",                     (void*)Character_SetZ);
    ccAddExternalFunctionForPlugin("Character::get_z",                     (void*)Character_GetZ);
    ccAddExternalFunctionForPlugin("Character::set_z",                     (void*)Character_SetZ);
}
