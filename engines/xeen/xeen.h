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

#ifndef XEEN_XEEN_H
#define XEEN_XEEN_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "xeen/debugger.h"
#include "xeen/events.h"
#include "xeen/screen.h"
#include "xeen/sound.h"

/**
 * This is the namespace of the Xeen engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Might & Magic 4: Clouds of Xeen
 * - Might & Magic 5: Dark Side of Xeen
 * - Might & Magic: World of Xeen
 * - Might & Magic: Swords of Xeen
 */
namespace Xeen {

enum {
	GType_Clouds = 1,
	GType_DarkSide = 2,
	GType_WorldOfXeen = 3,
	GType_Swords = 4
};

enum XeenDebugChannels {
	kDebugPath      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2,
	kDebugSound     = 1 << 3
};

enum Mode {
	MODE_0 = 0,
	MODE_9 = 9
};

struct XeenGameDescription;

#define XEEN_SAVEGAME_VERSION 1
#define GAME_FRAME_TIME 50

struct XeenSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class XeenEngine : public Engine {
private:
	const XeenGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	int _loadSaveSlot;
protected:
	/**
	* Play the game
	*/
	virtual void playGame();
private:
	void initialize();

	/**
	* Synchronize savegame data
	*/
	void synchronize(Common::Serializer &s);

	/**
	* Support method that generates a savegame name
	* @param slot		Slot number
	*/
	Common::String generateSaveName(int slot);

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	Debugger *_debugger;
	EventsManager *_events;
	Screen *_screen;
	SoundManager *_sound;
	Mode _mode;
	GameEvent _gameEvent;
	Common::SeekableReadStream *_eventData;
public:
	XeenEngine(OSystem *syst, const XeenGameDescription *gameDesc);
	virtual ~XeenEngine();

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;

	int getRandomNumber(int maxNumber);

	/**
	 * Load a savegame
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Save the game
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	/**
	 * Returns true if a savegame can currently be loaded
	 */
	bool canLoadGameStateCurrently();

	/**
	* Returns true if the game can currently be saved
	*/
	bool canSaveGameStateCurrently();

	/**
	 * Read in a savegame header
	 */
	static bool readSavegameHeader(Common::InSaveFile *in, XeenSavegameHeader &header);

	/**
	 * Write out a savegame header
	 */
	void writeSavegameHeader(Common::OutSaveFile *out, XeenSavegameHeader &header);
};

} // End of namespace Xeen

#endif /* XEEN_XEEN_H */
