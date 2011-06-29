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

#ifndef M4_H
#define M4_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/random.h"
#include "audio/mididrv.h"

#include "engines/engine.h"

#include "m4/globals.h"
#include "m4/graphics.h"
#include "m4/resource.h"
#include "m4/saveload.h"
#include "m4/viewmgr.h"
#include "m4/gui.h"
#include "m4/events.h"
#include "m4/font.h"
#include "m4/scene.h"
#include "m4/mads_player.h"
#include "m4/mads_scene.h"
#include "m4/m4_scene.h"
#include "m4/actor.h"
#include "m4/sound.h"
#include "m4/rails.h"
#include "m4/converse.h"
#include "m4/animation.h"

//#define DUMP_SCRIPTS

/**
 * This is the namespace of the M4 engine.
 *
 * Status of this engine:
 * This engine is eventually intended to compromise two evolutions of the same engine: 'MADS' for the
 * first generation of the engine, and 'M4' for the second generation. This engine is called M4 overall
 * simply because work first began on the M4 side (focusing on the game 'Orion Burger').
 *
 * MADS Status: The current game being focused on is 'Rex Nebular' by DreamMaster, who is implementing
 * functionality as he disassembles the games original executables. Currently the engine has no particular
 * game logic implemented, although it does have the title screen implemented, and displays the initial
 * game screen and some game interface elements
 *
 * M4 Status: Work on this engine began with the game 'Orion Burger'. Some of the user interface
 * functionality has been implemented. No further work has been done on this for some time, so progress
 * on this part of the engine can be considered frozen.
 *
 * Games using this engine:
 * MADS Games: Dragonsphere, Return of the Phantom, Rex Nebular and the Cosmic Gender Bender
 * M4 Games: Orion Burger, The Riddle of Master Lu
 */
namespace M4 {

class MidiPlayer;
class FileSystem;
class ResourceManager;
class Mouse;
class Events;
class Scene;
class ViewManager;
class View;
class Inventory;
class M4InterfaceView;
class ConversationView;
class Actor;
class Converse;
class MadsConversation;
class ScriptInterpreter;
class WoodScript;
class Animation;

enum M4GameType {
	GType_Riddle = 1,
	GType_Burger = 2,
	GType_RexNebular = 3,
	GType_DragonSphere = 4,
	GType_Phantom = 5
};

enum Features {
	kFeaturesNone = 0,
	kFeaturesCD = 1 << 0,
	kFeaturesDemo = 1 << 1
};

enum {
	kFileTypeHash,
	kFileTypeHAG
};

enum {
	kDebugScript = 1 << 0,
	kDebugConversations = 1 << 1,
	kDebugGraphics = 1 << 2,
	kDebugSound = 1 << 3,
	kDebugCore = 1 << 4
};

#define MESSAGE_BASIC 1
#define MESSAGE_INTERMEDIATE 2
#define MESSAGE_DETAILED 3

struct M4GameDescription;

#define GAME_FRAME_DELAY 20

#define VALIDATE_MADS assert(!_vm->isM4())

inline void str_lower(char *s) { while (*s) { *s = tolower(*s); s++; } }
inline void str_upper(char *s) { while (*s) { *s = toupper(*s); s++; } }

inline long FixedMul(long a, long b) { return (long)(((float)a * (float)b) / 65536.0); }
inline long FixedDiv(long a, long b) { return (long)(((float)a / (float)b) * 65536.0); }

class MadsM4Engine : public Engine {
private:
	Common::Error goMADS();
	Common::Error goM4();

protected:
	// Engine APIs
	virtual Common::Error run();

	void shutdown();

	MidiPlayer *_midi;

public:
	MadsM4Engine(OSystem *syst, const M4GameDescription *gameDesc);
	virtual ~MadsM4Engine();

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	bool isM4() const { return (getGameType() == GType_Riddle) || (getGameType() == GType_Burger); }

	const char *getGameFile(int fileType);
	Common::EventManager *eventMan() { return _eventMan; }

	const M4GameDescription *_gameDescription;

	ResourceManager *res() const { return _resourceManager; }
	MidiPlayer *midi() { return _midi; }
	Common::SaveFileManager *saveManager() { return _saveFileMan; }
	void dumpFile(const char *filename, bool uncompress);
	void eventHandler();
	bool delay(int duration, bool keyAborts = true, bool clickAborts = true);
	void loadMenu(MenuType menuType, bool loadSaveFromHotkey = false,
		bool calledFromMainMenu = false);

	// TODO: eventually these have to be removed
	int32 seed;
	void imath_seed(int32 seednum) { seed = seednum; }
	uint32 imath_random() { return(seed = (25173*seed + 13849) & 0xffff); }
	int32 imath_ranged_rand(int32 a, int32 b) { return (a + (((1 + ABS<int32>(b-a))*imath_random())>>16)); }
	long imath_ranged_rand16(long a, long b) { return ((a + FixedMul(1+ABS<int32>(b-a),imath_random()))); }
	//

	ResourceManager *_resourceManager;
	Globals *_globals;

	SaveLoad *_saveLoad;
	ViewManager *_viewManager;
	Palette *_palette;
	Kernel *_kernel;
	Player *_player;
	Mouse *_mouse;
	Events *_events;
	FontManager *_font;
	Actor *_actor;
	Scene *_scene;
	Dialogs *_dialogs;
	M4Surface *_screen;
	Inventory *_inventory;
	ConversationView *_conversationView;
	Sound *_sound;
	Rails *_rails;
	ScriptInterpreter *_script;
	WoodScript *_ws;
	Common::RandomSource *_random;

	Scene *scene() { return _scene; }
};

class MadsEngine : public MadsM4Engine {
private:
	void showDialog();
public:
	MadsConversation _converse;
	uint32 _currentTimer;
	MadsPlayer _player;
public:
	MadsEngine(OSystem *syst, const M4GameDescription *gameDesc);
	virtual ~MadsEngine();

	virtual Common::Error run();

	MadsGlobals *globals() { return (MadsGlobals *)_globals; }
	MadsScene *scene() { return (MadsScene *)_scene; }
	void startScene(int sceneNum) {
		if (!_scene) {
			_scene = new MadsScene(this);
			((MadsScene *)_scene)->initialize();
		}
		_scene->show();
		_scene->loadScene(101);
	}
};

class M4Engine : public MadsM4Engine {
public:
	Converse *_converse;
public:
	M4Engine(OSystem *syst, const M4GameDescription *gameDesc);
	virtual ~M4Engine();

	virtual Common::Error run();

	M4Globals *globals() { return (M4Globals *)_globals; }
	M4Scene *scene() { return (M4Scene *)_scene; }
};

// FIXME: remove globals
extern MadsM4Engine *_vm;
extern MadsEngine *_madsVm;
extern M4Engine *_m4Vm;

} // End of namespace M4

#endif
