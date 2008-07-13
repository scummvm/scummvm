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
		void loadSamples();

		//load function loads file from stream, performs checks, initializes some variables
		bool load(Common::SeekableReadStream &stream);
		bool loadSamples(Common::SeekableReadStream &stream);

		//generic function to start playback
		//will likely change to playSong(uint8 songNumber)
		bool play();
		
protected:
		//uint8 stream for whole MDAT file
		uint8 *_data;      
		uint32 _dataSize; 

		//uint8 stream for sample data from SMPL file
		uint8 *_sampleData;
		uint32 _sampleSize;

		//addresses of tables in MDAT file
		uint32 _trackTableOffset;
		uint32 _patternTableOffset;
		uint32 _macroTableOffset;

		//addresses of patterns and macros in MDAT file
		uint32 _patternPointers[128];
		uint32 _macroPointers[128]; 

		//uint16 stream for current song trackstep
		//need count and length to keep running tally
		uint16 *_trackData;
		uint32 _trackCount;
		uint32 _trackLength;
		uint16 _tempo; //current value for tempo

		//note table , needs verification
		static const uint16 notes[]; 

		//Song structure
		struct Song {
			uint16 startPosition;
			uint16 endPosition;
			uint16 tempoValue;
		}_songs[32];
		
		//Note structure
		struct Note {
			uint8 noteNumber;
			uint8 macroNumber;
			uint8 channelNumber;
			uint8 volume;
			uint8 wait;
			uint8 type; //4 types of notes
		};

		//Pattern structure; contains note
		struct Pattern {
			uint32 *data;
			uint8 number;
			uint8 transpose;
			uint32 count;
			bool newFlag;
			bool jumpFlag;
			bool returnFlag;
			Note note;
			uint8 wait;
			uint16 offset;
			uint8 saveNumber1;
			uint8 saveNumber2;
		//	bool loopFlag;
		//	uint16 loopCount;
		};

		//Sample structure; initialized by macro commands
		struct Sample {
			int8 *data;
			uint32 offset; //offset into sample file
			uint32 length; //length of sample

		};

		//Track structure; contains pattern
		//Setup as 8-track array, each track gets updated on interrupt
		struct Track {
			uint16 data;
			bool updateFlag;
			bool activeFlag;
			Sample sample;
			Pattern pattern;
		//	uint16 volume;
		//	bool loopFlag;
		//	uint16 loopCount;
		}_tracks[8];
		
		//Channel structure
		//TODO: Need to setup the nessecary information in channel to pass to PAULA via interrupt()
		struct Channel {
			uint8 period;
		}_channels[4];

		//TODO: Will likely need to add more data members to structures
		//for effects and macro proccessing. 

		//functions used in playback (in order by relationship)
		void playSong(uint8 songNumber);
		void updateTrackstep();
		void updatePattern(uint8 trackNumber);
		void updateNote(uint8 trackNumber);
		void doMacros(uint8 trackNumber);
		
		//trackstep functions
		void stopPlayer();
		void setTempo();
		void volumeSlide();

		//PAULA Interrupt override
		virtual void interrupt();

		//Debugging function to dump information to console
		void dumpTracks();

};//End of TFMX class
} // End of namespace Audio

#endif
