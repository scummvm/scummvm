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

#include "mads/mads.h"

namespace MADS {

MADSEngine::MADSEngine(OSystem *syst, const MADSGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("MADS") {
}

MADSEngine::~MADSEngine() {
}

int MADSEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

int MADSEngine::getRandomNumber(int minNumber, int maxNumber) {
	int range = maxNumber - minNumber;

	return minNumber + _randomSource.getRandomNumber(range);
}

} // namespace MADS
