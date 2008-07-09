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
	if (_trackData)
		delete[] _trackData;
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
	
	//test
	//playSong(0);
}
bool Tfmx::load(Common::SeekableReadStream &stream) {
	return true;
}
bool Tfmx::play() {
	return true;
}
void Tfmx::playSong(uint8 songNumber) {
/* TODO:: Take the role of old readTrackstep function
Looks up songNumber, then initialize 8-track table.
Then starts PAULA.
Then each interrupt, updateTracks() is called and track data is reloaded.
*/
	if(songNumber >= 32) {
		//TODO:: Error. Not a valid index. Maybe index songs from 1-32 rather than 0-31.
	}

//Read trackstep start/end boundaries of song songNumber
//Read number of trackstep blocks and lines
	uint32 startPosition;
	uint32 endPosition;
	int32 numBlocks;
	int32 numLines;
	startPosition = (_songs[songNumber].startPosition * 16) + _trackTableOffset;
	endPosition = (_songs[songNumber].endPosition * 16) + _trackTableOffset + 16;
	numBlocks = (endPosition - startPosition) / 2;
	numLines = numBlocks / 8;

//Make dataStream MemoryReadStream from _data
//Make trackSubStream SubReadStream from dataStream bounded by start/end positions
	Common::MemoryReadStream dataStream(_data, _dataSize);
	Common::SeekableSubReadStream trackSubStream(&dataStream, startPosition, endPosition);

//Load trackSubStream into _trackData[] array
	_trackData = new uint16[numBlocks];
	_trackCount = 0;
	_trackLength = numBlocks;
	_tempo = _songs[songNumber].tempoValue; //TODO:: Need to setup for the case when tempoValue < 15
	for (int i = 0; i < numBlocks; i++) {
		_trackData[i] = trackSubStream.readUint16BE();
	}

//Initialize 8 tracks and get ready for update
	for (int i = 0; i < 8; i++) { 
			_tracks[i].data = 0;
			_tracks[i].updateFlag = true; 
			_tracks[i].activeFlag = true;
			_tracks[i].pattern.data = 0;
			_tracks[i].pattern.number = 0;
			_tracks[i].pattern.transpose = 0;
			_tracks[i].pattern.wait = 0;
			_tracks[i].pattern.newFlag = true;
			_tracks[i].pattern.jumpFlag = false;
			_tracks[i].pattern.returnFlag = false;
			_tracks[i].pattern.count = 0;
			_tracks[i].pattern.offset = 0;
			
	}

	//StartPaula()
	// and trigger updateTrackstep() cycle, called at each interrupt

/*//Test loop. Update and dump to console i times.
	for(int i = 0; i < 4; i++) {
	updateTrackstep();
	dumpTracks();
	}
*/
}
void Tfmx::updateTrackstep() {

	for (int i = 0; i < 8; i++) {
		if (_tracks[i].updateFlag && _trackCount <= _trackLength) {
			_tracks[i].updateFlag = false;
			_tracks[i].activeFlag = true;
			_tracks[i].data = _trackData[_trackCount];
		}
		_trackCount++;
	}

	if (_tracks[0].data == 61438) { //you have a line of trackstep commands
		switch (_tracks[1].data) {
		case 0: // EFFE0000 Stops player.
			stopPlayer();
			break;
		case 1: //EFFE0001: NOT USED IN MI.
			//Plays a selection.
			//uint16 selectionPosition = tracks[2]; 
			//uint16 selectionLoops = tracks[3];    
			break;
		case 2: //EFFE002 Set the tempo
			//MI uses this command but it seems broken - it sets both parameters to 0 only. 
			//Might be safe to ignore for now and just use tempo set in header.
			//uint16 tempoDivisor = tracks[2];    
			//uint16 tempoBPM = tracks[3];        
			setTempo(); //TODO: will accept tempoDivisor & tempoBPM as parameters
			break;
		case 3: //EFFE0003: NOT USED IN MI. 
			//Purpose not clear. Flagged as multimode/7 channel setting in other players.
			break;
		case 4: //EFFE0004 Volume slide
			//uint16 volumeDivisor = tracks[2];    
			//uint16 volumeTarget = tracks[3];     
			volumeSlide(); //TODO: will accept volumeDivisor & volumeTarget as parameters
			break;
		default: //Non-existant command
			//TODO: error
			break;
		} // end switch _tracks[1].data

		for (int i = 0; i < 8; i++) {
			_tracks[i].updateFlag = true;
		} // end for updateFlag = true loop
	} // end if track[].data = command

	else { //each tracks[].data is 16 bits; first 8 bits is pattern number, second 8 bits is transpose number
		for (int i = 0; i < 8; i++) {
			if (_tracks[i].data == 65280) {    //kill track command
				_tracks[i].activeFlag = false; 
				_tracks[i].updateFlag = true; 
			}
				
			if (_tracks[i].activeFlag) {
				if (_tracks[i].pattern.newFlag) {
					_tracks[i].pattern.number = _tracks[i].data >> 8;
					_tracks[i].pattern.transpose = _tracks[i].data & 0x00FF;

					if (_tracks[i].pattern.jumpFlag) {
						_tracks[i].pattern.number = _tracks[i].pattern.saveNumber1;
						_tracks[i].pattern.transpose = 0;
					}

					if (_tracks[i].pattern.returnFlag) {
						_tracks[i].pattern.number = _tracks[i].pattern.saveNumber2;
						_tracks[i].pattern.transpose = 0;
					}
				}
				if (_tracks[i].pattern.wait == 0) {
				updatePattern(i);			
				}
				else {
					_tracks[i].pattern.wait--;
				}
			}
		}//end pattern loop
	}//end else
}
void Tfmx::updatePattern(uint8 trackNumber) {
//IF it is a NEW pattern being loaded, then initialize pattern.data[] table
	if (_tracks[trackNumber].pattern.newFlag) { 
		_tracks[trackNumber].pattern.newFlag = false;
		uint32 startPosition;
		uint32 endPosition;
		int32 numCommands;		//number of longword pattern commands or notes
		startPosition = _patternPointers[_tracks[trackNumber].pattern.number] + _tracks[trackNumber].pattern.offset;
		endPosition = _patternPointers[_tracks[trackNumber].pattern.number + 1];
		numCommands = (endPosition - startPosition) / 4;
		_tracks[trackNumber].pattern.offset = 0;
		_tracks[trackNumber].pattern.count = 0;

		Common::MemoryReadStream dataStream(_data, _dataSize);
		Common::SeekableSubReadStream patternSubStream(&dataStream, startPosition, endPosition);

		_tracks[trackNumber].pattern.data = new uint32[numCommands];
		for (int i = 0; i < numCommands; i++) {
			_tracks[trackNumber].pattern.data[i] = patternSubStream.readUint32BE();
		}
	}
	//MASKING for uint32 pattern data
	uint8 byte1 = *(_tracks[trackNumber].pattern.data) & 0xFF000000 >> 24;
	uint8 byte2 = *(_tracks[trackNumber].pattern.data) & 0x00FF0000 >> 16;
	uint8 byte3 = *(_tracks[trackNumber].pattern.data) & 0x0000FF00 >> 8;
	uint8 byte4 = *(_tracks[trackNumber].pattern.data) & 0x000000FF;
	uint16 bytes34 = *(_tracks[trackNumber].pattern.data) & 0x0000FFFF;

	if (byte1 >= 0xF0) {
		switch (byte1) {
		case 0xF0: //end pattern + advance track
			//end pattern? also needs to know the track this is playing on
			_tracks[trackNumber].updateFlag = true; 
			_tracks[trackNumber].pattern.newFlag = true;
			break;
		case 0xF1: //repeat block/loop
			break;
		case 0xF2: //pattern jump
			//offset of byte34
			_tracks[trackNumber].pattern.saveNumber1 = byte2;
			_tracks[trackNumber].pattern.offset = bytes34;
			_tracks[trackNumber].pattern.jumpFlag = true;
			_tracks[trackNumber].pattern.newFlag = true;
			
			break;
		case 0xF3:
			//waits byte2 + 1 jiffies
			//_tracks[trackNumber].pattern.wait = byte2 + 1;
			break;
		case 0xF4: //kills track until new pointer is loaded
			//need to know track this pattern is on, then needs to stop reading proceeding cmds
			_tracks[trackNumber].activeFlag = false;
			break;
		case 0xF5: //Key up
			break;
		case 0xF6: //Vibrato
			break;
		case 0xF7: //Envelope
			break;
		case 0xF8: //same as 0xF2 except it saves current pattern address for return
			_tracks[trackNumber].pattern.saveNumber1 = byte2;
			_tracks[trackNumber].pattern.saveNumber2 = _tracks[trackNumber].pattern.number;
			_tracks[trackNumber].pattern.offset = (_tracks[trackNumber].pattern.count) * 32;
			_tracks[trackNumber].pattern.jumpFlag = true;
			_tracks[trackNumber].pattern.newFlag = true;
			break;
		case 0xF9: //return to saved address ; ie reload oldPattern
			_tracks[trackNumber].pattern.returnFlag = true;
			_tracks[trackNumber].pattern.newFlag = true;		
			break;
		case 0xFA: //master volume slide
			break;
		case 0xFB: 
			break;
		case 0xFC: //Portamento
			break;
		case 0xFD: //Channel lock
			//locks the channel specified in byte2 for bytes34 ticks
			break;
		case 0xFE: //disable track, same as F4 apparently
			_tracks[trackNumber].activeFlag = false;
			break;
		case 0xFF: //Do nothing - advance pattern pointer
			break;
		default:
			//invalid cmd
			break;
		} // end switch
	} // end if data = command
	
	else { 
		_tracks[trackNumber].pattern.note.noteNumber = byte1;
		_tracks[trackNumber].pattern.note.macroNumber = byte2;
		_tracks[trackNumber].pattern.note.channelNumber = (byte3 & 0xF0) >> 4;
		_tracks[trackNumber].pattern.note.volume = (byte3 & 0x0F);
		_tracks[trackNumber].pattern.note.wait = byte4; //should set pattern wait to this
		//updateNote();
	}

	_tracks[trackNumber].pattern.data++;
	_tracks[trackNumber].pattern.count++;
}

void Tfmx::updateNote(uint8 trackNumber) {
}
void Tfmx::doMacros(uint8 trackNumber) {
}
void Tfmx::stopPlayer() {
	for(int i = 0; i < 8; i++) {
	_tracks[i].activeFlag = false;
	}
	stopPaula();
}
void Tfmx::setTempo() {
}
void Tfmx::volumeSlide() {
}
void Tfmx::interrupt() {
}
void Tfmx::dumpTracks() {
	for (int i = 0; i < 8; i++) {
		warning("TRACK # %d", i);
		warning("Data::: %X", _tracks[i].data);

		if(_tracks[i].pattern.data)  {
		warning("PATTERN # %X", _tracks[i].pattern.number);
		warning("Data::: %X", *(_tracks[i].pattern.data - 1) );
		}
	}
}

} // End of namespace Audio

