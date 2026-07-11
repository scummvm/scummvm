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

namespace Comfy {

const PlainGameDescriptor comfyGames[] = {
	{ "comfyland", "ComfyLand" },
	{ "boo", "Didi and Boo" },
	{ "first", "The First Step" },
	{ "match", "A Purrfect Match" },
	{ "colors", "Colorful Adventures" },
	{ "concert", "The Great Concert" },
	{ "friends", "Playground of Friends" },
	{ "panther", "The Pink Panther's House of Numbers" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"comfyland",
		nullptr,
		AD_ENTRY3s("COMFY1/ENGLISH/COMFY.OBJ",    "7b4ce03e92a27c2271d3657d3c13084b", 221184,
				   "COMFY1/ENGLISH/MIDIFILE.DAT", "fddbb3593b2c2df638fac22626b890cf", 530316,
				   "INTRO/COMFY.OBJ",             "b8fb0c9d325ec4db8ce37ef0d08da130", 4096),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"comfyland",
		"1999",
		AD_ENTRY5s("COMFY1/ENGLISH/COMFY.OBJ",    "8bbeefa3c59c8e80046f35a9202f6095", 221184,
				   "COMFY1/ENGLISH/MIDIFILE.DAT", "fddbb3593b2c2df638fac22626b890cf", 530316,
				   "COMFY1/ENGLISH/ANMFILE.DAT",  "46f4fa67879827e706dc9f4d2eaad3aa", 20,
				   "INTRO/COMFY.OBJ",             "b8fb0c9d325ec4db8ce37ef0d08da130", 4096,
				   "KEYBOARD.DAT",                "08abb613ff84fa5347c7dff9128d83e2", 61),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"boo",
		nullptr,
		AD_ENTRY3s("BOO/ENGLISH/COMFY.OBJ",    "5d847524a1e1cfce9799a31acca777f3", 301056,
				   "BOO/ENGLISH/MIDIFILE.DAT", "1fe05885c32470022de11639de4f532f", 141282,
				   "BOO/ENGLISH/ANMFILE.DAT",  "cbd26fc45b5263ee47e4bffd1bfd00ef", 21219438),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"first",
		nullptr,
		AD_ENTRY2s("FIRST/ENGLISH/COMFY.OBJ",    "c187ac3fd5d547f59ff18b98a6a2f10b", 43008,
				   "FIRST/ENGLISH/MIDIFILE.DAT", "4ab6c35308ed504a8abb96ae3fe81fe4", 1232),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"match",
		nullptr,
		AD_ENTRY2s("MATCH/ENGLISH/COMFY.OBJ",    "daf2c1e6d3bc019f026edbe5465df13b", 337920,
				   "MATCH/ENGLISH/MIDIFILE.DAT", "253d28e78576b2564f60588e2afa1d66", 2640),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"colors",
		nullptr,
		AD_ENTRY3s("COLORS/ENGLISH/COMFY.OBJ",    "6679b5e35db4c4312d4cb85076403760", 268288,
				   "COLORS/ENGLISH/MIDIFILE.DAT", "aac1bd8350dcbc0b6c2e9dd9a7f25ba4", 289982,
				   "COLORS/ENGLISH/ANMFILE.DAT",  "46f4fa67879827e706dc9f4d2eaad3aa", 20),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"concert",
		nullptr,
		AD_ENTRY3s("CONCERT/ENGLISH/COMFY.OBJ",    "c060145864e951fc13ba6106e75f4845", 272384,
				   "CONCERT/ENGLISH/MIDIFILE.DAT", "182ae9a4a3e49d3f9230c274f9828fdf", 598593,
				   "CONCERT/ENGLISH/ANMFILE.DAT",  "46f4fa67879827e706dc9f4d2eaad3aa", 20),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"friends",
		nullptr,
		AD_ENTRY3s("FRIENDS/ENGLISH/COMFY.OBJ",    "d4c620ba375834094b4151ebef862647", 364544,
				   "FRIENDS/ENGLISH/MIDIFILE.DAT", "20992cb6b10be823262301ea36253ab8", 239443,
				   "FRIENDS/ENGLISH/ANMFILE.DAT",  "46f4fa67879827e706dc9f4d2eaad3aa", 20),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	{
		"panther",
		nullptr,
		AD_ENTRY3s("PANTHER/ENGLISH/COMFY.OBJ",    "05420f4122187233112a9dc374303c15", 65536,
				   "PANTHER/ENGLISH/MIDIFILE.DAT", "8cbbbbb2ebfc96e6a69a87f010f393e8", 51028,
				   "PANTHER/ENGLISH/ANMFILE.DAT",  "93ad11d4236610ff946f8a8c320b89a1", 135664335),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Comfy
