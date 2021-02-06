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

#include "ac/dynobj/scriptdrawingsurface.h"
#include "ac/spritecache.h"
#include "ac/runtime_defines.h"
#include "ac/common.h"
#include "ac/drawingsurface.h"
#include "ac/gamestate.h"
#include "ac/gamesetupstruct.h"
#include "game/roomstruct.h"
#include "gfx/bitmap.h"

using namespace AGS::Common;

extern RoomStruct thisroom;
extern SpriteCache spriteset;
extern Bitmap *dynamicallyCreatedSurfaces[MAX_DYNAMIC_SURFACES];
extern GameState play;
extern GameSetupStruct game;

Bitmap* ScriptDrawingSurface::GetBitmapSurface()
{
    // TODO: consider creating weak_ptr here, and store one in the DrawingSurface!
    if (roomBackgroundNumber >= 0)
        return thisroom.BgFrames[roomBackgroundNumber].Graphic.get();
    else if (dynamicSpriteNumber >= 0)
        return spriteset[dynamicSpriteNumber];
    else if (dynamicSurfaceNumber >= 0)
        return dynamicallyCreatedSurfaces[dynamicSurfaceNumber];
    else if (linkedBitmapOnly != nullptr)
        return linkedBitmapOnly;
    else if (roomMaskType > kRoomAreaNone)
        return thisroom.GetMask(roomMaskType);
    quit("!DrawingSurface: attempted to use surface after Release was called");
    return nullptr;
}

Bitmap *ScriptDrawingSurface::StartDrawing()
{
    //abufBackup = abuf;
    return this->GetBitmapSurface();
}

void ScriptDrawingSurface::FinishedDrawingReadOnly()
{
    //abuf = abufBackup;
}

void ScriptDrawingSurface::FinishedDrawing()
{
    FinishedDrawingReadOnly();
    modified = 1;
}

int ScriptDrawingSurface::Dispose(const char *address, bool force) {

    // dispose the drawing surface
    DrawingSurface_Release(this);
    delete this;
    return 1;
}

const char *ScriptDrawingSurface::GetType() {
    return "DrawingSurface";
}

int ScriptDrawingSurface::Serialize(const char *address, char *buffer, int bufsize) {
    StartSerialize(buffer);
    SerializeInt(roomBackgroundNumber & 0xFFFF | (roomMaskType << 16));
    SerializeInt(dynamicSpriteNumber);
    SerializeInt(dynamicSurfaceNumber);
    SerializeInt(currentColour);
    SerializeInt(currentColourScript);
    SerializeInt(highResCoordinates);
    SerializeInt(modified);
    SerializeInt(hasAlphaChannel);
    SerializeInt(isLinkedBitmapOnly ? 1 : 0);
    return EndSerialize();
}

void ScriptDrawingSurface::Unserialize(int index, const char *serializedData, int dataSize) {
    StartUnserialize(serializedData, dataSize);
    int room_ds = UnserializeInt();
    roomBackgroundNumber = static_cast<short>(room_ds & 0xFFFF);
    roomMaskType = (RoomAreaMask)(room_ds >> 16);
    dynamicSpriteNumber = UnserializeInt();
    dynamicSurfaceNumber = UnserializeInt();
    currentColour = UnserializeInt();
    currentColourScript = UnserializeInt();
    highResCoordinates = UnserializeInt();
    modified = UnserializeInt();
    hasAlphaChannel = UnserializeInt();
    isLinkedBitmapOnly = (UnserializeInt() != 0);
    ccRegisterUnserializedObject(index, this, this);
}

ScriptDrawingSurface::ScriptDrawingSurface() 
{
    roomBackgroundNumber = -1;
    roomMaskType = kRoomAreaNone;
    dynamicSpriteNumber = -1;
    dynamicSurfaceNumber = -1;
    isLinkedBitmapOnly = false;
    linkedBitmapOnly = nullptr;
    currentColour = play.raw_color;
    currentColourScript = 0;
    modified = 0;
    hasAlphaChannel = 0;
    highResCoordinates = 0;
    // NOTE: Normally in contemporary games coordinates ratio will always be 1:1.
    // But we still support legacy drawing, so have to set this up even for modern games,
    // otherwise we'd have to complicate conversion conditions further.
    if (game.IsLegacyHiRes() && game.IsDataInNativeCoordinates())
    {
        highResCoordinates = 1;
    }
}
