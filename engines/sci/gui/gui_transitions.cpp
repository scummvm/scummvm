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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_transitions.h"

namespace Sci {

SciGuiTransitions::SciGuiTransitions(SciGui *gui, SciGuiScreen *screen, SciGuiPalette *palette, bool isVGA)
	: _gui(gui), _screen(screen), _palette(palette), _isVGA(isVGA) {
	init();
}

SciGuiTransitions::~SciGuiTransitions() {
}

void SciGuiTransitions::init() {
}

void SciGuiTransitions::setup(int16 number) {
	_number = number;
}

void SciGuiTransitions::doit(Common::Rect picRect) {
	_picRect = picRect;

	if (_isVGA) {
		// === VGA transitions
		switch (_number) {
		case SCI_TRANSITIONS_VGA_FADEPALETTE:
			fadeOut(); setNewScreen(); fadeIn();
			break;

		default:
			warning("SciGuiTransitions: VGA-%d not implemented", _number);
			setNewPalette(); setNewScreen();
		}
	} else {
		// === EGA transitions
		switch (_number) {
		case SCI_TRANSITIONS_EGA_FADEPALETTE:
			fadeOut(); setNewScreen(); fadeIn();
			break;

		default:
			warning("SciGuiTransitions: VGA-%d not implemented", _number);
			setNewPalette(); setNewScreen();
		}
	}
	_screen->_picNotValid = 0;
}

void SciGuiTransitions::setNewPalette() {
	_palette->setOnScreen();
}

void SciGuiTransitions::setNewScreen() {
	_screen->copyRectToScreen(_picRect);
	g_system->updateScreen();
}

void SciGuiTransitions::fadeOut() {
	byte oldPalette[4 * 256], workPalette[4 * 256];
	int16 stepNr, colorNr;

	g_system->grabPalette(oldPalette, 0, 256);

	for (stepNr = 100; stepNr >= 0; stepNr -= 5) {
		for (colorNr = 0; colorNr < 256; colorNr++){
			workPalette[colorNr * 4 + 0] = oldPalette[colorNr * 4] * stepNr / 100;
			workPalette[colorNr * 4 + 1] = oldPalette[colorNr * 4 + 1] * stepNr / 100;
			workPalette[colorNr * 4 + 2] = oldPalette[colorNr * 4 + 2] * stepNr / 100;
		}
		g_system->setPalette(workPalette, 0, 256);
		_gui->wait(1);
	}
}

void SciGuiTransitions::fadeIn() {
	byte workPalette[4 * 256];
	GuiPalette *newPalette = &_palette->_sysPalette;
	int16 stepNr, colorNr;

	for (stepNr = 0; stepNr <= 100; stepNr += 5) {
		for (colorNr = 0; colorNr < 256; colorNr++){
			workPalette[colorNr * 4 + 0] = newPalette->colors[colorNr].r * stepNr / 100;
			workPalette[colorNr * 4 + 1] = newPalette->colors[colorNr].g * stepNr / 100;
			workPalette[colorNr * 4 + 2] = newPalette->colors[colorNr].b * stepNr / 100;
			workPalette[colorNr * 4 + 3] = 100;
		}
		g_system->setPalette(workPalette, 0, 256);
		_gui->wait(1);
	}
}

} // End of namespace Sci
