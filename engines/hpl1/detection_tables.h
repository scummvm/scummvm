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

namespace Hpl1 {

const PlainGameDescriptor GAME_NAMES[] = {
	{"penumbraoverture", "Penumbra: Overture"},
	{"penumbrablackplague", "Penumbra: Black Plague"},
	{0, 0}
};

const ADGameDescription GAME_DESCRIPTIONS[] = {

	// Penumbra: Overture (Humble Indie Bundle #1 Release)
	{
		"penumbraoverture",
		nullptr,
		AD_ENTRY1s("Penumbra.exe", "43c33c142b18268c9e98d4c5a6d911b7", 2727936),
		Common::Language::EN_ANY,
		Common::Platform::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},

	// Penumbra: Overture (GOG v1.0)
	{
		"penumbraoverture",
		nullptr,
		AD_ENTRY1s("Penumbra.exe", "f840b972ee889200ba501ee3a465317e", 2744320),
		Common::Language::EN_ANY,
		Common::Platform::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},

	// Penumbra: Overture (The Penumbra Collection)
	// TRAC #14674
	{
		"penumbraoverture",
		nullptr,
		AD_ENTRY1s("Penumbra.exe", "202990aa064ae67a675e2b187384036a", 2736128),
		Common::Language::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},

	// Penumbra: Overture (Steam - Windows)
	{
		"penumbraoverture",
		nullptr,
		AD_ENTRY1s("Penumbra.exe", "384e33ddc55f51debca07b6538087e75", 3104768),
		Common::Language::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},

	// Penumbra: Overture (Steam - Linux)
	{
		"penumbraoverture",
		nullptr,
		AD_ENTRY1s("penumbra.bin", "41950a7597b7d9976f77a73a43e8871d", 6301460),
		Common::Language::EN_ANY,
		Common::kPlatformLinux,
		ADGF_TESTING,
		GUIO0()
	},

	// Penumbra: Black Plague (GOG v1.0)
	{
		"penumbrablackplague",
		nullptr,
		AD_ENTRY1s("Penumbra.exe", "a066f7284e063ac3e5d7409102b7a497", 2985984),
		Common::Language::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},

	// Penumbra: Black Plague (CD)
	{
		"penumbrablackplague",
		nullptr,
		AD_ENTRY1s("Penumbra.exe", "be48f62ecd16975e31083989129eafd3", 8159232),
		Common::Language::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSUPPORTED,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // namespace Hpl1
