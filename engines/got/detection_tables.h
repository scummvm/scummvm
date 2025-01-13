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

namespace Got {

const PlainGameDescriptor gotGames[] = {
	{"got", "God of Thunder"},
	{nullptr, nullptr}
};

const ADGameDescription gameDescriptions[] = {
	{
		"got",
		 "v1.0:143",
		 AD_ENTRY1s("gotres.dat", "0fe859bcfd06772be0885bf510c2877d", 739732),
		 Common::EN_ANY,
		 Common::kPlatformDOS,
		 ADGF_UNSTABLE,
		 GUIO1(GUIO_NOSPEECH)
	},
	{
		"got",
		 "v1.0:143 Shareware",
		 AD_ENTRY1s("gotres.dat", "c316221a3b9d12a838faef55fde2f34c", 482738),
		 Common::EN_ANY,
		 Common::kPlatformDOS,
		 ADGF_UNSTABLE | ADGF_DEMO,
		 GUIO1(GUIO_NOSPEECH)
	},
	{
		"got",
		 "v1.1:144",
		 AD_ENTRY1s("gotres.dat", "747ed508ffa3808156a4eb080e9859f8", 739710),
		 Common::EN_ANY,
		 Common::kPlatformDOS,
		 ADGF_UNSTABLE,
		 GUIO1(GUIO_NOSPEECH)
	},
	{
		"got",
		 "v1.1:144 Shareware",
		 AD_ENTRY1s("gotres.dat", "c443aa09450566ee32998c3ebb15cbaa", 485858),
		 Common::EN_ANY,
		 Common::kPlatformDOS,
		 ADGF_UNSTABLE | ADGF_DEMO,
		 GUIO1(GUIO_NOSPEECH)
	},

	AD_TABLE_END_MARKER};

} // End of namespace Got
