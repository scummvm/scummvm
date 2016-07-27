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
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/system.h"

#include "macventure/macventure.h"

namespace MacVenture {

	const char *MacVentureEngine::getGameFileName() const {
		return _gameDescription->filesDescriptions[0].fileName;
	}

}

namespace MacVenture {

#include "macventure/detection_tables.h"

static const PlainGameDescriptor macventureGames[] = {
	{ "shadowgate", "Shadowgate" },
	{ "deja_vu", "Deja Vu"},
	{ 0, 0 }
};

class MacVentureMetaEngine : public AdvancedMetaEngine {
public:
	MacVentureMetaEngine() : AdvancedMetaEngine(MacVenture::gameDescriptions, sizeof(ADGameDescription), macventureGames) {
		_guiOptions = GUIO0();
		_md5Bytes = 5000000; // TODO: Upper limit, adjust it once all games are added
	}

	virtual const char * getName() const override {
		return "MacVenture";
	}
	virtual const char * getOriginalCopyright() const override {
		return "(C) ICOM Simulations";
	}

	virtual bool createInstance(OSystem * syst, Engine ** engine, const ADGameDescription * desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
};

bool MacVentureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool MacVentureMetaEngine::createInstance(OSystem * syst, Engine ** engine, const ADGameDescription *game) const {
	if (game) {
		*engine = new MacVentureEngine(syst, game);
	}
	return game != 0;
}




} // End of namespace MacVenture

#if PLUGIN_ENABLED_DYNAMIC(MACVENTURE)
 REGISTER_PLUGIN_DYNAMIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#else
 REGISTER_PLUGIN_STATIC(MACVENTURE, PLUGIN_TYPE_ENGINE, MacVenture::MacVentureMetaEngine);
#endif
