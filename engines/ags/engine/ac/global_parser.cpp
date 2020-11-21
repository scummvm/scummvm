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

#include <cstring> //strcpy()
#include "ac/global_parser.h"
#include "ac/common.h"
#include "ac/gamestate.h"
#include "ac/string.h"

extern GameState play;

int SaidUnknownWord (char*buffer) {
    VALIDATE_STRING(buffer);
    strcpy (buffer, play.bad_parsed_word);
    if (play.bad_parsed_word[0] == 0)
        return 0;
    return 1;
}
