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
#include "xeen/subtitles.h"
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
	Subtitles _subtitles;
	SpriteResource _mirror, _mirrBack;
	int _mergeX;
private:
	void loadScreen(const Common::String &name);

	/**
	 * Shows the Clouds of Xeen title screen
	 */
	bool showCloudsTitle();

	/**
	 * Inner implementation of the the Clouds of Xeen intro sequence
	 */
	bool showCloudsIntroInner();

	/**
	 * Shows part 1 of the Clouds of Xeen ending, with the castle being destroyed
	 */
	bool showCloudsEnding1();

	/**
	 * Shows part 2 of the Clouds of Xeen ending, King Roland's throneroom
	 */
	bool showCloudsEnding2();

	/**
	 * Shows part 3 of the Clouds of Xeen ending, which shows a display
	 * of the game's monsters
	 */
	bool showCloudsEnding3();

	/**
	 * Shows part 4 of the Clouds of Xeen ending, the final score
	 */
	bool showCloudsEnding4(uint finalScore);

	/**
	 * Shows part 5 of the Clouds of Xeen ending, final king display
	 */
	bool showCloudsEnding5();
public:
	CloudsCutscenes(XeenEngine *vm) : Cutscenes(vm), _mergeX(0) {}

	/**
	 * Shows the Clouds of Xeen intro sequence
	 */
	bool showCloudsIntro();

	/**
	 * Shows the Clouds of Xeen ending sequence
	 */
	void showCloudsEnding(uint finalScore);
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_CLOUDS_CUTSCENES_H */
