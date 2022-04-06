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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"

namespace Hypno {

BoyzEngine::BoyzEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
	_screenW = 320;
	_screenH = 200;
	_lives = 2;
}

void BoyzEngine::loadAssets() {
	LibFile *missions = loadLib("", "preload/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");
	loadArcadeLevel("c11.mi_", "c12.mi_", "??", "");
	loadArcadeLevel("c12.mi_", "c14.mi_", "??", "");
	//loadArcadeLevel("c13.mi_", "??", "??", "");
	loadArcadeLevel("c14.mi_", "c15.mi_", "??", "");
	loadArcadeLevel("c15.mi_", "c16.mi_", "??", "");

	loadLib("sound/", "misc/sound.lib", true);

	_weaponShootSound[0] = "pstlfire.raw";
	_weaponShootSound[2] = "ak47fire.raw";
	_weaponShootSound[4] = "shotfire.raw";

	Graphics::Surface *targets = decodeFrame("preload/targets.smk", 0, &_crosshairsPalette);

	Common::Rect cursorBox = Common::Rect(16, 8, 32, 24);
	_crosshairsInactive[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 40, 32, 56);
	_crosshairsActive[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 72, 32, 88);
	_crosshairsTarget[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	_nextLevel = "c11.mi_";
}

Common::String BoyzEngine::findNextLevel(const Common::String &level) { return level; }

} // namespace Hypno
