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
 * Copyright 2020 Google
 *
 */
#include "common/system.h"
#include "common/savefile.h"

#include "engines/advancedDetector.h"

#include "hadesch/hadesch.h"

#include "detection_tables.h"

namespace Hadesch {
static const PlainGameDescriptor hadeschGames[] = {
	{"hadesch", "Hades Challenge"},
	{0, 0}
};

// The list is pretty long but it's because we need just a few files but
// in pretty deep paths:
// * Setup.exe [Russian-Windows]
// * WIN9x/WORLD/wd.pod [English-Windows]
// * CDAssets/OLYMPUS/ol.pod [English-Windows]
// * Scenes/OLYMPUS/ol.pod [English-Mac and Russian-Windows]
// * Hades - Copy To Hard Drive/Hades Challenge/World/wd.pod [English-Mac]
// * Hades - Copy To Hard Drive/Hades Challenge/World/wd.pod [English-Mac]
// The difference between 2 last one is how the files were copied
static const char *const directoryGlobs[] = {
	"WIN9x",
	"WORLD",
	"CDAssets",
	"OLYMPUS",
	"Scenes",
	"Hades_-_Copy_To_Hard_Drive",
	"Hades - Copy To Hard Drive",
	"Hades Challenge",
	"Hades_Challenge",
	0
};
}

class HadeschMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HadeschMetaEngineDetection() : AdvancedMetaEngineDetection(Hadesch::gameDescriptions, sizeof(ADGameDescription), Hadesch::hadeschGames) {
		// mac puts wd.pod in Hades - Copy To Hard Drive/Hades Challenge/World. So we need 4 levels
		_maxScanDepth = 4;
		_directoryGlobs = Hadesch::directoryGlobs;
	}

	const char *getEngineId() const override {
		return "hadesch";
	}

	const char *getName() const override {
		return "Hades Challenge";
	}

	const char *getOriginalCopyright() const override {
		return "Hades Challenge (C) Disney's Interactive";
	}
};

REGISTER_PLUGIN_STATIC(HADESCH_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HadeschMetaEngineDetection);
