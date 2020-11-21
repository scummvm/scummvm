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
// Room version constants and information
//
//=============================================================================

#ifndef __AGS_CN_AC__ROOMVERSION_H
#define __AGS_CN_AC__ROOMVERSION_H

/* room file versions history
8:  final v1.14 release
9:  intermediate v2 alpha releases
10:  v2 alpha-7 release
11:  final v2.00 release
12:  v2.08, to add colour depth byte
13:  v2.14, add walkarea light levels
14:  v2.4, fixed so it saves walkable area 15
15:  v2.41, supports NewInteraction
16:  v2.5
17:  v2.5 - just version change to force room re-compile for new charctr struct
18:  v2.51 - vector scaling
19:  v2.53 - interaction variables
20:  v2.55 - shared palette backgrounds
21:  v2.55 - regions
22:  v2.61 - encrypt room messages
23:  v2.62 - object flags
24:  v2.7  - hotspot script names
25:  v2.72 - game id embedded
26:  v3.0 - new interaction format, and no script source
27:  v3.0 - store Y of bottom of object, not top
28:  v3.0.3 - remove hotspot name length limit
29:  v3.0.3 - high-res coords for object x/y, edges and hotspot walk-to point
30:  v3.4.0.4 - tint luminance for regions
31:  v3.4.1.5 - removed room object and hotspot name length limits
32:  v3.5.0 - 64-bit file offsets
33:  v3.5.0.8 - deprecated room resolution, added mask resolution
*/
enum RoomFileVersion
{
    kRoomVersion_Undefined = 0,
    kRoomVersion_pre114_3 = 3,  // exact version unknown
    kRoomVersion_pre114_4 = 4,  // exact version unknown
    kRoomVersion_pre114_5 = 5,  // exact version unknown
    kRoomVersion_pre114_6 = 6,  // exact version unknown
    kRoomVersion_114 = 8,
    kRoomVersion_200_alpha = 9,
    kRoomVersion_200_alpha7 = 10,
    kRoomVersion_200_final = 11,
    kRoomVersion_208 = 12,
    kRoomVersion_214 = 13,
    kRoomVersion_240 = 14,
    kRoomVersion_241 = 15,
    kRoomVersion_250a = 16,
    kRoomVersion_250b = 17,
    kRoomVersion_251 = 18,
    kRoomVersion_253 = 19,
    kRoomVersion_255a = 20,
    kRoomVersion_255b = 21,
    kRoomVersion_261 = 22,
    kRoomVersion_262 = 23,
    kRoomVersion_270 = 24,
    kRoomVersion_272 = 25,
    kRoomVersion_300a = 26,
    kRoomVersion_300b = 27,
    kRoomVersion_303a = 28,
    kRoomVersion_303b = 29,
    kRoomVersion_3404 = 30,
    kRoomVersion_3415 = 31,
    kRoomVersion_350 = 32,
    kRoomVersion_3508 = 33,
    kRoomVersion_Current = kRoomVersion_3508
};

#endif // __AGS_CN_AC__ROOMVERSION_H
