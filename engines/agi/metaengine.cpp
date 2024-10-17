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
#include "common/file.h"
#include "common/md5.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "agi/agi.h"
#include "agi/preagi/preagi.h"
#include "agi/preagi/mickey.h"
#include "agi/preagi/troll.h"
#include "agi/preagi/winnie.h"

#include "agi/detection.h"

namespace Agi {

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
	if (_gameDescription->desc.language != Common::UNK_LANG)
		return _gameDescription->desc.language;
	else if (ConfMan.hasKey("language"))
		return Common::parseLanguage(ConfMan.get("language"));
	else
		return Common::UNK_LANG;
}

bool AgiBase::isLanguageRTL() const {
	return getLanguage() == Common::HE_ISR;
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

void AgiBase::initVersion() {
	_gameVersion = _gameDescription->version;
}

const char *AgiBase::getDiskName(uint16 id) {
	for (int i = 0; _gameDescription->desc.filesDescriptions[i].fileName != nullptr; i++)
		if (_gameDescription->desc.filesDescriptions[i].fileType == id)
			return _gameDescription->desc.filesDescriptions[i].fileName;

	return "";
}

bool AgiBase::hasFeature(EngineFeature f) const {
	return
	    (f == kSupportsReturnToLauncher) ||
	    (f == kSupportsLoadingDuringRuntime) ||
	    (f == kSupportsSavingDuringRuntime);
}

} // End of namespace Agi

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_AMIGA_ALTERNATIVE_PALETTE,
		{
			_s("Use an alternative palette"),
			_s("Use an alternative palette, common for all Amiga games. This was the old behavior"),
			"altamigapalette",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_DISABLE_MOUSE,
		{
			_s("Mouse support"),
			_s("Enables mouse support. Allows to use mouse for movement and in game menus."),
			"mousesupport",
			true,
			0,
			0
		}
	},

	{
		GAMEOPTION_USE_HERCULES_FONT,
		{
			_s("Use Hercules hires font"),
			_s("Uses Hercules hires font, when font file is available."),
			"herculesfont",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_COMMAND_PROMPT_WINDOW,
		{
			_s("Pause when entering commands"),
			_s("Shows a command prompt window and pauses the game (like in SCI) instead of a real-time prompt."),
			"commandpromptwindow",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_APPLE2GS_ADD_SPEED_MENU,
		{
			_s("Add speed menu"),
			_s("Add game speed menu (similar to PC version)"),
			"apple2gs_speedmenu",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable copy protection"),
			_s("Enable any copy protection that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

using namespace Agi;

class AgiMetaEngine : public AdvancedMetaEngine<Agi::AGIGameDescription> {
public:
	const char *getName() const override {
		return "agi";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Agi::AGIGameDescription *gd) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	bool removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;

	bool hasFeature(MetaEngineFeature f) const override;
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

Common::Error AgiMetaEngine::createInstance(OSystem *syst, Engine **engine, const Agi::AGIGameDescription *gd) const {
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
			return Common::kUnsupportedGameidError;
		}
		break;
	case Agi::GType_V1:
	case Agi::GType_V2:
	case Agi::GType_V3:
	case Agi::GType_A2:
		*engine = new Agi::AgiEngine(syst, gd);
		break;
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
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
						Common::strcpy_s(description, "[broken saved game]");
					}
				} else {
					Common::strcpy_s(description, "[not an AGI saved game]");
				}

				delete in;

				saveList.push_back(SaveStateDescriptor(this, slotNr, description));
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool AgiMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	return g_system->getSavefileManager()->removeSavefile(fileName);
}

int AgiMetaEngine::getMaximumSaveSlot() const { return 999; }

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

			SaveStateDescriptor descriptor(this, slotNr, "[broken saved game]");
			return descriptor;
		}

		SaveStateDescriptor descriptor(this, slotNr, description);

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
			emptySave.setAutosave(true);
			emptySave.setWriteProtectedFlag(true);
		} else {
			emptySave.setWriteProtectedFlag(false);
		}
		return emptySave;
	}
}

#if PLUGIN_ENABLED_DYNAMIC(AGI)
	REGISTER_PLUGIN_DYNAMIC(AGI, PLUGIN_TYPE_ENGINE, AgiMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AGI, PLUGIN_TYPE_ENGINE, AgiMetaEngine);
#endif

namespace Agi {

bool AgiBase::canLoadGameStateCurrently(Common::U32String *msg) {
	if (getGameType() == GType_PreAGI) {
		if (msg)
			*msg = _("This game does not support loading");
		return false;
	}

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

	return false;
}

bool AgiBase::canSaveGameStateCurrently(Common::U32String *msg) {
	if (getGameType() == GType_PreAGI) {
		if (msg)
			*msg = _("This game does not support saving");
		return false;
	}

	if (getGameID() == GID_BC) // Technically in Black Cauldron we may save anytime
		return true;

	if (getFlag(VM_FLAG_MENUS_ACCESSIBLE)) {
		if (!_noSaveLoadAllowed) {
			if (!cycleInnerLoopIsActive()) {
				if (promptIsEnabled()) {
					return true;
				}
			}
		}
	}

	return false;
}

} // End of namespace Agi
