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

#include "illusions/graphics.h"
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

#include "engines/engine.h"
#include "graphics/surface.h"
#include "illusions/detection.h"

namespace Illusions {

char *debugW2I(uint16 *wstr);
void swapBytesInWideString(byte * wstr);

#define ILLUSIONS_SAVEGAME_VERSION 0

class ResourceSystem;
class BaseResourceReader;

struct SurfInfo;

class ActorInstanceList;
struct ActorType;
class BackgroundInstanceList;
class BackgroundResource;
class Camera;
class Control;
class Controls;
class Cursor;
class Dictionary;
struct Fader;
class FramesList;
class Input;
class Screen;
class ScreenText;
class ScriptOpcodes;
class ScriptResource;
class ScriptStack;
struct Sequence;
class SoundMan;
class SpecialCode;
class TalkInstanceList;
class ThreadList;
class UpdateFunctions;
class GameState;
class ScreenPaletteBase;

class IllusionsEngine : public Engine {
public:
	IllusionsEngine(OSystem *syst, const IllusionsGameDescription *gd);
	~IllusionsEngine() override;
	const Common::String getTargetName() { return _targetName; }
private:
	const IllusionsGameDescription *_gameDescription;
	Graphics::PixelFormat _pixelFormat;
public:

	Common::RandomSource *_random;
	Dictionary *_dict;
	ResourceSystem *_resSys;
	BaseResourceReader *_resReader;
	UpdateFunctions *_updateFunctions;
	GameState *_gameState;

	void updateEvents();

	Screen *_screen;
	ScreenPaletteBase *_screenPalette;
	ScreenText *_screenText;
	Input *_input;
	ActorInstanceList *_actorInstances;
	BackgroundInstanceList *_backgroundInstances;
	Camera *_camera;
	Controls *_controls;
	TalkInstanceList *_talkItems;
	ScriptOpcodes *_scriptOpcodes;
	SpecialCode *_specialCode;
	ThreadList *_threads;
	SoundMan *_soundMan;

	uint32 _nextTempThreadId;
	bool _doScriptThreadInit;
	ScriptStack *_stack;
	ScriptResource *_scriptResource;
	bool _rerunThreads;

	Fader *_fader;

	int _pauseCtr;

	int _resGetCtr;
	uint32 _resGetTime;
	bool _unpauseControlActorFlag;
	uint32 _lastUpdateTime;

	int _resumeFromSavegameRequested;
	int _savegameSlotNum;
	Common::String _savegameDescription;
	uint32 _savegameSceneId;
	uint32 _savegameThreadId;

	uint32 _fontId;
	int _field8;
	uint32 _fieldA;
	uint32 _subtitleDuration;

	WidthHeight _defaultTextDimensions;
	Common::Point _defaultTextPosition;

	int16 _menuChoiceOfs;

	int getGameId() const;
	Common::Language getGameLanguage() const;

	void runUpdateFunctions();
	int updateActors(uint flags);
	int updateSequences(uint flags);
	int updateGraphics(uint flags);
	int updateSoundMan(uint flags);
	int updateSprites(uint flags);

	uint32 getElapsedUpdateTime();
	Common::Point *getObjectActorPositionPtr(uint32 objectId);
	int getRandom(int max);
	int convertPanXCoord(int16 x);
	bool calcPointDirection(Common::Point &srcPt, Common::Point &dstPt, uint &facing);
	bool isSoundActive();

	virtual void updateFader() {};
	virtual void clearFader() {};
	virtual void pauseFader() {};
	virtual void unpauseFader() {};
	virtual bool isVideoPlaying() { return false; }

	void setCurrFontId(uint32 fontId);
	bool checkActiveTalkThreads();
	void setTextDuration(int kind, uint32 duration);
	uint32 clipTextDuration(uint32 duration);
	void getDefaultTextDimensions(WidthHeight &dimensions);
	void setDefaultTextDimensions(WidthHeight &dimensions);
	void getDefaultTextPosition(Common::Point &position);
	void setDefaultTextPosition(Common::Point &position);

	uint16 getSubtitleDuration();
	void setSubtitleDuration(uint16 duration);

	FramesList *findActorSequenceFrames(Sequence *sequence);

	virtual void setDefaultTextCoords() = 0;
	virtual void loadSpecialCode(uint32 resId) = 0;
	virtual void unloadSpecialCode(uint32 resId) = 0;
	virtual void notifyThreadId(uint32 &threadId) = 0;
	virtual bool testMainActorFastWalk(Control *control) = 0;
	virtual bool testMainActorCollision(Control *control) = 0;
	virtual Control *getObjectControl(uint32 objectId) = 0;
	virtual Common::Point getNamedPointPosition(uint32 namedPointId) = 0;
	virtual uint32 getPriorityFromBase(int16 priority) = 0;
	virtual uint32 getPrevScene() = 0;
	virtual uint32 getCurrentScene() = 0;
	virtual bool isCursorObject(uint32 actorTypeId, uint32 objectId) = 0;
	virtual void setCursorControlRoutine(Control *control) = 0;
	virtual void placeCursorControl(Control *control, uint32 sequenceId) = 0;
	virtual void setCursorControl(Control *control) = 0;
	virtual void showCursor() = 0;
	virtual void hideCursor() = 0;
	virtual void startScriptThreadSimple(uint32 threadId, uint32 callingThreadId) = 0;
	virtual uint32 startTempScriptThread(byte *scriptCodeIp, uint32 callingThreadId,
		uint32 value8, uint32 valueC, uint32 value10) = 0;

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

	bool canLoadGameStateCurrently() override { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() override { return _isSaveAllowed; }
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error removeGameState(int slot);
	bool savegame(const char *filename, const char *description);
	bool loadgame(const char *filename);
	const char *getSavegameFilename(int num);
	bool existsSavegame(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);

};

} // End of namespace Illusions

#endif // ILLUSIONS_ILLUSIONS_H
