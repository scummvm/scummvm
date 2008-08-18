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
		Tfmx(bool stereo = false, int rate = 44100, int interruptFreq = 0);
		~Tfmx();
	
		//TODO:: Will change to bool load(Common::SeekableReadStream &stream) 
		//TODO:: Will change to bool loadSamples(Common::SeekableReadStream &stream)
		void load();
		void loadSamples();

		//After loading the data file and the sample file, you chose which song to playback.
		//The playback will terminate automatically or when you call stop().
		void playSong(uint8 songNumber);
		void stop();
		
		//DEBUGGING FUNCTIONS:: Function to test individual macros/patterns for playback.
		void testMacro(uint8 macroNumber);
		void testPattern(uint8 patternNumber);
		
protected:
		//DEBUGGING::
		bool _macroTest;
		bool _patternTest;
	
		//UINT8 Stream for data file.
		uint8 *_data;      
		uint32 _dataSize; 

		//UINT8 Stream for sample file.
		int8 *_sampleData;
		uint32 _sampleSize;

		//Addresses of tables in the data file.
		uint32 _trackTableOffset;
		uint32 _patternTableOffset;
		uint32 _macroTableOffset;

		//Pointers to the patterns and macros in the data file.
		uint32 _patternPointers[128];
		uint32 _macroPointers[128];  

		//Current song trackstep steam and flags.
		uint16 *_trackData;
		uint32 _trackCount;
		uint32 _trackLength;
		uint16 _tempo; 
		bool _trackAdvance;
		bool _trackEnd;
		
		//UINT16 Period Table
		static const uint16 periods[]; 

		//Addresses of maximum 32 possible song stored in data file.
		struct Song {
			uint16 startPosition;
			uint16 endPosition;
			uint16 tempoValue;
		}_songs[32];
		
		//Macro structure, one active macro.
		struct Macro {
			uint32 *data;
			uint32 macroCount;
			uint32 macroLength;
			uint16 macroWait; //Internal wait specified by the macro.
			uint8 noteNumber;
			uint16 notePeriod;
			uint8 noteVelocity;
			uint8 noteChannel;
			uint8 noteType;
			uint8 noteWait; //External wait specified by the pattern. 
			int8 noteFineTune;
			float fineTune;
			//The members below are used for key-up looping.
			bool keyWaitOn;
			int8 keyCount;
			int8 keyWait;
			uint32 positionLoaded; //Pattern count where the macro was loaded.
		};

		//Pattern structure, one active pattern.
		struct Pattern {
			uint32 *data;
			uint32 patternCount;
			uint32 patternLength;
			uint8 patternWait;
			uint8 patternTranspose;
			bool newPattern;
		//TODO:: Add members for pattern jumping/go to commands (not currently used in Monkey Island).
		//TODO:: Add members for pattern effects (not currently used in Monkey Island).
		};

		//Track structure, 8 tracks.
		struct Track {
			uint16 data;
			bool trackOn;
			bool patternOn;
			bool macroOn;
			uint8 patternNumber;
			uint8 macroNumber;
			Pattern activePattern;
			Macro activeMacro;
		//TODO:: Add members for other trackstep commands (not currently used in Monkey Island).
		}_tracks[8];

		//Channel structure, 4 channels used
		struct Channel {
			uint16 period;
			int8 volume;
			uint32 sampleOffset;
			uint32 sampleLength;
			bool sampleOn;
			bool updateOn;
			bool keyUp;
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

		//PAULA Interrupt override.
		virtual void interrupt(void);
        
		//Loading functions.
		bool loadSong(uint8 songNumber); 
		void loadPattern(uint8 trackNumber, uint8 patternNumber);
		void loadMacro(uint8 trackNumber, uint8 macroNumber);
		
		//Update cycle functions.
		void updateTrackstep();
		void updatePattern(uint8 trackNumber);
		void doMacro(uint8 trackNumber);
		void doEffects(uint8 channelNumber);
		
		//Trackstep commands.
		void setTempo();
		void volumeSlide();

};//End of Tfmx class
} //End of namespace Audio

#endif
