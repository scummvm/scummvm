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


#include "common/config-manager.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/md5.h"

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "engines/metaengine.h"

#include "agi/detection.h"
#include "agi/disk_image.h"
#include "agi/wagparser.h" // for fallback detection
#include "agi/agi.h"

static const DebugChannelDef debugFlagList[] = {
	{Agi::kDebugLevelMain, "Main", "Generic debug level"},
	{Agi::kDebugLevelResources, "Resources", "Resources debugging"},
	{Agi::kDebugLevelPictures, "Pictures", "Pictures debugging"},
	{Agi::kDebugLevelSprites, "Sprites", "Sprites debugging"},
	{Agi::kDebugLevelInventory, "Inventory", "Inventory debugging"},
	{Agi::kDebugLevelInput, "Input", "Input events debugging"},
	{Agi::kDebugLevelMenu, "Menu", "Menu debugging"},
	{Agi::kDebugLevelScripts, "Scripts", "Scripts debugging"},
	{Agi::kDebugLevelSound, "Sound", "Sound debugging"},
	{Agi::kDebugLevelText, "Text", "Text output debugging"},
	{Agi::kDebugLevelSavegame, "Savegame", "Saving & restoring game debugging"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor agiGames[] = {
	{"agi", "Sierra AGI game"},
	{"agi-fanmade", "Fanmade AGI game"},
	{"agidemo", "AGI Demo"},
	{"bc", "The Black Cauldron"},
	{"caitlyn", "Caitlyn's Destiny"},
	{"ddp", "Donald Duck's Playground"},
	{"goldrush", "Gold Rush!"},
	{"kq1", "King's Quest: Quest for the Crown"},
	{"kq2", "King's Quest II: Romancing the Throne"},
	{"kq3", "King's Quest III: To Heir Is Human"},
	{"kq4", "King's Quest IV: The Perils of Rosella"},
	{"lsl1", "Leisure Suit Larry in the Land of the Lounge Lizards"},
	{"mickey", "Mickey\'s Space Adventure"},
	{"mixedup", "Mixed-Up Mother Goose"},
	{"mh1", "Manhunter: New York"},
	{"mh2", "Manhunter 2: San Francisco"},
	{"pq1", "Police Quest: In Pursuit of the Death Angel"},
	{"serguei1", "Serguei's Destiny 1"},
	{"serguei2", "Serguei's Destiny 2"},
	{"sq0", "Space Quest 0: Replicated"},
	{"sq1", "Space Quest: Chapter I - The Sarien Encounter"},
	{"sq2", "Space Quest II: Chapter II - Vohaul's Revenge"},
	{"sqx", "Space Quest: The Lost Chapter"},
	{"tetris", "AGI Tetris"},
	{"troll", "Troll\'s Tale"},
	{"winnie", "Winnie the Pooh in the Hundred Acre Wood"},
	{"xmascard", "Xmas Card"},

	{nullptr, nullptr}
};

#include "agi/detection_tables.h"

namespace Agi {

class AgiMetaEngineDetection : public AdvancedMetaEngineDetection<AGIGameDescription> {
	mutable Common::String _gameid;
	mutable Common::String _extra;

public:
	AgiMetaEngineDetection() : AdvancedMetaEngineDetection(Agi::gameDescriptions, agiGames) {
		_guiOptions = GUIO_NOSPEECH GUIO_RENDEREGA GUIO_RENDERCGA GUIO_RENDERHERCAMBER GUIO_RENDERHERCGREEN
			GUIO_RENDERAMIGA GUIO_RENDERAPPLE2GS GUIO_RENDERATARIST GUIO_RENDERMACINTOSH;

		_maxScanDepth = 2;
		_flags = kADFlagMatchFullPaths;
	}

	const char *getName() const override {
		return "agi";
	}

	const char *getEngineName() const override {
		return "AGI preAGI + v2 + v3";
	}

	const char *getOriginalCopyright() const override {
		return "Sierra AGI Engine (C) Sierra On-Line Software";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;

	ADDetectedGames detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra, uint32 skipADFlags, bool skipIncomplete) override;

private:
	static void getPotentialDiskImages(const FileMap &allFiles, const char * const *imageExtensions, size_t extensionCount, Common::Array<Common::Path> &imageFiles);

	static ADDetectedGame detectPcDiskImageGame(const FileMap &allFiles, uint32 skipADFlags);
	static Common::String getLogDirHashFromPcDiskImageV1(Common::SeekableReadStream &stream);
	static Common::String getLogDirHashFromPcDiskImageV2001(Common::SeekableReadStream &stream);

	static ADDetectedGame detectA2DiskImageGame(const FileMap &allFiles, uint32 skipADFlags);
	static Common::String getLogDirHashFromA2DiskImage(Common::SeekableReadStream &stream);

	static Common::String getLogDirHashFromDiskImage(Common::SeekableReadStream &stream, uint32 position);
};

ADDetectedGame AgiMetaEngineDetection::fallbackDetect(const FileMap &allFilesXXX, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	typedef Common::HashMap<Common::String, int32> IntMap;
	IntMap allFiles;
	bool matchedUsingFilenames = false;
	bool matchedUsingWag = false;
	int wagFileCount = 0;
	WagFileParser wagFileParser;
	Common::FSNode wagFileNode;
	Common::String description;

	// // Set the defaults for gameid and extra
	_gameid = "agi-fanmade";
	_extra.clear();

	// Set the default values for the fallback descriptor's ADGameDescription part.
	g_fallbackDesc.desc.language = Common::UNK_LANG;
	g_fallbackDesc.desc.platform = Common::kPlatformDOS;
	g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's AGIGameDescription part.
	g_fallbackDesc.gameID = GID_FANMADE;
	g_fallbackDesc.features = GF_FANMADE;
	g_fallbackDesc.version = 0x2917;

	// First grab all filenames and at the same time count the number of *.wag files
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory()) continue;
		Common::String filename = file->getName();
		filename.toLowercase();
		allFiles[filename] = true; // Save the filename in a hash table

		if (filename.hasSuffix(".wag")) {
			// Save latest found *.wag file's path (Can be used to open the file, the name can't)
			wagFileNode = *file;
			wagFileCount++; // Count found *.wag files
		}
	}

	if (allFiles.contains("logdir") && allFiles.contains("object") &&
	        allFiles.contains("picdir") && allFiles.contains("snddir") &&
	        allFiles.contains("viewdir") && allFiles.contains("vol.0") &&
	        allFiles.contains("words.tok")) { // Check for v2

		// The default AGI interpreter version 0x2917 is okay for v2 games
		// so we don't have to change it here.
		matchedUsingFilenames = true;

		// Check for AGIPAL by checking for existence of any of the files "pal.100" - "pal.109"
		bool agipal = false;
		char agipalFile[] = "pal.xxx";
		for (uint i = 100; i <= 109; i++) {
			Common::sprintf_s(agipalFile, "pal.%d", i);
			if (allFiles.contains(agipalFile)) {
				agipal = true; // We found a file "pal.x" where 100 <= x <= 109 so it's AGIPAL
				break;
			}
		}

		if (agipal) { // Check if it is AGIPAL
			description = "Unknown v2 AGIPAL Game";
		} else { // Not AGIPAL so just plain v2
			description = "Unknown v2 Game";
		}
	} else { // Try v3
		char name[8];

		for (IntMap::const_iterator f = allFiles.begin(); f != allFiles.end(); ++f) {
			if (f->_key.hasSuffix("vol.0")) {
				memset(name, 0, 8);
				strncpy(name, f->_key.c_str(), MIN((uint)8, f->_key.size() > 5 ? f->_key.size() - 5 : f->_key.size()));

				if (allFiles.contains("object") && allFiles.contains("words.tok") &&
				        allFiles.contains(Common::String(name) + "dir")) {
					matchedUsingFilenames = true;
					description = "Unknown v3 Game";
					g_fallbackDesc.version = 0x3149; // Set the default AGI version for an AGI v3 game
					break;
				}
			}
		}
	}

	// WinAGI produces *.wag file with interpreter version, game name and other parameters.
	// If there's exactly one *.wag file and it parses successfully then we'll use its information.
	if (wagFileCount == 1 && wagFileParser.parse(wagFileNode)) {
		matchedUsingWag = true;

		const WagProperty *wagAgiVer = wagFileParser.getProperty(WagProperty::PC_INTVERSION);
		const WagProperty *wagGameID = wagFileParser.getProperty(WagProperty::PC_GAMEID);
		const WagProperty *wagGameDesc = wagFileParser.getProperty(WagProperty::PC_GAMEDESC);
		const WagProperty *wagGameVer = wagFileParser.getProperty(WagProperty::PC_GAMEVERSION);
		const WagProperty *wagGameLastEdit = wagFileParser.getProperty(WagProperty::PC_GAMELAST);

		// If there is an AGI version number in the *.wag file then let's use it
		if (wagAgiVer != nullptr && wagFileParser.checkAgiVersionProperty(*wagAgiVer)) {
			// TODO/FIXME: Check that version number is something we support before trying to use it.
			//     If the version number is unsupported then it'll get switched to 0x2917 later.
			//     But there's the possibility that file based detection has detected something else
			//     than a v2 AGI game. So there's a possibility for conflicting information.
			g_fallbackDesc.version = wagFileParser.convertToAgiVersionNumber(*wagAgiVer);
		}

		// Set gameid according to *.wag file information if it's present and it's a known value
		if (wagGameID != nullptr && findPlainGameDescriptor(wagGameID->getData(), agiGames)) {
			_gameid = wagGameID->getData();
			debug(3, "Agi::fallbackDetector: Using game id (%s) from WAG file", _gameid.c_str());
		}

		// Set game description and extra according to *.wag file information if they're present
		if (wagGameDesc != nullptr && Common::String(wagGameDesc->getData()) != "\"\"") {
			description = wagGameDesc->getData();
			debug(3, "Agi::fallbackDetector: Game description (%s) from WAG file", wagGameDesc->getData());

			// If there's game version in the *.wag file, set extra to it
			if (wagGameVer != nullptr && Common::String(wagGameVer->getData()) != "\"\"") {
				_extra = wagGameVer->getData();
				debug(3, "Agi::fallbackDetector: Game version (%s) from WAG file", wagGameVer->getData());
			}

			// If there's game last edit date in the *.wag file, add it to extra
			if (wagGameLastEdit != nullptr) {
				if (!_extra.empty())
					_extra += " ";
				_extra += wagGameLastEdit->getData();
				debug(3, "Agi::fallbackDetector: Game's last edit date (%s) from WAG file", wagGameLastEdit->getData());
			}
		}
	} else if (wagFileCount > 1) { // More than one *.wag file, confusing! So let's not use them.
		warning("More than one (%d) *.wag files found. WAG files ignored", wagFileCount);
	}

	// Check that the AGI interpreter version is a supported one
	if (!(g_fallbackDesc.version >= 0x2000 && g_fallbackDesc.version < 0x4000)) {
		warning("Unsupported AGI interpreter version 0x%x in AGI's fallback detection. Using default 0x2917", g_fallbackDesc.version);
		g_fallbackDesc.version = 0x2917;
	}

	// Set game type (v2 or v3) according to the AGI interpreter version number
	if (g_fallbackDesc.version >= 0x2000 && g_fallbackDesc.version < 0x3000)
		g_fallbackDesc.gameType = GType_V2;
	else if (g_fallbackDesc.version >= 0x3000 && g_fallbackDesc.version < 0x4000)
		g_fallbackDesc.gameType = GType_V3;

	// Check if we found a match with any of the fallback methods
	if (matchedUsingWag || matchedUsingFilenames) {
		_extra = description + (!_extra.empty() ? " " : "") + _extra; // Let's combine the description and extra

		// Override the gameid & extra values in g_fallbackDesc.desc. This only works
		// until the fallback detector is called again, and while the MetaEngine instance
		// is alive (as else the string storage is modified/deleted).
		g_fallbackDesc.desc.gameId = _gameid.c_str();
		g_fallbackDesc.desc.extra = _extra.c_str();

		Common::String fallbackWarning;

		fallbackWarning = "Your game version has been detected using fallback matching as a\n";
		fallbackWarning += Common::String::format("variant of %s (%s).\n", g_fallbackDesc.desc.gameId, g_fallbackDesc.desc.extra);
		fallbackWarning += "If this is an original and unmodified version or new made Fanmade game,\n";
		fallbackWarning += "please report any information previously printed by ScummVM to the team.\n";

		g_system->logMessage(LogMessageType::kWarning, fallbackWarning.c_str());

		return ADDetectedGame(&g_fallbackDesc.desc);
	}

	return ADDetectedGame();
}

/**
 * Detection override for handling disk images after file-based detection.
 */
ADDetectedGames AgiMetaEngineDetection::detectGame(
	const Common::FSNode &parent,
	const FileMap &allFiles,
	Common::Language language,
	Common::Platform platform,
	const Common::String &extra,
	uint32 skipADFlags,
	bool skipIncomplete) {

	// Run the file-based detection first, if it finds a match then do not search for disk images.
	ADDetectedGames matched = AdvancedMetaEngineDetection::detectGame(parent, allFiles, language, platform, extra, skipADFlags, skipIncomplete);

	// Detect games within PC disk images. This detection will find one game at most.
	if (matched.empty() &&
		(language == Common::UNK_LANG || language == Common::EN_ANY) &&
		(platform == Common::kPlatformUnknown || platform == Common::kPlatformDOS)) {
		ADDetectedGame game = detectPcDiskImageGame(allFiles, skipADFlags);
		if (game.desc != nullptr) {
			matched.push_back(game);
		}
	}

	// Detect games within Apple II disk images. This detection will find one game at most.
	if (matched.empty() &&
		(language == Common::UNK_LANG || language == Common::EN_ANY) &&
		(platform == Common::kPlatformUnknown || platform == Common::kPlatformApple2)) {
		ADDetectedGame game = detectA2DiskImageGame(allFiles, skipADFlags);
		if (game.desc != nullptr) {
			matched.push_back(game);
		}
	}

	return matched;
}

void AgiMetaEngineDetection::getPotentialDiskImages(
	const FileMap &allFiles,
	const char * const *imageExtensions,
	size_t imageExtensionCount,
	Common::Array<Common::Path> &imageFiles) {

	// build an array of files with disk image extensions
	for (FileMap::const_iterator f = allFiles.begin(); f != allFiles.end(); ++f) {
		for (size_t i = 0; i < imageExtensionCount; i++) {
			if (f->_key.baseName().hasSuffixIgnoreCase(imageExtensions[i])) {
				debug(3, "potential disk image: %s", f->_key.baseName().c_str());
				imageFiles.push_back(f->_key);
				break;
			}
		}
	}

	// sort potential image files by name
	Common::sort(imageFiles.begin(), imageFiles.end());
}

/**
 * Detects a PC Booter game by searching for 360k floppy images, reading LOGDIR,
 * hashing LOGDIR, and comparing to DOS GType_V1 entries in the detection table.
 * See AgiLoader_v1 in loader_v1.cpp for more details.
 */
ADDetectedGame AgiMetaEngineDetection::detectPcDiskImageGame(const FileMap &allFiles, uint32 skipADFlags) {
	// build array of files with pc disk image extensions
	Common::Array<Common::Path> imageFiles;
	getPotentialDiskImages(allFiles, pcDiskImageExtensions, ARRAYSIZE(pcDiskImageExtensions), imageFiles);

	// find disk one by reading potential images until a match is found
	for (const Common::Path &imageFile : imageFiles) {
		Common::SeekableReadStream *stream = openPCDiskImage(imageFile, allFiles[imageFile]);
		if (stream == nullptr) {
			continue;
		}

		// attempt to locate and hash logdir using both possible inidir disk locations
		Common::String logdirHash1 = getLogDirHashFromPcDiskImageV1(*stream);
		Common::String logdirHash2 = getLogDirHashFromPcDiskImageV2001(*stream);
		delete stream;

		if (!logdirHash1.empty()) {
			debug(3, "pc disk logdir hash: %s, %s", logdirHash1.c_str(), imageFile.baseName().c_str());
		}
		if (!logdirHash2.empty()) {
			debug(3, "pc disk logdir hash: %s, %s", logdirHash2.c_str(), imageFile.baseName().c_str());
		}

		// if logdir hash found then compare against hashes of DOS GType_V1 entries
		if (!logdirHash1.empty() || !logdirHash2.empty()) {
			for (const AGIGameDescription *game = gameDescriptions; game->desc.gameId != nullptr; game++) {
				if (game->desc.platform == Common::kPlatformDOS && game->gameType == GType_V1 && !(game->desc.flags & skipADFlags)) {
					const ADGameFileDescription *file;
					for (file = game->desc.filesDescriptions; file->fileName != nullptr; file++) {
						// select the logdir hash to use by the game's interpreter version
						Common::String &logdirHash = (game->version < 0x2001) ? logdirHash1 : logdirHash2;
						if (file->md5 != nullptr && !logdirHash.empty() && file->md5 == logdirHash) {
							debug(3, "disk image match: %s, %s, %s", game->desc.gameId, game->desc.extra, imageFile.baseName().c_str());

							// logdir hash match found
							ADDetectedGame detectedGame(&game->desc);
							FileProperties fileProps;
							fileProps.md5 = file->md5;
							fileProps.md5prop = kMD5Archive;
							fileProps.size = PC_DISK_SIZE;
							detectedGame.matchedFiles[imageFile] = fileProps;
							return detectedGame;
						}
					}
				}
			}
		}
	}

	return ADDetectedGame();
}

Common::String AgiMetaEngineDetection::getLogDirHashFromPcDiskImageV1(Common::SeekableReadStream &stream) {
	// read magic number from initdir resource header
	stream.seek(PC_INITDIR_POSITION_V1);
	uint16 magic = stream.readUint16BE();
	if (magic != 0x1234) {
		return "";
	}

	// seek to initdir entry for logdir (and skip remaining 3 bytes of header)
	stream.skip(3 + (PC_INITDIR_LOGDIR_INDEX_V1 * PC_INITDIR_ENTRY_SIZE_V1));

	// read logdir location
	byte volume = stream.readByte();
	byte head = stream.readByte();
	uint16 track = stream.readUint16LE();
	uint16 sector = stream.readUint16LE();
	uint16 offset = stream.readUint16LE();

	// logdir volume must be one
	if (volume != 1) {
		return "";
	}

	// read logdir
	uint32 logDirPosition = PC_DISK_POSITION(head, track, sector, offset);
	return getLogDirHashFromDiskImage(stream, logDirPosition);
}

Common::String AgiMetaEngineDetection::getLogDirHashFromPcDiskImageV2001(Common::SeekableReadStream &stream) {
	// seek to initdir entry for logdir
	stream.seek(PC_INITDIR_POSITION_V2001 + (PC_INITDIR_LOGDIR_INDEX_V2001 * PC_INITDIR_ENTRY_SIZE_V2001));

	// read logdir location
	// volume      4 bits
	// position   12 bits  (in half-sectors)
	byte b0 = stream.readByte();
	byte b1 = stream.readByte();
	byte volume = b0 >> 4;
	uint32 position = (((b0 & 0x0f) << 8) + b1) * 256;

	// logdir volume must be one
	if (volume != 1) {
		return "";
	}

	// read logdir
	return getLogDirHashFromDiskImage(stream, position);
}

/**
 * Detects an Apple II game by searching for 140k floppy images, reading LOGDIR,
 * hashing LOGDIR, and comparing to Apple II entries in the detection table.
 * See AgiLoader_A2 in loader_a2.cpp for more details.
 */
ADDetectedGame AgiMetaEngineDetection::detectA2DiskImageGame(const FileMap &allFiles, uint32 skipADFlags) {
	// build array of files with a2 disk image extensions
	Common::Array<Common::Path> imageFiles;
	getPotentialDiskImages(allFiles, a2DiskImageExtensions, ARRAYSIZE(a2DiskImageExtensions), imageFiles);

	// find disk one by reading potential images until a match is found
	for (const Common::Path &imageFile : imageFiles) {
		// lazily-load disk image tracks as they're accessed.
		// prevents decoding entire disks just to read a few dynamic sectors.
		// this would create a significant delay for images in the .woz format.
		const bool loadAllTracks = false;
		Common::SeekableReadStream *stream = openA2DiskImage(imageFile, allFiles[imageFile], loadAllTracks);
		if (stream == nullptr) {
			warning("unable to open disk image: %s", imageFile.baseName().c_str());
			continue;
		}

		// attempt to locate and hash logdir by reading initdir,
		// and also known logdir locations for games without initdir.
		Common::String logdirHashInitdir = getLogDirHashFromA2DiskImage(*stream);
		Common::String logdirHashBc = getLogDirHashFromDiskImage(*stream, A2_BC_LOGDIR_POSITION);
		Common::String logdirHashKq2 = getLogDirHashFromDiskImage(*stream, A2_KQ2_LOGDIR_POSITION);
		delete stream;

		if (!logdirHashInitdir.empty()) {
			debug(3, "disk image logdir hash: %s, %s", logdirHashInitdir.c_str(), imageFile.baseName().c_str());
		}
		if (!logdirHashBc.empty()) {
			debug(3, "disk image logdir hash: %s, %s", logdirHashBc.c_str(), imageFile.baseName().c_str());
		}
		if (!logdirHashKq2.empty()) {
			debug(3, "disk image logdir hash: %s, %s", logdirHashKq2.c_str(), imageFile.baseName().c_str());
		}

		// if logdir hash found then compare against hashes of Apple II entries
		if (!logdirHashInitdir.empty() || !logdirHashBc.empty() || !logdirHashKq2.empty()) {
			for (const AGIGameDescription *game = gameDescriptions; game->desc.gameId != nullptr; game++) {
				if (game->desc.platform == Common::kPlatformApple2 && !(game->desc.flags & skipADFlags)) {
					const ADGameFileDescription *file;
					for (file = game->desc.filesDescriptions; file->fileName != nullptr; file++) {
						// select the logdir hash to use
						Common::String &logdirHash = (game->gameID == GID_BC)  ? logdirHashBc :
						                             (game->gameID == GID_KQ2) ? logdirHashKq2 :
						                             logdirHashInitdir;
						if (file->md5 != nullptr && !logdirHash.empty() && file->md5 == logdirHash) {
							debug(3, "disk image match: %s, %s, %s", game->desc.gameId, game->desc.extra, imageFile.baseName().c_str());

							// logdir hash match found
							ADDetectedGame detectedGame(&game->desc);
							FileProperties fileProps;
							fileProps.md5 = file->md5;
							fileProps.md5prop = kMD5Archive;
							fileProps.size = A2_DISK_SIZE;
							detectedGame.matchedFiles[imageFile] = fileProps;
							return detectedGame;
						}
					}
				}
			}
		}
	}

	return ADDetectedGame();
}

Common::String AgiMetaEngineDetection::getLogDirHashFromA2DiskImage(Common::SeekableReadStream &stream) {
	// read magic number from initdir resource header
	stream.seek(A2_INITDIR_POSITION);
	uint16 magic = stream.readUint16BE();
	if (magic != 0x1234) {
		return "";
	}

	// seek to initdir entry for logdir (and skip remaining 3 bytes of header)
	// also skip the one-byte volume number at the start of initdir
	stream.skip(3 + 1 + (A2_INITDIR_LOGDIR_INDEX * A2_INITDIR_ENTRY_SIZE));

	// read logdir location
	byte volume = stream.readByte();
	byte track = stream.readByte();
	byte sector = stream.readByte();
	byte offset = stream.readByte();

	// logdir volume must be one
	if (volume != 1) {
		return "";
	}

	// read logdir
	uint32 logDirPosition = A2_DISK_POSITION(track, sector, offset);
	return getLogDirHashFromDiskImage(stream, logDirPosition);
}

// this works for both pc and a2 disk images
Common::String AgiMetaEngineDetection::getLogDirHashFromDiskImage(Common::SeekableReadStream &stream, uint32 position) {
	stream.seek(position);
	uint16 magic = stream.readUint16BE();
	if (magic != 0x1234) {
		return "";
	}
	stream.skip(1); // volume
	uint16 logDirSize = stream.readUint16LE();
	if (!(stream.pos() + logDirSize <= stream.size())) {
		return "";
	}

	return Common::computeStreamMD5AsString(stream, logDirSize);
}

} // end of namespace Agi

REGISTER_PLUGIN_STATIC(AGI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Agi::AgiMetaEngineDetection);
