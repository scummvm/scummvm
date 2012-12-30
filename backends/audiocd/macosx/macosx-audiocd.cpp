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
 * The Mac OS X audio cd manager. Implements real audio cd playback.
 */
class MacOSXAudioCDManager : public DefaultAudioCDManager {
public:
	MacOSXAudioCDManager() {}
	~MacOSXAudioCDManager();

	void playCD(int track, int num_loops, int start_frame, int duration);
	void closeCD();

protected:
	bool openCD(int drive);

private:
	struct Drive {
		Drive(const Common::String &m, const Common::String &d, const Common::String &f) :
			mountPoint(m), deviceName(d), fsType(f) {}

		Common::String mountPoint;
		Common::String deviceName;
		Common::String fsType;
	};

	typedef Common::Array<Drive> DriveList;
	DriveList detectAllDrives();
	DriveList detectCDDADrives();

	bool findTrackNames(const Common::String &drivePath);

	Common::HashMap<uint, Common::String> _trackMap;
};

MacOSXAudioCDManager::~MacOSXAudioCDManager() {
	closeCD();
}

/**
 * Find the base disk number of device name.
 * Returns -1 if mount point is not /dev/disk*
 */
static int findBaseDiskNumber(const Common::String &diskName) {
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
	closeCD();

	DriveList allDrives = detectAllDrives();
	if (allDrives.empty())
		return false;

	DriveList cddaDrives;

	// Try to get the volume related to the game's path
	if (ConfMan.hasKey("path")) {
		Common::String gamePath = ConfMan.get("path");
		struct statfs gamePathStat;
		if (statfs(gamePath.c_str(), &gamePathStat) == 0) {
			int baseDiskNumber = findBaseDiskNumber(gamePathStat.f_mntfromname);
			if (baseDiskNumber >= 0) {
				// Look for a CDDA drive with the same base disk number
				for (uint32 i = 0; i < allDrives.size(); i++) {
					if (allDrives[i].fsType == "cddafs" && findBaseDiskNumber(allDrives[i].deviceName) == baseDiskNumber) {
						debug(1, "Preferring drive '%s'", allDrives[i].mountPoint.c_str());
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

	debug(1, "Using '%s' as the CD drive", cddaDrives[drive].mountPoint.c_str());

	return findTrackNames(cddaDrives[drive].mountPoint);
}

void MacOSXAudioCDManager::closeCD() {
	stop();
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
		drives.push_back(Drive(driveStats[i].f_mntonname, driveStats[i].f_mntfromname, driveStats[i].f_fstypename));

	return drives;
}

void MacOSXAudioCDManager::playCD(int track, int numLoops, int startFrame, int duration) {
	if (!_trackMap.contains(track) || (!numLoops && !startFrame))
		return;

	// Now load the AIFF track from the name
	Common::String fileName = _trackMap[track];
	Common::SeekableReadStream *stream = StdioStream::makeFromPath(fileName.c_str(), false);

	if (!stream) {
		warning("Failed to open track '%s'", fileName.c_str());
		return;
	}

	Audio::AudioStream *audioStream = Audio::makeAIFFStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Track '%s' is not an AIFF track", fileName.c_str());
		return;
	}

	Audio::SeekableAudioStream *seekStream = dynamic_cast<Audio::SeekableAudioStream *>(audioStream);
	if (!seekStream) {
		warning("Track '%s' is not seekable", fileName.c_str());
		return;
	}

	Audio::Timestamp start = Audio::Timestamp(0, startFrame, 75);
	Audio::Timestamp end = duration ? Audio::Timestamp(0, startFrame + duration, 75) : seekStream->getLength();

	// Fake emulation since we're really playing an AIFF file
	_emulating = true;

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle,
			Audio::makeLoopingAudioStream(seekStream, start, end, (numLoops < 1) ? numLoops + 1 : numLoops), -1, _cd.volume, _cd.balance);
}

bool MacOSXAudioCDManager::findTrackNames(const Common::String &drivePath) {
	Common::FSNode directory(drivePath);

	if (!directory.exists()) {
		warning("Directory '%s' does not exist", drivePath.c_str());
		return false;
	}

	if (!directory.isDirectory()) {
		warning("'%s' is not a directory", drivePath.c_str());
		return false;
	}

	Common::FSList children;
	if (!directory.getChildren(children, Common::FSNode::kListFilesOnly)) {
		warning("Failed to find children for '%s'", drivePath.c_str());
		return false;
	}

	for (uint32 i = 0; i < children.size(); i++) {
		if (!children[i].isDirectory()) {
			Common::String fileName = children[i].getName();

			if (fileName.hasSuffix(".aiff") || fileName.hasSuffix(".cdda")) {
				uint trackID = 0, j = 0;

				for (; j < fileName.size() && !Common::isDigit(fileName[j]); j++)
					;

				for (; j < fileName.size() && Common::isDigit(fileName[j]); j++)
					trackID = trackID * 10 + (fileName[j] - '0');

				_trackMap[trackID - 1] = drivePath + '/' + fileName;
			}
		}
	}

	return true;
}

AudioCDManager *createMacOSXAudioCDManager() {
	return new MacOSXAudioCDManager();
}
