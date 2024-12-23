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

#ifndef SCUMM_SOUNDCD_H
#define SCUMM_SOUNDCD_H

#include "backends/audiocd/audiocd.h"
#include "common/scummsys.h"
#include "common/serializer.h"

namespace Scumm {

// The number of "ticks" (1/10th of a second) into the Overture that the
// LucasFilm logo should appear. This corresponds to a timer value of 204.
// The default value is selected to work well with the Ozawa recording.

#define DEFAULT_LOOM_OVERTURE_TRANSITION 1160

class ScummEngine;
class SoundSE;

class SoundCD {
private:
	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	SoundSE *_soundSE;

	Audio::SoundHandle *_fileBasedCDAudioHandle;
	bool _hasFileBasedCDAudio = false;
	AudioCDManager::Status _fileBasedCDStatus;

	int _musicTimer = 0;

	bool _useReplacementAudioTracks = false;
	int _loomOvertureTransition = 0;
	uint32 _replacementTrackStartTime = 0;

public:
	int16 _currentCDSound = 0;
	uint32 _cdMusicTimerMod = 0;
	uint32 _cdMusicTimer = 0;
	bool _soundsPaused = false;

	SoundCD(ScummEngine *parent, Audio::Mixer *mixer, SoundSE *soundSE, bool useReplacementAudioTracks);
	~SoundCD();

	bool triggerCDSound(int soundID);
	void stopCDSound(int sound);
	void stopAllCDSounds();
	void pauseCDSounds(bool pause);

	bool useReplacementAudio() const { return _useReplacementAudioTracks; }
	void updateMusicTimer();
	int getMusicTimer() const { return _musicTimer; }
	int getCDMusicTimer() const { return _cdMusicTimer; }

	void startCDTimer();
	void stopCDTimer();

	void playCDTrack(int track, int numLoops, int startFrame, int duration);
	void stopCD();
	int pollCD() const;
	void updateCD();
	AudioCDManager::Status getCDStatus();
	int getCurrentCDSound() const { return _currentCDSound; }

	void restoreAfterLoad();
	void restoreCDAudioAfterLoad(AudioCDManager::Status &info);

	bool isRolandLoom() const;

private:
	int getReplacementAudioTrack(int soundID);
	void playCDTrackInternal(int track, int numLoops, int startFrame, int duration);
	int getCDTrackIdFromSoundId(int soundId, int &loops, int &start);
};

} // End of namespace Scumm

#endif
