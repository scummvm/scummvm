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

class IMuse : public MusicEngine {
private:
	OSystem *_system;
	IMuseInternal *_target;
	mutable Common::MutexRef _mutex;

	IMuse(OSystem *system, IMuseInternal *target);
	void in() const;
	void out() const;

public:
	~IMuse();

	enum {
		PROP_TEMPO_BASE,
		PROP_NATIVE_MT32,
		PROP_GS,
		PROP_LIMIT_PLAYERS,
		PROP_RECYCLE_PLAYERS,
		PROP_DIRECT_PASSTHROUGH
	};

	void on_timer(MidiDriver *midi);
	void pause(bool paused);
	int save_or_load(Serializer *ser, ScummEngine *scumm);
	void setMusicVolume(int vol);
	void startSound(int sound);
	void stopSound(int sound);
	void stopAllSounds();
	int getSoundStatus(int sound) const;
	bool get_sound_active(int sound) const;
	int getMusicTimer() const;
	int32 doCommand(int a, int b, int c, int d, int e, int f, int g, int h);
	int32 doCommand(int numargs, int args[]);
	int clear_queue();
	void setBase(byte **base);
	uint32 property(int prop, uint32 value);
	void terminate();

	// Factory methods
	static IMuse *create(OSystem *syst, MidiDriver *nativeMidiDriver, MidiDriver *adlibMidiDriver);
};

} // End of namespace Scumm

#endif
