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
#ifndef __AGS_EE_AC__HOTSPOT_H
#define __AGS_EE_AC__HOTSPOT_H

#include "ac/dynobj/scripthotspot.h"

void    Hotspot_SetEnabled(ScriptHotspot *hss, int newval);
int     Hotspot_GetEnabled(ScriptHotspot *hss);
int     Hotspot_GetID(ScriptHotspot *hss);
ScriptHotspot *GetHotspotAtScreen(int xx, int yy);
int     Hotspot_GetWalkToX(ScriptHotspot *hss);;
int     Hotspot_GetWalkToY(ScriptHotspot *hss);
void    Hotspot_GetName(ScriptHotspot *hss, char *buffer);
const char* Hotspot_GetName_New(ScriptHotspot *hss);
bool    Hotspot_IsInteractionAvailable(ScriptHotspot *hhot, int mood);
void    Hotspot_RunInteraction (ScriptHotspot *hss, int mood);

int     Hotspot_GetProperty (ScriptHotspot *hss, const char *property);
void    Hotspot_GetPropertyText (ScriptHotspot *hss, const char *property, char *bufer);
const char* Hotspot_GetTextProperty(ScriptHotspot *hss, const char *property);

// Gets hotspot ID at the given room coordinates;
// if hotspot is disabled or non-existing, returns 0 (no area)
int     get_hotspot_at(int xpp,int ypp);

#endif // __AGS_EE_AC__HOTSPOT_H
