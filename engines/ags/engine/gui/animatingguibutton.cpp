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

#include "gui/animatingguibutton.h"
#include "util/stream.h"

using AGS::Common::Stream;

void AnimatingGUIButton::ReadFromFile(Stream *in)
{
    buttonid = in->ReadInt16();
    ongui = in->ReadInt16();
    onguibut = in->ReadInt16();
    view = in->ReadInt16();
    loop = in->ReadInt16();
    frame = in->ReadInt16();
    speed = in->ReadInt16();
    repeat = in->ReadInt16();
    wait = in->ReadInt16();
}

void AnimatingGUIButton::WriteToFile(Stream *out)
{
    out->WriteInt16(buttonid);
    out->WriteInt16(ongui);
    out->WriteInt16(onguibut);
    out->WriteInt16(view);
    out->WriteInt16(loop);
    out->WriteInt16(frame);
    out->WriteInt16(speed);
    out->WriteInt16(repeat);
    out->WriteInt16(wait);
}
