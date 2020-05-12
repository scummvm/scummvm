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
#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "agi/agi.h"
#include "agi/preagi.h"
#include "agi/preagi_mickey.h"
#include "agi/preagi_troll.h"
#include "agi/preagi_winnie.h"
#include "agi/wagparser.h"


namespace Agi {

struct AGIGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 AgiBase::getGameID() const {
	return _gameDescription->gameID;
}

uint32 AgiBase::getFeatures() const {
	return _gameFeatures;
}

Common::Platform AgiBase::getPlatform() const {
	return _gameDescription->desc.platform;
}

Common::Language AgiBase::getLanguage() const {
	return _gameDescription->desc.language;
}

uint16 AgiBase::getVersion() const {
	return _gameVersion;
}

uint16 AgiBase::getGameType() const {
	return _gameDescription->gameType;
}

const char *AgiBase::getGameMD5() const {
	return _gameDescription->desc.filesDescriptions[0].md5;
}

void AgiBase::initFeatures() {
	_gameFeatures = _gameDescription->features;
}

void AgiBase::setFeature(uint32 feature) {
	_gameFeatures |= feature;
}

void AgiBase::setVersion(uint16 version) {
	_gameVersion = version;
}

void AgiBase::initVersion() {
	_gameVersion = _gameDescription->version;
}

const char *AgiBase::getDiskName(uint16 id) {
	for (int i = 0; _gameDescription->desc.filesDescriptions[i].fileName != NULL; i++)
		if (_gameDescription->desc.filesDescriptions[i].fileType == id)
			return _gameDescription->desc.filesDescriptions[i].fileName;

	return "";
}

}

static const PlainGameDescriptor agiGames[] = {
	{"agi", "Sierra AGI game"},
	{"agi-fanmade", "Fanmade AGI game"},
	{"agidemo", "AGI Demo"},
	{"bc", "The Black Cauldron"},
	{"caitlyn", "Caitlyn's Destiny"},
	{"ddp", "Donald Duck's Playground"},
	{"goldrush", "Gold Rush!"},
	{"kq1", "King's Quest I: Quest for the Crown"},
	{"kq2", "King's Quest II: Romancing the Throne"},
	{"kq3", "King's Quest III: To Heir Is Human"},
	{"kq4", "King's Quest IV: The Perils of Rosella"},
	{"lsl1", "Leisure Suit Larry in the Land of the Lounge Lizards"},
	{"mickey", "Mickey\'s Space Adventure"},
	{"mixedup", "Mixed-Up Mother Goose"},
	{"mh1", "Manhunter 1: New York"},
	{"mh2", "Manhunter 2: San Francisco"},
	{"pq1", "Police Quest I: In Pursuit of the Death Angel"},
	{"serguei1", "Serguei's Destiny 1"},
	{"serguei2", "Serguei's Destiny 2"},
	{"sq0", "Space Quest 0: Replicated"},
	{"sq1", "Space Quest I: The Sarien Encounter"},
	{"sq2", "Space Quest II: Vohaul's Revenge"},
	{"sqx", "Space Quest X: The Lost Chapter"},
	{"tetris", "AGI Tetris"},
	{"troll", "Troll\'s Tale"},
	{"winnie", "Winnie the Pooh in the Hundred Acre Wood"},
	{"xmascard", "Xmas Card"},

	{0, 0}
};

#include "agi/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_AMIGA_ALTERNATIVE_PALETTE,
		{
			_s("Use an alternative palette"),
			_s("Use an alternative palette, common for all Amiga games. This was the old behavior"),
			"altamigapalette",
			false
		}
	},

	{
		GAMEOPTION_DISABLE_MOUSE,
		{
			_s("Mouse support"),
			_s("Enables mouse support. Allows to use mouse for movement and in game menus."),
			"mousesupport",
			true
		}
	},

	{
		GAMEOPTION_USE_HERCULES_FONT,
		{
			_s("Use Hercules hires font"),
			_s("Uses Hercules hires font, when font file is available."),
			"herculesfont",
			false
		}
	},

	{
		GAMEOPTION_COMMAND_PROMPT_WINDOW,
		{
			_s("Pause when entering commands"),
			_s("Shows a command prompt window and pauses the game (like in SCI) instead of a real-time prompt."),
			"commandpromptwindow",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

using namespace Agi;

class AgiMetaEngine : public AdvancedMetaEngine {
	mutable Common::String _gameid;
	mutable Common::String _extra;

public:
	AgiMetaEngine() : AdvancedMetaEngine(Agi::gameDescriptions, sizeof(Agi::AGIGameDescription), agiGames, optionsList) {
		_guiOptions = GUIO1(GUIO_NOSPEECH);
	}

	const char *getEngineId() const override {
		return "agi";
	}

	const char *getName() const override {
		return "AGI preAGI + v2 + v3";
	}

	const char *getOriginalCopyright() const override {
		return "Sierra AGI Engine (C) Sierra On-Line Software";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
};

bool AgiMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate) ||
	    (f == kSavesSupportPlayTime) ||
		(f == kSimpleSavesNames);
}

bool AgiBase::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime);
}


bool AgiMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Agi::AGIGameDescription *gd = (const Agi::AGIGameDescription *)desc;
	bool res = true;

	switch (gd->gameType) {
	case Agi::GType_PreAGI:
		switch (gd->gameID) {
		case GID_MICKEY:
			*engine = new Agi::MickeyEngine(syst, gd);
			break;
		case GID_TROLL:
			*engine = new Agi::TrollEngine(syst, gd);
			break;
		case GID_WINNIE:
			*engine = new Agi::WinnieEngine(syst, gd);
			break;
		default:
			res = false;
			error("PreAGI engine: unknown gameID");
			break;
		}
		break;
	case Agi::GType_V1:
	case Agi::GType_V2:
	case Agi::GType_V3:
		*engine = new Agi::AgiEngine(syst, gd);
		break;
	default:
		res = false;
		error("AGI engine: unknown gameType");
	}

	return res;
}

SaveStateList AgiMetaEngine::listSaves(const char *target) const {
	const uint32 AGIflag = MKTAG('A', 'G', 'I', ':');
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNr = atoi(file->c_str() + file->size() - 3);

		if (slotNr >= 0 && slotNr <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				uint32 type = in->readUint32BE();
				char description[31];

				if (type == AGIflag) {
					uint16 descriptionPos = 0;

					in->read(description, 31);

					// Security-check, if saveDescription has a terminating NUL
					while (description[descriptionPos]) {
						descriptionPos++;
						if (descriptionPos >= sizeof(description))
							break;
					}
					if (descriptionPos >= sizeof(description)) {
						strcpy(description, "[broken saved game]");
					}
				} else {
					strcpy(description, "[not an AGI saved game]");
				}

				delete in;

				saveList.push_back(SaveStateDescriptor(slotNr, description));
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

int AgiMetaEngine::getMaximumSaveSlot() const { return 999; }

void AgiMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

SaveStateDescriptor AgiMetaEngine::querySaveMetaInfos(const char *target, int slotNr) const {
	const uint32 AGIflag = MKTAG('A', 'G', 'I', ':');
	Common::String fileName = Common::String::format("%s.%03d", target, slotNr);

	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		if (in->readUint32BE() != AGIflag) {
			delete in;
			return SaveStateDescriptor();
		}

		char description[31];
		uint16 descriptionPos = 0;

		in->read(description, 31);

		while (description[descriptionPos]) {
			descriptionPos++;
			if (descriptionPos >= sizeof(description))
				break;
		}
		if (descriptionPos >= sizeof(description)) {
			// broken description, ignore it
			delete in;

			SaveStateDescriptor descriptor(slotNr, "[broken saved game]");
			return descriptor;
		}

		SaveStateDescriptor descriptor(slotNr, description);

		// Do not allow save slot 0 (used for auto-saving) to be deleted or
		// overwritten.
		if (slotNr == 0) {
			descriptor.setWriteProtectedFlag(true);
			descriptor.setDeletableFlag(false);
		} else {
			descriptor.setWriteProtectedFlag(false);
			descriptor.setDeletableFlag(true);
		}

		char saveVersion = in->readByte();
		if (saveVersion >= 4) {
			Graphics::Surface *thumbnail;
			if (!Graphics::loadThumbnail(*in, thumbnail)) {
				delete in;
				return SaveStateDescriptor();
			}

			descriptor.setThumbnail(thumbnail);

			uint32 saveDate = in->readUint32BE();
			uint16 saveTime = in->readUint16BE();
			if (saveVersion >= 9) {
				in->readByte(); // skip over seconds of saveTime (not needed here)
			}
			if (saveVersion >= 6) {
				uint32 playTime = in->readUint32BE();
				descriptor.setPlayTime(playTime * 1000);
			}

			int day = (saveDate >> 24) & 0xFF;
			int month = (saveDate >> 16) & 0xFF;
			int year = saveDate & 0xFFFF;

			descriptor.setSaveDate(year, month, day);

			int hour = (saveTime >> 8) & 0xFF;
			int minutes = saveTime & 0xFF;

			descriptor.setSaveTime(hour, minutes);
		}

		delete in;

		return descriptor;

	} else {
		SaveStateDescriptor emptySave;
		// Do not allow save slot 0 (used for auto-saving) to be overwritten.
		if (slotNr == 0) {
			emptySave.setWriteProtectedFlag(true);
		} else {
			emptySave.setWriteProtectedFlag(false);
		}
		return emptySave;
	}
}

ADDetectedGame AgiMetaEngine::fallbackDetect(const FileMap &allFilesXXX, const Common::FSList &fslist) const {
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
			sprintf(agipalFile, "pal.%d", i);
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
		if (wagAgiVer != NULL && wagFileParser.checkAgiVersionProperty(*wagAgiVer)) {
			// TODO/FIXME: Check that version number is something we support before trying to use it.
			//     If the version number is unsupported then it'll get switched to 0x2917 later.
			//     But there's the possibility that file based detection has detected something else
			//     than a v2 AGI game. So there's a possibility for conflicting information.
			g_fallbackDesc.version = wagFileParser.convertToAgiVersionNumber(*wagAgiVer);
		}

		// Set gameid according to *.wag file information if it's present and it doesn't contain whitespace.
		if (wagGameID != NULL && !Common::String(wagGameID->getData()).contains(" ")) {
			_gameid = wagGameID->getData();
			debug(3, "Agi::fallbackDetector: Using game id (%s) from WAG file", _gameid.c_str());
		}

		// Set game description and extra according to *.wag file information if they're present
		if (wagGameDesc != NULL) {
			description = wagGameDesc->getData();
			debug(3, "Agi::fallbackDetector: Game description (%s) from WAG file", wagGameDesc->getData());

			// If there's game version in the *.wag file, set extra to it
			if (wagGameVer != NULL) {
				_extra = wagGameVer->getData();
				debug(3, "Agi::fallbackDetector: Game version (%s) from WAG file", wagGameVer->getData());
			}

			// If there's game last edit date in the *.wag file, add it to extra
			if (wagGameLastEdit != NULL) {
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
		fallbackWarning += "please report any, information previously printed by ScummVM to the team.\n";

		g_system->logMessage(LogMessageType::kWarning, fallbackWarning.c_str());

		return ADDetectedGame(&g_fallbackDesc.desc);
	}

	return ADDetectedGame();
}

#if PLUGIN_ENABLED_DYNAMIC(AGI)
	REGISTER_PLUGIN_DYNAMIC(AGI, PLUGIN_TYPE_ENGINE, AgiMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AGI, PLUGIN_TYPE_ENGINE, AgiMetaEngine);
#endif

namespace Agi {

bool AgiBase::canLoadGameStateCurrently() {
	if (!(getGameType() == GType_PreAGI)) {
		if (getFlag(VM_FLAG_MENUS_ACCESSIBLE)) {
			if (!_noSaveLoadAllowed) {
				if (!cycleInnerLoopIsActive()) {
					// We can't allow to restore a game, while inner loop is active
					// For example Mixed Up Mother Goose has an endless loop for user name input
					// Which means even if we abort the inner loop, the game would keep on calling
					// GetString() until something is entered. And this would of course also happen
					// right after restoring a saved game.
					return true;
				}
			}
		}
	}
	return false;
}

bool AgiBase::canSaveGameStateCurrently() {
	if (getGameID() == GID_BC) // Technically in Black Cauldron we may save anytime
		return true;

	if (!(getGameType() == GType_PreAGI)) {
		if (getFlag(VM_FLAG_MENUS_ACCESSIBLE)) {
			if (!_noSaveLoadAllowed) {
				if (!cycleInnerLoopIsActive()) {
					if (promptIsEnabled()) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

int AgiEngine::agiDetectGame() {
	int ec = errOK;

	assert(_gameDescription != NULL);

	if (getVersion() <= 0x2001) {
		_loader = new AgiLoader_v1(this);
	} else if (getVersion() <= 0x2999) {
		_loader = new AgiLoader_v2(this);
	} else {
		_loader = new AgiLoader_v3(this);
	}
	ec = _loader->detectGame();

	return ec;
}

} // End of namespace Agi
