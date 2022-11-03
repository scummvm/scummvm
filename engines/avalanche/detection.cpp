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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "avalanche/detection.h"

namespace Avalanche {

static const PlainGameDescriptor avalancheGames[] = {
	{"avalanche", "Lord Avalot d'Argent"},
	{nullptr, nullptr}
};

static const ADGameDescription gameDescriptions[] = {
	{
		"avalanche", nullptr,
		AD_ENTRY2s("avalot.sez",	"de10eb353228013da3d3297784f81ff9", 48763,
				   "mainmenu.avd",	"89f31211af579a872045b175cc264298", 18880),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

class AvalancheMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AvalancheMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(AvalancheGameDescription), avalancheGames) {
	}

	const char *getName() const override {
		return "avalanche";
	}

	const char *getEngineName() const override {
		return "Avalanche";
	}

	const char *getOriginalCopyright() const override {
		return "Avalanche (C) 1994-1995 Mike, Mark and Thomas Thurman.";
	}
};

} // End of namespace Avalanche

REGISTER_PLUGIN_STATIC(AVALANCHE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Avalanche::AvalancheMetaEngineDetection);
