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

#include "queen/version.h"
#include "queen/defs.h"

#include "common/config-manager.h"
#include "common/file.h"

namespace Queen {

//! known FOTAQ versions
static const RetailGameVersion gameVersions[] = {
	{ "PEM10", 1, 0x00000008,  22677657 },
	{ "CEM10", 1, 0x0000584E, 190787021 },
	{ "PFM10", 1, 0x0002CD93,  22157304 },
	{ "CFM10", 1, 0x00032585, 186689095 },
	{ "PGM10", 1, 0x00059ACA,  22240013 },
	{ "CGM10", 1, 0x0005F2A7, 217648975 },
	{ "PIM10", 1, 0x000866B1,  22461366 },
	{ "CIM10", 1, 0x0008BEE2, 190795582 },
	{ "CSM10", 1, 0x000B343C, 190730602 },
	{ "CHM10", 1, 0x000DA981, 190705558 },
	{ "PE100", 1, 0x00101EC6,   3724538 },
	{ "PE100", 1, 0x00102B7F,   3732177 },
	{ "PEint", 1, 0x00103838,   1915913 },
	{ "aEM10", 2, 0x00103F1E,    351775 },
	{ "CE101", 2, 0x00107D8D,    563335 },
	{ "PE100", 2, 0x001086D4,    597032 },
	{ "aGM10", 3, 0x00108C6A,    344575 }
};

bool detectVersion(DetectedGameVersion *ver, Common::File *f) {
	memset(ver, 0, sizeof(DetectedGameVersion));

	if (f->readUint32BE() == MKTAG('Q','T','B','L')) {
		f->read(ver->str, 6);
		f->skip(2);
		ver->compression = f->readByte();
		ver->features = GF_REBUILT;
		ver->queenTblVersion = 0;
		ver->queenTblOffset = 0;
	} else {
		const RetailGameVersion *gameVersion = detectGameVersionFromSize(f->size());
		if (gameVersion == nullptr) {
			warning("Unknown/unsupported FOTAQ version");
			return false;
		}
		Common::strcpy_s(ver->str, gameVersion->str);
		ver->compression = COMPRESSION_NONE;
		ver->features = 0;
		ver->queenTblVersion = gameVersion->queenTblVersion;
		ver->queenTblOffset = gameVersion->queenTblOffset;

		// Handle game versions for which versionStr information is irrevelant
		if (gameVersion == &gameVersions[VER_AMI_DEMO]) { // CE101
			ver->language = Common::EN_ANY;
			ver->features |= GF_FLOPPY | GF_DEMO;
			ver->platform = Common::kPlatformAmiga;
			return true;
		}
		if (gameVersion == &gameVersions[VER_AMI_INTERVIEW]) { // PE100
			ver->language = Common::EN_ANY;
			ver->features |= GF_FLOPPY | GF_INTERVIEW;
			ver->platform = Common::kPlatformAmiga;
			return true;
		}
	}

	switch (ver->str[1]) {
	case 'E':
		if (Common::parseLanguage(ConfMan.get("language")) == Common::RU_RUS) {
			ver->language = Common::RU_RUS;
		} else if (Common::parseLanguage(ConfMan.get("language")) == Common::EL_GRC) {
			ver->language = Common::EL_GRC;
		} else {
			ver->language = Common::EN_ANY;
		}
		break;
	case 'F':
		ver->language = Common::FR_FRA;
		break;
	case 'G':
		ver->language = Common::DE_DEU;
		break;
	case 'H':
		ver->language = Common::HE_ISR;
		break;
	case 'I':
		ver->language = Common::IT_ITA;
		break;
	case 'S':
		ver->language = Common::ES_ESP;
		break;
	case 'g':
		ver->language = Common::EL_GRC;
		break;
	case 'R':
		ver->language = Common::RU_RUS;
		break;
	default:
		error("Invalid language id '%c'", ver->str[1]);
		break;
	}

	switch (ver->str[0]) {
	case 'P':
		ver->features |= GF_FLOPPY;
		ver->platform = Common::kPlatformDOS;
		break;
	case 'C':
		ver->features |= GF_TALKIE;
		ver->platform = Common::kPlatformDOS;
		break;
	case 'a':
		ver->features |= GF_FLOPPY;
		ver->platform = Common::kPlatformAmiga;
		break;
	default:
		error("Invalid platform id '%c'", ver->str[0]);
		break;
	}

	if (strcmp(ver->str + 2, "100") == 0 || strcmp(ver->str + 2, "101") == 0) {
		ver->features |= GF_DEMO;
	} else if (strcmp(ver->str + 2, "int") == 0) {
		ver->features |= GF_INTERVIEW;
	}
	return true;
}

const RetailGameVersion *detectGameVersionFromSize(uint32 size) {
	for (int i = 0; i < VER_COUNT; ++i) {
		if (gameVersions[i].dataFileSize == size) {
			return &gameVersions[i];
		}
	}
	return nullptr;
}

} // End of namespace Queen
