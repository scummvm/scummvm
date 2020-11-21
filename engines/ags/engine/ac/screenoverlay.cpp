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

#include "screenoverlay.h"
#include "util/stream.h"

using AGS::Common::Stream;

void ScreenOverlay::ReadFromFile(Stream *in, int32_t cmp_ver)
{
    // Skipping bmp and pic pointer values
    // TODO: find out if it's safe to just drop these pointers!! replace with unique_ptr?
    bmp = nullptr;
    pic = nullptr;
    in->ReadInt32(); // bmp
    hasSerializedBitmap = in->ReadInt32() != 0;
    type = in->ReadInt32();
    x = in->ReadInt32();
    y = in->ReadInt32();
    timeout = in->ReadInt32();
    bgSpeechForChar = in->ReadInt32();
    associatedOverlayHandle = in->ReadInt32();
    hasAlphaChannel = in->ReadBool();
    positionRelativeToScreen = in->ReadBool();
    if (cmp_ver >= 1)
    {
        _offsetX = in->ReadInt32();
        _offsetY = in->ReadInt32();
    }
}

void ScreenOverlay::WriteToFile(Stream *out) const
{
    // Writing bitmap "pointers" to correspond to full structure writing
    out->WriteInt32(0); // bmp
    out->WriteInt32(pic ? 1 : 0); // pic
    out->WriteInt32(type);
    out->WriteInt32(x);
    out->WriteInt32(y);
    out->WriteInt32(timeout);
    out->WriteInt32(bgSpeechForChar);
    out->WriteInt32(associatedOverlayHandle);
    out->WriteBool(hasAlphaChannel);
    out->WriteBool(positionRelativeToScreen);
    // since cmp_ver = 1
    out->WriteInt32(_offsetX);
    out->WriteInt32(_offsetY);
}
