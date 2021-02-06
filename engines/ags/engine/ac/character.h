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
//
//
//=============================================================================
#ifndef __AGS_EE_AC__CHARACTER_H
#define __AGS_EE_AC__CHARACTER_H

#include "ac/characterinfo.h"
#include "ac/characterextras.h"
#include "ac/dynobj/scriptobject.h"
#include "ac/dynobj/scriptinvitem.h"
#include "ac/dynobj/scriptoverlay.h"
#include "game/viewport.h"
#include "util/geometry.h"

// **** CHARACTER: FUNCTIONS ****

void    Character_AddInventory(CharacterInfo *chaa, ScriptInvItem *invi, int addIndex);
void    Character_AddWaypoint(CharacterInfo *chaa, int x, int y);
void    Character_Animate(CharacterInfo *chaa, int loop, int delay, int repeat, int blocking, int direction);
void    Character_ChangeRoomAutoPosition(CharacterInfo *chaa, int room, int newPos);
void    Character_ChangeRoom(CharacterInfo *chaa, int room, int x, int y);
void    Character_ChangeRoomSetLoop(CharacterInfo *chaa, int room, int x, int y, int direction);
void    Character_ChangeView(CharacterInfo *chap, int vii);
void    Character_FaceDirection(CharacterInfo *char1, int direction, int blockingStyle);
void    Character_FaceCharacter(CharacterInfo *char1, CharacterInfo *char2, int blockingStyle);
void    Character_FaceLocation(CharacterInfo *char1, int xx, int yy, int blockingStyle);
void    Character_FaceObject(CharacterInfo *char1, ScriptObject *obj, int blockingStyle);
void    Character_FollowCharacter(CharacterInfo *chaa, CharacterInfo *tofollow, int distaway, int eagerness);
int     Character_IsCollidingWithChar(CharacterInfo *char1, CharacterInfo *char2);
int     Character_IsCollidingWithObject(CharacterInfo *chin, ScriptObject *objid);
bool    Character_IsInteractionAvailable(CharacterInfo *cchar, int mood);
void    Character_LockView(CharacterInfo *chap, int vii);
void    Character_LockViewEx(CharacterInfo *chap, int vii, int stopMoving);
void    Character_LockViewAligned(CharacterInfo *chap, int vii, int loop, int align);
void    Character_LockViewAlignedEx(CharacterInfo *chap, int vii, int loop, int align, int stopMoving);
void    Character_LockViewFrame(CharacterInfo *chaa, int view, int loop, int frame);
void    Character_LockViewFrameEx(CharacterInfo *chaa, int view, int loop, int frame, int stopMoving);
void    Character_LockViewOffset(CharacterInfo *chap, int vii, int xoffs, int yoffs);
void    Character_LockViewOffsetEx(CharacterInfo *chap, int vii, int xoffs, int yoffs, int stopMoving);
void    Character_LoseInventory(CharacterInfo *chap, ScriptInvItem *invi);
void    Character_PlaceOnWalkableArea(CharacterInfo *chap);
void    Character_RemoveTint(CharacterInfo *chaa);
int     Character_GetHasExplicitTint(CharacterInfo *chaa);
void    Character_Say(CharacterInfo *chaa, const char *text);
void    Character_SayAt(CharacterInfo *chaa, int x, int y, int width, const char *texx);
ScriptOverlay* Character_SayBackground(CharacterInfo *chaa, const char *texx);
void    Character_SetAsPlayer(CharacterInfo *chaa);
void    Character_SetIdleView(CharacterInfo *chaa, int iview, int itime);
void    Character_SetOption(CharacterInfo *chaa, int flag, int yesorno);
void    Character_SetSpeed(CharacterInfo *chaa, int xspeed, int yspeed);
void    Character_StopMoving(CharacterInfo *charp);
void    Character_Tint(CharacterInfo *chaa, int red, int green, int blue, int opacity, int luminance);
void    Character_Think(CharacterInfo *chaa, const char *text);
void    Character_UnlockView(CharacterInfo *chaa);
void    Character_UnlockViewEx(CharacterInfo *chaa, int stopMoving);
void    Character_Walk(CharacterInfo *chaa, int x, int y, int blocking, int direct);
void    Character_Move(CharacterInfo *chaa, int x, int y, int blocking, int direct);
void    Character_WalkStraight(CharacterInfo *chaa, int xx, int yy, int blocking);

void    Character_RunInteraction(CharacterInfo *chaa, int mood);

// **** CHARACTER: PROPERTIES ****

int Character_GetProperty(CharacterInfo *chaa, const char *property);
void Character_GetPropertyText(CharacterInfo *chaa, const char *property, char *bufer);
const char* Character_GetTextProperty(CharacterInfo *chaa, const char *property);

ScriptInvItem* Character_GetActiveInventory(CharacterInfo *chaa);
void    Character_SetActiveInventory(CharacterInfo *chaa, ScriptInvItem* iit);
int     Character_GetAnimating(CharacterInfo *chaa);
int     Character_GetAnimationSpeed(CharacterInfo *chaa);
void    Character_SetAnimationSpeed(CharacterInfo *chaa, int newval);
int     Character_GetBaseline(CharacterInfo *chaa);
void    Character_SetBaseline(CharacterInfo *chaa, int basel);
int     Character_GetBlinkInterval(CharacterInfo *chaa);
void    Character_SetBlinkInterval(CharacterInfo *chaa, int interval);
int     Character_GetBlinkView(CharacterInfo *chaa);
void    Character_SetBlinkView(CharacterInfo *chaa, int vii);
int     Character_GetBlinkWhileThinking(CharacterInfo *chaa);
void    Character_SetBlinkWhileThinking(CharacterInfo *chaa, int yesOrNo);
int     Character_GetBlockingHeight(CharacterInfo *chaa);
void    Character_SetBlockingHeight(CharacterInfo *chaa, int hit);
int     Character_GetBlockingWidth(CharacterInfo *chaa);
void    Character_SetBlockingWidth(CharacterInfo *chaa, int wid);
int     Character_GetDiagonalWalking(CharacterInfo *chaa);
void    Character_SetDiagonalWalking(CharacterInfo *chaa, int yesorno);
int     Character_GetClickable(CharacterInfo *chaa);
void    Character_SetClickable(CharacterInfo *chaa, int clik);
int     Character_GetID(CharacterInfo *chaa);
int     Character_GetFrame(CharacterInfo *chaa);
void    Character_SetFrame(CharacterInfo *chaa, int newval);
int     Character_GetIdleView(CharacterInfo *chaa);
int     Character_GetIInventoryQuantity(CharacterInfo *chaa, int index);
int     Character_HasInventory(CharacterInfo *chaa, ScriptInvItem *invi);
void    Character_SetIInventoryQuantity(CharacterInfo *chaa, int index, int quant);
int     Character_GetIgnoreLighting(CharacterInfo *chaa);
void    Character_SetIgnoreLighting(CharacterInfo *chaa, int yesorno);
int     Character_GetIgnoreScaling(CharacterInfo *chaa);
void    Character_SetIgnoreScaling(CharacterInfo *chaa, int yesorno);
void    Character_SetManualScaling(CharacterInfo *chaa, int yesorno);
int     Character_GetIgnoreWalkbehinds(CharacterInfo *chaa);
void    Character_SetIgnoreWalkbehinds(CharacterInfo *chaa, int yesorno);
int     Character_GetMovementLinkedToAnimation(CharacterInfo *chaa);
void    Character_SetMovementLinkedToAnimation(CharacterInfo *chaa, int yesorno);
int     Character_GetLoop(CharacterInfo *chaa);
void    Character_SetLoop(CharacterInfo *chaa, int newval);
int     Character_GetMoving(CharacterInfo *chaa);
const char* Character_GetName(CharacterInfo *chaa);
void    Character_SetName(CharacterInfo *chaa, const char *newName);
int     Character_GetNormalView(CharacterInfo *chaa);
int     Character_GetPreviousRoom(CharacterInfo *chaa);
int     Character_GetRoom(CharacterInfo *chaa);
int     Character_GetScaleMoveSpeed(CharacterInfo *chaa);
void    Character_SetScaleMoveSpeed(CharacterInfo *chaa, int yesorno);
int     Character_GetScaleVolume(CharacterInfo *chaa);
void    Character_SetScaleVolume(CharacterInfo *chaa, int yesorno);
int     Character_GetScaling(CharacterInfo *chaa);
void    Character_SetScaling(CharacterInfo *chaa, int zoomlevel);
int     Character_GetSolid(CharacterInfo *chaa);
void    Character_SetSolid(CharacterInfo *chaa, int yesorno);
int     Character_GetSpeaking(CharacterInfo *chaa);
int     Character_GetSpeechColor(CharacterInfo *chaa);
void    Character_SetSpeechColor(CharacterInfo *chaa, int ncol);
void    Character_SetSpeechAnimationDelay(CharacterInfo *chaa, int newDelay);
int     Character_GetSpeechView(CharacterInfo *chaa);
void    Character_SetSpeechView(CharacterInfo *chaa, int vii);
int     Character_GetThinkView(CharacterInfo *chaa);
void    Character_SetThinkView(CharacterInfo *chaa, int vii);
int     Character_GetTransparency(CharacterInfo *chaa);
void    Character_SetTransparency(CharacterInfo *chaa, int trans);
int     Character_GetTurnBeforeWalking(CharacterInfo *chaa);
void    Character_SetTurnBeforeWalking(CharacterInfo *chaa, int yesorno);
int     Character_GetView(CharacterInfo *chaa);
int     Character_GetWalkSpeedX(CharacterInfo *chaa);
int     Character_GetWalkSpeedY(CharacterInfo *chaa);
int     Character_GetX(CharacterInfo *chaa);
void    Character_SetX(CharacterInfo *chaa, int newval);
int     Character_GetY(CharacterInfo *chaa);
void    Character_SetY(CharacterInfo *chaa, int newval);
int     Character_GetZ(CharacterInfo *chaa);
void    Character_SetZ(CharacterInfo *chaa, int newval);
int     Character_GetSpeakingFrame(CharacterInfo *chaa);

//=============================================================================

struct MoveList;
namespace AGS { namespace Common { class Bitmap; } }
using namespace AGS; // FIXME later

void animate_character(CharacterInfo *chap, int loopn,int sppd,int rept, int noidleoverride = 0, int direction = 0, int sframe = 0);
void walk_character(int chac,int tox,int toy,int ignwal, bool autoWalkAnims);
int  find_looporder_index (int curloop);
// returns 0 to use diagonal, 1 to not
int  useDiagonal (CharacterInfo *char1);
// returns 1 normally, or 0 if they only have horizontal animations
int  hasUpDownLoops(CharacterInfo *char1);
void start_character_turning (CharacterInfo *chinf, int useloop, int no_diagonal);
void fix_player_sprite(MoveList*cmls,CharacterInfo*chinf);
// Check whether two characters have walked into each other
int  has_hit_another_character(int sourceChar);
int  doNextCharMoveStep (CharacterInfo *chi, int &char_index, CharacterExtras *chex);
int  find_nearest_walkable_area_within(int *xx, int *yy, int range, int step);
void find_nearest_walkable_area (int *xx, int *yy);
void walk_character(int chac,int tox,int toy,int ignwal, bool autoWalkAnims);
void FindReasonableLoopForCharacter(CharacterInfo *chap);
void walk_or_move_character(CharacterInfo *chaa, int x, int y, int blocking, int direct, bool isWalk);
int  is_valid_character(int newchar);
int  wantMoveNow (CharacterInfo *chi, CharacterExtras *chex);
void setup_player_character(int charid);
void CheckViewFrameForCharacter(CharacterInfo *chi);
Common::Bitmap *GetCharacterImage(int charid, int *isFlipped);
CharacterInfo *GetCharacterAtScreen(int xx, int yy);
// Get character ID at the given room coordinates
int is_pos_on_character(int xx,int yy);
void get_char_blocking_rect(int charid, int *x1, int *y1, int *width, int *y2);
// Check whether the source char has walked onto character ww
int is_char_on_another (int sourceChar, int ww, int*fromxptr, int*cwidptr);
int my_getpixel(Common::Bitmap *blk, int x, int y);
// X and Y co-ordinates must be in 320x200 format
int check_click_on_character(int xx,int yy,int mood);
int is_pos_on_character(int xx,int yy);
void _DisplaySpeechCore(int chid, const char *displbuf);
void _DisplayThoughtCore(int chid, const char *displbuf);
void _displayspeech(const char*texx, int aschar, int xx, int yy, int widd, int isThought);
int get_character_currently_talking();
void DisplaySpeech(const char*texx, int aschar);
int update_lip_sync(int talkview, int talkloop, int *talkframeptr);

// Calculates character's bounding box in room coordinates (takes only in-room transform into account)
// use_frame_0 optionally tells to use frame 0 of current loop instead of current frame.
Rect GetCharacterRoomBBox(int charid, bool use_frame_0 = false);
// Find a closest viewport given character is to. Checks viewports in their order in game's array,
// and returns either first viewport character's bounding box intersects with (or rather with its camera),
// or the one that is least far away from its camera; calculated as a perpendicular distance between two AABBs.
PViewport FindNearestViewport(int charid);

extern CharacterInfo*playerchar;
extern CharacterExtras *charextra;
extern MoveList *mls;
extern int32_t _sc_PlayerCharPtr;

// order of loops to turn character in circle from down to down
extern int turnlooporder[8];

#endif // __AGS_EE_AC__CHARACTER_H
