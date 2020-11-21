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
#ifndef __AGS_EE_AC__GLOBALHOTSPOT_H
#define __AGS_EE_AC__GLOBALHOTSPOT_H

void DisableHotspot(int hsnum);
void EnableHotspot(int hsnum);
int  GetHotspotPointX (int hotspot);
int  GetHotspotPointY (int hotspot);
// Gets hotspot ID at the given screen coordinates;
// if hotspot is disabled or non-existing, returns 0 (no area)
int  GetHotspotIDAtScreen(int xxx,int yyy);
void GetHotspotName(int hotspot, char *buffer);
void RunHotspotInteraction (int hotspothere, int mood);

int  GetHotspotProperty (int hss, const char *property);
void GetHotspotPropertyText (int item, const char *property, char *bufer);


#endif // __AGS_EE_AC__GLOBALHOTSPOT_H
