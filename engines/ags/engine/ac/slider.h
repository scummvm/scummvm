/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_ENGINE_AC_SLIDER_H
#define AGS_ENGINE_AC_SLIDER_H

#include "ags/shared/gui/gui_slider.h"

namespace AGS3 {

using AGS::Shared::GUISlider;

void    Slider_SetMax(GUISlider *guisl, int valn);
int     Slider_GetMax(GUISlider *guisl);
void    Slider_SetMin(GUISlider *guisl, int valn);
int     Slider_GetMin(GUISlider *guisl);
void    Slider_SetValue(GUISlider *guisl, int valn);
int     Slider_GetValue(GUISlider *guisl);
int     Slider_GetBackgroundGraphic(GUISlider *guisl);
void    Slider_SetBackgroundGraphic(GUISlider *guisl, int newImage);
int     Slider_GetHandleGraphic(GUISlider *guisl);
void    Slider_SetHandleGraphic(GUISlider *guisl, int newImage);
int     Slider_GetHandleOffset(GUISlider *guisl);
void    Slider_SetHandleOffset(GUISlider *guisl, int newOffset);

} // namespace AGS3

#endif
