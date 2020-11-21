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
#ifndef __AGS_EE_AC__SLIDER_H
#define __AGS_EE_AC__SLIDER_H

#include "gui/guislider.h"

using AGS::Common::GUISlider;

void	Slider_SetMax(GUISlider *guisl, int valn);
int		Slider_GetMax(GUISlider *guisl);
void	Slider_SetMin(GUISlider *guisl, int valn);
int		Slider_GetMin(GUISlider *guisl);
void	Slider_SetValue(GUISlider *guisl, int valn);
int		Slider_GetValue(GUISlider *guisl);
int		Slider_GetBackgroundGraphic(GUISlider *guisl);
void	Slider_SetBackgroundGraphic(GUISlider *guisl, int newImage);
int		Slider_GetHandleGraphic(GUISlider *guisl);
void	Slider_SetHandleGraphic(GUISlider *guisl, int newImage);
int		Slider_GetHandleOffset(GUISlider *guisl);
void	Slider_SetHandleOffset(GUISlider *guisl, int newOffset);

#endif // __AGS_EE_AC__SLIDER_H
