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
 * Copyright 2020 Google
 *
 */

#ifndef HADESCH_DETECTION_TABLES_H
#define HADESCH_DETECTION_TABLES_H

namespace Hadesch {

static const ADGameDescription gameDescriptions[] = {

	// Hades Challenge
	{
		"hadesch",
		0,
		{
			{"hadesch.exe", 0, "178b3a69171cb5a4eeeddd0d5993b8c5", 1134592},
			{"WD.POD", 0, "be7030fc4229e69e719ee2c756eb6ba1", 7479768},
			{"ol.pod", 0, "7cabba8d1d4f1239e312e045ef4e9735", 5621074},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"hadesch",
		0,
		{
			{"Hades Challenge PPC", 0, "c7213a365a3cab7e9f2b423fa4a204f5", 1724646},
			{"WD.POD", 0, "be7030fc4229e69e719ee2c756eb6ba1", 7479768},
			{"ol.pod", 0, "7cabba8d1d4f1239e312e045ef4e9735", 5621074},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM | ADGF_MACRESFORK,
		GUIO1(GUIO_NOMIDI)

	},
	{
		"hadesch",
		0,
		{
			{"hadesch.exe", 0, "660735787346ab1bfe0d219bea441486", 1007616},
			{"WD.POD", 0, "5098edae755135814bb86f2676c41cc2", 8691909},
			{"ol.pod", 0, "c82e105d9013edc2cc20f0a630e304d5", 5684953},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)

	},
	{
		"hadesch",
		0,
		{
			{"setup.exe", 0, "853c199f1ef35d576213f71092bcd0c3", 7491209},
			{"ol.pod", 0, "c82e105d9013edc2cc20f0a630e304d5", 5684953},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)

	},

	// Bad dumps from archive.org
	{
		"hadesch",
		0,
		{
			{"hadesch.exe", 0, "178b3a69171cb5a4eeeddd0d5993b8c5", 1134592},
			{"WD.POD", 0, "be7030fc4229e69e719ee2c756eb6ba1", 7479768},
			{"ol.pod", 0, "d41d8cd98f00b204e9800998ecf8427e", 5621074},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM | ADGF_PIRATED,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"hadesch",
		0,
		{
			{"hadesch.exe", 0, "178b3a69171cb5a4eeeddd0d5993b8c5", 1134592},
			{"WD.POD", 0, "be7030fc4229e69e719ee2c756eb6ba1", 7479768},
			{"ol.pod", 0, "6bf95a48f366bdf8af3a198c7b723c77", 5621074},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM | ADGF_PIRATED,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Hadesch

#endif
