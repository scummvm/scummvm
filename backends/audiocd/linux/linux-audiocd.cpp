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

// Enable all forbidden symbols to allow us to include and use necessary APIs.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/audiocd/linux/linux-audiocd.h"

#ifdef USE_LINUXCD

#include "backends/audiocd/audiocd-stream.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "common/array.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "common/debug.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>

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

// Helper function to convert an error code into a human-readable message
static Common::String getErrorMessage(int errorCode) {
	char buf[256];
	buf[0] = 0;

#ifdef _GNU_SOURCE
	// glibc sucks
	return Common::String(strerror_r(errorCode, buf, sizeof(buf)));
#else
	strerror_r(errorCode, buf, sizeof(buf));
	return Common::String(buf);
#endif
}

class LinuxAudioCDStream : public AudioCDStream {
public:
	LinuxAudioCDStream(int fd, const cdrom_tocentry &startEntry, const cdrom_tocentry &endEntry);
	~LinuxAudioCDStream();

protected:
	uint getStartFrame() const;
	uint getEndFrame() const;
	bool readFrame(int frame, int16 *buffer);

private:
	int _fd;
	const cdrom_tocentry &_startEntry, &_endEntry;
};

LinuxAudioCDStream::LinuxAudioCDStream(int fd, const cdrom_tocentry &startEntry, const cdrom_tocentry &endEntry) :
	_fd(fd), _startEntry(startEntry), _endEntry(endEntry) {
	// We fill the buffer here already to prevent any out of sync issues due
	// to the CD not yet having spun up.
	startTimer(true);
}

LinuxAudioCDStream::~LinuxAudioCDStream() {
	stopTimer();
}

bool LinuxAudioCDStream::readFrame(int frame, int16 *buffer) {
	// Create the argument
	union {
		cdrom_msf msf;
		char buffer[kBytesPerFrame];
	} arg;

	int seconds = frame / kFramesPerSecond;
	frame %= kFramesPerSecond;
	int minutes = seconds / kSecondsPerMinute;
	seconds %= kSecondsPerMinute;

	// Request to read that frame
	// We don't use CDROMREADAUDIO, as it seems to cause kernel
	// panics on ejecting discs. Probably bad to eject the disc
	// while playing, but at least let's try to prevent that case.
	arg.msf.cdmsf_min0 = minutes;
	arg.msf.cdmsf_sec0 = seconds;
	arg.msf.cdmsf_frame0 = frame;
	// The "end" part is irrelevant (why isn't cdrom_msf0 the type
	// instead?)

	if (ioctl(_fd, CDROMREADRAW, &arg) < 0) {
		warning("Failed to CD read audio: %s", getErrorMessage(errno).c_str());
		return false;
	}

	memcpy(buffer, arg.buffer, kBytesPerFrame);
	return true;
}

uint LinuxAudioCDStream::getStartFrame() const {
	return getFrameCount(_startEntry.cdte_addr.msf);
}

uint LinuxAudioCDStream::getEndFrame() const {
	return getFrameCount(_endEntry.cdte_addr.msf);
}


class LinuxAudioCDManager : public DefaultAudioCDManager {
public:
	LinuxAudioCDManager();
	~LinuxAudioCDManager();

	bool open() override;
	void close() override;
	bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
			Audio::Mixer::SoundType soundType) override;

protected:
	bool openCD(int drive) override;
	bool openCD(const Common::String &drive) override;

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
	close();
}

bool LinuxAudioCDManager::open() {
	close();

	if (openRealCD())
		return true;

	return DefaultAudioCDManager::open();
}

void LinuxAudioCDManager::close() {
	DefaultAudioCDManager::close();

	if (_fd < 0)
		return;

	::close(_fd);
	memset(&_tocHeader, 0, sizeof(_tocHeader));
	_tocEntries.clear();
}

bool LinuxAudioCDManager::openCD(int drive) {
	DeviceList devices = scanDevices();
	if (drive >= (int)devices.size())
		return false;

	_fd = ::open(devices[drive].name.c_str(), O_RDONLY | O_NONBLOCK, 0);
	if (_fd < 0)
		return false;

	if (!loadTOC()) {
		close();
		return false;
	}

	return true;
}

bool LinuxAudioCDManager::openCD(const Common::String &drive) {
	DeviceList devices;
	if (!tryAddDrive(devices, drive) && !tryAddPath(devices, drive))
		return false;

	_fd = ::open(devices[0].name.c_str(), O_RDONLY | O_NONBLOCK, 0);
	if (_fd < 0)
		return false;

	if (!loadTOC()) {
		close();
		return false;
	}

	return true;
}

bool LinuxAudioCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	// Prefer emulation
	if (DefaultAudioCDManager::play(track, numLoops, startFrame, duration, onlyEmulate, soundType))
		return true;

	// If we're set to only emulate, or have no CD drive, return here
	if (onlyEmulate || _fd < 0)
		return false;

	// HACK: For now, just assume that track number is right
	// That only works because ScummVM uses the wrong track number anyway

	if (track >= (int)_tocEntries.size() - 1) {
		warning("No such track %d", track);
		return false;
	}

	// Bail if the track isn't an audio track
	if ((_tocEntries[track].cdte_ctrl & 0x04) != 0) {
		warning("Track %d is not audio", track);
		return false;
	}

	// Create the AudioStream and play it
	debug(1, "Playing CD track %d", track);

	Audio::SeekableAudioStream *audioStream = new LinuxAudioCDStream(_fd, _tocEntries[track], _tocEntries[track + 1]);

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
	int fd = ::open(drive.c_str(), O_RDONLY | O_NONBLOCK, 0);
	if (fd >= 0) {
		cdrom_subchnl info;
		info.cdsc_format = CDROM_MSF;

		bool isCD = ioctl(fd, CDROMSUBCHNL, &info) == 0 || isTrayEmpty(errno);
		::close(fd);
		if (isCD) {
			devices.push_back(Device(drive, device));
			return true;
		}
	}

	return false;
}

bool LinuxAudioCDManager::tryAddDrive(DeviceList &devices, dev_t device) {
	// Construct the block name
	// TODO: libblkid's blkid_devno_to_devname is exactly what we look for.
	// This requires an external dependency though.
	Common::String name = Common::String::format("/dev/block/%d:%d", major(device), minor(device));

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

#endif // USE_LINUXCD
