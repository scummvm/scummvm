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
 * Original license header:
 *
 * Cabal - Legacy Game Implementations
 *
 * Cabal is the legal property of its developers, whose names
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

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "backends/audiocd/win32/win32-audiocd.h"

#include "audio/audiostream.h"
#include "backends/audiocd/audiocd-stream.h"
#include "backends/audiocd/default/default-audiocd.h"
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
	           NULL);
}


class Win32AudioCDManager final : public DefaultAudioCDManager {
public:
	Win32AudioCDManager();
	~Win32AudioCDManager();

	bool open() override;
	void close() override;
	bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
			Audio::Mixer::SoundType soundType) override;

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
	_driveHandle = CreateFileA(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
	_driveHandle = CreateFileA(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
	                  NULL);
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
		char fullPath[MAX_PATH];
		DWORD result = GetFullPathNameA(gamePath.c_str(), sizeof(fullPath), fullPath, 0);

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
	Common::String drivePath = Common::String::format("%c:\\", drive);

	// Ensure it's an actual CD drive
	if (GetDriveTypeA(drivePath.c_str()) != DRIVE_CDROM)
		return false;

	debug(2, "Detected drive %c:\\ as a CD drive", drive);
	drives.push_back(drive);
	return true;
}

AudioCDManager *createWin32AudioCDManager() {
	return new Win32AudioCDManager();
}

#endif // WIN32
