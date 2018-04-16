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

#ifndef MADS_MADS_H
#define MADS_MADS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "mads/conversations.h"
#include "mads/debugger.h"
#include "mads/dialogs.h"
#include "mads/events.h"
#include "mads/font.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/resources.h"
#include "mads/sound.h"

/**
 * This is the namespace of the MADS engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Rex Nebular and the Cosmic Gender Bender
 */
namespace MADS {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

enum MADSDebugChannels {
	kDebugPath      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2
};

enum {
	GType_RexNebular = 0,
	GType_Dragonsphere = 1,
	GType_Phantom = 2
};

enum ScreenFade {
	SCREEN_FADE_SMOOTH = 0,
	SCREEN_FADE_MEDIUM = 1,
	SCREEN_FADE_FAST = 2
};

struct MADSGameDescription;


class MADSEngine : public Engine {
private:
	const MADSGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	/**
	 * Handles basic initialisation
	 */
	void initialize();

	void loadOptions();
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	Debugger *_debugger;
	Dialogs *_dialogs;
	EventsManager *_events;
	Font *_font;
	Game *_game;
	GameConversations * _gameConv;
	Palette *_palette;
	Resources *_resources;
	Screen *_screen;
	SoundManager *_sound;
	AudioPlayer *_audio;
	bool _easyMouse;
	bool _invObjectsAnimated;
	bool _textWindowStill;
	ScreenFade _screenFade;
	bool _musicFlag;
	bool _soundFlag;
	bool _dithering;
	bool _disableFastwalk;
public:
	MADSEngine(OSystem *syst, const MADSGameDescription *gameDesc);
	virtual ~MADSEngine();

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;

	int getRandomNumber(int maxNumber);
	int getRandomNumber(int minNumber, int maxNumber);
	int hypotenuse(int xv, int yv);

	/**
	* Returns true if it is currently okay to restore a game
	*/
	bool canLoadGameStateCurrently();

	/**
	* Returns true if it is currently okay to save the game
	*/
	bool canSaveGameStateCurrently();

	/**
	* Support method that generates a savegame name
	* @param slot		Slot number
	*/
	Common::String generateSaveName(int slot);

	/**
	 * Handles loading a game via the GMM
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Handles saving the game via the GMM
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	/**
	 * Handles updating sound settings after they're changed in the GMM dialog
	 */
	virtual void syncSoundSettings();

	void saveOptions();
};

} // End of namespace MADS

#endif /* MADS_MADS_H */
