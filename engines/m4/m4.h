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
 * $URL$
 * $Id$
 *
 */

#ifndef M4_H
#define M4_H

#include "common/scummsys.h"
#include "common/util.h"

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
#include "m4/actor.h"
#include "m4/sound.h"
#include "m4/rails.h"
#include "m4/converse.h"
#include "m4/animation.h"

//#define DUMP_SCRIPTS

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
class GameInterfaceView;
class ConversationView;
class Actor;
class Converse;
class ScriptInterpreter;
class WoodScript;
class Animation;

enum M4GameType {
	GType_Riddle = 1,
	GType_Burger,
	GType_RexNebular,
	GType_DragonSphere,
	GType_Phantom
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
	kDebugConversations = 2 << 0
};

#define MESSAGE_BASIC 1
#define MESSAGE_INTERMEDIATE 2
#define MESSAGE_DETAILED 3

struct M4GameDescription;

#define GAME_FRAME_DELAY 50

inline void str_lower(char *s) { while (*s) { *s = tolower(*s); s++; } }
inline void str_upper(char *s) { while (*s) { *s = toupper(*s); s++; } }

inline long FixedMul(long a, long b) { return (long)(((float)a * (float)b) / 65536.0); }
inline long FixedDiv(long a, long b) { return (long)(((float)a / (float)b) * 65536.0); }

class M4Engine : public Engine {
private:
	Common::Error goMADS();
	Common::Error goM4();

protected:
	// Engine APIs
	virtual Common::Error run();

	void shutdown();

	MidiPlayer *_midi;

public:
	M4Engine(OSystem *syst, const M4GameDescription *gameDesc);
	virtual ~M4Engine();

	int getGameType() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	bool isM4() const { return (getGameType() == GType_Riddle) || (getGameType() == GType_Burger); }

	const char *getGameFile(int fileType);
	Common::EventManager *eventMan() { return _eventMan; }
	OSystem *system() { return _system; }

	const M4GameDescription *_gameDescription;

	ResourceManager *res() const { return _resourceManager; }
	MidiPlayer *midi() { return _midi; }
	Common::SaveFileManager *saveManager() { return _saveFileMan; }
	void dumpFile(const char* filename, bool uncompress = false);
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
	SaveLoad *_saveLoad;
	ViewManager *_viewManager;
	Palette *_palette;
	Kernel *_kernel;
	Globals *_globals;
	Player *_player;
	Mouse *_mouse;
	Events *_events;
	Font *_font;
	Actor *_actor;
	Scene *_scene;
	Dialogs *_dialogs;
	M4Surface *_screen;
	Inventory *_inventory;
	GameInterfaceView *_interfaceView;
	ConversationView *_conversationView;
	Sound *_sound;
	Rails *_rails;
	Converse *_converse;
	ScriptInterpreter *_script;
	WoodScript *_ws;
	Animation *_animation;
	Common::RandomSource *_random;
};

// FIXME: remove global
extern M4Engine *_vm;

} // End of namespace M4

#endif
