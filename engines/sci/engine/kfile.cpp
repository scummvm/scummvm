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

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/str.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/memstream.h"
#include "common/str-enc.h"

#include "gui/saveload.h"

#include "sci/sci.h"
#include "sci/engine/file.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/sound/audio.h"
#include "sci/console.h"
#ifdef ENABLE_SCI32
#include "graphics/thumbnail.h"
#include "sci/engine/guest_additions.h"
#endif
#include "sci/engine/message.h"
#include "sci/resource/resource.h"

namespace Sci {

/**
 * Writes the cwd to the supplied address and returns the address in acc.
 */
reg_t kGetCWD(EngineState *s, int argc, reg_t *argv) {
	// We do not let the scripts see the file system, instead pretending
	// we are always in the same directory.
	// TODO/FIXME: Is "/" a good value? Maybe "" or "." or "C:\" are better?
	s->_segMan->strcpy_(argv[0], "/");

	debugC(kDebugLevelFile, "kGetCWD() -> %s", "/");

	return argv[0];
}

enum {
	K_DEVICE_INFO_GET_DEVICE = 0,
	K_DEVICE_INFO_GET_CURRENT_DEVICE = 1,
	K_DEVICE_INFO_PATHS_EQUAL = 2,
	K_DEVICE_INFO_IS_FLOPPY = 3,
	K_DEVICE_INFO_GET_CONFIG_PATH = 5,
	K_DEVICE_INFO_GET_SAVECAT_NAME = 7,
	K_DEVICE_INFO_GET_SAVEFILE_NAME = 8
};

reg_t kDeviceInfo(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getGameId() == GID_FANMADE && argc == 1) {
		// WORKAROUND: The fan game script library calls kDeviceInfo with one parameter.
		// According to the scripts, it wants to call CurDevice. However, it fails to
		// provide the subop to the function.
		s->_segMan->strcpy_(argv[0], "/");
		return s->r_acc;
	}

	int mode = argv[0].toUint16();

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE: {
		Common::String input_str = s->_segMan->getString(argv[1]);

		s->_segMan->strcpy_(argv[2], "/");
		debug(3, "K_DEVICE_INFO_GET_DEVICE(%s) -> %s", input_str.c_str(), "/");
		break;
	}
	case K_DEVICE_INFO_GET_CURRENT_DEVICE:
		s->_segMan->strcpy_(argv[1], "/");
		debug(3, "K_DEVICE_INFO_GET_CURRENT_DEVICE() -> %s", "/");
		break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		Common::String path1_s = s->_segMan->getString(argv[1]);
		Common::String path2_s = s->_segMan->getString(argv[2]);
		debug(3, "K_DEVICE_INFO_PATHS_EQUAL(%s,%s)", path1_s.c_str(), path2_s.c_str());

		return make_reg(0, Common::matchString(path2_s.c_str(), path1_s.c_str(), false, "/"));
		}
		break;

	case K_DEVICE_INFO_IS_FLOPPY: {
		Common::String input_str = s->_segMan->getString(argv[1]);
		debug(3, "K_DEVICE_INFO_IS_FLOPPY(%s)", input_str.c_str());
		return NULL_REG; /* Never */
	}
	case K_DEVICE_INFO_GET_CONFIG_PATH: {
		// Early versions return drive letter, later versions a path string
		// FIXME: Implement if needed, for now return NULL_REG
		return NULL_REG;
	}
	/* SCI uses these in a less-than-portable way to delete savegames.
	** Read https://web.archive.org/web/20060913122014/http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		Common::String game_prefix = s->_segMan->getString(argv[2]);
		s->_segMan->strcpy_(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVECAT_NAME(%s) -> %s", game_prefix.c_str(), "__throwaway");
		}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		Common::String game_prefix = s->_segMan->getString(argv[2]);
		uint virtualId = argv[3].toUint16();
		s->_segMan->strcpy_(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVEFILE_NAME(%s,%d) -> %s", game_prefix.c_str(), virtualId, "__throwaway");
		if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
			error("kDeviceInfo(deleteSave): invalid savegame ID specified");
		uint savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
		Common::Array<SavegameDesc> saves;
		listSavegames(saves);
		if (findSavegame(saves, savegameId) != -1) {
			// Confirmed that this id still lives...
			Common::String filename = g_sci->getSavegameName(savegameId);
			Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
			saveFileMan->removeSavefile(filename);
		}
		break;
	}

	default:
		error("Unknown DeviceInfo() sub-command: %d", mode);
		break;
	}

	return s->r_acc;
}

reg_t kCheckFreeSpace(EngineState *s, int argc, reg_t *argv) {
	// A file path to test is also passed to this function as a separate
	// argument, but we do not actually check anything, so it is unused

	enum {
		kSaveGameSize      = 0,
		kFreeDiskSpace     = 1,
		kEnoughSpaceToSave = 2
	};

	int16 subop;
	// In SCI2.1mid, the call is moved into kFileIO and the arguments are
	// flipped
	if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
		subop = argc > 0 ? argv[0].toSint16() : 2;
	} else {
		subop = argc > 1 ? argv[1].toSint16() : 2;
	}

	switch (subop) {
	case kSaveGameSize:
		return make_reg(0, 0);

	case kFreeDiskSpace: // in KiB; up to 32MiB maximum
		return make_reg(0, 0x7fff);

	case kEnoughSpaceToSave:
		return make_reg(0, 1);

	default:
		error("kCheckFreeSpace: called with unknown sub-op %d", subop);
	}
}

reg_t kValidPath(EngineState *s, int argc, reg_t *argv) {
	Common::String path = s->_segMan->getString(argv[0]);

	debug(3, "kValidPath(%s) -> %d", path.c_str(), s->r_acc.getOffset());

	// Always return true
	return make_reg(0, 1);
}

#ifdef ENABLE_SCI32

reg_t kCD(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kCheckCD(EngineState *s, int argc, reg_t *argv) {
	// Mac interpreters would display a dialog prompting for the disc.
	//  kCheckCD took an optional second boolean parameter, which we
	//  ignore, that affected the dialog's text.
	const int16 cdNo = argc > 0 ? argv[0].toSint16() : 0;

	if (cdNo) {
		g_sci->getResMan()->findDisc(cdNo);
	}

	return make_reg(0, g_sci->getResMan()->getCurrentDiscNo());
}

reg_t kGetSavedCD(EngineState *s, int argc, reg_t *argv) {
	// Normally this code would read the CD number from the currently loaded
	// save game file, but since we don't have one of those, just return the
	// disc number from the resource manager
	return make_reg(0, g_sci->getResMan()->getCurrentDiscNo());
}

#endif

// ---- FileIO operations -----------------------------------------------------

#ifdef ENABLE_SCI32
static bool isSaveCatalogue(const Common::String &name) {
	return name == "autosave.cat" || name.hasSuffix("sg.cat");
}

// SCI32 save game scripts check for, and write directly to, the save game
// catalogue. Since ScummVM does not use these catalogues, when looking for a
// catalogue, we instead check for save games within ScummVM that are logically
// equivalent to the behaviour of SSCI.
static bool saveCatalogueExists(const Common::String &name) {
	bool exists = false;
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();

	// There will always be one save game in some games, the "new game"
	// game, which should be ignored when deciding if there are any save
	// games available
	uint numPermanentSaves;
	switch (g_sci->getGameId()) {
	case GID_TORIN:
	case GID_LSL7:
	case GID_LIGHTHOUSE:
		numPermanentSaves = 1;
		break;
	default:
		numPermanentSaves = 0;
		break;
	}

	// Torin uses autosave.cat; LSL7 uses autosvsg.cat
	if (name == "autosave.cat" || name == "autosvsg.cat") {
		exists = !saveFileMan->listSavefiles(g_sci->getSavegameName(0)).empty();
	} else {
		exists = saveFileMan->listSavefiles(g_sci->getSavegamePattern()).size() > numPermanentSaves;
	}

	return exists;
}

static Common::String getRamaSaveName(EngineState *s, const uint saveNo) {
	const reg_t catalogId = s->variables[VAR_GLOBAL][kGlobalVarRamaCatalogFile];
	if (catalogId.isNull()) {
		error("Could not find CatalogFile object to retrieve save game name");
	}

	const List *list = s->_segMan->lookupList(readSelector(s->_segMan, catalogId, SELECTOR(elements)));
	if (!list) {
		error("Could not read CatalogFile object list");
	}

	Node *node = s->_segMan->lookupNode(list->first);
	while (node) {
		const reg_t entryId = node->value;
		if (readSelectorValue(s->_segMan, entryId, SELECTOR(fileNumber)) == saveNo) {
			reg_t description = readSelector(s->_segMan, entryId, SELECTOR(description));
			if (s->_segMan->isObject(description)) {
				description = readSelector(s->_segMan, description, SELECTOR(data));
			}
			return s->_segMan->getString(description);
		}

		node = s->_segMan->lookupNode(node->succ);
	}

	error("Could not find a save name for save %u", saveNo);
}
#endif

reg_t kFileIO(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kFileIOOpen(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	kFileOpenMode mode = (kFileOpenMode)argv[1].toUint16();
	bool unwrapFilename = true;

	// SQ4 floppy prepends /\ to the filenames, QFG4 import does too.
	// Do this before the empty test to handle QFG4.
	if (name.hasPrefix("/\\")) {
		name.deleteChar(0);
		name.deleteChar(0);
	}

	if (name.empty()) {
		// Happens many times during KQ1 (e.g. when typing something)
		debugC(kDebugLevelFile, "Attempted to open a file with an empty filename");
		return SIGNAL_REG;
	}

	// SQ4 floppy attempts to update the savegame index file sq4sg.dir when
	// deleting saved games. We don't use an index file for saving or loading,
	// so just stop the game from modifying the file here in order to avoid
	// having it saved in the ScummVM save directory.
	if (name == "sq4sg.dir") {
		debugC(kDebugLevelFile, "Not opening unused file sq4sg.dir");
		return SIGNAL_REG;
	}

	// ECO2 has a print feature in the Ecorder that writes text to PRN,
	// which is a DOS alias for the printer. Ignore these attempts,
	// otherwise an actual file will be created.
	if (g_sci->getGameId() == GID_ECOQUEST2 && name == "prn") {
		return SIGNAL_REG;
	}

#ifdef ENABLE_SCI32
	// GK1, GK2, KQ7, LSL6hires, Phant1, PQ4, PQ:SWAT, and SQ6 read in
	// their game version from the VERSION file
	if (name.compareToIgnoreCase("version") == 0) {
		unwrapFilename = false;

		// LSL6hires version is in a file with an empty extension
		if (Common::File::exists(Common::Path(name + "."))) {
			name += ".";
		}
	}

	if (g_sci->getGameId() == GID_SHIVERS && name.hasSuffix(".SG")) {
		// Shivers stores the name and score of save games in separate %d.SG
		// files, which are used by the save/load screen
		if (mode == kFileOpenModeOpenOrCreate || mode == kFileOpenModeCreate) {
			// Suppress creation of the SG file, since it is not necessary
			debugC(kDebugLevelFile, "Not creating unused file %s", name.c_str());
			return SIGNAL_REG;
		} else if (mode == kFileOpenModeOpenOrFail) {
			// Create a virtual file containing the save game description
			// and current score progress, as the game scripts expect.
			int saveNo;
			sscanf(name.c_str(), "%d.SG", &saveNo);
			saveNo += kSaveIdShift;

			SavegameDesc save;
			if (!fillSavegameDesc(g_sci->getSavegameName(saveNo), save)) {
				return SIGNAL_REG;
			}

			Common::String score;
			if (!save.highScore) {
				score = Common::String::format("%u", save.lowScore);
			} else {
				score = Common::String::format("%u%03u", save.highScore, save.lowScore);
			}

			const uint nameLength = Common::strnlen(save.name, kMaxSaveNameLength);
			const uint size = nameLength + /* \r\n */ 2 + score.size();
			byte *buffer = (byte *)malloc(size);
			memcpy(buffer, save.name, nameLength);
			buffer[nameLength] = '\r';
			buffer[nameLength + 1] = '\n';
			memcpy(buffer + nameLength + 2, score.c_str(), score.size());

			const uint handle = findFreeFileHandle(s);

			s->_fileHandles[handle]._in = new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
			s->_fileHandles[handle]._out = nullptr;
			s->_fileHandles[handle]._name = "";

			return make_reg(0, handle);
		}
	} else if (g_sci->getGameId() == GID_MOTHERGOOSEHIRES && name.hasSuffix(".DTA")) {
		// MGDX stores the name and avatar ID in separate %d.DTA files, which
		// are used by the save/load screen
		if (mode == kFileOpenModeOpenOrCreate || mode == kFileOpenModeCreate) {
			// Suppress creation of the DTA file, since it is not necessary
			debugC(kDebugLevelFile, "Not creating unused file %s", name.c_str());
			return SIGNAL_REG;
		} else if (mode == kFileOpenModeOpenOrFail) {
			// Create a virtual file containing the save game description
			// and avatar ID, as the game scripts expect.
			int saveNo;

			// The 4-language release uses a slightly different filename
			// structure that includes the letter of the language at the start
			// of the filename
			const int skip = name.firstChar() < '0' || name.firstChar() > '9';

			if (sscanf(name.c_str() + skip, "%d.DTA", &saveNo) != 1) {
				warning("Could not parse game filename %s", name.c_str());
			}

			saveNo += kSaveIdShift;

			SavegameDesc save;
			if (!fillSavegameDesc(g_sci->getSavegameName(saveNo), save)) {
				return SIGNAL_REG;
			}

			const Common::String avatarId = Common::String::format("%02d", save.avatarId);
			const uint nameLength = Common::strnlen(save.name, kMaxSaveNameLength);
			const uint size = nameLength + /* \r\n */ 2 + avatarId.size() + 1;
			char *buffer = (char *)malloc(size);
			memcpy(buffer, save.name, nameLength);
			buffer[nameLength] = '\r';
			buffer[nameLength + 1] = '\n';
			memcpy(buffer + nameLength + 2, avatarId.c_str(), avatarId.size() + 1);

			const uint handle = findFreeFileHandle(s);

			s->_fileHandles[handle]._in = new Common::MemoryReadStream((byte *)buffer, size, DisposeAfterUse::YES);
			s->_fileHandles[handle]._out = nullptr;
			s->_fileHandles[handle]._name = "";

			return make_reg(0, handle);
		}
	} else if (g_sci->getGameId() == GID_KQ7) {
		// KQ7 creates a temp.tmp file to perform an atomic rewrite of the
		// catalogue, but since we do not create catalogues for most SCI32
		// games, ignore the write
		if (name == "temp.tmp") {
			return make_reg(0, kVirtualFileHandleSci32Save);
		}
	} else if (g_sci->getGameId() == GID_PQSWAT) {
		// PQSWAT tries to create subdirectories for each game profile
		for (Common::String::iterator it = name.begin(); it != name.end(); ++it) {
			if (*it == '\\') {
				*it = '_';
			}
		}
	} else if (g_sci->getGameId() == GID_PHANTASMAGORIA2 && name == "RESDUK.PAT") {
		// Ignore the censorship password file in lieu of our game option
		return SIGNAL_REG;
	} else if (g_sci->getGameId() == GID_RAMA) {
		if (name == "PREF.DAT") {
			return SIGNAL_REG;
		}

		int saveNo = -1;
		if (name == "911.sg" || name == "autorama.sg") {
			saveNo = kAutoSaveId;
		} else if (sscanf(name.c_str(), "ramasg.%d", &saveNo) == 1) {
			saveNo += kSaveIdShift;
		}

		if (saveNo != -1) {
			Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
			const Common::String fileName = g_sci->getSavegameName(saveNo);
			Common::SeekableReadStream *in = nullptr;
			Common::OutSaveFile *out = nullptr;
			bool valid = false;

			if (mode == kFileOpenModeOpenOrFail) {
				in = saveFileMan->openForLoading(fileName);
				if (in) {
					SavegameMetadata meta;
					if (get_savegame_metadata(in, meta)) {
						Graphics::skipThumbnail(*in);
						valid = true;
					}
					if (meta.version >= 34) {
						g_sci->setTickCount(meta.playTime);
					}
				}
			} else {
				out = saveFileMan->openForSaving(fileName);
				if (out) {
					Common::String saveName;
					if (saveNo == kAutoSaveId) {
						saveName = _("(Autosave)");
					} else {
						saveName = getRamaSaveName(s, saveNo - kSaveIdShift);
					}
					Common::ScopedPtr<Common::SeekableReadStream> versionFile(SearchMan.createReadStreamForMember("VERSION"));
					const Common::String gameVersion = versionFile->readLine();
					set_savegame_metadata(out, saveName, gameVersion);
					valid = true;
				}
			}

			if (valid) {
				uint handle = findFreeFileHandle(s);
				s->_fileHandles[handle]._in = in;
				s->_fileHandles[handle]._out = out;
				s->_fileHandles[handle]._name = "-scummvm-save-";
				return make_reg(0, handle);
			}
		}
	}

	// See kMakeSaveCatName
	if (name == "fake.cat") {
		return make_reg(0, kVirtualFileHandleSci32Save);
	}

	if (isSaveCatalogue(name)) {
		const bool exists = saveCatalogueExists(name);
		if (exists) {
			// KQ7 & RAMA read out game information from catalogues directly
			// instead of using the standard kSaveGetFiles function
			if (name == "kq7cdsg.cat" || name == "ramasg.cat") {
				if (mode == kFileOpenModeOpenOrCreate || mode == kFileOpenModeCreate) {
					// Suppress creation of the catalogue file, since it is not necessary
					debugC(kDebugLevelFile, "Not creating unused file %s", name.c_str());
					return SIGNAL_REG;
				} else if (mode == kFileOpenModeOpenOrFail) {
					const uint handle = findFreeFileHandle(s);

					if (name == "kq7cdsg.cat") {
						s->_fileHandles[handle]._in = makeCatalogue(10, kMaxSaveNameLength, "", false);
					} else {
						s->_fileHandles[handle]._in = makeCatalogue(100, 20, "ramasg.%d", true);
					}
					s->_fileHandles[handle]._out = nullptr;
					s->_fileHandles[handle]._name = "";
					return make_reg(0, handle);
				}
			}

			// Dummy handle is used to represent the catalogue and ignore any
			// direct game script writes
			return make_reg(0, kVirtualFileHandleSci32Save);
		} else {
			return SIGNAL_REG;
		}
	}
#endif

	debugC(kDebugLevelFile, "kFileIO(open): %s, 0x%x", name.c_str(), mode);

	if (name.hasPrefix("sciAudio\\")) {
		// fan-made sciAudio extension, don't create those files and instead return a virtual handle
		return make_reg(0, kVirtualFileHandleSciAudio);
	}

	// QFG import rooms get a virtual filelisting instead of an actual one
	if (g_sci->inQfGImportRoom()) {
		// We need to find out what the user actually selected, "savedHeroes" is
		// already destroyed when we get here. That's why we need to remember
		// selection via kDrawControl.
		name = s->_dirseeker.getVirtualFilename(s->_chosenQfGImportItem);
		unwrapFilename = false;
	}

	return file_open(s, name, mode, unwrapFilename);
}

reg_t kFileIOClose(EngineState *s, int argc, reg_t *argv) {
	debugC(kDebugLevelFile, "kFileIO(close): %d", argv[0].toUint16());

	if (argv[0] == SIGNAL_REG)
		return s->r_acc;

	uint16 handle = argv[0].toUint16();

	if (handle >= kVirtualFileHandleStart) {
		// it's a virtual handle? ignore it
		return TRUE_REG;
	}

	FileHandle *f = getFileFromHandle(s, handle);
	if (f) {
		f->close();
		if (getSciVersion() <= SCI_VERSION_0_LATE)
			return s->r_acc;	// SCI0 semantics: no value returned
		return TRUE_REG;
	}

	if (getSciVersion() <= SCI_VERSION_0_LATE)
		return s->r_acc;	// SCI0 semantics: no value returned
	return NULL_REG;
}

reg_t kFileIOReadRaw(EngineState *s, int argc, reg_t *argv) {
	uint16 handle = argv[0].toUint16();
	reg_t dest = argv[1];
	uint16 size = argv[2].toUint16();
	int bytesRead = 0;
	byte *buf = new byte[size];
	debugC(kDebugLevelFile, "kFileIO(readRaw): %d, %d", handle, size);

	FileHandle *f = getFileFromHandle(s, handle);
	if (f) {
#ifdef ENABLE_SCI32
		SegmentRef destReference = s->_segMan->dereference(dest);
		SegmentObj *destObject = s->_segMan->getSegmentObj(dest.getSegment());
		if (destObject == nullptr) {
			error("kFileIO(readRaw): invalid destination %04x:%04x", PRINT_REG(dest));
		}

		if (destReference.maxSize == size - 4 && destObject->getType() == SEG_TYPE_ARRAY) {
			// This is an array structure, which starts with the number of
			// elements in the array and the size of each element. Skip
			// these bytes. These structures are stored in the ARC files of
			// the Behind the Developer's Shield and Inside the Chest demos.
			f->_in->skip(4);
			size -= 4;
		}
#endif

		bytesRead = f->_in->read(buf, size);
	}

	if (bytesRead > 0)
		s->_segMan->memcpy(dest, buf, bytesRead);

	delete[] buf;
	return make_reg(0, bytesRead);
}

reg_t kFileIOWriteRaw(EngineState *s, int argc, reg_t *argv) {
	uint16 handle = argv[0].toUint16();
	uint16 size = argv[2].toUint16();

#ifdef ENABLE_SCI32
	if (handle == kVirtualFileHandleSci32Save) {
		return make_reg(0, size);
	}
#endif

	char *buf = new char[size];
	uint bytesWritten = 0;
	bool success = false;
	s->_segMan->memcpy((byte *)buf, argv[1], size);

	FileHandle *f = getFileFromHandle(s, handle);
	if (f) {
		bytesWritten = f->_out->write(buf, size);
		success = !f->_out->err();
	}

	debugC(kDebugLevelFile, "kFileIO(writeRaw): %d, %d (%d, %d)", handle, size, bytesWritten, success);

	delete[] buf;

	if (success) {
		return make_reg(0, bytesWritten);
	}
	return getSciVersion() >= SCI_VERSION_2 ? SIGNAL_REG : NULL_REG;
}

reg_t kFileIOUnlink(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	bool result = false;

	// SQ4 floppy prepends /\ to the filenames
	if (name.hasPrefix("/\\")) {
		name.deleteChar(0);
		name.deleteChar(0);
	}

	if (g_sci->getGameId() == GID_SQ4 && name.hasPrefix("sq4sg.")) {
		// Special case for SQ4 floppy: This game has hardcoded save game names.
		// They are named "sq4sg.xxx", where xxx is the virtual ID. We construct
		// the appropriate save game name and delete it.
		const int savegameId = atoi(name.c_str() + name.size() - 3) - SAVEGAMEID_OFFICIALRANGE_START;
		name = g_sci->getSavegameName(savegameId);
		result = saveFileMan->removeSavefile(name);
#ifdef ENABLE_SCI32
	} else if (getSciVersion() >= SCI_VERSION_2) {
		// Special cases for KQ7 & RAMA, basically identical to the SQ4 case
		// above, where the game hardcodes its save game names
		int saveNo;
		if (sscanf(name.c_str(), "kq7cdsg.%d", &saveNo) == 1 ||
			sscanf(name.c_str(), "ramasg.%d", &saveNo) == 1) {

			name = g_sci->getSavegameName(saveNo + kSaveIdShift);
		} else if (g_sci->getGameId() == GID_RAMA && (name == "911.sg" || name == "autorama.sg")) {
			name = g_sci->getSavegameName(kAutoSaveId);
		}

		// The file name may be already wrapped, so check both cases
		result = saveFileMan->removeSavefile(name);
		if (!result) {
			const Common::String wrappedName = g_sci->wrapFilename(name);
			result = saveFileMan->removeSavefile(wrappedName);
		}
#endif
	} else if (g_sci->getGameId() == GID_KQ5 && 
				g_sci->getPlatform() == Common::kPlatformFMTowns && 
				name.hasPrefix("a:\\KQ5sg.")) {
		// KQ5 FM-Towns uses a custom save/restore UI in script 764.
		// It directly deletes save files using a hard-coded path.
		int saveNo = 0;
		sscanf(name.c_str(), "a:\\KQ5sg.%d", &saveNo);
		if (1 <= saveNo && saveNo <= 10) { // UI has ten buttons
			name = g_sci->getSavegameName(saveNo);
			result = saveFileMan->removeSavefile(name);
		}
	} else {
		const Common::String wrappedName = g_sci->wrapFilename(name);
		result = saveFileMan->removeSavefile(wrappedName);
	}

	debugC(kDebugLevelFile, "kFileIO(unlink): %s", name.c_str());

	return make_reg(0, result);
}

reg_t kFileIOReadString(EngineState *s, int argc, reg_t *argv) {
	uint16 maxsize = argv[1].toUint16();
	char *buf = new char[maxsize];
	uint16 handle = argv[2].toUint16();
	debugC(kDebugLevelFile, "kFileIO(readString): %d, %d", handle, maxsize);
	uint32 bytesRead;

	bytesRead = fgets_wrapper(s, buf, maxsize, handle);

	// Fix up size too large for destination.
	SegmentRef dest_r = s->_segMan->dereference(argv[0]);
	if (!dest_r.isValid()) {
		error("kFileIO(readString): invalid destination %04x:%04x", PRINT_REG(argv[0]));
	} else if ((int)bytesRead > dest_r.maxSize) {
		error("kFileIO(readString) attempting to read %u bytes into buffer of size %u", bytesRead, dest_r.maxSize);
	} else if (maxsize > dest_r.maxSize) {
		maxsize = dest_r.maxSize;
	}

	s->_segMan->memcpy(argv[0], (const byte*)buf, maxsize);
	delete[] buf;
	return bytesRead ? argv[0] : NULL_REG;
}

reg_t kFileIOWriteString(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	Common::String str = s->_segMan->getString(argv[1]);
	debugC(kDebugLevelFile, "kFileIO(writeString): %d", handle);

	// Handle sciAudio calls in fanmade games here. sciAudio is an
	// external .NET library for playing MP3 files in fanmade games.
	// It runs in the background, and obtains sound commands from the
	// currently running game via text files (called "conductor files").
	// We skip creating these files, and instead handle the calls
	// directly. Since the sciAudio calls are only creating text files,
	// this is probably the most straightforward place to handle them.
	if (handle == kVirtualFileHandleSciAudio) {
		Common::List<ExecStack>::const_iterator iter = s->_executionStack.reverse_begin();
		iter--;	// sciAudio
		iter--;	// sciAudio child
		g_sci->_audio->handleFanmadeSciAudio(iter->sendp, s->_segMan);
		return NULL_REG;
	}

	FileHandle *f = getFileFromHandle(s, handle);

	if (f && f->_out) {
		uint32 bytesWritten = f->_out->write(str.c_str(), str.size());
		return make_reg(0, bytesWritten);
	}

	return getSciVersion() >= SCI_VERSION_2 ? SIGNAL_REG : NULL_REG;
}

reg_t kFileIOSeek(EngineState *s, int argc, reg_t *argv) {
	uint16 handle = argv[0].toUint16();
	int16 offset = argv[1].toSint16();
	uint16 whence = argv[2].toUint16();
	debugC(kDebugLevelFile, "kFileIO(seek): %d, %d, %d", handle, offset, whence);

	FileHandle *f = getFileFromHandle(s, handle);

	if (f && f->_in) {
		offset = MIN<int16>(offset, f->_in->size());
		const bool success = f->_in->seek(offset, whence);
		if (getSciVersion() >= SCI_VERSION_2) {
			if (success) {
				return make_reg(0, f->_in->pos());
			}
			return SIGNAL_REG;
		}
		return make_reg(0, success);
	} else if (f && f->_out) {
		error("kFileIOSeek: Unsupported seek operation on a writeable stream (offset: %d, whence: %d)", offset, whence);
	}

	return SIGNAL_REG;
}

reg_t kFileIOFindFirst(EngineState *s, int argc, reg_t *argv) {
	Common::String mask = s->_segMan->getString(argv[0]);
	reg_t buf = argv[1];
	int attr = argv[2].toUint16(); // We won't use this, Win32 might, though...
	debugC(kDebugLevelFile, "kFileIO(findFirst): %s, 0x%x", mask.c_str(), attr);

	// We remove ".*". mask will get prefixed, so we will return all additional files for that gameid
	if (mask == "*.*")
		mask = "*";
	return s->_dirseeker.firstFile(mask, buf, s->_segMan);
}

reg_t kFileIOFindNext(EngineState *s, int argc, reg_t *argv) {
	debugC(kDebugLevelFile, "kFileIO(findNext)");
	return s->_dirseeker.nextFile(s->_segMan);
}

reg_t kFileIOExists(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);

	if (g_sci->getGameId() == GID_PEPPER) {
		// HACK: Special case for Pepper's Adventure in Time
		// The game checks like crazy for the file CDAUDIO when entering the game menu.
		// On at least Windows that makes the engine slow down to a crawl and takes at least 1 second.
		// Should get solved properly by changing the code below. This here is basically for 1.8.0 release.
		// TODO: Fix this properly.
		if (name == "CDAUDIO")
			return NULL_REG;
	}

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		if (isSaveCatalogue(name)) {
			return saveCatalogueExists(name) ? TRUE_REG : NULL_REG;
		}

		int findSaveNo = -1;
		if (g_sci->getGameId() == GID_LSL7 && name == "autosvsg.000") {
			// LSL7 checks to see if the autosave save exists when deciding whether
			// to go to the main menu or not on startup
			findSaveNo = kAutoSaveId;
		} else if (g_sci->getGameId() == GID_RAMA) {
			// RAMA checks to see if save game files exist before showing them in
			// the native save/load dialogue
			if (name == "autorama.sg") {
				findSaveNo = kAutoSaveId;
			} else if (sscanf(name.c_str(), "ramasg.%d", &findSaveNo) == 1) {
				findSaveNo += kSaveIdShift;
			}
		}

		if (findSaveNo != -1) {
			return g_sci->getSaveFileManager()->listSavefiles(g_sci->getSavegameName(findSaveNo)).empty() ? NULL_REG : TRUE_REG;
		}
		// TODO: It may apparently be worth caching the existence of
		// phantsg.dir, and possibly even keeping it open persistently
	}
#endif

	if (g_sci->getGameId() == GID_KQ5 && g_sci->getPlatform() == Common::kPlatformFMTowns) {
		// KQ5 FM-Towns uses a custom save/restore UI in script 764.
		// It directly tests for save files using a hard-coded path.
		int saveNo = 0;
		sscanf(name.c_str(), "a:\\KQ5sg.%d", &saveNo);
		if (1 <= saveNo && saveNo <= 10) { // UI has ten buttons
			Common::Array<SavegameDesc> saves;
			listSavegames(saves);
			return (findSavegame(saves, saveNo) != -1) ? TRUE_REG : NULL_REG;
		}
	}

	// Check for regular file
	bool exists = Common::File::exists(Common::Path(name));

	// Check for a savegame with the name
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	if (!exists)
		exists = !saveFileMan->listSavefiles(name).empty();

	// Try searching for the file prepending "target-"
	const Common::String wrappedName = g_sci->wrapFilename(name);
	if (!exists) {
		exists = !saveFileMan->listSavefiles(wrappedName).empty();
	}

	// SCI2+ debug mode
	if (DebugMan.isDebugChannelEnabled(kDebugLevelDebugMode)) {
		if (!exists && name == "1.scr")		// PQ4
			exists = true;
		if (!exists && name == "18.scr")	// QFG4
			exists = true;
		if (!exists && name == "99.scr")	// GK1, KQ7
			exists = true;
		if (!exists && name == "classes")	// GK2, SQ6, LSL7
			exists = true;
	}

	// GK1 easter egg at the Voodoo Hounfour in script 805. In this easter
	// egg, Gabriel draws a doodle of Jane Jensen in the whiteboard, if the
	// player uses the operate action below the whiteboard's eraser. This
	// easter egg looks for a file named "buster" to be present, so that it
	// is enabled. We always report that this file exists, to unlock the
	// easter egg.
	if (!exists && name == "buster")
		exists = true;
	
	// Special case for non-English versions of LSL5: The English version of
	// LSL5 calls kFileIO(), case K_FILEIO_OPEN for reading to check if
	// memory.drv exists (which is where the game's password is stored). If
	// it's not found, it calls kFileIO() again, case K_FILEIO_OPEN for
	// writing and creates a new file. Non-English versions call kFileIO(),
	// case K_FILEIO_FILE_EXISTS instead, and fail if memory.drv can't be
	// found. We create a default memory.drv file with no password, so that
	// the game can continue.
	if (!exists && name == "memory.drv") {
		// Create a new file, and write the bytes for the empty password
		// string inside
		byte defaultContent[] = { 0xE9, 0xE9, 0xEB, 0xE1, 0x0D, 0x0A, 0x31, 0x30, 0x30, 0x30 };
		Common::WriteStream *outFile = saveFileMan->openForSaving(wrappedName);
		for (int i = 0; i < 10; i++)
			outFile->writeByte(defaultContent[i]);
		outFile->finalize();
		exists = !outFile->err();	// check whether we managed to create the file.
		delete outFile;
	}

	// Special case for KQ6 Mac: The game checks for two video files to see
	// if they exist before it plays them. Since we support multiple naming
	// schemes for resource fork files, we also need to support that here in
	// case someone has a "HalfDome.bin" file, etc.
	if (!exists && g_sci->getGameId() == GID_KQ6 && g_sci->getPlatform() == Common::kPlatformMacintosh &&
			(name == "HalfDome" || name == "Kq6Movie"))
		exists = Common::MacResManager::exists(Common::Path(name));

	debugC(kDebugLevelFile, "kFileIO(fileExists) %s -> %d", name.c_str(), exists);
	return make_reg(0, exists);
}

reg_t kFileIORename(EngineState *s, int argc, reg_t *argv) {
	Common::String oldName = s->_segMan->getString(argv[0]);
	Common::String newName = s->_segMan->getString(argv[1]);

	// We don't fully implement all cases that could occur here, and
	// assume the file to be renamed is a wrapped filename.
	// Known usage: In Phant1 and KQ7 while deleting savegames.
	// The scripts rewrite the dir file as a temporary file, and then
	// rename it to the actual dir file.

	oldName = g_sci->wrapFilename(oldName);
	newName = g_sci->wrapFilename(newName);

	// Phantasmagoria 1 files are small and interoperable with the
	//  original interpreter so they aren't compressed, see file_open().
	bool isCompressed = (g_sci->getGameId() != GID_PHANTASMAGORIA);

	// SCI1.1 returns 0 on success and a DOS error code on fail. SCI32
	// returns -1 on fail. We just return -1 for all versions.
	if (g_sci->getSaveFileManager()->renameSavefile(oldName, newName, isCompressed))
		return NULL_REG;
	else
		return SIGNAL_REG;
}

reg_t kFileIOCopy(EngineState *s, int argc, reg_t *argv) {
	Common::String oldName = s->_segMan->getString(argv[0]);
	Common::String newName = s->_segMan->getString(argv[1]);

	oldName = g_sci->wrapFilename(oldName);
	newName = g_sci->wrapFilename(newName);

	// Phantasmagoria 1 files are small and interoperable with the
	//  original interpreter so they aren't compressed, see file_open().
	bool isCompressed = (g_sci->getGameId() != GID_PHANTASMAGORIA);

	// SCI1.1 returns 0 on success and a DOS error code on fail. SCI32
	// returns -1 on fail. We just return -1 for all versions.
	if (g_sci->getSaveFileManager()->copySavefile(oldName, newName, isCompressed))
		return NULL_REG;
	else
		return SIGNAL_REG;
}

#ifdef ENABLE_SCI32
reg_t kFileIOReadByte(EngineState *s, int argc, reg_t *argv) {
	// Read the byte into the low byte of the accumulator
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (!f)
		return NULL_REG;
	return make_reg(0, (s->r_acc.toUint16() & 0xff00) | f->_in->readByte());
}

reg_t kFileIOWriteByte(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (f) {
		f->_out->writeByte(argv[1].toUint16() & 0xff);
		return make_reg(0, 1); // bytesWritten
	}
	return SIGNAL_REG;
}

reg_t kFileIOReadWord(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (!f)
		return s->r_acc;

	reg_t value;
	if (f->_name == "-scummvm-save-") {
		value._segment = f->_in->readUint16LE();
		value._offset = f->_in->readUint16LE();
	} else {
		value = make_reg(0, f->_in->readUint16LE());
	}

	if (f->_in->err()) {
		return s->r_acc;
	}

	return value;
}

reg_t kFileIOWriteWord(EngineState *s, int argc, reg_t *argv) {
	const uint16 handle = argv[0].toUint16();

	if (handle == kVirtualFileHandleSci32Save) {
		return make_reg(0, 2); // bytesWritten
	}

	FileHandle *f = getFileFromHandle(s, handle);
	if (!f) {
		return SIGNAL_REG;
	}

	if (f->_name == "-scummvm-save-") {
		f->_out->writeUint16LE(argv[1]._segment);
		f->_out->writeUint16LE(argv[1]._offset);
	} else {
		if (argv[1].isPointer()) {
			error("kFileIO(WriteWord): Attempt to write non-number %04x:%04x to non-save file", PRINT_REG(argv[1]));
		}
		f->_out->writeUint16LE(argv[1].toUint16());
	}

	return make_reg(0, 2); // bytesWritten
}

reg_t kFileIOGetCWD(EngineState *s, int argc, reg_t *argv) {
	SciArray &fileName = *s->_segMan->lookupArray(argv[0]);
	fileName.fromString("C:\\SIERRA\\");
	return argv[0];
}

reg_t kFileIOIsValidDirectory(EngineState *s, int argc, reg_t *argv) {
	// Used in Torin's Passage, LSL7, and RAMA to determine if the directory
	// passed as a parameter (usually the save directory) is valid. We always
	// return true here because we do not use this directory information when
	// saving games.
	return TRUE_REG;
}

#endif

// ---- Save operations -------------------------------------------------------

#ifdef ENABLE_SCI32

reg_t kSave(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

#endif

reg_t kSaveGame(EngineState *s, int argc, reg_t *argv) {
	// slot 0 is the ScummVM auto-save slot, which is not used by us, but is
	// still reserved
	enum {
		SAVEGAMESLOT_FIRST = 1,
		SAVEGAMESLOT_LAST = 99
	};

	Common::String game_id = !argv[0].isNull() ? s->_segMan->getString(argv[0]) : "";
	int16 virtualId = argv[1].toSint16();
	int16 savegameId = -1;
	Common::String game_description;
	Common::String version;

	if (argc > 3)
		version = s->_segMan->getString(argv[3]);

	// We check here, we don't want to delete a users save in case we are within a kernel function
	if (s->executionStackBase) {
		warning("kSaveGame - won't save from within kernel function");
		return NULL_REG;
	}

	if (argv[0].isNull()) {
		// Direct call, from a patched Game::save
		if ((argv[1] != SIGNAL_REG) || (!argv[2].isNull()))
			error("kSaveGame: assumed patched call isn't accurate");

		// we are supposed to show a dialog for the user and let him choose where to save
		g_sci->_soundCmd->pauseAll(true); // pause music
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		savegameId = dialog->runModalWithCurrentTarget();
		game_description = dialog->getResultString();
		if (game_description.empty()) {
			// create our own description for the saved game, the user didn't enter it
			game_description = dialog->createDefaultSaveDescription(savegameId);
		}
		delete dialog;
		g_sci->_soundCmd->pauseAll(false); // unpause music (we can't have it paused during save)
		if (savegameId < 0)
			return NULL_REG;
	} else {
		// Real call from script
		if (argv[2].isNull())
			error("kSaveGame: called with description being NULL");
		game_description = s->_segMan->getString(argv[2]);
		if (g_sci->getLanguage() == Common::HE_ISR) {
			Common::U32String u32string = game_description.decode(Common::kWindows1255);
			game_description = u32string.encode(Common::kUtf8);
		} else if (g_sci->getLanguage() == Common::RU_RUS) {
			Common::U32String u32string = game_description.decode(Common::kDos866);
			game_description = u32string.encode(Common::kUtf8);
		};


		debug(3, "kSaveGame(%s,%d,%s,%s)", game_id.c_str(), virtualId, game_description.c_str(), version.c_str());

		Common::Array<SavegameDesc> saves;
		listSavegames(saves);

		if ((virtualId >= SAVEGAMEID_OFFICIALRANGE_START) && (virtualId <= SAVEGAMEID_OFFICIALRANGE_END)) {
			// savegameId is an actual Id, so search for it just to make sure
			savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
			if (findSavegame(saves, savegameId) == -1)
				return NULL_REG;
		} else if (virtualId < SAVEGAMEID_OFFICIALRANGE_START) {
			// virtualId is low, we assume that scripts expect us to create new slot
			switch (g_sci->getGameId()) {
			case GID_JONES:
				// Jones has one save slot only
				savegameId = 0;
				break;
			case GID_KQ5:
				if (g_sci->getPlatform() == Common::kPlatformFMTowns) {
					// KQ5 FM-Towns uses custom save/restore code.
					// Use the provided id.
					savegameId = virtualId;
					// Use a default description, game passes path since it wasn't displayed.
					game_description = Common::String::format("Save %d", savegameId);
				}
				break;
			case GID_QFG3: {
				// Auto-save system used by QFG3
				reg_t autoSaveNameId;
				s->_segMan->allocDynmem(kMaxSaveNameLength, "kSaveGame", &autoSaveNameId);
				MessageTuple autoSaveNameTuple(0, 0, 16, 1);
				s->_msgState->getMessage(0, autoSaveNameTuple, autoSaveNameId);
				Common::String autoSaveName = s->_segMan->getString(autoSaveNameId);
				if (game_description == autoSaveName) {
					savegameId = kAutoSaveId;
				}

				s->_segMan->freeDynmem(autoSaveNameId);
				break;
			}
			case GID_FANMADE: {
				// Fanmade game, try to identify the game
				const char *gameName = g_sci->getGameObjectName();

				if (strcmp(gameName, "CascadeQuest") == 0) {
					// Cascade Quest calls us directly to auto-save and uses slot 99,
					//  put that save into slot 0 (ScummVM auto-save slot) see bug #7007
					if (virtualId == (SAVEGAMEID_OFFICIALRANGE_START - 1)) {
						savegameId = 0;
					}
				}
				break;
			}
			default:
				break;
			}

			if (savegameId < 0) {
				// savegameId not set yet
				if (virtualId == s->_lastSaveVirtualId) {
					// if last virtual id is the same as this one, we assume that caller wants to overwrite last save
					savegameId = s->_lastSaveNewId;
				} else {
					uint savegameNr;
					// savegameId is in lower range, scripts expect us to create a new slot
					for (savegameId = SAVEGAMESLOT_FIRST; savegameId <= SAVEGAMESLOT_LAST; savegameId++) {
						for (savegameNr = 0; savegameNr < saves.size(); savegameNr++) {
							if (savegameId == saves[savegameNr].id)
								break;
						}
						if (savegameNr == saves.size()) // Slot not found, seems to be good to go
							break;
					}
					if (savegameId > SAVEGAMESLOT_LAST)
						error("kSavegame: no more savegame slots available");
				}
			}

			// WORKAROUND: Mothergoose256 has a unique scheme for calculating the current save id
			// and storing it in a global. The SCI1.1 floppy version uses this to auto-save and
			// auto-delete at the end of the game. This is incompatible with our virtual id system
			// so we work around this by setting the game's save global to the virtual id here and
			// also when restoring so that it's always correct. See gamestate_afterRestoreFixUp().
			// Fixes bug #5294
			if (g_sci->getGameId() == GID_MOTHERGOOSE256) {
				s->variables[VAR_GLOBAL][0xB3].setOffset(SAVEGAMEID_OFFICIALRANGE_START + savegameId);
			}
		} else {
			error("kSaveGame: invalid savegameId used");
		}

		// Save in case caller wants to overwrite last newly created save
		s->_lastSaveVirtualId = virtualId;
		s->_lastSaveNewId = savegameId;
	}

	if (gamestate_save(s, savegameId, game_description, version)) {
		return TRUE_REG;
	}
	return NULL_REG;
}

reg_t kRestoreGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = !argv[0].isNull() ? s->_segMan->getString(argv[0]) : "";
	int16 savegameId = argv[1].toSint16();
	bool pausedMusic = false;

	debug(3, "kRestoreGame(%s,%d)", game_id.c_str(), savegameId);

	if (argv[0].isNull()) {
		// Direct call, either from launcher or from a patched Game::restore
		if (savegameId == -1) {
			// We are supposed to show a dialog for the user and let him choose a saved game.
			// Pause music if necessary. There are script situations where the pause does not
			// get properly released. In that case we don't add another pause here.
			if (!g_sci->_soundCmd->isGlobalPauseActive()) 
				g_sci->_soundCmd->pauseAll(true); 
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
			savegameId = dialog->runModalWithCurrentTarget();
			delete dialog;
			if (savegameId < 0) {
				g_sci->_soundCmd->pauseAll(false); // unpause music
				return s->r_acc;
			}
			pausedMusic = true;
		}
		// don't adjust ID of the saved game, it's already correct
	} else {
		if (g_sci->getGameId() == GID_JONES) {
			// Jones has one save slot only
			savegameId = 0;
		} else if (g_sci->getGameId() == GID_KQ5 &&	g_sci->getPlatform() == Common::kPlatformFMTowns) {
			// KQ5 FM-Towns uses custom save/restore code.
			// Use the provided id.
		} else {
			// Real call from script, we need to adjust ID
			if ((savegameId < SAVEGAMEID_OFFICIALRANGE_START) || (savegameId > SAVEGAMEID_OFFICIALRANGE_END)) {
				warning("Savegame ID %d is not allowed", savegameId);
				return TRUE_REG;
			}
			savegameId -= SAVEGAMEID_OFFICIALRANGE_START;
		}
	}

	s->r_acc = NULL_REG; // signals success

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);
	if (findSavegame(saves, savegameId) == -1) {
		s->r_acc = TRUE_REG;
		warning("Savegame ID %d not found", savegameId);
	} else if (!gamestate_restore(s, savegameId)) {
		s->r_acc = TRUE_REG; // signals failure
	}

	if (pausedMusic) {
		if (!s->r_acc.isNull()) // no success?
			g_sci->_soundCmd->pauseAll(false); // unpause music
		else
			g_sci->_soundCmd->resetGlobalPauseCounter(); // reset music global pause counter without affecting the individual sounds
	} else if (s->r_acc.isNull() && g_sci->_soundCmd->isGlobalPauseActive()) {
		g_sci->_soundCmd->resetGlobalPauseCounter(); // reset music global pause counter without affecting the individual sounds
	}

	return s->r_acc;
}

reg_t kGetSaveDir(EngineState *s, int argc, reg_t *argv) {
#ifdef ENABLE_SCI32
	// SCI32 uses a parameter here. It is used to modify a string, stored in a
	// global variable, so that game scripts store the save directory. We
	// don't really set a save game directory, thus not setting the string to
	// anything is the correct thing to do here.
	//if (argc > 0)
	//	warning("kGetSaveDir called with %d parameter(s): %04x:%04x", argc, PRINT_REG(argv[0]));
#endif
		return s->_segMan->getSaveDirPtr();
}

reg_t kCheckSaveGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);
	uint16 virtualId = argv[1].toUint16();

	debug(3, "kCheckSaveGame(%s, %d)", game_id.c_str(), virtualId);

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	// we allow 0 (happens in QfG2 when trying to restore from an empty saved game list) and return false in that case
	if (virtualId == 0)
		return NULL_REG;

	uint savegameId = 0;
	if (g_sci->getGameId() == GID_JONES) {
		// Jones has one save slot only
	} else if (g_sci->getGameId() == GID_KQ5 &&	g_sci->getPlatform() == Common::kPlatformFMTowns) {
		// KQ5 FM-Towns uses custom save/restore code.
		// Use the provided id.
		savegameId = virtualId;
	} else {
		// Find saved game
		if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
			error("kCheckSaveGame: called with invalid savegame ID (%d)", virtualId);
		savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
	}

	int savegameNr = findSavegame(saves, savegameId);
	if (savegameNr == -1)
		return NULL_REG;

	// Check for compatible savegame version
	int ver = saves[savegameNr].version;
	if (ver < MINIMUM_SAVEGAME_VERSION || ver > CURRENT_SAVEGAME_VERSION)
		return NULL_REG;

	// Otherwise we assume the savegame is OK
	return TRUE_REG;
}

reg_t kGetSaveFiles(EngineState *s, int argc, reg_t *argv) {
	// Scripts ask for current save files, we can assume that if afterwards they ask us to create a new slot they really
	//  mean new slot instead of overwriting the old one
	s->_lastSaveVirtualId = SAVEGAMEID_OFFICIALRANGE_START;

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);
	uint totalSaves = MIN<uint>(saves.size(), kMaxNumSaveGames);

	Common::String game_id = s->_segMan->getString(argv[0]);

	debug(3, "kGetSaveFiles(%s)", game_id.c_str());

	reg_t *slot = s->_segMan->derefRegPtr(argv[2], totalSaves);

	if (!slot) {
		warning("kGetSaveFiles: %04X:%04X invalid or too small to hold slot data", PRINT_REG(argv[2]));
		totalSaves = 0;
	}

	const uint bufSize = (totalSaves * kMaxSaveNameLength) + 1;
	char *saveNames = new char[bufSize];
	char *saveNamePtr = saveNames;

	for (uint i = 0; i < totalSaves; i++) {
		*slot++ = make_reg(0, saves[i].id + SAVEGAMEID_OFFICIALRANGE_START); // Store the virtual savegame ID (see above)
		Common::strlcpy(saveNamePtr, saves[i].name, kMaxSaveNameLength);
		saveNamePtr += kMaxSaveNameLength;
	}

	*saveNamePtr = 0; // Terminate list

	s->_segMan->memcpy(argv[1], (byte *)saveNames, bufSize);
	delete[] saveNames;

	return make_reg(0, totalSaves);
}

#ifdef ENABLE_SCI32

reg_t kSaveGame32(EngineState *s, int argc, reg_t *argv) {
	// fix bug #9752 - make sure that control panel (in case of QFG4),
	// keyboard (in case of Shivers), or any other obstacle (in other unknown SCI32 games...)
	// will be hidden before saving
	kFrameOut(s, 0, NULL);

	const Common::String gameName = s->_segMan->getString(argv[0]);
	int16 saveNo = argv[1].toSint16();
	Common::String saveDescription = argv[2].isNull() ? "" : s->_segMan->getString(argv[2]);
	const Common::String gameVersion = (argc <= 3 || argv[3].isNull()) ? "" : s->_segMan->getString(argv[3]);

	debugC(kDebugLevelFile, "Game name %s save %d desc %s ver %s", gameName.c_str(), saveNo, saveDescription.c_str(), gameVersion.c_str());

	// Display the save prompt for Mac games with native dialogs. Passing
	//  zero for the save number would trigger these, but we can't act solely
	//  on that since we shift save numbers around to accommodate autosave
	//  slots, causing some games to pass zero that normally wouldn't.
	if (g_sci->hasMacSaveRestoreDialogs() && saveNo == 0) {
		saveNo = g_sci->_guestAdditions->runSaveRestore(true, argv[2]);
		if (saveNo == -1) {
			return NULL_REG;
		}
		saveDescription = s->_segMan->getString(argv[2]);
	}

	// Auto-save system used by Torin and LSL7
	if (gameName == "Autosave" || gameName == "Autosv") {
		if (saveNo == 0) {
			// Autosave slot 0 is the autosave
		} else {
			// Autosave slot 1 is a "new game" save
			saveNo = kNewGameId;
		}
	} else {
		saveNo = shiftSciToScummVMSaveId(saveNo);
	}

	if (g_sci->getGameId() == GID_PHANTASMAGORIA2 && s->callInStack(g_sci->getGameObject(), SELECTOR(bookMark))) {
		saveNo = kAutoSaveId;
	} else if (g_sci->getGameId() == GID_LIGHTHOUSE && gameName == "rst") {
		saveNo = kNewGameId;
	} else if (g_sci->getGameId() == GID_QFG4) {
		// Auto-save system used by QFG4
		reg_t autoSaveNameId;
		SciArray &autoSaveName = *s->_segMan->allocateArray(kArrayTypeString, 0, &autoSaveNameId);
		MessageTuple autoSaveNameTuple(0, 0, 16, 1);
		s->_msgState->getMessage(0, autoSaveNameTuple, autoSaveNameId);

		if (saveDescription == autoSaveName.toString()) {
			saveNo = kAutoSaveId;
		}

		s->_segMan->freeArray(autoSaveNameId);
	}

	if (gamestate_save(s, saveNo, saveDescription, gameVersion)) {
		return TRUE_REG;
	}
	return NULL_REG;
}

reg_t kRestoreGame32(EngineState *s, int argc, reg_t *argv) {
	const Common::String gameName = s->_segMan->getString(argv[0]);
	int16 saveNo = argv[1].toSint16();
	//const Common::String gameVersion = argv[2].isNull() ? "" : s->_segMan->getString(argv[2]);

	// Display the restore prompt for Mac games with native dialogs. Passing
	//  zero for the save number would trigger these, but we can't act solely
	//  on that since we shift save numbers around to accommodate autosave
	//  slots, causing some games to pass zero that normally wouldn't.
	if (g_sci->hasMacSaveRestoreDialogs() && saveNo == 0) {
		saveNo = g_sci->_guestAdditions->runSaveRestore(false, NULL_REG, s->_delayedRestoreGameId);
		if (saveNo == -1) {
			return NULL_REG;
		}
	}

	if (gameName == "Autosave" || gameName == "Autosv") {
		if (saveNo == 0) {
			// Autosave slot 0 is the autosave
		} else {
			// Autosave slot 1 is a "new game" save
			saveNo = kNewGameId;
		}
	} else {
		saveNo = shiftSciToScummVMSaveId(saveNo);
	}

	if (gamestate_restore(s, saveNo)) {
		return TRUE_REG;
	}
	return NULL_REG;
}

reg_t kCheckSaveGame32(EngineState *s, int argc, reg_t *argv) {
	const Common::String gameName = s->_segMan->getString(argv[0]);
	int16 saveNo = argv[1].toSint16();
	Common::String gameVersion = argv[2].isNull() ? "" : s->_segMan->getString(argv[2]);

	// If the game version is empty, fall back to loading it from the VERSION file
	if (gameVersion == "") {
		Common::ScopedPtr<Common::SeekableReadStream> versionFile(SearchMan.createReadStreamForMember("VERSION"));
		gameVersion = versionFile ? versionFile->readLine() : "";
	}

	if (gameName == "Autosave" || gameName == "Autosv") {
		if (saveNo == 1) {
			saveNo = kNewGameId;
		}
	} else {
		saveNo = shiftSciToScummVMSaveId(saveNo);
	}

	SavegameDesc save;
	if (!fillSavegameDesc(g_sci->getSavegameName(saveNo), save)) {
		return NULL_REG;
	}

	if (save.version < MINIMUM_SCI32_SAVEGAME_VERSION) {
		warning("Save version %d is below minimum SCI32 savegame version %d", save.version, MINIMUM_SCI32_SAVEGAME_VERSION);
		return NULL_REG;
	}

	if (save.version > CURRENT_SAVEGAME_VERSION) {
		warning("Save version %d is above maximum SCI32 savegame version %d", save.version, CURRENT_SAVEGAME_VERSION);
		return NULL_REG;
	}

	if (save.gameVersion != gameVersion && gameVersion != "" && save.gameVersion != "") {
		warning("Save game was created for game version %s, but the current game version is %s", save.gameVersion.c_str(), gameVersion.c_str());
		return NULL_REG;
	}

	if (save.gameObjectOffset > 0 && save.script0Size > 0) {
		Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
		assert(script0);

		if (save.script0Size != script0->size()) {
			warning("Save game was created for a game with a script 0 size of %u, but the current game script 0 size is %u", save.script0Size, script0->size());
			return NULL_REG;
		}

		if (save.gameObjectOffset != g_sci->getGameObject().getOffset()) {
			warning("Save game was created for a game with the main game object at offset %u, but the current main game object offset is %u", save.gameObjectOffset, g_sci->getGameObject().getOffset());
			return NULL_REG;
		}
	}

	return TRUE_REG;
}

reg_t kGetSaveFiles32(EngineState *s, int argc, reg_t *argv) {
	// argv[0] is gameName, used in SSCI as the name of the save game catalogue
	// but unused here since ScummVM does not support multiple catalogues
	SciArray &descriptions = *s->_segMan->lookupArray(argv[1]);
	SciArray &saveIds = *s->_segMan->lookupArray(argv[2]);

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	// Normally SSCI limits to 20 games per directory, but ScummVM allows more
	// than that with games that use the standard save-load dialogue
	descriptions.resize(kMaxSaveNameLength * saves.size() + 1, true);
	saveIds.resize(saves.size() + 1, true);

	for (uint i = 0; i < saves.size(); ++i) {
		const SavegameDesc &save = saves[i];
		char *target = &descriptions.charAt(kMaxSaveNameLength * i);
		// At least Phant2 requires use of strncpy, since it creates save game
		// names of exactly kMaxSaveNameLength
		strncpy(target, save.name, kMaxSaveNameLength);
		int16 sciSaveId = shiftScummVMToSciSaveId(save.id);
		saveIds.setFromInt16(i, sciSaveId);
	}

	descriptions.charAt(kMaxSaveNameLength * saves.size()) = '\0';
	saveIds.setFromInt16(saves.size(), 0);

	return make_reg(0, saves.size());
}

reg_t kMakeSaveCatName(EngineState *s, int argc, reg_t *argv) {
	// ScummVM does not use SCI catalogues for save games, but game scripts try
	// to write out catalogues manually after a save game is deleted, so we need
	// to be able to identify and ignore these IO operations by always giving
	// back a fixed catalogue name and then ignoring it in kFileIO
	SciArray &outCatName = *s->_segMan->lookupArray(argv[0]);
	outCatName.fromString("fake.cat");
	return argv[0];
}

reg_t kMakeSaveFileName(EngineState *s, int argc, reg_t *argv) {
	SciArray &outFileName = *s->_segMan->lookupArray(argv[0]);
	// argv[1] is the game name, which is not used by ScummVM
	const int16 saveNo = argv[2].toSint16();
	outFileName.fromString(g_sci->getSavegameName(saveNo + kSaveIdShift));
	return argv[0];
}

reg_t kScummVMSaveLoad(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_guestAdditions->kScummVMSaveLoad(s, argc, argv);
}

#endif

} // End of namespace Sci
