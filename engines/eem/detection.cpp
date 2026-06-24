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
	{ "eem2", "Eagle Eye Mysteries in London" },
	{ nullptr, nullptr }
};

#define GUI_OPTIONS_EEM_FLOPPY GUIO4(GAMEOPTION_HIDE_HIGHLIGHT_BOXES, GAMEOPTION_SKIP_REPEATED_CASES, GUIO_MIDIADLIB, GUIO_MIDIMT32)
#define GUI_OPTIONS_EEM_CD     GUIO6(GAMEOPTION_HIDE_HIGHLIGHT_BOXES, GAMEOPTION_FIT_DIALOG_BALLOONS, GAMEOPTION_SKIP_REPEATED_CASES, GAMEOPTION_RESTORED_CONTENT, GUIO_MIDIADLIB, GUIO_MIDIMT32)
#define GUI_OPTIONS_EEM_DEMO   GUIO2(GAMEOPTION_HIDE_HIGHLIGHT_BOXES, GUIO_NOMIDI)
#define GUI_OPTIONS_EEM_MAC    GUIO3(GAMEOPTION_HIDE_HIGHLIGHT_BOXES, GAMEOPTION_FIT_DIALOG_BALLOONS, GUIO_NOMIDI)

const ADGameDescription gameDescriptions[] = {
	{
		"eem",
		"CD",
		AD_ENTRY2s("EEMCD.EXE", "286d586113863ceb0e12df5b36fd8504", 272608,
				   "PICS.DBD",  "cc05ec256cd5a04df9019e9aebfb7c32", 994057),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUI_OPTIONS_EEM_CD
	},
	{
		"eem",
		"Floppy",
		AD_ENTRY2s("EEM.EXE",   "692a5e6e7f4516d6e40c1f80cbc1b2cc", 109542,
				   "PICS.DBD",  "26b97e8586f798ea90440e88d3d527cd", 959160),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUI_OPTIONS_EEM_FLOPPY
	},
	{
		// Spanish floppy: same EEM.EXE as English floppy, localised PICS.DBD.
		"eem",
		"Floppy",
		AD_ENTRY2s("EEM.EXE",   "692a5e6e7f4516d6e40c1f80cbc1b2cc", 109542,
				   "PICS.DBD",  "199150e7d612f87477814bc5f4a1967a", 955332),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUI_OPTIONS_EEM_FLOPPY
	},
	{
		"eem",
		"Demo",
		AD_ENTRY2s("EEMDEMO.EXE", "723eb5e1744f5b88562068d732990c7b", 107300,
				   "PICS.DBD",   "fc6be43a0391a79263535760a8bfaecd", 322194),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUI_OPTIONS_EEM_DEMO
	},
	{
		"eem",
		"",
		AD_ENTRY2s("MysteryData", "d94c087c27e68cc299d7c5e737e458f9", 941029,
				   "PICS.DBD",    "8905041070ff1352666d98cd78d5501c", 3800445),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUI_OPTIONS_EEM_MAC
	},
	{
		// Macintosh release played straight from its installer files
		// ("Eagle Eye Installer" + "EEM Install Data 2".."6"); decompressed on
		// the fly (see installer.cpp). "d:" hashes the data fork so this matches
		// both the raw-fork (CD/floppy) and MacBinary-wrapped captures.
		"eem",
		"",
		AD_ENTRY2s("Eagle Eye Installer", "d:08440dbf0cb47fb57e522f050159ffaa", 1391577,
				   "EEM Install Data 2",  "aebccc677e149b37285f291f6ac72f57", 1446428),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUI_OPTIONS_EEM_MAC
	},
	{
		// Eagle Eye Mysteries in London 
		"eem2",
		"CD",
		AD_ENTRY2s("EEM2CD.EXE", "211a376b23a1b6259d0c36cf46d26ed4", 172560,
				   "PICS.DBD",   "da0b13a117bc3a207aec907c05769cd8", 2972988),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO3(GAMEOPTION_FIT_DIALOG_BALLOONS, GUIO_MIDIADLIB, GUIO_MIDIMT32)
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
