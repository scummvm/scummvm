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
#ifndef __AGS_EE_AC__REGION_H
#define __AGS_EE_AC__REGION_H

#include "ac/dynobj/scriptregion.h"

ScriptRegion *GetRegionAtRoom(int xx, int yy);
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
void	Region_RunInteraction(ScriptRegion *ssr, int mood);

void    generate_light_table();

#endif // __AGS_EE_AC__REGION_H
