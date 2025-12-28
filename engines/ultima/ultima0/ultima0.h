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

#ifndef ULTIMA0_H
#define ULTIMA0_H

#include "common/random.h"
#include "graphics/palette.h"
#include "engines/engine.h"
#include "ultima/detection.h"
#include "ultima/ultima0/defines.h"
#include "ultima/ultima0/events.h"

namespace Ultima {
namespace Ultima0 {

class Ultima0Engine : public Engine, public Events {
private:
	Common::RandomSource _randomSource;

	const UltimaGameDescription *_gameDescription;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Palette _palette;

	Ultima0Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima0Engine() override;

	/**
	 * Returns true if the game should quit
	 */
	bool shouldQuit() const override {
		return Engine::shouldQuit();
	}

	/**
	 * Returns supported engine features
	 */
	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsReturnToLauncher);
	}

	/**
	 * Sets the random number seed
	 */
	void setRandomSeed(uint seed) {
		_randomSource.setSeed(seed);
	}

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint maxVal = RND_MAX) { return _randomSource.getRandomNumber(maxVal); }
};

extern Ultima0Engine *g_engine;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
