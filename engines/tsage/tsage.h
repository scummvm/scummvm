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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/tsage.h $
 * $Id: tsage.h 212 2011-02-06 10:19:01Z dreammaster $
 *
 */

#ifndef TSAGE_H
#define TSAGE_H

#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "common/rect.h"
#include "audio/mixer.h"
#include "common/file.h"

#include "tsage/core.h"
#include "tsage/resources.h"
#include "tsage/debugger.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "tsage/resources.h"


namespace tSage {

enum {
	GType_Ringworld = 0
};

enum {
	GF_CD		= 1 <<  0,
	GF_LNGUNK	= 1 << 15
};

enum {
	kRingDebugScripts = 1 << 0
};

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_CENTRE_X 160
#define SCREEN_CENTRE_Y 100

class TSageEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
public:
	TSageEngine(OSystem *system, const ADGameDescription *gameDesc);
	~TSageEngine();
	virtual bool hasFeature(EngineFeature f) const;

	MemoryManager _memoryManager;
	Debugger *_debugger;
	RlbManager *_tSageManager;
	RlbManager *_dataManager;

	const char *getGameId() const;

	virtual Common::Error init();
	virtual Common::Error run();
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const char *desc);
	Common::String generateSaveName(int slot);

	void initialise();
};

extern TSageEngine *_vm;

#define ALLOCATE_HANDLE(x) _vm->_memoryManager.allocate(x)
#define ALLOCATE(x) _vm->_memoryManager.allocate2(x)
#define DEALLOCATE(x) _vm->_memoryManager.deallocate(x)

} // End of namespace tSage

#endif
