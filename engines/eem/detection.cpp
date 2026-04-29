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

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "eem/detection.h"

namespace EEM {

const PlainGameDescriptor eemGames[] = {
	{ "eem", "Eagle Eye Mysteries" },
	{ nullptr, nullptr }
};

const ADGameDescription gameDescriptions[] = {
	{
		"eem",
		"CD",
		AD_ENTRY2s("EEMCD.EXE", "286d586113863ceb0e12df5b36fd8504", 272608,
				   "PICS.DBD",  "cc05ec256cd5a04df9019e9aebfb7c32", 994057),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},
	{
		"eem",
		"Floppy",
		AD_ENTRY2s("EEM.EXE",   "692a5e6e7f4516d6e40c1f80cbc1b2cc", 109542,
				   "PICS.DBD",  "26b97e8586f798ea90440e88d3d527cd", 959160),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},
	{
		// Spanish floppy release — same EEM.EXE binary as the English
		// floppy (the engine code is identical), with a localised
		// PICS.DBD that swaps the embedded English image text for
		// Spanish equivalents.
		"eem",
		"Floppy",
		AD_ENTRY2s("EEM.EXE",   "692a5e6e7f4516d6e40c1f80cbc1b2cc", 109542,
				   "PICS.DBD",  "199150e7d612f87477814bc5f4a1967a", 955332),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

const DebugChannelDef debugFlagList[] = {
	{ kDebugGeneral, "general", "General debug" },
	{ kDebugScript,  "script",  "Script execution" },
	{ kDebugMystery, "mystery", "Mystery loading and state" },
	{ kDebugSite,    "site",    "Site rendering and hot-spots" },
	{ kDebugGfx,     "gfx",     "Graphics, palette, animations" },
	{ kDebugSound,   "sound",   "Sound and music" },
	DEBUG_CHANNEL_END
};

} // End of namespace EEM

class EEMMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	EEMMetaEngineDetection() : AdvancedMetaEngineDetection(EEM::gameDescriptions, EEM::eemGames) {
	}

	const char *getName() const override {
		return "eem";
	}

	const char *getEngineName() const override {
		return "Eagle Eye Mysteries";
	}

	const char *getOriginalCopyright() const override {
		return "Eagle Eye Mysteries (C) 1994 EA Kids";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return EEM::debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(EEM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, EEMMetaEngineDetection);
