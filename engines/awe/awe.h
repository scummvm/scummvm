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
#include "common/random.h"
#include "engines/engine.h"
#include "awe/detection.h"

namespace Awe {

class AweEngine : public ::Engine {
private:
	const AweGameDescription *_gameDescription;
	Common::RandomSource _random;

public:
	AweEngine(OSystem *syst, const AweGameDescription *gameDesc);
	~AweEngine() override;

	Common::Error run() override;
#ifdef TODO
	bool hasFeature(EngineFeature f) const override;
	bool isDemo() const;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
#endif

	int getRandomNumber(int max) {
		return _random.getRandomNumber(max);
	}
	int getRandomNumber(int min, int max) {
		return min + _random.getRandomNumber(max - min);
	}

	DataType getDataType() const;
	Common::Language getLanguage() const;
};

extern AweEngine *g_engine;

} // namespace Awe

#endif
