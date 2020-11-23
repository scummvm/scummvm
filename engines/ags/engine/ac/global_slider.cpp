/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/shared/ac/global_slider.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/slider.h"
#include "ags/shared/gui/guimain.h"
#include "ags/shared/gui/guislider.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetupStruct game;

void SetSliderValue(int guin, int objn, int valn) {
	if ((guin < 0) | (guin >= game.numgui)) quit("!SetSliderValue: invalid GUI number");
	if (guis[guin].GetControlType(objn) != kGUISlider)
		quit("!SetSliderValue: specified control is not a slider");

	GUISlider *guisl = (GUISlider *)guis[guin].GetControl(objn);
	Slider_SetValue(guisl, valn);
}

int GetSliderValue(int guin, int objn) {
	if ((guin < 0) | (guin >= game.numgui)) quit("!GetSliderValue: invalid GUI number");
	if (guis[guin].GetControlType(objn) != kGUISlider)
		quit("!GetSliderValue: specified control is not a slider");

	GUISlider *guisl = (GUISlider *)guis[guin].GetControl(objn);
	return Slider_GetValue(guisl);
}

} // namespace AGS3
