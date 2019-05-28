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
#ifndef SCUMMVM_DRAGONS_H
#define SCUMMVM_DRAGONS_H

#include "gui/EventRecorder.h"
#include "engines/engine.h"

namespace Dragons {

enum {
	kGameIdDragons = 1
};

struct DragonsGameDescription {
	ADGameDescription desc;
	int gameId;
};

struct SaveHeader {
	Common::String description;
	uint32 version;
	uint32 flags;
	uint32 saveDate;
	uint32 saveTime;
	uint32 playTime;
	Graphics::Surface *thumbnail;
};

enum kReadSaveHeaderError {
	kRSHENoError = 0,
	kRSHEInvalidType = 1,
	kRSHEInvalidVersion = 2,
	kRSHEIoError = 3
};

enum Flags {
	ENGINE_FLAG_1 = 1,
	ENGINE_FLAG_2 = 2,
	ENGINE_FLAG_4 = 4,
	ENGINE_FLAG_8 = 8,
	ENGINE_FLAG_10 = 0x10,
	ENGINE_FLAG_20 = 0x20,
	ENGINE_FLAG_40 = 0x40,
	ENGINE_FLAG_80 = 0x80, // Inventory bag visible
	ENGINE_FLAG_100 = 0x100,
	ENGINE_FLAG_200 = 0x200,
	ENGINE_FLAG_400 = 0x400,

	ENGINE_FLAG_1000_TEXT_ENABLED = 0x1000,
	ENGINE_FLAG_10000    =    0x10000,
	ENGINE_FLAG_80000    =    0x80000,
	ENGINE_FLAG_100000   =   0x100000,
	ENGINE_FLAG_200000   =   0x200000,
	ENGINE_FLAG_400000   =   0x400000,
	ENGINE_FLAG_2000000  =  0x2000000,
	ENGINE_FLAG_4000000  =  0x4000000,
	ENGINE_FLAG_8000000  =  0x8000000,
	ENGINE_FLAG_20000000 = 0x20000000,
	ENGINE_FLAG_80000000 = 0x80000000
};

enum UnkFlags {
	ENGINE_UNK1_FLAG_1 = 1,
	ENGINE_UNK1_FLAG_2 = 2,
	ENGINE_UNK1_FLAG_4 = 4,
	ENGINE_UNK1_FLAG_8 = 8,
	ENGINE_UNK1_FLAG_10 = 0x10,
	ENGINE_UNK1_FLAG_20 = 0x20,
	ENGINE_UNK1_FLAG_40 = 0x40,
	ENGINE_UNK1_FLAG_80 = 0x80
};

struct opCode1AStruct {
	int16 field0;
	int16 field2;
	int16 field4;
	int16 field6;
	int16 field8;
};

class BigfileArchive;
class BackgroundResourceLoader;
class Cursor;
class DragonFLG;
class DragonIMG;
class DragonOBD;
class DragonRMS;
class DragonVAR;
class DragonINIResource;
class Inventory;
class Scene;
class Screen;
class ActorManager;
class SequenceOpcodes;
class ScriptOpcodes;
class Talk;
struct DragonINI;


class DragonsEngine : public Engine {
public:
	DragonOBD *_dragonOBD;
	DragonIMG *_dragonIMG;
	DragonRMS *_dragonRMS;
	ActorManager *_actorManager;
	DragonINIResource *_dragonINIResource;
	ScriptOpcodes *_scriptOpcodes;
	Scene *_scene;
	uint16 data_800633fa;
	Inventory *_inventory;
	Cursor *_cursor;
	Talk *_talk;

	uint16 unkArray_uint16[42];
	opCode1AStruct opCode1A_tbl[8];

	uint16 data_800633fc;

private:
	Screen *_screen;
	BigfileArchive *_bigfileArchive;
	DragonFLG *_dragonFLG;
	DragonVAR *_dragonVAR;
	BackgroundResourceLoader *_backgroundResourceLoader;
	SequenceOpcodes *_sequenceOpcodes;
	uint32 _nextUpdatetime;
	uint32 _flags;
	uint32 _unkFlags1;
	Common::Point _cursorPosition;
	uint16 _sceneId1;
	uint32 _counter;
	uint32 bit_flags_8006fbd8;
	uint16 data_8006a3a0_flag; // screen related flags?
	//unk

	uint16 run_func_ptr_unk_countdown_timer;

	// input
	bool _leftMouseButtonUp;
	bool _rightMouseButtonUp;
	bool _iKeyUp;
protected:
	virtual bool hasFeature(EngineFeature f) const;
public:
	DragonsEngine(OSystem *syst);
	~DragonsEngine();

	void updateEvents();
	virtual Common::Error run();

	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);

	virtual Common::Error loadGameState(int slot);
	virtual bool canLoadGameStateCurrently();
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual bool canSaveGameStateCurrently();

	void updateActorSequences();
	void setFlags(uint32 flags);
	void clearFlags(uint32 flags);
	bool isFlagSet(uint32 flag);
	bool isUnkFlagSet(uint32 flag);

	void setUnkFlags(uint32 flags);
	void clearUnkFlags(uint32 flags);

	byte *getBackgroundPalette();
	DragonINI *getINI(uint32 index);
	uint16 getVar(uint16 offset);
	void setVar(uint16 offset, uint16 value);
	uint16 getCurrentSceneId();

	void waitForFrames(uint16 numFrames);

	void playSound(uint16 soundId);

	//TODO what are these functions really doing?
	void call_fade_related_1f();
	void fade_related(uint32 flags);

	uint16 ipt_img_file_related();
	void works_with_obd_data_1();

	void reset_screen_maybe();

	void loadScene(uint16 sceneId);

	void reset();

private:
	bool savegame(const char *filename, const char *description);
	bool loadgame(const char *filename);
	void gameLoop();
	void updateHandler();
	void updatePathfindingActors();
	uint32 calulateTimeLeft();
	void wait();
	uint16 getIniFromImg();
	void runINIScripts();
	void engineFlag0x20UpdateFunction();


	bool isInputEnabled();
	bool checkForInventoryButtonRelease();
	bool checkForActionButtonRelease();

	void FUN_8003130c();
	void FUN_80038890();
	void walkFlickerToObject();
	void FUN_80038994();

	void FUN_8002931c();
};

DragonsEngine *getEngine();

} // End of namespace Dragons

#endif //SCUMMVM_DRAGONS_H
