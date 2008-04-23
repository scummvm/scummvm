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
 * $URL$
 * $Id$
 *
 */

#ifndef MADE_H
#define MADE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"

#include "graphics/surface.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Made {

enum MadeGameFeatures {
	GF_DEMO = 1 << 0,
	GF_CD = 1 << 1,
	GF_CD_COMPRESSED = 1 << 2,
	GF_FLOPPY = 1 << 3
};

struct MadeGameDescription;

class ProjectReader;
class PmvPlayer;
class Screen;
class ScriptInterpreter;
class GameDatabase;
class Music;

class MadeEngine : public ::Engine {
	int _gameId;
	Common::KeyState _keyPressed;

protected:

	int init();
	int go();

public:
	MadeEngine(OSystem *syst, const MadeGameDescription *gameDesc);
	virtual ~MadeEngine();
	int getGameId() {
		return _gameId;
	}

	Common::RandomSource *_rnd;
	const MadeGameDescription *_gameDescription;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	void update_events();

private:
public:
	PmvPlayer *_pmvPlayer;
	ProjectReader *_res;
	Screen *_screen;
	GameDatabase *_dat;
	ScriptInterpreter *_script;
	Music *_music;

	int _eventMouseX, _eventMouseY;
	uint16 _eventKey;
	int _soundRate;
	int _musicVolume;

	int32 _timers[50];
	int16 getTimer(int16 timerNum);
	void setTimer(int16 timerNum, int16 value);
	void resetTimer(int16 timerNum);
	int16 allocTimer();
	void freeTimer(int16 timerNum);

};

} // End of namespace Made

#endif /* MADE_H */
