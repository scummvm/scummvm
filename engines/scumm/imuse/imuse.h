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
 * $URL$
 * $Id$
 *
 */

#ifndef IMUSE_H
#define IMUSE_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "scumm/music.h"

class MidiDriver;
class OSystem;

namespace Scumm {

class IMuseInternal;
class ScummEngine;
class Serializer;

/**
 * iMuse implementation interface.
 * MusicEngine derivative for state-tracked, interactive,
 * persistent event-based music playback and control.
 * This class serves as an interface to actual implementations
 * so that client code is not exposed to the details of
 * any specific implementation.
 */
class IMuse : public MusicEngine {
public:
	enum {
		PROP_TEMPO_BASE,
		PROP_NATIVE_MT32,
		PROP_GS,
		PROP_LIMIT_PLAYERS,
		PROP_RECYCLE_PLAYERS,
		PROP_DIRECT_PASSTHROUGH,
		PROP_GAME_ID
	};

public:
	virtual void on_timer(MidiDriver *midi) = 0;
	virtual void pause(bool paused) = 0;
	virtual int save_or_load(Serializer *ser, ScummEngine *scumm) = 0;
	virtual bool get_sound_active(int sound) const = 0;
	virtual int32 doCommand(int numargs, int args[]) = 0;
	virtual int clear_queue() = 0;
	virtual void setBase(byte **base) = 0;
	virtual uint32 property(int prop, uint32 value) = 0;

public:
	// MusicEngine base class methods.
	// Not actually redefined here because none are implemented.

public:
	// Factory methods
	static IMuse *create(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver);
};

} // End of namespace Scumm

#endif
