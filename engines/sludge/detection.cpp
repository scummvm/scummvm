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
#include "common/debug.h"
#include "common/stream.h"
#include "common/file.h"

#include "engines/advancedDetector.h"

#include "sludge/detection.h"

static const PlainGameDescriptor sludgeGames[] = {
	{ "sludge",			"Sludge Game" },
	{ "welcome",		"Welcome Example" },
	{ "verbcoin",		"Verb Coin" },
	{ "robinsrescue",	"Robin's Rescue" },
	{ "outoforder",		"Out Of Order" },
	{ "frasse",			"Frasse and the Peas of Kejick" },
	{ "interview",		"The Interview" },
	{ "life",			"Life Flashes By" },
	{ "tgttpoacs",		"The Game That Takes Place on a Cruise Ship" },
	{ "mandy",			"Mandy Christmas Adventure" },
	{ "cubert",			"Cubert Badbone, P.I." },
	{ "gjgagsas",		"The Game Jam Game About Games, Secrets and Stuff" },
	{ "tsotc",			"The Secret of Tremendous Corporation" },
	{ "nsc",			"Nathan's Second Chance" },
	{ "atw",			"Above The Waves" },
	{ "leptonsquest",	"Lepton's Quest" },
	{ 0, 0 }
};

#include "sludge/detection_tables.h"

static Sludge::SludgeGameDescription s_fallbackDesc =
{
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NOMIDI)
	},
	0
};

static char s_fallbackFileNameBuffer[51];

class SludgeMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	SludgeMetaEngineDetection() : AdvancedMetaEngineDetection(Sludge::gameDescriptions, sizeof(Sludge::SludgeGameDescription), sludgeGames) {
		_maxScanDepth = 1;
	}

	const char *getEngineId() const override {
		return "sludge";
	}

	const char *getName() const override {
		return "Sludge";
	}

	const char *getOriginalCopyright() const override {
		return "Sludge (C) 2000-2014 Hungry Software and contributors";
	}

	// for fall back detection
	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
};

ADDetectedGame SludgeMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// reset fallback description
	s_fallbackDesc.desc.gameId = "sludge";
	s_fallbackDesc.desc.extra = "";
	s_fallbackDesc.desc.language = Common::EN_ANY;
	s_fallbackDesc.desc.flags = ADGF_UNSTABLE;
	s_fallbackDesc.desc.platform = Common::kPlatformUnknown;
	s_fallbackDesc.desc.guiOptions = GUIO1(GUIO_NOMIDI);
	s_fallbackDesc.languageID = 0;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String fileName = file->getName();
		fileName.toLowercase();
		if (!(fileName.hasSuffix(".slg") || fileName == "gamedata"))
			continue;

		Common::File f;
		if (!f.open(*file))
			continue;

		bool headerBad = false;
		if (f.readByte() != 'S')
			headerBad = true;
		if (f.readByte() != 'L')
			headerBad = true;
		if (f.readByte() != 'U')
			headerBad = true;
		if (f.readByte() != 'D')
			headerBad = true;
		if (f.readByte() != 'G')
			headerBad = true;
		if (f.readByte() != 'E')
			headerBad = true;
		if (headerBad) {
			continue;
		}

		strncpy(s_fallbackFileNameBuffer, fileName.c_str(), 50);
		s_fallbackFileNameBuffer[50] = '\0';
		s_fallbackDesc.desc.filesDescriptions[0].fileName = s_fallbackFileNameBuffer;

		ADDetectedGame game;
		game.desc = &s_fallbackDesc.desc;

		FileProperties tmp;
		if (getFileProperties(allFiles, s_fallbackDesc.desc, fileName, tmp)) {
			game.hasUnknownFiles = true;
			game.matchedFiles[fileName] = tmp;
		}

		return game;
	}

	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(SLUDGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SludgeMetaEngineDetection);
