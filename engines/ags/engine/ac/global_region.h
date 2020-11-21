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
#ifndef __AGS_EE_AC__GLOBALREGION_H
#define __AGS_EE_AC__GLOBALREGION_H

// Gets region ID at the given room coordinates;
// if region is disabled or non-existing, returns 0 (no area)
int  GetRegionIDAtRoom(int xxx, int yyy);
void SetAreaLightLevel(int area, int brightness);
void SetRegionTint (int area, int red, int green, int blue, int amount, int luminance = 100);
void DisableRegion(int hsnum);
void EnableRegion(int hsnum);
void DisableGroundLevelAreas(int alsoEffects);
void EnableGroundLevelAreas();
void RunRegionInteraction (int regnum, int mood);

#endif // __AGS_EE_AC__GLOBALREGION_H
