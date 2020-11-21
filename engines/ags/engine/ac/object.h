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
// [IKM] 2012-06-25: This bugs me that type is called 'Object'; in modern
// world of programming 'object' is usually a base class; should not we
// rename this to RoomObject one day?
//=============================================================================
#ifndef __AGS_EE_AC__OBJECT_H
#define __AGS_EE_AC__OBJECT_H

#include "ac/common_defines.h"
#include "ac/dynobj/scriptobject.h"

namespace AGS { namespace Common { class Bitmap; } }
using namespace AGS; // FIXME later

extern AGS_INLINE int is_valid_object(int obtest);
int     Object_IsCollidingWithObject(ScriptObject *objj, ScriptObject *obj2);
ScriptObject *GetObjectAtScreen(int xx, int yy);
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
const char* Object_GetName_New(ScriptObject *objj);
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

int     Object_GetProperty (ScriptObject *objj, const char *property);
void    Object_GetPropertyText(ScriptObject *objj, const char *property, char *bufer);
const char* Object_GetTextProperty(ScriptObject *objj, const char *property);

void    move_object(int objj,int tox,int toy,int spee,int ignwal);
void    get_object_blocking_rect(int objid, int *x1, int *y1, int *width, int *y2);
int     isposinbox(int mmx,int mmy,int lf,int tp,int rt,int bt);
int     is_pos_in_sprite(int xx,int yy,int arx,int ary, Common::Bitmap *sprit, int spww,int sphh, int flipped = 0);
// X and Y co-ordinates must be in native format
// X and Y are ROOM coordinates
int     check_click_on_object(int roomx, int roomy, int mood);

#endif // __AGS_EE_AC__OBJECT_H

