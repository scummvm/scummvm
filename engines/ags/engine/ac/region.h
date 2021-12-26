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

#ifndef AGS_ENGINE_AC_REGION_H
#define AGS_ENGINE_AC_REGION_H

#include "ags/engine/ac/dynobj/script_region.h"

namespace AGS3 {

ScriptRegion *GetRegionAtRoom(int xx, int yy);
ScriptRegion *GetRegionAtScreen(int x, int y);
void    Region_SetLightLevel(ScriptRegion *ssr, int brightness);
int     Region_GetLightLevel(ScriptRegion *ssr);
int     Region_GetTintEnabled(ScriptRegion *srr);
int     Region_GetTintRed(ScriptRegion *srr);
int     Region_GetTintGreen(ScriptRegion *srr);
int     Region_GetTintBlue(ScriptRegion *srr);
int     Region_GetTintSaturation(ScriptRegion *srr);
int     Region_GetTintLuminance(ScriptRegion *srr);
void    Region_Tint(ScriptRegion *srr, int red, int green, int blue, int amount, int luminance);
void    Region_SetEnabled(ScriptRegion *ssr, int enable);
int     Region_GetEnabled(ScriptRegion *ssr);
int     Region_GetID(ScriptRegion *ssr);
void    Region_RunInteraction(ScriptRegion *ssr, int mood);

void    generate_light_table();
void    Region_TintNoLum(ScriptRegion *srr, int red, int green, int blue, int amount);

} // namespace AGS3

#endif
