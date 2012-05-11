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

#ifndef TONY_H
#define TONY_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/array.h"
#include "common/coroutines.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"

#include "tony/mpal/mpal.h"
#include "tony/mpal/memory.h"
#include "tony/gfxengine.h"
#include "tony/loc.h"
#include "tony/utils.h"
#include "tony/window.h"

/**
 * This is the namespace of the Tony engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Tony Tough
 */
namespace Tony {

using namespace MPAL;

enum {
	kTonyDebugAnimations = 1 << 0,
	kTonyDebugActions = 1 << 1,
	kTonyDebugSound = 1 << 2,
	kTonyDebugMusic = 2 << 3
};

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

struct TonyGameDescription;

#define MAX_SFX_CHANNELS	32

struct VoiceHeader {
	int offset;
	int code;
	int parts;
};
#define VOICE_HEADER_SIZE 12

class TonyEngine : public Engine {
private:
	Common::ErrorCode Init();

	void InitMusic();
	void CloseMusic();
	bool OpenVoiceDatabase();
	void CloseVoiceDatabase();
	void InitCustomFunctionMap();
	static void PlayProcess(CORO_PARAM, const void *param);
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	LPCUSTOMFUNCTION FuncList[300];
	Common::String FuncListStrings[300];
	Common::RandomSource _randomSource;
	MPAL::MemoryManager _memoryManager;
	RMResUpdate _resUpdate;
	uint32 m_hEndOfFrame;
	Common::File _vdbFP;
	Common::Array<VoiceHeader> _voices;
	FPSOUND	_theSound;

	enum DATADIR {
		DD_BASE = 1,
		DD_SAVE,
		DD_SHOTS,
		DD_MUSIC,
		DD_LAYER,
		DD_UTILSFX,
		DD_VOICES,
		DD_BASE2
	};

	FPSTREAM *m_stream[6];
	FPSFX *m_sfx[MAX_SFX_CHANNELS];
	FPSFX *m_utilSfx[MAX_SFX_CHANNELS];
	RMFont *fonts[2];
	bool m_bPaused;
	bool m_bDrawLocation;
	int m_startTime;
	uint16 *m_curThumbnail;

	// Bounding box list manager
	RMGameBoxes _theBoxes;
	RMWindow _window;
	RMGfxEngine _theEngine;

	bool m_bQuitNow;
	bool m_bTimeFreezed;
	int m_nTimeFreezed;
public:
	TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc);
	virtual ~TonyEngine();

	const TonyGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	uint16 getVersion() const;
	bool getIsDemo() const;
	RMGfxEngine *GetEngine() { return &_theEngine; }
	void GUIError(const Common::String &msg);

	// Avverte che siamo guidati dal GDI
	void GDIControl(bool bCon);

	// Loop che gestisce i messaggi quando siamo in pausa
	void PauseLoop(void);

	void Play();
	void Close();
	void Abort();

	void GetDataDirectory(DATADIR dir, char *path);

	void SwitchFullscreen(bool bFull);
	void OptionScreen(void);

	void ShowLocation(void) { m_bDrawLocation = true; }
	void HideLocation(void) { m_bDrawLocation = false; }

	// Mette o leva la pausa
	void Pause(bool bPause);
	bool IsPaused() { return m_bPaused; }

	// Reads the time
	uint32 GetTime(void);
	void FreezeTime(void);
	void UnfreezeTime(void);

	// Music
	// ******
	void PlayMusic(int nChannel, const char *fn, int nFX, bool bLoop, int nSync);
	void StopMusic(int nChannel);

	void PlaySFX(int nSfx, int nFX = 0);
	void StopSFX(int nSfx);

	void PlayUtilSFX(int nSfx, int nFX = 0);
	void StopUtilSFX(int nSfx);

	FPSFX *CreateSFX(byte *buf);

	void PreloadSFX(int nSfx, const char *fn);
	void UnloadAllSFX(void);
	
	void PreloadUtilSFX(int nSfx, const char *fn);
	void UnloadAllUtilSFX(void);

	// Ferma tutta la parte audio
	void PauseSound(bool bPause);

	void SetMusicVolume(int nChannel, int volume);
	int GetMusicVolume(int nChannel);

	// Salvataggio
	void AutoSave(CORO_PARAM);
	void SaveState(int n, const char *name);
	void LoadState(CORO_PARAM, int n);
	void GetSaveStateFileName(int n, char *buf);

	// Prende il thumbnail
	void GrabThumbnail(void);
	uint16 *GetThumbnail(void) { return m_curThumbnail; }

	void Quit(void) { m_bQuitNow = true; }

	void OpenInitLoadMenu(CORO_PARAM);
	void OpenInitOptions(CORO_PARAM);
};

// Global reference to the TonyEngine object
extern TonyEngine *_vm;

} // End of namespace Tony

#endif /* TONY_H */
