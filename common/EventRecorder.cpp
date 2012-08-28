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


#include "common/EventRecorder.h"
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
namespace Common {

DECLARE_SINGLETON(EventRecorder);

const int kMaxRecordsNames = 0x64;
const int kDefaultScreenshotPeriod = 60000;
const int kDefaultBPP = 2;

uint32 readTime(ReadStream *inFile) {
	uint32 d = inFile->readByte();
	if (d == 0xff) {
		d = inFile->readUint32LE();
	}

	return d;
}

void writeTime(WriteStream *outFile, uint32 d) {
		//Simple RLE compression
	if (d >= 0xff) {
		outFile->writeByte(0xff);
		outFile->writeUint32LE(d);
	} else {
		outFile->writeByte(d);
	}
}

EventRecorder::EventRecorder() {
	_recordMode = kPassthrough;
	_timerManager = NULL;
	_fakeMixerManager = NULL;
	_initialized = false;
	_needRedraw = false;
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
	controlPanel->close();
	delete controlPanel;
	debugC(3, kDebugLevelEventRec, "EventRecorder: deinit");
	g_system->getEventManager()->getEventDispatcher()->unregisterSource(this);
	_recordMode = kPassthrough;
	_playbackFile->close();
	delete _playbackFile;
	switchMixer();
	switchTimerManagers();
}


void EventRecorder::processMillis(uint32 &millis) {
	if (!_initialized) {
		return;
	}
	if (_recordMode == kRecorderPlaybackPause) {
		millis = _fakeTimer;
	}
	uint32 millisDelay;
	RecorderEvent timerEvent;
	switch (_recordMode) {
	case kRecorderRecord:
		updateSubsystems();
		millisDelay = millis - _lastMillis;
		_lastMillis = millis;
		_fakeTimer += millisDelay;
		controlPanel->setReplayedTime(_fakeTimer);
		timerEvent.type = EVENT_TIMER;
		timerEvent.time = _fakeTimer;
		_playbackFile->writeEvent(timerEvent);
		takeScreenshot();
		_timerManager->handler();
		break;
	case kRecorderPlayback:
		updateSubsystems();
		if (_nextEvent.type == EVENT_TIMER) {
			_fakeTimer = _nextEvent.time;
			_nextEvent = _playbackFile->getNextEvent();
			_timerManager->handler();
		}
		millis = _fakeTimer;
		controlPanel->setReplayedTime(_fakeTimer);
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

void EventRecorder::checkForKeyCode(const Event &event) {
	if ((event.type == EVENT_KEYDOWN) && (event.kbd.flags&KBD_CTRL) && (event.kbd.keycode == KEYCODE_p) && (!event.synthetic)) {
		togglePause();
	}
}

bool EventRecorder::pollEvent(Event &ev) {
	if ((_recordMode != kRecorderPlayback) || !_initialized)
		return false;
	
	if ((_nextEvent.type ==  EVENT_INVALID) || (_nextEvent.type == EVENT_TIMER)) {
		return false;
	}

	switch (_nextEvent.type) {
	case EVENT_MOUSEMOVE:
	case EVENT_LBUTTONDOWN:
	case EVENT_LBUTTONUP:
	case EVENT_RBUTTONDOWN:
	case EVENT_RBUTTONUP:
	case EVENT_WHEELUP:
	case EVENT_WHEELDOWN:
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
		controlPanel->runModal();
		_recordMode = oldState;
		_initialized = true;
		break;
	case kRecorderPlaybackPause:
		controlPanel->close();
		break;
	default:
		break;
	}
}

void EventRecorder::RegisterEventSource() {
	g_system->getEventManager()->getEventDispatcher()->registerMapper(this);
}

uint32 EventRecorder::getRandomSeed(const String &name) {
	uint32 result = g_system->getMillis();
	if (_recordMode == kRecorderRecord) {
		_playbackFile->getHeader().randomSourceRecords[name] = result;
	} else if (_recordMode == kRecorderPlayback) {
		result = _playbackFile->getHeader().randomSourceRecords[name];
	}
	return result;
}

String EventRecorder::generateRecordFileName(const String &target) {
	String pattern(target+".r??");
	StringArray files = g_system->getSavefileManager()->listSavefiles(pattern);
	for (int i = 0; i < kMaxRecordsNames; ++i) {
		String recordName = String::format("%s.r%02d", target.c_str(), i);
		if (find(files.begin(), files.end(), recordName) != files.end()) {
			continue;
		}
		return recordName;
	}
	return "";
}


void EventRecorder::init(String recordFileName, RecordMode mode) {
	_fakeMixerManager = new NullSdlMixerManager();
	_fakeMixerManager->init();
	_fakeMixerManager->suspendAudio();
	_fakeTimer = 0;
	_lastMillis = g_system->getMillis();
	_playbackFile = new PlaybackFile();
	_lastScreenshotTime = 0;
	_recordMode = mode;
	_needcontinueGame = false;
	_fastPlayback = false;

	g_system->getEventManager()->getEventDispatcher()->registerSource(this, false);
	_screenshotPeriod = ConfMan.getInt("screenshot_period");
	if (_screenshotPeriod == 0) {
		_screenshotPeriod = kDefaultScreenshotPeriod;
	}
	if (!openRecordFile(recordFileName)) {
		deinit();
		return;
	}
	if (_recordMode != kPassthrough) {
		controlPanel = new GUI::OnScreenDialog(_recordMode == kRecorderRecord);
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

bool EventRecorder::openRecordFile(const String &fileName) {
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
	if ((gameDesc == NULL) && (_playbackFile->getHeader().hashRecords.size() != 0)) {
		warning("Engine doesn't contain description table");
		return false;
	}
	for (const ADGameFileDescription *fileDesc = gameDesc->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (_playbackFile->getHeader().hashRecords.find(fileDesc->fileName) == _playbackFile->getHeader().hashRecords.end()) {
			warning("MD5 hash for file %s not found in record file", fileDesc->fileName);
			return false;
		}
		if (_playbackFile->getHeader().hashRecords[fileDesc->fileName] != fileDesc->md5) {
			warning("Incorrect version of game file %s. Stored MD5 is %s. MD5 of loaded game is %s", fileDesc->fileName, _playbackFile->getHeader().hashRecords[fileDesc->fileName].c_str(), fileDesc->md5);
			return false;
		}
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

void EventRecorder::getConfigFromDomain(ConfigManager::Domain *domain) {
	for (ConfigManager::Domain::iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		_playbackFile->getHeader().settingsRecords[entry->_key] = entry->_value;
	}
}

void EventRecorder::getConfig() {
	getConfigFromDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	getConfigFromDomain(ConfMan.getActiveDomain());
	_playbackFile->getHeader().settingsRecords["save_slot"] = ConfMan.get("save_slot");
}


void EventRecorder::applyPlaybackSettings() {
	for (StringMap::iterator i = _playbackFile->getHeader().settingsRecords.begin(); i != _playbackFile->getHeader().settingsRecords.end(); ++i) {
		String currentValue = ConfMan.get(i->_key);
		if (currentValue != i->_value) {
			warning("Config value <%s>: %s -> %s", i->_key.c_str(), i->_value.c_str(), currentValue.c_str());
			ConfMan.set(i->_key, i->_value, ConfMan.kTransientDomain);
		}
	}
	removeDifferentEntriesInDomain(ConfMan.getDomain(ConfMan.kApplicationDomain));
	removeDifferentEntriesInDomain(ConfMan.getActiveDomain());
}

void EventRecorder::removeDifferentEntriesInDomain(ConfigManager::Domain *domain) {
	for (ConfigManager::Domain::iterator entry = domain->begin(); entry!= domain->end(); ++entry) {
		if (_playbackFile->getHeader().settingsRecords.find(entry->_key) == _playbackFile->getHeader().settingsRecords.end()) {
			warning("Config value <%s>: %s -> (null)", entry->_key.c_str(), entry->_value.c_str());
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

List<Event> EventRecorder::mapEvent(const Event &ev, EventSource *source) {

	if ((!_initialized) && (_recordMode != kRecorderPlaybackPause)) {
		return DefaultEventMapper::mapEvent(ev, source);
	}

	checkForKeyCode(ev);

	Event evt = ev;
	evt.mouse.x = evt.mouse.x * (g_system->getOverlayWidth() / g_system->getWidth());
	evt.mouse.y = evt.mouse.y * (g_system->getOverlayHeight() / g_system->getHeight());
	switch (_recordMode) {
	case kRecorderPlayback:
		if (ev.synthetic != true) {
			return List<Event>();
		}
		return DefaultEventMapper::mapEvent(ev, source);
		break;
	case kRecorderRecord:
		g_gui.processEvent(evt, controlPanel);
		if (((evt.type == EVENT_LBUTTONDOWN) || (evt.type == EVENT_LBUTTONUP) || (evt.type == EVENT_MOUSEMOVE)) && controlPanel->isMouseOver()) {
			return List<Event>();
		} else {
			RecorderEvent e;
			memcpy(&e, &ev, sizeof(ev));
			e.time = _fakeTimer;
			_playbackFile->writeEvent(e);
			return DefaultEventMapper::mapEvent(ev, source);
		}
		break;
	case kRecorderPlaybackPause: {
		Event dialogEvent;
			if (controlPanel->isEditDlgVisible()) {
				dialogEvent = ev;
			} else {
				dialogEvent = evt;
			}
			g_gui.processEvent(dialogEvent, controlPanel->getActiveDlg());
			if (((dialogEvent.type == EVENT_LBUTTONDOWN) || (dialogEvent.type == EVENT_LBUTTONUP) || (dialogEvent.type == EVENT_MOUSEMOVE)) && controlPanel->isMouseOver()) {
				return List<Event>();
			}
			return DefaultEventMapper::mapEvent(dialogEvent, source);
		}
		break;
	default:
		return DefaultEventMapper::mapEvent(ev, source);
	}
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
	}
}

void EventRecorder::deleteRecord(const String& fileName) {
	g_system->getSavefileManager()->removeSavefile(fileName);
}

void EventRecorder::setAuthor(const String &author) {
	_author = author;
}

void EventRecorder::setNotes(const String &desc) {
	_desc = desc;
}

void EventRecorder::setName(const String &name) {
	_name = name;
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
	MemoryReadStream bitmapStream((const byte*)screen.pixels, screen.w * screen.h * screen.format.bytesPerPixel);
	computeStreamMD5(bitmapStream, md5);
	return true;
}

SeekableReadStream *EventRecorder::processSaveStream(const String &fileName) {
	InSaveFile *saveFile;
	switch (_recordMode) {
	case kRecorderPlayback:
		for (HashMap<String, PlaybackFile::SaveFileBuffer>::iterator  i = _playbackFile->getHeader().saveFiles.begin(); i != _playbackFile->getHeader().saveFiles.end(); ++i) {
			debug("%s %d ", i->_key.c_str(), i->_value.size);
		}
		return new MemoryReadStream(_playbackFile->getHeader().saveFiles[fileName].buffer, _playbackFile->getHeader().saveFiles[fileName].size);
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

SaveFileManager *EventRecorder::getSaveManager(SaveFileManager *realSaveManager) {
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
		g_gui.theme()->openDialog(true, GUI::ThemeEngine::kShadingNone);
		controlPanel->drawDialog();
		g_gui.theme()->finishBuffering();
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

StringArray EventRecorder::listSaveFiles(const String &pattern) {
	if (_recordMode == kRecorderPlayback) {
		StringArray result;
		for (HashMap<String, PlaybackFile::SaveFileBuffer>::iterator  i = _playbackFile->getHeader().saveFiles.begin(); i != _playbackFile->getHeader().saveFiles.end(); ++i) {
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
	const EnginePlugin *plugin = 0;
	GameDescriptor desc = EngineMan.findGame(ConfMan.getActiveDomainName(), &plugin);
	g_system->getTimeAndDate(t);
	if (_author.empty()) {
		setAuthor("Unknown Author");
	}
	if (_name.empty()) {
		g_eventRec.setName(String::format("%.2d.%.2d.%.4d ", t.tm_mday, t.tm_mon, 1900 + t.tm_year) + desc.description());
	}
	_playbackFile->getHeader().author = _author;
	_playbackFile->getHeader().notes = _desc;
	_playbackFile->getHeader().name = _name;
}

SDL_Surface *EventRecorder::getSurface(int width, int height) {
	SDL_Surface *surface = new SDL_Surface();
	surface->format = new SDL_PixelFormat();
	surface->flags = 0;
	surface->format->palette = NULL;
	surface->format->BitsPerPixel = 16;
	surface->format->BytesPerPixel = 2;
	surface->format->Rloss = 3;
	surface->format->Gloss = 2;
	surface->format->Bloss = 3;
	surface->format->Aloss = 8;
	surface->format->Rshift = 11;
	surface->format->Gshift = 5;
	surface->format->Bshift = 0;
	surface->format->Ashift = 0;
	surface->format->Rmask = 63488;
	surface->format->Gmask = 2016;
	surface->format->Bmask = 31;
	surface->format->Amask = 0;
	surface->format->colorkey = 0;
	surface->format->alpha = 255;
	surface->w = width;
	surface->h = height;
	surface->pitch = width * 2;
	surface->pixels = (char *)malloc(surface->pitch * surface->h);
	surface->offset = 0;
	surface->hwdata = NULL;
	surface->clip_rect.x = 0;
	surface->clip_rect.y = 0;
	surface->clip_rect.w = width;
	surface->clip_rect.h = height;
	surface->unused1 = 0;
	surface->locked = 0;
	surface->map = NULL;
	surface->format_version = 4;
	surface->refcount = 1;
	return surface;
}

bool EventRecorder::switchMode() {
	const String gameId = ConfMan.get("gameid");
	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);
	bool metaInfoSupport = (*plugin)->hasFeature(MetaEngine::kSavesSupportMetaInfo);
	bool featuresSupport = metaInfoSupport &&
						  g_engine->canSaveGameStateCurrently() &&
						  (*plugin)->hasFeature(MetaEngine::kSupportsListSaves) &&
						  (*plugin)->hasFeature(MetaEngine::kSupportsDeleteSave);
	if (!featuresSupport) {
		return false;
	}

	int emptySlot = 1;
	SaveStateList saveList = (*plugin)->listSaves(gameId.c_str());
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
	String saveName;
	if (emptySlot >= 0) {
		saveName = String::format("Save %d", emptySlot + 1);
		Error status = g_engine->saveGameState(emptySlot, saveName);
		if (status.getCode() == kNoError) {
			Event eventRTL;
			eventRTL.type = EVENT_RTL;
			g_system->getEventManager()->pushEvent(eventRTL);
		}
	}
	ConfMan.set("record_mode", "", ConfigManager::kTransientDomain);
	ConfMan.setInt("save_slot", emptySlot, ConfigManager::kTransientDomain);
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
	const String gameId = ConfMan.get("gameid");
	const EnginePlugin *plugin = 0;
	EngineMan.findGame(gameId, &plugin);
	 (*plugin)->removeSaveState(gameId.c_str(), _temporarySlot);
	_temporarySlot = -1;
}

} // End of namespace Common
