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

namespace Quux {
static const PlainGameDescriptor quuxGames[] = {
	{ "quux", "Quux the Example Module" },
	{ "quuxcd", "Quux the Example Module (CD version)" },
	{ 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
	{
		"quux",
		0,
		AD_ENTRY1s("quux.txt", 0, 0),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};
} // End of namespace Quux

class QuuxMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	QuuxMetaEngineDetection() : AdvancedMetaEngineDetection(Quux::gameDescriptions, sizeof(ADGameDescription), Quux::quuxGames) {
	}

	const char *getEngineId() const override {
		return "quux";
	}

	const char *getName() const override {
		return "Quux";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) Quux Entertainment Ltd.";
	}
};

REGISTER_PLUGIN_STATIC(QUUX_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, QuuxMetaEngineDetection);
