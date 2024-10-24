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

#ifndef TSAGE_TSAGE_H
#define TSAGE_TSAGE_H

#include "engines/engine.h"
#include "gui/debugger.h"

#include "tsage/resources.h"
#include "tsage/debugger.h"
#include "tsage/events.h"
#include "tsage/resources.h"
#include "tsage/detection.h"


namespace TsAGE {

enum {
	kRingDebugScripts = 1 << 0,
	ktSageSound = 1 << 1,
	ktSageCore = 1 << 2,
	ktSageDebugGraphics = 1 << 3
};

class TSageEngine : public Engine {
private:
	const tSageGameDescription *_gameDescription;
public:
	TSageEngine(OSystem *system, const tSageGameDescription *gameDesc);
	~TSageEngine() override;
	bool hasFeature(EngineFeature f) const override;

	MemoryManager _memoryManager;

	const char *getGameId() const;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Path getPrimaryFilename() const;

	virtual Common::Error init();
	Common::Error run() override;
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	void syncSoundSettings() override;

	void initialize();
	void deinitialize();
};

extern TSageEngine *g_vm;

#define ALLOCATE_HANDLE(x) g_vm->_memoryManager.allocate(x)
#define ALLOCATE(x) g_vm->_memoryManager.allocate2(x)
#define DEALLOCATE(x) g_vm->_memoryManager.deallocate(x)

} // End of namespace TsAGE

#endif
