/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __AGS_EE_AC__RICHGAMEMEDIA_H
#define __AGS_EE_AC__RICHGAMEMEDIA_H

// Windows Vista Rich Save Games, modified to be platform-agnostic

#define RM_MAXLENGTH    1024
#define RM_MAGICNUMBER  "RGMH"

// Forward declaration
namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

#pragma pack(push)
#pragma pack(1)
typedef struct _RICH_GAME_MEDIA_HEADER
{
    int       dwMagicNumber;
    int       dwHeaderVersion;
    int       dwHeaderSize;
    int       dwThumbnailOffsetLowerDword;
    int       dwThumbnailOffsetHigherDword;
    int       dwThumbnailSize;
    unsigned char guidGameId[16];
    unsigned short szGameName[RM_MAXLENGTH];
    unsigned short szSaveName[RM_MAXLENGTH];
    unsigned short szLevelName[RM_MAXLENGTH];
    unsigned short szComments[RM_MAXLENGTH];

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out);
} RICH_GAME_MEDIA_HEADER;
#pragma pack(pop)

#endif // __AGS_EE_AC__RICHGAMEMEDIA_H
