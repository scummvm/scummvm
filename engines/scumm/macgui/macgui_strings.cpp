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

#define SKIP_C(x) { MacGuiImpl::kMSISkip, "C",      x }
#define SKIP_P(x) { MacGuiImpl::kMSISkip, "Pascal", x }

static MacGuiImpl::MacSTRSParsingEntry strsIndy3Table[] = {
	SKIP_C(6),
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	SKIP_P(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	SKIP_C(67),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "C",      1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	// SKIP_C(1),
	// SKIP_P(4),
	// SKIP_C(3),
	// SKIP_P(2),
	// SKIP_C(4),
	// SKIP_P(1),
	// SKIP_C(1),
	// SKIP_P(5),
};

static MacGuiImpl::MacSTRSParsingEntry strsLoomTable[] = {
	SKIP_C(6),
	SKIP_P(2),
	SKIP_C(2),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(2),
	SKIP_P(1),
	SKIP_C(67),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "C",      1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
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

static MacGuiImpl::MacSTRSParsingEntry strsMI1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	// SKIP_C(75)
};

static MacGuiImpl::MacSTRSParsingEntry strsMI2Variant1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString39,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString40,              "Pascal", 1 },
	// SKIP_C(95)
};

static MacGuiImpl::MacSTRSParsingEntry strsMI2Variant2Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_C(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString39,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString40,              "Pascal", 1 },
	// SKIP_C(95)
};

static MacGuiImpl::MacSTRSParsingEntry strsIndy4CDVariant1Table[] = {
	SKIP_C(144),
	SKIP_P(1),
	SKIP_C(2),
	SKIP_P(10),
	SKIP_C(51),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
};

static MacGuiImpl::MacSTRSParsingEntry strsIndy4FloppyVariant1Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
};

static MacGuiImpl::MacSTRSParsingEntry strsIndy4FloppyVariant2Table[] = {
	SKIP_C(93),
	SKIP_P(1),
	{ MacGuiImpl::kMSIAboutGameName,              "Pascal", 1 },
	SKIP_P(2),
	{ MacGuiImpl::kMSIAreYouSureYouWantToQuit,    "C",      1 },
	{ MacGuiImpl::kMSIRoughCommandMsg,            "C",      1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIAreYouSureYouWantToRestart, "C",      1 },
	SKIP_C(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSIGameName,                   "Pascal", 1 },
	SKIP_C(1),
	{ MacGuiImpl::kMSIOpenGameFile,               "Pascal", 1 },
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	SKIP_P(1),
	{ MacGuiImpl::kMSISaveGameFileAs,             "Pascal", 1 },
	{ MacGuiImpl::kMSIGameFile,                   "Pascal", 1 },
	SKIP_C(6),
	SKIP_P(2),
	{ MacGuiImpl::kMSIAboutString1,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString2,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString3,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString4,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString5,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString6,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString7,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString8,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString9,               "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString10,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString11,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString12,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString13,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString14,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString15,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString16,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString17,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString18,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString19,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString20,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString21,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString22,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString23,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString38,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString24,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString25,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString26,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString27,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString28,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString29,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString30,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString31,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString32,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString33,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString34,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString35,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString36,              "Pascal", 1 },
	{ MacGuiImpl::kMSIAboutString37,              "Pascal", 1 },
};

#undef SKIP_C
#undef SKIP_P

void MacGuiImpl::readStrings() {
	Common::MacResManager resource;
	resource.open(_resourceFile);
	uint32 strsLen = resource.getResLength(MKTAG('S', 'T', 'R', 'S'), 0);

	if (strsLen <= 0)
		return;

	Common::SeekableReadStream *strsStream = resource.getResource(MKTAG('S', 'T', 'R', 'S'), 0);
	uint8 *strsBlock = (uint8 *)malloc(strsLen);
	strsStream->read(strsBlock, strsLen);

	uint8 *strsData = strsBlock;
	MacSTRSParsingEntry *parsingTable = nullptr;
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
		}
	} else {
		error("MacGuiImpl::readStrings(): String parsing table not defined for this game");
	}

	if (parsingTable)
		parseSTRSBlock(strsData, parsingTable, parsingTableSize);
	else
		warning("MacGuiImpl::readStrings(): String parsing table not defined for this variant of the game; STRS resource %d bytes", strsLen);

	free(strsBlock);
	delete strsStream;
}

void MacGuiImpl::parseSTRSBlock(uint8 *strsData, MacSTRSParsingEntry *parsingTable, int parsingTableSize) {
	_strsStrings.clear();
	_strsStrings.reserve(128);
	for (int i = 0; i < 128; i++) {
		_strsStrings.emplace_back("");
	}

	for (int i = 0; i < parsingTableSize; i++) {
		MacSTRSParsingEntry entry = parsingTable[i];

		if (entry.strId == kMSISkip) {
			for (int j = 0; j < entry.numStrings; j++) {
				entry.parsingMethod == "C" ? readCString(strsData) : readPascalString(strsData);
			}
		} else {
			for (int j = 0; j < entry.numStrings; j++) {
				if (entry.parsingMethod == "C") {
					_strsStrings[entry.strId] = readCString(strsData);
				} else if (entry.parsingMethod == "Pascal") {
					_strsStrings[entry.strId] = readPascalString(strsData);
				} else {
					error("MacGuiImpl::parseSTRSBlock(): invalid parsing method encountered (%s)", entry.parsingMethod.c_str());
				}
			}
		}
	}
}

} // End of namespace Scumm
