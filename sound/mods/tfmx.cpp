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

const uint16 Tfmx::periods[] =
	{0x06AE,0x064E,0x05F4,0x059E,0x054D,0x0501,
	 0x04B9,0x0475,0x0435,0x03F9,0x03C0,0x038C,0x0358,0x032A,0x02FC,0x02D0,0x02A8,0x0282,
	 0x025E,0x023B,0x021B,0x01FD,0x01E0,0x01C6,0x001AC,0x0194,0x017D,0x0168,0x0154,0x0140,
	 0x012F,0x011E,0x010E,0x00FE,0x00F0,0x00E3,0x00D6,0x00CA,0x00BF,0x00B4,0x00AA,0x00A0,
	 0x0097,0x008F,0x0087,0x007F,0x0078,0x0071,0x00D6,0x00CA,0x00BF,0x00B4,0x00AA,0x00A0,
	 0x0097,0x008F,0x0087,0x007F,0x0078,0x0071,0x00D6,0x00CA,0x00BF,0x00B4};

Tfmx::Tfmx(bool stereo, int rate, int interruptFreq)
	: Paula(stereo, rate, interruptFreq) {
		_data = 0;
		_trackData = 0;
		_sampleData = 0;
} 
Tfmx::~Tfmx() {
		delete[] _data;
		delete[] _trackData;
		delete[] _sampleData;
}
void Tfmx::playSong(uint8 songNumber){
	if (loadSong(songNumber)) {
		startPaula();
		warning("PAULA STARTED.");
		warning("Playing Song # %d.",songNumber);
	}
}
void Tfmx::stop() {
	warning("PAULA STOPPED.");
	stopPaula();
}
void Tfmx::loadSamples() {
	//FIXME:: temporary loader - just creates seekablereadstream from c:\mk.smpl
	Common::SeekableReadStream *stream = NULL;
	Common::File myfile;
	myfile.addDefaultDirectory("C:");
	myfile.open("mk.smpl");
	stream = myfile.readStream(myfile.size());
	myfile.close();
	//FIXME:: end temporary loader. normally the seekablereadstream will be function parameter

	_sampleSize = stream->size();
	_sampleData = new int8[_sampleSize];
	stream->seek(0);
	stream->read(_sampleData, _sampleSize);
}

void Tfmx::load() {
	//FIXME:: temporary loader - just creates seekablereadstream from c:\mk.mdat
	Common::SeekableReadStream *stream = NULL;
	Common::File myfile;
	myfile.addDefaultDirectory("C:");
	myfile.open("mk.mdat");
	stream = myfile.readStream(myfile.size());
	myfile.close();
	//FIXME:: end temporary loader. normally the seekablereadstream will be function parameter

	_dataSize = stream->size();
	_data = new uint8[_dataSize];
	stream->seek(0);
	stream->read(_data, _dataSize);

	//TODO:: should implement a check here to read header and ensure TFMXness
	Common::MemoryReadStream dataStream(_data, _dataSize);
	dataStream.seek(256);
	//TODO:: Will later modify to dynamically scale arrays for smaller TFMX files
	//In the case of Monkey Island, it uses 22/32 songs, and 128/128 for patterns/macros
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
}

bool Tfmx::loadSong(uint8 songNumber) {
	if(songNumber >= 32) {
		error("Invalid Song Number.");
		return false;
	}

	uint32 startPosition;
	uint32 endPosition;
	int32 numBlocks;
	startPosition = (_songs[songNumber].startPosition * 16) + _trackTableOffset;
	endPosition = (_songs[songNumber].endPosition * 16) + _trackTableOffset + 16;
	numBlocks = (endPosition - startPosition) / 2;

	Common::MemoryReadStream dataStream(_data, _dataSize);
	Common::SeekableSubReadStream trackSubStream(&dataStream, startPosition, endPosition);

	_trackLength = numBlocks;
	_trackCount = 0;
	_trackData = new uint16[_trackLength];
	_tempo = _songs[songNumber].tempoValue; 
	for (int i = 0; i < numBlocks; i++) {
		_trackData[i] = trackSubStream.readUint16BE();
	}

	_trackAdvance = true;
	_trackEnd = false;
	_macroTest = false;
	_patternTest = false;

	//INITIALIZE TRACKS
	for (int track = 0; track < 8; track++) { 
		//initialize Track objects
		_tracks[track].data = 0;
		_tracks[track].trackOn = true;
		_tracks[track].patternOn = false;
		_tracks[track].macroOn = false;
		_tracks[track].patternNumber = 0xFF; //obviously impossible pattern number

		//initialize Pattern objects
		_tracks[track].activePattern.data = 0;
		_tracks[track].activePattern.patternCount = 0;
		_tracks[track].activePattern.patternLength = 0;
		_tracks[track].activePattern.patternWait = 0;
		_tracks[track].activePattern.patternTranspose = 0;
		_tracks[track].activePattern.newPattern = false;

		//initialize Macro objects
		_tracks[track].activeMacro.data = 0;
		_tracks[track].activeMacro.macroCount = 0;
		_tracks[track].activeMacro.macroLength = 0;
		_tracks[track].activeMacro.macroWait = 0;
		_tracks[track].activeMacro.noteNumber = 0;
		_tracks[track].activeMacro.notePeriod = 0;
		_tracks[track].activeMacro.noteVelocity = 0;
		_tracks[track].activeMacro.noteChannel = 0;
		_tracks[track].activeMacro.noteType = 0;
		_tracks[track].activeMacro.noteWait = 0;
		_tracks[track].activeMacro.fineTune = 0;
		_tracks[track].activeMacro.keyWaitOn = false;
		_tracks[track].activeMacro.keyCount = 0;
		_tracks[track].activeMacro.keyWait = 0;
	}

	//INITIALIZE CHANNELS
	for (int channel = 0; channel < 4; channel++) {
		_channels[channel].period = 0;
		_channels[channel].volume = 0;
		_channels[channel].sampleOffset = 0;
		_channels[channel].sampleLength = 0;
	    _channels[channel].sampleOn = false;
		_channels[channel].updateOn = false;
		_channels[channel].keyUp = false;
		_channels[channel].envelopeOn = false;
		_channels[channel].envelopeTarget = 0;
		_channels[channel].envelopeRate = 0;
		_channels[channel].envelopeCount = 0;
		_channels[channel].envelopeSpeed = 0;
		_channels[channel].vibratoOn = false;
		_channels[channel].vibratoDirection = false;
		_channels[channel].vibratoRate = 0;
		_channels[channel].vibratoSpeed = 0;
		_channels[channel].vibratoCount = 0;
	}

	//SET INTERRUPT FREQUENCY
	if (_tempo >= 0x10) {
	setInterruptFreq( (int)( getRate() / (_tempo * 0.4)));
	}
	else {
	setInterruptFreq( (int)( getRate() / (1 / _tempo * 24)));
	}

	setInterruptFreq( (int)( getRate() / 80) );
	return true;
}
void Tfmx::updateTrackstep() {
	//If _trackEnd, stop Paula and terminate update.
	if (_trackEnd) {
		stop();
		return;
	}

	//Load in line of trackstep commands.
	for (int track = 0; track < 8; track++) {
			_tracks[track].trackOn = true;
			_tracks[track].data = _trackData[_trackCount];
			_trackCount++;
	}

	//check if you reached the last trackstep block
	if (_trackCount >= _trackLength) {
		_trackEnd = true;
	}

	//proccess trackstep commands
	if (_tracks[0].data == 61438) { //you have a line of trackstep commands
		switch (_tracks[1].data) {
		case 0: // EFFE0000 Stops player.
			stop();
			break;
		case 1: //EFFE0001: NOT USED IN MI.
			//Plays a selection.
			//uint16 selectionPosition = tracks[2]; 
			//uint16 selectionLoops = tracks[3];    
			break;
		case 2: //EFFE002: SAFE TO IGNORE FOR NOW
			//MI uses this command but it seems broken - it sets both parameters to 0 only. 
			//Might be safe to ignore for now and just use tempo set in header.
			//uint16 tempoDivisor = tracks[2];    
			//uint16 tempoBPM = tracks[3];        
			//setTempo(); //TODO: will accept tempoDivisor & tempoBPM as parameters
			break;
		case 3: //EFFE0003: NOT USED IN MI. 
			//Purpose not clear. Flagged as multimode/7 channel setting in other players.
			break;
		case 4: //EFFE0004 Volume slide
			//uint16 volumeDivisor = tracks[2];    
			//uint16 volumeTarget = tracks[3];     
			//volumeSlide(); //TODO: will accept volumeDivisor & volumeTarget as parameters
			break;
		default: //Non-existant command
			//TODO: error
			break;
		} // end switch 
	} // end if track[].data = command

	else { //each tracks[].data is 16 bits; first 8 bits is pattern number, second 8 bits is transpose number
		for (int track = 0; track < 8; track++) {
			
			if (_tracks[track].data == 0xFF00) {    //kill track command
				_tracks[track].trackOn = false; 
			}
				
			if (_tracks[track].trackOn) {	
				_tracks[track].patternNumber = _tracks[track].data >> 8;
				_tracks[track].activePattern.patternTranspose = _tracks[track].data & 0x00FF;
				_tracks[track].patternOn = true;
				loadPattern( track, (_tracks[track].data >> 8) );
			}
		}
	}//end else
}
void Tfmx::loadPattern(uint8 trackNumber, uint8 patternNumber) {
	uint32 startPosition;
	uint32 endPosition;
	int32 numCommands;		
	startPosition = _patternPointers[patternNumber]; //+ _tracks[trackNumber].pattern.offset;
	endPosition = _patternPointers[patternNumber + 1];
	numCommands = (endPosition - startPosition) / 4;
	//_tracks[trackNumber].activePattern.offset = 0;
	_tracks[trackNumber].activePattern.patternCount = 0;
	_tracks[trackNumber].activePattern.patternLength = numCommands;

	Common::MemoryReadStream dataStream(_data, _dataSize);
	Common::SeekableSubReadStream patternSubStream(&dataStream, startPosition, endPosition);

	_tracks[trackNumber].activePattern.data = new uint32[numCommands];
	for (int i = 0; i < numCommands; i++) {
		_tracks[trackNumber].activePattern.data[i] = patternSubStream.readUint32BE();
	}
}
void Tfmx::updatePattern(uint8 trackNumber) {
	if (_tracks[trackNumber].activePattern.patternWait != 0) {
		//printf("PATTERN WAIT:: %02x \n", _tracks[trackNumber].activePattern.patternWait);
		_tracks[trackNumber].activePattern.patternWait--;
		return;
	}

	uint8 byte1 = ( *(_tracks[trackNumber].activePattern.data) & 0xFF000000 ) >> 24;
	uint8 byte2 = ( *(_tracks[trackNumber].activePattern.data) & 0x00FF0000 ) >> 16;
	uint8 byte3 = ( *(_tracks[trackNumber].activePattern.data) & 0x0000FF00 ) >> 8;
	uint8 byte4 = ( *(_tracks[trackNumber].activePattern.data) & 0x000000FF );
	uint16 bytes34 = ( *(_tracks[trackNumber].activePattern.data) & 0x0000FFFF );

//	printf("PATTERN COMMAND:: %02x \n", byte1);

	if (byte1 >= 0xF0) {
		switch (byte1) {
		case 0xF0: //end pattern + advance track
			//end pattern? also needs to know the track this is playing on
			if (!_tracks[trackNumber].activeMacro.keyWaitOn) {
				_tracks[trackNumber].patternOn = false; 
			}
			//_trackAdvance = true;
			break;
		case 0xF1: //repeat block/loop
			break;
		case 0xF2: //pattern jump
			//offset of byte34
			/*
			_tracks[trackNumber].pattern.saveNumber1 = byte2;
			_tracks[trackNumber].pattern.offset = bytes34;
			_tracks[trackNumber].pattern.jumpFlag = true;
			_tracks[trackNumber].pattern.newFlag = true;
			*/
			break;
		case 0xF3:
			//waits byte2 + 1 jiffies
			_tracks[trackNumber].activePattern.patternWait = byte2;
			break;
		case 0xF4: //kills track until new pointer is loaded
			//need to know track this pattern is on, then needs to stop reading proceeding cmds
			if (!_tracks[trackNumber].activeMacro.keyWaitOn) {
				_tracks[trackNumber].patternOn = false; 
				_tracks[trackNumber].trackOn = false;
			}
			break;
		case 0xF5: //Key up
			_channels[(byte3 & 0x0F)].keyUp = true;
			break;
		case 0xF6: //Vibrato
			break;
		case 0xF7: //Envelope
			break;
		case 0xF8: //same as 0xF2 except it saves current pattern address for return
			/*
			_tracks[trackNumber].pattern.saveNumber1 = byte2;
			_tracks[trackNumber].pattern.saveNumber2 = _tracks[trackNumber].pattern.number;
			_tracks[trackNumber].pattern.offset = (_tracks[trackNumber].pattern.count) * 32;
			_tracks[trackNumber].pattern.jumpFlag = true;
			_tracks[trackNumber].pattern.newFlag = true;
			*/
			break;
		case 0xF9: //return to saved address ; ie reload oldPattern
			/*
			_tracks[trackNumber].pattern.returnFlag = true;
			_tracks[trackNumber].pattern.newFlag = true;		
			*/
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
			_tracks[trackNumber].patternOn = false; 
			_tracks[trackNumber].trackOn = false;
			break;
		case 0xFF: //Do nothing - advance pattern pointer
			break;
		default:
			//invalid cmd
			break;
		} // end switch
	} 
	
	else { 
		if (!_tracks[trackNumber].macroOn) {
			_tracks[trackNumber].macroNumber = byte2;
			loadMacro( trackNumber, (_tracks[trackNumber].macroNumber) );
			if (byte1 < 0x80) {
				_tracks[trackNumber].activeMacro.noteType = 1; 		//byte4 = finetune
				_tracks[trackNumber].activeMacro.noteFineTune = byte4;
			}
			else if (byte1 < 0xC0) {
				_tracks[trackNumber].activeMacro.noteType = 2;      //byte4 = wait
				_tracks[trackNumber].activeMacro.noteWait = byte4;
			}
			else {
				_tracks[trackNumber].activeMacro.noteType = 3;      //byte4 = portamento rate
			}
			_tracks[trackNumber].macroOn = true; //set to false again when macro terminates
			_tracks[trackNumber].activeMacro.noteNumber = byte1;
			_tracks[trackNumber].activeMacro.noteVelocity = (byte3 & 0xF0) >> 4;
			_tracks[trackNumber].activeMacro.noteChannel = (byte3 & 0x0F);
			_tracks[trackNumber].activeMacro.notePeriod = periods[(_tracks[trackNumber].activeMacro.noteNumber + _tracks[trackNumber].activePattern.patternTranspose) & 0x3F];
		}

		else {
			while (_tracks[trackNumber].activeMacro.macroWait == 0 && (_tracks[trackNumber].macroOn == true) ) {
				doMacro(trackNumber);
			}
		}
		if (_tracks[trackNumber].activeMacro.macroWait != 0) {
		printf("MACRO WAIT:: %02x \n", _tracks[trackNumber].activeMacro.macroWait);
		_tracks[trackNumber].activeMacro.macroWait--;
		}
	}//END ELSE

	//ADVANCE PATTERN COUNT, INCREASE COUNT
	//IF MACRO IS ON, WAIT TO ADVANCE
	if ( (!_tracks[trackNumber].macroOn) || (_tracks[trackNumber].activeMacro.keyWaitOn) ) {
		_tracks[trackNumber].activePattern.data++;
		_tracks[trackNumber].activePattern.patternCount++;
		if (_tracks[trackNumber].activeMacro.noteType == 2 && (!_tracks[trackNumber].activeMacro.keyWaitOn) ) {
			_tracks[trackNumber].activePattern.patternWait += _tracks[trackNumber].activeMacro.noteWait;
		}
	}
	//IF THE END IS REACHED, TURN PATTERN OFF
	if (_tracks[trackNumber].activePattern.patternCount == _tracks[trackNumber].activePattern.patternLength) {
		if (!_tracks[trackNumber].activeMacro.keyWaitOn) {
		_tracks[trackNumber].patternOn = false;
		}
	}
}
void Tfmx::loadMacro(uint8 trackNumber, uint8 macroNumber){
	uint32 startPosition;
	uint32 endPosition;
	int32 numCommands;		
	startPosition = _macroPointers[macroNumber]; //TODO:: Need to add offset for jump commands
	endPosition = _macroPointers[macroNumber + 1];
	numCommands = (endPosition - startPosition) / 4; 

	_tracks[trackNumber].activeMacro.macroCount = 0;
	_tracks[trackNumber].activeMacro.macroLength = numCommands;
	_tracks[trackNumber].activeMacro.noteType = 0;
	_tracks[trackNumber].activeMacro.noteFineTune = 0;
	
	Common::MemoryReadStream dataStream(_data, _dataSize);
	Common::SeekableSubReadStream macroSubStream(&dataStream, startPosition, endPosition);

	_tracks[trackNumber].activeMacro.data = new uint32[numCommands];
	for (int i = 0; i < numCommands; i++) {
		_tracks[trackNumber].activeMacro.data[i] = macroSubStream.readUint32BE();
	}

	printf("MACRO NUMBER:: %02x \n", macroNumber);
	printf("MACRO LENGTH:: %02x \n", numCommands);
}
void Tfmx::doMacro(uint8 trackNumber) {
	uint8 byte1 = ( *(_tracks[trackNumber].activeMacro.data) ) >> 24;
	uint8 byte2 = ( *(_tracks[trackNumber].activeMacro.data) & 0x00FF0000 ) >> 16;
	uint8 byte3 = ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FF00 ) >> 8;
	int8 sbyte3 = ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FF00 ) >> 8;
	uint8 byte4 = ( *(_tracks[trackNumber].activeMacro.data) & 0x000000FF );
	uint8 currentChannel = _tracks[trackNumber].activeMacro.noteChannel;
	uint16 tunedPeriod = 0;
	
	printf("MACRO COUNT:: %02x ::::", _tracks[trackNumber].activeMacro.macroCount);
	printf("MACRO COMMAND:: %02x \n", byte1);

	switch (byte1) {
	case 0x00: //DMAoff reset + CLEARS EFFECTS
		_channels[currentChannel].sampleOn = false;
		//_channels[currentChannel].updateOn = true;
		_channels[currentChannel].sampleOffset = 0;
		_channels[currentChannel].sampleLength = 0;
		_channels[currentChannel].envelopeOn = false;
		_channels[currentChannel].vibratoOn = false;
		//_tracks[trackNumber].activeMacro.macroWait = 1;
		break;
	case 0x01:
		_channels[currentChannel].sampleOn = true;
		_channels[currentChannel].updateOn = true;
		//_tracks[trackNumber].activeMacro.macroWait = 1;
		break;
	case 0x02: //set sample offset
		_channels[currentChannel].sampleOffset = *(_tracks[trackNumber].activeMacro.data) & 0x00FFFFFF;
		_channels[currentChannel].updateOn = true;
		break;
	case 0x03: //set sample length
		//_channels[currentChannel].sampleLength = ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF ) * 2;
		_channels[currentChannel].sampleLength = ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF );
		_channels[currentChannel].updateOn = true;
		break;
	case 0x04: //wait
		if ( (*(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF) == 0 ) {
			_tracks[trackNumber].activeMacro.macroWait = 1;
		}
		else {
			_tracks[trackNumber].activeMacro.macroWait = ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF );
		}
		break;
	case 0x07:
		_channels[currentChannel].sampleOn = false; 
		_tracks[trackNumber].macroOn = false;
		break;
	case 0x13: //DMA OFF BUT DOESNT CLEAR EFFECTS
		_channels[currentChannel].sampleOn = false;
		break;
	case 0x0D: //add volume to channel;.
		if (byte3 != 0xFE) {
			_channels[currentChannel].volume = (_tracks[trackNumber].activeMacro.noteVelocity * 3) + byte4;
			
			if (_channels[currentChannel].volume > 0x40) {
				_channels[currentChannel].volume = 0x40;
			}
		}
		break;
	case 0x0E: //set volume
		if (byte3 != 0xFE) {
			_channels[currentChannel].volume = byte2;
		}
		break;
	case 0x19: //set one shot ? cut off?
		_tracks[trackNumber].activeMacro.notePeriod = 0;
		_channels[currentChannel].sampleOffset = 0;
		_channels[currentChannel].sampleLength = 0;
		_channels[currentChannel].updateOn = true;
		break;
	case 0x18: //sampleloop
		//_channels[currentChannel].sampleOffset += ( *(_tracks[trackNumber].activeMacro.data) & 0x00FFFFFF );
		//_channels[currentChannel].sampleLength -= ( *(_tracks[trackNumber].activeMacro.data) & 0x00FFFFFF );
		_channels[currentChannel].sampleOffset += ( ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF ) & 0xFFFE );
		_channels[currentChannel].sampleLength -= ( ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF ) >> 1);
		//_channels[currentChannel].updateOn = true;
		break;
	case 0x08: //add note
		tunedPeriod = (periods[(byte2 + _tracks[trackNumber].activeMacro.noteNumber) & (0x3F)]);
		_tracks[trackNumber].activeMacro.fineTune = sbyte3 / 0x100;
		_tracks[trackNumber].activeMacro.fineTune += (_tracks[trackNumber].activeMacro.noteFineTune / 0x100);
		_tracks[trackNumber].activeMacro.fineTune += 1;
		_tracks[trackNumber].activeMacro.notePeriod = (int)(tunedPeriod * _tracks[trackNumber].activeMacro.fineTune);
		_tracks[trackNumber].activeMacro.macroWait = 1;
		break;
	case 0x09: //set note
		tunedPeriod = (periods[(byte2) & (0x3F)]);
		_tracks[trackNumber].activeMacro.fineTune = sbyte3 / 0x100;
		_tracks[trackNumber].activeMacro.fineTune += (_tracks[trackNumber].activeMacro.noteFineTune / 0x100);
		_tracks[trackNumber].activeMacro.fineTune += 1;
		_tracks[trackNumber].activeMacro.notePeriod = (int)(tunedPeriod * _tracks[trackNumber].activeMacro.fineTune);
		_tracks[trackNumber].activeMacro.macroWait = 1;
		break;
	case 0x17: //set period
		_tracks[trackNumber].activeMacro.notePeriod = ( *(_tracks[trackNumber].activeMacro.data) & 0x0000FFFF );
		_tracks[trackNumber].activeMacro.macroWait = 1;
		break;
	case 0x0F://envelope
		_channels[currentChannel].envelopeOn = true;
		_channels[currentChannel].envelopeTarget = byte4;
		_channels[currentChannel].envelopeRate = byte2;
		_channels[currentChannel].envelopeSpeed = byte3; 
		_channels[currentChannel].envelopeCount = 0;
		break;
	case 0x0C: //vibrato
		_channels[currentChannel].vibratoOn = true;
		_channels[currentChannel].vibratoRate = byte4;
		_channels[currentChannel].vibratoSpeed = byte2 * 2;
		_channels[currentChannel].vibratoCount = 0;
		_channels[currentChannel].vibratoDirection = false; //vibrato down first
		break;
	case 0x14://wait for key-up
		_tracks[trackNumber].activeMacro.keyWaitOn = true;
		_tracks[trackNumber].activeMacro.keyWait = byte4;
		if (_tracks[trackNumber].activeMacro.keyWait == 0) {
			_tracks[trackNumber].activeMacro.keyWait = -1; //infinite loop
		}
		
		if (_tracks[trackNumber].activeMacro.keyCount == 0) {
			printf("KEY UP WAIT TIME:: %02x \n", _tracks[trackNumber].activeMacro.keyWait);
			_channels[currentChannel].updateOn = true;
			_channels[currentChannel].keyUp = false;
		}

		if ( (!_channels[currentChannel].keyUp) && (_tracks[trackNumber].activeMacro.keyCount != _tracks[trackNumber].activeMacro.keyWait) ){
			//waiting for key up to be set
			_tracks[trackNumber].activeMacro.data--;
			_tracks[trackNumber].activeMacro.macroCount--;
			_tracks[trackNumber].activeMacro.macroWait = 1;
			printf("KEY UP WAIT COUNT:: %02x \n", _tracks[trackNumber].activeMacro.keyCount);
			_tracks[trackNumber].activeMacro.keyCount++;
			if (_tracks[trackNumber].activeMacro.keyCount == 0x7F) {
				_tracks[trackNumber].activeMacro.keyCount = 1;
			}
		}
		else if ( (_channels[currentChannel].keyUp) || (_tracks[trackNumber].activeMacro.keyCount == _tracks[trackNumber].activeMacro.keyWait) ) {
			//key up has been set or key wait time has been reached
			_channels[currentChannel].keyUp = false;
			_tracks[trackNumber].activeMacro.keyWaitOn = false;
			_tracks[trackNumber].activeMacro.keyWait = 0;
			_tracks[trackNumber].activeMacro.keyCount = 0;
		}
		break;
	default:
		break;
	}//end switch

	if (_channels[currentChannel].volume == 0) {
		_channels[currentChannel].volume = _tracks[trackNumber].activeMacro.noteVelocity;
	}
	_channels[currentChannel].period = _tracks[trackNumber].activeMacro.notePeriod;
	
	_tracks[trackNumber].activeMacro.data++;
	_tracks[trackNumber].activeMacro.macroCount++;

	if (_tracks[trackNumber].activeMacro.macroCount == _tracks[trackNumber].activeMacro.macroLength) {
		_tracks[trackNumber].macroOn = false;
		//_channels[currentChannel].sampleOn = false;
	}
}
void Tfmx::testMacro(uint8 macroNumber) {
	_macroTest = true;
	loadMacro(0, macroNumber);
	_tracks[0].macroOn = true; //set to false again when macro terminates
	_tracks[0].activeMacro.noteNumber = 0x1E;  //middle C
	_tracks[0].activeMacro.noteVelocity = 0x0F;
	_tracks[0].activeMacro.noteChannel = 0;
	_tracks[0].activeMacro.noteWait = 0; 
	_tracks[0].activeMacro.notePeriod = periods[(_tracks[0].activeMacro.noteNumber & 0x3F)];
	startPaula();
}
void Tfmx::interrupt(void) {
	static int count = 0;
	count++;
	//printf("INTERRUPT COUNT:: %02x \n", count);
	
	if (!_macroTest && !_patternTest) { //Would be the normal case unless macro testing is on.

		if (_trackAdvance) {
			updateTrackstep();
		}

		//CYCLE THROUGH THE 8 TRACKS TO FIND WHICH PATTERNS ARE ON AND THEN UPDATE
		//WILL SKIP UPDATES FOR A WAIT
		for (int track = 0; track < 8; track++) {
			if (_tracks[track].patternOn && _tracks[track].trackOn) {
					updatePattern(track);
			}
		}

		for (int i = 1; i < 4; i++) {
			doEffects(i);
			setChannelPeriod(i,_channels[i].period);
			setChannelVolume(i,_channels[i].volume);
			if ( (_channels[i].sampleOn) && (_channels[i].updateOn) ) {
				printf("SAMPLE ON:: Channel # %02x \n", i);
				printf("SAMPLE OFFSET:: %02x \n", _channels[i].sampleOffset);
				printf("SAMPLE LENGTH:: %02x \n", _channels[i].sampleLength);
				printf("SAMPLE VOLUME:: %02x \n", _channels[i].volume);
				printf("SAMPLE PERIOD:: %02x \n", _channels[i].period);
				//setChannelData(i, _sampleData + _channels[i].sampleOffset, 0, _channels[i].sampleLength, 0);
				setChannelData(i, _sampleData + _channels[i].sampleOffset, _sampleData + _channels[i].sampleOffset, _channels[i].sampleLength, _channels[i].sampleLength);
				_channels[i].updateOn = false;
			}	
		}
		
		//CHECK IF PATTERNS ARE ON: IF SO, TRACK ADVANCE IS FALSE
		for (int track = 0; track < 8; track++) {
			if (_tracks[track].patternOn) {
				_trackAdvance = false;
				break;
			} 
			else {
				_trackAdvance = true;
			}
		}

	}//IF MACRO TEST = FALSE

	else if (_macroTest) { //MACRO TEST IS ON
		while (_tracks[0].activeMacro.macroWait == 0 && (_tracks[0].macroOn == true) ) {
				doMacro(0);
		}
		doEffects(0);
		setChannelPeriod(0,_channels[0].period);
		setChannelVolume(0,_channels[0].volume);
				if ( (_channels[0].sampleOn) && (_channels[0].updateOn) ) {
				printf("SAMPLE ON:: Channel #0 \n");
				printf("SAMPLE OFFSET:: %02x \n", _channels[0].sampleOffset);
				printf("SAMPLE LENGTH:: %02x \n", _channels[0].sampleLength);
				printf("SAMPLE VOLUME:: %02x \n", _channels[0].volume);
				printf("SAMPLE PERIOD:: %02x \n", _channels[0].period);
				printf("SAMPLE PERIOD NUMBER:: %02x \n", _tracks[0].activeMacro.noteNumber & 0x3F);
				//setChannelData(0, _sampleData + _channels[0].sampleOffset, 0, _channels[0].sampleLength, 0);
				setChannelData(0, _sampleData + _channels[0].sampleOffset, _sampleData + _channels[0].sampleOffset, _channels[0].sampleLength, _channels[0].sampleLength);
				_channels[0].updateOn = false;
		}		

		if (_tracks[0].activeMacro.macroWait != 0) {
		_tracks[0].activeMacro.macroWait--;
		}
	}

	else if (_patternTest) {
		//printf("INTERRUPT COUNT:: %02x \n", count);
		if (_tracks[0].patternOn && _tracks[0].trackOn) {
			updatePattern(0);
		}

		for (int i = 0; i < 4; i++) {
			//setChannelPeriod(i,periods[(_tracks[0].activeMacro.noteNumber & 0x3F)]);
			doEffects(i);
			setChannelPeriod(i,_channels[i].period);
			setChannelVolume(i,_channels[i].volume);
			
			if ( (_channels[i].sampleOn) && (_channels[i].updateOn) ) {
				printf("SAMPLE ON:: Channel # %02x \n", i);
				printf("SAMPLE OFFSET:: %02x \n", _channels[i].sampleOffset);
				printf("SAMPLE LENGTH:: %02x \n", _channels[i].sampleLength);
				printf("SAMPLE VOLUME:: %02x \n", _channels[i].volume);
				printf("SAMPLE PERIOD:: %02x \n", _channels[i].period);
				printf("SAMPLE PERIOD NUMBER:: %02x \n", _tracks[0].activeMacro.noteNumber & 0x3F);
				//setChannelData(i, _sampleData + _channels[i].sampleOffset, 0, _channels[i].sampleLength, 0);
				setChannelData(i, _sampleData + _channels[i].sampleOffset, _sampleData + _channels[i].sampleOffset, _channels[i].sampleLength, _channels[i].sampleLength);
				_channels[i].updateOn = false;
			}	
		}

		//if (!_tracks[0].patternOn) {
		//	stopPaula();
		//}
	}//end pattern test
}
void Tfmx::testPattern(uint8 patternNumber) {
	_patternTest = true;
	loadPattern(0, patternNumber); //load the pattern into track[0]
	_tracks[0].patternOn = true;
	startPaula();
}
void Tfmx::doEffects(uint8 channelNumber) {
	//Envelope Effect
	if (_channels[channelNumber].envelopeOn) {
		//Tick at the begining or the end?
		//_channels[channelNumber].envelopeCount++;
		if(_channels[channelNumber].envelopeCount == _channels[channelNumber].envelopeSpeed) {
			
			//Subtract Volume, target volume is lower
			if (_channels[channelNumber].volume > _channels[channelNumber].envelopeTarget) {
				_channels[channelNumber].volume -= _channels[channelNumber].envelopeRate;
				//Fix if you pass target volume
				if (_channels[channelNumber].volume < _channels[channelNumber].envelopeTarget) {
					_channels[channelNumber].volume = _channels[channelNumber].envelopeTarget;
				}
			}
			//Add Volume, target volume is higher
			if (_channels[channelNumber].volume < _channels[channelNumber].envelopeTarget) {
				_channels[channelNumber].volume += _channels[channelNumber].envelopeRate;
				//Fix if you pass target volume
				if (_channels[channelNumber].volume > _channels[channelNumber].envelopeTarget) {
					_channels[channelNumber].volume = _channels[channelNumber].envelopeTarget;
				}
			}
			//Reset count.
			_channels[channelNumber].envelopeCount = 0;
		}
		//Tick at the begining or the end?
		_channels[channelNumber].envelopeCount++;

		if (_channels[channelNumber].volume == _channels[channelNumber].envelopeTarget) {
			_channels[channelNumber].envelopeOn = false;
		}
	}
	//Vibrato Effect
	if (_channels[channelNumber].vibratoOn) {
		
		if (_channels[channelNumber].vibratoCount == _channels[channelNumber].vibratoSpeed) {
			//vibrato up
			if (_channels[channelNumber].vibratoDirection) { //true = up
			_channels[channelNumber].period += _channels[channelNumber].vibratoRate;
			_channels[channelNumber].vibratoDirection = false;
			}
			//vibrato down
			else if (!_channels[channelNumber].vibratoDirection) { //false = down
			_channels[channelNumber].period -= _channels[channelNumber].vibratoRate;
			_channels[channelNumber].vibratoDirection = true;
			}
			//reset count
			_channels[channelNumber].vibratoCount = 0;
		}
		_channels[channelNumber].vibratoCount++;
	}
	//Period Effects
}

void Tfmx::setTempo() {
	//Safe to ignore for now
}
void Tfmx::volumeSlide() {
	//Safe to ignore for now
}

} // End of namespace Audio

