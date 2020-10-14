/** @file music.h
	@brief
	This file contains music playing routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MUSIC_H
#define MUSIC_H

#include "sys.h"

/** Track number of the current playing music */
int32 currentMusic;

/** Music volume
	@param current volume number */
void musicVolume(int32 volume);
/** Play CD music
	@param track track number to play */
void playTrackMusicCd(int32 track);
/** Stop CD music */
void stopTrackMusicCd();
/** Play MP3 music
	@param track track number to play */
void playTrackMusicMp3(int32 track);
/** Stop MP3 music */
void stopTrackMusicMp3();
/** Generic play music, according with settings it plays CD or high quality sounds instead
	@param track track number to play*/
void playTrackMusic(int32 track);
/** Generic stop music according with settings */
void stopTrackMusic();
/** Play MIDI music
	@param midiIdx music index under mini_mi_win.hqr*/
void playMidiMusic(int32 midiIdx, int32 loop);
/** Stop MIDI music */
void stopMidiMusic();

/** Initialize CD-Rom */
int32 initCdrom();

/** Stop MIDI and Track music */
void stopMusic();

#endif
