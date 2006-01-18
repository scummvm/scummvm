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

#ifndef CD_MSA_H
#define CD_MSA_H

#include "cdaudio.h"

class MsaCDPlayer : public CDAudio {
public:
 	MsaCDPlayer(OSystem *sys);

 	bool init();
 	void release();

 	bool poll();
 	void update();
 	void play(int track, int num_loops, int start_frame, int duration);
 	void stop();

 	void setVolume(int volume);

 private:
 	void initInternal();

 	OSystem *_sys;
 	UInt16 _msaRefNum;

	UInt32 _volumeLLimit;
	UInt32 _volumeRLimit;

 	// cdrom
	AlbumInfoType _msaAlbum;
	UInt16 _msaLoops;

	UInt32 _msaStopTime;

	UInt32 _msaStartFrame;
	UInt32 _msaDuration;

	UInt16 _msaTrack;
	UInt32 _msaTrackStartSu, _msaTrackEndSu;
	UInt32 _msaTrackLength;
};
 
#endif
