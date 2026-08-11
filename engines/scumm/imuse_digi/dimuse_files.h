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

#if !defined(SCUMM_IMUSE_DIGI_FILES_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_IMUSE_DIGI_FILES_H

#include "common/scummsys.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "scumm/resource.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_bndmgr.h"
#include "scumm/imuse_digi/dimuse_sndmgr.h"

namespace Scumm {

class IMuseDigiFilesHandler {

private:
	IMuseDigital *_engine;
	ImuseDigiSndMgr *_sound;
	ScummEngine_v7 *_vm;
	Common::Mutex _mutex;
	IMuseDigiSndBuffer _soundBuffers[4];
	char _currentSpeechFilename[60] = {};
	ScummFile *_ftSpeechFile;
	char _ftSpeechFilename[160] = {};
	int _ftSpeechSubFileOffset;
	int _ftSpeechFileSize;
	int _ftSpeechFileCurPos;

	void getFilenameFromSoundId(int soundId, char *fileName, size_t size);
public:
	IMuseDigiFilesHandler(IMuseDigital *engine, ScummEngine_v7 *vm);
	~IMuseDigiFilesHandler();

	uint8 *getSoundAddrData(int soundId);
	int getSoundAddrDataSize(int soundId, bool hasStream);
	int getNextSound(int soundId);
	int seek(int soundId, int32 offset, int mode, int bufId);
	int read(int soundId, uint8 *buf, int32 size, int bufId);
	IMuseDigiSndBuffer *getBufInfo(int bufId);
	int openSound(int soundId);
	void closeSound(int soundId);
	void closeAllSounds();
	void allocSoundBuffer(int bufId, int32 size, int32 loadSize, int32 criticalSize);
	void deallocSoundBuffer(int bufId);
	void flushSounds();
	int setCurrentSpeechFilename(const char *fileName);
	void setCurrentFtSpeechFile(const char *fileName, ScummFile *file, uint32 offset, uint32 size);
	void closeSoundImmediatelyById(int soundId);
	void saveLoad(Common::Serializer &ser);
};

} // End of namespace Scumm
#endif
