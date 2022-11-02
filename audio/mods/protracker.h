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

/**
 * @file
 * Sound decoder used in engines:
 *  - agos
 *  - parallaction
 *  - gob
 *  - hopkins
 *  - chewy (subclass)
 */

#ifndef AUDIO_MODS_PROTRACKER_H
#define AUDIO_MODS_PROTRACKER_H

#include "audio/mods/paula.h"
#include "audio/mods/module.h"

namespace Common {
class SeekableReadStream;
}

namespace Modules {

class ProtrackerStream : public ::Audio::Paula {
protected:
	Module *_module;

private:
	int _tick;
	int _row;
	int _pos;

	int _speed;
	int _bpm;

	// For effect 0xB - Jump To Pattern;
	bool _hasJumpToPattern;
	int _jumpToPattern;

	// For effect 0xD - PatternBreak;
	bool _hasPatternBreak;
	int _skipRow;

	// For effect 0xE6 - Pattern Loop
	bool _hasPatternLoop;
	int _patternLoopCount;
	int _patternLoopRow;

	// For effect 0xEE - Pattern Delay
	byte _patternDelay;

	static const int16 sinetable[];

	struct Track {
		byte sample;
		byte lastSample;
		uint16 period;
		Offset offset;

		byte vol;
		byte finetune;

		// For effect 0x0 - Arpeggio
		bool arpeggio;
		byte arpeggioNotes[3];

		// For effect 0x3 - Porta to note
		uint16 portaToNote;
		byte portaToNoteSpeed;

		// For effect 0x4 - Vibrato
		int vibrato;
		byte vibratoPos;
		byte vibratoSpeed;
		byte vibratoDepth;

		// For effect 0xED - Delay sample
		byte delaySample;
		byte delaySampleTick;
	} _track[4];

public:
	ProtrackerStream(Common::SeekableReadStream *stream, int offs, int rate, bool stereo);

protected:
	ProtrackerStream(int rate, bool stereo);

public:
	virtual ~ProtrackerStream();

	Modules::Module *getModule() {
		// Ordinarily, the Module is not meant to be seen outside of
		// this class, but occasionally, it's useful to be able to
		// manipulate it directly. The Hopkins engine uses this to
		// repair a broken song.
		return _module;
	}

private:
	void interrupt() override;

	void doPorta(int track);
	void doVibrato(int track);
	void doVolSlide(int track, byte ex, byte ey);

	void updateRow();
	void updateEffects();
};

}

namespace Audio {

class AudioStream;

/*
 * Factory function for ProTracker streams. Reads all data from the
 * given ReadStream and creates an AudioStream from this. No reference
 * to the 'stream' object is kept, so you can safely delete it after
 * invoking this factory.
 *
 * @param stream	the ReadStream from which to read the ProTracker data
 * @param rate		TODO
 * @param stereo	TODO
 * @param module	can be used to return the Module object (rarely useful)
 * @return	a new AudioStream, or NULL, if an error occurred
 */
AudioStream *makeProtrackerStream(Common::SeekableReadStream *stream, int offs = 0, int rate = 44100, bool stereo = true, Modules::Module **module = 0);

} // End of namespace Audio

#endif
