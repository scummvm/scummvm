/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_SFXPLAYER_H_
#define CINE_SFXPLAYER_H_

namespace Cine {

struct BasesonEntry {
	char name[14];
	uint32 offset;
	uint32 size;
	uint32 unpackedSize;
};

struct SfxState {
	uint8 *songData;
	int currentInstrumentChannel[4];
	uint8 *instruments[15];
	int currentOrder;
	int currentPos;
	int numOrders;
};

extern uint16 snd_eventsDelay;
extern int snd_songIsPlaying;
extern uint8 snd_nullInstrument[];
extern SfxState snd_sfxState;

extern int snd_loadBasesonEntries(const char *fileName);
extern void snd_clearBasesonEntries();
extern void snd_stopSong();
extern void snd_freeSong();
extern int snd_loadSong(const char *songName);
extern void snd_fadeOutSong();
extern void snd_playSong();
extern void snd_handleEvents();
extern void snd_handlePattern(int channelNum, const uint8 *patternData);

} // End of namespace Cine

#endif				/* _SFXPLAYER_H_ */
