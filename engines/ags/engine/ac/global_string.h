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
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__GLOBALSTRING_H
#define __AGS_EE_AC__GLOBALSTRING_H

int StrGetCharAt (const char *strin, int posn);
void StrSetCharAt (char *strin, int posn, int nchar);
void _sc_strcat(char*s1, const char*s2);
void _sc_strlower (char *desbuf);
void _sc_strupper (char *desbuf);
void _sc_strcpy(char*destt, const char*text);

#endif // __AGS_EE_AC__GLOBALSTRING_H
