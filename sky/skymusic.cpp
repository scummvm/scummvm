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

#include "skymusic.h"

void SkyMusic::passMixerFunc(void *param, int16 *buf, uint len) {

	((SkyMusic*)param)->premixerCall(buf, len);
}

void SkyMusic::premixerCall(int16 *buf, uint len) {

	if (_musicData == NULL) {
		// no music loaded
		memset(buf, 0, len * sizeof(int16));
		return;
	} else if ((_currentMusic == 0) || (_numberOfChannels == 0)) {
		// music loaded but not played as of yet
		memset(buf, 0, len * sizeof(int16));
		// poll anyways as pollMusic() can activate the music
		pollMusic();
		_nextMusicPoll = _sampleRate/50;
		return;
	}
	uint32 render;
	while (len) {
		render = (len > _nextMusicPoll) ? (_nextMusicPoll) : (len);
		len -= render;
		_nextMusicPoll -= render;
		YM3812UpdateOne(_opl, buf, render);
		buf += render;
		if (_nextMusicPoll == 0) {
			pollMusic();
			_nextMusicPoll = _sampleRate/50;
		}
	}
}

SkyMusic::SkyMusic(SoundMixer *mixer, SkyDisk *pSkyDisk) {

	_musicData = NULL;
	_allowedCommands = 0;
	_mixer = mixer;
	_sampleRate = g_system->property(OSystem::PROP_GET_SAMPLE_RATE, 0);
	int env_bits = g_system->property(OSystem::PROP_GET_FMOPL_ENV_BITS, NULL);
	int eg_ent = g_system->property(OSystem::PROP_GET_FMOPL_EG_ENT, NULL);
	OPLBuildTables((env_bits ? env_bits : FMOPL_ENV_BITS_HQ), (eg_ent ? eg_ent : FMOPL_EG_ENT_HQ));
	_opl = OPLCreate(OPL_TYPE_YM3812, 3579545, _sampleRate);
	_mixer->setupPremix(this, passMixerFunc);
	_skyDisk = pSkyDisk;
}

SkyMusic::~SkyMusic(void)
{
	if (_currentMusic) stopMusic();
	_mixer->setupPremix(NULL, NULL);
	if (_musicData) free(_musicData);
	OPLDestroy(_opl);
}

void SkyMusic::loadSectionMusic(uint8 pSection)
{
	if (_currentMusic) stopMusic();
	if (_musicData) free(_musicData);
	_musicData = _skyDisk->loadFile(MUSIC_BASE_FILE + FILES_PER_SECTION*pSection,NULL);
	_allowedCommands = 0;
	_musicTempo0 = 0x78; // init constants taken from idb file, area ~0x1060
	_musicTempo1 = 0xC0;
	_musicVolume = 0x100;
	_numberOfChannels = _currentMusic = 0;
	_musicDataLoc = (_musicData[0x1202]<<8)|_musicData[0x1201];
	_initSequence = _musicData+0xE91;
	_onNextPoll.doReInit = false;
	_onNextPoll.doStopMusic = false;
	_onNextPoll.musicToProcess = 0;
	_tempo = _aktTime = 0x10001;
	_nextMusicPoll = 0;
	startAdlibDriver();
}

/*void SkyMusic::loadData(uint8 *pMusicData)
{
	if (_currentMusic) stopMusic();
	if (_musicData) free(_musicData);
	_musicData = pMusicData;
	_allowedCommands = 0;
	_musicTempo0 = 0x78; // init constants taken from idb file, area ~0x1060
	_musicTempo1 = 0xC0;
	_musicVolume = 0x100;
	_numberOfChannels = _currentMusic = 0;
	_musicDataLoc = (_musicData[0x1202]<<8)|_musicData[0x1201];
	_initSequence = _musicData+0xE91;
	_onNextPoll.doReInit = false;
	_onNextPoll.doStopMusic = false;
	_onNextPoll.musicToProcess = 0;
	_tempo = _aktTime = 0x10001;
	_nextMusicPoll = 0;
}*/

void SkyMusic::musicCommand(uint16 command)
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
			//startAdlibDriver(); break;
			debug(1,"SkyMusic: got call to startAdlibDriver(). Not necessary in this implementation.\n");
			break;
		case 1: 
			//StopDriver(command&0xFF); break;
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

void SkyMusic::setFMVolume(uint16 param)
{
	_musicVolume = param;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++)
		_channels[cnt]->updateVolume(_musicVolume);
}

void SkyMusic::startAdlibDriver(void)
{
	uint16 cnt = 0;
	while (_initSequence[cnt] || _initSequence[cnt+1]) {
		OPLWriteReg(_opl, _initSequence[cnt], _initSequence[cnt+1]);
		cnt += 2;
	}
	_allowedCommands = 0xD;
}

void SkyMusic::stopMusic(void)
{
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		_channels[cnt]->stopNote();
		delete _channels[cnt];
	}
	_numberOfChannels = 0;
}

void SkyMusic::updateTempo(void)
{
	uint16 tempoMul = _musicTempo0*_musicTempo1;
	uint16 divisor = 0x4446390/23864;
	_tempo = (tempoMul / divisor)<<16;
	_tempo |= (((tempoMul%divisor)<<16) | (tempoMul/divisor)) / divisor;
}

void SkyMusic::loadNewMusic(void)
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

		_musicTempo1 = _musicData[musicPos+1];
		_musicTempo0 = _musicData[musicPos];
		_numberOfChannels = _musicData[musicPos+2];
		musicPos += 3;

		for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
			uint16 chDataStart = ((_musicData[musicPos+1]<<8) | _musicData[musicPos]) + _musicDataLoc;
			_channels[cnt] = new SkyChannel(_musicData, chDataStart, _opl);
			musicPos += 2;		
		}
		updateTempo();
	}
}

void SkyMusic::pollMusic(void)
{
	uint8 newTempo;
	if (_onNextPoll.doReInit) startAdlibDriver();
	if (_onNextPoll.doStopMusic) stopMusic();
	if (_onNextPoll.musicToProcess) loadNewMusic();
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
