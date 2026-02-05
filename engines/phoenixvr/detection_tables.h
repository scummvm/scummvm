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
	{"cameronlochness", "Cameron Files: The Secret at Loch Ness"},
	{"messenger", "The Messenger/Louvre: The Final Curse"},
	{0, 0}};

const ADGameDescription gameDescriptions[] = {
	{"necrono",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "86294b9c445c3e06e24269c84036a207", 223,
		 "textes.txt", "f795f35b079cb8ef599724a2a7336c7e", 5319),
	 Common::EN_ANY,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	{"lochness",
	 nullptr,
	 AD_ENTRY2s(
		 "script.pak", "a7ee3aae653658f93bba7f237bcf06f3", 1904,
		 "textes.txt", "294efb30581661615359ce234e2e85fb", 1596),
	 Common::EN_ANY,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_UNSUPPORTED,
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
