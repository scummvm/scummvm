/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
//
// [IKM] 2012-06-25: This bugs me that type is called 'Object'; in modern
// world of programming 'object' is usually a base class; should not we
// rename this to RoomObject one day?
//=============================================================================

#ifndef AGS_ENGINE_AC_OBJECT_H
#define AGS_ENGINE_AC_OBJECT_H

#include "ags/shared/ac/common_defines.h"
#include "ags/engine/ac/dynobj/script_object.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

extern AGS_INLINE int is_valid_object(int obtest);
int     Object_IsCollidingWithObject(ScriptObject *objj, ScriptObject *obj2);
ScriptObject *GetObjectAtScreen(int xx, int yy);
ScriptObject *GetObjectAtRoom(int x, int y);
void    Object_Tint(ScriptObject *objj, int red, int green, int blue, int saturation, int luminance);
void    Object_RemoveTint(ScriptObject *objj);
void    Object_SetView(ScriptObject *objj, int view, int loop, int frame);
void    Object_SetTransparency(ScriptObject *objj, int trans);
int     Object_GetTransparency(ScriptObject *objj);
void    Object_SetBaseline(ScriptObject *objj, int basel);
int     Object_GetBaseline(ScriptObject *objj);
void    Object_Animate(ScriptObject *objj, int loop, int delay, int repeat, int blocking, int direction);
void    Object_StopAnimating(ScriptObject *objj);
void    Object_MergeIntoBackground(ScriptObject *objj);
void    Object_StopMoving(ScriptObject *objj);
void    Object_SetVisible(ScriptObject *objj, int onoroff);
int     Object_GetView(ScriptObject *objj);
int     Object_GetLoop(ScriptObject *objj);
int     Object_GetFrame(ScriptObject *objj);
int     Object_GetVisible(ScriptObject *objj);
void    Object_SetGraphic(ScriptObject *objj, int slott);
int     Object_GetGraphic(ScriptObject *objj);
int     Object_GetX(ScriptObject *objj);
int     Object_GetY(ScriptObject *objj);
int     Object_GetAnimating(ScriptObject *objj);
int     Object_GetMoving(ScriptObject *objj);
void    Object_SetPosition(ScriptObject *objj, int xx, int yy);
void    Object_SetX(ScriptObject *objj, int xx);
void    Object_SetY(ScriptObject *objj, int yy);
void    Object_GetName(ScriptObject *objj, char *buffer);
const char *Object_GetName_New(ScriptObject *objj);
bool    Object_IsInteractionAvailable(ScriptObject *oobj, int mood);
void    Object_Move(ScriptObject *objj, int x, int y, int speed, int blocking, int direct);
void    Object_SetClickable(ScriptObject *objj, int clik);
int     Object_GetClickable(ScriptObject *objj);
void    Object_SetIgnoreScaling(ScriptObject *objj, int newval);
int     Object_GetIgnoreScaling(ScriptObject *objj);
void    Object_SetSolid(ScriptObject *objj, int solid);
int     Object_GetSolid(ScriptObject *objj);
void    Object_SetBlockingWidth(ScriptObject *objj, int bwid);
int     Object_GetBlockingWidth(ScriptObject *objj);
void    Object_SetBlockingHeight(ScriptObject *objj, int bhit);
int     Object_GetBlockingHeight(ScriptObject *objj);
int     Object_GetID(ScriptObject *objj);
void    Object_SetIgnoreWalkbehinds(ScriptObject *chaa, int clik);
int     Object_GetIgnoreWalkbehinds(ScriptObject *chaa);
void    Object_RunInteraction(ScriptObject *objj, int mode);

int     Object_GetProperty(ScriptObject *objj, const char *property);
void    Object_GetPropertyText(ScriptObject *objj, const char *property, char *bufer);
const char *Object_GetTextProperty(ScriptObject *objj, const char *property);

void    move_object(int objj, int tox, int toy, int spee, int ignwal);
void    get_object_blocking_rect(int objid, int *x1, int *y1, int *width, int *y2);
int     isposinbox(int mmx, int mmy, int lf, int tp, int rt, int bt);
int     is_pos_in_sprite(int xx, int yy, int arx, int ary, Shared::Bitmap *sprit, int spww, int sphh, int flipped = 0);
// X and Y co-ordinates must be in native format
// X and Y are ROOM coordinates
int     check_click_on_object(int roomx, int roomy, int mood);
// TODO: pick out some kind of "animation" struct
// Tests if the standard animate parameters are in valid range, if not then clamps them and
// reports a script warning.
void ValidateViewAnimParams(const char *apiname, int &repeat, int &blocking, int &direction);
// General view animation algorithm: find next loop and frame, depending on anim settings;
// loop and frame values are passed by reference and will be updated;
// returns whether the animation should continue.
bool    CycleViewAnim(int view, uint16_t &loop, uint16_t &frame, bool forwards, int repeat);

} // namespace AGS3

#endif
