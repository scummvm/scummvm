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
#include "tony/tony.h"
#include "tony/custom.h"
#include "tony/game.h"
#include "tony/mpal/mpal.h"

namespace Tony {

TonyEngine *_vm;

TonyEngine::TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc) : Engine(syst), 
		_gameDescription(gameDesc), _randomSource("tony") {
	_vm = this;
	_loadSlotNumber = -1;

	DebugMan.addDebugChannel(kTonyDebugAnimations, "animations", "Animations debugging");
	DebugMan.addDebugChannel(kTonyDebugActions, "actions", "Actions debugging");
	DebugMan.addDebugChannel(kTonyDebugSound, "sound", "Sound debugging");
	DebugMan.addDebugChannel(kTonyDebugMusic, "music", "Music debugging");

	// Set up load slot number
	_initialLoadSlotNumber = -1;
	if (ConfMan.hasKey("save_slot")) {
		int slotNumber = ConfMan.getInt("save_slot");
		if (slotNumber >= 0 && slotNumber <= 99)
			_initialLoadSlotNumber = slotNumber;
	}
}

TonyEngine::~TonyEngine() {
	// Close the voice database
	CloseVoiceDatabase();

	// Reset the coroutine scheduler
	CoroScheduler.reset();
}

/**
 * Run the game
 */
Common::Error TonyEngine::run() {
	Common::ErrorCode result = Init();
	if (result != Common::kNoError)
		return result;

	Play();
	Close();

	return Common::kNoError;
}

/**
 * Initialise the game
 */
Common::ErrorCode TonyEngine::Init() {
	m_hEndOfFrame = CoroScheduler.createEvent(false, false);

	m_bPaused = false;
	m_bDrawLocation = true;
	m_startTime = g_system->getMillis();

	// Init static class fields 
	RMText::InitStatics();
	RMTony::InitStatics();

	// Reset the scheduler
	CoroScheduler.reset();

	// Initialise the graphics window
	_window.Init();

	// Initialise the function list
	Common::fill(FuncList, FuncList + 300, (LPCUSTOMFUNCTION)NULL);
	InitCustomFunctionMap();

	// Initializes MPAL system, passing the custom functions list
	Common::File f;
	if (!f.open("ROASTED.MPC"))
		return Common::kReadingFailed;
	f.close();

	if (!mpalInit("ROASTED.MPC", "ROASTED.MPR", FuncList, FuncListStrings))
		return Common::kUnknownError;

	// Initialise the update resources
	_resUpdate.Init("ROASTED.MPU");

	// Initialise the music
	InitMusic();

	// Initialise the voices database
	if (!OpenVoiceDatabase())
		return Common::kReadingFailed;

	// Initialise the boxes
	_theBoxes.Init();

	// Link to the custom graphics engine
	_theEngine.InitCustomDll();
	_theEngine.Init();

	// Allocate space for thumbnails when saving the game
	m_curThumbnail = new uint16[160 * 120];

	// Set up global defaults
	GLOBALS.bCfgInvLocked = false;
	GLOBALS.bCfgInvNoScroll = false;
	GLOBALS.bCfgTimerizedText = true;
	GLOBALS.bCfgInvUp = false;
	GLOBALS.bCfgAnni30 = false;
	GLOBALS.bCfgAntiAlias = false;
	GLOBALS.bCfgTransparence = true;
	GLOBALS.bCfgInterTips = true;
	GLOBALS.bCfgSottotitoli = true;
	GLOBALS.nCfgTonySpeed = 3;
	GLOBALS.nCfgTextSpeed = 5;
	GLOBALS.bCfgDubbing = true;
	GLOBALS.bCfgMusic = true;
	GLOBALS.bCfgSFX = true;
	GLOBALS.nCfgDubbingVolume = 10;
	GLOBALS.nCfgMusicVolume = 7;
	GLOBALS.nCfgSFXVolume = 10;
	m_bQuitNow = false;

	return Common::kNoError;
}

void TonyEngine::InitCustomFunctionMap() {
	INIT_CUSTOM_FUNCTION(FuncList, FuncListStrings);
}

/**
 * Display an error message
 */
void TonyEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

void TonyEngine::PlayMusic(int nChannel, const char *fn, int nFX, bool bLoop, int nSync) {
	warning("TODO: TonyEngine::PlayMusic");
//	g_system->lockMutex(csMusic);

	if (nChannel < 4)
		if (GLOBALS.flipflop)
			nChannel = nChannel + 1;
	
	switch (nFX) {
	case 0:
	case 1:
	case 2:
		m_stream[nChannel]->Stop();
		m_stream[nChannel]->UnloadFile();
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
	_splitpath(path_buffer,drive,dir,NULL,NULL);
	_splitpath(fn,NULL,NULL,fname,ext);
	_makepath(path_buffer,drive,dir,fname,ext);

	_makepath(path_buffer,drive,dir,fname,ext);

	if (nFX==22) // Sync a tempo
	{
		curChannel=nChannel;		
		strcpy(nextMusic, path_buffer);
		nextLoop=bLoop;
		nextSync=nSync;
		if (flipflop)
			nextChannel=nChannel-1;
		else
		  nextChannel=nChannel+1;
		DWORD id;
		HANDLE hThread=CreateThread(NULL,10240,(LPTHREAD_START_ROUTINE)DoNextMusic,m_stream,0,&id);
		SetThreadPriority(hThread,THREAD_PRIORITY_HIGHEST);
	}
	else if (nFX==44) // Cambia canale e lascia finire il primo
	{
		if (flipflop)
			nextChannel=nChannel-1;
		else
		  nextChannel=nChannel+1;

		m_stream[nextChannel]->Stop();
		m_stream[nextChannel]->UnloadFile();

		if (!getIsDemo()) {
			if (!m_stream[nextChannel]->LoadFile(path_buffer,FPCODEC_ADPCM,nSync))
				theGame.Abort();
		} else {
			m_stream[nextChannel]->LoadFile(path_buffer,FPCODEC_ADPCM,nSync);
		}

		m_stream[nextChannel]->SetLoop(bLoop);
		m_stream[nextChannel]->Play();

		flipflop = 1-flipflop;
	}
	else
	{
		if (!getIsDemo()) {
			if (!m_stream[nChannel]->LoadFile(path_buffer,FPCODEC_ADPCM,nSync))
				theGame.Abort();
		} else {
			m_stream[nChannel]->LoadFile(path_buffer,FPCODEC_ADPCM,nSync);
		}

		m_stream[nChannel]->SetLoop(bLoop);
		m_stream[nChannel]->Play();
	}
#endif

//	g_system->unlockMutex(csMusic);
}

void TonyEngine::PlaySFX(int nChannel, int nFX) {
	if (m_sfx[nChannel] == NULL)
		return;

	switch (nFX) {
	case 0:
		m_sfx[nChannel]->SetLoop(false);
		break;

	case 1:
		m_sfx[nChannel]->SetLoop(true);
		break;
	}

	m_sfx[nChannel]->Play();
}

void TonyEngine::StopMusic(int nChannel) {
//	g_system->lockMutex(csMusic);

	if (nChannel < 4)
		m_stream[nChannel + GLOBALS.flipflop]->Stop();
	else
		m_stream[nChannel]->Stop();

//	g_system->unlockMutex(csMusic);
}

void TonyEngine::StopSFX(int nChannel) {
	m_sfx[nChannel]->Stop();
}

void TonyEngine::PlayUtilSFX(int nChannel, int nFX) {
	if (m_utilSfx[nChannel] == NULL)
		return;

	switch (nFX) {
	case 0:
		m_utilSfx[nChannel]->SetLoop(false);
		break;

	case 1:
		m_utilSfx[nChannel]->SetLoop(true);
		break;
	}

	m_utilSfx[nChannel]->SetVolume(52);
	m_utilSfx[nChannel]->Play();
}

void TonyEngine::StopUtilSFX(int nChannel) {
	m_utilSfx[nChannel]->Stop();
}

void TonyEngine::PreloadSFX(int nChannel, const char *fn) {
	if (m_sfx[nChannel] != NULL) {
		m_sfx[nChannel]->Stop();
		m_sfx[nChannel]->Release();
		m_sfx[nChannel] = NULL;
	}

	_theSound.CreateSfx(&m_sfx[nChannel]);

	m_sfx[nChannel]->LoadFile(fn, FPCODEC_ADPCM);
}

FPSFX *TonyEngine::CreateSFX(byte *buf) {
	FPSFX *sfx;
	
	_theSound.CreateSfx(&sfx);
	sfx->LoadFile(buf,FPCODEC_WAV);
	return sfx;
}

void TonyEngine::PreloadUtilSFX(int nChannel, const char *fn) {
	warning("TonyEngine::PreloadUtilSFX");
}

void TonyEngine::UnloadAllSFX(void) {
	warning("TonyEngine::UnloadAllSFX");
}

void TonyEngine::UnloadAllUtilSFX(void) {
	warning("TonyEngine::UnloadAllUtilSFX");
}

void TonyEngine::InitMusic() {
	int i;

	_theSound.Init(/*_window*/);
	_theSound.SetMasterVolume(63);
	
	for (i = 0; i < 6; i++)
		_theSound.CreateStream(&m_stream[i]);

	for (i = 0; i < MAX_SFX_CHANNELS; i++) {
		m_sfx[i] = m_utilSfx[i] = NULL;
	}

	// Create the mutex for controlling music access
//	csMusic = g_system->createMutex();

	// Preload sound effects
	PreloadUtilSFX(0, "U01.ADP"); // Reversed!!
	PreloadUtilSFX(1, "U02.ADP");
}

void TonyEngine::CloseMusic() {
	for (int i = 0; i < 6; i++) {
		m_stream[i]->Stop();
		m_stream[i]->UnloadFile();
		m_stream[i]->Release();
	}

//	g_system->deleteMutex(csMusic);

	UnloadAllSFX();
	UnloadAllUtilSFX();
}

void TonyEngine::PauseSound(bool bPause) {
}

void TonyEngine::SetMusicVolume(int nChannel, int volume) {
}

int TonyEngine::GetMusicVolume(int nChannel) {
	return 255;
}


Common::String TonyEngine::GetSaveStateFileName(int n) {
	return Common::String::format("tony.%03d", n);
}

void TonyEngine::AutoSave(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
		Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GrabThumbnail();
	CORO_INVOKE_0(MainWaitFrame);
	CORO_INVOKE_0(MainWaitFrame);
	MainFreeze();
	_ctx->buf = GetSaveStateFileName(0);
	_theEngine.SaveState(_ctx->buf, (byte *)m_curThumbnail, "Autosave");
	MainUnfreeze();

	CORO_END_CODE;
}


void TonyEngine::SaveState(int n, const char *name) {
	Common::String buf = GetSaveStateFileName(n);
	_theEngine.SaveState(buf.c_str(), (byte *)m_curThumbnail, name);
}


void TonyEngine::LoadState(CORO_PARAM, int n) {
	CORO_BEGIN_CONTEXT;
		Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->buf = GetSaveStateFileName(n);
	CORO_INVOKE_1(_theEngine.LoadState, _ctx->buf.c_str());

	CORO_END_CODE;
}

bool TonyEngine::OpenVoiceDatabase() {
	char id[4];
	uint32 numfiles;

	// Add the voices folder to the search directory list
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "voices");

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
		vh.offset = _vdbFP.readUint32LE();
		vh.code = _vdbFP.readUint32LE();
		vh.parts = _vdbFP.readUint32LE();

		_voices.push_back(vh);
	}

	return true;
}

void TonyEngine::CloseVoiceDatabase() {
	if (_vdbFP.isOpen())
		_vdbFP.close();

	if (_voices.size() > 0)
		_voices.clear();
}

void TonyEngine::GrabThumbnail(void) {
	_window.GrabThumbnail(m_curThumbnail);
}

void TonyEngine::OptionScreen(void) {
}

void TonyEngine::OpenInitLoadMenu(CORO_PARAM) {
	_theEngine.OpenOptionScreen(coroParam, 1);
}

void TonyEngine::OpenInitOptions(CORO_PARAM) {
	_theEngine.OpenOptionScreen(coroParam, 2);
}

void TonyEngine::Abort(void) {
	m_bQuitNow = true;
}

/**
 * Main process for playing the game.
 *
 * @remarks		This needs to be in a separate process, since there are some things that can briefly
 * block the execution of process. For now, all ScummVm event handling is dispatched to within the context of this
 * process. If it ever proves a problem, we may have to look into whether it's feasible to have it still remain
 * in the outer 'main' process.
 */
void TonyEngine::PlayProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		Common::String fn;
	CORO_END_CONTEXT(_ctx);


	CORO_BEGIN_CODE(_ctx);

	// Game loop. We rely on the outer main process to detect if a shutdown is required,
	// and kill the scheudler and all the processes, including this one
	for (;;) {
		// Se siamo in pausa, entra nel loop appropriato
		if (_vm->m_bPaused)
			_vm->PauseLoop();

		// If a savegame needs to be loaded, then do so
		if (_vm->_loadSlotNumber != -1 && GLOBALS.GfxEngine != NULL) {
			_ctx->fn = GetSaveStateFileName(_vm->_loadSlotNumber);
			CORO_INVOKE_1(GLOBALS.GfxEngine->LoadState, _ctx->fn);
			_vm->_loadSlotNumber = -1;
		}

		// Wait for the next frame
		CORO_INVOKE_1(CoroScheduler.sleep, 50);

  		// Call the engine to handle the next frame
		CORO_INVOKE_1(_vm->_theEngine.DoFrame, _vm->m_bDrawLocation);

		// Warns that a frame is finished
		CoroScheduler.pulseEvent(_vm->m_hEndOfFrame);

		// Handle drawing the frame
		if (!_vm->m_bPaused) {
			if (!_vm->_theEngine.m_bWiping)
				_vm->_window.GetNewFrame(_vm->_theEngine, NULL);
			else
				_vm->_window.GetNewFrame(_vm->_theEngine, &_vm->_theEngine.m_rcWipeEllipse);
		}

		// Paint the frame onto the screen
		_vm->_window.Repaint();
	}

	CORO_END_CODE;
}

/**
 * Play the game
 */
void TonyEngine::Play(void) {
	// Create the game player process
	CoroScheduler.createProcess(PlayProcess, NULL);

	// Loop through calling the scheduler until it's time for the game to quit
	while (!shouldQuit() && !m_bQuitNow) {
		// Delay for a brief amount
		g_system->delayMillis(10);

		// Call any scheduled processes
		CoroScheduler.schedule();
	}
}



void TonyEngine::Close(void) {
	CloseMusic();
	CoroScheduler.closeEvent(m_hEndOfFrame);
	_theBoxes.Close();
	_theEngine.Close();
	_window.Close();
	delete[] m_curThumbnail;
}

void TonyEngine::SwitchFullscreen(bool bFull) {
	_window.SwitchFullscreen(bFull);
	_theEngine.SwitchFullscreen(bFull);
}

void TonyEngine::GDIControl(bool bCon) {
	_theEngine.GDIControl(bCon);
}


void TonyEngine::PauseLoop(void) {
	warning("TODO: TonyEngine::PauseLoop");

#if 0
	MSG msg;
	int st,et;

	st = GetTime();

	while (m_bPaused && GetMessage(&msg,_window,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	et = GetTime();

	m_startTime += et - st;
#endif
}

void TonyEngine::Pause(bool bPause) {
	// If the new status already matches the current one, do nothing
	if (m_bPaused == bPause)
		return;

warning("TODO: TonyEninge::Pause");
/*
	m_bPaused = bPause;										
	theEngine.GDIControl(m_bPaused);

	if (m_bPaused) {
		SetWindowText(_window, "Tony Tough and the night of Roasted Moths - PAUSED");
	} else {
		SetWindowText(_window, "Tony Tough and the night of Roasted Moths");
	}
*/
}

void TonyEngine::FreezeTime(void) {
	m_bTimeFreezed = true;
	m_nTimeFreezed = GetTime() - m_startTime;
}

void TonyEngine::UnfreezeTime(void) {
	m_bTimeFreezed = false;
}


/**
 * Returns the millisecond timer
 */
uint32 TonyEngine::GetTime() {
	return g_system->getMillis();
}

bool TonyEngine::canLoadGameStateCurrently() {
	return GLOBALS.GfxEngine != NULL && GLOBALS.GfxEngine->CanLoadSave();
}
bool TonyEngine::canSaveGameStateCurrently() {
	return GLOBALS.GfxEngine != NULL && GLOBALS.GfxEngine->CanLoadSave();
}

Common::Error TonyEngine::loadGameState(int slot) {
	_loadSlotNumber = slot;
	return Common::kNoError;
}

Common::Error TonyEngine::saveGameState(int slot, const Common::String &desc) {
	if (!GLOBALS.GfxEngine)
		return Common::kUnknownError;

	RMSnapshot s;
	s.GrabScreenshot(*GLOBALS.GfxEngine, 4, m_curThumbnail);

	GLOBALS.GfxEngine->SaveState(GetSaveStateFileName(slot), (byte *)m_curThumbnail, desc);
	return Common::kNoError;
}

} // End of namespace Tony
