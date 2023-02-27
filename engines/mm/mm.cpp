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

#include "mm/mm.h"

namespace MM {

MMEngine *g_engine;

MMEngine::MMEngine(OSystem *syst, const MM::MightAndMagicGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("MightAndMagic") {
	g_engine = this;
}

bool MMEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

uint32 MMEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MMEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language MMEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Platform MMEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool MMEngine::getIsCD() const {
	return getFeatures() & ADGF_CD;
}

} // namespace MM
