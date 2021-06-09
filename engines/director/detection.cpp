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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/file.h"

#include "director/detection.h"
#include "director/director.h"

#include "director/detection_tables.h"

namespace Director {
const char *directoryGlobs[] = {
	"install",
	"l_zone",
	"win_data",						// L-ZONE
	"data",
	"gadget",						// Gadget
	"vnc",
	"vnc2",							// Virtual Nightclub
	"program",						// Arc Media products
	"demodata",						// Edmark demos
	"media",						// Gundam
	"mvm",							// Master of the Elements
	"module_1",
	"module_2",						// Takeru
	"simpdata",
	"simpsons",						// Simpsons Cartoon Studio
	"dane",							// Polish Kontyngent '99 catalog
	"datas",						// O!KAY! subscription from Domino Verlag
	"daten",						// Bibi Blocksberg games (German)
	"source",						// Grand Prix 2006 directory
	"data_k",
	"data_1",
	"data_2",						// JumpStart World
	"numbers",						// JumpStart Learning Games
	"xtras",
	"files",						// Babar
	"swanlake",						// Barbie Swan Lake
	"assets",						// Barbie Nail Designer
	"fjpc",							// Forestia Jr
	"movies",						// Mulle Meck (Gary Gadget) series
	"demos",						// Headbone samplers
	"blender",						// Blender CD-ROM magazines
	"bilder",
	"endsoung",
	"gerhelp",
	"gervideo",
	"mausejagd mit ton",			// Rename from Mäusejagd mit Ton
	"music",
	"solos",
	"sounds",
	"startsnd",						// GGMD German
	"duthelp",
	"dutvideo",
	"endsound",
	"muizen in \'t hotel, geluid",	// GGMD Dutch
	"macos9",
	"macosx",						// Sethi 3
	"kripos",
	"strisser",
	"polis",
	"poliisi",
	"cd_data",						// Polis
	"fusion macintosh folder",		// Fusion
	"oslo",							// Oslo's Tools & Gadgets
	"ojo2000",
	"stuff",						// Abrapalabra
	"tooned",						// Stay Tooned
	"sam rupert",					// Who Killed Sam Rupert?
	"content",
	"pcinst",						// Gus and the CyberBuds series
	"p2data",
	"p2media",
	"p3media",
	"annat",						// Pettson & Findus
	"fctdata",
	"fct",					   		// Felix the Cat
	"dayfiles",
	"xmasfun",						// Fisher-Price Xmas Fun
	"95instal",						// Fisher-Price Little People
	"01",					   		// Doing it in C++
	"freading",
	"fsetup",						// My First Reading Words (Eureka)
	"english",
	"deutsch",
	"francais",						// Rename from Français
	"italiano",						// Virtual Tourism Paris
	"please copy to hd. G3",		// Rename from HDにｺﾋﾟｰして下さい。G3
	"_files_",						// The Gate
	"popup",						// Pop Up Computer
	"alpha",						// Interactive Alphabet
	"technik",						// Glasklar Technology Interactive
	0
};
}

static const DebugChannelDef debugFlagList[] = {
	{Director::kDebugCompile, "compile", "Lingo Compilation"},
	{Director::kDebugCompileOnly, "compileonly", "Skip Lingo code execution"},
	{Director::kDebugDesktop, "desktop", "Show the Classic Mac desktop"},
	{Director::kDebugEndVideo, "endvideo", "Fake that the end of video is reached setting"},
	{Director::kDebugEvents, "events", "Event processing"},
	{Director::kDebugFast, "fast", "Fast (no delay) playback"},
	{Director::kDebugFewFramesOnly, "fewframesonly", "Only run the first 10 frames"},
	{Director::kDebugImages, "images", "Image drawing"},
	{Director::kDebugLingoExec, "lingoexec", "Lingo Execution"},
	{Director::kDebugLoading, "loading", "Loading"},
	{Director::kDebugNoBytecode, "nobytecode", "Do not execute Lscr bytecode"},
	{Director::kDebugNoLoop, "noloop", "Do not loop the playback"},
	{Director::kDebugParse, "parse", "Lingo code parsing"},
	{Director::kDebugPreprocess, "preprocess", "Lingo preprocessing"},
	{Director::kDebugScreenshot, "screenshot", "screenshot each frame"},
	{Director::kDebugSlow, "slow", "Slow playback"},
	{Director::kDebugText, "text", "Text rendering"},
	DEBUG_CHANNEL_END
};

class DirectorMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	DirectorMetaEngineDetection() : AdvancedMetaEngineDetection(Director::gameDescriptions, sizeof(Director::DirectorGameDescription), directorGames) {
		_maxScanDepth = 5;
		_directoryGlobs = Director::directoryGlobs;
	}

	const char *getEngineId() const override {
		return "director";
	}

	const char *getName() const override {
		return "Macromedia Director";
	}

	const char *getOriginalCopyright() const override {
		return "Macromedia Director (C) 1990-1995 Macromedia";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
};

static Director::DirectorGameDescription s_fallbackDesc = {
	{
		"director",
		"",
		AD_ENTRY1(0, 0),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	Director::GID_GENERIC,
	0
};

static char s_fallbackFileNameBuffer[51];
static char s_fallbackExtraBuf[256];

ADDetectedGame DirectorMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// TODO: Handle Mac fallback

	// reset fallback description
	Director::DirectorGameDescription *desc = &s_fallbackDesc;
	desc->desc.gameId = "director";
	desc->desc.extra = "";
	desc->desc.language = Common::UNK_LANG;
	desc->desc.flags = ADGF_NO_FLAGS;
	desc->desc.platform = Common::kPlatformWindows;
	desc->desc.guiOptions = GUIO0();
	desc->desc.filesDescriptions[0].fileName = 0;
	desc->version = 0;
	desc->gameGID = Director::GID_GENERIC;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String fileName = file->getName();
		fileName.toLowercase();
		if (!fileName.hasSuffix(".exe"))
			continue;

		Common::File f;
		if (!f.open(*file))
			continue;

		f.seek(-4, SEEK_END);

		uint32 offset = f.readUint32LE();

		if (f.eos() || offset == 0 || offset >= (uint32)(f.size() - 4))
			continue;

		f.seek(offset);

		uint32 tag = f.readUint32LE();

		switch (tag) {
		case MKTAG('P', 'J', '9', '3'):
		case MKTAG('3', '9', 'J', 'P'):
			desc->version = 400;
			break;
		case MKTAG('P', 'J', '9', '5'):
			desc->version = 500;
			break;
		case MKTAG('P', 'J', '0', '0'):
			desc->version = 700;
			break;
		default:
			// Prior to version 4, there was no tag here. So we'll use a bit of a
			// heuristic to detect. The first field is the entry count, of which
			// there should only be one.
			if ((tag & 0xFFFF) != 1)
				continue;

			f.skip(3);

			uint32 mmmSize = f.readUint32LE();

			if (f.eos() || mmmSize == 0)
				continue;

			byte fileNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(fileNameSize);
			byte directoryNameSize = f.readByte();

			if (f.eos())
				continue;

			f.skip(directoryNameSize);

			if (f.pos() != f.size() - 4)
				continue;

			// Assume v3 at this point (for now at least)
			desc->version = 300;
		}

		strncpy(s_fallbackFileNameBuffer, fileName.c_str(), 50);
		s_fallbackFileNameBuffer[50] = '\0';
		desc->desc.filesDescriptions[0].fileName = s_fallbackFileNameBuffer;

		Common::String extra = Common::String::format("v%d.%02d", desc->version / 100, desc->version % 100);
		Common::strlcpy(s_fallbackExtraBuf, extra.c_str(), sizeof(s_fallbackExtraBuf) - 1);
		desc->desc.extra = s_fallbackExtraBuf;

		warning("Director fallback detection %s", extra.c_str());

		ADDetectedGame game(&desc->desc);

		FileProperties tmp;
		if (getFileProperties(allFiles, desc->desc, file->getName(), tmp)) {
			game.hasUnknownFiles = true;
			game.matchedFiles[file->getName()] = tmp;
		}

		return game;
	}

	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(DIRECTOR_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DirectorMetaEngineDetection);
