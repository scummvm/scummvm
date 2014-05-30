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

#include "voyeur/staticres.h"

namespace Voyeur {

const int COMPUTER_DEFAULTS[] = {
	18, 1, 0, 1, 33, 0, 998, -1, 18, 2, 0, 1, 41, 0,
	998, -1, 18, 3, 0, 1, 47, 0, 998, -1, 18, 4, 0,
	1, 53, 0, 998, -1, 18, 5, 0, 1, 46, 0, 998, -1,
	18, 6, 0, 1, 50, 0, 998, -1, 18, 7, 0, 1, 40, 0,
	998, -1, 18, 8, 0, 1, 43, 0, 998, -1, 19, 1, 0,
	2, 28, 0, 998, -1
};

const int RESOLVE_TABLE[] = {
	0x2A00, 0x4A00, 0x1000, 0x4B00, 0x2C00, 0x4F00, 0x1400, 0x5000,
	0x1700, 0x5100, 0x1800, 0x5200, 0x3300, 0x5400, 0x3700, 0x5500,
	0x1A00, 0x1C00, 0x1E00, 0x1F00, 0x2100, 0x2200, 0x2400, 0x2700,
	0x2B00, 0x1100, 0x4C00, 0x1200, 0x4D00, 0x1300, 0x4E00, 0x2E00,
	0x1900, 0x3200, 0x3400, 0x3800, 0x2800, 0x3E00, 0x4100, 0x2900,
	0x4400, 0x4600, 0x5300, 0x3900, 0x7600, 0x7200, 0x7300, 0x7400,
	0x7500
};

const int LEVEL_H[] = {
	4, 7, 7, 8, 9, 10, 2, 2, 4, 8, 8, 9, 9, 10, 10, 11, 11
};

const int LEVEL_M[] = {
	0, 0, 30, 0, 30, 0, 0, 0, 30, 0, 30, 0, 45, 0, 30, 0, 30
};

const int BLIND_TABLE[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 9, 10, 11, 1, 11, 5, 12,
	13, 16, 15, 16, 17, 18, 5, 6, 18, 17, 13, 13, 14, 14,
	5, 12, 6, 6, 13, 14, 13
};

const int COMPUTER_SCREEN_TABLE[] = {
	269, 128, 307, 163,
	269, 128, 307, 163,
	68, 79, 98, 102,
	68, 79, 98, 102,
	68, 79, 98, 102,
	68, 79, 98, 102,
	248, 138, 291, 163,
	83, 132, 143, 156,
	248, 138, 291, 163,
	83, 132, 143, 156,
	83, 132, 143, 156,
	248, 138, 291, 163,
	68, 79, 98, 102,
	68, 79, 98, 102
};

const char *const SZ_FILENAMES[] = {
	"A2110100", nullptr, "A2300100", nullptr, "B1220100", nullptr, "C1220100", nullptr,
	"C1290100", nullptr, "D1220100", nullptr, "D1270100", nullptr, "E1210100", nullptr,
	"E1260100", nullptr, "E1280100", nullptr, "E1325100", nullptr, "F1200100", nullptr,
	"G1250100", nullptr, "G1260100", nullptr, "H1200100", nullptr, "H1230100", nullptr,
	"H1310100", nullptr, "I1300100", nullptr, "J1220100", nullptr, "J1230100", nullptr,
	"J1320100", nullptr, "K1260100", nullptr, "K1280100", nullptr, "K1325100", nullptr,
	"L1210100", nullptr, "L1280100", nullptr, "L1290100", nullptr, "L1300100", nullptr,
	"L1310100", nullptr, "M1260100", nullptr, "M1310100", nullptr, "N1210100", nullptr,
	"N1225100", nullptr, "N1275510", nullptr, "N1280510", nullptr, "N1325100", nullptr,
	"O1230100", nullptr, "O1260100", nullptr, "O1260520", nullptr, "O1280100", nullptr,
	"O1325540", nullptr, "P1276710", nullptr, "P1280540", nullptr, "P1280740", nullptr,
	"P1290510", nullptr, "P1325100", nullptr, "P1325300", nullptr, "P1325520", nullptr,
	"Q1230100", nullptr, "Q1240530", nullptr, "Q1240730", nullptr, "Q1260100", nullptr,
	"Q1260520", nullptr, "Q1260720", nullptr, "Q1325100", nullptr, "R1280540", nullptr,
	"Z1110510", nullptr, "Z1110520", nullptr, "Z1110530", nullptr, "Z1110540", nullptr,
	"Z1110545", nullptr, "Z2320100", nullptr, "Z2905300", nullptr, "Z3110100", nullptr,
	"Z3115510", nullptr, "Z3115520", nullptr, "Z3115530", nullptr, "Z3115540", nullptr,
	"Z4915100", nullptr, "Z4915200", nullptr, "Z4915300",
	nullptr, nullptr, nullptr, nullptr, nullptr,
	"MMARG", "MZACK", "MREED", "MJESSI", "MCHLOE", "MCAMERA", "MENDCRED",
	"NEWCALL2", "PHONE1", "PHONE2", "PHONE3", "PHONE6", "PHONE8",
	"B1300100", "C1250100", "C1320100", "D1320100", "E1210200", "E1260200",
	"E1280200", "E1310100", "G1230100", "G1300100", "I1210100", "I1270100",
	"I1280100", "J1250100", "J1280100", "K1260200", "K1270100", "K1325200",
	"L1240100", "M1200100", "M1230100", "M1290100", "N1250100", "N1260100",
	"N1280100", "O1250510", "O1290510", "O1320510", "O1320710", "P1240100",
	"P1240530", "P1260100", "P1270100", "P1280100", "P1280530", "P1320530",
	"Q1240100", "E1325100"
};

const char *const SATURDAY = "Saturday";
const char *const SUNDAY = "Sunday";
const char *const MONDAY = "Monday Morning";
const char *const AM = "am";
const char *const PM = "pm";

const char *const START_OF_MESSAGE = "*** Start of Message ***";
const char *const END_OF_MESSAGE = "*** End of Message ***";

const char *const EVENT_TYPE_STRINGS[4] = { "Video", "Audio", "Evidence", "Computer" };

int DOT_LINE_START[9] = {
	0xE880, 0xE9C0, 0xEB00, 0xEC40, 0xED80, 0xEEC0, 0xF000, 0xF140, 0xF280
};
int DOT_LINE_OFFSET[9] = {
	144, 143, 142, 141, 141, 141, 142, 143, 144
};
int DOT_LINE_LENGTH[9] = {
	5, 7, 9, 11, 11, 11, 9, 7, 5
};

const char *const PIRACY_MESSAGE[] = {
	"It is illegal to make",
	"unauthorized copies of",
	"this software. Duplication",
	"of this software for any",
	"reason including sale,",
	"loan, rental, or gift is a",
	"crime. Penalties include",
	"fines of up to $50,000",
	"and jail terms up to",
	"5 years."
};

} // End of namespace Voyeur
