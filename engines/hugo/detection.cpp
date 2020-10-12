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

#include "hugo/detection.h"

namespace Hugo {

static const PlainGameDescriptor hugoGames[] = {
	// Games
	{"hugo1", "Hugo 1: Hugo's House of Horrors"},
	{"hugo2", "Hugo 2: Whodunit?"},
	{"hugo3", "Hugo 3: Jungle of Doom"},
	{0, 0}
};

static const HugoGameDescription gameDescriptions[] = {

	// Hugo1 DOS
	{
		{
			"hugo1", 0,
			AD_ENTRY1s("house.art", "c9403b2fe539185c9fd569b6cc4ff5ca", 14811),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeHugo1
	},
	// Hugo1 Windows
	{
		{
			"hugo1", 0,
			AD_ENTRY1s("objects.dat", "3ba0f108f7690a05a34c56a02fbe644a", 126488),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo1
	},
	// Hugo2 DOS
	{
		{
			"hugo2", 0,
			AD_ENTRY1s("objects.dat", "88a718cc0ff2b3b25d49aaaa69d6d52c", 155240),
			Common::EN_ANY,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo2
	},
	// Hugo2 Windows
	{
		{
			"hugo2", 0,
			AD_ENTRY1s("objects.dat", "5df4ffc851e66a544c0e95e4e084a806", 158480),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo2
	},
	// Hugo3 DOS
	{
		{
			"hugo3", 0,
			AD_ENTRY1s("objects.dat", "bb1b061538a445f2eb99b682c0f506cc", 136419),
			Common::EN_ANY,
			Common::kPlatformDOS,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo3
	},
	// Hugo3 Windows
	{
		{
			"hugo3", 0,
			AD_ENTRY1s("objects.dat", "c9a8af7aa14cc907434eecee3ddd06d3", 136638),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo3
	},

	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class HugoMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HugoMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(HugoGameDescription), hugoGames) {
	}

	const char *getEngineId() const override {
		return "hugo";
	}

	const char *getName() const override {
		return "Hugo";
	}

	const char *getOriginalCopyright() const override {
		return "Hugo Engine (C) 1989-1997 David P. Gray";
	}
};

} // End of namespace Hugo

REGISTER_PLUGIN_STATIC(HUGO_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Hugo::HugoMetaEngineDetection);
