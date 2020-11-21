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

#include <string.h>
#include "ac/common.h"
#include "ac/display.h"
#include "ac/gamestate.h"
#include "ac/global_translation.h"
#include "ac/string.h"
#include "ac/tree_map.h"
#include "platform/base/agsplatformdriver.h"
#include "plugin/agsplugin.h"
#include "plugin/plugin_engine.h"
#include "util/memory.h"
#include "core/types.h"

using namespace AGS::Common::Memory;

extern GameState play;
extern AGSPlatformDriver *platform;
extern TreeMap *transtree;
extern char transFileName[MAX_PATH];

const char *get_translation (const char *text) {
    if (text == nullptr)
        quit("!Null string supplied to CheckForTranslations");

    source_text_length = GetTextDisplayLength(text);

#if AGS_PLATFORM_64BIT
    // check if a plugin wants to translate it - if so, return that
    // TODO: plugin API is currently strictly 32-bit, so this may break on 64-bit systems
    char *plResult = Int32ToPtr<char>(pl_run_plugin_hooks(AGSE_TRANSLATETEXT, PtrToInt32(text)));
    if (plResult) {
        return plResult;
    }
#endif

    if (transtree != nullptr) {
        // translate the text using the translation file
        char * transl = transtree->findValue (text);
        if (transl != nullptr)
            return transl;
    }
    // return the original text
    return text;
}

int IsTranslationAvailable () {
    if (transtree != nullptr)
        return 1;
    return 0;
}

int GetTranslationName (char* buffer) {
    VALIDATE_STRING (buffer);
    const char *copyFrom = transFileName;

    while (strchr(copyFrom, '\\') != nullptr)
    {
        copyFrom = strchr(copyFrom, '\\') + 1;
    }
    while (strchr(copyFrom, '/') != nullptr)
    {
        copyFrom = strchr(copyFrom, '/') + 1;
    }

    strcpy (buffer, copyFrom);
    // remove the ".tra" from the end of the filename
    if (strstr (buffer, ".tra") != nullptr)
        strstr (buffer, ".tra")[0] = 0;

    return IsTranslationAvailable();
}
