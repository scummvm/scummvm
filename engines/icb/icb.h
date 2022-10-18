/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_H
#define ICB_H

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "engines/icb/detection.h"

#include "common/random.h"

namespace ICB {

class IcbEngine : public Engine {
public:
	IcbEngine(OSystem *syst, const IcbGameDescription *gameDescIcb);
	virtual ~IcbEngine();

	Common::RandomSource *getRandomSource() { return _randomSource; }
	static Common::Array<Common::Keymap *> initKeymapsIcb(const char *target);
	static Common::Array<Common::Keymap *> initKeymapsEldorado(const char *target);
	bool hasFeature(EngineFeature f) const override;
	IcbGameType getGameType() { return _gameType; }

protected:
	// Engine APIs
	Common::Error run() override;

private:
	IcbGameType _gameType;

	void initGfx(bool hwAccel, bool fullscreen);
	void mainLoop();
	Common::RandomSource *_randomSource;
	const ADGameDescription *_gameDescription;
};

extern IcbEngine *g_icb;

} // End of namespace ICB

#endif // ICB_H
