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

#include "advancedDetector.h"
namespace PhoenixVR {

const PlainGameDescriptor phoenixvrGames[] = {
	{"necrono", "Necronomicon: The Dawning of Darkness"},
	{"lochness", "The Cameron Files: The Secret at Loch Ness"},
	{"messenger", "The Messenger/Louvre: The Final Curse"},
	{0, 0}};

const ADGameDescription gameDescriptions[] = {
	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "f795f35b079cb8ef599724a2a7336c7e", 5319),
	 Common::EN_USA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "542b626e7d56e4b3b5a73616e772a246", 5503),
	 Common::FR_FRA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "8a6fde4fbe90262cad639903a0469863", 5394),
	 Common::IT_ITA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "c3af2d55b4fa55d200b047b6ec0d9f73", 5507),
	 Common::DE_DEU,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "e54b9e37d3b96b52f78e7ca266116c4b", 5423),
	 Common::ES_ESP,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "4a0055fb933c8f044a7fa1a321eac7db", 5265),
	 Common::PT_PRT,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "294efb30581661615359ce234e2e85fb", 1596),
	 Common::EN_USA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "f1d44e0b71736e5b0e2516dbfe9bf7e3", 1770),
	 Common::FR_FRA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "b38570cda689d7994806635e0e34ed7f", 1825),
	 Common::IT_ITA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "640d6d3f53986b73f97e1eaad700093a", 1732),
	 Common::DE_DEU,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "4474a0c84b6e7363e1d65cc67278b923", 1769),
	 Common::ES_ESP,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "5ea7264941b31b5961860c7016537641", 1776),
	 Common::PT_PRT,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	{"messenger",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "1e0f9cb47bc203e9e2983b03ffa85174", 185,
		 "textes.txt", "23f577d1201bc3024ca49cb11f9f7347", 5261),
	 Common::EN_ANY,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSUPPORTED,
	 GUIO1(GUIO_NONE)},
	AD_TABLE_END_MARKER};

} // End of namespace PhoenixVR
