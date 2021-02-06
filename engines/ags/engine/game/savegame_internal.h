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

#ifndef __AGS_EE_GAME__SAVEGAMEINTERNAL_H
#define __AGS_EE_GAME__SAVEGAMEINTERNAL_H

#include <memory>
#include <vector>
#include "ac/common_defines.h"
#include "gfx/bitmap.h"
#include "media/audio/audiodefines.h"


namespace AGS
{
namespace Engine
{

using AGS::Common::Bitmap;

typedef std::shared_ptr<Bitmap> PBitmap;

// PreservedParams keeps old values of particular gameplay
// parameters that are saved before the save restoration
// and either applied or compared to new values after
// loading save data
struct PreservedParams
{
    // Whether speech and audio packages available
    int SpeechVOX;
    int MusicVOX;
    // Script global data sizes
    int GlScDataSize;
    std::vector<int> ScMdDataSize;

    PreservedParams();
};

enum GameViewCamFlags
{
    kSvgGameAutoRoomView = 0x01
};

enum CameraSaveFlags
{
    kSvgCamPosLocked = 0x01
};

enum ViewportSaveFlags
{
    kSvgViewportVisible = 0x01
};

// RestoredData keeps certain temporary data to help with
// the restoration process
struct RestoredData
{
    int                     FPS;
    // Unserialized bitmaps for dynamic surfaces
    std::vector<Bitmap*>    DynamicSurfaces;
    // Scripts global data
    struct ScriptData
    {
        std::shared_ptr<char> Data;
        size_t              Len;

        ScriptData();
    };
    ScriptData              GlobalScript;
    std::vector<ScriptData> ScriptModules;
    // Room data (has to be be preserved until room is loaded)
    PBitmap                 RoomBkgScene[MAX_ROOM_BGFRAMES];
    short                   RoomLightLevels[MAX_ROOM_REGIONS];
    int                     RoomTintLevels[MAX_ROOM_REGIONS];
    short                   RoomZoomLevels1[MAX_WALK_AREAS + 1];
    short                   RoomZoomLevels2[MAX_WALK_AREAS + 1];
    RoomVolumeMod           RoomVolume;
    // Mouse cursor parameters
    int                     CursorID;
    int                     CursorMode;
    // General audio
    struct ChannelInfo
    {
        int ClipID = 0;
        int Pos = 0;
        int Priority = 0;
        int Repeat = 0;
        int Vol = 0;
        int VolAsPercent = 0;
        int Pan = 0;
        int PanAsPercent = 0;
        int Speed = 0;
        // since version 1
        int XSource = -1;
        int YSource = -1;
        int MaxDist = 0;
    };
    ChannelInfo             AudioChans[MAX_SOUND_CHANNELS + 1];
    // Ambient sounds
    int                     DoAmbient[MAX_SOUND_CHANNELS];
    // Viewport and camera data, has to be preserved and applied only after
    // room gets loaded, because we must clamp these to room parameters.
    struct ViewportData
    {
        int ID = -1;
        int Flags = 0;
        int Left = 0;
        int Top = 0;
        int Width = 0;
        int Height = 0;
        int ZOrder = 0;
        int CamID = -1;
    };
    struct CameraData
    {
        int ID = -1;
        int Flags = 0;
        int Left = 0;
        int Top = 0;
        int Width = 0;
        int Height = 0;
    };
    std::vector<ViewportData> Viewports;
    std::vector<CameraData> Cameras;
    int32_t Camera0_Flags = 0; // flags for primary camera, when data is read in legacy order

    RestoredData();
};

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GAME__SAVEGAMEINTERNAL_H
