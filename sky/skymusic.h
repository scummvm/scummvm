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


#ifndef SKYMUSIC_H
#define SKYMUSIC_H

#include "stdafx.h"
#include "sound/fmopl.h"
#include "sound/mixer.h"
#include "common/engine.h"
#include "skychannel.h"
#include "disk.h"

#define MUSIC_BASE_FILE 60202 // usually 60200 ( + 0 for Roland and +2 for Adlib)
#define FILES_PER_SECTION 4

typedef struct {
	bool doReInit, doStopMusic;
	uint8 musicToProcess;
} Actions;

class SkyMusic {
public:
	SkyMusic(SoundMixer *mixer, SkyDisk *pSkyDisk);
	~SkyMusic(void);
	//void loadData(uint8 *pMusicData);
	void loadSectionMusic(uint8 pSection);
	void musicCommand(uint16 command);
	void startMusic(uint16 param) { _onNextPoll.musicToProcess = param & 0xF; }; // 4
	
private:
	SoundMixer *_mixer;
	SkyDisk *_skyDisk;
	FM_OPL *_opl;
	uint8 *_musicData;
	uint8 *_initSequence;
	uint8 _allowedCommands;
	uint16 _musicDataLoc;
	SkyChannel *_channels[10];

	uint16 _musicVolume, _numberOfChannels;
	uint8 _currentMusic;
	uint8 _musicTempo0; // can be changed by music stream
	uint8 _musicTempo1; // given once per music
	uint32 _tempo;      // calculated from musicTempo0 and musicTempo1
	uint32 _aktTime;
	uint32 _sampleRate, _nextMusicPoll;
	Actions _onNextPoll;
	
	void premixerCall(int16 *buf, uint len);
	static void passMixerFunc(void *param, int16 *buf, uint len);
	void updateTempo(void);
	void loadNewMusic(void);
	//-                           functions from CommandTable @0x90 (the main interface)
	void startAdlibDriver(void);                                                 // 0
	void StopDriver(void);                                                       // 1
	void setTempo(uint16 newTempo);                                              // 2
	void pollMusic();                                                            // 3
	void reinitFM(void) { _onNextPoll.doReInit = true; };                        // 6
	void stopMusic();                                                            // 7
	void setFMVolume(uint16 param);                                              // 13
};

#endif //SKYMUSIC_H
