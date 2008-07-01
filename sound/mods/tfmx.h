/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SOUND_MODS_TFMX_H
#define SOUND_MODS_TFMX_H

#include "common/stream.h"
#include "common/file.h"
#include "sound/mods/paula.h"

namespace Audio {

class Tfmx : public Paula {
public:
		//constructor with appropiate default values and initialization
		Tfmx(bool stereo = false, int rate = 44100, int interruptFreq = 0);
		~Tfmx();

		//temporary loader function, will be moved to one below
		void load();
		//load function loads file from stream, performs checks, initializes some variables
		bool load(Common::SeekableReadStream &stream);
		
		//generic function to start playback
		bool play();
		
protected:
		byte *_data;      //buffer
		uint32 _dataSize; //buffer size

		uint32 _trackTableOffset;
		uint32 _patternTableOffset;
		uint32 _macroTableOffset;

		uint32 _patternPointers[128];
		uint32 _macroPointers[128]; 

		static const uint16 notes[]; //note table , needs verification

		struct Note {
			uint8 noteValue;
			uint8 macroNumber;
			uint8 volume;
			uint8 channelNumber;
			uint8 wait;
		};

		struct Song {
			uint16 startPosition;
			uint16 endPosition;
			uint16 tempoValue;
		}_songs[32];

		struct Channel {
			//empty 
			//need to implement channel structure properly
			//will have data to send to Paula via interrupt()
			uint8 crap;
		}_channels[4];

		struct Track {
			//empty
			//should setup as 8-track array 
			//each track gets updated as trackstep progresses at predefined speed
			uint8 crap2;
		}_trackz[8];

		//functions used in playback (in order by relationship)
		void loadSongs();
		void readTrackstep(uint8 songNumber);
		void readPattern(uint8 patternNumber);
		void readNote(Note _aNote);
		void readMacro(int _macroNumber);
		//trackstep functions
		void stopPlayer();
		void playSelection();
		void setTempo();
		void volumeSlide();

		//pattern functions + macro functions will either be handled as discrete functions
		//or handled directly in the readX functions.
		//F0 -> FF pattern commands
		//00 -> 29 macro commands

		//PAULA Interrupt override
		virtual void interrupt();

};//End of TFMX class
} // End of namespace Audio

#endif
