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

#include "common/debug.h"
#include "common/stream.h"

#include "engines/advancedDetector.h"

#include "sludge/sludge.h"
#include "sludge/detection.h"

namespace Sludge {

uint SludgeEngine::getLanguageID() const { return _gameDescription->languageID; }
const char *SludgeEngine::getGameId() const { return _gameDescription->desc.gameId;}
uint32 SludgeEngine::getFeatures() const { return _gameDescription->desc.flags; }
Common::Language SludgeEngine::getLanguage() const { return _gameDescription->desc.language; }
const char *SludgeEngine::getGameFile() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

} // End of namespace Sludge

class SludgeMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "sludge";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override {
		*engine = new Sludge::SludgeEngine(syst, (const Sludge::SludgeGameDescription *)desc);
		return Common::kNoError;
	}
};

#if PLUGIN_ENABLED_DYNAMIC(SLUDGE)
	REGISTER_PLUGIN_DYNAMIC(SLUDGE, PLUGIN_TYPE_ENGINE, SludgeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SLUDGE, PLUGIN_TYPE_ENGINE, SludgeMetaEngine);
#endif
