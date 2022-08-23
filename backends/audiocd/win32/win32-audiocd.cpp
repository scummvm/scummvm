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
 * Original license header:
 *
 * Cabal - Legacy Game Implementations
 *
 * Cabal is the legal property of its developers, whose names
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

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "backends/audiocd/win32/win32-audiocd.h"

#include "audio/audiostream.h"
#include "backends/audiocd/audiocd-stream.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"
#include "common/array.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/str.h"
#include "common/timer.h"

#include <winioctl.h>
#if _MSC_VER < 1900
// WORKAROUND: Older versions of MSVC might not supply DDK headers by default.
// Visual Studio 2015 contains the required headers. We use a compatability
// header from MinGW's w32api for all older versions.
// TODO: Limit this to the Visual Studio versions which actually require this.
#include "msvc/ntddcdrm.h"
#elif defined(__MINGW32__) && !defined(__MINGW64__)
// Classic MinGW uses non standard paths for DDK headers.
#include <ddk/ntddcdrm.h>
#else
#include <ntddcdrm.h>
#endif

class Win32AudioCDStream final : public AudioCDStream {
public:
	Win32AudioCDStream(HANDLE handle, const TRACK_DATA &startEntry, const TRACK_DATA &endEntry);
	~Win32AudioCDStream();

protected:
	uint getStartFrame() const override;
	uint getEndFrame() const override;
	bool readFrame(int frame, int16 *buffer) override;

private:
	HANDLE _driveHandle;
	const TRACK_DATA &_startEntry, &_endEntry;

	enum {
		// The CD-ROM pre-gap is 2s
		kPreGapFrames = kFramesPerSecond * 2
	};

	static int getFrameCount(const TRACK_DATA &data) {
		int time = data.Address[1];
		time *= kSecondsPerMinute;
		time += data.Address[2];
		time *= kFramesPerSecond;
		time += data.Address[3];
		return time;
	}
};

Win32AudioCDStream::Win32AudioCDStream(HANDLE handle, const TRACK_DATA &startEntry, const TRACK_DATA &endEntry) :
	_driveHandle(handle), _startEntry(startEntry), _endEntry(endEntry) {
	// We fill the buffer here already to prevent any out of sync issues due
	// to the CD not yet having spun up.
	startTimer(true);
}

Win32AudioCDStream::~Win32AudioCDStream() {
	stopTimer();
}

uint Win32AudioCDStream::getStartFrame() const {
	return getFrameCount(_startEntry);
}

uint Win32AudioCDStream::getEndFrame() const {
	return getFrameCount(_endEntry);
}

bool Win32AudioCDStream::readFrame(int frame, int16 *buffer) {
	// Request to read that frame
	RAW_READ_INFO readAudio;
	memset(&readAudio, 0, sizeof(readAudio));
	readAudio.DiskOffset.QuadPart = (frame - kPreGapFrames) * 2048;
	readAudio.SectorCount = 1;
	readAudio.TrackMode = CDDA;

	DWORD bytesReturned;
	return DeviceIoControl(
	           _driveHandle,
	           IOCTL_CDROM_RAW_READ,
	           &readAudio,
	           sizeof(readAudio),
	           buffer,
	           kBytesPerFrame,
	           &bytesReturned,
	           nullptr);
}


class Win32AudioCDManager final : public DefaultAudioCDManager {
public:
	Win32AudioCDManager();
	~Win32AudioCDManager();

	bool open() override;
	void close() override;
	bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
			Audio::Mixer::SoundType soundType) override;
	bool isDataAndCDAudioReadFromSameCD() override;

protected:
	bool openCD(int drive) override;
	bool openCD(const Common::String &drive) override;

private:
	bool loadTOC();

	typedef Common::Array<char> DriveList;
	DriveList detectDrives();
	bool tryAddDrive(char drive, DriveList &drives);

	HANDLE _driveHandle;
	int _firstTrack, _lastTrack;
	Common::Array<TRACK_DATA> _tocEntries;
};

Win32AudioCDManager::Win32AudioCDManager() {
	_driveHandle = INVALID_HANDLE_VALUE;
	_firstTrack = _lastTrack = 0;
}

Win32AudioCDManager::~Win32AudioCDManager() {
	close();
}

bool Win32AudioCDManager::open() {
	close();

	if (openRealCD())
		return true;

	return DefaultAudioCDManager::open();
}

bool Win32AudioCDManager::openCD(int drive) {
	// Fetch the drive list
	DriveList drives = detectDrives();
	if (drive >= (int)drives.size())
		return false;

	debug(1, "Opening CD drive %c:\\", drives[drive]);

	// Construct the drive path and try to open it
	Common::String drivePath = Common::String::format("\\\\.\\%c:", drives[drive]);
	TCHAR *tDrivePath = Win32::stringToTchar(drivePath);
	_driveHandle = CreateFile(tDrivePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	free(tDrivePath);
	if (_driveHandle == INVALID_HANDLE_VALUE) {
		warning("Failed to open drive %c:\\, error %d", drives[drive], (int)GetLastError());
		return false;
	}

	if (!loadTOC()) {
		close();
		return false;
	}

	return true;
}

bool Win32AudioCDManager::openCD(const Common::String &drive) {
	// Just some bounds checking
	if (drive.empty() || drive.size() > 3)
		return false;

	if (!Common::isAlpha(drive[0]) || drive[1] != ':')
		return false;

	if (drive[2] != 0 && drive[2] != '\\')
		return false;

	DriveList drives;
	if (!tryAddDrive(toupper(drive[0]), drives))
		return false;

	// Construct the drive path and try to open it
	Common::String drivePath = Common::String::format("\\\\.\\%c:", drives[0]);
	TCHAR *tDrivePath = Win32::stringToTchar(drivePath);
	_driveHandle = CreateFile(tDrivePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	free(tDrivePath);
	if (_driveHandle == INVALID_HANDLE_VALUE) {
		warning("Failed to open drive %c:\\, error %d", drives[0], (int)GetLastError());
		return false;
	}

	if (!loadTOC()) {
		close();
		return false;
	}

	return true;
}

void Win32AudioCDManager::close() {
	DefaultAudioCDManager::close();

	if (_driveHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(_driveHandle);
		_driveHandle = INVALID_HANDLE_VALUE;
	}

	_firstTrack = _lastTrack = 0;
	_tocEntries.clear();
}

bool Win32AudioCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	// Prefer emulation
	if (DefaultAudioCDManager::play(track, numLoops, startFrame, duration, onlyEmulate, soundType))
		return true;

	// If we're set to only emulate, or have no CD drive, return here
	if (onlyEmulate || _driveHandle == INVALID_HANDLE_VALUE)
		return false;

	// HACK: For now, just assume that track number is right
	// That only works because ScummVM uses the wrong track number anyway

	if (track >= (int)_tocEntries.size() - 1) {
		warning("No such track %d", track);
		return false;
	}

	// Bail if the track isn't an audio track
	if ((_tocEntries[track].Control & 0x04) != 0) {
		warning("Track %d is not audio", track);
		return false;
	}

	// Create the AudioStream and play it
	debug(1, "Playing CD track %d", track);

	Audio::SeekableAudioStream *audioStream = new Win32AudioCDStream(_driveHandle, _tocEntries[track], _tocEntries[track + 1]);

	Audio::Timestamp start = Audio::Timestamp(0, startFrame, 75);
	Audio::Timestamp end = (duration == 0) ? audioStream->getLength() : Audio::Timestamp(0, startFrame + duration, 75);

	// Fake emulation since we're really playing an AudioStream
	_emulating = true;

	_mixer->playStream(
	    soundType,
	    &_handle,
	    Audio::makeLoopingAudioStream(audioStream, start, end, (numLoops < 1) ? numLoops + 1 : numLoops),
	    -1,
	    _cd.volume,
	    _cd.balance,
	    DisposeAfterUse::YES,
	    true);
	return true;
}

bool Win32AudioCDManager::loadTOC() {
	CDROM_READ_TOC_EX tocRequest;
	memset(&tocRequest, 0, sizeof(tocRequest));
	tocRequest.Format = CDROM_READ_TOC_EX_FORMAT_TOC;
	tocRequest.Msf = 1;
	tocRequest.SessionTrack = 0;

	DWORD bytesReturned;
	CDROM_TOC tocData;
	bool result = DeviceIoControl(
	                  _driveHandle,
	                  IOCTL_CDROM_READ_TOC_EX,
	                  &tocRequest,
	                  sizeof(tocRequest),
	                  &tocData,
	                  sizeof(tocData),
	                  &bytesReturned,
	                  nullptr);
	if (!result) {
		debug("Failed to query the CD TOC: %d", (int)GetLastError());
		return false;
	}

	_firstTrack = tocData.FirstTrack;
	_lastTrack = tocData.LastTrack;
#if 0
	debug("First Track: %d", tocData.FirstTrack);
	debug("Last Track: %d", tocData.LastTrack);
#endif

	for (uint32 i = 0; i < (bytesReturned - 4) / sizeof(TRACK_DATA); i++)
		_tocEntries.push_back(tocData.TrackData[i]);

#if 0
	for (uint32 i = 0; i < _tocEntries.size(); i++) {
		const TRACK_DATA &entry = _tocEntries[i];
		debug("Entry:");
		debug("\tTrack: %d", entry.TrackNumber);
		debug("\tAdr: %d", entry.Adr);
		debug("\tCtrl: %d", entry.Control);
		debug("\tMSF: %d:%d:%d\n", entry.Address[1], entry.Address[2], entry.Address[3]);
	}
#endif

	return true;
}

Win32AudioCDManager::DriveList Win32AudioCDManager::detectDrives() {
	DriveList drives;

	// Try to get the game path's drive
	char gameDrive = 0;
	if (ConfMan.hasKey("path")) {
		Common::String gamePath = ConfMan.get("path");
		TCHAR *tGamePath = Win32::stringToTchar(gamePath);
		TCHAR fullPath[MAX_PATH];
		DWORD result = GetFullPathName(tGamePath, MAX_PATH, fullPath, nullptr);
		free(tGamePath);

		if (result > 0 && result < sizeof(fullPath) && Common::isAlpha(fullPath[0]) && fullPath[1] == ':' && tryAddDrive(toupper(fullPath[0]), drives))
			gameDrive = drives[0];
	}

	// Try adding the rest of the drives
	for (char drive = 'A'; drive <= 'Z'; drive++)
		if (drive != gameDrive)
			tryAddDrive(drive, drives);

	return drives;
}

bool Win32AudioCDManager::tryAddDrive(char drive, DriveList &drives) {
	if (!Win32::isDriveCD(drive)) {
		return false;
	}

	debug(2, "Detected drive %c:\\ as a CD drive", drive);
	drives.push_back(drive);
	return true;
}

bool Win32AudioCDManager::isDataAndCDAudioReadFromSameCD() {
	// It is a known bug under Windows that games that play CD audio cause
	// ScummVM to crash if the data files are read from the same CD.
	char driveLetter;
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	if (!gameDataDir.getPath().empty()) {
		driveLetter = gameDataDir.getPath()[0];
	} else {
		// That's it! I give up!
		Common::FSNode currentDir(".");
		if (!currentDir.getPath().empty()) {
			driveLetter = currentDir.getPath()[0];
		} else {
			return false;
		}
	}

	return Win32::isDriveCD(driveLetter);
}

AudioCDManager *createWin32AudioCDManager() {
	return new Win32AudioCDManager();
}

#endif // WIN32
