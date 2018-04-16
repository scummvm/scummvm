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

#ifndef XEEN_WORLDOFXEEN_WORLDOFXEEN_CUTSCENES_H
#define XEEN_WORLDOFXEEN_WORLDOFXEEN_CUTSCENES_H

#include "xeen/worldofxeen/clouds_cutscenes.h"
#include "xeen/worldofxeen/darkside_cutscenes.h"
#include "xeen/xeen.h"

namespace Xeen {
namespace WorldOfXeen {

enum GooberState {
	NON_GOOBER = 0, GOOBER = 1, SUPER_GOOBER = 2
};

class WorldOfXeenCutscenes : public CloudsCutscenes,
	public DarkSideCutscenes {
private:
	XeenEngine *_vm;
	Subtitles _subtitles;
	GooberState _goober;
	uint _finalScore;
private:
	/**
	 * Sets a subtitle during the world of xeen ending
	 */
	void setSubtitle(const Common::String &msg);

	/**
	 * Sets a subtitle during the world of xeen ending
	 */
	void setSubtitle2(const Common::String &msg);

	/**
	 * Part 1 of World of Xeen ending, up to the point the Cube and Scepter have been
	 * placed onto the Altar of Joining
	 */
	bool worldEnding1();

	/**
	 * Part 2 of World of Xeen ending, showing beams of light erupting from Altar,
	 * and four beams arcing across the top and bottom sides of Xeen
	 */
	bool worldEnding2();

	/**
	 * Part 3 of World of Xeen ending, with further beams covering Xeen, and it
	 * transforming the world into a sphere
	 */
	bool worldEnding3();
public:
	WorldOfXeenCutscenes(XeenEngine *vm) : _vm(vm), CloudsCutscenes(vm),
		DarkSideCutscenes(vm), _goober(NON_GOOBER), _finalScore(0) {}

	/**
	 * Shows the World of Xeen ending sequence
	 */
	void showWorldOfXeenEnding(GooberState state, uint score);
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_WORLDOFXEEN_CUTSCENES_H */
