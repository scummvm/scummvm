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

#ifndef __AGS_EE_PLATFORM_UTIL_PE_H
#define __AGS_EE_PLATFORM_UTIL_PE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char version[15];
  char description[100];
  char internal_name[100];
} version_info_t;

int getVersionInformation(char* filename, version_info_t* version_info);

#ifdef __cplusplus
}
#endif

#endif // __AGS_EE_PLATFORM_UTIL_PE_H
