/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "musicbase.h"

SkyMusicBase::SkyMusicBase(SkyDisk *pSkyDisk) {

	_musicData = NULL;
	_allowedCommands = 0;
	_skyDisk = pSkyDisk;
	_currentMusic = 0;
	_musicVolume = 127;
}

SkyMusicBase::~SkyMusicBase(void)
{
	if (_musicData) free(_musicData);
}

void SkyMusicBase::loadSection(uint8 pSection)
{
	if (_currentMusic) stopMusic();
	if (_musicData) free(_musicData);
	_currentSection = pSection;
	_musicData = _skyDisk->loadFile(_driverFileBase + FILES_PER_SECTION * pSection, NULL);
	_allowedCommands = 0;
	_musicTempo0 = 0x78; // init constants taken from idb file, area ~0x1060
	_musicTempo1 = 0xC0;
	_onNextPoll.doReInit = false;
	_onNextPoll.doStopMusic = false;
	_onNextPoll.musicToProcess = 0;
	_tempo = _aktTime = 0x10001;
	_numberOfChannels = _currentMusic = 0;
	setupPointers();
	startDriver();
}

void SkyMusicBase::musicCommand(uint16 command)
{
	if (_musicData == NULL) {
		debug(1,"Got music command but driver is not yet loaded.\n");
		return ;
	}
	if ((command >> 8) > _allowedCommands) {
		debug(1,"got musicCommand %d while expecting <= %d\n", command >> 8, _allowedCommands);
		return ;
	}
	switch(command >> 8) {
		case 0: 
			debug(1,"SkyMusic: got call to startAdlibDriver(). Not necessary in this implementation.\n");
			break;
		case 1: 
			debug(1,"SkyMusic: got call to stopDriver(). Not necessary in this implementation.\n");
			break;
		case 2:
			debug(1,"SkyMusic: got call to SetTempo(). Tempo is fixed in this implementation.\n");
			break;
		case 3: 
			debug(1,"SkyMusic: ignored direct call to driverPoll().\n");
			break;
		case 4: 
			startMusic(command&0xFF);
			break;
		case 6:
			reinitFM();
			break;
		case 7:
			stopMusic();
			break;
		case 13: 
			setFMVolume(command&0xFF); 
			break;
		default: 
			debug(1,"musicCommand %d ignored.\n",command>>8);
	}
}

void SkyMusicBase::setFMVolume(uint16 param)
{
	_musicVolume = param;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++)
		_channels[cnt]->updateVolume(_musicVolume);
}

void SkyMusicBase::stopMusic(void)
{
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		_channels[cnt]->stopNote();
		delete _channels[cnt];
	}
	_numberOfChannels = 0;
}

void SkyMusicBase::updateTempo(void)
{
	uint16 tempoMul = _musicTempo0*_musicTempo1;
	uint16 divisor = 0x4446390/23864;
	_tempo = (tempoMul / divisor)<<16;
	_tempo |= (((tempoMul%divisor)<<16) | (tempoMul/divisor)) / divisor;
}

void SkyMusicBase::loadNewMusic(void)
{
	uint16 musicPos;
	if (_onNextPoll.musicToProcess > _musicData[_musicDataLoc]) {
		error("Music %d requested but doesn't exist in file.\n", _onNextPoll.musicToProcess);
		return;
	}
	if (_currentMusic != 0) stopMusic();

	_currentMusic = _onNextPoll.musicToProcess;
	_onNextPoll.musicToProcess = 0;

	if (_currentMusic != 0) {
		musicPos = (_musicData[_musicDataLoc+2]<<8) | _musicData[_musicDataLoc+1];
		musicPos += _musicDataLoc+((_currentMusic-1)<<1);
		musicPos = ((_musicData[musicPos+1]<<8) | _musicData[musicPos]) + _musicDataLoc;

		_musicTempo0 = _musicData[musicPos];
		_musicTempo1 = _musicData[musicPos+1];

		setupChannels(_musicData + musicPos + 2);

		updateTempo();
	}
}

void SkyMusicBase::pollMusic(void)
{
	uint8 newTempo;
	if (_onNextPoll.doReInit) startDriver();
	if (_onNextPoll.doStopMusic) stopMusic();
	if (_onNextPoll.musicToProcess == _currentMusic) _onNextPoll.musicToProcess = 0;
	if (_onNextPoll.musicToProcess)	loadNewMusic();

	_aktTime += _tempo;

	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		newTempo = _channels[cnt]->process((uint16)(_aktTime >> 16));
		if (newTempo) {
			_musicTempo1 = newTempo;
			updateTempo();
		}
	}
	_aktTime &= 0xFFFF;
}
