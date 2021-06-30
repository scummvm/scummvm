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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SAGA2_H
#define SAGA2_H

#include "common/random.h"
#include "common/serializer.h"
#include "common/system.h"
#include "engines/engine.h"

#include "saga2/idtypes.h"
#include "saga2/weapons.h"

namespace Video {
class SmackerDecoder;
}

namespace Saga2 {

class Timer;
class TimerList;

enum {
	kDebugResources = 1 << 0,
	kDebugActors    = 1 << 1,
	kDebugScripts   = 1 << 2,
	kDebugEventLoop = 1 << 3,
	kDebugInit      = 1 << 4,
	kDebugTiles     = 1 << 5,
	kDebugPalettes  = 1 << 6,
	kDebugLoading   = 1 << 7
};

#define TICKSPERSECOND (728L/10L)

class Saga2Engine : public Engine {
public:
	Saga2Engine(OSystem *syst);
	~Saga2Engine();

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return true; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);

	void loadExeResources();
	void freeExeResources();

	// itevideo.cpp
	void startVideo(const char *fileName, int x, int y);
	bool checkVideo(void);
	void endVideo(void);
	void abortVideo(void);

public:
	// We need random numbers
	Common::RandomSource *_rnd;

	WeaponStuff _weaponRack[kMaxWeapons];
	weaponID _loadedWeapons = 0;
	Common::List<TimerList *> _timerLists;
	Common::List<Timer *> _timers;

private:
	Video::SmackerDecoder *_smkDecoder;
	int _videoX, _videoY;
};

extern Saga2Engine *g_vm;

} // End of namespace Saga2

#endif
