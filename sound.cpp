/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 * Change Log:
 * $Log$
 * Revision 1.2  2001/10/16 10:01:48  strigeus
 * preliminary DOTT support
 *
 * Revision 1.1.1.1  2001/10/09 14:30:13  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

void Scumm::addSoundToQueue(int sound) {
	_vars[VAR_LAST_SOUND] = sound;
	ensureResourceLoaded(4, sound);
	addSoundToQueue2(sound);
}

void Scumm::addSoundToQueue2(int sound) {
	if (_soundQue2Pos < 10) {
		_soundQue2[_soundQue2Pos++] = sound;
	}
}

void Scumm::unkSoundProc22() {
	byte d;
	int i,j;
	int num;
	int16 data[16];

	while (_soundQue2Pos){
		d=_soundQue2[--_soundQue2Pos];
		if (d)
			playSound(d);
	}

#if 0
	for (i=0; i<_soundQuePos; ) {
		num = _soundQue[i++];
		for (j=0; j<16; j++)
			data[j] = 0;
		if (num>0) {
			for (j=0; j<num; j++)
				_soundQue[i+j] = data[j];
			i += num;
			/* XXX: not implemented */
			warning("unkSoundProc22: not implemented");
//			vm.vars[VAR_SOUNDRESULT] = soundProcPtr1(...);
		}
	}
	#endif
	_soundQuePos = 0;
}

void Scumm::playSound(int sound) {
	getResourceAddress(4, sound);
	/* XXX: not implemented */
//	warning("stub playSound(%d)", sound);
}

int Scumm::unkSoundProc23(int a) {
	/* TODO: implement this */
//	warning("unkSoundProc23: not implemented");
	return 0;
}

void Scumm::unkSoundProc1(int a) {
	/* TODO: implement this */
//	warning("unkSoundProc: not implemented");
}

void Scumm::soundKludge(int16 *list) {
	int16 *ptr;
	int i;

	if (list[0]==-1) {
		unkSoundProc22();
		return;
	}
	_soundQue[_soundQuePos++] = 8;

	ptr = _soundQue + _soundQuePos;
	_soundQuePos += 8;

	for (i=0; i<8; i++)
		*ptr++ = list[i];
	if (_soundQuePos > 0x100)
		error("Sound que buffer overflow");
}