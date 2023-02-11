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
#include "common/serializer.h"
#include "mm/detection.h"
#include "mm/mm.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"

/**
 * This is the Might and Magic I engine
 */
namespace MM {
namespace MM1 {

class MM1Engine : public MMEngine, public Events {
private:
	// Engine APIs
	Common::Error run() override;

	void setupNormal();
	bool setupEnhanced();
public:
	Globals _globals;
public:
	MM1Engine(OSystem *syst, const MightAndMagicGameDescription *gameDesc);
	~MM1Engine() override;

	bool isEnhanced() const;

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber) {
		return _randomSource.getRandomNumber(maxNumber - minNumber + 1) + minNumber;
	}
	int getRandomNumber(int maxNumber) {
		return getRandomNumber(1, maxNumber - 1);
	}

	Common::String getTargetName() const {
		return _targetName;
	}

	/**
	 * Returns true if a game can be saved
	 */
	bool canSaveGameStateCurrently() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently() override;

	/**
	 * Saves or loads a savegame
	 */
	Common::Error synchronizeSave(Common::Serializer &s);

	/**
	 * Load a savegame
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return synchronizeSave(s);
	}

	/**
	 * Save a savegame
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return synchronizeSave(s);
	}
};

extern MM1Engine *g_engine;

} // namespace MM1
} // namespace MM

#endif
