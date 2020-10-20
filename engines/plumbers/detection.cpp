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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "engines/game.h"

static const PlainGameDescriptor plumbersGames[] = {
	{"plumbers", "Plumbers Don't Wear Ties!"},
	{0, 0}
};

namespace Plumbers {

static const ADGameDescription gameDescriptions[] = {
	// Plumbers PC version
	{
		"plumbers",
		0,
		AD_ENTRY1s("GAME.BIN", 0, 41622),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},

	// Plumbers 3DO version
	{
		"plumbers",
		0,
		AD_ENTRY1s("launchme", 0, 143300),
		Common::EN_ANY,
		Common::kPlatform3DO,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Plumbers

class PlumbersMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PlumbersMetaEngineDetection() : AdvancedMetaEngineDetection(Plumbers::gameDescriptions, sizeof(ADGameDescription), plumbersGames) {
	}

	const char *getEngineId() const override {
		return "plumbers";
	}

	const char *getName() const override {
		return "Plumbers Don't Wear Ties";
	}

	const char *getOriginalCopyright() const override {
		return "Plumbers Don't Wear Ties (C) 1993-94 Kirin Entertainment";
	}
};

REGISTER_PLUGIN_STATIC(PLUMBERS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PlumbersMetaEngineDetection);
