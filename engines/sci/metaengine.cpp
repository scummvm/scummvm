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

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/detection_internal.h"
#include "sci/dialogs.h"
#include "sci/engine/features.h"
#include "sci/engine/guest_additions.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {

struct DemoIdEntry {
	const char *demoId;
	const char *scummVMId;
};

static const DemoIdEntry s_demoIdTable[] = {
	{ "icedemo",   "iceman" },
	{ "eco2",      "ecoquest2" },
	{ "hoyledemo", "hoyle1" },
	{ "demo000",   "kq1sci" },
	{ "mm1",       "laurabow" },
	{ "rh",        "longbow" },
	{ "ll1",       "lsl1sci" },
	{ "ll5",       "lsl5" },
	{ "hq",        "qfg1" },	// QFG1 SCI0/EGA
	{ "hq2demo",   "qfg2" },
	{ "demo",      "rama" },
	{ "sq1demo",   "sq1sci" },
	{ nullptr,      nullptr }
};

static bool isSierraDemo(const Common::String &sierraId, uint32 resourceCount) {
	// If the game has less than the expected scripts, it's a demo
	uint32 demoThreshold = 100;
	// ...but there are some exceptions
	if (sierraId == "brain" || sierraId == "lsl1" ||
		sierraId == "mg" || sierraId == "pq" ||
		sierraId == "jones" ||
		sierraId == "cardgames" || sierraId == "solitare" ||
		sierraId == "catdate" ||
		sierraId == "hoyle4")
		demoThreshold = 40;
	if (sierraId == "hoyle3")
		demoThreshold = 45; // cnick-kq has 42 scripts. The actual hoyle 3 demo has 27.
	if (sierraId == "fp" || sierraId == "gk" || sierraId == "pq4" || sierraId == "swat")
		demoThreshold = 150;

	if (sierraId == "torin" && resourceCount) // Torin's Passage demo
		return true;

	// TODO: distinguish the full version of Phantasmagoria from the demo
	// TODO: distinguish the full version of RAMA from the demo
	// TODO: distinguish the full version of SQ6 from the demo

	return resourceCount < demoThreshold;
}

/**
 * Converts the builtin Sierra game object name to the ID we use in ScummVM
 * @param[in] sierraId		The internal `Game` object's name
 * @param[in] sciVersion    The detected SCI version
 * @param[in] resMan		The resource manager
 * @param[in] isDemo        Returns a flag if it's a demo
 * @return					The equivalent ScummVM game id
 */
static Common::String convertSierraGameId(Common::String sierraId, SciVersion sciVersion, ResourceManager &resMan, bool *isDemo) {
	// Convert the id to lower case, so that we match all upper/lower case variants.
	sierraId.toLowercase();

	Common::List<ResourceId> resources = resMan.listResources(kResourceTypeScript, -1);
	const uint32 scriptResourceCount = resources.size();
	*isDemo = isSierraDemo(sierraId, scriptResourceCount);

	if (*isDemo) {
		// Crazy Nick's Picks
		if (sierraId == "lsl1" && scriptResourceCount == 34)
			return "cnick-lsl";
		if (sierraId == "sq4" && scriptResourceCount == 34)
			return "cnick-sq";
		if (sierraId == "hoyle3" && scriptResourceCount == 42)
			return "cnick-kq";
		if (sierraId == "rh budget" && scriptResourceCount == 39)
			return "cnick-longbow";
		// TODO: cnick-laurabow (the name of the game object contains junk)

		// Handle Astrochicken 1 (SQ3) and 2 (SQ4)
		if (sierraId == "sq3" && scriptResourceCount == 20)
			return "astrochicken";
		if (sierraId == "sq4")
			return "msastrochicken";
	}

	for (const GameIdStrToEnum *cur = gameIdStrToEnum; cur->gameidStr; ++cur) {
		if (sierraId == cur->sierraIdStr) {
			// Distinguish same IDs via the SCI version
			if (cur->version != SCI_VERSION_NONE && cur->version != sciVersion)
				continue;

			return cur->gameidStr;
		}
	}

	// Check demo game IDs
	for (const DemoIdEntry *cur = s_demoIdTable; cur->demoId; ++cur) {
		if (sierraId == cur->demoId)
			return cur->scummVMId;
	}

	if (sierraId == "brain") {
		// Distinguish between Castle and Island of Dr. Brain from the SCI version.
		// Island is always SCI1.1, whereas Castle can be either SCI1 mid (Amiga) or
		// late (PC).
		return sciVersion < SCI_VERSION_1_1 ? "castlebrain" : "islandbrain";
	}

	if (sierraId == "glory") {
		// QFG1EGA, QFG1VGA, QFG3 and QFG4 all used the "glory" game ID

		// QFG4 floppy is SCI2, CD SCI2.1
		if (sciVersion >= SCI_VERSION_2)
			return "qfg4";

		// QFG1 EGA is SCI0
		if (sciVersion == SCI_VERSION_0_LATE)
			return "qfg1";

		// QFG1 VGA doesn't have view 1
		if (!resMan.testResource(ResourceId(kResourceTypeView, 1)))
			return "qfg1vga";

		// QFG4 demo has less than 50 scripts
		if (scriptResourceCount < 50)
			return "qfg4demo";

		// Otherwise it's QFG3
		return "qfg3";
	}

	// We don't recognize the game object name; assume it's a fan game.
	return "sci-fanmade";
}

} // End of namespace Sci

namespace Sci {

class SciMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "sci";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;

	SaveStateList listSaves(const char *target) const override;
	int getMaximumSaveSlot() const override;
	void removeSaveState(const char *target, int slot) const override;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const override;
	// Disable autosave (see mirrored method in sci.h for detailed explanation)
	int getAutosaveSlot() const override { return -1; }

	// A fallback detection method. This is not ideal as all detection lives in MetaEngine, but
	// here fb detection has many engine dependencies.
	ADDetectedGame fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;

	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

Common::Error SciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const GameIdStrToEnum *g = gameIdStrToEnum;
	for (; g->gameidStr; ++g) {
		if (0 == strcmp(desc->gameId, g->gameidStr)) {
#ifndef ENABLE_SCI32
			if (g->isSci32) {
				return Common::Error(Common::kUnsupportedGameidError, _s("SCI32 support not compiled in"));
			}
#endif
			*engine = new SciEngine(syst, desc, g->gameidEnum);

			// If the GUI options were updated, we catch this here and update them in the users config file transparently.
			Common::updateGameGUIOptions(customizeGuiOptions(ConfMan.getPath("path"), desc->guiOptions, g->version), getGameGUIOptionsDescriptionLanguage(desc->language));

			return Common::kNoError;
		}
	}

	return Common::kUnsupportedGameidError;
}

bool SciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportPlayTime);
}

bool SciEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

SaveStateList SciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".###";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	bool hasAutosave = false;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNr = atoi(file->c_str() + file->size() - 3);

		if (slotNr >= 0 && slotNr <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SavegameMetadata meta;
				if (!get_savegame_metadata(in, meta)) {
					// invalid
					delete in;
					continue;
				}
				SaveStateDescriptor descriptor(this, slotNr, meta.name);

				if (slotNr == 0) {
					// ScummVM auto-save slot (note however, that autosave support is currently revoked)
					hasAutosave = true;
				}

				saveList.push_back(descriptor);
				delete in;
			}
		}
	}

	if (!hasAutosave) {
		SaveStateDescriptor descriptor(this, 0, _("(Autosave)"));
		descriptor.setLocked(true);
		saveList.push_back(descriptor);
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

SaveStateDescriptor SciMetaEngine::querySaveMetaInfos(const char *target, int slotNr) const {
	const Common::String fileName = Common::String::format("%s.%03d", target, slotNr);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);
	SaveStateDescriptor descriptor(this, slotNr, "");

	if (in) {
		SavegameMetadata meta;

		if (!get_savegame_metadata(in, meta)) {
			// invalid
			delete in;

			descriptor.setDescription("*Invalid*");
			return descriptor;
		}

		descriptor.setDescription(meta.name);

		Graphics::Surface *thumbnail;
		if (!Graphics::loadThumbnail(*in, thumbnail)) {
			// invalid
			delete in;

			descriptor.setDescription("*Invalid*");
			return descriptor;
		}
		descriptor.setThumbnail(thumbnail);

		const int day = (meta.saveDate >> 24) & 0xFF;
		const int month = (meta.saveDate >> 16) & 0xFF;
		const int year = meta.saveDate & 0xFFFF;

		descriptor.setSaveDate(year, month, day);

		const int hour = (meta.saveTime >> 16) & 0xFF;
		const int minutes = (meta.saveTime >> 8) & 0xFF;

		descriptor.setSaveTime(hour, minutes);

		if (meta.version >= 34) {
			descriptor.setPlayTime(meta.playTime * 1000 / 60);
		} else {
			descriptor.setPlayTime(meta.playTime * 1000);
		}

		delete in;

		return descriptor;
	}
	// Return empty descriptor
	return descriptor;
}

int SciMetaEngine::getMaximumSaveSlot() const { return 99; }

void SciMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error SciEngine::loadGameState(int slot) {
	_gamestate->_delayedRestoreGameId = slot;
	return Common::kNoError;
}

Common::Error SciEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	const char *version = "";
	_soundCmd->pauseAll(false); // unpause music (we can't have it paused during save)
	const bool res = gamestate_save(_gamestate, slot, desc, version);
	_soundCmd->pauseAll(true); // pause music
	return res ? Common::kNoError : Common::kWritingFailed;
}

bool SciEngine::canLoadGameStateCurrently(Common::U32String *msg) {
#ifdef ENABLE_SCI32
	const Common::String &guiOptions = ConfMan.get("guioptions");
	if (getSciVersion() >= SCI_VERSION_2) {
		if (ConfMan.getBool("originalsaveload") ||
			Common::checkGameGUIOption(GUIO_NOLAUNCHLOAD, guiOptions)) {

			return false;
		}
	}
#endif

	return !_gamestate->executionStackBase;
}

bool SciEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return
		_features->canSaveFromGMM() &&
		!_gamestate->executionStackBase &&
		_guestAdditions->userHasControl();
}

} // End of namespace Sci

/**
 * External fallback detection-related code.
 */
namespace Sci {

Common::Language charToScummVMLanguage(const char c) {
	switch (c) {
	case 'F':
		return Common::FR_FRA;
	case 'S':
		return Common::ES_ESP;
	case 'I':
		return Common::IT_ITA;
	case 'G':
		return Common::DE_DEU;
	case 'J':
	case 'j':
		return Common::JA_JPN;
	case 'P':
		return Common::PT_BRA;
	default:
		return Common::UNK_LANG;
	}
}

Common::Language sciToScummVMLanguage(const int sciLanguage) {
	switch (sciLanguage) {
	case K_LANG_ENGLISH:    return Common::EN_ANY;
	case K_LANG_FRENCH:     return Common::FR_FRA;
	case K_LANG_SPANISH:    return Common::ES_ESP;
	case K_LANG_ITALIAN:    return Common::IT_ITA;
	case K_LANG_GERMAN:     return Common::DE_DEU;
	case K_LANG_JAPANESE:   return Common::JA_JPN;
	case K_LANG_PORTUGUESE: return Common::PT_BRA;
	default:                return Common::UNK_LANG;
	}
}

static char s_fallbackGameIdBuf[256];

/**
 * The fallback game descriptor used by the SCI engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector.
 */
static ADGameDescription s_fallbackDesc = {
	"",
	"",
	AD_ENTRY1(nullptr, nullptr), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_NO_FLAGS,
	GUIO3(GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MODE)
};

Common::Platform getSciFallbackDetectionPlatform(const AdvancedMetaEngineBase::FileMap &allFiles) {
	// Data1 contains both map and volume for SCI1.1+ Mac games
	if (allFiles.contains("Data1"))
		return Common::kPlatformMacintosh;

	// Determine the game platform
	// The existence of any of these files indicates an Amiga game
	if (allFiles.contains("9.pat") || allFiles.contains("spal") ||
		allFiles.contains("patch.005") || allFiles.contains("bank.001"))
		return Common::kPlatformAmiga;

	// The existence of 7.pat or patch.200 indicates a Mac game
	if (allFiles.contains("7.pat") || allFiles.contains("patch.200"))
		return Common::kPlatformMacintosh;

	// The data files for Atari ST versions are the same as their DOS counterparts

	return Common::kPlatformDOS;
}

bool necessarySciResourceFilesFound(const AdvancedMetaEngineBase::FileMap &allFiles) {
	bool foundResMap = false;
	bool foundRes000 = false;

	if (
		allFiles.contains("resource.map") ||
		allFiles.contains("resmap.000") ||
		allFiles.contains("resmap.001") ||
		// Data1 contains both map and volume for SCI1.1+ Mac games
		allFiles.contains("Data1")
	)
		foundResMap = true;

	if (
		allFiles.contains("resource.000") ||
		allFiles.contains("resource.001") ||
		allFiles.contains("ressci.000") ||
		allFiles.contains("ressci.001") ||
		// Data1 contains both map and volume for SCI1.1+ Mac games
		allFiles.contains("Data1")
	)
		foundRes000 = true;

	return foundResMap && foundRes000;
}

bool isSciCDVersion(const AdvancedMetaEngineBase::FileMap &allFiles) {
	// Determine if we got a CD version and set the CD flag accordingly, by checking for
	// resource.aud for SCI1.1 CD games, or audio001.002 for SCI1 CD games. We assume that
	// the file should be over 10MB, as it contains all the game speech and is usually
	// around 450MB+. The size check is for some floppy game versions like KQ6 floppy, which
	// also have a small resource.aud file
	if (allFiles.contains("resource.aud") || allFiles.contains("resaud.001") || allFiles.contains("audio001.002")) {
		Common::FSNode file = allFiles.contains("resource.aud") ? allFiles["resource.aud"] : (allFiles.contains("resaud.001") ? allFiles["resaud.001"] : allFiles["audio001.002"]);
		Common::SeekableReadStream *tmpStream = file.createReadStream();
		const int32 size = tmpStream->size();
		delete tmpStream;

		return size > 10 * 1024 * 1024;
	}

	return false;
}

void constructFallbackDetectionEntry(const Common::String &gameId, Common::Platform platform, SciVersion sciVersion, Common::Language language, bool hasEgaViews, bool isCD, bool isDemo) {
	Common::strlcpy(s_fallbackGameIdBuf, gameId.c_str(), sizeof(s_fallbackGameIdBuf));

	s_fallbackDesc.extra = "";
	s_fallbackDesc.language = language;
	s_fallbackDesc.flags = !isCD ? ADGF_NO_FLAGS : ADGF_CD;
	s_fallbackDesc.platform = platform;
	s_fallbackDesc.gameId = s_fallbackGameIdBuf;

	if (isDemo)
		s_fallbackDesc.flags |= ADGF_DEMO;

	if (isCD) {
		s_fallbackDesc.guiOptions = GUIO3(GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MODE);
	} else {
		s_fallbackDesc.guiOptions = GUIO4(GUIO_NOSPEECH, GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_MIDI_MODE);
	}

	// Is this an EGA version that might have a VGA pendant? Then we want
	// to mark it as such in the "extra" field.
	const bool markAsEGA = (
		hasEgaViews &&
		s_fallbackDesc.platform != Common::kPlatformAmiga &&
		sciVersion > SCI_VERSION_1_EGA_ONLY
	);

	// Fill in "extra" field
	if (gameId.hasSuffix("sci")) {
		s_fallbackDesc.extra = "SCI";

		// Differentiate EGA versions from the VGA ones, where needed
		if (markAsEGA)
			s_fallbackDesc.extra = "SCI/EGA";

		// Mark as demo.
		// Note: This overwrites the 'EGA' info, if it was previously set.
		if (isDemo)
			s_fallbackDesc.extra = "SCI/Demo";
	} else {
		if (markAsEGA)
			s_fallbackDesc.extra = "EGA";

		// Set "CD" and "Demo" as appropriate.
		// Note: This overwrites the 'EGA' info, if it was previously set.
		if (isDemo && isCD)
			s_fallbackDesc.extra = "CD Demo";
		else if (isDemo)
			s_fallbackDesc.extra = "Demo";
		else if (isCD)
			s_fallbackDesc.extra = "CD";
	}
}

ADDetectedGame SciMetaEngine::fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	// If necessary SCI files aren't found, it can't be SCI
	if (!necessarySciResourceFilesFound(allFiles))
		return ADDetectedGame();

	const bool isCD = isSciCDVersion(allFiles);
	Common::Platform platform = getSciFallbackDetectionPlatform(allFiles);

	ResourceManager resMan(true);
	resMan.addAppropriateSourcesForDetection(fslist);
	resMan.init();
	// TODO: Add error handling.
	const SciVersion sciVersion = getSciVersionForDetection();

#ifndef ENABLE_SCI32
	// Is SCI32 compiled in? If not, and this is a SCI32 game,
	// stop here
	if (sciVersion >= SCI_VERSION_2)
		return ADDetectedGame();
#endif

	const ViewType gameViews = resMan.getViewType();

	// Have we identified the game views? If not, stop here
	// Can't be SCI (or unsupported SCI views). Pinball Creep by Sierra also uses resource.map/resource.000 files
	// but doesn't share SCI format at all
	if (gameViews == kViewUnknown)
		return ADDetectedGame();

	// Set the platform to Amiga if the game is using Amiga views
	if (gameViews == kViewAmiga)
		platform = Common::kPlatformAmiga;

	// Determine the game id
	const Common::String sierraGameId = resMan.findSierraGameId();

	// If we don't have a game id, the game is not SCI
	if (sierraGameId.empty())
		return ADDetectedGame();

	bool isDemo = false;
	Common::String gameId = convertSierraGameId(sierraGameId, sciVersion, resMan, &isDemo);
	Common::Language language = Common::EN_ANY;

	// Try to determine the game language
	// Load up text 0 and start looking for "#" characters
	// Non-English versions contain strings like XXXX#YZZZZ
	// Where XXXX is the English string, #Y a separator indicating the language
	// (e.g. #G for German) and ZZZZ is the translated text
	// NOTE: This doesn't work for games which use message instead of text resources
	// (like, for example, Eco Quest 1 and all SCI1.1 games and newer, e.g. Freddy Pharkas).
	// As far as we know, these games store the messages of each language in separate
	// resources, and it's not possible to detect that easily
	// Also look for "%J" which is used in japanese games
	Resource *text = resMan.findResource(ResourceId(kResourceTypeText, 0), false);
	uint seeker = 0;
	if (text) {
		while (seeker < text->size()) {
			if (text->getUint8At(seeker) == '#')  {
				if (seeker + 1 < text->size())
					language = charToScummVMLanguage(text->getUint8At(seeker + 1));
				break;
			}
			if (text->getUint8At(seeker) == '%') {
				if ((seeker + 1 < text->size()) && (text->getUint8At(seeker + 1) == 'J')) {
					language = charToScummVMLanguage(text->getUint8At(seeker + 1));
					break;
				}
			}
			seeker++;
		}
	}

	// Try to determine the game language from config file (SCI1.1 and later)
	const char *configNames[] = { "resource.cfg", "resource.win" };
	for (int i = 0; i < ARRAYSIZE(configNames) && language == Common::EN_ANY; i++) {
		Common::File file;
		if (allFiles.contains(configNames[i]) && file.open(allFiles[configNames[i]])) {
			while (true) {
				Common::String line = file.readLine();
				if (file.eos()) {
					break;
				}
				uint32 separatorPos = line.find('=');
				if (separatorPos == Common::String::npos) {
					continue;
				}
				Common::String key = line.substr(0, separatorPos);
				key.trim();
				if (key.equalsIgnoreCase("language")) {
					Common::String val = line.substr(separatorPos + 1);
					val.trim();
					Common::Language parsedLanguage = sciToScummVMLanguage(atoi(val.c_str()));
					if (parsedLanguage != Common::UNK_LANG) {
						language = parsedLanguage;
					}
					break;
				}
			}
		}
	}

	constructFallbackDetectionEntry(gameId, platform, sciVersion, language, gameViews == kViewEga, isCD, isDemo);

	return ADDetectedGame(&s_fallbackDesc);
}

void SciMetaEngine::registerDefaultSettings(const Common::String &target) const {
	for (const ADExtraGuiOptionsMap *entry = optionsList; entry->guioFlag; ++entry)
		ConfMan.registerDefault(entry->option.configOption, entry->option.defaultState);

	for (const PopUpOptionsMap *entry = popUpOptionsList; entry->guioFlag; ++entry)
		ConfMan.registerDefault(entry->configOption, entry->defaultState);

	// enable_high_resolution_graphics is normally enabled by default,
	// except for KQ6 where it overrides the DOS platform with Windows.
	// If it were enabled by default for KQ6, then the DOS platform
	// would produce the Windows experience by default instead of DOS.
	if (ConfMan.get("gameid", target) == "kq6" && ConfMan.get("platform", target) == "pc") {
		ConfMan.registerDefault("enable_high_resolution_graphics", false);
	}
}

GUI::OptionsContainerWidget *SciMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new OptionsWidget(boss, name, target);
}

} // End of namespace Sci

#if PLUGIN_ENABLED_DYNAMIC(SCI)
	REGISTER_PLUGIN_DYNAMIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#endif
