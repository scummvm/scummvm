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

#include "engines/obsolete.h"

#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/md5.h"

#include "gui/dialog.h"
#include "gui/message.h"

#include "audio/mididrv.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "scumm/he/intern_he.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v8.h"
#include "scumm/dialogs.h"
#include "scumm/resource.h"

// Files related for detection.
#include "scumm/metaengine.h"
#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/file.h"
#include "scumm/file_nes.h"

namespace Scumm {

Common::Path ScummEngine::generateFilename(const int room) const {
	const int diskNumber = (room > 0) ? _res->_types[rtRoom][room]._roomno : 0;
	Common::String result;

	if (_game.version == 4) {
		if (room == 0 || room >= 900) {
			result = Common::String::format("%03d.lfl", room);
		} else {
			result = Common::String::format("disk%02d.lec", diskNumber);
		}
	} else {
		switch (_filenamePattern.genMethod) {
		case kGenDiskNum:
		case kGenDiskNumSteam:
			result = Common::String::format(_filenamePattern.pattern, diskNumber);
			break;

		case kGenRoomNum:
		case kGenRoomNumSteam:
			result = Common::String::format(_filenamePattern.pattern, room);
			break;

		case kGenUnchanged:
			result = _filenamePattern.pattern;
			break;

		default:
			error("generateFilename: Unsupported genMethod");
		}
	}

	return Common::Path(result, Common::Path::kNoSeparator);
}

Common::Path ScummEngine_v60he::generateFilename(const int room) const {
	Common::String result;
	char id = 0;

	switch (_filenamePattern.genMethod) {
	case kGenHEMac:
	case kGenHEMacNoParens:
	case kGenHEPC:
		if (room < 0) {
			id = '0' - room;
		} else {
			const int diskNumber = (room > 0) ? _res->_types[rtRoom][room]._roomno : 0;
			id = diskNumber + '0';
		}

		if (_filenamePattern.genMethod == kGenHEPC) {
			result = Common::String::format("%s.he%c", _filenamePattern.pattern, id);
		} else {
			if (id == '3') { // Special case for cursors.
				// For mac they're stored in game binary.
				result = _filenamePattern.pattern;
			} else {
				if (_filenamePattern.genMethod == kGenHEMac)
					result = Common::String::format("%s (%c)", _filenamePattern.pattern, id);
				else
					result = Common::String::format("%s %c", _filenamePattern.pattern, id);
			}
		}

		break;

	default:
		// Fallback to parent method.
		return ScummEngine::generateFilename(room);
	}

	return Common::Path(result, Common::Path::kNoSeparator);
}

Common::Path ScummEngine_v70he::generateFilename(const int room) const {
	Common::String result;
	char id = 0;

	Common::String bPattern = _filenamePattern.pattern;

	// Special cases for Blue's games, which share common (b) files.
	if (_game.id == GID_BIRTHDAYYELLOW || _game.id == GID_BIRTHDAYRED)
		bPattern = "Blue'sBirthday";
	else if (_game.id == GID_TREASUREHUNT)
		bPattern = "Blue'sTreasureHunt";

	switch (_filenamePattern.genMethod) {
	case kGenHEMac:
	case kGenHEMacNoParens:
	case kGenHEPC:
	case kGenHEIOS:
		if (_game.heversion >= 98 && room >= 0) {
			int disk = 0;
			if (_heV7DiskOffsets)
				disk = _heV7DiskOffsets[room];

			switch (disk) {
			case 2:
				id = 'b';
				result = bPattern + ".(b)";
				break;
			case 1:
				id = 'a';
				// Some of the newer HE games for iOS use the ".hea" suffix instead.
				if (_filenamePattern.genMethod == kGenHEIOS)
					result = Common::String::format("%s.hea", _filenamePattern.pattern);
				else
					result = Common::String::format("%s.(a)", _filenamePattern.pattern);
				break;
			default:
				id = '0';
				result = Common::String::format("%s.he0", _filenamePattern.pattern);
			}
		} else if (room < 0) {
			id = '0' - room;
		} else {
			id = (room == 0) ? '0' : '1';
		}

		if (_filenamePattern.genMethod == kGenHEPC || _filenamePattern.genMethod == kGenHEIOS) {
			if (id == '3' && _game.id == GID_MOONBASE) {
				result = Common::String::format("%s.u32", _filenamePattern.pattern);
				break;
			}

			// For HE >= 98, we already called snprintf above.
			if (_game.heversion < 98 || room < 0)
				result = Common::String::format("%s.he%c", _filenamePattern.pattern, id);
		} else {
			if (id == '3') { // Special case for cursors.
				// For mac they're stored in game binary.
				result = _filenamePattern.pattern;
			} else {
				Common::String pattern = id == 'b' ? Common::move(bPattern) : _filenamePattern.pattern;
				if (_filenamePattern.genMethod == kGenHEMac)
					result = Common::String::format("%s (%c)", pattern.c_str(), id);
				else
					result = Common::String::format("%s %c", pattern.c_str(), id);
			}
		}

		break;

	default:
		// Fallback to parent method.
		return ScummEngine_v60he::generateFilename(room);
	}

	return Common::Path(result, Common::Path::kNoSeparator);
}

bool ScummEngine::isMacM68kIMuse() const {
	return _game.platform == Common::kPlatformMacintosh && (_game.id == GID_MONKEY2 || _game.id == GID_INDY4) && !(_game.features & GF_MAC_CONTAINER);
}

} // End of namespace Scumm

#pragma mark -
#pragma mark --- Detection code ---
#pragma mark -


// Various methods to help in core detection.
#include "scumm/detection_internal.h"


#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -


using namespace Scumm;

const char *ScummMetaEngine::getName() const {
	return "scumm";
}

bool ScummMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool ScummEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions) ||
		(f == kSupportsHelp) ||
		(
			f == kSupportsChangingOptionsDuringRuntime &&
			(Common::String(_game.guioptions).contains(GAMEOPTION_AUDIO_OVERRIDE) ||
			 Common::String(_game.guioptions).contains(GAMEOPTION_NETWORK))
		) ||
		(f == kSupportsQuitDialogOverride && (gameSupportsQuitDialogOverride() || !ChainedGamesMan.empty()));
}

bool ScummEngine::gameSupportsQuitDialogOverride() const {
	bool supportsOverride = isUsingOriginalGUI();

	supportsOverride &= !(_game.platform == Common::kPlatformNES);
	supportsOverride &= !(_game.platform == Common::kPlatformSegaCD);

	return supportsOverride;
}


/**
 * Create a ScummEngine instance, based on the given detector data.
 *
 * This is heavily based on our MD5 detection scheme.
 */
Common::Error ScummMetaEngine::createInstance(OSystem *syst, Engine **engine,
	const DetectedGame &gameDescriptor, const void *metaEngineDescriptor) {
	assert(syst);
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	// We start by checking whether the specified game ID is obsolete.
	// If that is the case, we automatically upgrade the target to use
	// the correct new game ID (and platform, if specified).
	Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);

	// Fetch the list of files in the current directory.
	Common::FSList fslist;
	Common::FSNode dir(ConfMan.getPath("path"));
	if (!dir.isDirectory())
		return Common::kPathNotDirectory;
	if (!dir.getChildren(fslist, Common::FSNode::kListAll))
		return Common::kNoGameDataFoundError;

	// Invoke the detector, but fixed to the specified gameid.
	Common::List<DetectorResult> results;
	::detectGames(fslist, results, gameid);

	// Unable to locate game data.
	if (results.empty())
		return Common::kNoGameDataFoundError;

	// No unique match found. If a platform override is present, try to
	// narrow down the list a bit more.
	if (results.size() > 1 && ConfMan.hasKey("platform")) {
		Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
		Common::List<DetectorResult> tmp;

		// Copy only those candidates which match the platform setting.
		for (Common::List<DetectorResult>::iterator
		          x = results.begin(); x != results.end(); ++x) {
			if (x->game.platform == platform) {
				tmp.push_back(*x);
			}
		}

		// If we narrowed it down too much, print a warning, else use the list
		// we just computed as new candidates list.
		if (tmp.empty()) {
			warning("Engine_SCUMM_create: Game data inconsistent with platform override");
		} else {
			results = tmp;
		}
	}

	// Still no unique match found. Narrow down again excluding demos using extra
	// metadata
	if (results.size() > 1 && ConfMan.hasKey("extra")) {
		Common::String extra_cfg = ConfMan.get("extra");
		Common::List<DetectorResult> tmp;

		for (Common::List<DetectorResult>::iterator
				  x = results.begin(); x != results.end(); ++x) {

			// FIXME: there's no demo flag in confman, this is not 100% reliable
			if (Common::String(x->extra).contains("Demo") && extra_cfg.contains("Demo"))
				tmp.push_back(*x);
		}

		// If we narrowed it down too much, print a warning, else use the list
		// we just computed as new candidates list.
		if (tmp.empty()) {
			warning("Engine_SCUMM_create: Game data inconsistent with extra metadata settings");
		} else {
			results = tmp;
		}
	}

	// Still no unique match found -> print a warning.
	if (results.size() > 1)
		warning("Engine_SCUMM_create: No unique game candidate found, using first one");

	// Simply use the first match.
	DetectorResult res(*(results.begin()));
	debug(1, "Using gameid %s, variant %s, extra %s", res.game.gameid, res.game.variant, res.extra);
	debug(1, "  SCUMM version %d, HE version %d", res.game.version, res.game.heversion);

	// Print the MD5 of the game; either verbose using printf, in case of an
	// unknown MD5, or with a medium debug level in case of a known MD5 (for
	// debugging purposes).
	if (!findInMD5Table(res.md5.c_str())) {
		Common::String md5Warning;

		md5Warning = ("Your game version appears to be unknown. Please, report the following data to the\n"
		               "ScummVM team along with the name of the game you tried to add and its version,\n"
					   "language, etc.:\n");

		md5Warning += Common::String::format("  SCUMM gameid '%s', file '%s', MD5 '%s'\n\n",
				res.game.gameid,
				generateFilenameForDetection(res.fp.pattern, res.fp.genMethod, res.game.platform).c_str(),
				res.md5.c_str());

		g_system->logMessage(LogMessageType::kWarning, md5Warning.c_str());
	} else {
		debug(1, "Using MD5 '%s'", res.md5.c_str());
	}

	// We don't support the "Lite" version off puttzoo iOS because it contains
	// the full game.
	if (!strcmp(res.game.gameid, "puttzoo") && !strcmp(res.extra, "Lite")) {
		GUIErrorMessage(_("The Lite version of Putt-Putt Saves the Zoo iOS is not supported to avoid piracy.\n"
		                  "The full version is available for purchase from the iTunes Store."));
		return Common::kUnsupportedGameidError;
	}

	if (res.game.heversion != 0 && !strcmp(res.extra, "Steam")) {
		if (!strcmp(res.game.gameid, "baseball") ||
			!strcmp(res.game.gameid, "soccer") ||
			!strcmp(res.game.gameid, "baseball2001") ||
			!strcmp(res.game.gameid, "basketball") ||
			!strcmp(res.game.gameid, "football")) {
			GUI::MessageDialog dialog(_("Warning: this re-release version contains patched game scripts,\n"
										"and therefore it might crash or not work properly for the time being."));
			dialog.runModal();
		}
	}

	// If the GUI options were updated, we catch this here and update them in the users config
	// file transparently.
	Common::updateGameGUIOptions(customizeGuiOptions(res), getGameGUIOptionsDescriptionLanguage(res.language));

	// If the game was added really long ago, it may be missing its "extra"
	// field. When adding game-specific options, it may be our only way of
	// telling certain versions apart, so make sure it's updated.
	if (res.game.variant && res.game.variant[0] && !ConfMan.hasKey("extra"))
		ConfMan.setAndFlush("extra", res.game.variant);

	// Check for a user override of the platform. We allow the user to override
	// the platform, to make it possible to add games which are not yet in
	// our MD5 database but require a specific platform setting.
	// TODO: Do we really still need/want the platform override?
	if (ConfMan.hasKey("platform"))
		res.game.platform = Common::parsePlatform(ConfMan.get("platform"));

	// Language override.
	if (ConfMan.hasKey("language"))
		res.language = Common::parseLanguage(ConfMan.get("language"));

	// V3 FM-TOWNS games *always* should use the corresponding music driver,
	// anything else makes no sense for them.
	// TODO: Maybe allow the null driver, too?
	if (res.game.platform == Common::kPlatformFMTowns && res.game.version == 3)
		res.game.midi = MDT_TOWNS;
	// Finally, we have massaged the GameDescriptor to our satisfaction, and can
	// instantiate the appropriate game engine. Hooray!
	switch (res.game.version) {
	case 0:
		*engine = new ScummEngine_v0(syst, res);
		break;
	case 1:
	case 2:
		*engine = new ScummEngine_v2(syst, res);
		break;
	case 3:
		if (res.game.features & GF_OLD256)
			*engine = new ScummEngine_v3(syst, res);
		else
			*engine = new ScummEngine_v3old(syst, res);
		break;
	case 4:
		*engine = new ScummEngine_v4(syst, res);
		break;
	case 5:
		*engine = new ScummEngine_v5(syst, res);
		break;
	case 6:
		switch (res.game.heversion) {
#ifdef ENABLE_HE
		case 200:
			*engine = new ScummEngine_vCUPhe(syst, res);
			break;
		case 101:
		case 100:
			*engine = new ScummEngine_v100he(syst, res);
			break;
		case 99:
			*engine = new ScummEngine_v99he(syst, res);
			break;
		case 98:
		case 95:
			*engine = new ScummEngine_v95he(syst, res);
			break;
		case 90:
			*engine = new ScummEngine_v90he(syst, res);
			break;
		case 85:
		case 80:
			*engine = new ScummEngine_v80he(syst, res);
			break;
		case 74:
		case 73:
		case 72:
			*engine = new ScummEngine_v72he(syst, res);
			break;
		case 71:
			*engine = new ScummEngine_v71he(syst, res);
			break;
#else
		case 200:
		case 101:
		case 100:
		case 99:
		case 98:
		case 95:
		case 90:
		case 85:
		case 80:
		case 74:
		case 73:
		case 72:
		case 71:
			return Common::Error(Common::kUnsupportedGameidError, _s("HE v71+ support is not compiled in"));
#endif
		case 70:
			*engine = new ScummEngine_v70he(syst, res);
			break;
		case 62:
		case 61:
		case 60:
			*engine = new ScummEngine_v60he(syst, res);
			break;
		default:
			*engine = new ScummEngine_v6(syst, res);
		}
		break;
#ifdef ENABLE_SCUMM_7_8
	case 7:
		*engine = new ScummEngine_v7(syst, res);
		break;
	case 8:
		*engine = new ScummEngine_v8(syst, res);
		break;
#else
	case 7:
	case 8:
		return Common::Error(Common::kUnsupportedGameidError, _s("SCUMM v7-8 support is not compiled in"));
#endif
	default:
		return Common::kUnsupportedGameidError;
	}

	return Common::kNoError;
}

int ScummMetaEngine::getMaximumSaveSlot() const { return 99; }

namespace Scumm {
bool getSavegameName(Common::InSaveFile *in, Common::String &desc, int heversion);
} // End of namespace Scumm

SaveStateList ScummMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = target;
	pattern += ".s##";

	filenames = saveFileMan->listSavefiles(pattern);

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot.
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Scumm::getSavegameName(in, saveDesc, 0);	// FIXME: heversion?!?
				saveList.push_back(SaveStateDescriptor(this, slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

bool ScummMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = ScummEngine::makeSavegameName(target, slot, false);
	return g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor ScummMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String saveDesc;
	Graphics::Surface *thumbnail = nullptr;
	SaveStateMetaInfos infos;
	memset(&infos, 0, sizeof(infos));
	SaveStateMetaInfos *infoPtr = &infos;

	// FIXME: heversion?!?
	if (!ScummEngine::querySaveMetaInfos(target, slot, 0, saveDesc, thumbnail, infoPtr)) {
		return SaveStateDescriptor();
	}

	SaveStateDescriptor desc(this, slot, saveDesc);
	desc.setThumbnail(thumbnail);

	if (infoPtr) {
		int day = (infos.date >> 24) & 0xFF;
		int month = (infos.date >> 16) & 0xFF;
		int year = infos.date & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (infos.time >> 8) & 0xFF;
		int minutes = infos.time & 0xFF;

		desc.setSaveTime(hour, minutes);
		desc.setPlayTime(infos.playtime * 1000);
	}

	return desc;
}

GUI::OptionsContainerWidget *ScummMetaEngine::buildLoomOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", target));
	if (platform != Common::kPlatformUnknown && platform != Common::kPlatformDOS && platform != Common::kPlatformMacintosh)
		return nullptr;

	Common::String extra = ConfMan.get("extra", target);

	// The VGA Loom settings are only relevant for the DOS CD version, not
	// the Steam version (which is assumed to be well timed already).

	if (extra == "VGA")
		return new Scumm::LoomVgaGameOptionsWidget(boss, name, target);

	if (extra == "Steam")
		return MetaEngine::buildEngineOptionsWidget(boss, name, target);
	else if (platform == Common::kPlatformMacintosh)
		return new Scumm::LoomMonkeyMacGameOptionsWidget(boss, name, target, GID_LOOM);

	// These EGA Loom settings are only relevant for the EGA
	// version, since that is the only one that has an overture.
	return new Scumm::LoomEgaGameOptionsWidget(boss, name, target);
}

GUI::OptionsContainerWidget *ScummMetaEngine::buildMI1OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	Common::String extra = ConfMan.get("extra", target);
	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", target));

	if (platform == Common::kPlatformMacintosh && extra != "Steam")
		return new Scumm::LoomMonkeyMacGameOptionsWidget(boss, name, target, GID_MONKEY);

	if (extra != "CD" && extra != "FM-TOWNS" && extra != "SEGA")
		return nullptr;

	return new Scumm::MI1CdGameOptionsWidget(boss, name, target);
}


GUI::OptionsContainerWidget *ScummMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	Common::String gameid = ConfMan.get("gameid", target);
	Common::String extra = ConfMan.get("extra", target);

	if (gameid == "loom") {
		GUI::OptionsContainerWidget *widget = buildLoomOptionsWidget(boss, name, target);
		if (widget)
			return widget;
	} else if (gameid == "monkey") {
		GUI::OptionsContainerWidget *widget = buildMI1OptionsWidget(boss, name, target);
		if (widget)
			return widget;
	}
#ifdef USE_ENET
	else if (gameid == "football" || gameid == "baseball2001" || gameid == "football2002" ||
		gameid == "moonbase")
		return new Scumm::HENetworkGameOptionsWidget(boss, name, target, Common::move(gameid));
#endif

	const ExtraGuiOptions engineOptions = getExtraGuiOptions(target);

	if (!engineOptions.empty())
		return new Scumm::ScummGameOptionsWidget(boss, name, target, engineOptions);

	return MetaEngine::buildEngineOptionsWidget(boss, name, target);
}

static const ExtraGuiOption comiObjectLabelsOption = {
	_s("Show Object Line"),
	_s("Show the names of objects at the bottom of the screen"),
	"object_labels",
	true,
	0,
	0
};

static const ExtraGuiOption mmnesClassicPaletteOption = {
	_s("Use NES Classic Palette"),
	_s("Use a more neutral color palette that closely emulates the NES Classic"),
	"mm_nes_classic_palette",
	false,
	0,
	0
};

static const ExtraGuiOption fmtownsTrimTo200 = {
	_s("Trim FM-TOWNS games to 200 pixels height"),
	_s("Cut the extra 40 pixels at the bottom of the screen, to make it standard 200 pixels height, allowing using 'aspect ratio correction'"),
	"trim_fmtowns_to_200_pixels",
	false,
	0,
	0
};

static const ExtraGuiOption fmtownsForceHiResMode = {
	_s("Run in original 640 x 480 resolution"),
	_s("This allows more accurate pause/restart banners, but might impact performance or shader/scaler usage."),
	"force_fmtowns_hires_mode",
	false,
	0,
	0};

static const ExtraGuiOption macV3LowQualityMusic = {
	_s("Play simplified music"),
	_s("This music was intended for low-end Macs, and uses only one channel."),
	"mac_v3_low_quality_music",
	false,
	0,
	0
};

static const ExtraGuiOption smoothScrolling = {
	_s("Enable smooth scrolling"),
	_s("(instead of the normal 8-pixels steps scrolling)"),
	"smooth_scroll",
	true,
	0,
	0
};

static const ExtraGuiOption semiSmoothScrolling = {
	_s("Allow semi-smooth scrolling"),
	_s("Allow scrolling to be less smooth during the fast camera movement in the intro."),
	"semi_smooth_scroll",
	false,
	0,
	0
};

static const ExtraGuiOption enableEnhancements {
	"",
	"",
	"enhancements",
	true,
	0,
	0
};

static const ExtraGuiOption audioOverride {
	_s("Load modded audio"),
	_s("Replace music, sound effects, and speech clips with modded audio files, if available."),
	"audio_override",
	true,
	0,
	0
};

static const ExtraGuiOption enableOriginalGUI = {
	_s("Enable the original GUI and Menu"),
	_s("Allow the game to use the in-engine graphical interface and the original save/load menu. \
		Use it together with the \"Ask for confirmation on exit\" for a more complete experience."),
	"original_gui",
	true,
	0,
	0
};

static const ExtraGuiOption enableLowLatencyAudio = {
	_s("Enable low latency audio mode"),
	_s("Allows the game to use low latency audio, at the cost of sound accuracy. \
		It is recommended to enable this feature only if you incur in audio latency issues during normal gameplay."),
	"dimuse_low_latency_mode",
	false,
	0,
	0
};

static const ExtraGuiOption enableCOMISong = {
	_s("Enable the \"A Pirate I Was Meant To Be\" song"),
	_s("Enable the song at the beginning of Part 3 of the game, \"A Pirate I Was Meant To Be\", \
		which was cut in international releases. Beware though: subtitles may not be fully translated."),
	"enable_song",
	false,
	0,
	0
};

static const ExtraGuiOption enableCopyProtection = {
	_s("Enable copy protection"),
	_s("Enable any copy protection that would otherwise be bypassed by default."),
	"copy_protection",
	false,
	0,
	0
};

static const ExtraGuiOption mmDemoModeOption = {
	_s("Enable demo/kiosk mode"),
	_s("Enable demo/kiosk mode in the full retail version of Maniac Mansion."),
	"enable_demo_mode",
	false,
	0,
	0
};

const ExtraGuiOptions ScummMetaEngine::getExtraGuiOptions(const Common::String &target) const {
	ExtraGuiOptions options;
	// Query the GUI options
	const Common::String guiOptionsString = ConfMan.get("guioptions", target);
	const Common::String gameid = ConfMan.get("gameid", target);
	const Common::String extra = ConfMan.get("extra", target);
	const Common::String guiOptions = parseGameGUIOptions(guiOptionsString);
	const Common::Platform platform = Common::parsePlatform(ConfMan.get("platform", target));
	const Common::String language = ConfMan.get("language", target);

	if (target.empty() || guiOptions.contains(GAMEOPTION_ORIGINALGUI)) {
		options.push_back(enableOriginalGUI);
	}
	if (target.empty() || guiOptions.contains(GAMEOPTION_COPY_PROTECTION)) {
		options.push_back(enableCopyProtection);
	}
	if (target.empty() || guiOptions.contains(GAMEOPTION_ENHANCEMENTS)) {
		options.push_back(enableEnhancements);
	}
	if (target.empty() || guiOptions.contains(GAMEOPTION_LOWLATENCYAUDIO)) {
		options.push_back(enableLowLatencyAudio);
	}
	if (target.empty() || guiOptions.contains(GAMEOPTION_AUDIO_OVERRIDE)) {
		options.push_back(audioOverride);
	}
	if (target.empty() || gameid == "comi") {
		options.push_back(comiObjectLabelsOption);

		if (!language.equals("en")) {
			options.push_back(enableCOMISong);
		}
	}
	if (target.empty() || platform == Common::kPlatformNES) {
		options.push_back(mmnesClassicPaletteOption);
	}
	if (target.empty() || platform == Common::kPlatformFMTowns) {
		options.push_back(smoothScrolling);
		if (target.empty() || gameid == "loom")
			options.push_back(semiSmoothScrolling);
		if (guiOptions.contains(GAMEOPTION_TRIM_FMTOWNS_TO_200_PIXELS))
			options.push_back(fmtownsTrimTo200);
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
		if (platform == Common::kPlatformFMTowns && Common::parseLanguage(language) != Common::JA_JPN)
			options.push_back(fmtownsForceHiResMode);
#endif
	}
	if (target.empty() || gameid == "maniac") {
		// The kiosk demo script is in V1/V2 DOS, V2 Atari ST and V2 Amiga.
		bool isValidTarget = !extra.contains("Demo") &&
			(platform == Common::kPlatformDOS   ||
			 platform == Common::kPlatformAmiga ||
			 platform == Common::kPlatformAtariST) &&
			 !guiOptionsString.contains("lang_Italian");

		if (isValidTarget)
			options.push_back(mmDemoModeOption);
	}
	// The Steam Mac versions of Loom and Indy 3 are more akin to the VGA
	// DOS versions, and that's how ScummVM usually sees them. But that
	// rebranding does not happen until later.

	// The low quality music in Loom was probably intended for low-end
	// Macs. It plays only one channel, instead of three.

	if (target.empty() || (gameid == "indy3" && platform == Common::kPlatformMacintosh && extra != "Steam")) {
		options.push_back(macV3LowQualityMusic);
	}

	return options;
}

void ScummMetaEngine::registerDefaultSettings(const Common::String &) const {
	const ExtraGuiOptions engineOptions = getExtraGuiOptions("");
	for (uint i = 0; i < engineOptions.size(); i++) {
		if (strcmp(engineOptions[i].configOption, "enhancements") == 0)
			ConfMan.registerDefault(engineOptions[i].configOption, kEnhGameBreakingBugFixes | kEnhGrp1);
		else
			ConfMan.registerDefault(engineOptions[i].configOption, engineOptions[i].defaultState);
	}
}

Common::KeymapArray ScummMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Scumm;

	Common::KeymapArray keymaps = MetaEngine::initKeymaps(target);
	Common::String gameId = ConfMan.get("gameid", target);
	Action *act;

	if (gameId == "ft") {
		Keymap *insaneKeymap = new Keymap(Keymap::kKeymapTypeGame, insaneKeymapId, "SCUMM - Bike Fights");

		act = new Action("DOWNLEFT", _("Down Left"));
		act->setCustomEngineActionEvent(kScummActionInsaneDownLeft);
		act->addDefaultInputMapping("KP1");
		act->addDefaultInputMapping("END");
		insaneKeymap->addAction(act);

		act = new Action(kStandardActionMoveDown, _("Down"));
		act->setCustomEngineActionEvent(kScummActionInsaneDown);
		act->addDefaultInputMapping("DOWN");
		act->addDefaultInputMapping("KP2");
		act->addDefaultInputMapping("JOY_DOWN");
		insaneKeymap->addAction(act);

		act = new Action("DOWNRIGHT", _("Down Right"));
		act->setCustomEngineActionEvent(kScummActionInsaneDownRight);
		act->addDefaultInputMapping("KP3");
		act->addDefaultInputMapping("PAGEDOWN");
		insaneKeymap->addAction(act);

		act = new Action(kStandardActionMoveLeft, _("Left"));
		act->setCustomEngineActionEvent(kScummActionInsaneLeft);
		act->addDefaultInputMapping("LEFT");
		act->addDefaultInputMapping("KP4");
		act->addDefaultInputMapping("JOY_LEFT");
		insaneKeymap->addAction(act);

		act = new Action(kStandardActionMoveRight, _("Right"));
		act->setCustomEngineActionEvent(kScummActionInsaneRight);
		act->addDefaultInputMapping("RIGHT");
		act->addDefaultInputMapping("KP6");
		act->addDefaultInputMapping("JOY_RIGHT");
		insaneKeymap->addAction(act);

		act = new Action("UPLEFT", _("Up Left"));
		act->setCustomEngineActionEvent(kScummActionInsaneUpLeft);
		act->addDefaultInputMapping("KP7");
		act->addDefaultInputMapping("INSERT");
		insaneKeymap->addAction(act);

		act = new Action(kStandardActionMoveUp, _("Up"));
		act->setCustomEngineActionEvent(kScummActionInsaneUp);
		act->addDefaultInputMapping("UP");
		act->addDefaultInputMapping("KP8");
		act->addDefaultInputMapping("JOY_UP");
		insaneKeymap->addAction(act);

		act = new Action("UPRIGHT", _("Up Right"));
		act->setCustomEngineActionEvent(kScummActionInsaneUpRight);
		act->addDefaultInputMapping("KP9");
		act->addDefaultInputMapping("PAGEUP");
		insaneKeymap->addAction(act);

		act = new Action("ATTACK", _("Attack"));
		act->setCustomEngineActionEvent(kScummActionInsaneAttack);
		act->addDefaultInputMapping("RETURN");
		act->addDefaultInputMapping("JOY_A");
		insaneKeymap->addAction(act);

		act = new Action("SWITCH", _("Switch weapon"));
		act->setCustomEngineActionEvent(kScummActionInsaneSwitch);
		act->addDefaultInputMapping("TAB");
		act->addDefaultInputMapping("JOY_B");
		insaneKeymap->addAction(act);

		// TODO: Use a custom engine action here?
		act = new Action(kStandardActionSkip, _("Skip cutscene"));
		act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
		act->addDefaultInputMapping("ESCAPE");
		act->addDefaultInputMapping("JOY_Y");
		insaneKeymap->addAction(act);

		// I18N: Lets one skip the bike/car fight sequences in Full Throttle
		act = new Action("CHEAT", _("Win the bike fight cheat"));
		act->setCustomEngineActionEvent(kScummActionInsaneCheat);
		act->addDefaultInputMapping("S+v");
		insaneKeymap->addAction(act);

		keymaps.push_back(insaneKeymap);
	}

	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(SCUMM)
	REGISTER_PLUGIN_DYNAMIC(SCUMM, PLUGIN_TYPE_ENGINE, ScummMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCUMM, PLUGIN_TYPE_ENGINE, ScummMetaEngine);
#endif
