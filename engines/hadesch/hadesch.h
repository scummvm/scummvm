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
 * Copyright 2020 Google
 *
 */
#ifndef HADESCH_HADESCH_H
#define HADESCH_HADESCH_H

#include "common/random.h"
#include "common/stream.h"
#include "common/savefile.h"
#include "common/list.h"

#include "engines/engine.h"
#include "engines/savestate.h"

#include "gui/debugger.h"
#include "graphics/cursor.h"
#include "hadesch/pod_file.h"
#include "hadesch/video.h"
#include "hadesch/enums.h"
#include "hadesch/event.h"
#include "hadesch/herobelt.h"
#include "hadesch/persistent.h"
#include "common/translation.h"

struct ADGameDescription;

namespace Common {
class SeekableReadStream;
class PEResources;
}

namespace Graphics {
struct WinCursorGroup;
class MacCursor;
}

namespace Hadesch {

class VideoRoom;

class Handler {
public:
	virtual void handleClick(const Common::String &name) = 0;
	virtual void handleAbsoluteClick(Common::Point pnt) {}
	virtual bool handleClickWithItem(const Common::String &name, InventoryItem item) {
		return false;
	}
	virtual void handleEvent(int eventId) = 0;
	virtual void handleMouseOver(const Common::String &name) {}
	virtual void handleMouseOut(const Common::String &name) {}
	virtual void frameCallback() {}
	virtual void handleKeypress(uint32 ucode) {}
	virtual void prepareRoom() = 0;
	virtual bool handleCheat(const Common::String &cheat) {
		return false;
	}
	virtual void handleUnclick(const Common::String &name, Common::Point pnt) {}
	virtual ~Handler() {}
};

Common::SharedPtr<Hadesch::Handler> makeOlympusHandler();
Common::SharedPtr<Hadesch::Handler> makeWallOfFameHandler();
Common::SharedPtr<Hadesch::Handler> makeArgoHandler();
Common::SharedPtr<Hadesch::Handler> makeCreteHandler();
Common::SharedPtr<Hadesch::Handler> makeMinosHandler();
Common::SharedPtr<Hadesch::Handler> makeDaedalusHandler();
Common::SharedPtr<Hadesch::Handler> makeSeriphosHandler();
Common::SharedPtr<Hadesch::Handler> makeMedIsleHandler();
Common::SharedPtr<Hadesch::Handler> makeTroyHandler();
Common::SharedPtr<Hadesch::Handler> makeMinotaurHandler();
Common::SharedPtr<Hadesch::Handler> makeQuizHandler();
Common::SharedPtr<Hadesch::Handler> makeCatacombsHandler();
Common::SharedPtr<Hadesch::Handler> makePriamHandler();
Common::SharedPtr<Hadesch::Handler> makeAthenaHandler();
Common::SharedPtr<Hadesch::Handler> makeVolcanoHandler();
Common::SharedPtr<Hadesch::Handler> makeRiverStyxHandler();
Common::SharedPtr<Hadesch::Handler> makeHadesThroneHandler();
Common::SharedPtr<Hadesch::Handler> makeCreditsHandler(bool inOptions);
Common::SharedPtr<Hadesch::Handler> makeIntroHandler();
Common::SharedPtr<Hadesch::Handler> makeFerryHandler();
Common::SharedPtr<Hadesch::Handler> makeOptionsHandler();
Common::SharedPtr<Hadesch::Handler> makeMonsterHandler();
Common::SharedPtr<Hadesch::Handler> makeMedusaHandler();
Common::SharedPtr<Hadesch::Handler> makeTrojanHandler();

class HadeschEngine : public Engine, Common::NonCopyable {
public:
	HadeschEngine(OSystem *syst, const ADGameDescription *desc);
	~HadeschEngine() override;

	Common::Error run() override;

	bool hasFeature(EngineFeature f) const override;

	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return _persistent._currentRoomId != 0; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	Common::SeekableReadStream *openFile(const Common::String &name, bool addCurrentPath);

	Common::RandomSource &getRnd();

	const Common::String &getCDScenesPath() const;

	Common::SharedPtr<VideoRoom> getVideoRoom();

	void moveToRoom(RoomId id) {
		_nextRoom.push_back(id);
		_heroBelt->clearHold();
	}

	void handleEvent(EventHandlerWrapper event);
	int32 getCurrentTime() {
		return _currentTime;
	}
	Common::Point getMousePos();

	void addTimer(EventHandlerWrapper event, int period, int repeat = 1);
	void addSkippableTimer(EventHandlerWrapper event, int period, int repeat = 1);
	void cancelTimer(int eventId);

	Common::SharedPtr<PodFile> getWdPodFile() {
		return _wdPodFile;
	}

	RoomId getPreviousRoomId() const {
		return _persistent._previousRoomId;
	}

	bool isRoomVisited(RoomId id) const {
		return _persistent._roomVisited[id];
	}

	Persistent *getPersistent() {
		return &_persistent;
	}

	Common::SharedPtr<Handler> getCurrentHandler();

	Common::SharedPtr<HeroBelt> getHeroBelt() {
		return _heroBelt;
	}
	
	int firstAvailableSlot();

	void newGame();
	void enterOptions();
	void resetOptionsRoom();
	void exitOptions();
	void enterOptionsCredits();
	void quit();
	bool hasAnySaves();

	Common::Array<HadeschSaveDescriptor> getHadeschSavesList();
	void deleteSave(int slot);
	int genSubtitleID();
	uint32 getSubtitleDelayPerChar() const;
	void wrapSubtitles(const Common::U32String &str, Common::Array<Common::U32String> &lines);
	Common::U32String translate(const Common::String &str);

private:
	void addTimer(EventHandlerWrapper event, int32 start_time, int period,
		      int repeat, bool skippable);
	void moveToRoomReal(RoomId id);
	void setVideoRoom(Common::SharedPtr<VideoRoom> scene,
			  Common::SharedPtr<Handler> handler,
			  RoomId roomId);
	Common::ErrorCode loadCursors();
	bool handleGenericCheat(const Common::String &cheat);
	Common::ErrorCode loadWindowsCursors(Common::PEResources &exe);

	struct Timer {
		int32 next_time;
		int32 period;
		int32 period_count;
		EventHandlerWrapper event;
		bool skippable;
	};
	const ADGameDescription *_desc;

	Common::RandomSource _rnd;

	Common::String _cdScenesPath;
	
	Common::SharedPtr<VideoRoom> _sceneVideoRoom;
	Common::SharedPtr<Handler> _sceneHandler;
	Common::SharedPtr<VideoRoom> _optionsRoom;
	Common::SharedPtr<Handler> _optionsHandler;
	bool _isInOptions;
	uint32 _optionsEnterTime;
	uint32 _sceneStartTime;
	int32 _currentTime;
	Common::Array<Graphics::Cursor *> _cursors;
	Common::List<Timer> _sceneTimers;
	Common::SharedPtr<PodFile> _wdPodFile;
	Common::SharedPtr<HeroBelt> _heroBelt;
	Common::String _cheat;
	Common::SharedPtr<GfxContext8Bit> _gfxContext;
	bool _cheatsEnabled;
	Common::Point _mousePos;

	Persistent _persistent;
	Common::Array<RoomId> _nextRoom;
	bool _isRestoring;
  	bool _isQuitting;
	int _subtitleID;
	int _subtitleDelayPerChar;

#ifdef USE_TRANSLATION
	Common::TranslationManager *_transMan;
#endif

	// For freeing purposes
	Common::Array <Graphics::MacCursor *> _macCursors;
	Common::Array <Graphics::WinCursorGroup *> _winCursors;
};

extern HadeschEngine *g_vm;

} // End of namespace Hadesch

#endif
