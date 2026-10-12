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

namespace BCB {

static const HaroldGameDescription gameDescriptions[] = {
	{
		// Blue Chicago Blues Japanese Windows release.
		{
			"bcb",
			nullptr,
			AD_ENTRY5s(
				"check.bin", "f3b5ba01c47a9fca664293c73c721826", 37094,
				"l_tbl.bin", "dbbba1934727aa42f88754454e05ee09", 204,
				"l_tim.bin", "497a7a5e13a6bf496f6ab63b694746f3", 816,
				"all.bin", "49abcc9b900820dc4c0190d29b07cade", 142072,
				"bcb.exe", "dcb035ac343adc54dfa1ddd5dbd1500a", 92672),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()},
		kHaroldGameBCB
	},

	{
		// Blue Chicago Blues Korean Windows release.
		{
			"bcb",
			nullptr,
			AD_ENTRY5s(
				"check.bin", "f3b5ba01c47a9fca664293c73c721826", 37094,
				"l_tbl.bin", "dbbba1934727aa42f88754454e05ee09", 204,
				"l_tim.bin", "497a7a5e13a6bf496f6ab63b694746f3", 816,
				"all.bin", "49abcc9b900820dc4c0190d29b07cade", 142072,
				"bcb.exe", "6eeccaa692b3012a24097738773722db", 153600),
			Common::KO_KOR,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()},
		kHaroldGameBCB
	},

	{AD_TABLE_END_MARKER, kHaroldGameBCB}};

} // End of namespace BCB
