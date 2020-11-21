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

#include "ac/global_slider.h"
#include "ac/common.h"
#include "ac/gamesetupstruct.h"
#include "ac/slider.h"
#include "gui/guimain.h"
#include "gui/guislider.h"

using namespace AGS::Common;

extern GameSetupStruct game;

void SetSliderValue(int guin,int objn, int valn) {
    if ((guin<0) | (guin>=game.numgui)) quit("!SetSliderValue: invalid GUI number");
    if (guis[guin].GetControlType(objn)!=kGUISlider)
        quit("!SetSliderValue: specified control is not a slider");

    GUISlider*guisl=(GUISlider*)guis[guin].GetControl(objn);
    Slider_SetValue(guisl, valn);
}

int GetSliderValue(int guin,int objn) {
    if ((guin<0) | (guin>=game.numgui)) quit("!GetSliderValue: invalid GUI number");
    if (guis[guin].GetControlType(objn)!=kGUISlider)
        quit("!GetSliderValue: specified control is not a slider");

    GUISlider*guisl=(GUISlider*)guis[guin].GetControl(objn);
    return Slider_GetValue(guisl);
}
