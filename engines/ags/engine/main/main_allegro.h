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
#ifndef __AGS_EE_MAIN__MAINALLEGRO_H
#define __AGS_EE_MAIN__MAINALLEGRO_H

// Gets allegro_error as a const string.
// Please, use this getter to acquire error text, do not use allegro_error
// global variable directly.
const char *get_allegro_error();
// Sets allegro_error global variable and returns a resulting string.
// The maximal allowed text length is defined by ALLEGRO_ERROR_SIZE macro
// (usually 256). If the formatted message is larger than that it will be
// truncated. Null terminator is always guaranteed.
const char *set_allegro_error(const char *format, ...);

#endif // __AGS_EE_MAIN__MAINALLEGRO_H
