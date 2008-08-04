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
 *
 */

#include "base/plugins.h"

#include "common/advancedDetector.h"
#include "common/file.h"

#include "toltecs/toltecs.h"


namespace Toltecs {

struct ToltecsGameDescription {
	Common::ADGameDescription desc;
};

uint32 ToltecsEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ToltecsEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor toltecsGames[] = {
	{"toltecs", "3 Skulls of the Toltecs"},
	{0, 0}
};


namespace Toltecs {

static const ToltecsGameDescription gameDescriptions[] = {

	{
		// Toltecs English version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "05472037e9cfde146e953c434e74f0f4", 337643527),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Toltecs

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Toltecs::gameDescriptions,
	// Size of that superset structure
	sizeof(Toltecs::ToltecsGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	toltecsGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"toltecs",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0
};

class ToltecsMetaEngine : public Common::AdvancedMetaEngine {
public:
	ToltecsMetaEngine() : Common::AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Toltecs Engine";
	}

	virtual const char *getCopyright() const {
		return "Toltecs Engine (C) 1996";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const;
};

bool ToltecsMetaEngine::createInstance(OSystem *syst, Engine **engine, const Common::ADGameDescription *desc) const {
	const Toltecs::ToltecsGameDescription *gd = (const Toltecs::ToltecsGameDescription *)desc;
	if (gd) {
		*engine = new Toltecs::ToltecsEngine(syst, gd);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(TOLTECS)
	REGISTER_PLUGIN_DYNAMIC(TOLTECS, PLUGIN_TYPE_ENGINE, ToltecsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOLTECS, PLUGIN_TYPE_ENGINE, ToltecsMetaEngine);
#endif
