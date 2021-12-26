/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "gui/EventRecorder.h"

#ifdef ENABLE_EVENTRECORDER

namespace Common {
DECLARE_SINGLETON(GUI::EventRecorder);
}

#include "common/debug-channels.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/mixer/mixer.h"
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

EventRecorder::EventRecorder() {
	_timerManager = nullptr;
	_recordMode = kPassthrough;
	_fakeMixerManager = nullptr;
	_initialized = false;
	_needRedraw = false;
	_processingMillis = false;
	_fastPlayback = false;
	_lastTimeDate.tm_sec = 0;
	_lastTimeDate.tm_min = 0;
	_lastTimeDate.tm_hour = 0;
	_lastTimeDate.tm_mday = 0;
	_lastTimeDate.tm_mon = 0;
	_lastTimeDate.tm_year = 0;
	_lastTimeDate.tm_wday = 0;

	_fakeTimer = 0;
	_savedState = false;
	_acquireCount = 0;
	_needcontinueGame = false;
	_temporarySlot = 0;
	_realSaveManager = nullptr;
	_realMixerManager = nullptr;
	_controlPanel = nullptr;
	_lastMillis = 0;
	_lastScreenshotTime = 0;
	_screenshotPeriod = 0;
	_playbackFile = nullptr;
	_recordFile = nullptr;
}

EventRecorder::~EventRecorder() {
	delete _timerManager;
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
	_fakeMixerManager = nullptr;
	_controlPanel->close();
	delete _controlPanel;
	debugC(1, kDebugLevelEventRec, "playback:action=stopplayback");
	Common::EventDispatcher *eventDispatcher = g_system->getEventManager()->getEventDispatcher();
	eventDispatcher->unregisterSource(this);
	eventDispatcher->ignoreSources(false);
	_recordMode = kPassthrough;
	if (_playbackFile) {
		_playbackFile->close();
		delete _playbackFile;
	}
	if (_recordFile) {
		_recordFile->close();
		delete _recordFile;
	}
	switchMixer();
	switchTimerManagers();
	DebugMan.disableDebugChannel("EventRec");
}

void EventRecorder::updateFakeTimer(uint32 millis) {
	uint32 millisDelay = millis - _lastMillis;
	_lastMillis = millis;
	_fakeTimer += millisDelay;
	_controlPanel->setReplayedTime(_fakeTimer);
}

void EventRecorder::processTimeAndDate(TimeDate &td, bool skipRecord) {
	if (!_initialized) {
		return;
	}
	if (skipRecord) {
		td = _lastTimeDate;
		return;
	}
	Common::RecorderEvent timeDateEvent;
	if (_recordMode == kRecorderRecord) {
		timeDateEvent.recordedtype = Common::kRecorderEventTypeTimeDate;
		timeDateEvent.timeDate = td;
		_lastTimeDate = td;
		_recordFile->writeEvent(timeDateEvent);
	}
	if ((_recordMode == kRecorderPlayback) || (_recordMode == kRecorderUpdate)) {
		if (_nextEvent.recordedtype != Common::kRecorderEventTypeTimeDate) {
			// just re-use any previous date time value
			td = _lastTimeDate;
			if (_recordMode == kRecorderUpdate) {
				// if we make it to here, we're expecting there to be an event in the file
				// so add one to the updated recording
				debug(3, "inserting additional timedate event");
				timeDateEvent.recordedtype = Common::kRecorderEventTypeTimeDate;
				timeDateEvent.timeDate = td;
				_recordFile->writeEvent(timeDateEvent);
			}
			return;
		}
		_lastTimeDate = _nextEvent.timeDate;
		td = _lastTimeDate;
		debug(3, "timedate event");

		if (_recordMode == kRecorderUpdate) {
			// copy the event to the updated recording
			timeDateEvent.recordedtype = Common::kRecorderEventTypeTimeDate;
			timeDateEvent.timeDate = td;
			_recordFile->writeEvent(timeDateEvent);
		}

		_nextEvent = _playbackFile->getNextEvent();
	}
	if (_recordMode == kRecorderPlaybackPause)
		td = _lastTimeDate;
}

void EventRecorder::processMillis(uint32 &millis, bool skipRecord) {
	if (!_initialized) {
		return;
	}
	if (skipRecord || _processingMillis) {
		millis = _fakeTimer;
		return;
	}
	// to prevent calling this recursively
	if (_recordMode == kRecorderPlaybackPause) {
		millis = _fakeTimer;
	}
	Common::RecorderEvent timerEvent;
	switch (_recordMode) {
	case kRecorderRecord:
		updateSubsystems();
		updateFakeTimer(millis);
		timerEvent.recordedtype = Common::kRecorderEventTypeTimer;
		timerEvent.time = _fakeTimer;
		_recordFile->writeEvent(timerEvent);
		_timerManager->handler();
		break;
	case kRecorderUpdate: // fallthrough
	case kRecorderPlayback:
		if (_nextEvent.recordedtype != Common::kRecorderEventTypeTimer) {
			// just re-use any previous millis value
			// this might happen if you have EventSource instances registered, that
			// are querying the millis by themselves, too. If the EventRecorder::poll
			// is registered and thus dispatched after those EventSource instances, it
			// might look like it ran out-of-sync.
			millis = _fakeTimer;
			if (_recordMode == kRecorderUpdate) {
				// if we make it to here, we're expecting there to be an event in the file
				// so add one to the updated recording
				debug(3, "inserting additional timer event");
				timerEvent.recordedtype = Common::kRecorderEventTypeTimer;
				timerEvent.time = _fakeTimer;
				_recordFile->writeEvent(timerEvent);
			}
			return;
		}
		_processingMillis = true;
		_fakeTimer = _nextEvent.time;
		millis = _fakeTimer;
		if (_recordMode == kRecorderUpdate) {
			// copy the event to the updated recording
			timerEvent.recordedtype = Common::kRecorderEventTypeTimer;
			timerEvent.time = _fakeTimer;
			_recordFile->writeEvent(timerEvent);
		}
		updateSubsystems();
		_nextEvent = _playbackFile->getNextEvent();
		_timerManager->handler();
		_controlPanel->setReplayedTime(_fakeTimer);
		_processingMillis = false;
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

bool EventRecorder::processAutosave() {
	return _recordMode == kPassthrough;
}

void EventRecorder::processScreenUpdate() {
	if (!_initialized) {
		return;
	}

	Common::RecorderEvent screenUpdateEvent;
	switch (_recordMode) {
	case kRecorderRecord:
		updateSubsystems();
		screenUpdateEvent.recordedtype = Common::kRecorderEventTypeScreenUpdate;
		screenUpdateEvent.time = _fakeTimer;
		_recordFile->writeEvent(screenUpdateEvent);
		takeScreenshot();
		_timerManager->handler();
		break;
	case kRecorderUpdate: // fallthrough
	case kRecorderPlayback:
		// if the next event isn't a screen update, fast forward until we find one.
		if (_nextEvent.recordedtype != Common::kRecorderEventTypeScreenUpdate) {
			int numSkipped = 0;
			while (true) {
				_nextEvent = _playbackFile->getNextEvent();
				numSkipped += 1;
				if (_nextEvent.recordedtype == Common::kRecorderEventTypeScreenUpdate) {
					warning("Skipped %d events to get to the next screen update at %d", numSkipped, _nextEvent.time);
					break;
				}
			}
		}
		_processingMillis = true;
		_fakeTimer = _nextEvent.time;
		updateSubsystems();
		_nextEvent = _playbackFile->getNextEvent();
		if (_recordMode == kRecorderUpdate) {
			// write event to the updated file and update screenshot if necessary
			screenUpdateEvent.recordedtype = Common::kRecorderEventTypeScreenUpdate;
			screenUpdateEvent.time = _fakeTimer;
			_recordFile->writeEvent(screenUpdateEvent);
			takeScreenshot();
		}
		_timerManager->handler();
		_controlPanel->setReplayedTime(_fakeTimer);
		_processingMillis = false;
		break;
	default:
		break;
	}
}

void EventRecorder::checkForKeyCode(const Common::Event &event) {
	if ((event.type == Common::EVENT_KEYDOWN) && (event.kbd.flags & Common::KBD_CTRL) && (event.kbd.keycode == Common::KEYCODE_p) && (!event.kbdRepeat)) {
		togglePause();
	}
}

bool EventRecorder::pollEvent(Common::Event &ev) {
	if (((_recordMode != kRecorderPlayback) &&
		(_recordMode != kRecorderUpdate)) ||
		!_initialized)
		return false;

	if (_nextEvent.recordedtype == Common::kRecorderEventTypeTimer
	 || _nextEvent.recordedtype == Common::kRecorderEventTypeTimeDate
	 || _nextEvent.recordedtype == Common::kRecorderEventTypeScreenUpdate
	 || _nextEvent.type == Common::EVENT_INVALID) {
		return false;
	}

	ev = _nextEvent;
	_nextEvent = _playbackFile->getNextEvent();
	switch (ev.type) {
	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		g_system->warpMouse(ev.mouse.x, ev.mouse.y);
		break;
	default:
		break;
	}
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
	case kRecorderUpdate:
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
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, Common::EventManager::kEventRecorderPriority, false);
}

uint32 EventRecorder::getRandomSeed(const Common::String &name) {
	if (_recordMode == kRecorderPlayback) {
		return _playbackFile->getHeader().randomSourceRecords[name];
	}
	uint32 result = g_system->getMillis();
	if (_recordMode == kRecorderRecord) {
		_recordFile->getHeader().randomSourceRecords[name] = result;
	}
	return result;
}

Common::String EventRecorder::generateRecordFileName(const Common::String &target) {
	Common::String pattern(target + ".r??");
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


void EventRecorder::init(const Common::String &recordFileName, RecordMode mode) {
	_fakeMixerManager = new NullMixerManager();
	_fakeMixerManager->init();
	_fakeMixerManager->suspendAudio();
	_fakeTimer = 0;
	_lastMillis = g_system->getMillis();
	_lastScreenshotTime = 0;
	_recordMode = mode;
	_needcontinueGame = false;
	if (ConfMan.hasKey("disable_display")) {
		DebugMan.enableDebugChannel("EventRec");
		gDebugLevel = 1;
	}
	if ((_recordMode == kRecorderPlayback) || (_recordMode == kRecorderUpdate)) {
		debugC(1, kDebugLevelEventRec, "playback:action=\"Load file\" filename=%s", recordFileName.c_str());
		Common::EventDispatcher *eventDispatcher = g_system->getEventManager()->getEventDispatcher();
		eventDispatcher->clearEvents();
		eventDispatcher->ignoreSources(true);
		eventDispatcher->registerSource(this, false);
	}
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
		_controlPanel->reflowLayout();
	}
	if ((_recordMode == kRecorderPlayback) || (_recordMode == kRecorderUpdate)) {
		applyPlaybackSettings();
		_nextEvent = _playbackFile->getNextEvent();
	}
	if ((_recordMode == kRecorderRecord) || (_recordMode == kRecorderUpdate)) {
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
 * @param id of recording or playing back game
 * @return true in case of success, false in case of error
 */
bool EventRecorder::openRecordFile(const Common::String &fileName) {
	bool result;
	switch (_recordMode) {
	case kRecorderRecord:
		_recordFile = new Common::PlaybackFile();
		return _recordFile->openWrite(fileName);
	case kRecorderPlayback:
		_recordMode = kPassthrough;
		_playbackFile = new Common::PlaybackFile();
		result = _playbackFile->openRead(fileName);
		_recordMode = kRecorderPlayback;
		return result;
	case kRecorderUpdate:
		_recordMode = kPassthrough;
		_playbackFile = new Common::PlaybackFile();
		result = _playbackFile->openRead(fileName);
		_recordMode = kRecorderUpdate;
		_recordFile = new Common::PlaybackFile();
		result &= _recordFile->openWrite(fileName + ".new");
		return result;
	default:
		return false;
	}
	return true;
}

bool EventRecorder::checkGameHash(const ADGameDescription *gameDesc) {
	if (_playbackFile->getHeader().hashRecords.size() == 0) {
		warning("Engine doesn't contain description table, skipping hash check");
		return true;
	}
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (fileDesc->md5 == nullptr)
			continue;

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

void EventRecorder::registerMixerManager(MixerManager *mixerManager) {
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

MixerManager *EventRecorder::getMixerManager() {
	if (_recordMode == kPassthrough) {
		return _realMixerManager;
	} else {
		return _fakeMixerManager;
	}
}

void EventRecorder::getConfigFromDomain(const Common::ConfigManager::Domain *domain) {
	for (Common::ConfigManager::Domain::const_iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		_recordFile->getHeader().settingsRecords[entry->_key] = entry->_value;
	}
}

void EventRecorder::getConfig() {
	getConfigFromDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	getConfigFromDomain(ConfMan.getActiveDomain());
	_recordFile->getHeader().settingsRecords["save_slot"] = ConfMan.get("save_slot");
}


void EventRecorder::applyPlaybackSettings() {
	for (Common::StringMap::const_iterator i = _playbackFile->getHeader().settingsRecords.begin(); i != _playbackFile->getHeader().settingsRecords.end(); ++i) {
		if (_recordMode == kRecorderUpdate) {
			// copy the header values to the new recording
			_recordFile->getHeader().settingsRecords[i->_key] = i->_value;
		}

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

bool EventRecorder::notifyEvent(const Common::Event &ev) {
	if ((!_initialized) && (_recordMode != kRecorderPlaybackPause)) {
		return false;
	}

	checkForKeyCode(ev);
	Common::Event evt = ev;
	evt.mouse.x = evt.mouse.x * (g_system->getOverlayWidth() / g_system->getWidth());
	evt.mouse.y = evt.mouse.y * (g_system->getOverlayHeight() / g_system->getHeight());
	switch (_recordMode) {
	case kRecorderUpdate: // passthrough
	case kRecorderPlayback:
		// pass through screen updates to avoid loss of sync!
		if (evt.type == Common::EVENT_SCREEN_CHANGED)
			g_gui.processEvent(evt, _controlPanel);
		if (_recordMode == kRecorderUpdate) {
			// write a copy of the event to the output buffer
			Common::RecorderEvent e(ev);
			e.recordedtype = Common::kRecorderEventTypeNormal;
			e.time = _fakeTimer;
			_recordFile->writeEvent(e);
		}
		return false;
	case kRecorderRecord:
		g_gui.processEvent(evt, _controlPanel);
		if (((evt.type == Common::EVENT_LBUTTONDOWN) || (evt.type == Common::EVENT_LBUTTONUP) || (evt.type == Common::EVENT_MOUSEMOVE)) && _controlPanel->isMouseOver()) {
			return true;
		} else {
			Common::RecorderEvent e(ev);
			e.recordedtype = Common::kRecorderEventTypeNormal;
			e.time = _fakeTimer;
			_recordFile->writeEvent(e);
			return false;
		}
	case kRecorderPlaybackPause: {
		Common::Event dialogEvent;
		if (_controlPanel->isEditDlgVisible()) {
			dialogEvent = ev;
		} else {
			dialogEvent = evt;
		}
		g_gui.processEvent(dialogEvent, _controlPanel->getActiveDlg());
		if (((dialogEvent.type == Common::EVENT_LBUTTONDOWN) || (dialogEvent.type == Common::EVENT_LBUTTONUP) || (dialogEvent.type == Common::EVENT_MOUSEMOVE)) && _controlPanel->isMouseOver()) {
			return true;
		}
		return false;
	}
	default:
		return false;
	}
}

void EventRecorder::setGameMd5(const ADGameDescription *gameDesc) {
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (fileDesc->md5 != nullptr) {
			_recordFile->getHeader().hashRecords[fileDesc->fileName] = fileDesc->md5;
		}
	}
}

void EventRecorder::processGameDescription(const ADGameDescription *desc) {
	if ((_recordMode == kRecorderRecord) || (_recordMode == kRecorderUpdate)) {
		setGameMd5(desc);
	}
	if (((_recordMode == kRecorderPlayback) || (_recordMode == kRecorderUpdate)) && !checkGameHash(desc)) {
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
			_recordFile->saveScreenShot(screen, md5);
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
		if (saveFile != nullptr) {
			_recordFile->addSaveFile(fileName, saveFile);
			saveFile->seek(0);
		}
		return saveFile;
	default:
		return nullptr;
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
		g_gui.checkScreenChange();
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
	if ((_recordMode == kRecorderPlayback) || (_recordMode == kRecorderUpdate)) {
		Common::StringArray result;
		for (Common::HashMap<Common::String, Common::PlaybackFile::SaveFileBuffer>::iterator  i = _playbackFile->getHeader().saveFiles.begin(); i != _playbackFile->getHeader().saveFiles.end(); ++i) {
			if (i->_key.matchString(pattern, false, "/")) {
				result.push_back(i->_key);
			}
		}
		return result;
	} else {
		return _realSaveManager->listSavefiles(pattern);
	}
}

void EventRecorder::setFileHeader() {
	if ((_recordMode != kRecorderRecord) && (_recordMode != kRecorderUpdate)) {
		return;
	}
	TimeDate t;
	QualifiedGameDescriptor desc = EngineMan.findTarget(ConfMan.getActiveDomainName());
	g_system->getTimeAndDate(t);
	if (_author.empty()) {
		setAuthor("Unknown Author");
	}
	if (_name.empty()) {
		g_eventRec.setName(Common::String::format("%.2d.%.2d.%.4d ", t.tm_mday, t.tm_mon + 1, 1900 + t.tm_year) + desc.description);
	}
	_recordFile->getHeader().author = _author;
	_recordFile->getHeader().notes = _desc;
	_recordFile->getHeader().name = _name;
}

SDL_Surface *EventRecorder::getSurface(int width, int height) {
	// Create a RGB565 surface of the requested dimensions.
	return SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 16, 0xF800, 0x07E0, 0x001F, 0x0000);
}

bool EventRecorder::switchMode() {
	const Plugin *plugin = EngineMan.findPlugin(ConfMan.get("engineid"));
	bool metaInfoSupport = plugin->get<MetaEngine>().hasFeature(MetaEngine::kSavesSupportMetaInfo);
	bool featuresSupport = metaInfoSupport &&
						  g_engine->canSaveGameStateCurrently() &&
						  plugin->get<MetaEngine>().hasFeature(MetaEngine::kSupportsListSaves) &&
						  plugin->get<MetaEngine>().hasFeature(MetaEngine::kSupportsDeleteSave);
	if (!featuresSupport) {
		return false;
	}

	const Common::String target = ConfMan.getActiveDomainName();
	SaveStateList saveList = plugin->get<MetaEngine>().listSaves(target.c_str());

	int emptySlot = 1;
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
			Common::Event eventReturnToLauncher;
			eventReturnToLauncher.type = Common::EVENT_RETURN_TO_LAUNCHER;
			g_system->getEventManager()->pushEvent(eventReturnToLauncher);
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
	const Plugin *plugin = EngineMan.findPlugin(ConfMan.get("engineid"));
	const Common::String target = ConfMan.getActiveDomainName();
	 plugin->get<MetaEngine>().removeSaveState(target.c_str(), _temporarySlot);
	_temporarySlot = -1;
}

} // End of namespace GUI

#endif // ENABLE_EVENTRECORDER
