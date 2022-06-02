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

#ifndef MM1_MM1_H
#define MM1_MM1_H

#include "common/random.h"
#include "mm/detection.h"
#include "mm/mm.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"

/**
 * This is the Might and Magic I engine
 */
namespace MM {
namespace MM1 {

class MM1Engine : public Engine, public Events {
private:
	const MightAndMagicGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
private:
	// Engine APIs
	Common::Error run() override;

	bool isEnhanced() const;
	void setupNormal();
	bool setupEnhanced();
public:
	Globals _globals;
public:
	MM1Engine(OSystem *syst, const MightAndMagicGameDescription *gameDesc);
	~MM1Engine() override;

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber) {
		return _randomSource.getRandomNumber(maxNumber - minNumber + 1) + minNumber;
	}
	int getRandomNumber(int maxNumber) {
		return _randomSource.getRandomNumber(maxNumber);
	}

	Common::String getTargetName() const {
		return _targetName;
	}
};

extern MM1Engine *g_engine;

} // namespace MM1
} // namespace MM

#endif
