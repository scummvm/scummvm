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

 /*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "engines/advancedDetector.h"
#include "common/system.h"

#include "base/plugins.h"

#include "lab/lab.h"

static const PlainGameDescriptor lab_setting[] = {
	{ "lab", "Labyrith of Time" },
	{ 0, 0 }
};

enum GameFeatures {
        GF_LOWRES = 1 << 0
};

static const ADGameDescription labDescriptions[] = {
	{
		"lab",
		"",
		{
			{ "doors",       0, "d77536010e7e5ae17ee066323ceb9585", 2537 },
			{ "notes11",     0, "63e873f659f8f46f9809d16a2bf653c7", 3562 },
			{ "noteold.fon", 0, "6c1d90ad55149556e79d3f7bfddb4bd7", 9252 },
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"lab",
		"Lowres",
		{
			{ "doors",   0, "d77536010e7e5ae17ee066323ceb9585", 2537 },
			{ "notes11", 0, "63e873f659f8f46f9809d16a2bf653c7", 3562 },
			{ "64b",     0, "3a84d41bcc6a782f22e8e954bce09721", 39916 },
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformDOS,
		GF_LOWRES,
		GUIO0()
	},
	{
		"lab",
		"Rerelease",
		{
			{ "doors",   0, "d77536010e7e5ae17ee066323ceb9585", 2537 },
			{ "notes11", 0, "63e873f659f8f46f9809d16a2bf653c7", 3562 },
			{ "inv",     0, "0a5377f0673454b1743322471892ad43", 39120 },
			{ "48",      0, "fb2d990c5fb5dd36a618e5ca8136b740", 892 },
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"lab",
		"Trial",
		{
			{ "doors",   0, "d77536010e7e5ae17ee066323ceb9585", 2537 },
			{ "notes11", 0, "63e873f659f8f46f9809d16a2bf653c7", 3562 },
			{ "inv",     0, "0a5377f0673454b1743322471892ad43", 39120 },
			{ "48",      0, "a319ba9bab20b24200257e826b5494e1", 892 },
			{ NULL, 0, NULL, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO0()
	},
	{
		"lab",
		"",
		AD_ENTRY1s("doors", "7bf458df6ec30cc8ef4665e4d7c77f59", 2537),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const char *const directoryGlobs[] = {
        "fonts",
		"game",
		"pict",
		"spict",
		"rooms",
		"h2",
        0
};

class LabMetaEngine : public AdvancedMetaEngine {
public:
	LabMetaEngine() : AdvancedMetaEngine(labDescriptions, sizeof(ADGameDescription), lab_setting) {
		_singleid = "lab";

		_maxScanDepth = 4;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Lab";
	}

	virtual const char *getOriginalCopyright() const {
		return "Labytinth of Time (c) 2004 The Wyrmkeep Entertainment Co.";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription * /* desc */) const {
		// Instantiate Engine even if the game data is not found.
		*engine = new Lab::LabEngine(syst);
		return true;
	}

};

#if PLUGIN_ENABLED_DYNAMIC(LAB)
	REGISTER_PLUGIN_DYNAMIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LAB, PLUGIN_TYPE_ENGINE, LabMetaEngine);
#endif
