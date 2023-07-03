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

#include "base/main.h"

#include "engines/engine.h"
#include "engines/metaengine.h"

#include "gui/gui-manager.h"

#include "backends/graphics/ios/ios-graphics.h"
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
	Common::String _sandboxRootPath;
public:

	SandboxedSaveFileManager(Common::String sandboxRootPath, Common::String defaultSavepath)
			: DefaultSaveFileManager(defaultSavepath), _sandboxRootPath(sandboxRootPath) {
	}

	bool removeSavefile(const Common::String &filename) override {
		Common::String chrootedFile = getSavePath() + "/" + filename;
		Common::String realFilePath = _sandboxRootPath + chrootedFile;

		if (remove(realFilePath.c_str()) != 0) {
			if (errno == EACCES)
				setError(Common::kWritePermissionDenied, "Search or write permission denied: "+chrootedFile);

			if (errno == ENOENT)
				setError(Common::kPathDoesNotExist, "removeSavefile: '"+chrootedFile+"' does not exist or path is invalid");
			return false;
		} else {
			return true;
		}
	}
};

OSystem_iOS7::OSystem_iOS7() :
	_mixer(NULL), _lastMouseTap(0), _queuedEventTime(0),
	_mouseNeedTextureUpdate(false), _secondaryTapped(false), _lastSecondaryTap(0),
	_screenOrientation(kScreenOrientationFlippedLandscape),
	_fullScreenIsDirty(false), _fullScreenOverlayIsDirty(false),
	_mouseDirty(false), _timeSuspended(0), _lastDragPosX(-1), _lastDragPosY(-1), _screenChangeCount(0),
	_mouseCursorPaletteEnabled(false), _gfxTransactionError(kTransactionSuccess) {
	_queuedInputEvent.type = Common::EVENT_INVALID;
	_touchpadModeEnabled = ConfMan.getBool("touchpad_mode");
	_mouseClickAndDragEnabled = ConfMan.getBool("clickanddrag_mode");

	_chrootBasePath = iOS7_getDocumentsDir();
	ChRootFilesystemFactory *chFsFactory = new ChRootFilesystemFactory(_chrootBasePath);
	_fsFactory = chFsFactory;
	// Add virtual drive for bundle path
	Common::String appBubdlePath = iOS7_getAppBundleDir();
	if (!appBubdlePath.empty())
		chFsFactory->addVirtualDrive("appbundle:", appBubdlePath);

	initVideoContext();

	memset(_gamePalette, 0, sizeof(_gamePalette));
	memset(_gamePaletteRGBA5551, 0, sizeof(_gamePaletteRGBA5551));
	memset(_mouseCursorPalette, 0, sizeof(_mouseCursorPalette));
}

OSystem_iOS7::~OSystem_iOS7() {
	AudioQueueDispose(s_AudioQueue.queue, true);

	delete _mixer;
	// Prevent accidental freeing of the screen texture here. This needs to be
	// checked since we might use the screen texture as framebuffer in the case
	// of hi-color games for example. Otherwise this can lead to a double free.
	if (_framebuffer.getPixels() != _videoContext->screenTexture.getPixels())
		_framebuffer.free();
	_mouseBuffer.free();
	delete _graphicsManager;
}

bool OSystem_iOS7::touchpadModeEnabled() const {
	return _touchpadModeEnabled;
}

#if defined(USE_OPENGL) && defined(USE_GLAD)
void *OSystem_iOS7::getOpenGLProcAddress(const char *name) const {
	return dlsym(RTLD_SELF, name);
}
#endif

int OSystem_iOS7::timerHandler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_iOS7::initBackend() {
	_savefileManager = new SandboxedSaveFileManager(_chrootBasePath, "/Savegames");

	_timerManager = new DefaultTimerManager();

	_startTime = CACurrentMediaTime();

	_graphicsManager = new iOSGraphicsManager();

	setupMixer();

	setTimerCallback(&OSystem_iOS7::timerHandler, 10);

	EventsBaseBackend::initBackend();
}

bool OSystem_iOS7::hasFeature(Feature f) {
	switch (f) {
	case kFeatureCursorPalette:
	case kFeatureFilteringMode:
	case kFeatureVirtualKeyboard:
#if TARGET_OS_IOS
	case kFeatureClipboardSupport:
#endif
	case kFeatureOpenUrl:
	case kFeatureNoQuit:
	case kFeatureKbdMouseSpeed:
		return true;

	default:
		return ModularGraphicsBackend::hasFeature(f);
	}
}

void OSystem_iOS7::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureCursorPalette:
		if (_mouseCursorPaletteEnabled != enable) {
			_mouseNeedTextureUpdate = true;
			_mouseDirty = true;
			_mouseCursorPaletteEnabled = enable;
		}
		break;
	case kFeatureFilteringMode:
		_videoContext->filtering = enable;
		break;
	case kFeatureAspectRatioCorrection:
		_videoContext->asprectRatioCorrection = enable;
		break;
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
	case kFeatureCursorPalette:
		return _mouseCursorPaletteEnabled;
	case kFeatureFilteringMode:
		return _videoContext->filtering;
	case kFeatureAspectRatioCorrection:
		return _videoContext->asprectRatioCorrection;
	case kFeatureVirtualKeyboard:
		return isKeyboardShown();

	default:
		return ModularGraphicsBackend::getFeatureState(f);
	}
}

void OSystem_iOS7::suspendLoop() {
	bool done = false;
	uint32 startTime = getMillis();

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

	_timeSuspended += getMillis() - startTime;
}

void OSystem_iOS7::saveState() {
	// Clear any previous restore state to avoid having and obsolete one if we don't save it again below.
	clearState();

	// If there is an engine running and it accepts autosave, do an autosave and add the current
	// running target to the config file.
	if (g_engine && g_engine->hasFeature(Engine::kSupportsSavingDuringRuntime) && g_engine->canSaveAutosaveCurrently()) {
		Common::String targetName(ConfMan.getActiveDomainName());
		int saveSlot = g_engine->getAutosaveSlot();
		// Make sure we do not overwrite a user save
		SaveStateDescriptor desc = g_engine->getMetaEngine()->querySaveMetaInfos(targetName.c_str(), saveSlot);
		if (desc.getSaveSlot() != -1 && !desc.isAutosave())
			return;

		// Do the auto-save, and if successful store this it in the config
		if (g_engine->saveGameState(saveSlot, _("Autosave"), true).getCode() == Common::kNoError) {
			ConfMan.set("restore_target", targetName, Common::ConfigManager::kApplicationDomain);
			ConfMan.setInt("restore_slot", saveSlot, Common::ConfigManager::kApplicationDomain);
			ConfMan.flushToDisk();
		}
	}
}

void OSystem_iOS7::restoreState() {
	Common::String target;
	int slot = -1;
	if (ConfMan.hasKey("restore_target", Common::ConfigManager::kApplicationDomain) &&
		ConfMan.hasKey("restore_slot", Common::ConfigManager::kApplicationDomain)) {
		target = ConfMan.get("restore_target", Common::ConfigManager::kApplicationDomain);
		slot = ConfMan.getInt("restore_slot", Common::ConfigManager::kApplicationDomain);
		clearState();
	}

	// If the g_engine is still running (i.e. the application was not terminated) we don't need to do anything.
	if (g_engine)
		return;

	if (!target.empty() && slot != -1) {
		ConfMan.setInt("save_slot", slot, Common::ConfigManager::kTransientDomain);
		ConfMan.setActiveDomain(target);
		if (GUI::GuiManager::hasInstance())
			g_gui.exitLoop();
	}
}

void OSystem_iOS7::clearState() {
	if (ConfMan.hasKey("restore_target", Common::ConfigManager::kApplicationDomain) &&
	ConfMan.hasKey("restore_slot", Common::ConfigManager::kApplicationDomain)) {
		ConfMan.removeKey("restore_target", Common::ConfigManager::kApplicationDomain);
		ConfMan.removeKey("restore_slot", Common::ConfigManager::kApplicationDomain);
		ConfMan.flushToDisk();
	}
}

uint32 OSystem_iOS7::getMillis(bool skipRecord) {
	CFTimeInterval timeInSeconds = CACurrentMediaTime();
	return (uint32) ((timeInSeconds - _startTime) * 1000.0) - _timeSuspended;
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

Common::String OSystem_iOS7::getDefaultConfigFileName() {
	Common::String path = "/Preferences";
	return path;
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

bool iOS7_touchpadModeEnabled() {
	OSystem_iOS7 *sys = dynamic_cast<OSystem_iOS7 *>(g_system);
	return sys && sys->touchpadModeEnabled();
}

void iOS7_buildSharedOSystemInstance() {
	OSystem_iOS7::sharedInstance();
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
