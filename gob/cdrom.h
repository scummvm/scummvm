/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"

namespace Gob {

extern bool cd_globFlag;

void cd_readLIC(const char *fname);
void cd_freeLICbuffer(void);

void cd_startTrack(const char *s);
void cd_playBgMusic();
void cd_playMultMusic();
void cd_play(uint32 from, uint32 to);
int32 cd_getTrackPos(void);
void cd_stopPlaying(void);
void cd_stop(void);
void cd_testCD(int trySubst, const char *label);

} // End of namespace Gob
