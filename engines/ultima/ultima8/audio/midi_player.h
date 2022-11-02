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

#ifndef ULTIMA8_AUDIO_MIDI_PLAYER_H
#define ULTIMA8_AUDIO_MIDI_PLAYER_H

#include "audio/mixer.h"
#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"

namespace Ultima {
namespace Ultima8 {

class MidiPlayer {
public:
	MidiPlayer();
	~MidiPlayer();

	/**
	 * Load the specified music data
	 */
	void load(byte *data, size_t size, int seqNo);

	/**
	 * Load the XMIDI data containing the transition tracks.
	 * Call this function before calling playTransition.
	 */
	void loadTransitionData(byte *data, size_t size);

	/**
	 * Play the specified music track, starting at the
	 * specified branch. Use branchNo -1 to start from the
	 * beginning.
	 */
	void play(int trackNo, int branchNo);

	/**
	 * Plays the specified transition track. If overlay is specified, the
	 * transition is overlaid on the currently playing music track and this
	 * track is stopped when the transition ends. If overlay is not specified,
	 * the currently playing music track is stopped before the transition is
	 * started.
	 */
	void playTransition(int trackNo, bool overlay);

	/**
	 * Stop the currently playing track.
	 */
	void stop();

	/**
	 * Pause or resume playback of the current track.
	 */
	void pause(bool pause);

	/**
	 * Returns true if a track is playing.
	 */
	bool isPlaying();

	/**
	 * Starts a fade-out of the specified duration (in milliseconds).
	 */
	void startFadeOut(uint16 length);

	/**
	 * Returns true if the music is currently fading.
	 */
	bool isFading();

	/**
	 * Synchronizes the user volume settings with those of the game.
	 */
	void syncSoundSettings();

	/**
	 * Sets whether the music should loop
	 */
	void setLooping(bool loop);

	/**
	 * Returns true if the current music track has a branch
	 * defined for the specified index.
	 */
	bool hasBranchIndex(uint8 index);

	bool isFMSynth() const {
		return _isFMSynth;
	};

	static void xmidiCallback(byte eventData, void *data);

	byte getSequenceCallbackData(int seq) const {
		assert(seq == 0 || seq == 1);
		return _callbackData[seq];
	}

	void onTimer();
	static void timerCallback(void *data);

private:
	MidiDriver_Multisource *_driver;
	MidiParser *_parser;
	MidiParser *_transitionParser;

	bool _isFMSynth;
	bool _playingTransition;
	static byte _callbackData[2];
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
