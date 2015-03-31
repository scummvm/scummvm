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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHERLOCK_HOLMES_H
#define SHERLOCK_HOLMES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/util.h"
#include "engines/engine.h"
#include "sherlock/animation.h"
#include "sherlock/debugger.h"
#include "sherlock/events.h"
#include "sherlock/inventory.h"
#include "sherlock/journal.h"
#include "sherlock/people.h"
#include "sherlock/resources.h"
#include "sherlock/scene.h"
#include "sherlock/screen.h"
#include "sherlock/scripts.h"
#include "sherlock/sound.h"
#include "sherlock/talk.h"
#include "sherlock/user_interface.h"

namespace Sherlock {

enum {
	kFileTypeHash
};

enum {
	kDebugScript = 1 << 0
};

enum {
	GType_SerratedScalpel = 0,
	GType_RoseTattoo = 1
};

#define SHERLOCK_SCREEN_WIDTH 320
#define SHERLOCK_SCREEN_HEIGHT 200
#define SHERLOCK_SCENE_HEIGHT 138

struct SherlockGameDescription;

class Resource;

class SherlockEngine : public Engine {
private:
	void sceneLoop();

	void handleInput();
protected:
	virtual void initialize();

	virtual void showOpening() = 0;

	virtual void startScene() {}
public:
	const SherlockGameDescription *_gameDescription;
	Animation *_animation;
	Debugger *_debugger;
	Events *_events;
	Inventory *_inventory;
	Journal *_journal;
	People *_people;
	Resources *_res;
	Scene *_scene;
	Screen *_screen;
	Scripts *_scripts;
	Sound *_sound;
	Talk *_talk;
	UserInterface *_ui;
	Common::RandomSource _randomSource;
	Common::Array<bool> _flags;
	Common::String _soundOverride;
	Common::String _titleOverride;
	bool _useEpilogue2;
	bool _justLoaded;
	bool _loadingSavedGame;
	int _oldCharPoint;					// Old scene
	Common::Point _over;				// Old map position
	Common::Array<Common::Point> _map;	// Map locations for each scene
	bool _onChessboard;
	bool _slowChess;
public:
	SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	virtual ~SherlockEngine();

	virtual Common::Error run();

	int getGameType() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	Common::String getGameFile(int fileType);

	int getRandomNumber(int limit) { return _randomSource.getRandomNumber(limit - 1); }

	bool readFlags(int flagNum);

	void setFlags(int flagNum);

	void freeSaveGameList();
};

} // End of namespace Sherlock

#endif
