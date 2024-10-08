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
 * the Free Software Foundation, either version 2 of the License, or
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

#ifdef MACOSX

#include <sys/stat.h>
#include <sys/mount.h>

#include "common/scummsys.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/timestamp.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/fs.h"
#include "common/hashmap.h"
#include "common/textconsole.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "backends/audiocd/macosx/macosx-audiocd.h"
#include "backends/fs/stdiostream.h"

// Partially based on SDL's code

/**
 * The macOS audio cd manager. Implements real audio cd playback.
 */
class MacOSXAudioCDManager : public DefaultAudioCDManager {
public:
	MacOSXAudioCDManager() {}
	~MacOSXAudioCDManager();

	bool open() override;
	void close() override;
	bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
			Audio::Mixer::SoundType soundType) override;

protected:
	bool openCD(int drive) override;
	bool openCD(const Common::Path &drive) override;

private:
	struct Drive {
		Drive(const Common::Path &m, const Common::Path &d, const Common::String &f) :
			mountPoint(m), deviceName(d), fsType(f) {}

		Common::Path mountPoint;
		Common::Path deviceName;
		Common::String fsType;
	};

	typedef Common::Array<Drive> DriveList;
	DriveList detectAllDrives();

	bool findTrackNames(const Common::Path &drivePath);

	Common::HashMap<uint, Common::Path> _trackMap;
};

MacOSXAudioCDManager::~MacOSXAudioCDManager() {
	close();
}

bool MacOSXAudioCDManager::open() {
	close();

	if (openRealCD())
		return true;

	return DefaultAudioCDManager::open();
}

/**
 * Find the base disk number of device name.
 * Returns -1 if mount point is not /dev/disk*
 */
static int findBaseDiskNumber(const Common::Path &diskPath) {
	Common::String diskName(diskPath.toString('/'));
	if (!diskName.hasPrefix("/dev/disk"))
		return -1;

	const char *startPtr = diskName.c_str() + 9;
	char *endPtr;
	int baseDiskNumber = strtol(startPtr, &endPtr, 10);
	if (startPtr == endPtr)
		return -1;

	return baseDiskNumber;
}

bool MacOSXAudioCDManager::openCD(int drive) {
	DriveList allDrives = detectAllDrives();
	if (allDrives.empty())
		return false;

	DriveList cddaDrives;

	// Try to get the volume related to the game's path
	if (ConfMan.hasKey("path")) {
		Common::String gamePath = ConfMan.getPath("path").toString(Common::Path::kNativeSeparator);
		struct statfs gamePathStat;
		if (statfs(gamePath.c_str(), &gamePathStat) == 0) {
			int baseDiskNumber = findBaseDiskNumber(gamePathStat.f_mntfromname);
			if (baseDiskNumber >= 0) {
				// Look for a CDDA drive with the same base disk number
				for (uint32 i = 0; i < allDrives.size(); i++) {
					if (allDrives[i].fsType == "cddafs" && findBaseDiskNumber(allDrives[i].deviceName) == baseDiskNumber) {
						debug(1, "Preferring drive '%s'", allDrives[i].mountPoint.toString(Common::Path::kNativeSeparator).c_str());
						cddaDrives.push_back(allDrives[i]);
						allDrives.remove_at(i);
						break;
					}
				}
			}
		}
	}

	// Add the remaining CDDA drives to the CDDA list
	for (uint32 i = 0; i < allDrives.size(); i++)
		if (allDrives[i].fsType == "cddafs")
			cddaDrives.push_back(allDrives[i]);

	if (drive >= (int)cddaDrives.size())
		return false;

	debug(1, "Using '%s' as the CD drive", cddaDrives[drive].mountPoint.toString(Common::Path::kNativeSeparator).c_str());

	return findTrackNames(cddaDrives[drive].mountPoint);
}

bool MacOSXAudioCDManager::openCD(const Common::Path &drive) {
	DriveList drives = detectAllDrives();

	for (uint32 i = 0; i < drives.size(); i++) {
		if (drives[i].fsType != "cddafs")
			continue;

		if (drives[i].mountPoint == drive || drives[i].deviceName == drive) {
			debug(1, "Using '%s' as the CD drive", drives[i].mountPoint.toString(Common::Path::kNativeSeparator).c_str());
			return findTrackNames(drives[i].mountPoint);
		}
	}

	return false;
}

void MacOSXAudioCDManager::close() {
	DefaultAudioCDManager::close();
	_trackMap.clear();
}

enum {
	// Some crazy high number that we'll never actually hit
	kMaxDriveCount = 256
};

MacOSXAudioCDManager::DriveList MacOSXAudioCDManager::detectAllDrives() {
	// Fetch the lists of drives
	struct statfs driveStats[kMaxDriveCount];
	int foundDrives = getfsstat(driveStats, sizeof(driveStats), MNT_WAIT);
	if (foundDrives <= 0)
		return DriveList();

	DriveList drives;
	for (int i = 0; i < foundDrives; i++)
		drives.push_back(Drive(Common::Path(driveStats[i].f_mntonname, Common::Path::kNativeSeparator),
			Common::Path(driveStats[i].f_mntfromname, Common::Path::kNativeSeparator), driveStats[i].f_fstypename));

	return drives;
}

bool MacOSXAudioCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	// Prefer emulation
	if (DefaultAudioCDManager::play(track, numLoops, startFrame, duration, onlyEmulate, soundType))
		return true;

	// If we're set to only emulate, or have no CD drive, return here
	if (onlyEmulate || !_trackMap.contains(track))
		return false;

	if (!numLoops && !startFrame)
		return false;

	// Now load the AIFF track from the name
	Common::Path fileName = _trackMap[track];
	Common::SeekableReadStream *stream = StdioStream::makeFromPath(fileName.toString(Common::Path::kNativeSeparator).c_str(), StdioStream::WriteMode_Read);

	if (!stream) {
		warning("Failed to open track '%s'", fileName.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	Audio::AudioStream *audioStream = Audio::makeAIFFStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Track '%s' is not an AIFF track", fileName.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(audioStream);
	if (!seekStream) {
		warning("Track '%s' is not seekable", fileName.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	Audio::Timestamp start = Audio::Timestamp(0, startFrame, 75);
	Audio::Timestamp end = duration ? Audio::Timestamp(0, startFrame + duration, 75) : seekStream->getLength();

	// Fake emulation since we're really playing an AIFF file
	_emulating = true;

	_mixer->playStream(soundType, &_handle,
	                   Audio::makeLoopingAudioStream(seekStream, start, end, (numLoops < 1) ? numLoops + 1 : numLoops), -1, _cd.volume, _cd.balance);
	return true;
}

bool MacOSXAudioCDManager::findTrackNames(const Common::Path &drivePath) {
	Common::FSNode directory(drivePath);

	if (!directory.exists()) {
		warning("Directory '%s' does not exist", drivePath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	if (!directory.isDirectory()) {
		warning("'%s' is not a directory", drivePath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	Common::FSList children;
	if (!directory.getChildren(children, Common::FSNode::kListFilesOnly)) {
		warning("Failed to find children for '%s'", drivePath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	for (uint32 i = 0; i < children.size(); i++) {
		if (!children[i].isDirectory()) {
			Common::String fileName = children[i].getFileName();

			if (fileName.hasSuffix(".aiff") || fileName.hasSuffix(".cdda")) {
				uint j = 0;

				// Search for the track ID in the file name.
				for (; j < fileName.size() && !Common::isDigit(fileName[j]); j++)
					;

				const char *trackIDString = fileName.c_str() + j;
				char *endPtr = nullptr;
				long trackID = strtol(trackIDString, &endPtr, 10);

				if (trackIDString != endPtr && trackID > 0 && trackID < UINT_MAX) {
					_trackMap[trackID - 1] = drivePath.appendComponent(fileName);
				} else {
					warning("Invalid track file name: '%s'", fileName.c_str());
				}
			}
		}
	}

	return true;
}

AudioCDManager *createMacOSXAudioCDManager() {
	return new MacOSXAudioCDManager();
}

#endif // MACOSX
