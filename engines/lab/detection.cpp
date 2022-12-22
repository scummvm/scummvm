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
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "engines/advancedDetector.h"
#include "lab/detection.h"

static const PlainGameDescriptor lab_setting[] = {
	{ "lab", "The Labyrinth of Time" },
	{ nullptr, nullptr }
};

static const ADGameDescription labDescriptions[] = {
	{
		"lab",
		"",
		AD_ENTRY2s("doors",		 "d77536010e7e5ae17ee066323ceb9585", 2537,		// game/doors
				   "noteold.fon", "6c1d90ad55149556e79d3f7bfddb4bd7", 9252),	// game/spict/noteold.fon
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"lab",
		"Lowres",
		AD_ENTRY2s("doors",		"d77536010e7e5ae17ee066323ceb9585", 2537,		// game/doors
				   "64b",		"3a84d41bcc6a782f22e8e954bce09721", 39916),		// game/pict/h2/64b
		Common::EN_ANY,
		Common::kPlatformDOS,
		Lab::GF_LOWRES | ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"lab",
		"Rerelease",
		{
			{ "doors",   0, "d77536010e7e5ae17ee066323ceb9585", 2537 }, // game/doors
			{ "noteold.fon", 0, "6c1d90ad55149556e79d3f7bfddb4bd7", 9252 }, // game/spict/noteold.fon
			{ "wyrmkeep",0, "97c7064c54c28b952d37c4ebff6efa50", 52286 }, // game/spict/intro
			{ nullptr, 0, nullptr, 0 }
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"lab",
		"",
		AD_ENTRY1s("doors", "7bf458df6ec30cc8ef4665e4d7c77f59", 2537), // game/doors
		Common::EN_ANY,
		Common::kPlatformAmiga,
		Lab::GF_LOWRES | ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};

static const char *const directoryGlobs[] = {
		"game",
		"pict",
		"spict",
		"rooms",
		"h2",
		"intro",
		nullptr
};



class LabMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	LabMetaEngineDetection() : AdvancedMetaEngineDetection(labDescriptions, sizeof(ADGameDescription), lab_setting) {
		_maxScanDepth = 4;
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagUseExtraAsHint;
	}

	const char *getName() const override {
		return "lab";
	}

	const char *getEngineName() const override {
		return "Labyrinth of Time";
	}

	const char *getOriginalCopyright() const override {
		return "The Labyrinth of Time (C) 2004 The Wyrmkeep Entertainment Co. and Terra Nova Development";
	}
};

REGISTER_PLUGIN_STATIC(LAB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, LabMetaEngineDetection);
