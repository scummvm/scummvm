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

#ifndef BACKENDS_AUDIOCD_ABSTRACT_H
#define BACKENDS_AUDIOCD_ABSTRACT_H

#include "audio/mixer.h"
#include "common/scummsys.h"
#include "common/noncopyable.h"

/**
* Abstract Audio CD manager class. Subclasses implement the actual
* functionality.
*/
class AudioCDManager : Common::NonCopyable {
public:
	virtual ~AudioCDManager() {}

	/**
	* A structure containing the current playback information
	*/
	struct Status {
		bool playing;
		int track;
		int start;
		int duration;
		int numLoops;
		int volume;
		int balance;
	};

	/**
	 * Initialize the specified CD drive for audio playback.
	 * @return true if the CD drive was inited successfully
	 */
	virtual bool open() = 0;

	/**
	 * Close the currently open CD drive
	 */
	virtual void close() = 0;

	/**
	 * Start audio CD playback
	 * @param track          the track to play.
	 * @param numLoops       how often playback should be repeated (<=0 means infinitely often).
	 * @param startFrame     the frame at which playback should start (75 frames = 1 second).
	 * @param duration       the number of frames to play.
	 * @param onlyEmulate    determines if the track should be emulated only
	 * @param soundType      What sound type to play as. By default, it's as music
	 * @note The @c onlyEmulate parameter is deprecated.
	 * @return @c true if the track started playing, @c false otherwise
	 */
	virtual bool play(int track, int numLoops, int startFrame, int duration, bool onlyEmulate = false,
		Audio::Mixer::SoundType soundType = Audio::Mixer::kMusicSoundType) = 0;

	/**
	 * Get if audio is being played.
	 * @return true if CD audio is playing
	 */
	virtual bool isPlaying() const = 0;

	/**
	 * Set the audio volume
	 */
	virtual void setVolume(byte volume) = 0;

	/**
	 * Set the speakers balance
	 */
	virtual void setBalance(int8 balance) = 0;

	/**
	 * Stop audio playback.
	 */
	virtual void stop() = 0;

	/**
	 * Update audio status.
	 */
	virtual void update() = 0;

	/**
	 * Get the playback status.
	 * @return a Status struct with playback data.
	 */
	virtual Status getStatus() const = 0;

	/**
	 * Checks whether the extracted audio cd tracks exists as files in
	 * the search paths.
	 * @return true if audio files of the expected naming scheme are found, and supported by ScummVM.
	 */
	virtual bool existExtractedCDAudioFiles(uint track) = 0;

	/**
	 * Checks if game data are read from the same CD drive which should also play game CD audio.
	 * @return true, if this case is applicable, and the system doesn't allow it.
	 */
	virtual bool isDataAndCDAudioReadFromSameCD() = 0;
};

#endif
