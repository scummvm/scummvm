/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 
 #ifndef CD_POCKETTUNES_H
 #define CD_POCKETTUNES_H
 
 #include "cdaudio.h"
 #include "pockettunes.h"
 
 class PckTunesCDPlayer : public CDAudio {
 public:
 	PckTunesCDPlayer(OSystem *sys);

 	bool init();
 	void release();
 	
 	bool poll();
 	void update();
 	void play(int track, int num_loops, int start_frame, int duration);
 	void stop();
 	
 	void setVolume(int volume);
 	
 private:
 	OSystem *_sys;
 	Char gameP[15];
 	
 	PocketTunesAction *_pAction;	// prevent memory fragmentation
 	EventType _eAction;
 	
 	UInt32 getStatus();
 	void setPosition(UInt32 value);
 	UInt32 getDuration();
 	UInt32 getPosition(UInt32 deft);
 
 	UInt32 _volumeLimit;	
 	UInt32 getVolumeLimit();
	
 	Boolean _isPlaying;
 	// cdrom
	UInt16 _pckLoops, _pckTrack;
	UInt32 _pckTrackStartFrame, _pckTrackEndFrame;
	UInt32 _pckStopTime, _pckTrackDuration;
 };
 
 #endif
 
