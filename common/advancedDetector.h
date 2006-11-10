/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef COMMON_ADVANCED_DETECTOR_H
#define COMMON_ADVANCED_DETECTOR_H

#include "common/fs.h"

namespace Common {

struct ADGameFileDescription {
	const char *fileName;
	uint16 fileType;
	const char *md5;
};

struct ADGameDescription {
	const char *name;
	const char *extra;
	int filesCount;
	const ADGameFileDescription *filesDescriptions;
	Language language;
	Platform platform;
};

struct ADObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

bool ADTrue(void);

typedef Array<int> ADList;
typedef Array<const ADGameDescription*> ADGameDescList;

#define ADVANCED_DETECTOR_GAMEID_LIST(engine,list) \
	GameList Engine_##engine##_gameIDList() { \
		GameList games; \
		const PlainGameDescriptor *g = list; \
		while (g->gameid) { \
			games.push_back(*g); \
			g++; \
		} \
		 \
		return games; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_FIND_GAMEID(engine,list,obsoleteList)					  \
	GameDescriptor Engine_##engine##_findGameID(const char *gameid) { \
		const PlainGameDescriptor *g = list; \
		while (g->gameid) { \
			if (0 == scumm_stricmp(gameid, g->gameid)) \
				return *g; \
			g++; \
		} \
		 \
		GameDescriptor gs; \
		if (obsoleteList) {\
			const Common::ADObsoleteGameID *o = obsoleteList;	\
			while (o->from) { \
				if (0 == scumm_stricmp(gameid, o->from)) { \
					gs.gameid = gameid; \
					gs.description = "Obsolete game ID"; \
					return gs; \
				} \
				o++; \
			} \
		} else \
			return *g; \
		return gs; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DETECT_GAMES(engine,function) \
	DetectedGameList Engine_##engine##_detectGames(const FSList &fslist) { \
		return function(fslist);						\
	} \
	void dummyFuncToAllowTrailingSemicolon()


#define ADVANCED_DETECTOR_ENGINE_CREATE(engine,createFunction,engineName,obsoleteList) \
	PluginError Engine_##engine##_create(OSystem *syst, Engine **engine) { \
		assert(syst); \
		assert(engine); \
		const char *gameid = ConfMan.get("gameid").c_str(); \
		 \
		if (obsoleteList) { \
			for (const Common::ADObsoleteGameID *o = obsoleteList; o->from; ++o) { \
				if (!scumm_stricmp(gameid, o->from)) { \
					gameid = o->to; \
					ConfMan.set("gameid", o->to); \
					 \
					if (o->platform != Common::kPlatformUnknown) \
						ConfMan.set("platform", Common::getPlatformCode(o->platform)); \
					\
					warning("Target upgraded from %s to %s", o->from, o->to); \
					ConfMan.flushToDisk(); \
					break; \
				} \
			} \
		} \
		 \
		FSList fslist; \
		FilesystemNode dir(ConfMan.get("path")); \
		if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) { \
			warning("%s: invalid game path '%s'", engineName, dir.path().c_str()); \
			return kInvalidPathError; \
		} \
		 \
		DetectedGameList detectedGames = Engine_##engine##_detectGames(fslist); \
		 \
		for (uint i = 0; i < detectedGames.size(); i++) { \
			if (detectedGames[i].gameid == gameid) { \
				*engine = new createFunction(syst); \
				return kNoError; \
			} \
		} \
		 \
		warning("%s: Unable to locate game data at path '%s'", engineName, dir.path().c_str()); \
		return kNoGameDataFoundError; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_TO_DETECTED_GAME(list) \
	DetectedGame toDetectedGame(const ADGameDescription &g) { \
		const char *title = 0; \
		\
		const PlainGameDescriptor *sg = list; \
		while (sg->gameid) { \
			if (!scumm_stricmp(g.name, sg->gameid)) \
				title = sg->description; \
			sg++; \
		} \
		\
		DetectedGame dg(g.name, title, g.language, g.platform); \
		dg.updateDesc(g.extra); \
		return dg; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(function,descriptions) \
	DetectedGameList function(const FSList &fslist) { \
		DetectedGameList detectedGames; \
		Common::AdvancedDetector AdvDetector; \
		Common::ADList matches; \
		Common::ADGameDescList descList; \
		\
		for (int i = 0; i < ARRAYSIZE(descriptions); i++) \
			descList.push_back((const ADGameDescription *)&descriptions[i]); \
		\
		AdvDetector.registerGameDescriptions(descList); \
		AdvDetector.setFileMD5Bytes(FILE_MD5_BYTES); \
		\
		matches = AdvDetector.detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown); \
		\
		for (uint i = 0; i < matches.size(); i++) \
			detectedGames.push_back(toDetectedGame(descriptions[matches[i]].desc)); \
		\
		return detectedGames; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DETECT_INIT_GAME(function,descriptions,varname,postFunction) \
	bool function() { \
		int gameNumber = -1; \
		\
		DetectedGameList detectedGames; \
		Common::AdvancedDetector AdvDetector; \
		Common::ADList matches; \
		Common::ADGameDescList descList; \
		\
		Common::Language language = Common::UNK_LANG; \
		Common::Platform platform = Common::kPlatformUnknown; \
		\
		if (ConfMan.hasKey("language")) \
			language = Common::parseLanguage(ConfMan.get("language")); \
		if (ConfMan.hasKey("platform")) \
			platform = Common::parsePlatform(ConfMan.get("platform")); \
		\
		Common::String gameid = ConfMan.get("gameid"); \
		\
		for (int i = 0; i < ARRAYSIZE(descriptions); i++) \
			descList.push_back((const ADGameDescription *)&descriptions[i]); \
		\
		AdvDetector.registerGameDescriptions(descList); \
		AdvDetector.setFileMD5Bytes(FILE_MD5_BYTES); \
		\
		matches = AdvDetector.detectGame(NULL, language, platform); \
		\
		for (uint i = 0; i < matches.size(); i++) { \
			if (toDetectedGame(descriptions[matches[i]].desc).gameid == gameid) { \
				gameNumber = matches[i]; \
				break; \
			} \
		} \
		\
		if (gameNumber >= ARRAYSIZE(descriptions) || gameNumber == -1) { \
			error("%s wrong gameNumber", "##function##");								\
		} \
		\
		debug(2, "Running %s", toDetectedGame(descriptions[gameNumber].desc).description.c_str()); \
		\
		varname = &descriptions[gameNumber]; \
		\
		return postFunction(); \
	} \
	void dummyFuncToAllowTrailingSemicolon()


class AdvancedDetector {

public:
	AdvancedDetector();
	~AdvancedDetector() {};


	void registerGameDescriptions(ADGameDescList gameDescriptions) {
		_gameDescriptions = gameDescriptions;
	}

	/**
	 * Specify number of bytes which are used to calculate MD5.
	 * Default value is 0 which means whole file.
	 */
	void setFileMD5Bytes(int bytes) { _fileMD5Bytes = bytes; }

	/**
	 * Detect games in specified directory.
	 * Parameters language and platform are used to pass on values
	 * specified by the user. I.e. this is used to restrict search scope.
	 *
	 * @param fslist	FSList to scan or NULL for scanning all specified
	 *  default directories.
	 * @param language	restrict results to specified language only
	 * @param platform	restrict results to specified platform only
	 * @return	list of indexes to GameDescriptions of matched games
	 */
	ADList detectGame(const FSList *fslist, Language language, Platform platform);

private:
	ADGameDescList _gameDescriptions;

	int _fileMD5Bytes;

	String getDescription(int num) const;
};

}	// End of namespace Common

#endif
