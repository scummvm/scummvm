/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#define GAMEOPTION_OBJECT_LABELS GUIO_GAMEOPTIONS1

namespace Sword2 {

static const Sword2GameDescription gameDescriptions[] = {
	{
		{
			"sword2",
			"Demo",
			AD_ENTRY1s("players.clu", "effc399b33e5f85329d7633ad4dc5e90", 10091432),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword2",
			"Demo",
			AD_ENTRY1s("players.clu", "5068815a62ba932afba7267bafc9786d", 9904289),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO | GF_SPANISHDEMO
	},

	{
		{
			"sword2",
			"Demo",
			AD_ENTRY1s("players.clu", "e8786804d399310bda3fcbf897bc44f7", 3085812),
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Sword2
