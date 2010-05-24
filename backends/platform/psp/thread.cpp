/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.h $
 * $Id: osys_psp.h 49173 2010-05-24 03:05:17Z bluddy $
 *
 */

#include <time.h> 
#include <psptypes.h>
#include <psprtc.h>
#include <pspthreadman.h> 

#include "backends/platform/psp/thread.h"
 
void PspThread::delayMillis(uint32 ms) {
	sceKernelDelayThread(ms * 1000);
}

void PspThread::delayMicros(uint32 us) {
	sceKernelDelayThread(us);
}

uint32 PspThread::getMillis() {
	uint32 ticks[2];
	sceRtcGetCurrentTick((u64 *)ticks);
	return (ticks[0]/1000);	
}

uint32 PspThread::getMicros() {
	uint32 ticks[2];
	sceRtcGetCurrentTick((u64 *)ticks);
	return ticks[0]; 
}

