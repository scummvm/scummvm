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

#ifndef MADS_NEBULAR_H
#define MADS_NEBULAR_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "mads/mads.h"
#include "mads/nebular/debugger.h"
#include "mads/core/conversations.h"
#include "mads/core/dialogs.h"
#include "mads/core/events.h"
#include "mads/core/font.h"
#include "mads/core/game.h"
#include "mads/core/screen.h"
#include "mads/core/msurface.h"
#include "mads/core/resources.h"
#include "mads/core/sound.h"

namespace MADS {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

class RexNebularEngine : public MADSEngine {
private:
	/**
	 * Handles basic initialisation
	 */
	void initialize();

	void loadOptions();

protected:
	// Engine APIs
	Common::Error run() override;

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
	RexNebularEngine(OSystem *syst, const MADSGameDescription *gameDesc);
	~RexNebularEngine() override;

	/**
	* Returns true if it is currently okay to restore a game
	*/
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	* Returns true if it is currently okay to save the game
	*/
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Handles loading a game via the GMM
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Handles saving the game via the GMM
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Handles updating sound settings after they're changed in the GMM dialog
	 */
	void syncSoundSettings() override;

	void saveOptions();
};

} // namespace MADS

#endif
