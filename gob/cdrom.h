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

class CDROM {
public:
	bool _cdPlaying;

	void readLIC(const char *fname);
	void freeLICbuffer(void);

	void startTrack(const char *s);
	void playBgMusic();
	void playMultMusic();
	void play(uint32 from, uint32 to);
	int32 getTrackPos(void);
	void stopPlaying(void);
	void stop(void);
	void testCD(int trySubst, const char *label);

	CDROM(GobEngine *vm);

protected:
	byte *_LICbuffer;
	char _curTrack[16];
	uint16 _numTracks;
	uint32 _trackStop;
	uint32 _startTime;
	GobEngine *_vm;
};

} // End of namespace Gob
