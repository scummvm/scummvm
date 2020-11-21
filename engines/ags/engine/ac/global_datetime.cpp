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

#include <time.h>
#include "ac/global_datetime.h"
#include "ac/datetime.h"
#include "ac/common.h"

int sc_GetTime(int whatti) {
    ScriptDateTime *sdt = DateTime_Now_Core();
    int returnVal = 0;

    if (whatti == 1) returnVal = sdt->hour;
    else if (whatti == 2) returnVal = sdt->minute;
    else if (whatti == 3) returnVal = sdt->second;
    else if (whatti == 4) returnVal = sdt->day;
    else if (whatti == 5) returnVal = sdt->month;
    else if (whatti == 6) returnVal = sdt->year;
    else quit("!GetTime: invalid parameter passed");

    delete sdt;

    return returnVal;
}

int GetRawTime () {
    // TODO: we might need to modify script API to support larger time type
    return static_cast<int>(time(nullptr));
}
