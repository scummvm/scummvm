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
#include "common/config-manager.h"
#include "common/file.h"

#include "pegasus/pegasus.h"

namespace Pegasus {

struct PegasusGameDescription {
	ADGameDescription desc;
};

bool PegasusEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool PegasusEngine::isDemo() const {
	return (_gameDescription->desc.flags & ADGF_DEMO) != 0;
}

} // End of namespace Pegasus

static const PlainGameDescriptor pegasusGames[] = {
	{"pegasus", "The Journeyman Project: Pegasus Prime"},
	{0, 0}
};


namespace Pegasus {

static const PegasusGameDescription gameDescriptions[] = {
	{
		{
			"pegasus",
			"",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 2009943),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			Common::GUIO_NONE
		},
	},

	{
		{
			"pegasus",
			"",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 360129),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK|ADGF_DEMO,
			Common::GUIO_NONE
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Pegasus


class PegasusMetaEngine : public AdvancedMetaEngine {
public:
	PegasusMetaEngine() : AdvancedMetaEngine(Pegasus::gameDescriptions, sizeof(Pegasus::PegasusGameDescription), pegasusGames) {
		params.singleid = "pegasus";
	}

	virtual const char *getName() const {
		return "The Journeyman Project: Pegasus Prime";
	}

	virtual const char *getOriginalCopyright() const {
		return "The Journeyman Project: Pegasus Prime (C) Presto Studios";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool PegasusMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Pegasus::PegasusGameDescription *gd = (const Pegasus::PegasusGameDescription *)desc;

	if (gd)
		*engine = new Pegasus::PegasusEngine(syst, gd);

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(PEGASUS)
	REGISTER_PLUGIN_DYNAMIC(PEGASUS, PLUGIN_TYPE_ENGINE, PegasusMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PEGASUS, PLUGIN_TYPE_ENGINE, PegasusMetaEngine);
#endif

