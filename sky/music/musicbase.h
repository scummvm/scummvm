/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef MUSICBASE_H
#define MUSICBASE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/mutex.h"

namespace Sky {

class Disk;

#define FILES_PER_SECTION 4

typedef struct {
	bool doReInit, doStopMusic;
	uint8 musicToProcess;
} Actions;

class ChannelBase {
public:
	virtual ~ChannelBase() {};
	virtual void stopNote(void) = 0;
	virtual uint8 process(uint16 aktTime) = 0;
	virtual void updateVolume(uint16 pVolume) = 0;
	virtual bool isActive(void) = 0;
private:
};

class MusicBase {
public:
	MusicBase(Disk *pDisk);
	virtual ~MusicBase(void);
	void loadSection(uint8 pSection);
	void musicCommand(uint16 command);
	void startMusic(uint16 param) { _onNextPoll.musicToProcess = param & 0xF; }; // 4
	void stopMusic();                                                            // 7
	bool musicIsPlaying(void);
	virtual void setVolume(uint8 volume) = 0;
	uint8 giveVolume(void) { return (uint8)_musicVolume; };
	uint8 giveCurrentMusic(void) { return _currentMusic; };

protected:

	Disk *_skyDisk;
	uint8 *_musicData;
	uint8 _allowedCommands;
	uint16 _musicDataLoc;
	uint16 _driverFileBase;

	uint16 _musicVolume, _numberOfChannels;
	uint8 _currentMusic, _currentSection;
	uint8 _musicTempo0; // can be changed by music stream
	uint8 _musicTempo1; // given once per music
	uint32 _tempo;      // calculated from musicTempo0 and musicTempo1
	uint32 _aktTime;
	Actions _onNextPoll;
	ChannelBase *_channels[10];
	Common::Mutex _mutex;

	virtual void setupPointers(void) = 0;
	virtual void setupChannels(uint8 *channelData) = 0;

	void updateTempo(void);
	void loadNewMusic(void);
	//-                           functions from CommandTable @0x90 (the main interface)
	virtual void startDriver(void) = 0;                                          // 0
	void StopDriver(void);                                                       // 1
	void setTempo(uint16 newTempo);                                              // 2
	void pollMusic();                                                            // 3
	void reinitFM(void) { _onNextPoll.doReInit = true; };                        // 6
	void setFMVolume(uint16 param);                                              // 13
};

} // End of namespace Sky

#endif //MUSICBASE_H
