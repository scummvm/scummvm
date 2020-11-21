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
#ifndef __AGS_EE_AC__GLOBALWALKABLEAREA_H
#define __AGS_EE_AC__GLOBALWALKABLEAREA_H

int   GetScalingAt (int x, int y);
void  SetAreaScaling(int area, int min, int max);
void  RemoveWalkableArea(int areanum);
void  RestoreWalkableArea(int areanum);
// Gets walkable area at the given room coordinates;
// if area is disabled or non-existing, returns 0 (no area)
int   GetWalkableAreaAtRoom(int x, int y);
// Gets walkable area at the given screen coordinates
// if area is disabled or non-existing, returns 0 (no area)
int   GetWalkableAreaAtScreen(int x, int y);

#endif // __AGS_EE_AC__GLOBALWALKABLEAREA_H
