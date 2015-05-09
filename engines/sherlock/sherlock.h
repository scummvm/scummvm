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

#ifndef SHERLOCK_HOLMES_H
#define SHERLOCK_HOLMES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/hash-str.h"
#include "common/serializer.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/util.h"
#include "engines/engine.h"
#include "sherlock/animation.h"
#include "sherlock/debugger.h"
#include "sherlock/events.h"
#include "sherlock/inventory.h"
#include "sherlock/journal.h"
#include "sherlock/map.h"
#include "sherlock/people.h"
#include "sherlock/resources.h"
#include "sherlock/saveload.h"
#include "sherlock/scene.h"
#include "sherlock/screen.h"
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

enum GameType {
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

	void loadConfig();
protected:
	virtual void initialize();

	virtual void showOpening() = 0;

	virtual void startScene() {}

	virtual bool hasFeature(EngineFeature f) const;
public:
	const SherlockGameDescription *_gameDescription;
	Animation *_animation;
	Debugger *_debugger;
	Events *_events;
	Inventory *_inventory;
	Journal *_journal;
	Map *_map;
	People *_people;
	Resources *_res;
	SaveManager *_saves;
	Scene *_scene;
	Screen *_screen;
	Sound *_sound;
	Talk *_talk;
	UserInterface *_ui;
	Common::RandomSource _randomSource;
	Common::Array<bool> _flags;
	Common::String _soundOverride;
	Common::String _titleOverride;
	bool _useEpilogue2;
	int _loadGameSlot;
	bool _canLoadSave;
public:
	SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	virtual ~SherlockEngine();

	virtual Common::Error run();

	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual void syncSoundSettings();

	int getGameType() const;
	GameType getGameID() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	Common::String getGameFile(int fileType);

	int getRandomNumber(int limit) { return _randomSource.getRandomNumber(limit - 1); }

	bool readFlags(int flagNum);

	void setFlags(int flagNum);

	void saveConfig();

	void synchronize(Common::Serializer &s);
};

} // End of namespace Sherlock

#endif
