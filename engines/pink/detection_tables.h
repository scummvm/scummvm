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

namespace Pink {

static const ADGameDescription gameDescriptions[] = {
	// Danish, Version 1.0
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "eae050c09d7f6bfbb3166d2c39957e31", 608976918),
		Common::DA_DAN,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Dutch
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "4ae829fb3988ad783409ce8311f95ddc", 613211963),
		Common::NL_NLD,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// English
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "223d0114d443904d8949263d512859da", 618203600),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// English Great Britain
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "1ee65e570adb37d446a3be657e4b2e9a", 619145676),
		Common::EN_GRB,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Finnish
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "32c31829bf7e74a64968dd05f3224ce9", 612549215),
		Common::FI_FIN,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// French
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "fd641b5735fbe41d14db698ece29d2b1", 607185037),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// German
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "4ee8514f7303dea1949d7fc72ff65d8c", 609695309),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Hebrew
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "52e2aba46d6cc47225bd5345775eeb59", 616292424),
		Common::HE_ISR,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Polish
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "3b987bb529e131b92c3eb912871dedbd", 644839372),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Portuguese
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "88a498256c4609550cf59497a372b7a3", 642216577),
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Russian
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "4802bace9cd89a73eb915a075b230646", 635322616),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Spanish
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "1225f76fe3a60d2ed2321ac92e2c1e79", 633626567),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Swedish
	{
		"peril",
		0,
		AD_ENTRY1s("PPTP.ORB", "eadbc52f4c43b85edb7cc493b4149ba0", 633843917),
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Dutch, Version 1.0
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "b769855e2fc94b9180763211c349a9ed", 509498007),
		Common::NL_NLD,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Dutch, Version 2.0
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "993b0491d507efee0010e4f1c000ab8b", 509498617),
		Common::NL_NLD,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// English
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "f480597a78ab70c2021b4141fe44a512", 503443586),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// French
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "d5af74262276f0ffef6605ea0db861d2", 492220293),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// German
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "a396a310e9d42ff43798ffdee2589a1a", 543000636),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Hebrew
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "518453b73021c31566f084b3e4b8bdbf", 502988485),
		Common::HE_ISR,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Polish
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "51fb70412a6a5a6590dcaee842a940ab", 539274161),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Portuguese
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "149661ec6c35488a300293776a74b460", 526755539),
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Russian
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "1e5155c2219b3baea599563e02596ce5", 526369062),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Spanish
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "68040543f153e494e42efe9fab47b850", 508716126),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Swedish
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "7b7909414d93f847ff0c023a06ae1f7e", 500103742),
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	// Danish
	// Version 1.0
	// Contributed by sauravisus in Trac#10919
	{
		"pokus",
		0,
		AD_ENTRY1s("HPP.orb", "3428dda98c21c4b6cd798750016796ab", 513518023),
		Common::DA_DAN,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_DROPPLATFORM,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Pink

#endif
