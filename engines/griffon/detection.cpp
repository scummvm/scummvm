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

static const PlainGameDescriptor griffonGames[] = {
	{"griffon", "The Griffon Legend"},
	{NULL, NULL}
};

namespace Griffon {

static const ADGameDescription gameDescriptions[] = {
	{
		"griffon",
		NULL,
		AD_ENTRY1s("objectdb.dat", "ec5371da28f01ccf88980b32d9de2232", 27754),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

}

class GriffonMetaEngineDetection: public AdvancedMetaEngineDetection {
public:
	GriffonMetaEngineDetection() : AdvancedMetaEngineDetection(Griffon::gameDescriptions, sizeof(ADGameDescription), griffonGames) {
	}

	const char *getEngineId() const override {
		return "griffon";
	}

	const char *getName() const override {
		return "Griffon Engine";
	}

	const char *getOriginalCopyright() const override {
		return "The Griffon Legend (c) 2005 Syn9 (Daniel Kennedy)";
	}
};

REGISTER_PLUGIN_STATIC(GRIFFON_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, GriffonMetaEngineDetection);
