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

#ifndef AWE_AWE_H
#define AWE_AWE_H

#include "common/scummsys.h"
#include "engines/engine.h"
#include "awe/detection.h"
#include "awe/intern.h"
#include "awe/logic.h"
#include "awe/resource.h"
#include "awe/system_stub.h"
#include "awe/video.h"

namespace Awe {

#define BYPASS_PROTECTION

class AweEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;

private:
	void setup();
	void finish();
	void processInput();

public:
	SystemStub *_stub;
	Logic _log;
	Resource _res;
	Video _vid;
	int _stateSlot = 0;

public:
	AweEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~AweEngine() override;

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool isDemo() const;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
};

extern AweEngine *g_engine;

} // namespace Awe

#endif
