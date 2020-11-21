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

#ifndef __AC_FILEBASEDAGSDEBUGGER_H
#define __AC_FILEBASEDAGSDEBUGGER_H

#include "debug/agseditordebugger.h"

struct FileBasedAGSDebugger : IAGSEditorDebugger
{
public:

    bool Initialize() override;
    void Shutdown() override;
    bool SendMessageToEditor(const char *message) override;
    bool IsMessageAvailable() override;
    char* GetNextMessage() override;

};

extern const char* SENT_MESSAGE_FILE_NAME;

#endif // __AC_FILEBASEDAGSDEBUGGER_H
