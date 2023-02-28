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

#ifndef MM_MM_H
#define MM_MM_H

#include "common/random.h"
#include "mm/detection.h"

namespace MM {

enum MightAndMagicDebugChannels {
	kDebugPath = 1 << 0,
	kDebugScripts = 1 << 1,
	kDebugGraphics = 1 << 2,
	kDebugSound = 1 << 3
};

class MMEngine : public Engine {
protected:
	const MightAndMagicGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
public:
	MMEngine(OSystem *syst, const MM::MightAndMagicGameDescription *gameDesc);
	~MMEngine() override {}

	/**
	 * Checks for feature flag
	 */
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Returns the features
	 */
	uint32 getFeatures() const;

	/**
	 * Returns the game language
	 */
	Common::Language getLanguage() const;

	/**
	 * Returns the game's platform
	 */
	Common::Platform getPlatform() const;

	/**
	 * Gets the game Id
	 */
	uint32 getGameID() const;

	/**
	 * Returns true if the game is the CD version
	 */
	bool getIsCD() const;

	/**
	 * Get a random number
	 */
	uint getRandomNumber(int max) {
		return _randomSource.getRandomNumber(max);
	}
};

extern MMEngine *g_engine;

} // namespace MM

#endif // MM_MM_H
