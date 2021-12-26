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

#ifndef AUDIO_SOFTSYNTH_PCSPK_H
#define AUDIO_SOFTSYNTH_PCSPK_H

#include "audio/audiostream.h"
#include "common/mutex.h"
#include "common/queue.h"

namespace Audio {

class PCSpeaker : public AudioStream {
public:
	enum WaveForm {
		kWaveFormSquare = 0,
		kWaveFormSine,
		kWaveFormSaw,
		kWaveFormTriangle,
		kWaveFormSilence
	};

protected:
	// PC speaker instruction: play this waveform at frequency x for y microseconds.
	struct Command {
		WaveForm waveForm;
		float frequency;
		uint32 length;

		Command(WaveForm waveForm, float frequency, uint32 length);
	};

public:
	PCSpeaker(int rate = 44100);
	~PCSpeaker();

	/** Play a note for length ms.
	 *
	 *  If length is negative, play until told to stop.
	 */
	void play(WaveForm wave, int freq, int32 length);
	/**
	 * Queue the specified playback instruction. It will be executed when all
	 * previously queued instructions have finished. Use this method for
	 * playback of effects which require timing precision of less than a
	 * millisecond.
	 *
	 * Calling this method will terminate any waveform started with the play
	 * method. Calling the play method will terminate the active queued
	 * instruction and clear the instruction queue.
	 *
	 * Use isPlaying to check if all queued instructions have finished playing.
	 * This will return true even if the current instruction is "playing"
	 * silence.
	 * 
	 * @param wave The waveform to use. For PC speaker, use square wave or
	 * silence.
	 * @param freq The frequency (in Hertz) to play.
	 * @param lengthus The length in microseconds for which to play the
	 * waveform.
	 */
	void playQueue(WaveForm wave, float freq, uint32 lengthus);
	/** Stop the currently playing note after delay ms. */
	void stop(int32 delay = 0);
	/** Adjust the volume. */
	void setVolume(byte volume);

	bool isPlaying() const;

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const	{ return false; }
	bool endOfData() const	{ return false; }
	bool endOfStream() const { return false; }
	int getRate() const	{ return _rate; }

protected:
	Common::Mutex _mutex;

	int _rate;
	WaveForm _wave;
	bool _playForever;
	uint32 _oscLength;
	uint32 _oscSamples;
	uint32 _remainingSamples;
	uint32 _mixedSamples;
	byte _volume;

	// The queue of playback instructions.
	Common::Queue<Command> *_commandQueue;
	// True if a playback instruction is currently being executed. False if
	// current playback was started by the play method (or if there is no
	// playback at all).
	bool _commandActive;

	typedef int8 (*generatorFunc)(uint32, uint32);
	static const generatorFunc generateWave[];

	static int8 generateSquare(uint32 x, uint32 oscLength);
	static int8 generateSine(uint32 x, uint32 oscLength);
	static int8 generateSaw(uint32 x, uint32 oscLength);
	static int8 generateTriangle(uint32 x, uint32 oscLength);
	static int8 generateSilence(uint32 x, uint32 oscLength);
};

} // End of namespace Audio

#endif // AUDIO_SOFTSYNTH_PCSPEAKER_H
