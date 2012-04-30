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
#include "common/file.h"
#include "tony/tony.h"
#include "tony/mpal/mpal.h"

namespace Tony {

TonyEngine *_vm;

TonyEngine::TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc) : Engine(syst), 
		_gameDescription(gameDesc), _randomSource("tony") {
	_vm = this;
}

TonyEngine::~TonyEngine() {
	// Close the voice database
	CloseVoiceDatabase();
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
	m_hEndOfFrame = CreateEvent(NULL, false, false, NULL);

	m_bPaused = false;
	m_bDrawLocation = true;
	m_startTime = g_system->getMillis();

	// Initialise the function list
	Common::fill(FuncList, FuncList + 300, (LPCUSTOMFUNCTION)NULL);

	// Initializes MPAL system, passing the custom functions list
	Common::File f;
	if (!f.open("ROASTED.MPC"))
		return Common::kReadingFailed;
	f.close();

	if (!mpalInit("ROASTED.MPC", "ROASTED.MPR", FuncList))
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


	return Common::kNoError;
}

/**
 * Display an error message
 */
void TonyEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

char nextMusic[_MAX_PATH];
bool nextLoop;
int nextChannel;
int nextSync;
int curChannel;
int flipflop=0;
OSystem::MutexRef csMusic;


void TonyEngine::PlayMusic(int nChannel, char* fn, int nFX, bool bLoop, int nSync) {
	warning("TonyEngine::PlayMusic");
}

void TonyEngine::PlaySFX(int nChannel, int nFX) {
	warning("TonyEngine::PlaySFX");
}

void TonyEngine::StopMusic(int nChannel) {
	warning("TonyEngine::StopMusic");
}

void TonyEngine::StopSFX(int nChannel) {
	warning("TonyEngine::StopSFX");
}

void TonyEngine::PlayUtilSFX(int nChannel, int nFX) {
	warning("TonyEngine::PlayUtilSFX");
}

void TonyEngine::StopUtilSFX(int nChannel) {
	warning("TonyEngine::StopUtilSFX");
}

void TonyEngine::PreloadSFX(int nChannel, char* fn) {
	warning("TonyEngine::PreloadSFX");
}

FPSFX *TonyEngine::CreateSFX(byte *buf) {
	warning("TonyEngine::CreateSFX");
	return NULL;
}

void TonyEngine::PreloadUtilSFX(int nChannel, char *fn) {
	warning("TonyEngine::PreloadUtilSFX");
}

void TonyEngine::UnloadAllSFX(void) {
	warning("TonyEngine::UnloadAllSFX");
}

void TonyEngine::UnloadAllUtilSFX(void) {
	warning("TonyEngine::UnloadAllUtilSFX");
}

void TonyEngine::InitMusic() {
	warning("TODO: TonyEngine::InitMusic");
}

void TonyEngine::CloseMusic() {
	warning("TODO: TonyEngine::CloseMusic");
}

void TonyEngine::PauseSound(bool bPause) {
}

void TonyEngine::SetMusicVolume(int nChannel, int volume) {
}

int TonyEngine::GetMusicVolume(int nChannel) {
	return 255;
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
	//m_wnd.GrabThumbnail(m_curThumbnail);
	warning("TODO: TonyEngine::GrabThumbnail");
}

} // End of namespace Tony
