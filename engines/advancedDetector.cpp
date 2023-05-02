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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/debug.h"
#include "common/util.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/md5.h"
#include "common/config-manager.h"
#include "common/punycode.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"
#include "common/translation.h"
#include "common/compression/clickteam.h"
#include "common/compression/installshield_cab.h"
#include "common/compression/installshieldv3_archive.h"
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
	FileMapArchive(const AdvancedMetaEngineDetectionBase::FileMap &fileMap) : _fileMap(fileMap) {}

	bool hasFile(const Common::Path &path) const override {
		return _fileMap.contains(path);
	}

	int listMembers(Common::ArchiveMemberList &list) const override {
		int files = 0;
		for (AdvancedMetaEngineDetectionBase::FileMap::const_iterator it = _fileMap.begin(); it != _fileMap.end(); ++it) {
			list.push_back(Common::ArchiveMemberPtr(new Common::FSNode(it->_value)));
			++files;
		}

		return files;
	}

	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override {
		AdvancedMetaEngineDetectionBase::FileMap::const_iterator it = _fileMap.find(path);
		if (it == _fileMap.end()) {
			return Common::ArchiveMemberPtr();
		}

		return Common::ArchiveMemberPtr(new Common::FSNode(it->_value));
	}

	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override {
		Common::FSNode fsNode = _fileMap.getValOrDefault(path);
		return fsNode.createReadStream();
	}

private:
	const AdvancedMetaEngineDetectionBase::FileMap &_fileMap;
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
				if (res.empty() || (int)word.size() < maxLen)
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
static Common::String generatePreferredTarget(const ADGameDescription *desc, int maxLen, Common::String targetID) {
	Common::String res;

	if (!targetID.empty()) {
		res = targetID;
	} else if (desc->flags & ADGF_AUTOGENTARGET && desc->extra && *desc->extra) {
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

	if (desc->flags & ADGF_DVD) {
		res = res + "-dvd";
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

DetectedGame AdvancedMetaEngineDetectionBase::toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const {
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

	if (extraInfo) {
		if (!extraInfo->gameName.empty())
			title = extraInfo->gameName.c_str();
	}

	DetectedGame game(getName(), desc->gameId, title, desc->language, desc->platform, extra, ((desc->flags & (ADGF_UNSUPPORTED | ADGF_WARNING)) != 0));
	game.hasUnknownFiles = adGame.hasUnknownFiles;
	game.matchedFiles = adGame.matchedFiles;

	if (extraInfo && !extraInfo->targetID.empty()) {
		game.preferredTarget = generatePreferredTarget(desc, _maxAutogenLength, extraInfo->targetID);
	} else {
		game.preferredTarget = generatePreferredTarget(desc, _maxAutogenLength, Common::String());
	}

	game.gameSupportLevel = kStableGame;
	if (desc->flags & ADGF_UNSTABLE)
		game.gameSupportLevel = kUnstableGame;
	else if (desc->flags & ADGF_TESTING)
		game.gameSupportLevel = kTestingGame;
	else if (desc->flags & ADGF_UNSUPPORTED)
		game.gameSupportLevel = kUnsupportedGame;
	else if (desc->flags & ADGF_WARNING)
		game.gameSupportLevel = kWarningGame;

	game.setGUIOptions(desc->guiOptions + _guiOptions);
	game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(desc->language));

	if (desc->flags & ADGF_ADDENGLISH)
		game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));

	if (_flags & kADFlagUseExtraAsHint)
		game.extra = desc->extra;

	return game;
}

bool AdvancedMetaEngineDetectionBase::cleanupPirated(ADDetectedGames &matched) const {
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

DetectedGames AdvancedMetaEngineDetectionBase::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	FileMap allFiles;

	if (fslist.empty())
		return DetectedGames();

	// Sometimes this method is called directly, so we have to build the maps, especially
	// the _directoryGlobsMap
	preprocessDescriptions();

	// Compose a hashmap of all files in fslist.
	composeFileHashMap(allFiles, fslist, (_maxScanDepth == 0 ? 1 : _maxScanDepth));

	// Run the detector on this
	ADDetectedGames matches = detectGame(fslist.begin()->getParent(), allFiles, Common::UNK_LANG, Common::kPlatformUnknown, "", skipADFlags, skipIncomplete);

	cleanupPirated(matches);

	DetectedGames detectedGames;
	for (uint i = 0; i < matches.size(); i++) {
		DetectedGame game = toDetectedGame(matches[i]);

		if (game.hasUnknownFiles && !(_flags & kADFlagCanPlayUnknownVariants)) {
			game.canBeAdded = false;
		}

		detectedGames.push_back(game);
	}

	bool foundKnownGames = false;
	for (uint i = 0; i < detectedGames.size(); i++) {
		foundKnownGames |= !detectedGames[i].hasUnknownFiles;
	}

	if (!foundKnownGames) {
		// Use fallback detector if there were no matches by other means
		ADDetectedGameExtraInfo *extraInfo = nullptr;
		ADDetectedGame fallbackDetectionResult = fallbackDetect(allFiles, fslist, &extraInfo);

		if (fallbackDetectionResult.desc) {
			DetectedGame fallbackDetectedGame = toDetectedGame(fallbackDetectionResult, extraInfo);

			if (extraInfo != nullptr) {
				// then it's our duty to free it
				delete extraInfo;
			} else {
				// don't add fallback when we are specifying the targetID
				fallbackDetectedGame.preferredTarget += "-fallback";
			}

			if (_flags & kADFlagPreferFallbackDetection) {
				// fallback detection was successful and this engine prefers
				// it over the partial matches found in the detection table
				detectedGames.clear();
			}
			detectedGames.push_back(fallbackDetectedGame);
		}
	}

	return detectedGames;
}

const ExtraGuiOptions AdvancedMetaEngineBase::getExtraGuiOptions(const Common::String &target) const {
	const ADExtraGuiOptionsMap *extraGuiOptions = getAdvancedExtraGuiOptions();
	if (!extraGuiOptions)
		return ExtraGuiOptions();

	ExtraGuiOptions options;

	// If there isn't any target specified, return all available GUI options.
	// Only used when an engine starts in order to set option defaults.
	if (target.empty()) {
		for (const ADExtraGuiOptionsMap *entry = extraGuiOptions; entry->guioFlag; ++entry)
			options.push_back(entry->option);

		return options;
	}

	// Query the GUI options
	const Common::String guiOptionsString = ConfMan.get("guioptions", target);
	const Common::String guiOptions = parseGameGUIOptions(guiOptionsString);

	// Add all the applying extra GUI options.
	for (const ADExtraGuiOptionsMap *entry = extraGuiOptions; entry->guioFlag; ++entry) {
		if (guiOptions.contains(entry->guioFlag))
			options.push_back(entry->option);
	}

	return options;
}

Common::Error AdvancedMetaEngineDetectionBase::identifyGame(DetectedGame &game, const void **descriptor) {
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

	Common::Path path;
	if (ConfMan.hasKey("path")) {
		path = ConfMan.getPath("path");
	} else {
		path = Common::Path(".");
		warning("No path was provided. Assuming the data files are in the current directory");
	}
	Common::FSNode dir(path);
	Common::FSList files;
	if (!dir.isDirectory() || !dir.getChildren(files, Common::FSNode::kListAll)) {
		warning("Game data path does not exist or is not a directory (%s)", path.toString(Common::Path::kNativeSeparator).c_str());
		return Common::kNoGameDataFoundError;
	}

	if (files.empty())
		return Common::kNoGameDataFoundError;

	// Sometimes this method is called directly, so we have to build the maps, especially
	// the _directoryGlobsMap
	preprocessDescriptions();

	// Compose a hashmap of all files in fslist.
	FileMap allFiles;
	composeFileHashMap(allFiles, files, (_maxScanDepth == 0 ? 1 : _maxScanDepth));

	// Clear md5 cache before each detection starts, just in case.
	ADCacheMan.clear();

	// Run the detector on this
	ADDetectedGames matches = detectGame(files.begin()->getParent(), allFiles, language, platform, extra);

	if (cleanupPirated(matches))
		return Common::kNoGameDataFoundError;

	ADDetectedGame agdDesc;
	for (uint i = 0; i < matches.size(); i++) {
		if (matches[i].desc->gameId == gameid && (!matches[i].hasUnknownFiles || (_flags & kADFlagCanPlayUnknownVariants))) {
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

	// Detection is done, no need to keep archives in memory anymore
	ADCacheMan.clearArchives();

	if (!agdDesc.desc)
		return Common::kNoGameDataFoundError;

	// We can't add search directories from AdvancedMetaEngine
	if (_flags & kADFlagMatchFullPaths) {
		Common::StringArray dirs = getPathsFromEntry(agdDesc.desc);
		Common::FSNode gameDataDir = Common::FSNode(ConfMan.getPath("path"));

		for (auto d = dirs.begin(); d != dirs.end(); ++d)
			SearchMan.addSubDirectoryMatching(gameDataDir, *d, 0, _fullPathGlobsDepth);
	}

	game = toDetectedGame(agdDesc);
	*descriptor = agdDesc.desc;

	return Common::kNoError;
}

void AdvancedMetaEngineDetectionBase::composeFileHashMap(FileMap &allFiles, const Common::FSList &fslist, int depth, const Common::Path &parentName) const {
	if (depth <= 0)
		return;

	if (fslist.empty())
		return;

	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		Common::String efname = Common::punycode_encodefilename(file->getName());
		Common::Path tstr = ((_flags & kADFlagMatchFullPaths) ? parentName : Common::Path()).appendComponent(efname);

		if (file->isDirectory()) {
			if (!_globsMap.contains(efname))
				continue;

			Common::FSList files;
			if (!file->getChildren(files, Common::FSNode::kListAll))
				continue;

			composeFileHashMap(allFiles, files, depth - 1, tstr);
			continue;
		}

		// Strip any trailing dot
		if (efname.lastChar() == '.') {
			efname.deleteLastChar();
			tstr = ((_flags & kADFlagMatchFullPaths) ? parentName : Common::Path()).appendComponent(efname);
		}

		debugC(9, kDebugGlobalDetection, "$$ ['%s'] ['%s'] in '%s", tstr.toString().c_str(), efname.c_str(), firstPathComponents(fslist.front().getPath().toString(), '/').c_str());

		allFiles[tstr] = *file;		// Record the presence of this file
		allFiles[Common::Path(efname, Common::Path::kNoSeparator)] = *file;	// ...and its file name
	}
}

/* Singleton Cache Storage for MD5 */

namespace Common {
	DECLARE_SINGLETON(AdvancedDetectorCacheManager);
}


static MD5Properties gameFileToMD5Props(const ADGameFileDescription *fileEntry, uint32 gameFlags) {
	MD5Properties ret = kMD5Head;
	if (fileEntry && fileEntry->md5 && strchr(fileEntry->md5, ':')) {
		const char *ptr;
		for (ptr = fileEntry->md5; *ptr != ':'; ptr++)
			switch (*ptr) {
			case 'r':
				ret = (MD5Properties)(ret | kMD5MacResFork);
				break;
			case 'd':
				ret = (MD5Properties)(ret | kMD5MacDataFork);
				break;
			case 't':
				ret = (MD5Properties)(ret | kMD5Tail);
				break;
			case 'A':
				ret = (MD5Properties)(ret | kMD5Archive);
			}
		return ret;
	}

	if (gameFlags & ADGF_MACRESFORK) {
		ret = (MD5Properties)(ret | kMD5MacResOrDataFork);
	}

	if (gameFlags & ADGF_TAILMD5) {
		ret = (MD5Properties)(ret | kMD5Tail);
	}

	return ret;
}

Common::String md5PropToGameFile(MD5Properties flags) {
	Common::String res;

	switch (flags & kMD5MacMask) {
	case kMD5MacDataFork:
		res = "d";
		break;

	case kMD5MacResOrDataFork:
		res = "m";
		break;

	case kMD5MacResFork:
		res = "r";
		break;

	default:
		break;
    }

	if (flags & kMD5Tail)
		res += "t";

	if (flags & kMD5Archive)
		res += "A";

	return res;
}

static bool getFilePropertiesIntern(uint md5Bytes, const AdvancedMetaEngineBase::FileMap &allFiles, MD5Properties md5prop, const Common::Path &fname, FileProperties &fileProps);

bool AdvancedMetaEngineDetectionBase::getFileProperties(const FileMap &allFiles, MD5Properties md5prop, const Common::Path &fname, FileProperties &fileProps) const {
	Common::String hashname = md5PropToCachePrefix(md5prop);
		hashname += ':';
		hashname += fname.toString('/');
		hashname += ':';
		hashname += Common::String::format("%d", _md5Bytes);

	if (ADCacheMan.containsMD5(hashname)) {
		fileProps.md5 = ADCacheMan.getMD5(hashname);
		fileProps.size = ADCacheMan.getSize(hashname);
		return true;
	}

	bool res = getFilePropertiesIntern(_md5Bytes, allFiles, md5prop, fname, fileProps);

	if (res) {
		ADCacheMan.setMD5(hashname, fileProps.md5);
		ADCacheMan.setSize(hashname, fileProps.size);
	}

	return res;
}

bool AdvancedMetaEngineBase::getFilePropertiesExtern(uint md5Bytes, const FileMap &allFiles, MD5Properties md5prop, const Common::Path &fname, FileProperties &fileProps) const {
	return getFilePropertiesIntern(md5Bytes, allFiles, md5prop, fname, fileProps);
}

static bool getFilePropertiesIntern(uint md5Bytes, const AdvancedMetaEngineBase::FileMap &allFiles, MD5Properties md5prop, const Common::Path &fname, FileProperties &fileProps) {
	if (md5prop & (kMD5MacResFork | kMD5MacDataFork)) {
		FileMapArchive fileMapArchive(allFiles);
		bool is_legacy = ((md5prop & kMD5MacMask) == kMD5MacResOrDataFork);
		if (md5prop & kMD5MacResFork) {
			Common::MacResManager macResMan;

			if (!macResMan.open(fname, fileMapArchive))
				return false;

			fileProps.md5 = macResMan.computeResForkMD5AsString(md5Bytes, ((md5prop & kMD5Tail) != 0));
			fileProps.size = macResMan.getResForkDataSize();

			if (fileProps.size != 0) {
				fileProps.md5prop = (MD5Properties)((md5prop & kMD5Tail) | kMD5MacResFork);
				return true;
			}
		}

		if (md5prop & kMD5MacDataFork) {
			Common::SeekableReadStream *dataFork = Common::MacResManager::openFileOrDataFork(fname, fileMapArchive);
			// Logically 0-sized data fork is valid but legacy code continues fallback
			if (dataFork && (dataFork->size() || !is_legacy)) {
				fileProps.size = dataFork->size();
				fileProps.md5 = Common::computeStreamMD5AsString(*dataFork, md5Bytes);
				fileProps.md5prop = (MD5Properties)((md5prop & kMD5Tail) | kMD5MacDataFork);
				delete dataFork;
				return true;
			}
			delete dataFork;
		}

		// In modern case stop here
		if (!is_legacy)
			return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> testFile;

	if (md5prop & kMD5Archive) {
		// The desired file is inside an archive

		// First, split the file string
		Common::StringTokenizer tok(fname.toString(), ":");
		Common::String archiveType = tok.nextToken();
		Common::Path archiveName(tok.nextToken());
		Common::Path fileName(tok.nextToken());

		if (!allFiles.contains(archiveName))
			return false;

		// Check if archive has already been opened and is stored in cache
		Common::Archive *archive = ADCacheMan.getArchive(allFiles[archiveName]);

		if (!archive) {
			// Archive not in cache. Find the appropriate type based on the type string,
			// open the archive, and add it to the cache
			if (archiveType.equals("is")) {
				// InstallShield (v4 and up)
				archive = Common::makeInstallShieldArchive(allFiles[archiveName]);
				ADCacheMan.addArchive(allFiles[archiveName], archive);
				if (!archive)
					return false;
			} else if (archiveType.equals("is3")) {
				// InstallShield v3
				archive = new Common::InstallShieldV3();
				if (((Common::InstallShieldV3 *)archive)->open(allFiles[archiveName])) {
					ADCacheMan.addArchive(allFiles[archiveName], archive);
				} else {
					delete archive;
					return false;
				}
			} else if (archiveType.equals("clk")) {
				// Clickteam
				archive = Common::ClickteamInstaller::open(allFiles[archiveName]);
				ADCacheMan.addArchive(allFiles[archiveName], archive);
				if (!archive)
					return false;
			} else {
				debugC(3, kDebugGlobalDetection, "WARNING: Archive type string '%s' not recognized", archiveType.c_str());
				return false;
			}
		}

		// Look for file with matching name inside the archive
		testFile.reset(archive->createReadStreamForMember(fileName));
		if (!testFile) {
			return false;
		}
	} else {
		if (!allFiles.contains(fname))
			return false;

		testFile.reset(new Common::File());
		if (!((Common::File *)testFile.get())->open(allFiles[fname]))
			return false;
	}

	if (md5prop & kMD5Tail) {
		if (testFile->size() > md5Bytes)
			testFile->seek(-(int64)md5Bytes, SEEK_END);
	}

	fileProps.size = testFile->size();
	fileProps.md5 = Common::computeStreamMD5AsString(*testFile.get(), md5Bytes);
	fileProps.md5prop = (MD5Properties) (md5prop & kMD5Tail);
	return true;
}

// Add backslash before double quotes (") and backslashes themselves (\)
Common::String escapeString(const char *string) {
	if (string == nullptr)
		return "";

	Common::String res = "";

	for (int i = 0; string[i] != '\0'; i++) {
		if (string[i] == '"' || string[i] == '\\')
			res += "\\";

		res += string[i];
	}

	return res;
}

void AdvancedMetaEngineDetectionBase::dumpDetectionEntries() const {
	const byte *descPtr;

	for (descPtr = _gameDescriptors; ((const ADGameDescription *)descPtr)->gameId != nullptr; descPtr += _descItemSize) {
		auto g = ((const ADGameDescription *)descPtr);
		const PlainGameDescriptor *gameDesc = findPlainGameDescriptor(g->gameId, _gameIds);
		const char *title = "";
		if (gameDesc != 0)
			title = gameDesc->description;

		printf("game (\n");
		printf("\tname \"%s\"\n", escapeString(g->gameId).c_str());
		printf("\ttitle \"%s\"\n", escapeString(title).c_str());
		printf("\textra \"%s\"\n", escapeString(g->extra).c_str());
		printf("\tlanguage \"%s\"\n", escapeString(getLanguageLocale(g->language)).c_str());
		printf("\tplatform \"%s\"\n", escapeString(getPlatformCode(g->platform)).c_str());
		printf("\tsourcefile \"%s\"\n", escapeString(getName()).c_str());
		printf("\tengine \"%s\"\n", escapeString(getEngineName()).c_str());

		for (auto fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			const char *fname = fileDesc->fileName;
			int64 fsize = fileDesc->fileSize == AD_NO_SIZE ? -1 : fileDesc->fileSize;
			Common::String md5 = fileDesc->md5;
			MD5Properties md5prop = gameFileToMD5Props(fileDesc, g->flags);
			Common::String md5Prefix = md5PropToGameFile(md5prop);
			Common::String key = md5;
			if (md5Prefix != "" && md5.find(':') == Common::String::npos)
				key = md5Prefix + ':' + md5;

			printf("\trom ( name \"%s\" size %lld md5-%d %s )\n", escapeString(fname).c_str(), static_cast<long long int>(fsize), _md5Bytes, key.c_str());
		}
		printf(")\n\n"); // Closing for 'game ('
	}
}

ADDetectedGames AdvancedMetaEngineDetectionBase::detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra, uint32 skipADFlags, bool skipIncomplete) {
	CachedPropertiesMap filesProps;
	ADDetectedGames matched;

	const ADGameFileDescription *fileDesc;
	const ADGameDescription *g;
	const byte *descPtr;

	debugC(3, kDebugGlobalDetection, "Starting detection for engine '%s' in dir '%s'", getName(), parent.getPath().toString(Common::Path::kNativeSeparator).c_str());

	preprocessDescriptions();

	// Check which files are included in some ADGameDescription *and* whether
	// they are present. Compute MD5s and file sizes for the available files.
	for (descPtr = _gameDescriptors; ((const ADGameDescription *)descPtr)->gameId != nullptr; descPtr += _descItemSize) {
		g = (const ADGameDescription *)descPtr;

		for (fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			MD5Properties md5prop = gameFileToMD5Props(fileDesc, g->flags);
			Common::String fname = fileDesc->fileName;
			Common::String key = md5PropToCachePrefix(md5prop);
				key += ':';
				key += fname;

			if (filesProps.contains(key))
				continue;

			FileProperties tmp;
			if (getFileProperties(allFiles, md5prop, Common::Path(fname), tmp)) {
				debugC(3, kDebugGlobalDetection, "> '%s': '%s' %ld", key.c_str(), tmp.md5.c_str(), long(tmp.size));
			}

			// Both positive and negative results are cached to avoid
			// repeatedly checking for files.
			filesProps[key] = tmp;
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
		     && !(language == Common::EN_ANY && (g->flags & ADGF_ADDENGLISH))
		     && !(language == Common::Language::ZH_CHN && g->language == Common::Language::ZH_TWN && (_flags & kADFlagCanTranscodeTraditionalChineseToSimplified))) ||
			(platform != Common::kPlatformUnknown && g->platform != Common::kPlatformUnknown && g->platform != platform)) {
			continue;
		}

		if ((_flags & kADFlagUseExtraAsHint) && !extra.empty() && g->extra != extra)
			continue;

		if (g->flags & skipADFlags) {
			debugC(3, kDebugGlobalDetection, "Skipping unsupported target for engine '%s' for the following reasons:\n\t'%s'", g->gameId, g->extra);
			continue;
		}

		ADDetectedGame game(g);
		bool allFilesPresent = true;
		int curFilesMatched = 0;

		// Try to match all files for this game
		for (fileDesc = game.desc->filesDescriptions; fileDesc->fileName; fileDesc++) {
			Common::String tstr = fileDesc->fileName;
			MD5Properties md5prop = gameFileToMD5Props(fileDesc, g->flags);
			Common::String key = md5PropToCachePrefix(md5prop);
				key += ':';
				key += tstr;

			if (!filesProps.contains(key) || filesProps[key].size == -1) {
				allFilesPresent = false;
				break;
			}

			game.matchedFiles[Common::Path(tstr)] = filesProps[key];

			if (game.hasUnknownFiles)
				continue;

			const char *md5_wo_prefix = fileDesc->md5;
			if (md5_wo_prefix && strchr(md5_wo_prefix, ':'))
				md5_wo_prefix = strchr(md5_wo_prefix, ':') + 1;

			if (md5_wo_prefix != nullptr && md5_wo_prefix != filesProps[key].md5) {
				debugC(3, kDebugGlobalDetection, "MD5 Mismatch. Skipping (%s) (%s)", fileDesc->md5, filesProps[key].md5.c_str());
				game.hasUnknownFiles = true;
				continue;
			}

			if (fileDesc->fileSize != AD_NO_SIZE && fileDesc->fileSize != filesProps[key].size) {
				debugC(3, kDebugGlobalDetection, "Size Mismatch. Skipping (%u) (%ld)", fileDesc->fileSize, long(filesProps[key].size));
				game.hasUnknownFiles = true;
				continue;
			}

			debugC(3, kDebugGlobalDetection, "Matched file: %s", tstr.c_str());
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
			// Do sanity check
			if (game.hasUnknownFiles && (skipIncomplete || isEntryGrayListed(g))) {
				debugC(3, kDebugGlobalDetection, "Skipping game: %s (%s %s/%s) (%d), %s %s", g->gameId, g->extra, getPlatformDescription(g->platform),
					getLanguageDescription(g->language), i, skipIncomplete ? "(ignore incomplete matches)" : "", isEntryGrayListed(g) ? "(didn't pass sanity test)" : "");
				continue;
			}

			if (matched.empty() || strcmp(matched.back().desc->gameId, g->gameId) != 0)
				matched.push_back(game);
		}

		if (allFilesPresent && !game.hasUnknownFiles) {
			debugC(2, kDebugGlobalDetection, "Found game: %s (%s %s/%s) (%d)", g->gameId, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);

			if (curFilesMatched > maxFilesMatched) {
				debugC(2, kDebugGlobalDetection, " ... new best match, removing all previous candidates");
				maxFilesMatched = curFilesMatched;

				matched.clear();	// Remove any prior, lower ranked matches.
				matched.push_back(game);
			} else if (curFilesMatched == maxFilesMatched) {
				matched.push_back(game);
			} else {
				debugC(2, kDebugGlobalDetection, " ... skipped");
			}

			gotAnyMatchesWithAllFiles = true;
		} else {
			debugC(7, kDebugGlobalDetection, "Skipping game: %s (%s %s/%s) (%d)", g->gameId, g->extra,
			 getPlatformDescription(g->platform), getLanguageDescription(g->language), i);
		}
	}

	debugC(2, "Totally found %d matches", matched.size());

	return matched;
}

ADDetectedGame AdvancedMetaEngineDetectionBase::detectGameFilebased(const FileMap &allFiles, const ADFileBasedFallback *fileBasedFallback) const {
	const ADFileBasedFallback *ptr;
	const char* const* filenames;

	int maxNumMatchedFiles = 0;
	ADDetectedGame result;

	for (ptr = fileBasedFallback; ptr->desc; ++ptr) {
		const ADGameDescription *agdesc = ptr->desc;
		int numMatchedFiles = 0;
		bool fileMissing = false;

		for (filenames = ptr->filenames; *filenames; ++filenames) {
			debugC(3, kDebugGlobalDetection, "++ %s", *filenames);
			if (!allFiles.contains(*filenames)) {
				fileMissing = true;
				break;
			}

			numMatchedFiles++;
		}

		if (!fileMissing) {
			debugC(4, kDebugGlobalDetection, "Matched: %s", agdesc->gameId);

			if (numMatchedFiles > maxNumMatchedFiles) {
				MD5Properties md5prop = gameFileToMD5Props(nullptr, agdesc->flags);
				maxNumMatchedFiles = numMatchedFiles;

				debugC(4, kDebugGlobalDetection, "and overridden");

				ADDetectedGame game(agdesc);
				game.hasUnknownFiles = true;

				for (filenames = ptr->filenames; *filenames; ++filenames) {
					FileProperties tmp;

					if (getFileProperties(allFiles, md5prop, *filenames, tmp))
						game.matchedFiles[*filenames] = tmp;
				}

				result = game;
			}
		}
	}

	return result;
}

PlainGameList AdvancedMetaEngineDetectionBase::getSupportedGames() const {
	return PlainGameList(_gameIds);
}

PlainGameDescriptor AdvancedMetaEngineDetectionBase::findGame(const char *gameId) const {
	// First search the list of supported gameids for a match.
	const PlainGameDescriptor *g = findPlainGameDescriptor(gameId, _gameIds);
	if (g)
		return *g;

	// No match found
	return PlainGameDescriptor::empty();
}

static const char *const grayList[] = {
	"game.exe",
	"demo.exe",
	"game",
	"gamedata",
	"demo",
	"demo.ini",
	"data",
	"data.z",
	"data.cab",
	"data1.hdr",
	"data1.cab",
	"data2.cab",
	"data3.cab",
	"data4.cab",
	"data5.cab",
	"Double-click me",
	"engine.exe",
	"install.exe",
	"play.exe",
	"start.exe",
	"item.dat",
	"abc.exe",
	"ppc",
	"www.exe",
	"shared.dir",
	"shared.dxr",
	"SHARDCST.MMM",
	0
};

AdvancedMetaEngineDetectionBase::AdvancedMetaEngineDetectionBase(const void *descs, uint descItemSize, const PlainGameDescriptor *gameIds)
	: _gameDescriptors((const byte *)descs), _descItemSize(descItemSize), _gameIds(gameIds) {

	_md5Bytes = 5000;
	_flags = 0;
	_guiOptions = GUIO_NONE;
	_maxScanDepth = 1;
	_directoryGlobs = NULL;
	_maxAutogenLength = 15;
	_fullPathGlobsDepth = 5;

	_hashMapsInited = false;

	for (auto f = grayList; *f; f++)
		_grayListMap.setVal(*f, true);
}

void AdvancedMetaEngineDetectionBase::preprocessDescriptions() {
	if (_hashMapsInited)
		return;

	_hashMapsInited = true;

	// Put all directory globs into a hashmap for faster usage
	if (_directoryGlobs) {
		for (auto glob = _directoryGlobs; *glob; glob++)
			_globsMap.setVal(*glob, true);
	}

	// Now scan all detection entries
	for (const byte *descPtr = _gameDescriptors; ((const ADGameDescription *)descPtr)->gameId != nullptr; descPtr += _descItemSize) {
		const ADGameDescription *g = (const ADGameDescription *)descPtr;

		// Scan for potential directory globs
		for (const ADGameFileDescription *fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
			if (strchr(fileDesc->fileName, '/')) {
				if (!(_flags & kADFlagMatchFullPaths))
					warning("Path component detected in entry for '%s:%s' but no kADFlagMatchFullPaths is set",
						getName(), g->gameId);

				Common::StringTokenizer tok(fileDesc->fileName, "/");

				uint32 depth = 0;

				while (!tok.empty()) {
					Common::String component = tok.nextToken();

					if (!tok.empty() && !_globsMap.contains(component.c_str())) { // If it is not the last component
						_globsMap.setVal(component, true);
						debugC(4, kDebugGlobalDetection, "  Added '%s' to globs", component.c_str());
					}

					depth++;
				}

				if (depth > _maxScanDepth) {
					_maxScanDepth = depth;

					debugC(4, kDebugGlobalDetection, "  Increased scan depth to %d", _maxScanDepth);
				}
			}
		}

		// Check if the detection entry have only files from the blacklist
		if (isEntryGrayListed(g)) {
			debug(0, "WARNING: Detection entry for '%s:%s' contains only blacklisted names. Add more files to the entry (%s)",
				getName(), g->gameId, g->filesDescriptions[0].md5);
		}
	}

#ifndef RELEASE_BUILD
	// Check the provided tables for sanity
	detectClashes();
#endif
}

Common::StringArray AdvancedMetaEngineDetectionBase::getPathsFromEntry(const ADGameDescription *g) {
	Common::StringArray result;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> unique;

	for (const ADGameFileDescription *fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (!strchr(fileDesc->fileName, '/'))
			continue;

		Common::StringTokenizer tok(fileDesc->fileName, "/");

		while (!tok.empty()) {
			Common::String component = tok.nextToken();

			if (!tok.empty()) { // If it is not the last component
				if (!unique.contains(component))
					result.push_back(component);

				unique[component] = true;
			}
		}
	}

	return result;
}

bool AdvancedMetaEngineDetectionBase::isEntryGrayListed(const ADGameDescription *g) const {
	bool grayIsPresent = false, nonGrayIsPresent = false;

	for (const ADGameFileDescription *fileDesc = g->filesDescriptions; fileDesc->fileName; fileDesc++) {
		if (_grayListMap.contains(fileDesc->fileName)) {
			grayIsPresent = true;
		} else {
			nonGrayIsPresent = true;
		}
	}

	return (grayIsPresent && !nonGrayIsPresent);
}

void AdvancedMetaEngineDetectionBase::detectClashes() const {
	// First, check that we do not have duplicated entries in _gameIds
	Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> idsMap;


	for (const PlainGameDescriptor *g = _gameIds; g->gameId; g++) {
		if (idsMap.contains(g->gameId))
			debug(0, "WARNING: Detection gameId for '%s:%s' has duplicates", getName(), g->gameId);

		idsMap[g->gameId] = 0;
	}

	for (const byte *descPtr = _gameDescriptors; ((const ADGameDescription *)descPtr)->gameId != nullptr; descPtr += _descItemSize) {
		const ADGameDescription *g = (const ADGameDescription *)descPtr;

		if (!idsMap.contains(g->gameId)) {
			debug(0, "WARNING: Detection gameId for '%s:%s' is not present in gameids", getName(), g->gameId);
		} else {
			idsMap[g->gameId]++;
		}

		// Perform sanity checks for entries with files inside archives
		for (const ADGameFileDescription &fileDesc : ((const ADGameDescription *)descPtr)->filesDescriptions) {
			if (fileDesc.fileName == nullptr && fileDesc.md5 == nullptr) {
				break;
			}

			if (gameFileToMD5Props(&fileDesc, ((const ADGameDescription *)descPtr)->flags) & kMD5Archive) {
				Common::StringTokenizer tok(fileDesc.fileName, ":");
				uint numTokens = 0;

				while (!tok.empty()) {
					if (tok.nextToken().empty()) {
						break;
					}
					++numTokens;
				}

				// We need exactly three tokens: <archive type> : <archive name> : <file name>
				if (numTokens != 3) {
					debug(0, "WARNING: Detection entry '%s' for gameId '%s:%s' is invalid", fileDesc.fileName, getName(), g->gameId);
				}
			}
		}
	}

	for (auto &k : idsMap) {
		if (k._value == 0 && k._key != getName())
			debug(0, "WARNING: Detection gameId for '%s:%s' has no games in the detection table", getName(), k._key.c_str());
	}
}

Common::Error AdvancedMetaEngineBase::createInstance(OSystem *syst, Engine **engine, const DetectedGame &gameDescriptor, const void *meDescriptor) {
	assert(engine);

	const ADGameDescription *adgDesc = (const ADGameDescription *)meDescriptor;

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

	if (gameDescriptor.gameSupportLevel == kWarningGame
			&& !Engine::warnUserAboutUnsupportedGame(gameDescriptor.extra))
		return Common::kUserCanceled;

	if (gameDescriptor.gameSupportLevel == kUnsupportedGame) {
		Engine::errorUnsupportedGame(gameDescriptor.extra);
		return Common::kUserCanceled;
	}

	debug("Running %s", gameDescriptor.description.c_str());
	Common::Array<Common::Path> filenames;
	for (FilePropertiesMap::const_iterator i = gameDescriptor.matchedFiles.begin(); i != gameDescriptor.matchedFiles.end(); ++i) {
		filenames.push_back(i->_key);
	}
	Common::sort(filenames.begin(), filenames.end());
	for (uint i = 0; i < filenames.size(); ++i) {
		const FileProperties &file = gameDescriptor.matchedFiles[filenames[i]];
		debug("%s: %s, %llu bytes.", filenames[i].toString().c_str(), file.md5.c_str(), (unsigned long long)file.size);
	}
	initSubSystems(adgDesc);

	return createInstance(syst, engine, meDescriptor);
}

void AdvancedMetaEngineBase::initSubSystems(const ADGameDescription *gameDesc) const {
#ifdef ENABLE_EVENTRECORDER
	if (gameDesc) {
		g_eventRec.processGameDescription(gameDesc);
	}
#endif
}

bool AdvancedMetaEngineBase::checkExtendedSaves(MetaEngineFeature f) const {
	return (f == kSavesUseExtendedFormat) ||
		(f == kSimpleSavesNames) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}
