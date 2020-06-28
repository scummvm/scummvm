/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/advancedDetector.h"
#include "engines/icb/icb.h"

#include "common/savefile.h"
#include "common/system.h"

namespace ICB {

static const PlainGameDescriptor icbGames[] = {
	{ "icb", "In Cold Blood" },
	{ 0, 0 }
};

static const char *directoryGlobs[] = {
	"engine",
	"linc",
	0
};

static const ADGameDescription gameDescriptions[] = {
	{
		// In Cold Blood
		// English GOG Version
		"icb",
		"GOG",
		AD_ENTRY1s("engine.exe", "0c4a7a5046ec13ccac89ab3f959cc217", 837632),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

/*	{
		// In Cold Blood
		// English Demo
		"icb",
		"Demo",
		AD_ENTRY1s("engine.exe", "0c4a7a5046ec13ccac89ab3f959cc217", 837632),  // TODO: Fill with correct values
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},*/

	AD_TABLE_END_MARKER
};

class IcbMetaEngine : public AdvancedMetaEngine {
public:
	IcbMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), icbGames) {
		_guiOptions = GUIO_NOMIDI;
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const { return "In Cold Blood Engine"; }

	const char *getEngineId() const override {
		return "icb";
	}

	virtual const char *getOriginalCopyright() const { return "(C) 2000 Revolution Software Ltd"; }

	virtual bool hasFeature(MetaEngineFeature f) const { return false; }

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool IcbMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new IcbEngine(syst, desc);

	return desc != nullptr;
}

} // End of namespace ICB

#if PLUGIN_ENABLED_DYNAMIC(ICB)
REGISTER_PLUGIN_DYNAMIC(ICB, PLUGIN_TYPE_ENGINE, ICB::IcbMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ICB, PLUGIN_TYPE_ENGINE, ICB::IcbMetaEngine);
#endif
