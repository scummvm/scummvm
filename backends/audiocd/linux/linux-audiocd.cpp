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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "backends/audiocd/linux/linux-audiocd.h"

#include "audio/audiostream.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "common/array.h"
#include "common/config-manager.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/timer.h"

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
	// Keep about a second's worth of audio in the buffer
	kBufferThreshold = kFramesPerSecond
};

static int getFrameCount(const cdrom_msf0 &msf) {
	int time = msf.minute;
	time *= kSecondsPerMinute;
	time += msf.second;
	time *= kFramesPerSecond;
	time += msf.frame;
	return time;
}

class LinuxAudioCDStream : public Audio::SeekableAudioStream {
public:
	LinuxAudioCDStream(int fd, const cdrom_tocentry &startEntry, const cdrom_tocentry &endEntry);
	~LinuxAudioCDStream();

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return true; }
	int getRate() const { return 44100; }
	bool endOfData() const;
	bool seek(const Audio::Timestamp &where);
	Audio::Timestamp getLength() const;

private:
	int _fd;
	const cdrom_tocentry &_startEntry, &_endEntry;
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

LinuxAudioCDStream::LinuxAudioCDStream(int fd, const cdrom_tocentry &startEntry, const cdrom_tocentry &endEntry) :
		_fd(fd), _startEntry(startEntry), _endEntry(endEntry), _buffer(), _frame(0), _bufferPos(kSamplesPerFrame), _bufferFrame(0) {
	startTimer();
}

LinuxAudioCDStream::~LinuxAudioCDStream() {
	stopTimer();
	emptyQueue();
}

int LinuxAudioCDStream::readBuffer(int16 *buffer, const int numSamples) {
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

bool LinuxAudioCDStream::readNextFrame() {
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

bool LinuxAudioCDStream::endOfData() const {
	return getFrameCount(_startEntry.cdte_addr.msf) + _frame >= getFrameCount(_endEntry.cdte_addr.msf) && _bufferPos == kSamplesPerFrame;
}

bool LinuxAudioCDStream::seek(const Audio::Timestamp &where) {
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

Audio::Timestamp LinuxAudioCDStream::getLength() const {
	return Audio::Timestamp(0, getFrameCount(_endEntry.cdte_addr.msf) - getFrameCount(_startEntry.cdte_addr.msf), 75);
}

void LinuxAudioCDStream::timerProc(void *refCon) {
	static_cast<LinuxAudioCDStream *>(refCon)->onTimer();
}

void LinuxAudioCDStream::onTimer() {
	// The goal here is to do as much work in this timer instead
	// of doing it in the readBuffer() call, which is the mixer.

	// If we're done, bail.
	if (getFrameCount(_startEntry.cdte_addr.msf) + _bufferFrame >= getFrameCount(_endEntry.cdte_addr.msf))
		return;

	// Get a quick count of the number of items in the queue
	// We don't care that much; we only need a quick estimate
	_mutex.lock();
	uint32 queueCount = _bufferQueue.size();
	_mutex.unlock();

	// If we have enough audio buffered, bail out
	if (queueCount >= kBufferThreshold)
		return;

	while (queueCount < kBufferThreshold && getFrameCount(_startEntry.cdte_addr.msf) + _bufferFrame < getFrameCount(_endEntry.cdte_addr.msf)) {
		int16 *buffer = new int16[kSamplesPerFrame];

		// Figure out the MSF of the frame we're looking for
		int frame = _bufferFrame + getFrameCount(_startEntry.cdte_addr.msf);

		int seconds = frame / kFramesPerSecond;
		frame %= kFramesPerSecond;
		int minutes = seconds / kSecondsPerMinute;
		seconds %= kSecondsPerMinute;

		// Request to read that frame
		cdrom_read_audio readAudio;
		readAudio.addr.msf.minute = minutes;
		readAudio.addr.msf.second = seconds;
		readAudio.addr.msf.frame = frame;
		readAudio.addr_format = CDROM_MSF;
		readAudio.nframes = 1;
		readAudio.buf = reinterpret_cast<__u8*>(buffer);

		if (ioctl(_fd, CDROMREADAUDIO, &readAudio) < 0) {
			warning("Failed to read audio");
			_bufferFrame = getFrameCount(_endEntry.cdte_addr.msf);
			return;
		}

		_bufferFrame++;

		// Now push the buffer onto the queue
		Common::StackLock lock(_mutex);
		_bufferQueue.push(buffer);
		queueCount = _bufferQueue.size();
	}
}

void LinuxAudioCDStream::startTimer() {
	g_system->getTimerManager()->installTimerProc(timerProc, 10 * 1000, this, "LinuxAudioCDStream");
}

void LinuxAudioCDStream::stopTimer() {
	g_system->getTimerManager()->removeTimerProc(timerProc);
}

void LinuxAudioCDStream::emptyQueue() {
	while (!_bufferQueue.empty())
		delete[] _bufferQueue.pop();
}

class LinuxAudioCDManager : public DefaultAudioCDManager {
public:
	LinuxAudioCDManager();
	~LinuxAudioCDManager();

	bool openCD(int drive);
	void closeCD();
	void playCD(int track, int numLoops, int startFrame, int duration);

private:
	struct Device {
		Device(const Common::String &n, dev_t d) : name(n), device(d) {}
		Common::String name;
		dev_t device;
	};

	typedef Common::Array<Device> DeviceList;
	DeviceList scanDevices();
	bool tryAddDrive(DeviceList &devices, const Common::String &drive);
	bool tryAddDrive(DeviceList &devices, const Common::String &drive, dev_t device);
	bool tryAddDrive(DeviceList &devices, dev_t device);
	bool tryAddPath(DeviceList &devices, const Common::String &path);
	bool tryAddGamePath(DeviceList &devices);
	bool loadTOC();
	static bool hasDevice(const DeviceList &devices, dev_t device);

	int _fd;
	cdrom_tochdr _tocHeader;
	Common::Array<cdrom_tocentry> _tocEntries;
};

static bool isTrayEmpty(int errorNumber) {
	switch (errorNumber) {
	case EIO:
	case ENOENT:
	case EINVAL:
#ifdef ENOMEDIUM
	case ENOMEDIUM:
#endif
		return true;
	}

	return false;
}

LinuxAudioCDManager::LinuxAudioCDManager() {
	_fd = -1;
	memset(&_tocHeader, 0, sizeof(_tocHeader));
}

LinuxAudioCDManager::~LinuxAudioCDManager() {
	closeCD();
}

bool LinuxAudioCDManager::openCD(int drive) {
	closeCD();

	DeviceList devices = scanDevices();
	if (drive >= (int)devices.size())
		return false;

	_fd = open(devices[drive].name.c_str(), O_RDONLY | O_NONBLOCK, 0);
	if (_fd < 0)
		return false;

	if (!loadTOC()) {
		closeCD();
		return false;
	}

	return true;
}

void LinuxAudioCDManager::closeCD() {
	if (_fd < 0)
		return;

	stop();
	close(_fd);
	memset(&_tocHeader, 0, sizeof(_tocHeader));
	_tocEntries.clear();
}

void LinuxAudioCDManager::playCD(int track, int numLoops, int startFrame, int duration) {
	// Stop any previous track
	stop();

	// HACK: For now, just assume that track number is right
	// That only works because ScummVM uses the wrong track number anyway	

	if (track >= (int)_tocEntries.size() - 1) {
		warning("No such track %d", track);
		return;
	}

	// Bail if the track isn't an audio track
	if ((_tocEntries[track].cdte_ctrl & 0x04) != 0) {
		warning("Track %d is not audio", track);
		return;
	}

	// Create the AudioStream and play it
	debug(1, "Playing CD track %d", track);

	Audio::SeekableAudioStream *audioStream = new LinuxAudioCDStream(_fd, _tocEntries[track], _tocEntries[track + 1]);

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

LinuxAudioCDManager::DeviceList LinuxAudioCDManager::scanDevices() {
	DeviceList devices;

	// Try to use the game's path first as the device
	tryAddGamePath(devices);

	// Try adding the default CD-ROM
	tryAddDrive(devices, "/dev/cdrom");

	// TODO: Try others?

	return devices;
}

bool LinuxAudioCDManager::tryAddDrive(DeviceList &devices, const Common::String &drive) {
	struct stat stbuf;
	if (stat(drive.c_str(), &stbuf) < 0)
		return false;

	// Must be a character or block device
	if (!S_ISCHR(stbuf.st_mode) && !S_ISBLK(stbuf.st_mode))
		return false;

	return tryAddDrive(devices, drive, stbuf.st_rdev);
}

bool LinuxAudioCDManager::tryAddDrive(DeviceList &devices, const Common::String &drive, dev_t device) {
	if (hasDevice(devices, device))
		return true;

	// Try opening the device and seeing if it is a CD-ROM drve
	int fd = open(drive.c_str(), O_RDONLY | O_NONBLOCK, 0);
	if (fd >= 0) {
		cdrom_subchnl info;
		info.cdsc_format = CDROM_MSF;

		bool isCD = ioctl(fd, CDROMSUBCHNL, &info) == 0 || isTrayEmpty(errno);
		close(fd);
		if (isCD) {
			devices.push_back(Device(drive, device));
			return true;
		}
	}

	return false;
}

bool LinuxAudioCDManager::tryAddDrive(DeviceList &devices, dev_t device) {
	// Construct the block name
	// (Does anyone have a better way to do this? bdevname is kernel only)
	Common::String name = Common::String::format("/dev/block/%d:%d", gnu_dev_major(device), gnu_dev_minor(device));

	return tryAddDrive(devices, name, device);
}

bool LinuxAudioCDManager::tryAddPath(DeviceList &devices, const Common::String &path) {
	struct stat stbuf;
	if (stat(path.c_str(), &stbuf) < 0)
		return false;

	return tryAddDrive(devices, stbuf.st_dev);
}

bool LinuxAudioCDManager::tryAddGamePath(DeviceList &devices) {
	if (!ConfMan.hasKey("path"))
		return false;

	return tryAddPath(devices, ConfMan.get("path"));
}

bool LinuxAudioCDManager::loadTOC() {
	if (_fd < 0)
		return false;

	if (ioctl(_fd, CDROMREADTOCHDR, &_tocHeader) < 0)
		return false;

	debug(4, "CD: Start Track: %d, End Track %d", _tocHeader.cdth_trk0, _tocHeader.cdth_trk1);

	for (int i = _tocHeader.cdth_trk0; i <= _tocHeader.cdth_trk1; i++) {
		cdrom_tocentry entry;
		memset(&entry, 0, sizeof(entry));
		entry.cdte_track = i;
		entry.cdte_format = CDROM_MSF;

		if (ioctl(_fd, CDROMREADTOCENTRY, &entry) < 0)
			return false;

#if 0
		debug("Entry:");
		debug("\tTrack: %d", entry.cdte_track);
		debug("\tAdr: %d", entry.cdte_adr);
		debug("\tCtrl: %d", entry.cdte_ctrl);
		debug("\tFormat: %d", entry.cdte_format);
		debug("\tMSF: %d:%d:%d", entry.cdte_addr.msf.minute, entry.cdte_addr.msf.second, entry.cdte_addr.msf.frame);
		debug("\tMode: %d\n", entry.cdte_datamode);
#endif

		_tocEntries.push_back(entry);
	}

	// Fetch the leadout so we can get the length of the last frame
	cdrom_tocentry entry;
	memset(&entry, 0, sizeof(entry));
	entry.cdte_track = kLeadoutTrack;
	entry.cdte_format = CDROM_MSF;

	if (ioctl(_fd, CDROMREADTOCENTRY, &entry) < 0)
		return false;

#if 0
	debug("Lead out:");
	debug("\tTrack: %d", entry.cdte_track);
	debug("\tAdr: %d", entry.cdte_adr);
	debug("\tCtrl: %d", entry.cdte_ctrl);
	debug("\tFormat: %d", entry.cdte_format);
	debug("\tMSF: %d:%d:%d", entry.cdte_addr.msf.minute, entry.cdte_addr.msf.second, entry.cdte_addr.msf.frame);
	debug("\tMode: %d\n", entry.cdte_datamode);
#endif

	_tocEntries.push_back(entry);
	return true;
}

bool LinuxAudioCDManager::hasDevice(const DeviceList &devices, dev_t device) {
	for (DeviceList::const_iterator it = devices.begin(); it != devices.end(); it++)
		if (it->device == device)
			return true;

	return false;
}

AudioCDManager *createLinuxAudioCDManager() {
	return new LinuxAudioCDManager();
}
