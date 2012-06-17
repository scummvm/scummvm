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

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/installshield_cab.h"
#include "tony/tony.h"
#include "tony/custom.h"
#include "tony/debugger.h"
#include "tony/game.h"
#include "tony/mpal/mpal.h"

namespace Tony {

TonyEngine *_vm;

TonyEngine::TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("tony") {
	_vm = this;
	_loadSlotNumber = -1;

	// Set the up the debugger
	_debugger = new Debugger();
	DebugMan.addDebugChannel(kTonyDebugAnimations, "animations", "Animations debugging");
	DebugMan.addDebugChannel(kTonyDebugActions, "actions", "Actions debugging");
	DebugMan.addDebugChannel(kTonyDebugSound, "sound", "Sound debugging");
	DebugMan.addDebugChannel(kTonyDebugMusic, "music", "Music debugging");

	// Add folders to the search directory list
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Roasted");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Music");

	// Set up load slot number
	_initialLoadSlotNumber = -1;
	if (ConfMan.hasKey("save_slot")) {
		int slotNumber = ConfMan.getInt("save_slot");
		if (slotNumber >= 0 && slotNumber <= 99)
			_initialLoadSlotNumber = slotNumber;
	}

	// Load the ScummVM sound settings
	syncSoundSettings();

	_hEndOfFrame = 0;
	for (int i = 0; i < 6; i++)
		_stream[i] = NULL;
	for (int i = 0; i < MAX_SFX_CHANNELS; i++) {
		_sfx[i] = NULL;
		_utilSfx[i] = NULL;
	}
	_bPaused = false;
	_bDrawLocation = false;
	_startTime = 0;
	_curThumbnail = NULL;
	_bQuitNow = false;
	_bTimeFreezed = false;
	_nTimeFreezed = 0;
}

TonyEngine::~TonyEngine() {
	// Close the voice database
	closeVoiceDatabase();

	// Reset the coroutine scheduler
	CoroScheduler.reset();

	delete _debugger;
}

/**
 * Run the game
 */
Common::Error TonyEngine::run() {
	Common::ErrorCode result = init();
	if (result != Common::kNoError)
		return result;

	play();
	close();

	return Common::kNoError;
}

/**
 * Initialise the game
 */
Common::ErrorCode TonyEngine::init() {
	if (isCompressed()) {
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("data1.cab");
		if (!stream)
			error("Failed to open data1.cab");

		Common::Archive *cabinet = Common::makeInstallShieldArchive(stream);
		if (!cabinet)
			error("Failed to parse data1.cab");

		SearchMan.add("data1.cab", cabinet);
	}

	_hEndOfFrame = CoroScheduler.createEvent(false, false);

	_bPaused = false;
	_bDrawLocation = true;
	_startTime = g_system->getMillis();

	// Init static class fields
	RMText::initStatics();
	RMTony::initStatics();

	// Reset the scheduler
	CoroScheduler.reset();

	// Initialise the graphics window
	_window.init();

	// Initialise the function list
	Common::fill(_funcList, _funcList + 300, (LPCUSTOMFUNCTION)NULL);
	initCustomFunctionMap();

	// Initializes MPAL system, passing the custom functions list
	Common::File f;
	if (!f.open("ROASTED.MPC"))
		return Common::kReadingFailed;
	f.close();

	if (!mpalInit("ROASTED.MPC", "ROASTED.MPR", _funcList, _funcListStrings))
		return Common::kUnknownError;

	// Initialise the update resources
	_resUpdate.init("ROASTED.MPU");

	// Initialise the music
	initMusic();

	// Initialise the voices database
	if (!openVoiceDatabase())
		return Common::kReadingFailed;

	// Initialise the boxes
	_theBoxes.init();

	// Link to the custom graphics engine
	_theEngine.initCustomDll();
	_theEngine.init();

	// Allocate space for thumbnails when saving the game
	_curThumbnail = new uint16[160 * 120];

	_bQuitNow = false;

	return Common::kNoError;
}

void TonyEngine::initCustomFunctionMap() {
	INIT_CUSTOM_FUNCTION(_funcList, _funcListStrings);
}

/**
 * Display an error message
 */
void TonyEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

void TonyEngine::playMusic(int nChannel, const char *fn, int nFX, bool bLoop, int nSync) {
	warning("TODO: TonyEngine::playMusic");
//	g_system->lockMutex(csMusic);

	if (nChannel < 4)
		if (GLOBALS._flipflop)
			nChannel = nChannel + 1;

	switch (nFX) {
	case 0:
	case 1:
	case 2:
		_stream[nChannel]->stop();
		_stream[nChannel]->unloadFile();
		break;

	case 22:
		break;
	}

#ifdef REFACTOR_ME
	// Mette il path giusto
	if (nChannel < 4)
		GetDataDirectory(DD_MUSIC, path_buffer);
	else
		GetDataDirectory(DD_LAYER, path_buffer);
	_splitpath(path_buffer, drive, dir, NULL, NULL);
	_splitpath(fn, NULL, NULL, fname, ext);
	_makepath(path_buffer, drive, dir, fname, ext);

	_makepath(path_buffer, drive, dir, fname, ext);

	if (nFX == 22) { // Sync a tempo
		curChannel = nChannel;
		strcpy(nextMusic, path_buffer);
		nextLoop = bLoop;
		nextSync = nSync;
		if (flipflop)
			nextChannel = nChannel - 1;
		else
			nextChannel = nChannel + 1;
		DWORD id;
		HANDLE hThread = CreateThread(NULL, 10240, (LPTHREAD_START_ROUTINE)DoNextMusic, _stream, 0, &id);
		SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
	} else if (nFX == 44) { // Cambia canale e lascia finire il primo
		if (flipflop)
			nextChannel = nChannel - 1;
		else
			nextChannel = nChannel + 1;

		_stream[nextChannel]->Stop();
		_stream[nextChannel]->UnloadFile();

		if (!getIsDemo()) {
			if (!_stream[nextChannel]->LoadFile(path_buffer, FPCODEC_ADPCM, nSync))
				theGame.Abort();
		} else {
			_stream[nextChannel]->LoadFile(path_buffer, FPCODEC_ADPCM, nSync);
		}

		_stream[nextChannel]->SetLoop(bLoop);
		_stream[nextChannel]->Play();

		flipflop = 1 - flipflop;
	} else {
		if (!getIsDemo()) {
			if (!_stream[nChannel]->LoadFile(path_buffer, FPCODEC_ADPCM, nSync))
				theGame.Abort();
		} else {
			_stream[nChannel]->LoadFile(path_buffer, FPCODEC_ADPCM, nSync);
		}

		_stream[nChannel]->SetLoop(bLoop);
		_stream[nChannel]->Play();
	}
#endif

//	g_system->unlockMutex(csMusic);
}

void TonyEngine::playSFX(int nChannel, int nFX) {
	if (_sfx[nChannel] == NULL)
		return;

	switch (nFX) {
	case 0:
		_sfx[nChannel]->setLoop(false);
		break;

	case 1:
		_sfx[nChannel]->setLoop(true);
		break;
	}

	_sfx[nChannel]->play();
}

void TonyEngine::stopMusic(int nChannel) {
//	g_system->lockMutex(csMusic);

	if (nChannel < 4)
		_stream[nChannel + GLOBALS._flipflop]->stop();
	else
		_stream[nChannel]->stop();

//	g_system->unlockMutex(csMusic);
}

void TonyEngine::stopSFX(int nChannel) {
	_sfx[nChannel]->stop();
}

void TonyEngine::playUtilSFX(int nChannel, int nFX) {
	if (_utilSfx[nChannel] == NULL)
		return;

	switch (nFX) {
	case 0:
		_utilSfx[nChannel]->setLoop(false);
		break;

	case 1:
		_utilSfx[nChannel]->setLoop(true);
		break;
	}

	_utilSfx[nChannel]->setVolume(52);
	_utilSfx[nChannel]->play();
}

void TonyEngine::stopUtilSFX(int nChannel) {
	_utilSfx[nChannel]->stop();
}

void TonyEngine::preloadSFX(int nChannel, const char *fn) {
	if (_sfx[nChannel] != NULL) {
		_sfx[nChannel]->stop();
		_sfx[nChannel]->release();
		_sfx[nChannel] = NULL;
	}

	_theSound.createSfx(&_sfx[nChannel]);

	_sfx[nChannel]->loadFile(fn, FPCODEC_ADPCM);
}

FPSfx *TonyEngine::createSFX(Common::SeekableReadStream *stream) {
	FPSfx *sfx;

	_theSound.createSfx(&sfx);
	sfx->loadWave(stream);
	return sfx;
}

void TonyEngine::preloadUtilSFX(int nChannel, const char *fn) {
	warning("TonyEngine::preloadUtilSFX");
}

void TonyEngine::unloadAllSFX(void) {
	warning("TonyEngine::unloadAllSFX");
}

void TonyEngine::unloadAllUtilSFX(void) {
	warning("TonyEngine::unloadAllUtilSFX");
}

void TonyEngine::initMusic() {
	int i;

	_theSound.init(/*_window*/);
	_theSound.setMasterVolume(63);

	for (i = 0; i < 6; i++)
		_theSound.createStream(&_stream[i]);

	for (i = 0; i < MAX_SFX_CHANNELS; i++) {
		_sfx[i] = _utilSfx[i] = NULL;
	}

	// Create the mutex for controlling music access
//	csMusic = g_system->createMutex();

	// Preload sound effects
	preloadUtilSFX(0, "U01.ADP"); // Reversed!!
	preloadUtilSFX(1, "U02.ADP");

	// Start check processes for sound
	CoroScheduler.createProcess(FPSfx::soundCheckProcess, NULL);
}

void TonyEngine::closeMusic() {
	for (int i = 0; i < 6; i++) {
		_stream[i]->stop();
		_stream[i]->unloadFile();
		_stream[i]->release();
	}

//	g_system->deleteMutex(csMusic);

	unloadAllSFX();
	unloadAllUtilSFX();
}

void TonyEngine::pauseSound(bool bPause) {
}

void TonyEngine::setMusicVolume(int nChannel, int volume) {
}

int TonyEngine::getMusicVolume(int nChannel) {
	return 255;
}

Common::String TonyEngine::getSaveStateFileName(int n) {
	return Common::String::format("tony.%03d", n);
}

void TonyEngine::autoSave(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	grabThumbnail();
	CORO_INVOKE_0(mainWaitFrame);
	CORO_INVOKE_0(mainWaitFrame);
	mainFreeze();
	_ctx->buf = getSaveStateFileName(0);
	_theEngine.saveState(_ctx->buf, (byte *)_curThumbnail, "Autosave");
	mainUnfreeze();

	CORO_END_CODE;
}


void TonyEngine::saveState(int n, const char *name) {
	Common::String buf = getSaveStateFileName(n);
	_theEngine.saveState(buf.c_str(), (byte *)_curThumbnail, name);
}


void TonyEngine::loadState(CORO_PARAM, int n) {
	CORO_BEGIN_CONTEXT;
	Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->buf = getSaveStateFileName(n);
	CORO_INVOKE_1(_theEngine.loadState, _ctx->buf.c_str());

	CORO_END_CODE;
}

bool TonyEngine::openVoiceDatabase() {
	char id[4];
	uint32 numfiles;

	// Open the voices database
	if (!_vdbFP.open("voices.vdb"))
		return false;

	_vdbFP.seek(-8, SEEK_END);
	numfiles = _vdbFP.readUint32LE();
	_vdbFP.read(id, 4);

	if (id[0] != 'V' || id[1] != 'D' || id[2] != 'B' || id[3] != '1') {
		_vdbFP.close();
		return false;
	}

	// Read in the index
	_vdbFP.seek(-8 - (numfiles * VOICE_HEADER_SIZE), SEEK_END);

	for (uint32 i = 0; i < numfiles; ++i) {
		VoiceHeader vh;
		vh._offset = _vdbFP.readUint32LE();
		vh._code = _vdbFP.readUint32LE();
		vh._parts = _vdbFP.readUint32LE();

		_voices.push_back(vh);
	}

	return true;
}

void TonyEngine::closeVoiceDatabase() {
	if (_vdbFP.isOpen())
		_vdbFP.close();

	if (_voices.size() > 0)
		_voices.clear();
}

void TonyEngine::grabThumbnail(void) {
	_window.grabThumbnail(_curThumbnail);
}

void TonyEngine::optionScreen(void) {
}

void TonyEngine::openInitLoadMenu(CORO_PARAM) {
	_theEngine.openOptionScreen(coroParam, 1);
}

void TonyEngine::openInitOptions(CORO_PARAM) {
	_theEngine.openOptionScreen(coroParam, 2);
}

void TonyEngine::abortGame(void) {
	_bQuitNow = true;
}

/**
 * Main process for playing the game.
 *
 * @remarks     This needs to be in a separate process, since there are some things that can briefly
 * block the execution of process. For now, all ScummVm event handling is dispatched to within the context of this
 * process. If it ever proves a problem, we may have to look into whether it's feasible to have it still remain
 * in the outer 'main' process.
 */
void TonyEngine::playProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	Common::String fn;
	CORO_END_CONTEXT(_ctx);


	CORO_BEGIN_CODE(_ctx);

	// Game loop. We rely on the outer main process to detect if a shutdown is required,
	// and kill the scheudler and all the processes, including this one
	for (;;) {
		// If a savegame needs to be loaded, then do so
		if (_vm->_loadSlotNumber != -1 && GLOBALS._gfxEngine != NULL) {
			_ctx->fn = getSaveStateFileName(_vm->_loadSlotNumber);
			CORO_INVOKE_1(GLOBALS._gfxEngine->loadState, _ctx->fn);
			_vm->_loadSlotNumber = -1;
		}

		// Wait for the next frame
		CORO_INVOKE_1(CoroScheduler.sleep, 50);

		// Call the engine to handle the next frame
		CORO_INVOKE_1(_vm->_theEngine.doFrame, _vm->_bDrawLocation);

		// Warns that a frame is finished
		CoroScheduler.pulseEvent(_vm->_hEndOfFrame);

		// Handle drawing the frame
		if (!_vm->_bPaused) {
			if (!_vm->_theEngine._bWiping)
				_vm->_window.getNewFrame(_vm->_theEngine, NULL);
			else
				_vm->_window.getNewFrame(_vm->_theEngine, &_vm->_theEngine._rcWipeEllipse);
		}

		// Paint the frame onto the screen
		_vm->_window.repaint();

		// Signal the ScummVM debugger
		_vm->_debugger->onFrame();
	}

	CORO_END_CODE;
}

/**
 * Play the game
 */
void TonyEngine::play(void) {
	// Create the game player process
	CoroScheduler.createProcess(playProcess, NULL);

	// Loop through calling the scheduler until it's time for the game to quit
	while (!shouldQuit() && !_bQuitNow) {
		// Delay for a brief amount
		g_system->delayMillis(10);

		// Call any scheduled processes
		CoroScheduler.schedule();
	}
}

void TonyEngine::close(void) {
	closeMusic();
	CoroScheduler.closeEvent(_hEndOfFrame);
	_theBoxes.close();
	_theEngine.close();
	_window.close();
	mpalFree();
	FreeMpc();
	delete[] _curThumbnail;
}

void TonyEngine::switchFullscreen(bool bFull) {
	_window.switchFullscreen(bFull);
	_theEngine.switchFullscreen(bFull);
}

void TonyEngine::GDIControl(bool bCon) {
	_theEngine.GDIControl(bCon);
}

void TonyEngine::freezeTime(void) {
	_bTimeFreezed = true;
	_nTimeFreezed = getTime() - _startTime;
}

void TonyEngine::unfreezeTime(void) {
	_bTimeFreezed = false;
}


/**
 * Returns the millisecond timer
 */
uint32 TonyEngine::getTime() {
	return g_system->getMillis();
}

bool TonyEngine::canLoadGameStateCurrently() {
	return GLOBALS._gfxEngine != NULL && GLOBALS._gfxEngine->canLoadSave();
}
bool TonyEngine::canSaveGameStateCurrently() {
	return GLOBALS._gfxEngine != NULL && GLOBALS._gfxEngine->canLoadSave();
}

Common::Error TonyEngine::loadGameState(int slot) {
	_loadSlotNumber = slot;
	return Common::kNoError;
}

Common::Error TonyEngine::saveGameState(int slot, const Common::String &desc) {
	if (!GLOBALS._gfxEngine)
		return Common::kUnknownError;

	RMSnapshot s;
	s.grabScreenshot(*GLOBALS._gfxEngine, 4, _curThumbnail);

	GLOBALS._gfxEngine->saveState(getSaveStateFileName(slot), (byte *)_curThumbnail, desc);
	return Common::kNoError;
}

void TonyEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	GLOBALS._bCfgDubbing = !ConfMan.getBool("mute") && !ConfMan.getBool("speech_mute");
	GLOBALS._bCfgSFX = !ConfMan.getBool("mute") && !ConfMan.getBool("sfx_mute");
	GLOBALS._bCfgMusic = !ConfMan.getBool("mute") && !ConfMan.getBool("music_mute");

	GLOBALS._nCfgDubbingVolume = ConfMan.getInt("speech_volume") * 10 / 256;
	GLOBALS._nCfgSFXVolume = ConfMan.getInt("sfx_volume") * 10 / 256;
	GLOBALS._nCfgMusicVolume = ConfMan.getInt("music_volume") * 10 / 256;

	GLOBALS._bShowSubtitles = ConfMan.getBool("subtitles");
	GLOBALS._nCfgTextSpeed = ConfMan.getInt("talkspeed") * 10 / 256;
}

void TonyEngine::saveSoundSettings() {
	ConfMan.setBool("speech_mute", GLOBALS._bCfgDubbing);
	ConfMan.setBool("sfx_mute", GLOBALS._bCfgSFX);
	ConfMan.setBool("music_mute", GLOBALS._bCfgMusic);

	ConfMan.setInt("speech_volume", GLOBALS._nCfgDubbingVolume * 256 / 10);
	ConfMan.setInt("sfx_volume", GLOBALS._nCfgSFXVolume * 256 / 10);
	ConfMan.setInt("music_volume", GLOBALS._nCfgMusicVolume * 256 / 10);

	ConfMan.setBool("subtitles", GLOBALS._bShowSubtitles);
	ConfMan.setBool("talkspeed", GLOBALS._nCfgTextSpeed);
}

} // End of namespace Tony
