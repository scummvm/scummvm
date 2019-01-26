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
	ENGINE_FLAG_80 = 0x80,

	ENGINE_FLAG_80000 =   0x80000,
	ENGINE_FLAG_100000 = 0x100000
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

class BigfileArchive;
class BackgroundResourceLoader;
class DragonFLG;
class DragonOBD;
class DragonRMS;
class DragonVAR;
class DragonINIResource;
class Scene;
class Screen;
class ActorManager;
class SequenceOpcodes;
class ScriptOpcodes;
struct DragonINI;

class DragonsEngine : public Engine {
public:
	DragonOBD *_dragonOBD;
	ActorManager *_actorManager;
private:
	Screen *_screen;
	BigfileArchive *_bigfileArchive;
	DragonFLG *_dragonFLG;
	DragonRMS *_dragonRMS;
	DragonVAR *_dragonVAR;
	DragonINIResource *_dragonINIResource;
	BackgroundResourceLoader *_backgroundResourceLoader;
	Scene *_scene;
	SequenceOpcodes *_sequenceOpcodes;
	ScriptOpcodes *_scriptOpcodes;
	uint32 _nextUpdatetime;
	uint32 _flags;
	uint32 _unkFlags1;
	Common::Point _cursorPosition;

public:
	DragonsEngine(OSystem *syst);
	~DragonsEngine();

	void updateEvents();
	virtual Common::Error run();

	const char *getSavegameFilename(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);

	void updateActorSequences();
	void setFlags(uint32 flags);
	void clearFlags(uint32 flags);
	bool isFlagSet(uint32 flag);

	void setUnkFlags(uint32 flags);
	void clearUnkFlags(uint32 flags);

	byte *getBackgroundPalette();
	DragonINI *getINI(uint32 index);
	uint16 getVar(uint16 offset);
	void setVar(uint16 offset, uint16 value);
	uint16 getCurrentSceneId();

private:
	void gameLoop();
	void updateHandler();
	uint32 calulateTimeLeft();
	void wait();
};

DragonsEngine *getEngine();

} // End of namespace Dragons

#endif //SCUMMVM_DRAGONS_H
