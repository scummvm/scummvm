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

#include "engines/obsolete.h"

#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/gui_options.h"
#include "common/md5.h"

#include "audio/mididrv.h"

#include "scumm/he/intern_he.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v8.h"
#include "scumm/resource.h"

// Files related for detection.
#include "scumm/metaengine.h"
#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/file.h"
#include "scumm/file_nes.h"

namespace Scumm {

Common::String ScummEngine::generateFilename(const int room) const {
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

	return result;
}

Common::String ScummEngine_v60he::generateFilename(const int room) const {
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

	return result;
}

Common::String ScummEngine_v70he::generateFilename(const int room) const {
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
				Common::String pattern = id == 'b' ? bPattern : _filenamePattern.pattern;
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

	return result;
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
    return "Scumm";
}

bool ScummMetaEngine::hasFeature(MetaEngineFeature f) const {
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

bool ScummEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}


/**
 * Create a ScummEngine instance, based on the given detector data.
 *
 * This is heavily based on our MD5 detection scheme.
 */
Common::Error ScummMetaEngine::createInstance(OSystem *syst, Engine **engine) const {
	assert(syst);
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	// We start by checking whether the specified game ID is obsolete.
	// If that is the case, we automatically upgrade the target to use
	// the correct new game ID (and platform, if specified).
	Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);

	// Fetch the list of files in the current directory.
	Common::FSList fslist;
	Common::FSNode dir(ConfMan.get("path"));
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

		md5Warning = ("Your game version appears to be unknown. If this is *NOT* a fan-modified\n"
		               "version (in particular, not a fan-made translation), please, report the\n"
		               "following data to the ScummVM team along with the name of the game you tried\n"
		               "to add and its version, language, etc.:\n");

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

	// If the GUI options were updated, we catch this here and update them in the users config
	// file transparently.
	Common::updateGameGUIOptions(res.game.guioptions, getGameGUIOptionsDescriptionLanguage(res.language));

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
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	// Sort saves based on slot number.
	Common::sort(saveList.begin(), saveList.end(), SaveStateDescriptorSlotComparator());
	return saveList;
}

void ScummMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = ScummEngine::makeSavegameName(target, slot, false);
	g_system->getSavefileManager()->removeSavefile(filename);
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

	SaveStateDescriptor desc(slot, saveDesc);

	// Do not allow save slot 0 (used for auto-saving) to be deleted or
	// overwritten.
	if (slot == 0) {
		desc.setWriteProtectedFlag(true);
		desc.setDeletableFlag(false);
	}

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

#if PLUGIN_ENABLED_DYNAMIC(SCUMM)
	REGISTER_PLUGIN_DYNAMIC(SCUMM, PLUGIN_TYPE_ENGINE, ScummMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCUMM, PLUGIN_TYPE_ENGINE, ScummMetaEngine);
#endif
