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

#ifndef BACKENDS_AUDIOCD_ATARI_H
#define BACKENDS_AUDIOCD_ATARI_H

#include "backends/audiocd/default/default-audiocd.h"

#include <mint/cdromio.h>
#include <mint/metados.h>

#include "common/array.h"

/**
 * The Atari MetaDOS audio cd manager. Implements real audio cd playback
 * via the MetaDOS BOS drivers.
 */
class AtariAudioCDManager final : public DefaultAudioCDManager {
public:
	AtariAudioCDManager();
	~AtariAudioCDManager() override;

	bool open() override;
	void close() override;
	bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate,
		Audio::Mixer::SoundType soundType) override;
	void stop() override;
	bool isPlaying() const override;
	void update() override;
	bool existExtractedCDAudioFiles(uint track) override;

protected:
	bool openCD(int drive) override;

private:
	enum { kMaxDrives = 32 };

	struct Drive {
		char letter;          // Drive letter ('A'..'Z'), 0 if absent
		metaopen_t metaopen;  // Filled in while the drive is open
	};

	bool loadTOC();
	int ioctl(int command, void *arg) const;
	bool startPlayback(int track, int startFrame, int duration);

	Drive _drives[kMaxDrives];
	int _numDrives;
	int _drive;  // Index into _drives, -1 if no drive is open

	// GEMDOS-to-BOS drive mapping table from metainit.info->log2phys
	// (NULL if MetaDOS didn't supply one)
	const char *_log2phys;

	Common::Array<cdrom_tocentry> _tocEntries;  // Includes the leadout as the final entry

	int _cdTrack;
	int _cdNumLoops;
	int _cdStartFrame;
	int _cdDuration;
	uint32 _cdEndTime;
	uint32 _cdStopTime;
};

#endif
