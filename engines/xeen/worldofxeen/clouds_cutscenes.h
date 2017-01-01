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

#ifndef XEEN_WORLDOFXEEN_CLOUDS_CUTSCENES_H
#define XEEN_WORLDOFXEEN_CLOUDS_CUTSCENES_H

#include "xeen/cutscenes.h"
#include "xeen/xeen.h"

namespace Xeen {
namespace WorldOfXeen {

class CloudsCutscenes : public Cutscenes {
private:
	static const char *const _INTRO_VOCS[14];
	static const int _INTRO_FRAMES_LOOKUP[14];
	static const int _INTRO_FRAMES_VALS[8][32];
	static const uint _INTRO_FRAMES_WAIT[8][32];
	static const byte _DECODE_TABLE1[256];
	static const byte _DECODE_TABLE2[256];
private:
	void loadScreen(const Common::String &name);
public:
	CloudsCutscenes(XeenEngine *vm) : Cutscenes(vm) {}

	/**
	 * Shows the Clouds of Xeen title screen
	 */
	bool showCloudsTitle();

	/**
	 * Shows the Clouds of Xeen intro sequence
	 */
	bool showCloudsIntro();

	/**
	 * Shows the Clouds of Xeen ending sequence
	 */
	bool showCloudsEnding();
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_CLOUDS_CUTSCENES_H */
