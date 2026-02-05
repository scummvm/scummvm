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

namespace Gamos {

const PlainGameDescriptor gamosGames[] = {
	{ "gamos", "Gamos" },
	{ "solgamer", "21 Solitaire" },
	{ "pilots", "Pilots 1" },
	{ "pilots2", "Pilots 2" },
	{ "wild", "WildSnakes"},
	{ "flop", "Flip-Flop"},
	{ "netwalk", "NetWalk"},
	{ "vitamin", "Vitamin"},
	{ 0, 0 }
};

const GamosGameDescription gameDescriptions[] = {
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			24.03.1998 */
			"solgamer",
			0,
			AD_ENTRY1s("solgamer.exe", "t:4a06f82f7638e865e27d68d2e071e827", 24658521),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"solgamer.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"solgamer",
			0,
			AD_ENTRY1s("solgamee.exe", "t:cad7d29da5aa843853fb8e7609a46300", 24624511),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"solgamee.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			24.03.1998 */
			"solgamer",
			0,
			AD_ENTRY1s("solgamee.exe", "t:a32bbae8c381d8cdc4cbac9e18acb0a8", 24676118),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"solgamee.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			30.08.1997 */
			"pilots",
			0,
			AD_ENTRY1s("pilots.exe", "t:82ae05090898af66447bac4f06e910f3", 48357155),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"pilots.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			04.09.1995 ??? */
			"pilots",
			0,
			AD_ENTRY1s("pilde_r.exe", "t:a1bbaa7e59f69cb2be3223c3336982ac", 10770038),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"pilde_r.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			04.09.1995 ??? */
			"pilots",
			0,
			AD_ENTRY1s("pildemo.exe", "t:a1bbaa7e59f69cb2be3223c3336982ac", 10770038),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"pildemo.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			04.09.1995 ??? */
			"pilots",
			0,
			AD_ENTRY1s("pilotsrd.exe", "t:a1bbaa7e59f69cb2be3223c3336982ac", 10770038),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"pilotsrd.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			20.09.1995 ??? */
			"pilots",
			0,
			AD_ENTRY1s("pilde_e.exe", "t:a784fc52f6923817d8ec8b9101e3f3ea", 10904737),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"pilde_e.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.50] by Vadim Sytnikov
			17.12.1997 */
			"pilots",
			0,
			AD_ENTRY1s("pilotsed.exe", "t:190cb2dcf936d0c3a891ba333dbc2f33", 10946501),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"pilotsed.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.54] by Vadim Sytnikov
			14.10.1998 */
			"pilots2",
			0,
			AD_ENTRY1s("pilots2.exe", "t:c434bd8787febde52ccb5ef0e0731c7b", 582283983),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"pilots2.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"wild",
			0,
			AD_ENTRY1s("wildus.exe", "t:320c8dfd26ae5935b71a37227b7fe67f", 11475754),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"wildus.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"wild",
			0,
			AD_ENTRY1s("wildus.exe", "t:a28e6f87a9c5cad5661108678522e27c", 11471282),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"wildus.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"wild",
			0,
			AD_ENTRY1s("wildru.exe", "t:35bcdf2b9a0b022fe546de08ab685bd9", 11510210),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"wildru.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.42] by Vadim Sytnikov
			31.07.1997 */
			"wild",
			0,
			AD_ENTRY1s("wildrudm.exe", "t:335fa22e185426843190460bc57d78a7", 11517820),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"wildrudm.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"wild",
			0,
			AD_ENTRY1s("wildusdm.exe", "t:bddc7069986caedcfaa38ce35d42520b", 11516932),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"wildusdm.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"flop",
			0,
			AD_ENTRY1s("ffloprd.exe", "t:a09793d184d211f819299a5286a75f56", 4637680),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"ffloprd.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"flop",
			0,
			AD_ENTRY1s("ffloped.exe", "t:2f90db8d9083b57c775375cf880dc777", 4609380),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"ffloped.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"flop",
			0,
			AD_ENTRY1s("fflope.exe", "t:7e098e0b565d97eb3d55eb3b6d45e413", 4633340),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"fflope.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"flop",
			0,
			AD_ENTRY1s("ffloped.exe", "t:1be4097f6564e9240c58079addfd4600", 4613290),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"ffloped.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.42] by Vadim Sytnikov
			31.07.1997 */
			"flop",
			0,
			AD_ENTRY1s("fflope.exe", "t:8ffba2d788278457bb2f9661b7bdf4ec", 4629006),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"fflope.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"flop",
			0,
			AD_ENTRY1s("fflopr.exe", "t:7e452d6f5fbfe32dc1ee3c53fba27f88", 4629153),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"fflopr.exe",
		0x80000016
	},
	{ // 1.13rus
		{ /* Copyright (C) 1996 by Gamos JSC, Moscow, Russia...  Run-time engine [1.13] by Vadim Sytnikov */
			"flop",
			0,
			AD_ENTRY1s("fflop.exe", "t:fd32bc85b8686504f3e63c8e00444c68", 6662497),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"fflop.exe",
		0x8000000b
	},
	{ // 1.13eng
		{ /* Copyright (C) 1996 by Gamos JSC, Moscow, Russia...  Run-time engine [1.13] by Vadim Sytnikov */
			"flop",
			0,
			AD_ENTRY1s("fflop.exe", "t:3f7b90481959b408c9f7dface714cb75", 6691028),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"fflop.exe",
		0x8000000b
	},
	{ // 1.29eng
		{ /* Copyright (C) 1996 by Gamos JSC, Moscow, Russia...  Run-time engine [1.29] by Vadim Sytnikov
			04.01.1997 */
			"flop",
			0,
			AD_ENTRY1s("fflop.exe", "t:a1f917e41e24cbb9e2bda3cf2da5e8fb", 6860990),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"fflop.exe",
		0x80000012
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"netwalk",
			0,
			AD_ENTRY1s("it.exe", "t:76c0cce851177ed28e0d78ab989c6b63", 4125894),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"it.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1996 by Gamos JSC, Moscow, Russia...  Run-time engine [1.13] by Vadim Sytnikov */
			"netwalk",
			0,
			AD_ENTRY1s("it.exe", "t:455ad057b10542674c69f428311b6daf", 4744297),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"it.exe",
		0x8000000b
	},
	{
		{ /* Copyright (C) 1996 by Gamos JSC, Moscow, Russia...  Run-time engine [1.13] by Vadim Sytnikov */
			"netwalk",
			0,
			AD_ENTRY1s("netwalk.exe", "t:f7e100541adf71f1cec35ecc914c8ca3", 5607140),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"netwalk.exe",
		0x8000000b
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"netwalk",
			0,
			AD_ENTRY1s("netwalk.exe", "t:3512b1127b93c2ffe074e292f19a0ffb", 4981212),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"netwalk.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.43] by Vadim Sytnikov
			06.08.1997 */
			"netwalk",
			0,
			AD_ENTRY1s("netwalk.exe", "t:f1b1713c122a4d60a773046f6aa1e1d8", 4972704),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"netwalk.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1996 by Gamos JSC, Moscow, Russia...  Run-time engine [1.27] by Vadim Sytnikov
			15.12.1996 */
			"netwalk",
			0,
			AD_ENTRY1s("netwalk.exe", "t:ee6ec7c4d1f451d30038665e2edcc278", 5624675),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"netwalk.exe",
		0x80000012
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"netwalk",
			0,
			AD_ENTRY1s("netwalk.exe", "t:140a884942bbc396ae0640645d8118aa", 4979981),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"netwalk.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"netwalk",
			0,
			AD_ENTRY1s("netwalk.exe", "t:4d78e86ef47ceb6c578081ba367ee8ea", 4984836),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
		"netwalk.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			04.09.1995 ??? */
			"vitamin",
			0,
			AD_ENTRY1s("vitam_e.exe", "t:04cae8b641a7d79a1842c7ac1899b534", 429943),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"vitam_e.exe",
		0x80000016
	},
	{
		{ /* Copyright (C) 1998 by Gamos JSC, Moscow, Russia...  Run-time engine [1.51] by Vadim Sytnikov
			03.12.1998 */
			"vitamin",
			0,
			AD_ENTRY1s("vitam_e.exe", "t:3506cd8e1a32ec3e841a2043d9dabc5b", 1122512),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"vitam_e.exe",
		0x80000018
	},
	{
		{ /* Copyright (C) 1997 by Gamos JSC, Moscow, Russia...  Run-time engine [1.47] by Vadim Sytnikov
			04.09.1995 ??? */
			"vitamin",
			0,
			AD_ENTRY1s("vitam_r.exe", "t:427c0269c7327c7ca4f434e22c5cfce9", 459546),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"vitam_r.exe",
		0x80000016
	},
	{
		AD_TABLE_END_MARKER,
		"",
		0
	}
};

} // End of namespace Gamos
