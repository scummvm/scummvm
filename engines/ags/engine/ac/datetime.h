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
#ifndef __AGS_EE_AC__DATETIME_H
#define __AGS_EE_AC__DATETIME_H

#include "ac/dynobj/scriptdatetime.h"

ScriptDateTime* DateTime_Now_Core();
ScriptDateTime* DateTime_Now();
int             DateTime_GetYear(ScriptDateTime *sdt);
int             DateTime_GetMonth(ScriptDateTime *sdt);
int             DateTime_GetDayOfMonth(ScriptDateTime *sdt);
int             DateTime_GetHour(ScriptDateTime *sdt);
int             DateTime_GetMinute(ScriptDateTime *sdt);
int             DateTime_GetSecond(ScriptDateTime *sdt);
int             DateTime_GetRawTime(ScriptDateTime *sdt);

#endif // __AGS_EE_AC__DATETIME_H
