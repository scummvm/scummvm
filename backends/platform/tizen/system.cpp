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

#define FORBIDDEN_SYMBOL_EXCEPTION_exit

#include <FUiCtrlMessageBox.h>
#include <FLocales.h>

#include "common/config-manager.h"
#include "common/file.h"
#include "engines/engine.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/fonts/bdf.h"
#include "backends/saves/default/default-saves.h"
#include "backends/events/default/default-events.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "backends/mutex/mutex.h"
#include "backends/fs/fs-factory.h"
#include "backends/timer/tizen/timer.h"

#include "backends/platform/tizen/form.h"
#include "backends/platform/tizen/system.h"
#include "backends/platform/tizen/graphics.h"
#include "backends/platform/tizen/audio.h"

using namespace Tizen::Base;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Locales;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::System;

#define DEFAULT_CONFIG_FILE "scummvm.ini"
#define MUTEX_BUFFER_SIZE 5

//
// TizenFilesystemFactory
//
class TizenFilesystemFactory : public FilesystemFactory {
	AbstractFSNode *makeRootFileNode() const;
	AbstractFSNode *makeCurrentDirectoryFileNode() const;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const;
};

AbstractFSNode *TizenFilesystemFactory::makeRootFileNode() const {
	return new TizenFilesystemNode("/");
}

AbstractFSNode *TizenFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new TizenFilesystemNode("/");
}

AbstractFSNode *TizenFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	AppAssert(!path.empty());
	return new TizenFilesystemNode(path);
}

//
// TizenSaveFileManager
//
struct TizenSaveFileManager : public DefaultSaveFileManager {
	bool removeSavefile(const Common::String &filename);
};

bool TizenSaveFileManager::removeSavefile(const Common::String &filename) {
	// Assure the savefile name cache is up-to-date.
	assureCached(getSavePath());
	if (getError().getCode() != Common::kNoError)
		return false;

	// Obtain node if exists.
	SaveFileCache::const_iterator file = _saveFileCache.find(filename);
	if (file == _saveFileCache.end()) {
		return false;
	} else {
		const Common::FSNode fileNode = file->_value;
		// Remove from cache, this invalidates the 'file' iterator.
		_saveFileCache.erase(file);
		file = _saveFileCache.end();

		String unicodeFileName;
		StringUtil::Utf8ToString(fileNode.getPath().c_str(), unicodeFileName);

		switch (Tizen::Io::File::Remove(unicodeFileName)) {
		case E_SUCCESS:
			return true;

		case E_ILLEGAL_ACCESS:
			setError(Common::kWritePermissionDenied, "Search or write permission denied: " +
						file.getName());
			break;

		default:
			setError(Common::kPathDoesNotExist, "removeSavefile: '" + file.getName() +
						"' does not exist or path is invalid");
			break;
		}

		return false;
	}
}

//
// TizenMutexManager
//
struct TizenMutexManager : public MutexManager {
	TizenMutexManager();
	~TizenMutexManager();
	OSystem::MutexRef createMutex();
	void lockMutex(OSystem::MutexRef mutex);
	void unlockMutex(OSystem::MutexRef mutex);
	void deleteMutex(OSystem::MutexRef mutex);
private:
	Mutex *_buffer[MUTEX_BUFFER_SIZE];
};

TizenMutexManager::TizenMutexManager() {
	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		_buffer[i] = NULL;
	}
}

TizenMutexManager::~TizenMutexManager() {
	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		if (_buffer[i] != NULL) {
			delete _buffer[i];
		}
	}
}

OSystem::MutexRef TizenMutexManager::createMutex() {
	Mutex *mutex = new Mutex();
	mutex->Create();

	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		if (_buffer[i] == NULL) {
			_buffer[i] = mutex;
			break;
		}
	}

	return (OSystem::MutexRef) mutex;
}

void TizenMutexManager::lockMutex(OSystem::MutexRef mutex) {
	Mutex *m = (Mutex *)mutex;
	m->Acquire();
}

void TizenMutexManager::unlockMutex(OSystem::MutexRef mutex) {
	Mutex *m = (Mutex *)mutex;
	m->Release();
}

void TizenMutexManager::deleteMutex(OSystem::MutexRef mutex) {
	Mutex *m = (Mutex *)mutex;

	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		if (_buffer[i] == m) {
			_buffer[i] = NULL;
		}
	}

	delete m;
}

//
// TizenEventManager
//
struct TizenEventManager : public DefaultEventManager {
	TizenEventManager(Common::EventSource *boss);
	void init();
	int shouldQuit() const;
};

TizenEventManager::TizenEventManager(Common::EventSource *boss) :
	DefaultEventManager(boss) {
}

void TizenEventManager::init() {
	DefaultEventManager::init();

	// theme and vkbd should have now loaded - clear the splash screen
	TizenSystem *system = (TizenSystem *)g_system;
	TizenGraphicsManager *graphics = system->getGraphics();
	if (graphics) {
		graphics->setReady();
	}
}

int TizenEventManager::shouldQuit() const {
	TizenSystem *system = (TizenSystem *)g_system;
	return DefaultEventManager::shouldQuit() || system->isClosing();
}

//
// TizenAppFrame - avoid drawing the misplaced UiTheme at startup
//
struct TizenAppFrame : Frame {
	result OnDraw(void) {
		logEntered();
		TizenAppForm *form = (TizenAppForm *)GetCurrentForm();
		if (form->isStarting()) {
			Canvas *canvas = GetCanvasN();
			canvas->SetBackgroundColor(Color::GetColor(COLOR_ID_BLACK));
			canvas->Clear();
			delete canvas;
		}
		return E_SUCCESS;
	}
};

//
// TizenSystem
//
TizenSystem::TizenSystem(TizenAppForm *appForm) :
	_appForm(appForm),
	_audioThread(0),
	_epoch(0) {
}

result TizenSystem::Construct(void) {
	logEntered();

	_fsFactory = new TizenFilesystemFactory();
	if (!_fsFactory) {
		return E_OUT_OF_MEMORY;
	}

	_resourcePath = fromString(App::GetInstance()->GetAppResourcePath());
	return E_SUCCESS;
}

TizenSystem::~TizenSystem() {
	logEntered();
}

result TizenSystem::initModules() {
	logEntered();

	_mutexManager = new TizenMutexManager();
	if (!_mutexManager) {
		return E_OUT_OF_MEMORY;
	}

	_timerManager = new TizenTimerManager();
	if (!_timerManager) {
		return E_OUT_OF_MEMORY;
	}

	_savefileManager = new TizenSaveFileManager();
	if (!_savefileManager) {
		return E_OUT_OF_MEMORY;
	}

	_graphicsManager = new TizenGraphicsManager(_appForm);
	if (!_graphicsManager || getGraphics()->Construct() != E_SUCCESS) {
		return E_OUT_OF_MEMORY;
	}

	// depends on _graphicsManager when ENABLE_VKEYBD enabled
	_eventManager = new TizenEventManager(this);
	if (!_eventManager) {
		return E_OUT_OF_MEMORY;
	}

	_audioThread = new AudioThread();
	if (!_audioThread) {
		return E_OUT_OF_MEMORY;
	}

	_mixer = _audioThread->Construct();
	if (!_mixer) {
		return E_OUT_OF_MEMORY;
	}

	_audiocdManager = (AudioCDManager *)new DefaultAudioCDManager();
	if (!_audiocdManager) {
		return E_OUT_OF_MEMORY;
	}

	if (IsFailed(_audioThread->Start())) {
		AppLog("Failed to start audio thread");
		return E_OUT_OF_MEMORY;
	}

	logLeaving();
	return E_SUCCESS;
}

void TizenSystem::initBackend() {
	logEntered();

	Common::String dataPath = fromString(App::GetInstance()->GetAppDataPath());

	// use the mobile device theme
	ConfMan.set("gui_theme", _resourcePath + "scummremastered");

	// allow tizen virtual keypad pack to be found
	ConfMan.set("vkeybdpath", _resourcePath + "vkeybd_bada");
	ConfMan.set("vkeybd_pack_name", "vkeybd_bada");

	// set default save path to writable area
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.set("savepath", dataPath);
	}

	// default to no auto-save
	if (!ConfMan.hasKey("autosave_period")) {
		ConfMan.setInt("autosave_period", 0);
	}

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.setBool("confirm_exit", false);

	SystemTime::GetTicks(_epoch);

	if (E_SUCCESS != initModules()) {
		AppLog("initModules failed");
	} else {
		OSystem::initBackend();

		// replace kBigGUIFont for the vkbd and on-screen messages
		Common::String fontCacheFile = dataPath + "helvR24.fcc";
		TizenFilesystemNode file(fontCacheFile);
		if (!file.exists()) {
			Common::String bdfFile = _resourcePath + "fonts/helvR24.bdf";
			TizenFilesystemNode file(bdfFile);
			if (file.exists()) {
				Common::SeekableReadStream *stream = file.createReadStream();
				Common::File fontFile;
				if (stream && fontFile.open(stream, bdfFile)) {
					Graphics::BdfFont *font = Graphics::BdfFont::loadFont(fontFile);
					Graphics::BdfFont::cacheFontData(*font, fontCacheFile);
					FontMan.setFont(Graphics::FontManager::kBigGUIFont, font);
				}
			}
		} else {
			Common::SeekableReadStream *stream = file.createReadStream();
			Common::File fontFile;
			if (stream && fontFile.open(stream, fontCacheFile)) {
				Graphics::BdfFont *font = Graphics::BdfFont::loadFromCache(fontFile);
				if (font) {
					FontMan.setFont(Graphics::FontManager::kBigGUIFont, font);
				}
			}
		}
	}
	logLeaving();
}

void TizenSystem::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// allow translations.dat and game .DAT files to be found
	s.addDirectory(_resourcePath, _resourcePath, priority);
}

void TizenSystem::destroyBackend() {
	closeAudio();

	delete _graphicsManager;
	_graphicsManager = NULL;

	delete _savefileManager;
	_savefileManager = NULL;

	delete _fsFactory;
	_fsFactory = NULL;

	delete _mixer;
	_mixer = NULL;

	delete _audiocdManager;
	_audiocdManager = NULL;

	delete _timerManager;
	_timerManager = NULL;

	delete _eventManager;
	_eventManager = NULL;

	delete _mutexManager;
	_mutexManager = NULL;
}

bool TizenSystem::pollEvent(Common::Event &event) {
	return _appForm->pollEvent(event);
}

uint32 TizenSystem::getMillis(bool skipRecord) {
	long long result, ticks = 0;
	SystemTime::GetTicks(ticks);
	result = ticks - _epoch;
	return result;
}

void TizenSystem::delayMillis(uint msecs) {
	if (!_appForm->isClosing()) {
		Thread::Sleep(msecs);
	}
}

void TizenSystem::updateScreen() {
	if (_graphicsManager != NULL) {
		_graphicsManager->updateScreen();
	}
}

void TizenSystem::getTimeAndDate(TimeDate &td) const {
	DateTime currentTime;

	if (E_SUCCESS == SystemTime::GetCurrentTime(WALL_TIME, currentTime)) {
		td.tm_sec = currentTime.GetSecond();
		td.tm_min = currentTime.GetMinute();
		td.tm_hour = currentTime.GetHour();
		td.tm_mday = currentTime.GetDay();
		td.tm_mon = currentTime.GetMonth();
		td.tm_year = currentTime.GetYear();

		Calendar *calendar = Calendar::CreateInstanceN(CALENDAR_GREGORIAN);
		calendar->SetTime(currentTime);
		td.tm_wday = calendar->GetTimeField(TIME_FIELD_DAY_OF_WEEK) - 1;
		delete calendar;
	}
}

void TizenSystem::fatalError() {
	systemError("ScummVM: Fatal internal error.");
}

void TizenSystem::exitSystem() {
	if (_appForm) {
		closeAudio();
		closeGraphics();
		_appForm->exitSystem();
	}
}

void TizenSystem::quit() {
	exit(0);
}

void TizenSystem::logMessage(LogMessageType::Type type, const char *message) {
	if (type == LogMessageType::kError) {
		systemError(message);
	} else {
		AppLog(message);
	}
}

Common::SeekableReadStream *TizenSystem::createConfigReadStream() {
	TizenFilesystemNode file(fromString(App::GetInstance()->GetAppDataPath()) + DEFAULT_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *TizenSystem::createConfigWriteStream() {
	TizenFilesystemNode file(fromString(App::GetInstance()->GetAppDataPath()) + DEFAULT_CONFIG_FILE);
	return file.createWriteStream();
}

void TizenSystem::closeAudio() {
	if (_audioThread) {
		_audioThread->Quit();
		_audioThread->Join();
		delete _audioThread;
		_audioThread = NULL;
	}
}

void TizenSystem::closeGraphics() {
	if (_graphicsManager) {
		delete _graphicsManager;
		_graphicsManager = NULL;
	}
}

void TizenSystem::setMute(bool on) {
	// only change mute after eventManager init() has completed
	if (_audioThread) {
		TizenGraphicsManager *graphics = getGraphics();
		if (graphics && graphics->isReady()) {
			_audioThread->setMute(on);
		}
	}
}

//
// create the ScummVM system
//
TizenAppForm *systemStart(Tizen::App::Application *app) {
	logEntered();

	Frame *appFrame = new (std::nothrow) TizenAppFrame();
	if (!appFrame || appFrame->Construct() == E_FAILURE) {
		AppLog("Failed to create appFrame");
		return NULL;
	}
	app->AddFrame(*appFrame);

	TizenAppForm *appForm = new TizenAppForm();
	if (!appForm) {
		AppLog("Failed to create appForm");
		return NULL;
	}

	if (E_SUCCESS != appForm->Construct() ||
		E_SUCCESS != appFrame->AddControl(appForm)) {
		delete appForm;
		AppLog("Failed to construct appForm");
		return NULL;
	}

	appFrame->SetCurrentForm(appForm);
	appForm->GetVisualElement()->SetShowState(false);

	logLeaving();
	return appForm;
}

//
// display a fatal error notification
//
void systemError(const char *message) {
	AppLog("Fatal system error: %s", message);

	if (strspn(message, "Config file buggy:") > 0) {
		Tizen::Io::File::Remove(App::GetInstance()->GetAppDataPath() + DEFAULT_CONFIG_FILE);
		Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT_ERR_CONFIG, NULL);
	} else {
		ArrayList *args = new ArrayList();
		args->Construct();
		args->Add(*(new String(message)));
		Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT_ERR, args);
	}

	if (g_system) {
		TizenSystem *system = (TizenSystem *)g_system;
		system->exitSystem();
	}
}
