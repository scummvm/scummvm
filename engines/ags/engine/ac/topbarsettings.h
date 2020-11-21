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
#ifndef __AGS_EE_AC__TOPBARSETTINGS_H
#define __AGS_EE_AC__TOPBARSETTINGS_H

struct TopBarSettings {
    int wantIt;
    int height;
    int font;
    char text[200];

    TopBarSettings() {
        wantIt = 0;
        height = 0;
        font = 0;
        text[0] = 0;
    }
};

#endif // __AGS_EE_AC__TOPBARSETTINGS_H
