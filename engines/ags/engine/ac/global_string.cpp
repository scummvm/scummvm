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
#include "ac/global_string.h"
#include "ac/global_translation.h"
#include "ac/runtime_defines.h"
#include "ac/string.h"
#include "util/string_compat.h"

extern int MAXSTRLEN;

int StrGetCharAt (const char *strin, int posn) {
    if ((posn < 0) || (posn >= (int)strlen(strin)))
        return 0;
    return strin[posn];
}

void StrSetCharAt (char *strin, int posn, int nchar) {
    if ((posn < 0) || (posn > (int)strlen(strin)) || (posn >= MAX_MAXSTRLEN))
        quit("!StrSetCharAt: tried to write past end of string");

    if (posn == (int)strlen(strin))
        strin[posn+1] = 0;
    strin[posn] = nchar;
}

void _sc_strcat(char*s1, const char*s2) {
    // make sure they don't try to append a char to the string
    VALIDATE_STRING (s2);
    check_strlen(s1);
    int mosttocopy=(MAXSTRLEN-strlen(s1))-1;
    //  int numbf=game.iface[4].numbuttons;
    my_strncpy(&s1[strlen(s1)], s2, mosttocopy);
}

void _sc_strlower (char *desbuf) {
    VALIDATE_STRING(desbuf);
    check_strlen (desbuf);
    ags_strlwr (desbuf);
}

void _sc_strupper (char *desbuf) {
    VALIDATE_STRING(desbuf);
    check_strlen (desbuf);
    ags_strupr (desbuf);
}

/*int _sc_strcmp (char *s1, char *s2) {
return strcmp (get_translation (s1), get_translation(s2));
}

int _sc_stricmp (char *s1, char *s2) {
return ags_stricmp (get_translation (s1), get_translation(s2));
}*/

void _sc_strcpy(char*destt, const char *text) {
    VALIDATE_STRING(destt);
    check_strlen(destt);
    my_strncpy(destt, text, MAXSTRLEN - 1);
}
