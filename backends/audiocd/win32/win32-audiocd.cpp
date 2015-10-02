/* Cabal - Legacy Game Implementations
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

#include "backends/audiocd/win32/win32-audiocd.h"

#include "audio/audiostream.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "common/array.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/str.h"
#include "common/timer.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
#include <winioctl.h>
#include <ntddcdrm.h>
#else
#include <ddk/ntddcdrm.h>
#endif

enum {
	kLeadoutTrack = 0xAA
};

enum {
	kBytesPerFrame = 2352,
	kSamplesPerFrame = kBytesPerFrame / 2
};

enum {
	kSecondsPerMinute = 60,
	kFramesPerSecond = 75
};

enum {
	// The CD-ROM pre-gap is 2s
	kPreGapFrames = kFramesPerSecond * 2
};

enum {
	// Keep about a second's worth of audio in the buffer
	kBufferThreshold = kFramesPerSecond
};

static int getFrameCount(const TRACK_DATA &data) {
	int time = data.Address[1];
	time *= kSecondsPerMinute;
	time += data.Address[2];
	time *= kFramesPerSecond;
	time += data.Address[3];
	return time;
}

class Win32AudioCDStream : public Audio::SeekableAudioStream {
public:
	Win32AudioCDStream(HANDLE handle, const TRACK_DATA &startEntry, const TRACK_DATA &endEntry);
	~Win32AudioCDStream();

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	int getRate() const { return 44100; }
	bool endOfData() const;
	bool seek(const Audio::Timestamp &where);
	Audio::Timestamp getLength() const;

private:
	HANDLE _driveHandle;
	const TRACK_DATA &_startEntry, &_endEntry;
	int16 _buffer[kSamplesPerFrame];
	int _frame;
	uint _bufferPos;

	Common::Queue<int16 *> _bufferQueue;
	int _bufferFrame;
	Common::Mutex _mutex;

	bool readNextFrame();
	static void timerProc(void *refCon);
	void onTimer();
	void emptyQueue();
	void startTimer();
	void stopTimer();
};

Win32AudioCDStream::Win32AudioCDStream(HANDLE handle, const TRACK_DATA &startEntry, const TRACK_DATA &endEntry) :
		_driveHandle(handle), _startEntry(startEntry), _endEntry(endEntry), _buffer(), _frame(0), _bufferPos(kSamplesPerFrame), _bufferFrame(0) {
	startTimer();
}

Win32AudioCDStream::~Win32AudioCDStream() {
	stopTimer();
	emptyQueue();
}

int Win32AudioCDStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;

	// See if any data is left first
	while (_bufferPos < kSamplesPerFrame && samples < numSamples)
		buffer[samples++] = _buffer[_bufferPos++];

	// Bail out if done
	if (endOfData())
		return samples;

	while (samples < numSamples && !endOfData()) {
		if (!readNextFrame())
			return samples;

		// Copy the samples over
		for (_bufferPos = 0; _bufferPos < kSamplesPerFrame && samples < numSamples; )
			buffer[samples++] = _buffer[_bufferPos++];
	}

	return samples;
}

bool Win32AudioCDStream::readNextFrame() {
	// Fetch a frame from the queue
	int16 *buffer;

	{
		Common::StackLock lock(_mutex);

		// Nothing we can do if it's empty
		if (_bufferQueue.empty())
			return false;

		buffer = _bufferQueue.pop();
	}

	memcpy(_buffer, buffer, kSamplesPerFrame * 2);
	delete[] buffer;
	_frame++;
	return true;
}

bool Win32AudioCDStream::endOfData() const {
	return getFrameCount(_startEntry) + _frame >= getFrameCount(_endEntry) && _bufferPos == kSamplesPerFrame;
}

bool Win32AudioCDStream::seek(const Audio::Timestamp &where) {
	// Stop the timer
	stopTimer();

	// Clear anything out of the queue
	emptyQueue();

	// Convert to the frame number
	// Really not much else needed
	_bufferPos = kSamplesPerFrame;
	_frame = where.convertToFramerate(kFramesPerSecond).totalNumberOfFrames();
	_bufferFrame = _frame;

	// Start the timer again
	startTimer();
	return true;
}

Audio::Timestamp Win32AudioCDStream::getLength() const {
	return Audio::Timestamp(0, getFrameCount(_endEntry) - getFrameCount(_startEntry), 75);
}

void Win32AudioCDStream::timerProc(void *refCon) {
	static_cast<Win32AudioCDStream *>(refCon)->onTimer();
}

void Win32AudioCDStream::onTimer() {
	// The goal here is to do as much work in this timer instead
	// of doing it in the readBuffer() call, which is the mixer.

	// If we're done, bail.
	if (getFrameCount(_startEntry) + _bufferFrame >= getFrameCount(_endEntry))
		return;

	// Get a quick count of the number of items in the queue
	// We don't care that much; we only need a quick estimate
	_mutex.lock();
	uint32 queueCount = _bufferQueue.size();
	_mutex.unlock();

	// If we have enough audio buffered, bail out
	if (queueCount >= kBufferThreshold)
		return;

	while (queueCount < kBufferThreshold && getFrameCount(_startEntry) + _bufferFrame < getFrameCount(_endEntry)) {
		int16 *buffer = new int16[kSamplesPerFrame];

		// Figure out the MSF of the frame we're looking for
		int frame = _bufferFrame + getFrameCount(_startEntry);

		// Request to read that frame
		RAW_READ_INFO readAudio;
		memset(&readAudio, 0, sizeof(readAudio));
		readAudio.DiskOffset.QuadPart = (frame - kPreGapFrames) * 2048;
		readAudio.SectorCount = 1;
		readAudio.TrackMode = CDDA;

		DWORD bytesReturned;
		bool result = DeviceIoControl(
			_driveHandle,
			IOCTL_CDROM_RAW_READ,
			&readAudio,
			sizeof(readAudio),
			buffer,
			kBytesPerFrame,
			&bytesReturned,
			NULL);
		if (!result) {
			warning("Failed to retrieve CD sector %d: %d", frame, (int)GetLastError());
			_bufferFrame = getFrameCount(_endEntry) - getFrameCount(_startEntry);
			return;
		}

		_bufferFrame++;

		// Now push the buffer onto the queue
		Common::StackLock lock(_mutex);
		_bufferQueue.push(buffer);
		queueCount = _bufferQueue.size();
	}
}

void Win32AudioCDStream::startTimer() {
	g_system->getTimerManager()->installTimerProc(timerProc, 10 * 1000, this, "Win32AudioCDStream");
}

void Win32AudioCDStream::stopTimer() {
	g_system->getTimerManager()->removeTimerProc(timerProc);
}

void Win32AudioCDStream::emptyQueue() {
	while (!_bufferQueue.empty())
		delete[] _bufferQueue.pop();
}

class Win32AudioCDManager : public DefaultAudioCDManager {
public:
	Win32AudioCDManager();
	~Win32AudioCDManager();

	bool openCD(int drive);
	void closeCD();
	void playCD(int track, int numLoops, int startFrame, int duration);

protected:
	bool openCD(const Common::String &drive);

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
	closeCD();
}

bool Win32AudioCDManager::openCD(int drive) {
	closeCD();

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
		closeCD();
		return false;
	}
	
	return false;
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
		closeCD();
		return false;
	}

	return true;
}

void Win32AudioCDManager::closeCD() {
	// Stop any previous track
	stop();

	if (_driveHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(_driveHandle);
		_driveHandle = INVALID_HANDLE_VALUE;
	}

	_firstTrack = _lastTrack = 0;
	_tocEntries.clear();
}

void Win32AudioCDManager::playCD(int track, int numLoops, int startFrame, int duration) {
	// Stop any previous track
	stop();

	// HACK: For now, just assume that track number is right
	// That only works because ScummVM uses the wrong track number anyway	

	if (track >= (int)_tocEntries.size() - 1) {
		warning("No such track %d", track);
		return;
	}

	// Bail if the track isn't an audio track
	if ((_tocEntries[track].Control & 0x04) != 0) {
		warning("Track %d is not audio", track);
		return;
	}

	// Create the AudioStream and play it
	debug(1, "Playing CD track %d", track);

	Audio::SeekableAudioStream *audioStream = new Win32AudioCDStream(_driveHandle, _tocEntries[track], _tocEntries[track + 1]);

	Audio::Timestamp start = Audio::Timestamp(0, startFrame, 75);
	Audio::Timestamp end = (duration == 0) ? audioStream->getLength() : Audio::Timestamp(0, startFrame + duration, 75);

	// Fake emulation since we're really playing an AudioStream
	_emulating = true;

	_mixer->playStream(
		Audio::Mixer::kMusicSoundType,
		&_handle,
		Audio::makeLoopingAudioStream(audioStream, start, end, (numLoops < 1) ? numLoops + 1 : numLoops),
		-1,
		_cd.volume,
		_cd.balance,
		DisposeAfterUse::YES,
		true);
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
