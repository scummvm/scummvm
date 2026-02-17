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

#include "colony/detection.h"

namespace Colony {

static const PlainGameDescriptor colonyGames[] = {
	{"colony", "The Colony"},
	{0, 0}
};

const ADGameDescription gameDescriptions[] = {
	{
		"colony",
		"",
		AD_ENTRY2s("logo1.pic", "70d44e40ac19ea0413f1253b781399de", 6689,
		           "MAP.1", "ab40dc3d9658e8cdc0bee63c2ca9c79b", 3350),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_RENDEREGA, GUIO_RENDERMACINTOSH)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Colony

class ColonyMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	ColonyMetaEngineDetection() : AdvancedMetaEngineDetection(Colony::gameDescriptions, Colony::colonyGames) {
		_guiOptions = GUIO1(GAMEOPTION_WIDESCREEN);
	}

	const char *getName() const override {
		return "colony";
	}

	const char *getEngineName() const override {
		return "The Colony";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) 1988 David A. Smith";
	}
};

REGISTER_PLUGIN_STATIC(COLONY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ColonyMetaEngineDetection);
