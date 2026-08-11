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

#include "common/macresman.h"

#include "scumm/macgui/macgui_impl.h"
#include "scumm/scumm.h"

namespace Scumm {

Common::String MacGuiImpl::readCString(uint8 *&data) {
	while (data[0] == '\0') {
		data++;
	}

	Common::String result(reinterpret_cast<const char *>(data));
	data += result.size() + 1;

	debug(8, "MacGuiImpl::readCString(): %s", result.c_str());
	return result;
}

Common::String MacGuiImpl::readPascalString(uint8 *&data) {
	while (data[0] == '\0') {
		data++;
	}

	Common::String result(reinterpret_cast<const char *>(&data[1]), (uint32)data[0]);
	data += (uint32)data[0] + 1;

	debug(8, "MacGuiImpl::readPascalString(): %s", result.c_str());
	return result;
}

#define SKIP_C(x) { MacGuiImpl::kMSISkip, MacGuiImpl::kStrC,      x }
#define SKIP_P(x) { MacGuiImpl::kMSISkip, MacGuiImpl::kStrPascal, x }

static const MacGuiImpl::MacSTRSParsingEntry strsIndy3Table[] = {
	SKIP_C(6),
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	SKIP_P(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	SKIP_C(67),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	// SKIP_C(1),
	// SKIP_P(4),
	// SKIP_C(3),
	// SKIP_P(2),
	// SKIP_C(4),
	// SKIP_P(1),
	// SKIP_C(1),
	// SKIP_P(5),
};

static const MacGuiImpl::MacSTRSParsingEntry strsLoomTable[] = {
	SKIP_C(6),
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(2),
	SKIP_P(1),
	SKIP_C(67),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	// SKIP_C(1),
	// SKIP_P(4),
	// SKIP_C(3),
	// SKIP_P(2),
	// SKIP_C(4),
	// SKIP_P(1),
	// SKIP_C(1),
	// SKIP_P(5),
	// SKIP_C(7),
};

static const MacGuiImpl::MacSTRSParsingEntry strsMI1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	// SKIP_C(75)
};

static const MacGuiImpl::MacSTRSParsingEntry strsMI2Variant1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString35,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString36,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString37,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString38,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString39,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString40,              MacGuiImpl::kStrPascal, 1 },
	// SKIP_C(95)
};

static const MacGuiImpl::MacSTRSParsingEntry strsMI2Variant2Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_C(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString35,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString36,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString37,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString38,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString39,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString40,              MacGuiImpl::kStrPascal, 1 },
	// SKIP_C(95)
};

static const MacGuiImpl::MacSTRSParsingEntry strsIndy4CDVariant1Table[] = {
	SKIP_C(144),
	SKIP_P(1),
	SKIP_C(2),
	SKIP_P(10),
	SKIP_C(51),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString35,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString36,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString37,              MacGuiImpl::kStrPascal, 1 },
};

static const MacGuiImpl::MacSTRSParsingEntry strsIndy4CDVariant2Table[] = {
	SKIP_C(97),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString35,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString36,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString37,              MacGuiImpl::kStrPascal, 1 },
};

static const MacGuiImpl::MacSTRSParsingEntry strsIndy4FloppyVariant1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString38,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString35,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString36,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString37,              MacGuiImpl::kStrPascal, 1 },
};

static const MacGuiImpl::MacSTRSParsingEntry strsIndy4FloppyVariant2Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               MacGuiImpl::kStrPascal, 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIGameFile,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString38,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString28,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString34,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString35,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString36,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString37,              MacGuiImpl::kStrPascal, 1 },
};

static const MacGuiImpl::MacSTRSParsingEntry strsIndy4DemoTable[] = {
	SKIP_C(98),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              MacGuiImpl::kStrPascal, 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    MacGuiImpl::kStrC,      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, MacGuiImpl::kStrC,      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   MacGuiImpl::kStrPascal, 1 },
	SKIP_C(9),
	{ MacGuiImpl::kMSIAboutString1,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString2,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString3,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString4,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString5,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString6,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString7,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString8,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString9,               MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString10,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString11,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString12,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString13,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString14,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString15,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString16,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString17,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString18,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString19,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString20,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString21,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString22,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString23,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString24,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString25,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString26,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString27,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString29,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString30,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString31,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString32,              MacGuiImpl::kStrPascal, 1 },
	{ MacGuiImpl::kMSIAboutString33,              MacGuiImpl::kStrPascal, 1 },
};

#undef SKIP_C
#undef SKIP_P

bool MacGuiImpl::readStrings() {
	if (_vm->_game.version >= 6 || _vm->_game.id == GID_MANIAC) {
		_strsStrings.clear();
		_strsStrings.reserve(128);
		for (int i = 0; i < 128; i++) {
			_strsStrings.emplace_back("");
		}
		return true;
	}

	Common::MacResManager resource;
	resource.open(_resourceFile);
	uint32 strsLen = resource.getResLength(MKTAG('S', 'T', 'R', 'S'), 0);

	if (strsLen <= 0)
		return false;

	Common::SeekableReadStream *strsStream = resource.getResource(MKTAG('S', 'T', 'R', 'S'), 0);
	uint8 *strsBlock = (uint8 *)malloc(strsLen);
	strsStream->read(strsBlock, strsLen);

	uint8 *strsData = strsBlock;
	const MacSTRSParsingEntry *parsingTable = nullptr;
	int parsingTableSize = 0;

	if (_vm->_game.id == GID_INDY3) {
		switch (strsLen) {
		case 2950:
			// 1.7 8/17/90
			parsingTable = strsIndy3Table;
			parsingTableSize = ARRAYSIZE(strsIndy3Table);
			break;
		}
	} else if (_vm->_game.id == GID_LOOM) {
		switch (strsLen) {
		case 3184:
			// 1.2 25-JAN-91
			parsingTable = strsLoomTable;
			parsingTableSize = ARRAYSIZE(strsLoomTable);
			break;
		}
	} else if (_vm->_game.id == GID_MONKEY) {
		switch (strsLen) {
		case 5986:
			// version 2.4
			parsingTable = strsMI1Table;
			parsingTableSize = ARRAYSIZE(strsMI1Table);
			break;
		}
	} else if (_vm->_game.id == GID_MONKEY2) {
		switch (strsLen) {
		case 6574:
			parsingTable = strsMI2Variant1Table;
			parsingTableSize = ARRAYSIZE(strsMI2Variant1Table);
			break;
		case 6602:
			// v1.0 11/5/92 from the LucasArts Mac CD Game Pack II
			parsingTable = strsMI2Variant2Table;
			parsingTableSize = ARRAYSIZE(strsMI2Variant2Table);
			break;
		}
	} else if (_vm->_game.id == GID_INDY4) {
		switch (strsLen) {
		case 6516:
			parsingTable = strsIndy4FloppyVariant1Table;
			parsingTableSize = ARRAYSIZE(strsIndy4FloppyVariant1Table);
			break;
		case 6612:
			// V1.0 10-9-92 from the LucasArts Mac CD Game Pack II
			parsingTable = strsIndy4FloppyVariant2Table;
			parsingTableSize = ARRAYSIZE(strsIndy4FloppyVariant2Table);
			break;
		case 6836: // CD
			// V1.0 10-9-92
			parsingTable = strsIndy4CDVariant1Table;
			parsingTableSize = ARRAYSIZE(strsIndy4CDVariant1Table);
			break;
		case 6772: // CD
			// fate_v1.5
			parsingTable = strsIndy4CDVariant2Table;
			parsingTableSize = ARRAYSIZE(strsIndy4CDVariant2Table);
			break;
		case 6312: // Demo
			parsingTable = strsIndy4DemoTable;
			parsingTableSize = ARRAYSIZE(strsIndy4DemoTable);
			break;
		}
	}

	if (parsingTable)
		parseSTRSBlock(strsData, parsingTable, parsingTableSize);
	else
		warning("MacGuiImpl::readStrings(): String parsing table not defined for this variant of the game; STRS resource %d bytes", strsLen);

	free(strsBlock);
	delete strsStream;

	return parsingTable != nullptr;
}

void MacGuiImpl::parseSTRSBlock(uint8 *strsData, const MacSTRSParsingEntry *parsingTable, int parsingTableSize) {
	_strsStrings.clear();
	_strsStrings.reserve(128);
	for (int i = 0; i < 128; i++) {
		_strsStrings.emplace_back("");
	}

	for (int i = 0; i < parsingTableSize; i++) {
		MacSTRSParsingEntry entry = parsingTable[i];

		if (entry.strId == kMSISkip) {
			for (int j = 0; j < entry.numStrings; j++) {
				entry.parsingMethod == MacGuiImpl::kStrC ? readCString(strsData) : readPascalString(strsData);
			}
		} else {
			for (int j = 0; j < entry.numStrings; j++) {
				if (entry.parsingMethod == MacGuiImpl::kStrC) {
					_strsStrings[entry.strId] = readCString(strsData);
				} else if (entry.parsingMethod == MacGuiImpl::kStrPascal) {
					_strsStrings[entry.strId] = readPascalString(strsData);
				} else {
					error("MacGuiImpl::parseSTRSBlock(): invalid parsing method encountered (%d)", entry.parsingMethod);
				}
			}
		}
	}
}

} // End of namespace Scumm
