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
#include "common/util.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "gui/EventRecorder.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

/**
 * Adapter to be able to use Common::Archive based code from the AD.
 */
class FileMapArchive : public Common::Archive {
public:
	FileMapArchive(const AdvancedMetaEngineDetection::FileMap &fileMap) : _fileMap(fileMap) {}

	bool hasFile(const Common::String &name) const override {
		return _fileMap.contains(name);
	}

	int listMembers(Common::ArchiveMemberList &list) const override {
		int files = 0;
		for (AdvancedMetaEngineDetection::FileMap::const_iterator it = _fileMap.begin(); it != _fileMap.end(); ++it) {
			list.push_back(Common::ArchiveMemberPtr(new Common::FSNode(it->_value)));
			++files;
		}

		return files;
	}

	const Common::ArchiveMemberPtr getMember(const Common::String &name) const override {
		AdvancedMetaEngineDetection::FileMap::const_iterator it = _fileMap.find(name);
		if (it == _fileMap.end()) {
			return Common::ArchiveMemberPtr();
		}

		return Common::ArchiveMemberPtr(new Common::FSNode(it->_value));
	}

	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const override {
		Common::FSNode fsNode = _fileMap[name];
		return fsNode.createReadStream();
	}

private:
	const AdvancedMetaEngineDetection::FileMap &_fileMap;
};

static Common::String sanitizeName(const char *name, int maxLen) {
	Common::String res;
	Common::String word;
	Common::String lastWord;
	const char *origname = name;

	do {
		if (Common::isAlnum(*name)) {
			word += tolower(*name);
		} else {
			// Skipping short words and "the"
			if ((word.size() > 2 && !word.equals("the")) || (!word.empty() && Common::isDigit(word[0]))) {
				// Adding first word, or when word fits
				if (res.empty() || word.size() < maxLen)
					res += word;

				maxLen -= word.size();
			}

			if ((*name && *(name + 1) == 0) || !*name) {
				if (res.empty()) // Make sure that we add at least something
					res += word.empty() ? lastWord : word;

				break;
			}

			if (!word.empty())
				lastWord = word;

			word.clear();
		}
		if (*name)
			name++;
	} while (maxLen > 0);

	if (res.empty())
		error("AdvancedDetector: Incorrect extra in game: \"%s\"", origname);

	return res;
}

/**
 * Generate a preferred target value as
 *   GAMEID-PLAFORM-LANG
 * or (if ADGF_DEMO has been set)
 *   GAMEID-demo-PLAFORM-LANG
 */
static Common::String generatePreferredTarget(const ADGameDescription *desc, int maxLen) {
	Common::String res;

	if (desc->flags & ADGF_AUTOGENTARGET && desc->extra && *desc->extra) {
		res = sanitizeName(desc->extra, maxLen);
	} else {
		res = desc->gameId;
	}

	if (desc->flags & ADGF_DEMO) {
		res = res + "-demo";
	}

	if (desc->flags & ADGF_CD) {
		res = res + "-cd";
	}

	if (desc->flags & ADGF_REMASTERED) {
		res = res + "-remastered";
	}

	if (desc->platform != Common::kPlatformDOS && desc->platform != Common::kPlatformUnknown && !(desc->flags & ADGF_DROPPLATFORM)) {
		res = res + "-" + getPlatformAbbrev(desc->platform);
	}

	if (desc->language != Common::EN_ANY && desc->language != Common::UNK_LANG && !(desc->flags & ADGF_DROPLANGUAGE)) {
		res = res + "-" + getLanguageCode(desc->language);
	}

	return res;
}

DetectedGame AdvancedMetaEngineDetection::toDetectedGame(const ADDetectedGame &adGame) const {
	const ADGameDescription *desc = adGame.desc;

	const char *title;
	const char *extra;
	if (desc->flags & ADGF_USEEXTRAASTITLE) {
		title = desc->extra;
		extra = "";
	} else {
		const PlainGameDescriptor *pgd = findPlainGameDescriptor(desc->gameId, _gameIds);
		if (pgd) {
			title = pgd->description;
		} else {
			title = "";
		}
		extra = desc->extra;
	}

	DetectedGame game(getEngineId(), desc->gameId, title, desc->language, desc->platform, extra, desc->flags & ADGF_UNSUPPORTED);
	game.hasUnknownFiles = adGame.hasUnknownFiles;
	game.matchedFiles = adGame.matchedFiles;
	game.preferredTarget = generatePreferredTarget(desc, _maxAutogenLength);

	game.gameSupportLevel = kStableGame;
	if (desc->flags & ADGF_UNSTABLE)
		game.gameSupportLevel = kUnstableGame;
	else if (desc->flags & ADGF_TESTING)
		game.gameSupportLevel = kTestingGame;
	else if (desc->flags & ADGF_UNSUPPORTED)
		game.gameSupportLevel = kUnupportedGame;

	game.setGUIOptions(desc->guiOptions + _guiOptions);
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(desc->language));

	if (desc->flags & ADGF_ADDENGLISH)
		game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));

	if (_flags & kADFlagUseExtraAsHint)
		game.extra = desc->extra;

	return game;
}

bool cleanupPirated(ADDetectedGames &matched) {
	// OKay, now let's sense presence of pirated games
	if (!matched.empty()) {
		for (uint j = 0; j < matched.size();) {
			if (matched[j].desc->flags & ADGF_PIRATED)
				matched.remove_at(j);
			else
				++j;
		}

		// We ruled out all variants and now have nothing
		if (matched.empty()) {
			warning("Illegitimate game copy detected. We provide no support in such cases");
			if (GUI::GuiManager::hasInstance()) {
				GUI::MessageDialog dialog(_("Illegitimate game copy detected. We provide no support in such cases"));
				dialog.runModal();
			};
			return true;
		}
	}

	return false;
}


DetectedGames AdvancedMetaEngineDetection::detectGames(const Common::FSList &fslist) const {
	FileMap allFiles;

	if (fslist.empty())
		return DetectedGames();

	// Compose a hashmap of all files in fslist.
	composeFileHashMap(allFiles, fslist, (_maxScanDepth == 0 ? 1 : _maxScanDepth));

	// Run the detector on this
	ADDetectedGames matches = detectGame(fslist.begin()->getParent(), allFiles, Common::UNK_LANG, Common::kPlatformUnknown, "");

	cleanupPirated(matches);

	DetectedGames detectedGames;
	for (uint i = 0; i < matches.size(); i++) {
		DetectedGame game = toDetectedGame(matches[i]);

		if (game.hasUnknownFiles) {
			// Non fallback games with unknown files cannot be added/launched
			game.canBeAdded = false;
		}

		detectedGames.push_back(game);
	}

	bool foundKnownGames = false;
	for (uint i = 0; i < detectedGames.size(); i++) {
		foundKnownGames |= detectedGames[i].canBeAdded;
	}

	if (!foundKnownGames) {
		// Use fallback detector if there were no matches by other means
		ADDetectedGame fallbackDetectionResult = fallbackDetect(allFiles, fslist);

		if (fallbackDetectionResult.desc) {
			DetectedGame fallbackDetectedGame = toDetectedGame(fallbackDetectionResult);
			fallbackDetectedGame.preferredTarget += "-fallback";

			detectedGames.push_back(fallbackDetectedGame);
		}
	}

	return detectedGames;
}

const ExtraGuiOptions AdvancedMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	if (!_extraGuiOptions)
		return ExtraGuiOptions();

	ExtraGuiOptions options;

	// If there isn't any target specified, return all available GUI options.
	// Only used when an engine starts in order to set option defaults.
	if (target.empty()) {
		for (const ADExtraGuiOptionsMap *entry = _extraGuiOptions; entry->guioFlag; ++entry)
			options.push_back(entry->option);

		return options;
	}

	// Query the GUI options
	const Common::String guiOptionsString = ConfMan.get("guioptions", target);
	const Common::String guiOptions = parseGameGUIOptions(guiOptionsString);

	// Add all the applying extra GUI options.
	for (const ADExtraGuiOptionsMap *entry = _extraGuiOptions; entry->guioFlag; ++entry) {
		if (guiOptions.contains(entry->guioFlag))
			options.push_back(entry->option);
	}

	return options;
}

Common::Error AdvancedMetaEngineDetection::createInstance(OSystem *syst, Engine **engine) const {
	assert(engine);

	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;
	Common::String extra;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));
	if (_flags & kADFlagUseExtraAsHint) {
		if (ConfMan.hasKey("extra"))
			extra = ConfMan.get("extra");
	}

	Common::String gameid = ConfMan.get("gameid");

	Common::String path;
	if (ConfMan.hasKey("path")) {
		path = ConfMan.get("path");
	} else {
		path = ".";
		warning("No path was provided. Assuming the data files are in the current directory");
	}
	Common::FSNode dir(path);
	Common::FSList files;
	if (!dir.isDirectory() || !dir.getChildren(files, Common::FSNode::kListAll)) {
		warning("Game data path does not exist or is not a directory (%s)", path.c_str());
		return Common::kNoGameDataFoundError;
	}

	if (files.empty())
		return Common::kNoGameDataFoundError;

	// Compose a hashmap of all files in fslist.
	FileMap allFiles;
	composeFileHashMap(allFiles, files, (_maxScanDepth == 0 ? 1 : _maxScanDepth));

	// Run the detector on this
	ADDetectedGames matches = detectGame(files.begin()->getParent(), allFiles, language, platform, extra);

	if (cleanupPirated(matches))
		return Common::kNoGameDataFoundError;

	ADDetectedGame agdDesc;
	for (uint i = 0; i < matches.size(); i++) {
		if (matches[i].desc->gameId == gameid && !matches[i].hasUnknownFiles) {
			agdDesc = matches[i];
			break;
		}
	}

	if (!agdDesc.desc) {
		// Use fallback detector if there were no matches by other means
		ADDetectedGame fallbackDetectedGame = fallbackDetect(allFiles, files);
		agdDesc = fallbackDetectedGame;
		if (agdDesc.desc) {
			// Seems we found a fallback match. But first perform a basic
			// sanity check: the gameid must match.
			if (agdDesc.desc->gameId != gameid)
				agdDesc = ADDetectedGame();
		}
	}

	if (!agdDesc.desc)
		return Common::kNoGameDataFoundError;

	DetectedGame gameDescriptor = toDetectedGame(agdDesc);

	// If the GUI options were updated, we catch this here and update them in the users config
	// file transparently.
	ConfMan.setAndFlush("guioptions", gameDescriptor.getGUIOptions());

	bool showTestingWarning = false;

#ifdef RELEASE_BUILD
	showTestingWarning = true;
#endif

	if (((gameDescriptor.gameSupportLevel == kUnstableGame
			|| (gameDescriptor.gameSupportLevel == kTestingGame
					&& showTestingWarning)))
			&& !Engine::warnUserAboutUnsupportedGame())
		return Common::kUserCanceled;

	if (gameDescriptor.gameSupportLevel == kUnupportedGame) {
		Engine::errorUnsupportedGame(gameDescriptor.extra);
		return Common::kUserCanceled;
	}

	debug(2, "Running %s", gameDescriptor.description.c_str());
	initSubSystems(agdDesc.desc);

	PluginList pl = EngineMan.getPlugins(PLUGIN_TYPE_ENGINE);
	Plugin *plugin = nullptr;

	// By this point of time, we should have only one plugin in memory.
	if (pl.size() == 1) {
		plugin = pl[0];
	}

	if (plugin) {
		// Call child class's createInstanceMethod.
		return plugin->get<AdvancedMetaEngine>().createInstance(syst, engine, agdDesc.desc);
	}

	return Common::Error(Common::kEnginePluginNotFound);
}

void AdvancedMetaEngineDetection::composeFileHashMap(FileMap &allFiles, const Common::FSList &fslist, int depth, const Common::String &parentName) const {
	if (depth <= 0)
		return;

	if (fslist.empty())
		return;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		Common::String tstr = (_matchFullPaths && !parentName.empty() ? parentName + "/" : "") + file->getName();

		if (file->isDirectory()) {
			Common::FSList files;

			if (!_directoryGlobs)
				continue;

			bool matched = false;
			for (const char * const *glob = _directoryGlobs; *glob; glob++)
				if (file->getName().matchString(*glob, true)) {
					matched = true;
					break;
				}

			if (!matched)
				continue;

			if (!file->getChildren(files, Common::FSNode::kListAll))
				continue;

			composeFileHashMap(allFiles, files, depth - 1, tstr);
		}

		// Strip any trailing dot
		if (tstr.lastChar() == '.')
			tstr.deleteLastChar();

		allFiles[tstr] = *file;	// Record the presence of this file
	}
}

bool AdvancedMetaEngineDetection::getFileProperties(const FileMap &allFiles, const ADGameDescription &game, const Common::String fname, FileProperties &fileProps) const {
	// FIXME/TODO: We don't handle the case that a file is listed as a regular
	// file and as one with resource fork.

	if (game.flags & ADGF_MACRESFORK) {
		FileMapArchive fileMapArchive(allFiles);

		Common::MacResManager macResMan;

		if (!macResMan.open(fname, fileMapArchive))
			return false;

		fileProps.md5 = macResMan.computeResForkMD5AsString(_md5Bytes);
		fileProps.size = macResMan.getResForkDataSize();

		if (fileProps.size != 0)
			return true;
	}

	if (!allFiles.contains(fname))
		return false;

	Common::File testFile;

	if (!testFile.open(allFiles[fname]))
		return false;

	fileProps.size = (int32)testFile.size();
	fileProps.md5 = Common::computeStreamMD5AsString(testFile, _md5Bytes);
	return true;
}

bool AdvancedMetaEngine::getFilePropertiesExtern(uint md5Bytes, const FileMap &allFiles, const ADGameDescription &game, const Common::String fname, FileProperties &fileProps) const {
	// FIXME/TODO: We don't handle the case that a file is listed as a regular
	// file and as one with resource fork.

	if (game.flags & ADGF_MACRESFORK) {
		FileMapArchive fileMapArchive(allFiles);

		Common::MacResManager macResMan;

		if (!macResMan.open(fname, fileMapArchive))
			return false;

		fileProps.md5 = macResMan.computeResForkMD5AsString(md5Bytes);
		fileProps.size = macResMan.getResForkDataSize();

		if (fileProps.size != 0)
			return true;
	}

	if (!allFiles.contains(fname))
		return false;

	Common::File testFile;

	if (!testFile.open(allFiles[fname]))
		return false;

	fileProps.size = (int32)testFile.size();
	fileProps.md5 = Common::computeStreamMD5AsString(testFile, md5Bytes);
	return true;
}

ADDetectedGames AdvancedMetaEngineDetection::detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra) const {
	FilePropertiesMap filesProps;
	ADDetectedGames matched;

	const ADGameFileDescription *fileDesc;
	const ADGameDescription *g;
	const byte *descPtr;

	debug(3, "Starting detection in dir '%s'", parent.getPath().c_str());

	// Check which files are included in some ADGameDescription *and* whether
	// they are present. Compute MD5s and file sizes for the available files.
	for (descPtr = _gameDescriptors; ((const ADGameDescription *)descPtr)->gameId != nullptr; descPtr += _descItemSize) {
		g = (const ADGameDescription *)descPtr;

		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			Common::String fname = fileDesc->fileName;

			if (filesProps.contains(fname))
				continue;

			FileProperties tmp;
			if (getFileProperties(allFiles, *g, fname, tmp)) {
				debug(3, "> '%s': '%s'", fname.c_str(), tmp.md5.c_str());
			}

			// Both positive and negative results are cached to avoid
			// repeatedly checking for files.
			filesProps[fname] = tmp;
		}
	}

	int maxFilesMatched = 0;
	bool gotAnyMatchesWithAllFiles = false;

	// MD5 based matching
	uint i;
	for (i = 0, descPtr = _gameDescriptors; ((const ADGameDescription *)descPtr)->gameId != nullptr; descPtr += _descItemSize, ++i) {
		g = (const ADGameDescription *)descPtr;

		// Do not even bother to look at entries which do not have matching
		// language and platform (if specified).
		if ((language != Common::UNK_LANG && g->language != Common::UNK_LANG && g->language != language
			 && !(language == Common::EN_ANY && (g->flags & ADGF_ADDENGLISH))) ||
			(platform != Common::kPlatformUnknown && g->platform != Common::kPlatformUnknown && g->platform != platform)) {
			continue;
		}

		if ((_flags & kADFlagUseExtraAsHint) && !extra.empty() && g->extra != extra)
			continue;

		ADDetectedGame game(g);
		bool allFilesPresent = true;
		int curFilesMatched = 0;

		// Try to match all files for this game
		for (fileDesc = game.desc->filesDescriptions; fileDesc->fileName; fileDesc++) {
			Common::String tstr = fileDesc->fileName;

			if (!filesProps.contains(tstr) || filesProps[tstr].size == -1) {
				allFilesPresent = false;
				break;
			}

			game.matchedFiles[tstr] = filesProps[tstr];

			if (game.hasUnknownFiles)
				continue;

			if (fileDesc->md5 != nullptr && fileDesc->md5 != filesProps[tstr].md5) {
				debug(3, "MD5 Mismatch. Skipping (%s) (%s)", fileDesc->md5, filesProps[tstr].md5.c_str());
				game.hasUnknownFiles = true;
				continue;
			}

			if (fileDesc->fileSize != -1 && fileDesc->fileSize != filesProps[tstr].size) {
				debug(3, "Size Mismatch. Skipping");
				game.hasUnknownFiles = true;
				continue;
			}

			debug(3, "Matched file: %s", tstr.c_str());
			curFilesMatched++;
		}

		// We found at least one entry with all required files present.
		// That means that we got new variant of the game.
		//
		// Without this check we would have erroneous checksum display
		// where only located files will be enlisted.
		//
		// Potentially this could rule out variants where some particular file
		// is really missing, but the developers should better know about such
		// cases.
		if (allFilesPresent && !gotAnyMatchesWithAllFiles) {
			if (matched.empty() || strcmp(matched.back().desc->gameId, g->gameId) != 0)
				matched.push_back(game);
		}

		if (allFilesPresent && !game.hasUnknownFiles) {
			debug(2, "Found game: %s (%s %s/%s) (%d)", g->gameId, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);

			if (curFilesMatched > maxFilesMatched) {
				debug(2, " ... new best match, removing all previous candidates");
				maxFilesMatched = curFilesMatched;

				matched.clear();	// Remove any prior, lower ranked matches.
				matched.push_back(game);
			} else if (curFilesMatched == maxFilesMatched) {
				matched.push_back(game);
			} else {
				debug(2, " ... skipped");
			}

			gotAnyMatchesWithAllFiles = true;
		} else {
			debug(5, "Skipping game: %s (%s %s/%s) (%d)", g->gameId, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);
		}
	}

	return matched;
}

ADDetectedGame AdvancedMetaEngineDetection::detectGameFilebased(const FileMap &allFiles, const ADFileBasedFallback *fileBasedFallback) const {
	const ADFileBasedFallback *ptr;
	const char* const* filenames;

	int maxNumMatchedFiles = 0;
	ADDetectedGame result;

	for (ptr = fileBasedFallback; ptr->desc; ++ptr) {
		const ADGameDescription *agdesc = ptr->desc;
		int numMatchedFiles = 0;
		bool fileMissing = false;

		for (filenames = ptr->filenames; *filenames; ++filenames) {
			debug(3, "++ %s", *filenames);
			if (!allFiles.contains(*filenames)) {
				fileMissing = true;
				break;
			}

			numMatchedFiles++;
		}

		if (!fileMissing) {
			debug(4, "Matched: %s", agdesc->gameId);

			if (numMatchedFiles > maxNumMatchedFiles) {
				maxNumMatchedFiles = numMatchedFiles;

				debug(4, "and overridden");

				ADDetectedGame game(agdesc);
				game.hasUnknownFiles = true;

				for (filenames = ptr->filenames; *filenames; ++filenames) {
					FileProperties tmp;

					if (getFileProperties(allFiles, *agdesc, *filenames, tmp))
						game.matchedFiles[*filenames] = tmp;
				}

				result = game;
			}
		}
	}

	return result;
}

PlainGameList AdvancedMetaEngineDetection::getSupportedGames() const {
	return PlainGameList(_gameIds);
}

PlainGameDescriptor AdvancedMetaEngineDetection::findGame(const char *gameId) const {
	// First search the list of supported gameids for a match.
	const PlainGameDescriptor *g = findPlainGameDescriptor(gameId, _gameIds);
	if (g)
		return *g;

	// No match found
	return PlainGameDescriptor::empty();
}

AdvancedMetaEngineDetection::AdvancedMetaEngineDetection(const void *descs, uint descItemSize, const PlainGameDescriptor *gameIds, const ADExtraGuiOptionsMap *extraGuiOptions)
	: _gameDescriptors((const byte *)descs), _descItemSize(descItemSize), _gameIds(gameIds),
	  _extraGuiOptions(extraGuiOptions) {

	_md5Bytes = 5000;
	_flags = 0;
	_guiOptions = GUIO_NONE;
	_maxScanDepth = 1;
	_directoryGlobs = NULL;
	_matchFullPaths = false;
	_maxAutogenLength = 15;
}

void AdvancedMetaEngineDetection::initSubSystems(const ADGameDescription *gameDesc) const {
#ifdef ENABLE_EVENTRECORDER
	if (gameDesc) {
		g_eventRec.processGameDescription(gameDesc);
	}
#endif
}

Common::Error AdvancedMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	PluginList pl = PluginMan.getPlugins(PLUGIN_TYPE_ENGINE);
	if (pl.size() == 1) {
		Plugin *metaEnginePlugin = PluginMan.getMetaEngineFromEngine(pl[0]);
		if (metaEnginePlugin) {
			return metaEnginePlugin->get<AdvancedMetaEngineDetection>().createInstance(syst, engine);
		}
	}

	return Common::Error();
}
