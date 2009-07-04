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
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"

#include "igor/igor.h"

struct IgorGameDescription {
	ADGameDescription desc;
	int gameVersion;
	int gameFlags;
};

using Common::GUIO_NONE;

static const IgorGameDescription igorGameDescriptions[] = {
	{
		{
			"igor",
			"Demo 1.00s",
			{
				{ "IGOR.DAT", 0, 0, 4086790 },
				{ "IGOR.FSD", 0, 0,  462564 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO_NONE
		},
		Igor::kIdEngDemo100,
		Igor::kFlagDemo | Igor::kFlagFloppy
	},
	{
		{
			"igor",
			"Demo 1.10s",
			{
				{ "IGOR.DAT", 0, 0, 4094103 },
				{ "IGOR.FSD", 0, 0,  462564 },
				{ 0, 0, 0, 0 }
			},
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO_NONE
		},
		Igor::kIdEngDemo110,
		Igor::kFlagDemo | Igor::kFlagFloppy
	},
	{
		{
			"igor",
			"Talkie",
			{
				{ "IGOR.EXE", 0, 0,  9115648 },
				{ "IGOR.DAT", 0, 0, 61682719 },
				{ 0, 0, 0, 0 }
			},
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		Igor::kIdSpaCD,
		Igor::kFlagTalkie
	},
	{ AD_TABLE_END_MARKER, 0, 0 }
};

static const PlainGameDescriptor igorGameDescriptors[] = {
	{ "igor", "Igor: Objective Uikokahonia" },
	{ 0, 0 }
};

static const ADParams igorDetectionParams = {
	(const byte *)igorGameDescriptions,
	sizeof(IgorGameDescription),
	1024, // number of md5 bytes
	igorGameDescriptors,
	0,
	"igor",
	0,
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NONE
};

class IgorMetaEngine : public AdvancedMetaEngine {
public:
	IgorMetaEngine() : AdvancedMetaEngine(igorDetectionParams) {}

	virtual const char *getName() const {
		return "Igor: Objective Uikokahonia";
	}

	virtual const char *getOriginalCopyright() const {
		return "Igor: Objective Uikokahonia (C) Pendulo Studios";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool IgorMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const IgorGameDescription *gd = (const IgorGameDescription *)desc;
	if (gd) {
		Igor::DetectedGameVersion dgv;
		dgv.version = gd->gameVersion;
		dgv.flags = gd->gameFlags;
		dgv.language = gd->desc.language;
		dgv.ovlFileName = gd->desc.filesDescriptions[0].fileName;
		dgv.sfxFileName = gd->desc.filesDescriptions[1].fileName;
		*engine = new Igor::IgorEngine(syst, &dgv);
	}
	return gd != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(IGOR)
	REGISTER_PLUGIN_DYNAMIC(IGOR, PLUGIN_TYPE_ENGINE, IgorMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(IGOR, PLUGIN_TYPE_ENGINE, IgorMetaEngine);
#endif
