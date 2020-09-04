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

#ifndef ULTIMA_SHARED_ENGINE_ULTIMA_H
#define ULTIMA_SHARED_ENGINE_ULTIMA_H

#include "ultima/detection.h"
#include "ultima/shared/engine/debugger.h"
#include "ultima/shared/engine/events.h"
#include "common/archive.h"
#include "common/random.h"
#include "engines/engine.h"

namespace Ultima {
namespace Shared {

enum UltimaDebugChannels {
	kDebugPath = 1 << 0,
	kDebugGraphics = 1 << 1
};

class UltimaEngine : public Engine, public EventsCallback {
private:
	Common::RandomSource _randomSource;
protected:
	const UltimaGameDescription *_gameDescription;
	Common::Archive *_dataArchive;
protected:
	/**
	 * Initializes needed data for the engine
	 */
	virtual bool initialize();

	/**
	 * Deinitialize the engine
	 */
	virtual void deinitialize() {}

	/**
	 * Returns the data archive folder and version that's required
	 */
	virtual bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
		return false;
	}

public:
	EventsManager *_events;
public:
	UltimaEngine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~UltimaEngine() override;

	/**
	 * Returns supported engine features
	 */
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Returns game features
	 */
	uint32 getFeatures() const;

	/**
	 * Return the game's language
	 */
	Common::Language getLanguage() const;

	/**
	 * Returns the game type being played
	 */
	GameId getGameId() const;

	/**
	 * Returns true if the game is running an enhanced version
	 * as compared to the original game
	 */
	bool isEnhanced() const {
		return getFeatures() & GF_VGA_ENHANCED;
	}

	/**
	 * Show a message in a GUI dialog
	 */
	void GUIError(const Common::U32String &msg);

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint maxVal) { return _randomSource.getRandomNumber(maxVal); }

	/**
	 * Gets a random number
	 */
	uint getRandomNumber(uint min, uint max) {
		return min + _randomSource.getRandomNumber(max - min);
	}

	/**
	 * Returns a file system node for the game directory
	 */
	Common::FSNode getGameDirectory() const;

	/**
	 * Indicates whether a game state can be loaded.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	virtual bool canLoadGameStateCurrently(bool isAutosave) = 0;

	/**
	 * Indicates whether a game state can be loaded.
	 */
	bool canLoadGameStateCurrently() override {
		return canLoadGameStateCurrently(false);
	}

	/**
	 * Indicates whether a game state can be saved.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	virtual bool canSaveGameStateCurrently(bool isAutosave) = 0;

	/**
	 * Indicates whether a game state can be saved.
	 */
	bool canSaveGameStateCurrently() override {
		return canSaveGameStateCurrently(false);
	}
};

extern UltimaEngine *g_ultima;

} // End of namespace Shared
} // End of namespace Ultima

#endif
