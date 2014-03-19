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

#ifndef ILLUSIONS_ILLUSIONS_H
#define ILLUSIONS_ILLUSIONS_H

#include "audio/mixer.h"
#include "audio/decoders/aiff.h"
#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "common/winexe.h"
#include "common/winexe_pe.h"
#include "engines/engine.h"
#include "graphics/surface.h"

struct ADGameDescription;

namespace Illusions {

#define ILLUSIONS_SAVEGAME_VERSION 0

class ResourceSystem;

struct SurfInfo;

class ActorItem;
class ActorItems;
class ActorType;
class BackgroundItem;
class BackgroundItems;
class BackgroundResource;
class Camera;
class Control;
class Controls;
class Dictionary;
class Input;
class Screen;
class ScriptResource;
class ScriptMan;
class Sequence;
class FramesList;

class IllusionsEngine : public Engine {
protected:
	Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	IllusionsEngine(OSystem *syst, const ADGameDescription *gd);
	~IllusionsEngine();
	const Common::String getTargetName() { return _targetName; }
	
private:
	const ADGameDescription *_gameDescription;
	Graphics::PixelFormat _pixelFormat;
public:	
	Common::RandomSource *_random;
	Dictionary *_dict;
	ResourceSystem *_resSys;
	
	void updateEvents();

	Screen *_screen;
	Input *_input;
	ScriptMan *_scriptMan;
	ActorItems *_actorItems;
	BackgroundItems *_backgroundItems;
	Camera *_camera;
	Controls *_controls;
	
	int _resGetCtr;
	uint32 _resGetTime;
	bool _unpauseControlActorFlag;
	uint32 _lastUpdateTime;

	Common::Point *getObjectActorPositionPtr(uint32 objectId);
	
	void notifyThreadId(uint32 &threadId);
	
	void setCursorControl(Control *control);
	void placeCursor(Control *control, uint32 sequenceId);
	bool showCursor();
	bool hideCursor();
	uint32 getElapsedUpdateTime();
	int updateActors();
	int updateSequences();
	int updateGraphics();
	int getRandom(int max);
	int convertPanXCoord(int16 x);
	Common::Point getNamedPointPosition(uint32 namedPointId);
	void playVideo(uint32 videoId, uint32 objectId, uint32 value, uint32 threadId);

#if 0

	// Savegame API

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &description);
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	bool existsSavegame(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, bool loadThumbnail, SaveHeader &header);

#endif
	
};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
