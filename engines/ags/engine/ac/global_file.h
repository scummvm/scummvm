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
#ifndef __AGS_EE_AC__GLOBALFILE_H
#define __AGS_EE_AC__GLOBALFILE_H

#include "util/file.h"

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

int32_t FileOpen(const char*fnmm, Common::FileOpenMode open_mode, Common::FileWorkMode work_mode);
// NOTE: FileOpenCMode is a backwards-compatible replacement for old-style global script function FileOpen
int32_t FileOpenCMode(const char*fnmm, const char* cmode);
void  FileClose(int32_t handle);
void  FileWrite(int32_t handle, const char *towrite);
void  FileWriteRawLine(int32_t handle, const char*towrite);
void  FileRead(int32_t handle,char*toread);
int   FileIsEOF (int32_t handle);
int   FileIsError(int32_t handle);
void  FileWriteInt(int32_t handle,int into);
int   FileReadInt(int32_t handle);
char  FileReadRawChar(int32_t handle);
int   FileReadRawInt(int32_t handle);
void  FileWriteRawChar(int32_t handle, int chartoWrite);

#endif // __AGS_EE_AC__GLOBALFILE_H
