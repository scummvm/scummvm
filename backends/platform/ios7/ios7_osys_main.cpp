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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <sys/time.h>
#include <QuartzCore/QuartzCore.h>
#include <dlfcn.h>

#include "common/scummsys.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/formats/ini-file.h"

#include "base/main.h"

#include "engines/engine.h"
#include "engines/metaengine.h"

#include "graphics/cursorman.h"
#include "gui/gui-manager.h"

#include "backends/graphics/ios/ios-graphics.h"
#include "backends/graphics3d/ios/ios-graphics3d.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"
#include "backends/mutex/pthread/pthread-mutex.h"
#include "backends/fs/chroot/chroot-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"
#include "audio/mixer.h"
#include "audio/mixer_intern.h"

#include "backends/platform/ios7/ios7_osys_main.h"


AQCallbackStruct OSystem_iOS7::s_AudioQueue;
SoundProc OSystem_iOS7::s_soundCallback = NULL;
void *OSystem_iOS7::s_soundParam = NULL;

class SandboxedSaveFileManager : public DefaultSaveFileManager {
	Common::Path _sandboxRootPath;
public:

	SandboxedSaveFileManager(const Common::Path &sandboxRootPath, const Common::Path &defaultSavepath)
			: DefaultSaveFileManager(defaultSavepath), _sandboxRootPath(sandboxRootPath) {
	}

	bool removeSavefile(const Common::String &filename) override {
		Common::Path chrootedFile = getSavePath().join(filename);
		Common::Path realFilePath = _sandboxRootPath.join(chrootedFile);

		if (remove(realFilePath.toString(Common::Path::kNativeSeparator).c_str()) != 0) {
			if (errno == EACCES)
				setError(Common::kWritePermissionDenied, "Search or write permission denied: "+chrootedFile.toString(Common::Path::kNativeSeparator));

			if (errno == ENOENT)
				setError(Common::kPathDoesNotExist, "removeSavefile: '"+chrootedFile.toString(Common::Path::kNativeSeparator)+"' does not exist or path is invalid");
			return false;
		} else {
			return true;
		}
	}
};

OSystem_iOS7::OSystem_iOS7() :
	_mixer(NULL), _queuedEventTime(0),
	_screenOrientation(kScreenOrientationAuto),
	_runningTasks(0) {
	_queuedInputEvent.type = Common::EVENT_INVALID;
	_currentTouchMode = kTouchModeTouchpad;

	_chrootBasePath = iOS7_getDocumentsDir();
	ChRootFilesystemFactory *chFsFactory = new ChRootFilesystemFactory(_chrootBasePath);
	_fsFactory = chFsFactory;
	// Add virtual drive for bundle path
	Common::String appBubdlePath = iOS7_getAppBundleDir();
	if (!appBubdlePath.empty())
		chFsFactory->addVirtualDrive("appbundle:", appBubdlePath);
}

OSystem_iOS7::~OSystem_iOS7() {
	AudioQueueDispose(s_AudioQueue.queue, true);

	delete _mixer;
	delete _graphicsManager;
}

#if defined(USE_OPENGL) && defined(USE_GLAD)
void *OSystem_iOS7::getOpenGLProcAddress(const char *name) const {
	return dlsym(RTLD_DEFAULT, name);
}
#endif

int OSystem_iOS7::timerHandler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_iOS7::initBackend() {
	_savefileManager = new SandboxedSaveFileManager(Common::Path(_chrootBasePath, Common::Path::kNativeSeparator), "/Savegames");

	_timerManager = new DefaultTimerManager();

	_startTime = CACurrentMediaTime();

	_graphicsManager = new iOSGraphicsManager();

	setupMixer();

	setTimerCallback(&OSystem_iOS7::timerHandler, 10);

	ConfMan.registerDefault("iconspath", Common::Path("/"));

	EventsBaseBackend::initBackend();
}

bool OSystem_iOS7::hasFeature(Feature f) {
	switch (f) {
	case kFeatureCursorPalette:
	case kFeatureCursorAlpha:
	case kFeatureFilteringMode:
	case kFeatureVirtualKeyboard:
#if TARGET_OS_IOS
	case kFeatureClipboardSupport:
#endif
	case kFeatureOpenUrl:
	case kFeatureNoQuit:
	case kFeatureKbdMouseSpeed:
	case kFeatureOpenGLForGame:
	case kFeatureShadersForGame:
	case kFeatureTouchscreen:
#ifdef SCUMMVM_NEON
	case kFeatureCpuNEON:
#endif
		return true;

	default:
		return ModularGraphicsBackend::hasFeature(f);
	}
}

void OSystem_iOS7::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureVirtualKeyboard:
		setShowKeyboard(enable);
		break;

	default:
		ModularGraphicsBackend::setFeatureState(f, enable);
		break;
	}
}

bool OSystem_iOS7::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureVirtualKeyboard:
		return isKeyboardShown();

	default:
		return ModularGraphicsBackend::getFeatureState(f);
	}
}

bool OSystem_iOS7::setGraphicsMode(int mode, uint flags) {
	bool render3d = flags & OSystem::kGfxModeRender3d;

	// Utilize the same way to switch between 2D and 3D graphics manager as
	// in SDL based backends and Android.
	iOSCommonGraphics *commonGraphics = dynamic_cast<iOSCommonGraphics *>(_graphicsManager);
	iOSCommonGraphics::State gfxManagerState = commonGraphics->getState();

	bool supports3D = _graphicsManager->hasFeature(kFeatureOpenGLForGame);
	bool switchedManager = false;

	// If the new mode and the current mode are not from the same graphics
	// manager, delete and create the new mode graphics manager
	if (render3d && !supports3D) {
		delete _graphicsManager;
		iOSGraphics3dManager *manager = new iOSGraphics3dManager();
		_graphicsManager = manager;
		commonGraphics = manager;
		switchedManager = true;
	} else if (!render3d && supports3D) {
		delete _graphicsManager;
		iOSGraphicsManager *manager = new iOSGraphicsManager();
		_graphicsManager = manager;
		commonGraphics = manager;
		switchedManager = true;
	}

	if (switchedManager) {
		// Setup the graphics mode and size first
		// This is needed so that we can check the supported pixel formats when
		// restoring the state.
		_graphicsManager->beginGFXTransaction();
		if (!_graphicsManager->setGraphicsMode(mode, flags))
			return false;
		_graphicsManager->initSize(gfxManagerState.screenWidth, gfxManagerState.screenHeight);
		_graphicsManager->endGFXTransaction();

		// This failing will probably have bad consequences...
		//if (!androidGraphicsManager->setState(gfxManagerState)) {
		//	return false;
		//}

		// Next setup the cursor again
		CursorMan.pushCursor(0, 0, 0, 0, 0, 0);
		CursorMan.popCursor();

		// Next setup cursor palette if needed
		if (_graphicsManager->getFeatureState(kFeatureCursorPalette)) {
			CursorMan.pushCursorPalette(0, 0, 0);
			CursorMan.popCursorPalette();
		}

		_graphicsManager->beginGFXTransaction();
		return true;
	} else {
		return _graphicsManager->setGraphicsMode(mode, flags);
	}
 }

void OSystem_iOS7::suspendLoop() {
	bool done = false;

	PauseToken pt;
	if (g_engine)
		pt = g_engine->pauseEngine();

	// We also need to stop the audio queue and restart it later in case there
	// is an audio interruption that render it invalid.
	stopSoundsystem();

	InternalEvent event;
	while (!done) {
		if (iOS7_fetchEvent(&event)) {
			if (event.type == kInputApplicationResumed)
				done = true;
			else if (event.type == kInputApplicationSaveState)
				handleEvent_applicationSaveState();
		}
		usleep(100000);
	}

	startSoundsystem();
}

void OSystem_iOS7::saveState() {
	// Clear any previous restore state to avoid having and obsolete one if we don't save it again below.
	clearState();

	// If there is an engine running and it both accepts autosave and loading from the launcher (required to restore the state),
	// do an autosave and save the current running target and autosave slot to a state file.
	if (g_engine && g_engine->hasFeature(Engine::kSupportsSavingDuringRuntime) && g_engine->canSaveAutosaveCurrently() &&
		g_engine->getMetaEngine()->hasFeature(MetaEngine::kSupportsLoadingDuringStartup)) {
		Common::String targetName(ConfMan.getActiveDomainName());
		int saveSlot = g_engine->getAutosaveSlot();
		if (saveSlot == -1)
			return;
		// Make sure we do not overwrite a user save
		SaveStateDescriptor desc = g_engine->getMetaEngine()->querySaveMetaInfos(targetName.c_str(), saveSlot);
		if (desc.getSaveSlot() != -1 && !desc.isAutosave())
			return;

		// Do the auto-save, and if successful create the state file with the target and save slot.
		if (g_engine->saveGameState(saveSlot, _("Autosave"), true).getCode() == Common::kNoError) {
			Common::INIFile stateFile;
			stateFile.addSection("state");
			stateFile.setKey("restore_target", "state", targetName);
			stateFile.setKey("restore_slot", "state", Common::String::format("%d", saveSlot));
			stateFile.saveToFile("/scummvm.state");
		}
	}
}

void OSystem_iOS7::restoreState() {
	// If the g_engine is still running (i.e. the application was not terminated) we don't need to do anything other than clear the saved state.
	if (g_engine) {
		clearState();
		return;
	}

	// Read the state
	Common::FSNode node("/scummvm.state");
	Common::File stateFile;
	if (!stateFile.open(node))
		return;

	Common::String targetName, slotString;
	int saveSlot = -1;
	Common::INIFile stateIniFile;
	if (stateIniFile.loadFromStream(stateFile) &&
		stateIniFile.getKey("restore_target", "state", targetName) &&
		stateIniFile.getKey("restore_slot", "state", slotString) &&
		!slotString.empty()) {
		char *errpos;
		saveSlot = (int)strtol(slotString.c_str(), &errpos, 10);
		if (slotString.c_str() == errpos)
			saveSlot = -1;
	}

	clearState();

	// Reload the state
	if (!targetName.empty() && saveSlot != -1) {
		ConfMan.setInt("save_slot", saveSlot, Common::ConfigManager::kTransientDomain);
		ConfMan.setActiveDomain(targetName);
		if (GUI::GuiManager::hasInstance())
			g_gui.exitLoop();
	}
}

void OSystem_iOS7::clearState() {
	Common::String statePath = _chrootBasePath + "/scummvm.state";
	remove(statePath.c_str());
}

uint32 OSystem_iOS7::getMillis(bool skipRecord) {
	CFTimeInterval timeInSeconds = CACurrentMediaTime();
	return (uint32) ((timeInSeconds - _startTime) * 1000.0);
}

void OSystem_iOS7::delayMillis(uint msecs) {
	//printf("delayMillis(%d)\n", msecs);
	usleep(msecs * 1000);
}

float OSystem_iOS7::getMouseSpeed() {
	switch (ConfMan.getInt("kbdmouse_speed")) {
	case 0:
		return 0.25;
	case 1:
		return 0.5;
	case 2:
		return 0.75;
	case 3:
		return 1.0;
	case 4:
		return 1.25;
	case 5:
		return 1.5;
	case 6:
		return 1.75;
	case 7:
		return 2.0;
	default:
		return 1.0;
	}
}

void OSystem_iOS7::setTimerCallback(TimerProc callback, int interval) {
	//printf("setTimerCallback()\n");
	dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
	dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);

	if (timer)
	{
		dispatch_source_set_timer(timer, dispatch_walltime(NULL, 0), interval * NSEC_PER_MSEC, interval * NSEC_PER_MSEC / 10);
		dispatch_source_set_event_handler(timer, ^{ callback(interval); });
		dispatch_resume(timer);
	}
}

Common::MutexInternal *OSystem_iOS7::createMutex() {
	return createPthreadMutexInternal();
}

void OSystem_iOS7::quit() {
}

void OSystem_iOS7::getTimeAndDate(TimeDate &td, bool skipRecord) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

Audio::Mixer *OSystem_iOS7::getMixer() {
	assert(_mixer);
	return _mixer;
}

OSystem_iOS7 *OSystem_iOS7::sharedInstance() {
	static OSystem_iOS7 *instance = new OSystem_iOS7();
	return instance;
}

Common::Path OSystem_iOS7::getDefaultConfigFileName() {
	return Common::Path("/Preferences");
}

void OSystem_iOS7::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Get URL of the Resource directory of the .app bundle
	CFURLRef fileUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (fileUrl) {
		// Try to convert the URL to an absolute path
		UInt8 buf[MAXPATHLEN];
		if (CFURLGetFileSystemRepresentation(fileUrl, true, buf, sizeof(buf))) {
			// Success: Add it to the search path
			Common::String bundlePath((const char *)buf);
			POSIXFilesystemNode *posixNode = new POSIXFilesystemNode(bundlePath);
			s.add("__IOS_BUNDLE__", new Common::FSDirectory(AbstractFSNode::makeFSNode(posixNode)), priority);
		}
		CFRelease(fileUrl);
	}
}

void iOS7_buildSharedOSystemInstance() {
	OSystem_iOS7::sharedInstance();
}

TouchMode iOS7_getCurrentTouchMode() {
	OSystem_iOS7 *sys = dynamic_cast<OSystem_iOS7 *>(g_system);
	if (!sys) {
		// If the system has not finished loading, just return a
		// default value.
		return kTouchModeDirect;
	}
	return sys->getCurrentTouchMode();
}

void iOS7_main(int argc, char **argv) {

	//OSystem_iOS7::migrateApp();

	Common::String logFilePath = iOS7_getDocumentsDir() + "/scummvm.log";
	FILE *logFile = fopen(logFilePath.c_str(), "a");
	if (logFile != nullptr) {
		// We check for log file size; if it's too big, we rewrite it.
		// This happens only upon app launch
		// NOTE: We don't check for file size each time we write a log message.
		long sz = ftell(logFile);
		if (sz > MAX_IOS7_SCUMMVM_LOG_FILESIZE_IN_BYTES) {
			fclose(logFile);
			fprintf(stdout, "Default log file is bigger than %dKB. It will be overwritten!", MAX_IOS7_SCUMMVM_LOG_FILESIZE_IN_BYTES / 1024);

			// Create the log file from scratch overwriting the previous one
			logFile = fopen(logFilePath.c_str(), "w");
			if (logFile == nullptr)
				fprintf(stdout, "Could not open default log file for rewrite!");
		}
		if (logFile != NULL) {
			fclose(stdout);
			fclose(stderr);
			*stdout = *logFile;
			*stderr = *logFile;
			setbuf(stdout, NULL);
			setbuf(stderr, NULL);
		}
	}

	chdir(iOS7_getDocumentsDir().c_str());

	g_system = OSystem_iOS7::sharedInstance();
	assert(g_system);

	// Invoke the actual ScummVM main entry point:
	scummvm_main(argc, (const char *const *) argv);
	g_system->quit();       // TODO: Consider removing / replacing this!

	if (logFile != NULL) {
		//*stdout = NULL;
		//*stderr = NULL;
		fclose(logFile);
	}
}
