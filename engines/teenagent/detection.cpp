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

#include "common/algorithm.h"

#include "base/plugins.h"

#include "engines/advancedDetector.h"

static const PlainGameDescriptor teenAgentGames[] = {
	{ "teenagent", "Teen Agent" },
	{ 0, 0 }
};

static const ADGameDescription teenAgentGameDescriptions[] = {
	{
		"teenagent",
		"",
		{
			{"off.res", 0, NULL, -1},
			{"on.res", 0, NULL, -1},
			{"ons.res", 0, NULL, -1},
			{"varia.res", 0, NULL, -1},
			{"lan_000.res", 0, NULL, -1},
			{"lan_500.res", 0, NULL, -1},
			{"mmm.res", 0, NULL, -1},
			{"sam_mmm.res", 0, NULL, -1},
			{"sam_sam.res", 0, NULL, -1},
			//{"unlogic.res", 0, NULL, -1}, //skipped if not present
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)
	},
	{
		"teenagent",
		"",
		{
			{"off.res", 0, NULL, -1},
			{"on.res", 0, NULL, -1},
			{"ons.res", 0, NULL, -1},
			{"varia.res", 0, NULL, -1},
			{"lan_000.res", 0, NULL, -1},
			{"lan_500.res", 0, NULL, -1},
			{"sam_sam.res", 0, NULL, -1},
			{"voices.res", 0, NULL, -1},
			{"cdlogo.res", 0, NULL, -1},
			AD_LISTEND
		},
		Common::CZ_CZE,
		Common::kPlatformDOS,
		ADGF_CD,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER,
};



class TeenAgentMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TeenAgentMetaEngineDetection() : AdvancedMetaEngineDetection(teenAgentGameDescriptions, sizeof(ADGameDescription), teenAgentGames) {
	}

	const char *getEngineId() const override {
		return "teenagent";
	}

	const char *getName() const override {
		return "TeenAgent";
	}

	const char *getOriginalCopyright() const override {
		return "TEENAGENT (C) 1994 Metropolis";
	}
};

REGISTER_PLUGIN_STATIC(TEENAGENT_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TeenAgentMetaEngineDetection);
