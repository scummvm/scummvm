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

#include "base/plugins.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/advancedDetector.h"

#include "daad/detection.h"

namespace DAAD {

static const PlainGameDescriptor daadGames[] = {
	{ "daad", "DAAD Game" },

	// Aventuras AD / Adventuras AD
	{ "chichenitza",      "Chichen Itza" },
	{ "cozumel",          "Cozumel" },
	{ "jabato",           "Jabato" },
	{ "latumbafaraon",    "La Tumba del Faraon" },
	{ "aventuraoriginal", "La Aventura Original" },
	{ "aventuraespacial", "La Aventura Espacial" },
	{ "loscarelli",       "Los Vengadores: Carelli" },
	{ "templomaya",       "El Templo Maya" },

	// Modern DAAD releases
	{ "rabenstein",       "Rabenstein" },
	{ "cityofgold",	      "City of Gold"},

	{ nullptr, nullptr }
};

/**
 * Known releases.
 *
 * The AD hashes the first 5000 bytes of the listed file by default, which is
 * what the md5s below are. Anything not listed here still runs: fallbackDetect()
 * validates the DDB header itself and reports it as a generic DAAD title.
 */
static const ADGameDescription gameDescriptions[] = {
	// Rabenstein (Puddle Software, 2020), DOS release: PART1.DDB v2, IBM PC
	// target, with PART1.DAT graphics and a PART1.CHR charset alongside it
	{
		"rabenstein",
		nullptr,
		AD_ENTRY1s("PART1.DDB", "1c8898bde87112b11b8b66d13e8d0140", 23166),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"cityofgold",
		nullptr,
		AD_ENTRY1s("PART1.DDB", "dfdde819bdaab2f8f18f907e334df152", 62016),
		Common::EN_ANY,
		Common::kPlatformAmiga,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
};

/**
 * Filled in by fallbackDetect() for a database that is structurally valid but
 * not in the table above.
 */
static ADGameDescription s_fallbackDesc = {
	"daad",
	"Unknown DAAD game",
	AD_ENTRY1(nullptr, nullptr),
	Common::UNK_LANG,
	Common::kPlatformUnknown,
	ADGF_UNSTABLE,
	GUIO1(GUIO_NOMIDI)
};

// The detection plugin links separately from the engine, so this file cannot
// call into ADP. The header check below is a deliberately conservative
// reimplementation of what DDB_ParseHeader() and DDB_IsPlausibleHeaderLayout()
// do in adp/src-common/ddb.cpp - keep the two in step when the vendored tree
// is updated.

#define DDB_MIN_SIZE 34
#define DDB_MAX_SIZE (1024 * 1024)

/**
 * Load address the offsets stored in the header are relative to, per target
 * machine. The 8 bit targets keep absolute Z80/6502 addresses in the file.
 */
static uint16 baseOffsetForMachine(byte machine) {
	switch (machine) {
	case 1:		// Spectrum
		return 0x8400;
	case 2:		// C64
		return 0x3880;
	case 3:		// CPC
		return 0x2880;
	case 4:		// MSX
		return 0x0100;
	case 7:		// PCW
		return 0x0100;
	case 14:	// Plus/4
		return 0x7080;
	default:	// IBM PC, Atari ST, Amiga, MSX2
		return 0;
	}
}

static bool isKnownMachine(byte machine) {
	// 13 is emitted by drb.php for the VGA256 PC interpreter
	return machine <= 7 || machine == 13 || machine == 14 || machine == 15;
}

static uint16 readOffset(const byte *data, bool littleEndian) {
	return littleEndian ? (uint16)(data[0] | (data[1] << 8))
	                    : (uint16)((data[0] << 8) | data[1]);
}

/**
 * Check whether the header's offset table is self consistent for one of the
 * two byte orders. Every stored offset must land inside the file once the
 * machine's load address has been subtracted.
 */
static bool isPlausibleLayout(const byte *data, uint32 fileSize, byte version, uint16 baseOffset,
		bool littleEndian) {
	uint32 minimumHeaderSize = (version == 1) ? 34 : 36;
	if (fileSize < minimumHeaderSize)
		return false;

	for (int n = 8; n < 30; n += 2) {
		uint16 rawOffset = readOffset(data + n, littleEndian);

		// The token block is optional
		if (n == 8 && rawOffset == 0)
			continue;

		if (rawOffset < baseOffset)
			return false;

		uint32 offset = (uint32)rawOffset - baseOffset;
		if (offset < minimumHeaderSize || offset >= fileSize)
			return false;
	}

	return true;
}

/**
 * Validate a DDB header, returning the target machine and language.
 */
static bool isValidDDB(const byte *data, uint32 fileSize, byte &machine, Common::Language &language) {
	if (fileSize < DDB_MIN_SIZE || fileSize > DDB_MAX_SIZE)
		return false;

	byte version = data[0];
	if (version < 1 || version > 3)
		return false;

	byte lang = data[1] & 0x0F;
	machine = data[1] >> 4;
	if (lang > 1 || !isKnownMachine(machine))
		return false;

	uint16 baseOffset = baseOffsetForMachine(machine);
	if (!isPlausibleLayout(data, fileSize, version, baseOffset, false) &&
		!isPlausibleLayout(data, fileSize, version, baseOffset, true))
		return false;

	// A database with no locations, no messages and no processes is not one
	if (data[4] == 0 && data[5] == 0 && data[7] == 0)
		return false;

	language = (lang == 1) ? Common::ES_ESP : Common::EN_ANY;
	return true;
}

} // End of namespace DAAD

class DAADMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	DAADMetaEngineDetection() : AdvancedMetaEngineDetection(DAAD::gameDescriptions, DAAD::daadGames) {
		// Multi-part games ship one database per part in the game folder
		_maxScanDepth = 1;
	}

	const char *getName() const override {
		return "daad";
	}

	const char *getEngineName() const override {
		return "DAAD Adventure Player";
	}

	const char *getOriginalCopyright() const override {
		return "DAAD (C) Aventuras AD / Tim Gilberts";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist,
			ADDetectedGameExtraInfo **extra) const override;
};

ADDetectedGame DAADMetaEngineDetection::fallbackDetect(const FileMap &allFiles,
		const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		// DAAD databases are always .DDB; snapshots and disk images holding a
		// database are deliberately not probed, they need the interpreter to
		// locate the database inside them.
		Common::String filename = file->getName();
		if (!filename.hasSuffixIgnoreCase(".ddb"))
			continue;

		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;

		uint32 fileSize = (uint32)gameFile.size();
		byte header[512];
		uint32 headerSize = MIN<uint32>(fileSize, sizeof(header));
		if (gameFile.read(header, headerSize) != headerSize) {
			gameFile.close();
			continue;
		}

		byte machine = 0;
		Common::Language language = Common::UNK_LANG;
		if (!DAAD::isValidDDB(header, fileSize, machine, language)) {
			gameFile.close();
			continue;
		}

		gameFile.seek(0);
		Common::String md5 = Common::computeStreamMD5AsString(gameFile, _md5Bytes);
		gameFile.close();

		DAAD::s_fallbackDesc.language = language;
		DAAD::s_fallbackDesc.platform = Common::kPlatformUnknown;

		ADDetectedGame game(&DAAD::s_fallbackDesc);
		game.hasUnknownFiles = true;

		FileProperties fp;
		fp.md5 = md5;
		fp.size = fileSize;
		game.matchedFiles[Common::Path(filename)] = fp;

		// A multi-part game ships one database per part (PART1.DDB,
		// PART2.DDB, ...) but is a single game: the interpreter enumerates
		// the folder and loads the parts itself, so stop at the first one.
		return game;
	}

	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(DAAD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DAADMetaEngineDetection);
