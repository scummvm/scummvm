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
		//TODO:: Ctor + Dtor might need some repair/cleanup
		Tfmx(bool stereo = false, int rate = 44100, int interruptFreq = 0);
		~Tfmx();
	
		//TODO:: Will change to bool load(Common::SeekableReadStream &stream) 
		//TODO:: Will change to bool loadSamples(Common::SeekableReadStream &stream)
		void load();
		void loadSamples();

		//After the TFMX file is loaded, you chose which song to playback.
		//The song will end automatically or when you call stop().
		void playSong(uint8 songNumber);
		void stop();
		
		//DEBUGGING FUNCTION:: Temporary function to test individual macros for playback
		void testMacro(uint8 macroNumber);
		void testPattern(uint8 patternNumber);
		bool loadSong(uint8 songNumber); //temporarly public
protected:
		//DEBUGGING::
		bool _macroTest;
		bool _patternTest;
	
		//uint8 stream for whole MDAT file
		uint8 *_data;      
		uint32 _dataSize; 

		//uint8 stream for whole SMPL file
		int8 *_sampleData;
		uint32 _sampleSize;

		//addresses of tables in MDAT file
		uint32 _trackTableOffset;
		uint32 _patternTableOffset;
		uint32 _macroTableOffset;

		//addresses of patterns and macros from MDAT file
		uint32 _patternPointers[128];
		uint32 _macroPointers[128];  

		//uint16 stream for current song trackstep
		uint16 *_trackData;
		uint32 _trackCount;
		uint32 _trackLength;
		bool _trackAdvance;
		bool _trackEnd;
		uint16 _tempo; //current value for tempo
		
		//note table
		static const uint16 periods[]; 

		//Song structure
		struct Song {
			uint16 startPosition;
			uint16 endPosition;
			uint16 tempoValue;
		}_songs[32];
		
		//Pattern structure
		struct Pattern {
			uint32 *data;
			uint32 patternCount;
			uint32 patternLength;
			uint8 patternWait;
			uint8 patternTranspose;
			bool newPattern;
		//	bool patternEnd;
		//	uint16 offset;
		//	uint8 saveNumber1;
		//	uint8 saveNumber2;
		//	bool jumpFlag;
		//	bool returnFlag;
		//	bool loopFlag;
		//	uint16 loopCount;
		};

		//Macro structure
		struct Macro {
			uint32 *data;
			uint32 macroCount;
			uint32 macroLength;
			uint16 macroWait; //internal wait
			//external note stuff here
			uint8 noteNumber;
			uint16 notePeriod;
			uint8 noteVelocity;
			uint8 noteChannel;
			uint8 noteType;
			uint8 noteWait;  //external wait
			int8 noteFineTune;
			float fineTune;
			bool keyUp;
			uint8 keyCount;
			uint8 keyWait;
		};

		//Track structure
		struct Track {
			uint16 data;
			bool trackOn;
			bool patternOn;
			bool macroOn;
			uint8 patternNumber;
			uint8 macroNumber;
			Pattern activePattern;
			Macro activeMacro;
		//	uint16 volume;
		//	bool loopFlag;
		//	uint16 loopCount;
		}_tracks[8];
		
		//Channel structure
		struct Channel {
			uint8 period;
			int8 volume;
			uint32 sampleOffset;
			uint32 sampleLength;
			//int8 *dataRepeat;
			//uint32 lengthRepeat;
			bool sampleOn;
			bool updateOn;
			//Envelope Effect
			bool envelopeOn;
			int8 envelopeTarget;
			uint8 envelopeRate;
			uint8 envelopeSpeed;
			uint8 envelopeCount;
			//Vibrato Effect
			bool vibratoOn;
			bool vibratoDirection;
			int8 vibratoRate;
			uint8 vibratoSpeed;
			uint8 vibratoCount;
		}_channels[4];

		//PAULA Interrupt override
		virtual void interrupt(void);

		//bool loadSong(uint8 songNumber);
		void updateTrackstep();
		void loadPattern(uint8 trackNumber, uint8 patternNumber);
		void updatePattern(uint8 trackNumber);
		void loadMacro(uint8 trackNumber, uint8 macroNumber);
		void doMacro(uint8 trackNumber);
		void doEffects(uint8 channelNumber);
		void runMacro(uint8 trackNumber);
		
		//Trackstep functions
		void setTempo();
		void volumeSlide();

};//End of Tfmx class
} //End of namespace Audio

#endif
