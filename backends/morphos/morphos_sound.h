/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 Rüdiger Hanke (MorphOS port)
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
 * MorphOS-specific header file
 *
 * $URL$
 * $Id$
 *
 */

#ifndef MORPHOS_SOUND_H
#define MORPHOS_SOUND_H

#include <exec/semaphores.h>
#include <devices/etude.h>

class OSystem_MorphOS;

int morphos_sound_thread(OSystem_MorphOS *syst, ULONG SampleType);
bool init_morphos_music(ULONG MidiUnit, ULONG DevFlags);
void exit_morphos_music();
bool etude_available();

extern STRPTR ScummMusicDriver;
extern LONG   ScummMidiUnit;
extern IOMidiRequest *ScummMidiRequest;

#endif

