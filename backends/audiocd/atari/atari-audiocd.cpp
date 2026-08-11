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
 * Based on SDL's MetaDOS CD-ROM driver by Patrice Mandin.
 *
 */

#define FORCE_TEXT_CONSOLE

#include "backends/audiocd/atari/atari-audiocd.h"

#include <errno.h>
#include <string.h>

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

// Some ioctl() errno values which occur when the tray is empty
#define ERRNO_TRAYEMPTY(err) \
	((err) == -EIO || (err) == -ENOENT || (err) == -EINVAL || (err) == -ENOMEDIUM)

enum {
	kFramesPerSecond = 75,
	kSecondsPerMinute = 60
};

static int msfToFrames(const cd_ad &addr) {
	return ((int)addr.msf.minute * kSecondsPerMinute + addr.msf.second)
		* kFramesPerSecond + addr.msf.frame;
}

static void framesToMsf(int frames, cdrom_msf &msf, bool start) {
	byte frame = frames % kFramesPerSecond;
	frames /= kFramesPerSecond;
	byte second = frames % kSecondsPerMinute;
	frames /= kSecondsPerMinute;
	byte minute = frames;

	if (start) {
		msf.cdmsf_min0 = minute;
		msf.cdmsf_sec0 = second;
		msf.cdmsf_frame0 = frame;
	} else {
		msf.cdmsf_min1 = minute;
		msf.cdmsf_sec1 = second;
		msf.cdmsf_frame1 = frame;
	}
}

AtariAudioCDManager::AtariAudioCDManager()
	: _numDrives(0),
	  _drive(-1),
	  _log2phys(nullptr),
	  _cdTrack(0),
	  _cdNumLoops(0),
	  _cdStartFrame(0),
	  _cdDuration(0),
	  _cdEndTime(0),
	  _cdStopTime(0) {

	memset(_drives, 0, sizeof(_drives));

	metainit_t metainit = { 0, 0, 0, 0 };
	Metainit(&metainit);

	if (metainit.version == nullptr) {
		debug("AtariAudioCDManager: MetaDOS not installed");
		return;
	}

	if (metainit.info != nullptr)
		_log2phys = metainit.info->log2phys;

	if (metainit.drives_map == 0) {
		debug("AtariAudioCDManager: no MetaDOS devices present");
		return;
	}

	// Probe each drive listed in the bitmap for a working CD subchannel ioctl
	for (int i = 'A'; i <= 'Z' && _numDrives < kMaxDrives; ++i) {
		if (!(metainit.drives_map & (1L << (i - 'A'))))
			continue;

		metaopen_t metaopen;
		if (Metaopen(i, &metaopen) != 0)
			continue;

		cdrom_subchnl info;
		info.cdsc_format = CDROM_MSF;
		long err = Metaioctl(i, METADOS_IOCTL_MAGIC, CDROMSUBCHNL, &info);
		if (err == 0 || ERRNO_TRAYEMPTY(err)) {
			_drives[_numDrives].letter = i;
			++_numDrives;
		}

		Metaclose(i);
	}

	debug("AtariAudioCDManager: detected %d MetaDOS CD-ROM drive(s)", _numDrives);
}

AtariAudioCDManager::~AtariAudioCDManager() {
	close();
}

bool AtariAudioCDManager::open() {
	close();

	if (openRealCD())
		return true;

	return DefaultAudioCDManager::open();
}

bool AtariAudioCDManager::openCD(int drive) {
	if (drive < 0 || drive >= _numDrives)
		return false;

	const char letter = _drives[drive].letter;
	if (Metaopen(letter, &_drives[drive].metaopen) != 0)
		return false;

	_drive = drive;

	if (!loadTOC()) {
		close();
		return false;
	}

	debug("AtariAudioCDManager: opened CD drive %c:", letter);
	return true;
}

void AtariAudioCDManager::close() {
	DefaultAudioCDManager::close();

	if (_drive >= 0) {
		// Make sure playback stops before we close the drive
		ioctl(CDROMSTOP, nullptr);
		Metaclose(_drives[_drive].letter);
		_drive = -1;
	}

	_tocEntries.clear();
	_cdNumLoops = 0;
	_cdEndTime = 0;
	_cdStopTime = 0;
}

int AtariAudioCDManager::ioctl(int command, void *arg) const {
	if (_drive < 0)
		return -1;

	long ret = Metaioctl(_drives[_drive].letter, METADOS_IOCTL_MAGIC, command, arg);
	if (ret < 0)
		warning("AtariAudioCDManager: ioctl(0x%x) failed: %s", command, strerror(-ret));
	return (int)ret;
}

bool AtariAudioCDManager::loadTOC() {
	cdrom_tochdr toc;
	if (ioctl(CDROMREADTOCHDR, &toc) < 0)
		return false;

	const int numTracks = toc.cdth_trk1 - toc.cdth_trk0 + 1;
	if (numTracks < 1)
		return false;

	_tocEntries.clear();
	_tocEntries.reserve(numTracks + 1);

	for (int i = 0; i <= numTracks; ++i) {
		cdrom_tocentry entry;
		memset(&entry, 0, sizeof(entry));
		entry.cdte_track = (i == numTracks) ? CDROM_LEADOUT : (toc.cdth_trk0 + i);
		entry.cdte_format = CDROM_MSF;

		if (ioctl(CDROMREADTOCENTRY, &entry) < 0) {
			_tocEntries.clear();
			return false;
		}

		_tocEntries.push_back(entry);
	}

	return true;
}

bool AtariAudioCDManager::startPlayback(int track, int startFrame, int duration) {
	const int trackStart = msfToFrames(_tocEntries[track].cdte_addr);
	const int trackEnd = msfToFrames(_tocEntries[track + 1].cdte_addr);

	const int startAbs = trackStart + startFrame;
	const int endAbs = (duration == 0) ? trackEnd : (startAbs + duration);

	if (endAbs <= startAbs || startAbs < trackStart)
		return false;

	cdrom_msf playtime;
	framesToMsf(startAbs, playtime, true);
	// The end MSF passed to CDROMPLAYMSF is inclusive
	framesToMsf(endAbs - 1, playtime, false);

	const int ret = ioctl(CDROMPLAYMSF, &playtime);
	if (ret < 0) {
		warning("AtariAudioCDManager: CDROMPLAYMSF on track %d [%d, %d) failed: %s",
			track, startAbs, endAbs, strerror(-ret));
		return false;
	}

	const int playFrames = endAbs - startAbs;
	_cdEndTime = g_system->getMillis() + (uint32)playFrames * 1000u / kFramesPerSecond;
	return true;
}

bool AtariAudioCDManager::play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) {
	// Prefer emulation
	if (DefaultAudioCDManager::play(track, numLoops, startFrame, duration, onlyEmulate, soundType))
		return true;

	// If we're set to only emulate, or have no CD, return here
	if (onlyEmulate || _drive < 0)
		return false;

	if (!numLoops && !startFrame)
		return false;

	if (track < 0 || track >= (int)_tocEntries.size() - 1) {
		warning("AtariAudioCDManager: no such track %d", track);
		return false;
	}

	// Bail out if the track isn't an audio track
	if (_tocEntries[track].cdte_ctrl & CDROM_DATA_TRACK) {
		warning("AtariAudioCDManager: track %d is not audio", track);
		return false;
	}

	// Pad the play range by five CDDA frames; introduced for MI1 CD audio
	// in commit 32d69e8c ("Fix MI1 CD Audio (hopefully)") against the SDL
	// backend, which drove a real CD drive the same way we do here.
	if (duration > 0)
		duration += 5;

	if (!startPlayback(track, startFrame, duration))
		return false;

	_cdTrack = track;
	_cdNumLoops = numLoops;
	_cdStartFrame = startFrame;
	_cdDuration = duration;
	_cdStopTime = 0;
	return true;
}

void AtariAudioCDManager::stop() {
	DefaultAudioCDManager::stop();

	// Stop CD audio in 1/10th of a second
	_cdStopTime = g_system->getMillis() + 100;
	_cdNumLoops = 0;
}

bool AtariAudioCDManager::isPlaying() const {
	if (DefaultAudioCDManager::isPlaying())
		return true;

	if (_drive < 0 || _cdNumLoops == 0)
		return false;

	if (g_system->getMillis() < _cdEndTime)
		return true;

	cdrom_subchnl info;
	info.cdsc_format = CDROM_MSF;
	if (ioctl(CDROMSUBCHNL, &info) < 0)
		return false;

	return info.cdsc_audiostatus == CDROM_AUDIO_PLAY;
}

bool AtariAudioCDManager::existExtractedCDAudioFiles(uint track) {
	// Extracted track files in the search path take priority
	if (DefaultAudioCDManager::existExtractedCDAudioFiles(track))
		return true;

	// Otherwise treat the physical drive as the source: if the game data
	// path lives on a MetaDOS CD-ROM drive we detected, the engine can
	// rely on AtariAudioCDManager::play() to drive the disc directly and
	// the "please rip your tracks" warning is misleading.
	if (_numDrives == 0 || !ConfMan.hasKey("path"))
		return false;

	const Common::String gamePath = ConfMan.getPath("path").toString(Common::Path::kNativeSeparator);
	if (gamePath.size() < 2 || gamePath[1] != ':')
		return false;

	const char letter = (gamePath[0] >= 'a' && gamePath[0] <= 'z')
		? (gamePath[0] - 'a' + 'A') : gamePath[0];

	// Direct match (GEMDOS letter == BOS letter)
	for (int i = 0; i < _numDrives; ++i)
		if (_drives[i].letter == letter)
			return true;

	// GEMDOS letter resolves through MetaDOS log2phys to a BOS letter
	// (e.g. ExtenDOS unidrive.bos with "p:z" presents BOS Z: as GEMDOS P:).
	// log2phys[] is indexed by GEMDOS drive number (A=0..Z=25) and stores
	// the BOS letter as a raw ASCII byte (or 0 if unmapped).
	if (_log2phys != nullptr && letter >= 'A' && letter <= 'Z') {
		const char bos = _log2phys[letter - 'A'];
		if (bos >= 'A' && bos <= 'Z') {
			for (int i = 0; i < _numDrives; ++i)
				if (_drives[i].letter == bos)
					return true;
		}
	}

	return false;
}

void AtariAudioCDManager::update() {
	DefaultAudioCDManager::update();

	if (_drive < 0)
		return;

	const uint32 now = g_system->getMillis();

	if (_cdStopTime != 0 && now >= _cdStopTime) {
		ioctl(CDROMSTOP, nullptr);
		_cdNumLoops = 0;
		_cdStopTime = 0;
		return;
	}

	if (_cdNumLoops == 0 || now < _cdEndTime)
		return;

	// Make sure the drive has really finished before we restart the track
	cdrom_subchnl info;
	info.cdsc_format = CDROM_MSF;
	const bool stillPlaying = (ioctl(CDROMSUBCHNL, &info) == 0)
		&& info.cdsc_audiostatus == CDROM_AUDIO_PLAY;

	if (_cdNumLoops != 1 && stillPlaying) {
		// Wait another second for it to be done
		_cdEndTime += 1000;
		return;
	}

	if (_cdNumLoops > 0)
		--_cdNumLoops;

	if (_cdNumLoops != 0)
		startPlayback(_cdTrack, _cdStartFrame, _cdDuration);
}
