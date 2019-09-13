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


#include "gui/EventRecorder.h"

#ifdef ENABLE_EVENTRECORDER

namespace Common {
DECLARE_SINGLETON(GUI::EventRecorder);
}

#include "common/debug-channels.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "common/config-manager.h"
#include "common/md5.h"
#include "gui/gui-manager.h"
#include "gui/widget.h"
#include "gui/onscreendialog.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"
#include "graphics/scaler.h"

namespace GUI {


const int kMaxRecordsNames = 0x64;
const int kDefaultScreenshotPeriod = 60000;

uint32 readTime(Common::ReadStream *inFile) {
	uint32 d = inFile->readByte();
	if (d == 0xff) {
		d = inFile->readUint32LE();
	}

	return d;
}

void writeTime(Common::WriteStream *outFile, uint32 d) {
		//Simple RLE compression
	if (d >= 0xff) {
		outFile->writeByte(0xff);
		outFile->writeUint32LE(d);
	} else {
		outFile->writeByte(d);
	}
}

EventRecorder::EventRecorder() {
	_timerManager = NULL;
	_recordMode = kPassthrough;
	_fakeMixerManager = NULL;
	_initialized = false;
	_needRedraw = false;
	_fastPlayback = false;

	_fakeTimer = 0;
	_savedState = false;
	_needcontinueGame = false;
	_temporarySlot = 0;
	_realSaveManager = 0;
	_realMixerManager = 0;
	_controlPanel = 0;
	_lastMillis = 0;
	_lastScreenshotTime = 0;
	_screenshotPeriod = 0;
	_playbackFile = 0;

	DebugMan.addDebugChannel(kDebugLevelEventRec, "EventRec", "Event recorder debug level");
}

EventRecorder::~EventRecorder() {
	if (_timerManager != NULL) {
		delete _timerManager;
	}
}

void EventRecorder::deinit() {
	if (!_initialized) {
		return;
	}
	setFileHeader();
	_needRedraw = false;
	_initialized = false;
	_recordMode = kPassthrough;
	delete _fakeMixerManager;
	_fakeMixerManager = NULL;
	_controlPanel->close();
	delete _controlPanel;
	debugC(1, kDebugLevelEventRec, "playback:action=stopplayback");
	g_system->getEventManager()->getEventDispatcher()->unregisterSource(this);
	_recordMode = kPassthrough;
	_playbackFile->close();
	delete _playbackFile;
	switchMixer();
	switchTimerManagers();
	DebugMan.disableDebugChannel("EventRec");
}

void EventRecorder::processMillis(uint32 &millis, bool skipRecord) {
	if (!_initialized) {
		return;
	}
	if (skipRecord) {
		millis = _fakeTimer;
		return;
	}
	if (_recordMode == kRecorderPlaybackPause) {
		millis = _fakeTimer;
	}
	uint32 millisDelay;
	Common::RecorderEvent timerEvent;
	switch (_recordMode) {
	case kRecorderRecord:
		updateSubsystems();
		millisDelay = millis - _lastMillis;
		_lastMillis = millis;
		_fakeTimer += millisDelay;
		_controlPanel->setReplayedTime(_fakeTimer);
		timerEvent.recordedtype = Common::kRecorderEventTypeTimer;
		timerEvent.time = _fakeTimer;
		_playbackFile->writeEvent(timerEvent);
		takeScreenshot();
		_timerManager->handler();
		break;
	case kRecorderPlayback:
		updateSubsystems();
		if (_nextEvent.recordedtype == Common::kRecorderEventTypeTimer) {
			_fakeTimer = _nextEvent.time;
			_nextEvent = _playbackFile->getNextEvent();
			_timerManager->handler();
		} else {
			if (_nextEvent.type == Common::EVENT_RTL) {
				error("playback:action=stopplayback");
			} else {
				uint32 seconds = _fakeTimer / 1000;
				Common::String screenTime = Common::String::format("%.2d:%.2d:%.2d", seconds / 3600 % 24, seconds / 60 % 60, seconds % 60);
				error("playback:action=error reason=\"synchronization error\" time = %s", screenTime.c_str());
			}
		}
		millis = _fakeTimer;
		_controlPanel->setReplayedTime(_fakeTimer);
		break;
	case kRecorderPlaybackPause:
		millis = _fakeTimer;
		break;
	default:
		break;
	}
}

bool EventRecorder::processDelayMillis() {
	return _fastPlayback;
}

void EventRecorder::checkForKeyCode(const Common::Event &event) {
	if ((event.type == Common::EVENT_KEYDOWN) && (event.kbd.flags & Common::KBD_CTRL) && (event.kbd.keycode == Common::KEYCODE_p) && (!event.kbdRepeat)) {
		togglePause();
	}
}

bool EventRecorder::pollEvent(Common::Event &ev) {
	if ((_recordMode != kRecorderPlayback) || !_initialized)
		return false;

	if ((_nextEvent.recordedtype == Common::kRecorderEventTypeTimer) || (_nextEvent.type ==  Common::EVENT_INVALID)) {
		return false;
	}

	switch (_nextEvent.type) {
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		g_system->warpMouse(_nextEvent.mouse.x, _nextEvent.mouse.y);
		break;
	default:
		break;
	}
	ev = _nextEvent;
	_nextEvent = _playbackFile->getNextEvent();
	return true;
}

void EventRecorder::switchFastMode() {
	if (_recordMode == kRecorderPlaybackPause) {
		_fastPlayback = !_fastPlayback;
	}
}

void EventRecorder::togglePause() {
	RecordMode oldState;
	switch (_recordMode) {
	case kRecorderPlayback:
	case kRecorderRecord:
		oldState = _recordMode;
		_recordMode = kRecorderPlaybackPause;
		_controlPanel->runModal();
		_recordMode = oldState;
		_initialized = true;
		break;
	case kRecorderPlaybackPause:
		_controlPanel->close();
		break;
	default:
		break;
	}
}

void EventRecorder::RegisterEventSource() {
	g_system->getEventManager()->getEventDispatcher()->registerMapper(this, false);
}

uint32 EventRecorder::getRandomSeed(const Common::String &name) {
	uint32 result = g_system->getMillis();
	if (_recordMode == kRecorderRecord) {
		_playbackFile->getHeader().randomSourceRecords[name] = result;
	} else if (_recordMode == kRecorderPlayback) {
		result = _playbackFile->getHeader().randomSourceRecords[name];
	}
	return result;
}

Common::String EventRecorder::generateRecordFileName(const Common::String &target) {
	Common::String pattern(target+".r??");
	Common::StringArray files = g_system->getSavefileManager()->listSavefiles(pattern);
	for (int i = 0; i < kMaxRecordsNames; ++i) {
		Common::String recordName = Common::String::format("%s.r%02d", target.c_str(), i);
		if (find(files.begin(), files.end(), recordName) != files.end()) {
			continue;
		}
		return recordName;
	}
	return "";
}


void EventRecorder::init(Common::String recordFileName, RecordMode mode) {
	_fakeMixerManager = new NullSdlMixerManager();
	_fakeMixerManager->init();
	_fakeMixerManager->suspendAudio();
	_fakeTimer = 0;
	_lastMillis = g_system->getMillis();
	_playbackFile = new Common::PlaybackFile();
	_lastScreenshotTime = 0;
	_recordMode = mode;
	_needcontinueGame = false;
	if (ConfMan.hasKey("disable_display")) {
		DebugMan.enableDebugChannel("EventRec");
		gDebugLevel = 1;
	}
	if (_recordMode == kRecorderPlayback) {
		debugC(1, kDebugLevelEventRec, "playback:action=\"Load file\" filename=%s", recordFileName.c_str());
	}
	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
	_screenshotPeriod = ConfMan.getInt("screenshot_period");
	if (_screenshotPeriod == 0) {
		_screenshotPeriod = kDefaultScreenshotPeriod;
	}
	if (!openRecordFile(recordFileName)) {
		deinit();
		error("playback:action=error reason=\"Record file loading error\"");
		return;
	}
	if (_recordMode != kPassthrough) {
		_controlPanel = new GUI::OnScreenDialog(_recordMode == kRecorderRecord);
	}
	if (_recordMode == kRecorderPlayback) {
		applyPlaybackSettings();
		_nextEvent = _playbackFile->getNextEvent();
	}
	if (_recordMode == kRecorderRecord) {
		getConfig();
	}

	switchMixer();
	switchTimerManagers();
	_needRedraw = true;
	_initialized = true;
}


/**
 * Opens or creates file depend of recording mode.
 *
 *@param id of recording or playing back game
 *@return true in case of success, false in case of error
 *
 */
bool EventRecorder::openRecordFile(const Common::String &fileName) {
	bool result;
	switch (_recordMode) {
	case kRecorderRecord:
		return _playbackFile->openWrite(fileName);
	case kRecorderPlayback:
		_recordMode = kPassthrough;
		result = _playbackFile->openRead(fileName);
		_recordMode = kRecorderPlayback;
		return result;
	default:
		return false;
	}
	return true;
}

bool EventRecorder::checkGameHash(const ADGameDescription *gameDesc) {
	if (_playbackFile->getHeader().hashRecords.size() == 0) {
		warning("Engine doesn't contain description table");
		return false;
	}
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (_playbackFile->getHeader().hashRecords.find(fileDesc->fileName) == _playbackFile->getHeader().hashRecords.end()) {
			warning("MD5 hash for file %s not found in record file", fileDesc->fileName);
			debugC(1, kDebugLevelEventRec, "playback:action=\"Check game hash\" filename=%s filehash=%s storedhash=\"\" result=different", fileDesc->fileName, fileDesc->md5);
			return false;
		}
		if (_playbackFile->getHeader().hashRecords[fileDesc->fileName] != fileDesc->md5) {
			warning("Incorrect version of game file %s. Stored MD5 is %s. MD5 of loaded game is %s", fileDesc->fileName, _playbackFile->getHeader().hashRecords[fileDesc->fileName].c_str(), fileDesc->md5);
			debugC(1, kDebugLevelEventRec, "playback:action=\"Check game hash\" filename=%s filehash=%s storedhash=%s result=different", fileDesc->fileName, fileDesc->md5, _playbackFile->getHeader().hashRecords[fileDesc->fileName].c_str());
			return false;
		}
		debugC(1, kDebugLevelEventRec, "playback:action=\"Check game hash\" filename=%s filehash=%s storedhash=%s result=equal", fileDesc->fileName, fileDesc->md5, _playbackFile->getHeader().hashRecords[fileDesc->fileName].c_str());
	}
	return true;
}

void EventRecorder::registerMixerManager(SdlMixerManager *mixerManager) {
	_realMixerManager = mixerManager;
}

void EventRecorder::switchMixer() {
	if (_recordMode == kPassthrough) {
		_realMixerManager->resumeAudio();
	} else {
		_realMixerManager->suspendAudio();
		_fakeMixerManager->resumeAudio();
	}
}

SdlMixerManager *EventRecorder::getMixerManager() {
	if (_recordMode == kPassthrough) {
		return _realMixerManager;
	} else {
		return _fakeMixerManager;
	}
}

void EventRecorder::getConfigFromDomain(const Common::ConfigManager::Domain *domain) {
	for (Common::ConfigManager::Domain::const_iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		_playbackFile->getHeader().settingsRecords[entry->_key] = entry->_value;
	}
}

void EventRecorder::getConfig() {
	getConfigFromDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	getConfigFromDomain(ConfMan.getActiveDomain());
	_playbackFile->getHeader().settingsRecords["save_slot"] = ConfMan.get("save_slot");
}


void EventRecorder::applyPlaybackSettings() {
	for (Common::StringMap::const_iterator i = _playbackFile->getHeader().settingsRecords.begin(); i != _playbackFile->getHeader().settingsRecords.end(); ++i) {
		Common::String currentValue = ConfMan.get(i->_key);
		if (currentValue != i->_value) {
			ConfMan.set(i->_key, i->_value, ConfMan.kTransientDomain);
			debugC(1, kDebugLevelEventRec, "playback:action=\"Apply settings\" key=%s storedvalue=%s currentvalue=%s result=different", i->_key.c_str(), i->_value.c_str(), currentValue.c_str());
		} else {
			debugC(1, kDebugLevelEventRec, "playback:action=\"Apply settings\" key=%s storedvalue=%s currentvalue=%s result=equal", i->_key.c_str(), i->_value.c_str(), currentValue.c_str());
		}
	}
	removeDifferentEntriesInDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	removeDifferentEntriesInDomain(ConfMan.getActiveDomain());
}

void EventRecorder::removeDifferentEntriesInDomain(Common::ConfigManager::Domain *domain) {
	for (Common::ConfigManager::Domain::const_iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		if (_playbackFile->getHeader().settingsRecords.find(entry->_key) == _playbackFile->getHeader().settingsRecords.end()) {
			debugC(1, kDebugLevelEventRec, "playback:action=\"Apply settings\" checksettings:key=%s storedvalue=%s currentvalue="" result=different", entry->_key.c_str(), entry->_value.c_str());
			domain->erase(entry->_key);
		}
	}
}

DefaultTimerManager *EventRecorder::getTimerManager() {
	return _timerManager;
}

void EventRecorder::registerTimerManager(DefaultTimerManager *timerManager) {
	_timerManager = timerManager;
}

void EventRecorder::switchTimerManagers() {
	delete _timerManager;
	if (_recordMode == kPassthrough) {
		_timerManager = new SdlTimerManager();
	} else {
		_timerManager = new DefaultTimerManager();
	}
}

void EventRecorder::updateSubsystems() {
	if (_recordMode == kPassthrough) {
		return;
	}
	RecordMode oldRecordMode = _recordMode;
	_recordMode = kPassthrough;
	_fakeMixerManager->update();
	_recordMode = oldRecordMode;
}

Common::List<Common::Event> EventRecorder::mapEvent(const Common::Event &ev, Common::EventSource *source) {
	if ((!_initialized) && (_recordMode != kRecorderPlaybackPause)) {
		return DefaultEventMapper::mapEvent(ev, source);
	}

	checkForKeyCode(ev);
	Common::Event evt = ev;
	evt.mouse.x = evt.mouse.x * (g_system->getOverlayWidth() / g_system->getWidth());
	evt.mouse.y = evt.mouse.y * (g_system->getOverlayHeight() / g_system->getHeight());
	switch (_recordMode) {
	case kRecorderPlayback:
		if (ev.kbdRepeat != true) {
			return Common::List<Common::Event>();
		}
		return Common::DefaultEventMapper::mapEvent(ev, source);
		break;
	case kRecorderRecord:
		g_gui.processEvent(evt, _controlPanel);
		if (((evt.type == Common::EVENT_LBUTTONDOWN) || (evt.type == Common::EVENT_LBUTTONUP) || (evt.type == Common::EVENT_MOUSEMOVE)) && _controlPanel->isMouseOver()) {
			return Common::List<Common::Event>();
		} else {
			Common::RecorderEvent e(ev);
			e.recordedtype = Common::kRecorderEventTypeNormal;
			e.time = _fakeTimer;
			_playbackFile->writeEvent(e);
			return DefaultEventMapper::mapEvent(ev, source);
		}
		break;
	case kRecorderPlaybackPause: {
		Common::Event dialogEvent;
		if (_controlPanel->isEditDlgVisible()) {
			dialogEvent = ev;
		} else {
			dialogEvent = evt;
		}
		g_gui.processEvent(dialogEvent, _controlPanel->getActiveDlg());
		if (((dialogEvent.type == Common::EVENT_LBUTTONDOWN) || (dialogEvent.type == Common::EVENT_LBUTTONUP) || (dialogEvent.type == Common::EVENT_MOUSEMOVE)) && _controlPanel->isMouseOver()) {
			return Common::List<Common::Event>();
		}
		return Common::DefaultEventMapper::mapEvent(dialogEvent, source);
	}
		break;
	default:
		return Common::DefaultEventMapper::mapEvent(ev, source);
	}

	return Common::DefaultEventMapper::mapEvent(ev, source);
}

void EventRecorder::setGameMd5(const ADGameDescription *gameDesc) {
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (fileDesc->md5 != NULL) {
			_playbackFile->getHeader().hashRecords[fileDesc->fileName] = fileDesc->md5;
		}
	}
}

void EventRecorder::processGameDescription(const ADGameDescription *desc) {
	if (_recordMode == kRecorderRecord) {
		setGameMd5(desc);
	}
	if ((_recordMode == kRecorderPlayback) && !checkGameHash(desc)) {
		deinit();
		error("playback:action=error reason=\"\"");
	}
}

void EventRecorder::deleteRecord(const Common::String& fileName) {
	g_system->getSavefileManager()->removeSavefile(fileName);
}

void EventRecorder::takeScreenshot() {
	if ((_fakeTimer - _lastScreenshotTime) > _screenshotPeriod) {
		Graphics::Surface screen;
		uint8 md5[16];
		if (grabScreenAndComputeMD5(screen, md5)) {
			_lastScreenshotTime = _fakeTimer;
			_playbackFile->saveScreenShot(screen, md5);
			screen.free();
		}
	}
}

bool EventRecorder::grabScreenAndComputeMD5(Graphics::Surface &screen, uint8 md5[16]) {
	if (!createScreenShot(screen)) {
		warning("Can't save screenshot");
		return false;
	}
	Common::MemoryReadStream bitmapStream((const byte*)screen.getPixels(), screen.w * screen.h * screen.format.bytesPerPixel);
	computeStreamMD5(bitmapStream, md5);
	return true;
}

Common::SeekableReadStream *EventRecorder::processSaveStream(const Common::String &fileName) {
	Common::InSaveFile *saveFile;
	switch (_recordMode) {
	case kRecorderPlayback:
		debugC(1, kDebugLevelEventRec, "playback:action=\"Process save file\" filename=%s len=%d", fileName.c_str(), _playbackFile->getHeader().saveFiles[fileName].size);
		return new Common::MemoryReadStream(_playbackFile->getHeader().saveFiles[fileName].buffer, _playbackFile->getHeader().saveFiles[fileName].size);
	case kRecorderRecord:
		saveFile = _realSaveManager->openForLoading(fileName);
		if (saveFile != NULL) {
			_playbackFile->addSaveFile(fileName, saveFile);
			saveFile->seek(0);
		}
		return saveFile;
	default:
		return NULL;
		break;
	}
}

Common::SaveFileManager *EventRecorder::getSaveManager(Common::SaveFileManager *realSaveManager) {
	_realSaveManager = realSaveManager;
	if (_recordMode != kPassthrough) {
		return &_fakeSaveManager;
	} else {
		return realSaveManager;
	}
}

void EventRecorder::preDrawOverlayGui() {
	if ((_initialized) || (_needRedraw)) {
		RecordMode oldMode = _recordMode;
		_recordMode = kPassthrough;
		g_system->showOverlay();
		g_gui.theme()->clearAll();
		g_gui.theme()->drawToBackbuffer();
		_controlPanel->drawDialog(kDrawLayerBackground);
		g_gui.theme()->drawToScreen();
		g_gui.theme()->copyBackBufferToScreen();
		_controlPanel->drawDialog(kDrawLayerForeground);
		g_gui.theme()->updateScreen();
		_recordMode = oldMode;
	}
}

void EventRecorder::postDrawOverlayGui() {
    if ((_initialized) || (_needRedraw)) {
		RecordMode oldMode = _recordMode;
		_recordMode = kPassthrough;
	    g_system->hideOverlay();
		_recordMode = oldMode;
	}
}

Common::StringArray EventRecorder::listSaveFiles(const Common::String &pattern) {
	if (_recordMode == kRecorderPlayback) {
		Common::StringArray result;
		for (Common::HashMap<Common::String, Common::PlaybackFile::SaveFileBuffer>::iterator  i = _playbackFile->getHeader().saveFiles.begin(); i != _playbackFile->getHeader().saveFiles.end(); ++i) {
			if (i->_key.matchString(pattern, false, true)) {
				result.push_back(i->_key);
			}
		}
		return result;
	} else {
		return _realSaveManager->listSavefiles(pattern);
	}
}

void EventRecorder::setFileHeader() {
	if (_recordMode != kRecorderRecord) {
		return;
	}
	TimeDate t;
	PlainGameDescriptor desc = EngineMan.findGame(ConfMan.getActiveDomainName());
	g_system->getTimeAndDate(t);
	if (_author.empty()) {
		setAuthor("Unknown Author");
	}
	if (_name.empty()) {
		g_eventRec.setName(Common::String::format("%.2d.%.2d.%.4d ", t.tm_mday, t.tm_mon, 1900 + t.tm_year) + desc.description);
	}
	_playbackFile->getHeader().author = _author;
	_playbackFile->getHeader().notes = _desc;
	_playbackFile->getHeader().name = _name;
}

SDL_Surface *EventRecorder::getSurface(int width, int height) {
	// Create a RGB565 surface of the requested dimensions.
	return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
}

bool EventRecorder::switchMode() {
	const Common::String gameId = ConfMan.get("gameid");
	const Plugin *plugin = nullptr;
	EngineMan.findGame(gameId, &plugin);
	bool metaInfoSupport = plugin->get<MetaEngine>().hasFeature(MetaEngine::kSavesSupportMetaInfo);
	bool featuresSupport = metaInfoSupport &&
						  g_engine->canSaveGameStateCurrently() &&
						  plugin->get<MetaEngine>().hasFeature(MetaEngine::kSupportsListSaves) &&
						  plugin->get<MetaEngine>().hasFeature(MetaEngine::kSupportsDeleteSave);
	if (!featuresSupport) {
		return false;
	}

	int emptySlot = 1;
	SaveStateList saveList = plugin->get<MetaEngine>().listSaves(gameId.c_str());
	for (SaveStateList::const_iterator x = saveList.begin(); x != saveList.end(); ++x) {
		int saveSlot = x->getSaveSlot();
		if (saveSlot == 0) {
			continue;
		}
		if (emptySlot != saveSlot) {
			break;
		}
		emptySlot++;
	}
	Common::String saveName;
	if (emptySlot >= 0) {
		saveName = Common::String::format("Save %d", emptySlot + 1);
		Common::Error status = g_engine->saveGameState(emptySlot, saveName);
		if (status.getCode() == Common::kNoError) {
			Common::Event eventRTL;
			eventRTL.type = Common::EVENT_RTL;
			g_system->getEventManager()->pushEvent(eventRTL);
		}
	}
	ConfMan.set("record_mode", "", Common::ConfigManager::kTransientDomain);
	ConfMan.setInt("save_slot", emptySlot, Common::ConfigManager::kTransientDomain);
	_needcontinueGame = true;
	return true;
}

bool EventRecorder::checkForContinueGame() {
	bool result = _needcontinueGame;
	_needcontinueGame = false;
	return result;
}

void EventRecorder::deleteTemporarySave() {
	if (_temporarySlot == -1) return;
	const Common::String gameId = ConfMan.get("gameid");
	const Plugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);
	 plugin->get<MetaEngine>().removeSaveState(gameId.c_str(), _temporarySlot);
	_temporarySlot = -1;
}

} // End of namespace GUI

#endif // ENABLE_EVENTRECORDER
