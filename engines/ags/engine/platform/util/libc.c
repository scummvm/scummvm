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
// Implementations for missing libc functions
//
//=============================================================================

#include "core/platform.h"

#if ! AGS_PLATFORM_OS_WINDOWS

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>

size_t mbstowcs(wchar_t *wcstr, const char *mbstr, size_t max)
{
  int count = 0;
  
  while ((count < max) && (*mbstr != 0))
  {
    *wcstr++ = *mbstr++;
    count++;
  }
  return count;

}

size_t wcstombs(char* mbstr, const wchar_t *wcstr, size_t max)
{
  int count = 0;

  while ((count < max) && (*wcstr != 0))
  {
    *mbstr++ = *wcstr++;
    count++;
  }
  return count;
}

#endif // ! AGS_PLATFORM_OS_WINDOWS
