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

#ifndef AGS_ENGINE_AC_HOTSPOT_H
#define AGS_ENGINE_AC_HOTSPOT_H

#include "ags/engine/ac/dynobj/script_hotspot.h"

namespace AGS3 {

void    Hotspot_SetEnabled(ScriptHotspot *hss, int newval);
int     Hotspot_GetEnabled(ScriptHotspot *hss);
int     Hotspot_GetID(ScriptHotspot *hss);
ScriptHotspot *GetHotspotAtRoom(int x, int y);
ScriptHotspot *GetHotspotAtScreen(int xx, int yy);
int     Hotspot_GetWalkToX(ScriptHotspot *hss);
int     Hotspot_GetWalkToY(ScriptHotspot *hss);
void    Hotspot_GetName(ScriptHotspot *hss, char *buffer);
const char *Hotspot_GetName_New(ScriptHotspot *hss);
bool    Hotspot_IsInteractionAvailable(ScriptHotspot *hhot, int mood);
void    Hotspot_RunInteraction(ScriptHotspot *hss, int mood);

int     Hotspot_GetProperty(ScriptHotspot *hss, const char *property);
void    Hotspot_GetPropertyText(ScriptHotspot *hss, const char *property, char *bufer);
const char *Hotspot_GetTextProperty(ScriptHotspot *hss, const char *property);

// Gets hotspot ID at the given room coordinates;
// if hotspot is disabled or non-existing, returns 0 (no area)
int     get_hotspot_at(int xpp, int ypp);

} // namespace AGS3

#endif
