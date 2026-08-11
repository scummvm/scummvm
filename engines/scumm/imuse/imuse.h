/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SCUMM_IMUSE_H
#define SCUMM_IMUSE_H

#include "audio/mididrv.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/mutex.h"
#include "scumm/music.h"

class MidiDriver;
class OSystem;

namespace Scumm {

class IMuseInternal;
class Player;
class ScummEngine;

typedef void (*sysexfunc)(Player *, const byte *, uint16);

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
		PROP_LIMIT_PLAYERS,
		PROP_RECYCLE_PLAYERS,
		PROP_QUALITY,
		PROP_MUSICVOLUME,
		PROP_SFXVOLUME
	};

public:
	virtual void on_timer(MidiDriver *midi) = 0;
	virtual void pause(bool paused) = 0;
	virtual void saveLoadIMuse(Common::Serializer &ser, ScummEngine *scumm, bool fixAfterLoad = true) = 0;
	virtual bool get_sound_active(int sound) const = 0;
	virtual int32 doCommand(int numargs, int args[]) = 0;
	virtual int clear_queue() = 0;
	virtual uint32 property(int prop, uint32 value) = 0;
	virtual void addSysexHandler(byte mfgID, sysexfunc handler) = 0;

public:
	virtual void startSoundWithNoteOffset(int sound, int offset) = 0;

	// MusicEngine base class methods. Only this one is implemented:
	void setQuality(int qual) override { property(PROP_QUALITY, qual); }

public:
	// Factory methods
	static IMuse *create(ScummEngine *vm, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver, MidiDriverFlags sndType, bool nativeMT32);
};

} // End of namespace Scumm

#endif
