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
#include "tot/detection.h"

namespace Tot {

const PlainGameDescriptor totGames[] = {
	{ "tot", "Trick or Treat" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		"tot",
		nullptr,
		AD_ENTRY3s(
			"AZCCOG.DAT", "2f66724fcd7f51c5b4a715b30f088581", 79916,
			"CREDITOS.DAT", "6885c1fadd25a0c0da1c88f071a30e63", 130080,
			"TOT.EXE", "cf3a34941be45d3207e38f8f5d66ba00", 209520
		),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO5(GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_NO_TRANSITIONS, GAMEOPTION_ORIGINAL_SAVELOAD_DIALOG, GAMEOPTION_OPL3_MODE)
	},
	{
		"tot",
		nullptr,
		AD_ENTRY3s(
			"AZCCOG.DAT", "2f66724fcd7f51c5b4a715b30f088581", 79916,
			"CREDITOS.DAT", "bca1c63cfee9ec8b722f7715e21b5e8e", 130080,
			"TOT.EXE", "e711989e516bd33d0da6c278c82a0493", 207280
		),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO5(GUIO_NOSPEECH, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_NO_TRANSITIONS, GAMEOPTION_ORIGINAL_SAVELOAD_DIALOG, GAMEOPTION_OPL3_MODE)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Tot
