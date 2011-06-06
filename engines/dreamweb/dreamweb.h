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
 * $URL: https://svn.scummvm.org:4444/svn/dreamweb/dreamweb.h $
 * $Id: dreamweb.h 77 2011-05-18 14:26:43Z digitall $
 *
 */

#ifndef DREAMWEB_H
#define DREAMWEB_H

#include "common/scummsys.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/error.h"

#include "engines/engine.h"
#include "dreamweb/console.h"

namespace Graphics {
	class Surface;
}

namespace DreamWeb {

// Engine Debug Flags
enum {
	kDebugAnimation = (1 << 0),
	kDebugSaveLoad = (1 << 1)
};

struct DreamWebGameDescription;

class DreamWebEngine : public Engine {
private:
	DreamWebConsole			*_console;
	bool					_vSyncInterrupt;
	static DreamWebEngine	*_instance;

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

public:
	DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc);
	virtual ~DreamWebEngine();
	static DreamWebEngine* instance() { return _instance; }

	void setVSyncInterrupt(bool flag);
	void waitForVSync();

	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *desc);

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();

//dreamgen public api:
	uint8 randomNumber() { return _rnd.getRandomNumber(255); }

private:
	const DreamWebGameDescription *_gameDescription;
	Common::RandomSource _rnd;
	Common::Point _mouse;
};

} // End of namespace DreamWeb

#endif
