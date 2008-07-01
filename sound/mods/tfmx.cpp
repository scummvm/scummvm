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

#include "sound/mods/tfmx.h"
#include "sound/mods/paula.h"

namespace Audio {

const uint16 Tfmx::notes[] =
	{0x06AE,0x064E,0x05F4,0x059E,0x054D,0x0501,
	 0x04B9,0x0475,0x0435,0x03F9,0x03C0,0x038C,0x0358,0x032A,0x02FC,0x02D0,0x02A8,0x0282,
	 0x025E,0x023B,0x021B,0x01FD,0x01E0,0x01C6,0x001AC,0x0194,0x017D,0x0168,0x0154,0x0140,
	 0x012F,0x011E,0x010E,0x00FE,0x00F0,0x00E3,0x00D6,0x00CA,0x00BF,0x00B4,0x00AA,0x00A0,
	 0x0097,0x008F,0x0087,0x007F,0x0078,0x0071,0x00D6,0x00CA,0x00BF,0x00B4,0x00AA,0x00A0,
	 0x0097,0x008F,0x0087,0x007F,0x0078,0x0071,0x00D6,0x00CA,0x00BF,0x00B4};

Tfmx::Tfmx(bool stereo, int rate, int interruptFreq)
	: Paula(stereo, rate, interruptFreq) {
	//blank now
}

Tfmx::~Tfmx() {
	if (_data)
		delete[] _data;
}

void Tfmx::load() {
	// FIXME: temporary loader - just creates seekablereadstream from c:\mk.mdat
	Common::SeekableReadStream *stream = NULL;
	Common::File myfile;
	myfile.addDefaultDirectory("C:");
	myfile.open("mk.mdat");
	stream = myfile.readStream(myfile.size());
	myfile.close();
	//FIXME: end temporary loader. normally the seekablereadstream will be function parameter

	_dataSize = stream->size();
	_data = new uint8[_dataSize];
	stream->seek(0);
	stream->read(_data, _dataSize);

	Common::MemoryReadStream dataStream(_data, _dataSize);
	//should implement a check here to read header and ensure TFMXness

	dataStream.seek(256);
	// TODO: should be able to put in one loop??
	//will also later to modify to dynamically scale arrays for smaller TFMX files
	//in the case of Monkey Island, it uses 22/32 songs, and 128/128 for patterns/macros
	for (int i = 0; i < 32; i++) {
		_songs[i].startPosition = dataStream.readUint16BE();
	}
	for (int i = 0; i < 32; i++) {
		_songs[i].endPosition = dataStream.readUint16BE(); 
	}
	for (int i = 0; i < 32; i++) {
		_songs[i].tempoValue = dataStream.readUint16BE();
	}
	
	//read table positions specified at $01D0 in file (packed module)
	dataStream.skip(16);
	_trackTableOffset = dataStream.readUint32BE();
	_patternTableOffset = dataStream.readUint32BE();
	_macroTableOffset = dataStream.readUint32BE();
	
	//unpacked module specification
	if (!_trackTableOffset) 
		_trackTableOffset = 2048;    //$800 position
	if (!_patternTableOffset)
		_patternTableOffset = 1024; //$400 position
	if (!_macroTableOffset)
		_macroTableOffset = 1536;   //$600 position

	//skip to positon of pointer tables
	dataStream.seek(_patternTableOffset, SEEK_SET);
	for (int i = 0; i < 128; i++) {
		_patternPointers[i] = dataStream.readUint32BE();
	}
	dataStream.seek(_macroTableOffset, SEEK_SET);
	for (int i = 0; i < 128; i++) {
		_macroPointers[i] = dataStream.readUint32BE();
	}
	
	//trackstep read test
	//readTrackstep( 0 );

	//pattern read test
	readPattern(12);
}
bool Tfmx::load(Common::SeekableReadStream &stream) {
	return true;
}
bool Tfmx::play() {
	return true;
}
void Tfmx::loadSongs() {
}
void Tfmx::readTrackstep(uint8 songNumber) {
	if (songNumber >= 32) {
		//TODO: error, not a valid index
	}

	uint32 startPosition;   //offset into file from start for trackstart
	uint32 endPosition;     //offset into file from start for trackend
	int32 numCommands;		//number of 1 word track commands or patterns
	int32 numSteps;         //number of tracksteps; not yet useful
	startPosition = (_songs[songNumber].startPosition * 16) + _trackTableOffset;
	//the file specifies the start position of the last line, so you need to add 16
	//to get to the actual position where the track ends
	endPosition = (_songs[songNumber].endPosition * 16) + _trackTableOffset + 16;
	numCommands = (endPosition - startPosition) / 2;
	numSteps = numCommands / 8;

	Common::MemoryReadStream dataStream(_data, _dataSize);
	Common::SeekableSubReadStream trackSubStream(&dataStream, startPosition, endPosition);

	//read entire track by each 1 word command into an array - could just directly manipulate stream
	//can then read to determine if it is a pattern or a command
	//temporary solution - will need to organize by the 8-track per trackstep structure
	uint16 *tracks;
	tracks = new uint16[numCommands];
	for (int i = 0; i < numCommands; i++) {
		tracks[i] = trackSubStream.readUint16BE();
	}
	//you can now read track[] command by command. Maybe trackSubStream can be deleted?

	//start track reading
	//need to implement tempo/timing system here
	for (int i = 0; i < numCommands; i++) {

		//flag for trackstep function is only found at the start of each line (every 8th tracks[] )
		if (i == 0 || i % 8 == 0 && tracks[i] == 61438) {
			//trackstep functions
			switch (tracks[i+1]) {
			case 0: // EFFE0000 Stop player
				stopPlayer();
				break;
			case 1: //EFFE0001 Play a selection
				//uint16 selectionPosition = tracks[i+2]; 
				//uint16 selectionLoops = tracks[i+3];    
				playSelection(); //TODO: will accept selectionPosition & selectionLoops as parameters
				break;
			case 2: //EFFE002 Set the tempo
				//uint16 tempoDivisor = tracks[i+2];    
				//uint16 tempoBPM = tracks[i+3];        
				setTempo(); //TODO: will accept tempoDivisor & tempoBPM as parameters
				break;
			case 3: //EFFE0003 Function??? Not used in MI. Flagged as multimode/7 channel setting in other players
				break;
			case 4: //EFFE0004 Volume slide
				//uint16 volumeDivisor = tracks[i+2];    
				//uint16 volumeTarget = tracks[i+3];     
				volumeSlide(); //TODO: will accept volumeDivisor & volumeTarget as parameters
				break;
			default: //Non-existant command
				//TODO: error
				break;
			}		
			i += 7; //skips to end of line, loop then terminates and moves to i += 8, the start of next line
		} else {
			//it is a pattern, readPattern for tracks[]
			
			//each track[] is 16 bits; first 8 bits is pattern number, second 8 bits is transpose number
			//could just skip the masking for patternNumber and shift it 8 bits to the right
			uint8 patternNumber = (tracks[i] & 0xFF00) >> 8;
			uint8 patternTranspose = (tracks[i] & 0x00FF);
			//readPattern(patternNumber);
		}
	}
}
void Tfmx::readPattern(uint8 patternNumber) {
	//TODO: setup lookup routine which reads pattern number and then finds corresponding address
	//maybe a stream containing the pattern should be created in same way it was done in readTrackstep()

	uint32 startPosition;
	uint32 endPosition;
	int32 numCommands;		//number of longword pattern commands or notes
	startPosition = _patternPointers[patternNumber];
	endPosition = _patternPointers[patternNumber + 1];
	numCommands = (endPosition - startPosition) / 4;

	Common::MemoryReadStream dataStream(_data, _dataSize);
	Common::SeekableSubReadStream patternSubStream(&dataStream, startPosition, endPosition);

	uint32 *pattern;
	pattern = new uint32[numCommands];
	for (int i = 0; i < numCommands; i++) {
		pattern[i] = patternSubStream.readUint32BE();
	}
	//pattern[] should now have each command or note ready for processing

	//TODO: read pattern[] and determine if it is a command or note.
	// Commands will be processed as nessecary. 
	// Notes will be read as note structures and then passes to a seperate function for processing.

}
void Tfmx::readNote(Note _aNote) {
}
void Tfmx::readMacro(int _macroNumber) {
}
void Tfmx::stopPlayer() {
}
void Tfmx::playSelection() {
}
void Tfmx::setTempo() {
}
void Tfmx::volumeSlide() {
}
void Tfmx::interrupt() {
}


} // End of namespace Audio

