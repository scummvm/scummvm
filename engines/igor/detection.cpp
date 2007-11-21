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

#include "common/advancedDetector.h"
#include "common/config-manager.h"

#include "igor/igor.h"

struct IgorGameDescription {
	Common::ADGameDescription desc;
	int gameVersion;
};

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
			Common::ADGF_DEMO
		},
		Igor::kIdEngDemo100
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
			Common::ADGF_DEMO
		},
		Igor::kIdEngDemo110
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
			Common::ADGF_NO_FLAGS
		},
		Igor::kIdSpaCD
	},
	{ AD_TABLE_END_MARKER, 0 }
};

static const PlainGameDescriptor igorGameDescriptors[] = {
	{ "igor", "Igor: Objective Uikokahonia" },
	{ 0, 0 }
};

static const Common::ADParams igorDetectionParams = {
	(const byte *)igorGameDescriptions,
	sizeof(IgorGameDescription),
	1024, // number of md5 bytes
	igorGameDescriptors,
	0,
	"igor",
	0,
	0,
	Common::kADFlagAugmentPreferredTarget
};

static bool Engine_IGOR_createInstance(OSystem *syst, Engine **engine, Common::EncapsulatedADGameDesc encapsulatedDesc) {
	const IgorGameDescription *gd = (const IgorGameDescription *)(encapsulatedDesc.realDesc);
	if (gd) {
		*engine = new Igor::IgorEngine(syst, gd->gameVersion);
	}
	return gd != 0;
}

ADVANCED_DETECTOR_DEFINE_PLUGIN(IGOR, Engine_IGOR_createInstance, igorDetectionParams);

REGISTER_PLUGIN(IGOR, "Igor: Objective Uikokahonia", "Igor: Objective Uikokahonia (C) Pendulo Studios");
