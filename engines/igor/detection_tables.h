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

namespace Igor {

const PlainGameDescriptor igorGames[] = {
	{ "igor", "Igor: Objective Uikokahonia" },
	{ 0, 0 }
};

const IgorGameDescription gameDescriptions[] = {
	{
		{
			"igor",
			"Demo 1.00s",
			AD_ENTRY2s(
				"IGOR.DAT", nullptr, 4086790,
				"IGOR.FSD", nullptr, 462564
			),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		kIdEngDemo100,
		kFlagDemo | kFlagFloppy,
		"IGOR.DAT",
		"IGOR.FSD"
	},
	{
		{
			"igor",
			"Demo 1.10s",
			AD_ENTRY2s(
				"IGOR.DAT", nullptr, 4094103,
				"IGOR.FSD", nullptr, 462564
			),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		kIdEngDemo110,
		kFlagDemo | kFlagFloppy,
		"IGOR.DAT",
		"IGOR.FSD"
	},
	{
		{
			"igor",
			"CD",
			AD_ENTRY2s(
				"IGOR.DAT", "b4ba3ec480ec66a2fa456265ab51f5f6", 61682719,
				"IGOR.EXE", "70f49859d1defddda140d144e1eb90b1", 9115648
			),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_CD,
			GUIO1(GUIO_NONE)
		},
		kIdSpaCD,
		kFlagTalkie,
		"IGOR.EXE",
		"IGOR.DAT"
	},
	{
		{
			"igor",
			"Floppy",
			AD_ENTRY2s(
				"IGOR.DAT", "69eec27e1d2fe2f0612a6fb265385dfb", 11199335,
				"IGOR.EXE", "91d60ccc00ea19c1bd1b50657a9c598b", 39537
			),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOSPEECH)
		},
		kIdSpaFloppy,
		kFlagFloppy,
		"IGOR.DAT",
		"IGOR.FSD"
	},

	{
		{
			"igor",
			"Floppy",
			AD_ENTRY2s(
				"IGOR.DAT", "dbbac4257430da5d616ae946a37750ef", 8819339,
				"IGOR.FSD", "d081e286da14913379f0d7f58a6a5686", 1324850
			),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOSPEECH)
		},
		kIdEngFloppy,
		kFlagFloppy,
		"IGOR.DAT",
		"IGOR.FSD"
	},

	{ AD_TABLE_END_MARKER, 0, 0, nullptr, nullptr }
};

} // End of namespace Igor
