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

#ifndef XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H
#define XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H

#include "xeen/cutscenes.h"

namespace Xeen {

class XeenEngine;

namespace WorldOfXeen {

class DarkSideCutscenes : public Cutscenes {
protected:
	/**
	 * Shows the Pharaoh ending screen where score text is shown
	 */
	void showPharaohEndText(const char *msg1, const char *msg2 = nullptr, const char *msg3 = nullptr);
public:
	DarkSideCutscenes(XeenEngine *vm) : Cutscenes(vm) {}

	/**
	 * Shows the Dark Side of Xeen title screen
	 */
	bool showDarkSideTitle();

	/**
	 * Shows the Dark Side of Xeen intro sequence
	 */
	bool showDarkSideIntro();

	/**
	 * Shows the Dark Side of Xeen ending sequence
	 */
	bool showDarkSideEnding();

	/**
	 * Show the ending score
	 */
	void showDarkSideScore();
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H */
