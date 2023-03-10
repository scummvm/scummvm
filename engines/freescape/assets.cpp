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

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::loadAssets() {
	if (isDemo())
		loadAssetsDemo();
	else
		loadAssetsFullGame();
}

void FreescapeEngine::loadAssetsDemo() {
}

void FreescapeEngine::loadAssetsAtariDemo() {
}

void FreescapeEngine::loadAssetsAmigaDemo() {
}

void FreescapeEngine::loadAssetsDOSDemo() {
}

void FreescapeEngine::loadAssetsZXDemo() {
}

void FreescapeEngine::loadAssetsCPCDemo() {
}

void FreescapeEngine::loadAssetsFullGame() {
}

void FreescapeEngine::loadAssetsAtariFullGame() {
}

void FreescapeEngine::loadAssetsAmigaFullGame() {
}

void FreescapeEngine::loadAssetsDOSFullGame() {
}

void FreescapeEngine::loadAssetsZXFullGame() {
}

void FreescapeEngine::loadAssetsCPCFullGame() {
}

} // End of namespace Freescape
