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

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "tsage/detection.h"
#include "tsage/tsage.h"

static const PlainGameDescriptor tSageGameTitles[] = {
	{ "ringworld", "Ringworld: Revenge of the Patriarch" },
	{ "blueforce", "Blue Force" },
	{ "ringworld2", "Return to Ringworld" },
	{ "sherlock-logo", "The Lost Files of Sherlock Holmes (Logo)" },
	{ 0, 0 }
};

static const DebugChannelDef debugFlagList[] = {
	{TsAGE::kRingDebugScripts, "scripts", "Scripts debugging"},
	DEBUG_CHANNEL_END
};

#include "tsage/detection_tables.h"

class TSageMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TSageMetaEngineDetection() : AdvancedMetaEngineDetection(TsAGE::gameDescriptions, sizeof(TsAGE::tSageGameDescription), tSageGameTitles) {
	}

	const char *getName() const override {
		return "tsage";
	}

	const char *getEngineName() const override {
		return "TsAGE";
	}

	const char *getOriginalCopyright() const override {
		return "(C) Tsunami Media";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(TSAGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TSageMetaEngineDetection);
