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

#ifndef BACKENDS_AUDIOCD_DEFAULT_H
#define BACKENDS_AUDIOCD_DEFAULT_H

#include "backends/audiocd/audiocd.h"
#include "audio/mixer.h"

namespace Common {
class String;
} // End of namespace Common

/**
 * The default audio cd manager. Implements emulation of audio cd playback.
 */
class DefaultAudioCDManager : public AudioCDManager {
public:
	DefaultAudioCDManager();
	virtual ~DefaultAudioCDManager();

	virtual bool open();
	virtual void close();
	virtual bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate = false,
		Audio::Mixer::SoundType soundType = Audio::Mixer::kMusicSoundType);
	virtual void stop();
	virtual bool isPlaying() const;
	virtual void setVolume(byte volume);
	virtual void setBalance(int8 balance);
	virtual void update();
	virtual Status getStatus() const; // Subclasses should override for better status results
	virtual bool existExtractedCDAudioFiles(uint track);
	virtual bool isDataAndCDAudioReadFromSameCD() { return false; }

private:
	void fillPotentialTrackNames(Common::Array<Common::String> &trackNames, int track) const;

protected:
	/**
	 * Open a CD using the cdrom config variable
	 */
	bool openRealCD();

	/**
	 * Open a CD using the specified drive index
	 * @param drive The index of the drive
	 * @note The index is implementation-defined, but 0 is always the best choice
	 */
	virtual bool openCD(int drive) { return false; }

	/**
	 * Open a CD from a specific drive
	 * @param drive The name of the drive/path
	 * @note The drive parameter is platform-specific
	 */
	virtual bool openCD(const Common::String &drive) { return false; }

	Audio::SoundHandle _handle;
	bool _emulating;

	Status _cd;
	Audio::Mixer *_mixer;
};

#endif
