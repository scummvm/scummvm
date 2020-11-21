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

#include "ac/dialogtopic.h"
#include "util/stream.h"

using AGS::Common::Stream;

void DialogTopic::ReadFromFile(Stream *in)
{
    in->ReadArray(optionnames, 150*sizeof(char), MAXTOPICOPTIONS);
    in->ReadArrayOfInt32(optionflags, MAXTOPICOPTIONS);
    // optionscripts pointer is not used anywhere in the engine
    optionscripts = nullptr;
    in->ReadInt32(); // optionscripts 32-bit pointer
    in->ReadArrayOfInt16(entrypoints, MAXTOPICOPTIONS);
    startupentrypoint = in->ReadInt16();
    codesize = in->ReadInt16();
    numoptions = in->ReadInt32();
    topicFlags = in->ReadInt32();
}

void DialogTopic::ReadFromSavegame(Common::Stream *in)
{
    in->ReadArrayOfInt32(optionflags, MAXTOPICOPTIONS);
}

void DialogTopic::WriteToSavegame(Common::Stream *out) const
{
    out->WriteArrayOfInt32(optionflags, MAXTOPICOPTIONS);
}
