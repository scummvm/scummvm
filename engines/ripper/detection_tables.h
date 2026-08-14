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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

namespace Ripper {

const PlainGameDescriptor ripperGames[] = {
	{ "ripper", "Ripper" },
	{ nullptr, nullptr }
};

const ADGameDescription gameDescriptions[] = {
	{
		"ripper",
		nullptr,
		AD_ENTRY2s(
			"interfac.pl", "96ee5c8c5a1236fcab47e39885ace4dd", 7771946,
			"sound.pl", "7e9a9194b059fd7542f72bf90d459102", 975066),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOSUBTITLES, GAMEOPTION_SKIP_INTRO)
	},
	{
		"ripper",
		"Demo",
		AD_ENTRY2s(
			"interfac.pl", "a33ad101a8ebc6fe8929efcb40239db7", 4688365,
			"sound.pl", "f130b8d291e3e78e04a7cf65c1cb772c", 1877450),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOSUBTITLES)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Ripper
