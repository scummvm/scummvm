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

#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/ini-file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/macresman.h"
#include "common/util.h"
#include "common/zlib.h"
#include "common/config-manager.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"

#include "graphics/surface.h"

#include "video/smk_decoder.h"

#include "hadesch/pod_file.h"
#include "hadesch/hadesch.h"
#include "hadesch/video.h"
#include "hadesch/pod_image.h"

#include "graphics/palette.h"
#include "common/memstream.h"
#include "common/winexe_pe.h"
#include "common/substream.h"
#include "common/md5.h"
#include "graphics/wincursor.h"
#include "graphics/cursorman.h"
#include "graphics/maccursor.h"

namespace Hadesch {

HadeschEngine *g_vm;

static const uint32 cursorids[] = {
	127, 128, 129, // normal and active
	125, 134, 135, 136, 137, 138, 139, 140, 141, 142, // waiting
	130, // up arrow
	131, // left arrow
	132, // down arrow
	133, // right arrow
	// 143:cross, 146:cross
	146
};

HadeschEngine::HadeschEngine(OSystem *system, const ADGameDescription *desc)
	: Engine(system), _desc(desc), _rnd("hadesch"), _cheatsEnabled(false) {

	DebugMan.addDebugChannel(kHadeschDebugGeneral, "general", "General issues");
	DebugMan.addDebugChannel(kHadeschDebugMessagingSystem, "resources", "Resources");
	DebugMan.addDebugChannel(kHadeschDebugMessagingSystem, "message_system", "Engine message system");
	DebugMan.addDebugChannel(kHadeschDebugDialogs, "dialogs", "Dialogs");

	g_vm = this;
	_sceneStartTime = _system->getMillis();
	_currentTime = 0;
	_isQuitting = false;
	_isRestoring = false;

	_subtitleID = 0;

	debug("HadeschEngine::ctor");
}

HadeschEngine::~HadeschEngine() {
	debug("HadeschEngine::dtor");
	DebugMan.clearAllDebugChannels();
	for (unsigned i = 0; i < _winCursors.size(); i++) {
		delete _winCursors[i];
		_winCursors[i] = nullptr;
	}
	for (unsigned i = 0; i < _macCursors.size(); i++) {
		delete _macCursors[i];
		_macCursors[i] = nullptr;
	}

#ifdef USE_TRANSLATION
	delete _transMan;
#endif
}

void HadeschEngine::setVideoRoom(Common::SharedPtr<VideoRoom> room,
				 Common::SharedPtr<Handler> handler,
				 RoomId roomId) {
	assert(!_isInOptions || _isRestoring);
	_sceneVideoRoom = room;
	_sceneHandler = handler;
	_currentTime = 0;
	_sceneStartTime = _system->getMillis();
	if (!_isRestoring) {
		_persistent._previousRoomId = _persistent._currentRoomId;
		_persistent._currentRoomId = roomId;
	}
	_sceneTimers.clear();
}

Common::Point HadeschEngine::getMousePos() {
	return _mousePos;
}

void HadeschEngine::handleEvent(EventHandlerWrapper event) {
	event();
}

void HadeschEngine::newGame() {
	_persistent = Persistent();
	_persistent._quest = kCreteQuest;
	moveToRoom(kWallOfFameRoom);
}

#if defined(USE_ZLIB)

struct WiseFile {
	uint start;
	uint end;
	uint uncompressedLength;
};

// Some variants use wise installer. Wise installer use raw zlib compressed files
// Rather than parsing out the wise structures, we just store offsets for
// the files we care about
static const struct {
	const char *md5;
	uint setupLength;
	WiseFile _wdPod;
	WiseFile _hadeschExe;
} setups[] = {
	{
		// Russian, Windows
		"9ddf1b0b271426b9d023dbf3edbb1caa",
		7491209,
		{0xB8DA2, 0x7246CB, 8691909},
		{0x4109C, 0xB4628, 1007616}
	}	
};

Common::MemoryReadStream *readWiseFile(Common::File &setupFile, const struct WiseFile &wiseFile) {
	// -4 to skip CRC
	byte *compressedBuffer = new byte[wiseFile.end - wiseFile.start - 4];
	byte *uncompressedBuffer = new byte[wiseFile.uncompressedLength];
	setupFile.seek(wiseFile.start);
	setupFile.read(compressedBuffer, wiseFile.end - wiseFile.start - 4);
	if (!Common::inflateZlibHeaderless(uncompressedBuffer, wiseFile.uncompressedLength,
					   compressedBuffer, wiseFile.end - wiseFile.start - 4)) {
		debug("wise inflate failed");
		delete[] compressedBuffer;
		delete[] uncompressedBuffer;
		return nullptr;
	}

	delete[] compressedBuffer;
	return new Common::MemoryReadStream(uncompressedBuffer, wiseFile.uncompressedLength);
}
#endif

Common::ErrorCode HadeschEngine::loadWindowsCursors(Common::PEResources &exe) {
	for (unsigned i = 0; i < ARRAYSIZE(cursorids); i++) {
		Graphics::WinCursorGroup *group = Graphics::WinCursorGroup::createCursorGroup(&exe, cursorids[i]);

		if (!group) {
			debug("Cannot find cursor group %d", cursorids[i]);
			return Common::kUnsupportedGameidError;
		}

		_cursors.push_back(group->cursors[0].cursor);
		_winCursors.push_back(group);
	}

	return Common::kNoError;
}

Common::ErrorCode HadeschEngine::loadCursors() {
	debug("HadeschEngine: loading cursors");

	{
		Common::PEResources exe = Common::PEResources();
		if (exe.loadFromEXE("HADESCH.EXE")) {
			return loadWindowsCursors(exe);
		}
	}

	const char *const macPaths[] = {
		"Hades_-_Copy_To_Hard_Drive/Hades_Challenge/Hades_Challenge_PPC",
		"Hades - Copy To Hard Drive/Hades Challenge/Hades Challenge PPC"
	};

	for (uint j = 0; j < ARRAYSIZE(macPaths); ++j) {
	  	Common::MacResManager resMan = Common::MacResManager();
		if (!resMan.open(macPaths[j])) {
			continue;
		}

		for (unsigned i = 0; i < ARRAYSIZE(cursorids); i++) {
			Common::SeekableReadStream *stream = resMan.getResource(MKTAG('c','r','s','r'), cursorids[i]);
			if (!stream) {
				debug("Couldn't load cursor %d", cursorids[i]);
				return Common::kUnsupportedGameidError;
			}
			Graphics::MacCursor *macCursor = new Graphics::MacCursor();
			macCursor->readFromStream(*stream);
			_cursors.push_back(macCursor);
			delete stream;
			_macCursors.push_back(macCursor);
		}
		return Common::kNoError;
	}

#if defined(USE_ZLIB)
	Common::File setupFile;
	if (setupFile.open("Setup.exe")) {
		uint len = setupFile.size();
		Common::String md5 = Common::computeStreamMD5AsString(setupFile, len);
		for (uint chosenSetup = 0; chosenSetup < ARRAYSIZE(setups); chosenSetup++) {
			if (setups[chosenSetup].setupLength == len && setups[chosenSetup].md5 == md5) {
				Common::MemoryReadStream *wdPod = readWiseFile(setupFile, setups[0]._wdPod);
				if (!wdPod) {
					debug("wd.pod inflate failed");
					return Common::kUnsupportedGameidError;
				}
				_wdPodFile = Common::SharedPtr<PodFile>(new PodFile("WD.POD"));
				_wdPodFile->openStore(Common::SharedPtr<Common::SeekableReadStream>(wdPod));

				Common::MemoryReadStream *hadeschExe = readWiseFile(setupFile, setups[0]._hadeschExe);
				if (!hadeschExe) {
					debug("hadesch.exe inflate failed");
					return Common::kUnsupportedGameidError;
				}

				Common::PEResources exe = Common::PEResources();
				if (exe.loadFromEXE(hadeschExe)) {
					return loadWindowsCursors(exe);
				}
			}
		}
	}
#endif

	debug("Cannot open hadesch.exe");
	return Common::kUnsupportedGameidError;
}

static const char *roomCheats[] = {
	"",
	"in",
	"mo",
	"ht",
	"se",
	"at",
	"mi",
	"me",
	"ar",
	"tr",
	"ca",
	"pr",
	"th",
	"cp",
	"mp",
	"dw",
	"mn",
	"vt",
	"nr",
	"htr",
	"ff",
	"mm",
	"hc",
	"cr",
	"op"
};

static const char *itemCheats[] = {
	"",
	"",
	"straw",
	"stone",
	"bricks",
	"message",
	"key",
	"decree",
	"wood",
	"statue1",
	"statue2",
	"statue3",
	"statue4",
	"statue",
	"coin",
	"potion",
	"shield",
	"sword",
	"bag",
	"helmet",
	"sandals",
	"torch"
};

bool HadeschEngine::handleGenericCheat(const Common::String &cheat) {
	if (cheat == "cheatsoff") {
		_cheatsEnabled = false;
		return true;
	}

	for (int i = kIntroRoom; i < kOptionsRoom; i++)
		if (cheat == roomCheats[i]) {
			moveToRoom((RoomId) i);
			getVideoRoom()->disableMouse();
			return true;
		}

	if (cheat == "commandson") {
		getVideoRoom()->enableMouse();
		return true;
	}

	if (cheat == "commandsoff") {
		getVideoRoom()->disableMouse();
		return true;
	}

	if (cheat == "cretequest") {
		_persistent._quest = kCreteQuest;
		_persistent._roomVisited[kWallOfFameRoom] = true;
		return true;
	}

	if (cheat == "troyquest") {
		_persistent._quest = kTroyQuest;
		_persistent._roomVisited[kWallOfFameRoom] = true;
		_persistent._roomVisited[kMinotaurPuzzle] = true;

		_persistent._powerLevel[0] = MAX(_persistent._powerLevel[0], 1);
		return true;
	}

	if (cheat == "medusaquest") {
		_persistent._quest = kMedusaQuest;
		_persistent._roomVisited[kWallOfFameRoom] = true;
		_persistent._roomVisited[kMinotaurPuzzle] = true;
		_persistent._roomVisited[kTrojanHorsePuzzle] = true;

		for (int i = 0; i < 2; i++)
			_persistent._powerLevel[i] = MAX(_persistent._powerLevel[i], 1);
		return true;
	}

	if (cheat == "rescuephilquest") {
		_persistent._quest = kRescuePhilQuest;
		_persistent._roomVisited[kWallOfFameRoom] = true;
		_persistent._roomVisited[kMedusaPuzzle] = true;
		_persistent._roomVisited[kTrojanHorsePuzzle] = true;
		_persistent._roomVisited[kMinotaurPuzzle] = true;

		for (int i = 0; i < 3; i++)
			_persistent._powerLevel[i] = MAX(_persistent._powerLevel[i], 1);

		return true;
	}

	// TODO: "noquest", "op", "click*", "memory", "showhotspots", "hidehotspots", "showcursor",
	// giveall, takeall, givekeyanddecree, takekeyanddecree

	for (int i = kStraw; i <= kTorch; i++)
		if (cheat == Common::String("give") + itemCheats[i]) {
			_heroBelt->placeToInventory((InventoryItem)i);
			return true;
		}

	for (int i = kStraw; i <= kTorch; i++)
		if (cheat == Common::String("take") + itemCheats[i]) {
			_heroBelt->removeFromInventory((InventoryItem)i);
			return true;
		}

	if (cheat == "hero") {
		_persistent._gender = kMale;
		return true;
	}

	if (cheat == "heroine") {
		_persistent._gender = kFemale;
		return true;
	}

	if (cheat.matchString("powerstrength#")) {
		_persistent._powerLevel[kPowerStrength] = cheat.substr(13).asUint64();
		return true;
	}

	if (cheat.matchString("powerstealth#")) {
		_persistent._powerLevel[kPowerStealth] = cheat.substr(12).asUint64();
		return true;
	}

	if (cheat.matchString("powerwisdom#")) {
		_persistent._powerLevel[kPowerWisdom] = cheat.substr(11).asUint64();
		return true;
	}

	if (cheat.matchString("powerall#")) {
		int val = cheat.substr(8).asUint64();
		_persistent._powerLevel[kPowerWisdom] = val;
		_persistent._powerLevel[kPowerStealth] = val;
		_persistent._powerLevel[kPowerStrength] = val;
		return true;
	}

	return false;
}

void HadeschEngine::resetOptionsRoom() {
	_optionsRoom = Common::SharedPtr<VideoRoom>(new VideoRoom("", "", "OPAssets.txt"));
}

void HadeschEngine::enterOptions() {
	_isInOptions = true;
	_optionsEnterTime = _system->getMillis();
	_sceneVideoRoom->pause();
	resetOptionsRoom();
	_optionsHandler = makeOptionsHandler();
	_optionsHandler->prepareRoom();
}

void HadeschEngine::enterOptionsCredits() {
	if (_isInOptions) {
		_sceneStartTime += _system->getMillis() - _optionsEnterTime;
	}
	_isInOptions = true;
	_optionsEnterTime = _system->getMillis();
	_optionsRoom = Common::SharedPtr<VideoRoom>(new VideoRoom("CREDITS", "CR", ""));
	_optionsHandler = makeCreditsHandler(true);
	_optionsHandler->prepareRoom();
}

void HadeschEngine::exitOptions() {
	_isInOptions = false;
	_sceneStartTime += _system->getMillis() - _optionsEnterTime;
	_optionsHandler.reset();
	_optionsRoom.reset();
	_sceneVideoRoom->unpause();
}

Common::U32String HadeschEngine::translate(const Common::String &str) {
#ifdef USE_TRANSLATION
	return _transMan->getTranslation(str);
#else
	return str.decode();
#endif
}

Common::Error HadeschEngine::run() {
	debug("HadeschEngine::run");

#ifdef USE_TRANSLATION
	_transMan = new Common::TranslationManager("hadesch_translations.dat");
	_transMan->setLanguage(TransMan.getCurrentLanguage());
#endif

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "WIN9x");

	Common::ErrorCode err = loadCursors();
	if (err != Common::kNoError)
		return err;

	if (!_wdPodFile) {
		const char *const wdpodpaths[] = {
			"WIN9x/WORLD/WD.POD", "WD.POD",
			"Hades_-_Copy_To_Hard_Drive/Hades_Challenge/World/wd.pod",
			"Hades - Copy To Hard Drive/Hades Challenge/World/wd.pod"};
		debug("HadeschEngine: loading wd.pod");
		for (uint i = 0; i < ARRAYSIZE(wdpodpaths); ++i) {
			Common::SharedPtr<Common::File> file(new Common::File());
			if (file->open(wdpodpaths[i])) {
				_wdPodFile = Common::SharedPtr<PodFile>(new PodFile("WD.POD"));
				_wdPodFile->openStore(file);
				break;
			}
		}
	}

	if (!_wdPodFile) {
		debug("Cannot find WD.POD");
		return Common::kUnsupportedGameidError;
	}

	_cdScenesPath = "";

	// It's tempting to use SearchMan for this but it
	// doesn't work because we need to access subdirs based
	// on cdScenePath
	const char *const scenepaths[] = {"CDAssets/", "Scenes/"};
	for (uint i = 0; i < ARRAYSIZE(scenepaths); ++i) {
		Common::ScopedPtr<Common::File> file(new Common::File());
		if (file->open(Common::String(scenepaths[i]) + "OLYMPUS/OL.POD")) {
			_cdScenesPath = scenepaths[i];
			break;
		}
	}

	if (_cdScenesPath == "") {
		debug("Cannot find OL.POD");
		return Common::kUnsupportedGameidError;
	}

	debug("HadeschEngine: intro");
	initGraphics(kVideoWidth, kVideoHeight);

	_heroBelt = Common::SharedPtr<HeroBelt>(new HeroBelt());
	_gfxContext = Common::SharedPtr<GfxContext8Bit>(new GfxContext8Bit(2 * kVideoWidth + 10, kVideoHeight + 50));
	_isInOptions = false;

	ConfMan.registerDefault("subtitles", "false");
	ConfMan.registerDefault("sfx_volume", 192);
	ConfMan.registerDefault("music_volume", 192);
	ConfMan.registerDefault("speech_volume", 192);
	ConfMan.registerDefault("mute", "false");
	ConfMan.registerDefault("speech_mute", "false");
	ConfMan.registerDefault("talkspeed", 60);
	_mixer->setVolumeForSoundType(_mixer->kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(_mixer->kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(_mixer->kSpeechSoundType, ConfMan.getInt("speech_volume"));

	if (!ConfMan.getBool("subtitles"))
		_subtitleDelayPerChar = -1;
	else
		_subtitleDelayPerChar = 4500 / ConfMan.getInt("talkspeed");

	debug("HadeschEngine: moving to main loop");
	_nextRoom.clear();
	int loadSlot = ConfMan.getInt("save_slot");
	if (loadSlot >= 0) {
		loadGameState(loadSlot);
	} else {
		_nextRoom.push_back(kIntroRoom);
	}

	while (true) {
		if (_isRestoring) {
			moveToRoomReal(_persistent._currentRoomId);
			_isRestoring = false;
			CursorMan.showMouse(true);
			CursorMan.replaceCursor(_cursors[3]);
		} else if (!_nextRoom.empty()) {
			moveToRoomReal(_nextRoom.remove_at(0));
			CursorMan.showMouse(true);
			CursorMan.replaceCursor(_cursors[3]);
		}

		if (_isQuitting) {
			return Common::kNoError;
		}
		Common::Event event;
		bool stopVideo = false;
		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return Common::kNoError;
			case Common::EVENT_LBUTTONDOWN: {
				if(getVideoRoom()->isMouseEnabled() && getVideoRoom()->isHeroBeltEnabled()
				   && _heroBelt->isPositionOverHeroBelt(event.mouse)) {
					debug("handling belt click");
					_heroBelt->handleClick(event.mouse);
					break;
				}
				const Common::String &q = getVideoRoom()->mapClick(event.mouse);
				debug("handling click on <%s>", q.c_str());
				if (getVideoRoom()->isHeroBeltEnabled() && _heroBelt->isHoldingItem())
					getCurrentHandler()->handleClickWithItem(q, _heroBelt->getHoldingItem());
				else {
					getCurrentHandler()->handleAbsoluteClick(event.mouse);
					getCurrentHandler()->handleClick(q);
				}
			}
				break;
			case Common::EVENT_LBUTTONUP:
				getCurrentHandler()->handleUnclick(getVideoRoom()->mapClick(event.mouse), event.mouse);
				break;
			case Common::EVENT_KEYDOWN:
				// TODO: make equivalents for mobile devices. Keyboard is
				// used for 4 things:
				//
				// * Skipping cutscenes (press space)
				// * Entering name.
				//      Original requires a non-empty name. We allow an
				//      empty name.
				// * Optional save name
				// * Cheats
				if (event.kbd.keycode == Common::KEYCODE_SPACE)
					stopVideo = true;
				if ((event.kbd.ascii >= 'a' && event.kbd.ascii <= 'z')
				    || (event.kbd.ascii >= '0' && event.kbd.ascii <= '9')) {
					_cheat += event.kbd.ascii;
				}
				if (event.kbd.keycode == Common::KEYCODE_RETURN) {
					Common::String cheat = _cheat;
					_cheat = "";
					if (cheat == "qazxcdewsrfvbnhytg") {
						debug("Cheats enabled");
						_cheatsEnabled = true;
						break;
					}
					if (_cheatsEnabled) {
						if (handleGenericCheat(cheat))
							break;
						if (getCurrentHandler()->handleCheat(cheat))
							break;
					}
				}
				getCurrentHandler()->handleKeypress(event.kbd.ascii);
				break;
			default:
				break;
			}
		}		

		if (_isInOptions) {
			_currentTime = _system->getMillis() - _optionsEnterTime;
		} else {
			_currentTime = _system->getMillis() - _sceneStartTime;
			for (Common::List<Timer>::iterator it = _sceneTimers.begin();
			     it != _sceneTimers.end();) {
				if ((it->period_count != 0 && it->next_time < _currentTime)
				    || (it->skippable && stopVideo)) {
					it->next_time = _currentTime + it->period;
					if (it->period_count != -1) {
						it->period_count--;
					}
					handleEvent(it->event);
				}

				if (it->period_count == 0) {
					it = _sceneTimers.erase(it);
				} else
					it++;
			}
		}

		Common::String oldhotzone = getVideoRoom()->getHotZone();
		_mousePos = _eventMan->getMousePos();
		getVideoRoom()->computeHotZone(_currentTime, _mousePos);
		
		Common::String newhotzone = getVideoRoom()->getHotZone();

		if (oldhotzone != newhotzone) {
			getCurrentHandler()->handleMouseOut(oldhotzone);
			getCurrentHandler()->handleMouseOver(newhotzone);
		}

		getCurrentHandler()->frameCallback();

		getVideoRoom()->nextFrame(_gfxContext, _currentTime, stopVideo);

		if (getVideoRoom()->getDragged()) {
			CursorMan.showMouse(true);
			CursorMan.replaceCursor(getVideoRoom()->getDragged());
		} else if (getVideoRoom()->isHeroBeltEnabled()
		    && _heroBelt->isHoldingItem()) {
			const Graphics::Cursor *cursor = _heroBelt->getHoldingItemCursor(
				getVideoRoom()->getCursorAnimationFrame(_currentTime));
			CursorMan.showMouse(true);
			CursorMan.replaceCursor(cursor);
		} else {
			int cursor = getVideoRoom()->getCursor();
			if (cursor >= 0) {
				CursorMan.showMouse(true);
				CursorMan.replaceCursor(_cursors[cursor]);
			} else {
				CursorMan.showMouse(true);
				CursorMan.replaceCursor(_cursors[3]);
			}
		}

		_system->updateScreen();
		_system->delayMillis(15);
	}

	return Common::kNoError;
}

Common::RandomSource &HadeschEngine::getRnd() {
	return _rnd;
}

bool HadeschEngine::hasFeature(EngineFeature f) const {
	return
		f == kSupportsReturnToLauncher ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime ||
		f == kSupportsChangingOptionsDuringRuntime;
}

Common::Error HadeschEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	if (!_persistent.syncGameStream(s))
		return Common::kUnknownError;

	_isRestoring = true;

	return Common::kNoError;
}

Common::Error HadeschEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	if (isAutosave)
		_persistent._slotDescription = "Autosave";
	if(_persistent._currentRoomId == 0)
		return Common::kUnknownError;
	bool res = _persistent.syncGameStream(s);
	_persistent._slotDescription = "";
	return res ? Common::kNoError
		: Common::kUnknownError;
}

const Common::String &HadeschEngine::getCDScenesPath() const {
	return _cdScenesPath;
}

void HadeschEngine::addTimer(EventHandlerWrapper eventId, int32 start_time, int period, int repeat, bool skippable) {
	struct Timer timer;
	assert(!_isInOptions);
	timer.next_time = start_time + period;
	timer.period_count = repeat;
	timer.period = period;
	timer.event = eventId;
	timer.skippable = skippable;
	_sceneTimers.push_back(timer);
}

void HadeschEngine::addTimer(EventHandlerWrapper eventId, int period, int repeat) {
	addTimer(eventId, _currentTime, period, repeat, false);
}

void HadeschEngine::addSkippableTimer(EventHandlerWrapper eventId, int period, int repeat) {
	addTimer(eventId, _currentTime, period, repeat, true);
}

void HadeschEngine::cancelTimer(int eventId) {
	assert(!_isInOptions);
	for (Common::List<Timer>::iterator it = _sceneTimers.begin();
	     it != _sceneTimers.end();) {
		if (it->event == eventId) {
			it = _sceneTimers.erase(it);
		} else
			it++;
	}
}

Common::SharedPtr<Handler> HadeschEngine::getCurrentHandler() {
	return _isInOptions ? _optionsHandler : _sceneHandler;
}
	
Common::SharedPtr<VideoRoom> HadeschEngine::getVideoRoom() {
	return _isInOptions ? _optionsRoom : _sceneVideoRoom;
}

void HadeschEngine::moveToRoomReal(RoomId id) {
	if (_sceneVideoRoom)
		_sceneVideoRoom->finish();

	_heroBelt->reset();

	switch (id) {
	case kWallOfFameRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("WALLFAME", "WF", "HTAssets.txt")),
			     makeWallOfFameHandler(), id);
		break;
	case kArgoRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("ARGO", "AR", "ARAssets.txt")),
			     makeArgoHandler(), id);
		break;
	case kCreteRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("CRETE", "CR", "")),
			     makeCreteHandler(), id);
			     break;
	case kOlympusRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("OLYMPUS", "OL", "MOAssets.txt")),
			     makeOlympusHandler(), id);
		break;
	case kMinosPalaceRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("MINOS", "MI", "MIAssets.txt")),
			     makeMinosHandler(), id);
		break;
	case kDaedalusRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("DAEDALUS", "DA", "DWAssets.txt")),
			     makeDaedalusHandler(), id);
		break;
	case kSeriphosRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("SERIPHOS", "SE", "")),
			     makeSeriphosHandler(), id);
		break;
	case kMedIsleRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("MEDISLE", "MI", "")),
			     makeMedIsleHandler(), id);
		break;
	case kMedusaPuzzle:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("MEDUSA", "ME", "")),
			     makeMedusaHandler(), id);
		break;
	case kTroyRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("TROY", "TR", "")),
			     makeTroyHandler(), id);
		break;
	case kTrojanHorsePuzzle:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("TROJAN", "TH", "")),
			     makeTrojanHandler(), id);
		break;
	case kMinotaurPuzzle:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("MINOTAUR", "MM", "")),
			     makeMinotaurHandler(), id);
			     break;
	case kQuiz:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("HADESCH", "HC", "HcAssets.txt")),
			     makeQuizHandler(), id);
		break;
	case kCatacombsRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("CATACOMB", "CA", "CaAssets.txt")),
			     makeCatacombsHandler(), id);
		break;
	case kPriamRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("PRIAM", "PR", "PrAssets.txt")),
			     makePriamHandler(), id);
		break;
	case kAthenaRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("ATHENA", "AT", "")),
			     makeAthenaHandler(), id);
		break;
	case kVolcanoRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("VOLCANO", "VO", "VTAssets.txt")),
			     makeVolcanoHandler(), id);
		break;
	case kRiverStyxRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("NEARR", "NR", "NRAssets.txt")),
			     makeRiverStyxHandler(), id);
		break;
	case kHadesThroneRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("THRONE", "TH", "HTRAsset.txt")),
			     makeHadesThroneHandler(), id);
		break;
	case kCreditsRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("CREDITS", "CR", "")),
			     makeCreditsHandler(false), id);
		break;
	case kIntroRoom:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("INTRO", "IN", "")),
			     makeIntroHandler(), id);
		break;
	case kFerrymanPuzzle:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("FERRY", "FF", "")),
			     makeFerryHandler(), id);
		break;
	case kMonsterPuzzle:
		setVideoRoom(Common::SharedPtr<VideoRoom>(new VideoRoom("MONSTER", "MM", "")),
			     makeMonsterHandler(), id);
		break;
	default:
		debug("unknown room %d", id);
		assert(0);
		return;
	}

	_sceneHandler->prepareRoom();

	_persistent._roomVisited[id] = true;
}

int HadeschEngine::genSubtitleID() {
	return _subtitleID++;
}

int HadeschEngine::firstAvailableSlot() {
	for (unsigned slot = 3; ; slot++) {
		SaveStateDescriptor desc = getMetaEngine().querySaveMetaInfos(_targetName.c_str(), slot);
		if (desc.getSaveSlot() == -1 && !desc.getWriteProtectedFlag())
			return slot;
	}
}

void HadeschEngine::quit() {
	_isQuitting = true;
}

bool HadeschEngine::hasAnySaves() {
	Common::SaveFileManager *saveFileMan = getSaveFileManager();
	Common::StringArray filenames;
	Common::String pattern(getMetaEngine().getSavegameFilePattern(_targetName.c_str()));

	filenames = saveFileMan->listSavefiles(pattern);

	return !filenames.empty();
}

Common::Array<HadeschSaveDescriptor> HadeschEngine::getHadeschSavesList() {
	Common::SaveFileManager *saveFileMan = getSaveFileManager();
	Common::StringArray filenames;
	Common::String pattern(getMetaEngine().getSavegameFilePattern(_targetName.c_str()));

	filenames = saveFileMan->listSavefiles(pattern);

	Common::Array<HadeschSaveDescriptor> saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0) {
			Common::ScopedPtr<Common::InSaveFile> in(saveFileMan->openForLoading(*file));
			if (!in) {
				continue;
			}

			Common::Serializer s(in.get(), nullptr);

			saveList.push_back(HadeschSaveDescriptor(s, slotNum));
		}
	}

	// Sort saves based on slot number.
	// TODO: change it to chronological save id
	Common::sort(saveList.begin(), saveList.end(), HadeschSaveDescriptorSlotComparator());
	return saveList;
}

void HadeschEngine::deleteSave(int slot) {
	getMetaEngine().removeSaveState(_targetName.c_str(), slot);
}

void EventHandlerWrapper::operator()() const {
  	if (_handler && _eventId == -1)
		debug("handling anon event");
	else if (_eventId != 20001 && _eventId != 14006 && _eventId != 15266)
		debug("handling event %d", _eventId);
	if (_handler)
		_handler->operator()();
	if (_eventId > 0)
		g_vm->getCurrentHandler()->handleEvent(_eventId);
}

bool EventHandlerWrapper::operator==(int b) const {
	return _eventId == b;
}

uint32 HadeschEngine::getSubtitleDelayPerChar() const {
	return _subtitleDelayPerChar;
}

} // End of namespace Hadesch
