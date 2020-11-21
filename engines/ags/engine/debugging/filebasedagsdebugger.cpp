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

#include <cstdio>
#include "debug/filebasedagsdebugger.h"
#include "ac/file.h"                    // filelength()
#include "util/stream.h"
#include "util/textstreamwriter.h"
#include "util/wgt2allg.h"              // exists()
#include "platform/base/agsplatformdriver.h"

using AGS::Common::Stream;
using AGS::Common::TextStreamWriter;

const char* SENT_MESSAGE_FILE_NAME = "dbgrecv.tmp";

bool FileBasedAGSDebugger::Initialize()
{
    if (exists(SENT_MESSAGE_FILE_NAME))
    {
        ::remove(SENT_MESSAGE_FILE_NAME);
    }
    return true;
}

void FileBasedAGSDebugger::Shutdown()
{
}

bool FileBasedAGSDebugger::SendMessageToEditor(const char *message) 
{
    while (exists(SENT_MESSAGE_FILE_NAME))
    {
        platform->YieldCPU();
    }

    Stream *out = Common::File::CreateFile(SENT_MESSAGE_FILE_NAME);
    // CHECKME: originally the file was opened as "wb" for some reason,
    // which means the message should be written as a binary array;
    // or shouldn't it?
    out->Write(message, strlen(message));
    delete out;
    return true;
}

bool FileBasedAGSDebugger::IsMessageAvailable()
{
    return (exists("dbgsend.tmp") != 0);
}

char* FileBasedAGSDebugger::GetNextMessage()
{
    Stream *in = Common::File::OpenFileRead("dbgsend.tmp");
    if (in == nullptr)
    {
        // check again, because the editor might have deleted the file in the meantime
        return nullptr;
    }
    int fileSize = in->GetLength();
    char *msg = (char*)malloc(fileSize + 1);
    in->Read(msg, fileSize);
    delete in;
    ::remove("dbgsend.tmp");
    msg[fileSize] = 0;
    return msg;
}
