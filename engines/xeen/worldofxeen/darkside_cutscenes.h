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

namespace Xeen {

class XeenEngine;

class DarkSideCutscenes {
private:
	XeenEngine *_vm;
public:
	DarkSideCutscenes(XeenEngine *vm) : _vm(vm) {}

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
};

} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H */
