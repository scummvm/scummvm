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
