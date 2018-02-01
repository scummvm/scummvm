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

class WorldOfXeenCutscenes : public CloudsCutscenes,
	public DarkSideCutscenes {
private:
	XeenEngine *_vm;
private:
	/**
	 * Sets a subtitle during the world of xeen ending
	 */
	void setWorldEndingSubtitle(const Common::String &msg);

	/**
	 * Part 1 of World of Xeen ending
	 */
	void worldEnding1();

	/**
	 * Part 2 of World of Xeen ending
	 */
	void worldEnding2();

	/**
	 * Part 3 of World of Xeen ending
	 */
	void worldEnding3();

	/**
	 * Part 4 of World of Xeen ending
	 */
	void worldEnding4();
protected:
	/**
	 * Shows the World of Xeen ending sequence
	 */
	void showWorldOfXeenEnding();
public:
	WorldOfXeenCutscenes(XeenEngine *vm) : _vm(vm), CloudsCutscenes(vm),
		DarkSideCutscenes(vm) {}
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_WORLDOFXEEN_CUTSCENES_H */
