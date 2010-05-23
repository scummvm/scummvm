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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/portdefs.h $
 * $Id: portdefs.h 38687 2009-02-21 12:08:52Z joostp $
 *
 */

#ifndef PSP_THREAD_H
#define PSP_THREAD_H

enum ThreadPriority {
	PRIORITY_MAIN_THREAD = 36,
	PRIORITY_AUDIO_THREAD = 35,		// We'll alternate between this and main thread priority
	PRIORITY_TIMER_THREAD = 30,
	PRIORITY_POWER_THREAD = 20,
	PRIORITY_DISPLAY_THREAD = 17
};

enum StackSizes {
	STACK_AUDIO_THREAD = 16 * 1024,
	STACK_TIMER_THREAD = 16 * 1024,
	STACK_DISPLAY_THREAD = 2 * 1024,
	STACK_POWER_THREAD = 4 * 1024
};
	
#endif /* PSP_THREADS_H */


