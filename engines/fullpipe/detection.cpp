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
#include "common/file.h"

#include "fullpipe/fullpipe.h"


namespace Fullpipe {

const char *FullpipeEngine::getGameId() const {
	return _gameDescription->gameid;
}

}

static const PlainGameDescriptor fullpipeGames[] = {
	{"fullpipe", "Full Pipe"},
	{0, 0}
};

namespace Fullpipe {

static const ADGameDescription gameDescriptions[] = {

	// Fullpipe Russian version
	{
		"fullpipe",
		0,
		AD_ENTRY1s("0654.sc2", "099f54f86d33ad2395f3b854b7e05058", 2272),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Fullpipe German version
	{
		"fullpipe",
		0,
		AD_ENTRY1s("0654.sc2", "d8743351fc53d205f42d91f6d791e51b", 2272),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Fullpipe

class FullpipeMetaEngine : public AdvancedMetaEngine {
public:
	FullpipeMetaEngine() : AdvancedMetaEngine(Fullpipe::gameDescriptions, sizeof(ADGameDescription), fullpipeGames) {
		_singleid = "fullpipe";
	}

	virtual const char *getName() const {
		return "Fullpipe Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Fullpipe Engine (C) Pipe Studio";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool FullpipeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Fullpipe::FullpipeEngine::hasFeature(EngineFeature f) const {
	return false;
}

bool FullpipeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Fullpipe::FullpipeEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(FULLPIPE)
	REGISTER_PLUGIN_DYNAMIC(FULLPIPE, PLUGIN_TYPE_ENGINE, FullpipeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(FULLPIPE, PLUGIN_TYPE_ENGINE, FullpipeMetaEngine);
#endif
