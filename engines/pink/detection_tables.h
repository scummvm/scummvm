/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PINK_DETECTION_TABLES_H
#define PINK_DETECTION_TABLES_H

#include "gui/EventRecorder.h"

namespace Pink {

static const ADGameDescription gameDescriptions[] = {
	// English, Version 1.0
	{
		"peril",
		0,
		{
			{"PPTP.ORB", NULL, "223d0114d443904d8949263d512859da", 618203600},
			{"PPTP.BRO", NULL, "bbbc78c0525694b32157570e2ec29784", 8945466},
			{"PPTP.EXE", NULL, "82460a604723ed5697fc87cddf9dbeed", 594432},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Russian
	{
		"peril",
		0,
		{
			{"PPTP.ORB", NULL, "4802bace9cd89a73eb915a075b230646", 635322616},
			{"PPTP.BRO", NULL, "87539dbbb34bafbe223c751a60254505", 8945466},
			{"PPTP.EXE", NULL, "739259d239acdc0304f904016e775fb3", 742912},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// English, Version 1.0
	{
		"pokus",
		0,
		{
			{"HPP.orb", NULL, "f480597a78ab70c2021b4141fe44a512", 503443586},
			{"hpp.exe", NULL, "05c321e75dd51e40f00c6d85cf9f2b34", 697856},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	// Russian
	{
		"pokus",
		0,
		{
			{"HPP.orb", NULL, "1e5155c2219b3baea599563e02596ce5", 526369062},
			{"hpp.exe", NULL, "32a5937f69dfce9f159b7be358cbd008", 699904},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Pink

#endif
