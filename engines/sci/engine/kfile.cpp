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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/str.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/console.h"

namespace Sci {

struct SavegameDesc {
	int16 id;
	int virtualId; // straight numbered, according to id but w/o gaps
	int date;
	int time;
	int version;
	char name[SCI_MAX_SAVENAME_LENGTH];
};

/*
 * Note on how file I/O is implemented: In ScummVM, one can not create/write
 * arbitrary data files, simply because many of our target platforms do not
 * support this. The only files one can create are savestates. But SCI has an
 * opcode to create and write to seemingly 'arbitrary' files. This is mainly
 * used in LSL3 for LARRY3.DRV (which is a game data file, not a driver) and
 * in LSL5 for MEMORY.DRV (which is again a game data file and contains the
 * game's password).
 * To implement that opcode, we combine the SaveFileManager with regular file
 * code, similarly to how the SCUMM HE engine does it.
 *
 * To handle opening a file called "foobar", what we do is this: First, we
 * create an 'augmented file name', by prepending the game target and a dash,
 * so if we running game target sq1sci, the name becomes "sq1sci-foobar".
 * Next, we check if such a file is known to the SaveFileManager. If so, we
 * we use that for reading/writing, delete it, whatever.
 *
 * If no such file is present but we were only asked to *read* the file,
 * we fallback to looking for a regular file called "foobar", and open that
 * for reading only.
 */



FileHandle::FileHandle() : _in(0), _out(0) {
}

FileHandle::~FileHandle() {
	close();
}

void FileHandle::close() {
	delete _in;
	delete _out;
	_in = 0;
	_out = 0;
	_name.clear();
}

bool FileHandle::isOpen() const {
	return _in || _out;
}



enum {
	_K_FILE_MODE_OPEN_OR_CREATE = 0,
	_K_FILE_MODE_OPEN_OR_FAIL = 1,
	_K_FILE_MODE_CREATE = 2
};



reg_t file_open(EngineState *s, const Common::String &filename, int mode, bool unwrapFilename) {
	Common::String englishName = g_sci->getSciLanguageString(filename, K_LANG_ENGLISH);
	Common::String wrappedName = unwrapFilename ? g_sci->wrapFilename(englishName) : englishName;
	Common::SeekableReadStream *inFile = 0;
	Common::WriteStream *outFile = 0;
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();

	if (mode == _K_FILE_MODE_OPEN_OR_FAIL) {
		// Try to open file, abort if not possible
		inFile = saveFileMan->openForLoading(wrappedName);
		// If no matching savestate exists: fall back to reading from a regular
		// file
		if (!inFile)
			inFile = SearchMan.createReadStreamForMember(englishName);

		// Special case for LSL3: It tries to create a new dummy file,
		// LARRY3.DRV. Apparently, if the file doesn't exist here, it should be
		// created. The game scripts then go ahead and fill its contents with
		// data. It seems to be a similar case as the dummy MEMORY.DRV file in
		// LSL5, but LSL5 creates the file if it can't find it with a separate
		// call to file_open().
		if (!inFile && englishName == "LARRY3.DRV") {
			outFile = saveFileMan->openForSaving(wrappedName);
			outFile->finalize();
			delete outFile;
			outFile = 0;
			inFile = SearchMan.createReadStreamForMember(wrappedName);
		}

		if (!inFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_OPEN_OR_FAIL): failed to open file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_CREATE) {
		// Create the file, destroying any content it might have had
		outFile = saveFileMan->openForSaving(wrappedName);
		if (!outFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_CREATE): failed to create file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_OPEN_OR_CREATE) {
		// Try to open file, create it if it doesn't exist
		outFile = saveFileMan->openForSaving(wrappedName);
		if (!outFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_CREATE): failed to create file '%s'", englishName.c_str());
		// QfG1 opens the character export file with _K_FILE_MODE_CREATE first,
		// closes it immediately and opens it again with this here. Perhaps
		// other games use this for read access as well. I guess changing this
		// whole code into using virtual files and writing them after close
		// would be more appropriate.
	} else {
		error("file_open: unsupported mode %d (filename '%s')", mode, englishName.c_str());
	}

	if (!inFile && !outFile) { // Failed
		debugC(kDebugLevelFile, "  -> file_open() failed");
		return SIGNAL_REG;
	}

	// Find a free file handle
	uint handle = 1; // Ignore _fileHandles[0]
	while ((handle < s->_fileHandles.size()) && s->_fileHandles[handle].isOpen())
		handle++;

	if (handle == s->_fileHandles.size()) {
		// Hit size limit => Allocate more space
		s->_fileHandles.resize(s->_fileHandles.size() + 1);
	}

	s->_fileHandles[handle]._in = inFile;
	s->_fileHandles[handle]._out = outFile;
	s->_fileHandles[handle]._name = englishName;

	debugC(kDebugLevelFile, "  -> opened file '%s' with handle %d", englishName.c_str(), handle);
	return make_reg(0, handle);
}

reg_t kFOpen(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	int mode = argv[1].toUint16();

	debugC(kDebugLevelFile, "kFOpen(%s,0x%x)", name.c_str(), mode);
	return file_open(s, name, mode, true);
}

static FileHandle *getFileFromHandle(EngineState *s, uint handle) {
	if (handle == 0) {
		error("Attempt to use file handle 0");
		return 0;
	}

	if ((handle >= s->_fileHandles.size()) || !s->_fileHandles[handle].isOpen()) {
		warning("Attempt to use invalid/unused file handle %d", handle);
		return 0;
	}

	return &s->_fileHandles[handle];
}

reg_t kFClose(EngineState *s, int argc, reg_t *argv) {
	debugC(kDebugLevelFile, "kFClose(%d)", argv[0].toUint16());
	if (argv[0] != SIGNAL_REG) {
		FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
		if (f)
			f->close();
	}
	return s->r_acc;
}

reg_t kFPuts(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	Common::String data = s->_segMan->getString(argv[1]);

	FileHandle *f = getFileFromHandle(s, handle);
	if (f)
		f->_out->write(data.c_str(), data.size());

	return s->r_acc;
}

static int fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle) {
	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return 0;

	if (!f->_in) {
		error("fgets_wrapper: Trying to read from file '%s' opened for writing", f->_name.c_str());
		return 0;
	}
	int readBytes = 0;
	if (maxsize > 1) {
		memset(dest, 0, maxsize);
		f->_in->readLine(dest, maxsize);
		readBytes = strlen(dest); // FIXME: sierra sci returned byte count and didn't react on NUL characters
		// The returned string must not have an ending LF
		if (readBytes > 0) {
			if (dest[readBytes - 1] == 0x0A)
				dest[readBytes - 1] = 0;
		}
	} else {
		*dest = 0;
	}

	debugC(kDebugLevelFile, "  -> FGets'ed \"%s\"", dest);
	return readBytes;
}

reg_t kFGets(EngineState *s, int argc, reg_t *argv) {
	int maxsize = argv[1].toUint16();
	char *buf = new char[maxsize];
	int handle = argv[2].toUint16();

	debugC(kDebugLevelFile, "kFGets(%d, %d)", handle, maxsize);
	int readBytes = fgets_wrapper(s, buf, maxsize, handle);
	s->_segMan->memcpy(argv[0], (const byte*)buf, maxsize);
	delete[] buf;
	return readBytes ? argv[0] : NULL_REG;
}

/**
 * Writes the cwd to the supplied address and returns the address in acc.
 */
reg_t kGetCWD(EngineState *s, int argc, reg_t *argv) {
	// We do not let the scripts see the file system, instead pretending
	// we are always in the same directory.
	// TODO/FIXME: Is "/" a good value? Maybe "" or "." or "C:\" are better?
	s->_segMan->strcpy(argv[0], "/");

	debugC(kDebugLevelFile, "kGetCWD() -> %s", "/");

	return argv[0];
}

static void listSavegames(Common::Array<SavegameDesc> &saves);
static int findSavegame(Common::Array<SavegameDesc> &saves, int16 saveId);

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
		s->_segMan->strcpy(argv[0], "/");
		return s->r_acc;
	}

	int mode = argv[0].toUint16();

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE: {
		Common::String input_str = s->_segMan->getString(argv[1]);

		s->_segMan->strcpy(argv[2], "/");
		debug(3, "K_DEVICE_INFO_GET_DEVICE(%s) -> %s", input_str.c_str(), "/");
		break;
	}
	case K_DEVICE_INFO_GET_CURRENT_DEVICE:
		s->_segMan->strcpy(argv[1], "/");
		debug(3, "K_DEVICE_INFO_GET_CURRENT_DEVICE() -> %s", "/");
		break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		Common::String path1_s = s->_segMan->getString(argv[1]);
		Common::String path2_s = s->_segMan->getString(argv[2]);
		debug(3, "K_DEVICE_INFO_PATHS_EQUAL(%s,%s)", path1_s.c_str(), path2_s.c_str());

		return make_reg(0, Common::matchString(path2_s.c_str(), path1_s.c_str(), false, true));
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
	** Read http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		Common::String game_prefix = s->_segMan->getString(argv[2]);
		s->_segMan->strcpy(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVECAT_NAME(%s) -> %s", game_prefix.c_str(), "__throwaway");
		}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		Common::String game_prefix = s->_segMan->getString(argv[2]);
		uint virtualId = argv[3].toUint16();
		s->_segMan->strcpy(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVEFILE_NAME(%s,%d) -> %s", game_prefix.c_str(), virtualId, "__throwaway");
		if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
			error("kDeviceInfo(deleteSave): invalid savegame-id specified");
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

reg_t kGetSaveDir(EngineState *s, int argc, reg_t *argv) {
#ifdef ENABLE_SCI32
	// TODO: SCI32 uses a parameter here.
	if (argc > 0)
		warning("kGetSaveDir called with %d parameter(s): %04x:%04x", argc, PRINT_REG(argv[0]));
#endif
		return s->_segMan->getSaveDirPtr();
}

reg_t kCheckFreeSpace(EngineState *s, int argc, reg_t *argv) {
	if (argc > 1) {
		// SCI1.1/SCI32
		// TODO: don't know if those are right for SCI32 as well
		// Please note that sierra sci supported both calls either w/ or w/o opcode in SCI1.1
		switch (argv[1].toUint16()) {
		case 0: // return saved game size
			return make_reg(0, 0); // we return 0

		case 1: // return free harddisc space (shifted right somehow)
			return make_reg(0, 0x7fff); // we return maximum

		case 2: // same as call w/o opcode
			break;
			return make_reg(0, 1);

		default:
			error("kCheckFreeSpace: called with unknown sub-op %d", argv[1].toUint16());
		}
	}

	Common::String path = s->_segMan->getString(argv[0]);

	debug(3, "kCheckFreeSpace(%s)", path.c_str());
	// We simply always pretend that there is enough space. The alternative
	// would be to write a big test file, which is not nice on systems where
	// doing so is very slow.
	return make_reg(0, 1);
}

static bool _savegame_sort_byDate(const SavegameDesc &l, const SavegameDesc &r) {
	if (l.date != r.date)
		return (l.date > r.date);
	return (l.time > r.time);
}

// Create a sorted array containing all found savedgames
static void listSavegames(Common::Array<SavegameDesc> &saves) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();

	// Load all saves
	Common::StringArray saveNames = saveFileMan->listSavefiles(g_sci->getSavegamePattern());

	for (Common::StringArray::const_iterator iter = saveNames.begin(); iter != saveNames.end(); ++iter) {
		Common::String filename = *iter;
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename))) {
			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta) || meta.name.empty()) {
				// invalid
				delete in;
				continue;
			}
			delete in;

			SavegameDesc desc;
			desc.id = strtol(filename.end() - 3, NULL, 10);
			desc.date = meta.saveDate;
			// We need to fix date in here, because we save DDMMYYYY instead of
			// YYYYMMDD, so sorting wouldn't work
			desc.date = ((desc.date & 0xFFFF) << 16) | ((desc.date & 0xFF0000) >> 8) | ((desc.date & 0xFF000000) >> 24);
			desc.time = meta.saveTime;
			desc.version = meta.version;

			if (meta.name.lastChar() == '\n')
				meta.name.deleteLastChar();

			Common::strlcpy(desc.name, meta.name.c_str(), SCI_MAX_SAVENAME_LENGTH);

			debug(3, "Savegame in file %s ok, id %d", filename.c_str(), desc.id);

			saves.push_back(desc);
		}
	}

	// Sort the list by creation date of the saves
	Common::sort(saves.begin(), saves.end(), _savegame_sort_byDate);
}

// Find a savedgame according to virtualId and return the position within our array
static int findSavegame(Common::Array<SavegameDesc> &saves, int16 savegameId) {
	for (uint saveNr = 0; saveNr < saves.size(); saveNr++) {
		if (saves[saveNr].id == savegameId)
			return saveNr;
	}
	return -1;
}

// The scripts get IDs ranging from 100->199, because the scripts require us to assign unique ids THAT EVEN STAY BETWEEN
//  SAVES and the scripts also use "saves-count + 1" to create a new savedgame slot.
//  SCI1.1 actually recycles ids, in that case we will currently get "0".
// This behavior is required especially for LSL6. In this game, it's possible to quick save. The scripts will use
//  the last-used id for that feature. If we don't assign sticky ids, the feature will overwrite different saves all the
//  time. And sadly we can't just use the actual filename ids directly, because of the creation method for new slots.

bool Console::cmdListSaves(int argc, const char **argv) {
	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	for (uint i = 0; i < saves.size(); i++) {
		Common::String filename = g_sci->getSavegameName(saves[i].id);
		DebugPrintf("%s: '%s'\n", filename.c_str(), saves[i].name);
	}

	return true;
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

	// Find saved-game
	if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
		error("kCheckSaveGame: called with invalid savegameId");
	uint savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
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
	Common::String game_id = s->_segMan->getString(argv[0]);

	debug(3, "kGetSaveFiles(%s)", game_id.c_str());

	// Scripts ask for current save files, we can assume that if afterwards they ask us to create a new slot they really
	//  mean new slot instead of overwriting the old one
	s->_lastSaveVirtualId = SAVEGAMEID_OFFICIALRANGE_START;

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);
	uint totalSaves = MIN<uint>(saves.size(), MAX_SAVEGAME_NR);

	reg_t *slot = s->_segMan->derefRegPtr(argv[2], totalSaves);

	if (!slot) {
		warning("kGetSaveFiles: %04X:%04X invalid or too small to hold slot data", PRINT_REG(argv[2]));
		totalSaves = 0;
	}

	const uint bufSize = (totalSaves * SCI_MAX_SAVENAME_LENGTH) + 1;
	char *saveNames = new char[bufSize];
	char *saveNamePtr = saveNames;

	for (uint i = 0; i < totalSaves; i++) {
		*slot++ = make_reg(0, saves[i].id + SAVEGAMEID_OFFICIALRANGE_START); // Store the virtual savegame-id ffs. see above
		strcpy(saveNamePtr, saves[i].name);
		saveNamePtr += SCI_MAX_SAVENAME_LENGTH;
	}

	*saveNamePtr = 0; // Terminate list

	s->_segMan->memcpy(argv[1], (byte *)saveNames, bufSize);
	delete[] saveNames;

	return make_reg(0, totalSaves);
}

reg_t kSaveGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id;
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
		const EnginePlugin *plugin = NULL;
		EngineMan.findGame(g_sci->getGameIdStr(), &plugin);
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
		dialog->setSaveMode(true);
		savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
		game_description = dialog->getResultString();
		if (game_description.empty()) {
			// create our own description for the saved game, the user didnt enter it
			TimeDate curTime;
			g_system->getTimeAndDate(curTime);
			curTime.tm_year += 1900; // fixup year
			curTime.tm_mon++; // fixup month
			game_description = Common::String::format("%02d.%02d.%04d / %02d:%02d:%02d", curTime.tm_mday, curTime.tm_mon, curTime.tm_year, curTime.tm_hour, curTime.tm_min, curTime.tm_sec);
		}
		delete dialog;
		g_sci->_soundCmd->pauseAll(false); // unpause music ( we can't have it paused during save)
		if (savegameId < 0)
			return NULL_REG;

	} else {
		// Real call from script
		game_id = s->_segMan->getString(argv[0]);
		if (argv[2].isNull())
			error("kSaveGame: called with description being NULL");
		game_description = s->_segMan->getString(argv[2]);

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
			if (virtualId == s->_lastSaveVirtualId) {
				// if last virtual id is the same as this one, we assume that caller wants to overwrite last save
				savegameId = s->_lastSaveNewId;
			} else {
				uint savegameNr;
				// savegameId is in lower range, scripts expect us to create a new slot
				for (savegameId = 0; savegameId < SAVEGAMEID_OFFICIALRANGE_START; savegameId++) {
					for (savegameNr = 0; savegameNr < saves.size(); savegameNr++) {
						if (savegameId == saves[savegameNr].id)
							break;
					}
					if (savegameNr == saves.size())
						break;
				}
				if (savegameId == SAVEGAMEID_OFFICIALRANGE_START)
					error("kSavegame: no more savegame slots available");
			}
		} else {
			error("kSaveGame: invalid savegameId used");
		}

		// Save in case caller wants to overwrite last newly created save
		s->_lastSaveVirtualId = virtualId;
		s->_lastSaveNewId = savegameId;
	}

	s->r_acc = NULL_REG;

	Common::String filename = g_sci->getSavegameName(savegameId);
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::OutSaveFile *out;

	out = saveFileMan->openForSaving(filename);
	if (!out) {
		warning("Error opening savegame \"%s\" for writing", filename.c_str());
	} else {
		if (!gamestate_save(s, out, game_description, version)) {
			warning("Saving the game failed");
		} else {
			s->r_acc = TRUE_REG; // save successful
		}

		out->finalize();
		if (out->err()) {
			warning("Writing the savegame failed");
			s->r_acc = NULL_REG; // write failure
		}
		delete out;
	}

	return s->r_acc;
}

reg_t kRestoreGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = !argv[0].isNull() ? s->_segMan->getString(argv[0]) : "";
	int16 savegameId = argv[1].toSint16();
	bool pausedMusic = false;

	debug(3, "kRestoreGame(%s,%d)", game_id.c_str(), savegameId);

	if (argv[0].isNull()) {
		// Direct call, either from launcher or from a patched Game::restore
		if (savegameId == -1) {
			// we are supposed to show a dialog for the user and let him choose a saved game
			g_sci->_soundCmd->pauseAll(true); // pause music
			const EnginePlugin *plugin = NULL;
			EngineMan.findGame(g_sci->getGameIdStr(), &plugin);
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"));
			dialog->setSaveMode(false);
			savegameId = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
			delete dialog;
			if (savegameId < 0) {
				g_sci->_soundCmd->pauseAll(false); // unpause music
				return s->r_acc;
			}
			pausedMusic = true;
		}
		// don't adjust ID of the saved game, it's already correct
	} else {
		if (argv[2].isNull())
			error("kRestoreGame: called with parameter 2 being NULL");
		// Real call from script, we need to adjust ID
		if ((savegameId < SAVEGAMEID_OFFICIALRANGE_START) || (savegameId > SAVEGAMEID_OFFICIALRANGE_END)) {
			warning("Savegame ID %d is not allowed", savegameId);
			return TRUE_REG;
		}
		savegameId -= SAVEGAMEID_OFFICIALRANGE_START;
	}

	s->r_acc = NULL_REG; // signals success

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);
	if (findSavegame(saves, savegameId) == -1) {
		s->r_acc = TRUE_REG;
		warning("Savegame ID %d not found", savegameId);
	} else {
		Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
		Common::String filename = g_sci->getSavegameName(savegameId);
		Common::SeekableReadStream *in;

		in = saveFileMan->openForLoading(filename);
		if (in) {
			// found a savegame file

			gamestate_restore(s, in);
			delete in;

			if (g_sci->getGameId() == GID_MOTHERGOOSE256) {
				// WORKAROUND: Mother Goose SCI1/SCI1.1 does some weird things for
				//  saving a previously restored game.
				// We set the current savedgame-id directly and remove the script
				//  code concerning this via script patch.
				s->variables[VAR_GLOBAL][0xB3].offset = SAVEGAMEID_OFFICIALRANGE_START + savegameId;
			}
		} else {
			s->r_acc = TRUE_REG;
			warning("Savegame #%d not found", savegameId);
		}
	}

	if (!s->r_acc.isNull()) {
		// no success?
		if (pausedMusic)
			g_sci->_soundCmd->pauseAll(false); // unpause music
	}

	return s->r_acc;
}

reg_t kValidPath(EngineState *s, int argc, reg_t *argv) {
	Common::String path = s->_segMan->getString(argv[0]);

	debug(3, "kValidPath(%s) -> %d", path.c_str(), s->r_acc.offset);

	// Always return true
	return make_reg(0, 1);
}

reg_t kFileIO(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kFileIOOpen(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);

	// SCI32 can call K_FILEIO_OPEN with only one argument. It seems to
	// just be checking if it exists.
	int mode = (argc < 2) ? (int)_K_FILE_MODE_OPEN_OR_FAIL : argv[1].toUint16();
	bool unwrapFilename = true;

	// SQ4 floppy prepends /\ to the filenames
	if (name.hasPrefix("/\\")) {
		name.deleteChar(0);
		name.deleteChar(0);
	}

	// SQ4 floppy attempts to update the savegame index file sq4sg.dir when
	// deleting saved games. We don't use an index file for saving or loading,
	// so just stop the game from modifying the file here in order to avoid
	// having it saved in the ScummVM save directory.
	if (name == "sq4sg.dir") {
		debugC(kDebugLevelFile, "Not opening unused file sq4sg.dir");
		return SIGNAL_REG;
	}

	if (name.empty()) {
		// Happens many times during KQ1 (e.g. when typing something)
		debugC(kDebugLevelFile, "Attempted to open a file with an empty filename");
		return SIGNAL_REG;
	}
	debugC(kDebugLevelFile, "kFileIO(open): %s, 0x%x", name.c_str(), mode);

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

	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (f) {
		f->close();
		return SIGNAL_REG;
	}
	return NULL_REG;
}

reg_t kFileIOReadRaw(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	int size = argv[2].toUint16();
	int bytesRead = 0;
	char *buf = new char[size];
	debugC(kDebugLevelFile, "kFileIO(readRaw): %d, %d", handle, size);

	FileHandle *f = getFileFromHandle(s, handle);
	if (f) {
		bytesRead = f->_in->read(buf, size);
		s->_segMan->memcpy(argv[1], (const byte*)buf, size);
	}

	delete[] buf;
	return make_reg(0, bytesRead);
}

reg_t kFileIOWriteRaw(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	int size = argv[2].toUint16();
	char *buf = new char[size];
	bool success = false;
	s->_segMan->memcpy((byte*)buf, argv[1], size);
	debugC(kDebugLevelFile, "kFileIO(writeRaw): %d, %d", handle, size);

	FileHandle *f = getFileFromHandle(s, handle);
	if (f) {
		f->_out->write(buf, size);
		success = true;
	}

	delete[] buf;
	if (success)
		return NULL_REG;
	return make_reg(0, 6); // DOS - invalid handle
}

reg_t kFileIOUnlink(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	bool result;

	// SQ4 floppy prepends /\ to the filenames
	if (name.hasPrefix("/\\")) {
		name.deleteChar(0);
		name.deleteChar(0);
	}

	// Special case for SQ4 floppy: This game has hardcoded names for all of
	// its savegames, and they are all named "sq4sg.xxx", where xxx is the
	// slot. We just take the slot number here, and delete the appropriate
	// save game.
	if (name.hasPrefix("sq4sg.")) {
		// Special handling for SQ4... get the slot number and construct the
		// save game name.
		int slotNum = atoi(name.c_str() + name.size() - 3);
		Common::Array<SavegameDesc> saves;
		listSavegames(saves);
		int savedir_nr = saves[slotNum].id;
		name = g_sci->getSavegameName(savedir_nr);
		result = saveFileMan->removeSavefile(name);
	} else {
		const Common::String wrappedName = g_sci->wrapFilename(name);
		result = saveFileMan->removeSavefile(wrappedName);
	}

	debugC(kDebugLevelFile, "kFileIO(unlink): %s", name.c_str());
	if (result)
		return NULL_REG;
	return make_reg(0, 2); // DOS - file not found error code
}

reg_t kFileIOReadString(EngineState *s, int argc, reg_t *argv) {
	int size = argv[1].toUint16();
	char *buf = new char[size];
	int handle = argv[2].toUint16();
	debugC(kDebugLevelFile, "kFileIO(readString): %d, %d", handle, size);

	int readBytes = fgets_wrapper(s, buf, size, handle);
	s->_segMan->memcpy(argv[0], (const byte*)buf, size);
	delete[] buf;
	return readBytes ? argv[0] : NULL_REG;
}

reg_t kFileIOWriteString(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	Common::String str = s->_segMan->getString(argv[1]);
	debugC(kDebugLevelFile, "kFileIO(writeString): %d", handle);

	FileHandle *f = getFileFromHandle(s, handle);

	if (f) {
		f->_out->write(str.c_str(), str.size());
		return NULL_REG;
	}

	return make_reg(0, 6); // DOS - invalid handle
}

reg_t kFileIOSeek(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	int offset = argv[1].toUint16();
	int whence = argv[2].toUint16();
	debugC(kDebugLevelFile, "kFileIO(seek): %d, %d, %d", handle, offset, whence);

	FileHandle *f = getFileFromHandle(s, handle);

	if (f)
		s->r_acc = make_reg(0, f->_in->seek(offset, whence));

	return SIGNAL_REG;
}

void DirSeeker::addAsVirtualFiles(Common::String title, Common::String fileMask) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::StringArray foundFiles = saveFileMan->listSavefiles(fileMask);
	if (!foundFiles.empty()) {
		_files.push_back(title);
		_virtualFiles.push_back("");
		Common::StringArray::iterator it;
		Common::StringArray::iterator it_end = foundFiles.end();

		for (it = foundFiles.begin(); it != it_end; it++) {
			Common::String regularFilename = *it;
			Common::String wrappedFilename = Common::String(regularFilename.c_str() + fileMask.size() - 1);

			Common::SeekableReadStream *testfile = saveFileMan->openForLoading(regularFilename);
			int32 testfileSize = testfile->size();
			delete testfile;
			if (testfileSize > 1024) // check, if larger than 1k. in that case its a saved game.
				continue; // and we dont want to have those in the list
			// We need to remove the prefix for display purposes
			_files.push_back(wrappedFilename);
			// but remember the actual name as well
			_virtualFiles.push_back(regularFilename);
		}
	}
}

Common::String DirSeeker::getVirtualFilename(uint fileNumber) {
	if (fileNumber >= _virtualFiles.size())
		error("invalid virtual filename access");
	return _virtualFiles[fileNumber];
}

reg_t DirSeeker::firstFile(const Common::String &mask, reg_t buffer, SegManager *segMan) {
	// Verify that we are given a valid buffer
	if (!buffer.segment) {
		error("DirSeeker::firstFile('%s') invoked with invalid buffer", mask.c_str());
		return NULL_REG;
	}
	_outbuffer = buffer;
	_files.clear();
	_virtualFiles.clear();

	int QfGImport = g_sci->inQfGImportRoom();
	if (QfGImport) {
		_files.clear();
		addAsVirtualFiles("-QfG1-", "qfg1-*");
		addAsVirtualFiles("-QfG1VGA-", "qfg1vga-*");
		if (QfGImport > 2)
			addAsVirtualFiles("-QfG2-", "qfg2-*");
		if (QfGImport > 3)
			addAsVirtualFiles("-QfG3-", "qfg3-*");

		if (QfGImport == 3) {
			// QfG3 sorts the filelisting itself, we can't let that happen otherwise our
			//  virtual list would go out-of-sync
			reg_t savedHeros = segMan->findObjectByName("savedHeros");
			if (!savedHeros.isNull())
				writeSelectorValue(segMan, savedHeros, SELECTOR(sort), 0);
		}

	} else {
		// Prefix the mask
		const Common::String wrappedMask = g_sci->wrapFilename(mask);

		// Obtain a list of all files matching the given mask
		Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
		_files = saveFileMan->listSavefiles(wrappedMask);
	}

	// Reset the list iterator and write the first match to the output buffer,
	// if any.
	_iter = _files.begin();
	return nextFile(segMan);
}

reg_t DirSeeker::nextFile(SegManager *segMan) {
	if (_iter == _files.end()) {
		return NULL_REG;
	}

	Common::String string;

	if (_virtualFiles.empty()) {
		// Strip the prefix, if we don't got a virtual filelisting
		const Common::String wrappedString = *_iter;
		string = g_sci->unwrapFilename(wrappedString);
	} else {
		string = *_iter;
	}
	if (string.size() > 12)
		string = Common::String(string.c_str(), 12);
	segMan->strcpy(_outbuffer, string.c_str());

	// Return the result and advance the list iterator :)
	++_iter;
	return _outbuffer;
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

	bool exists = false;

	// Check for regular file
	exists = Common::File::exists(name);

	// Check for a savegame with the name
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	if (!exists)
		exists = !saveFileMan->listSavefiles(name).empty();

	// Try searching for the file prepending "target-"
	const Common::String wrappedName = g_sci->wrapFilename(name);
	if (!exists) {
		exists = !saveFileMan->listSavefiles(wrappedName).empty();
	}

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

	debugC(kDebugLevelFile, "kFileIO(fileExists) %s -> %d", name.c_str(), exists);
	return make_reg(0, exists);
}

reg_t kFileIORename(EngineState *s, int argc, reg_t *argv) {
	Common::String oldName = s->_segMan->getString(argv[0]);
	Common::String newName = s->_segMan->getString(argv[1]);

	// SCI1.1 returns 0 on success and a DOS error code on fail. SCI32
	// returns -1 on fail. We just return -1 for all versions.
	if (g_sci->getSaveFileManager()->renameSavefile(oldName, newName))
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
	if (f)
		f->_out->writeByte(argv[1].toUint16() & 0xff);
	return s->r_acc; // FIXME: does this really not return anything?
}

reg_t kFileIOReadWord(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (!f)
		return NULL_REG;
	return make_reg(0, f->_in->readUint16LE());
}

reg_t kFileIOWriteWord(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (f)
		f->_out->writeUint16LE(argv[1].toUint16());
	return s->r_acc; // FIXME: does this really not return anything?
}

reg_t kFileIOCreateSaveSlot(EngineState *s, int argc, reg_t *argv) {
	// Used in Shivers when the user enters his name on the guest book
	// in the beginning to start the game.

	// Creates a new save slot, and returns if the operation was successful

	// Argument 0 denotes the save slot as a negative integer, 2 means "0"
	// Argument 1 is a string, with the file name, obtained from kSave(5).
	// The interpreter checks if it can be written to (by checking for free
	// disk space and write permissions)

	// We don't really use or need any of this...

	uint16 saveSlot = argv[0].toUint16();
	char* fileName = s->_segMan->lookupString(argv[1])->getRawData();
	warning("kFileIOCreateSaveSlot(%d, '%s')", saveSlot, fileName);

	return TRUE_REG;	// slot creation was successful
}

reg_t kCD(EngineState *s, int argc, reg_t *argv) {
	// TODO: Stub
	switch (argv[0].toUint16()) {
	case 0:
		// Return whether the contents of disc argv[1] is available.
		return TRUE_REG;
	default:
		warning("CD(%d)", argv[0].toUint16());
	}

	return NULL_REG;
}

reg_t kSave(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0:
		return kSaveGame(s, argc - 1,argv + 1);
	case 1:
		return kRestoreGame(s, argc - 1,argv + 1);
	case 2:
		return kGetSaveDir(s, argc - 1, argv + 1);
	case 5:
		return kGetSaveFiles(s, argc - 1, argv + 1);
	case 8:
		// TODO
		// This is a timer callback, with 1 parameter: the timer object
		// (e.g. "timers").
		// It's used for auto-saving (i.e. save every X minutes, by checking
		// the elapsed time from the timer object)

		// This function has to return something other than 0 to proceed
		return s->r_acc;
	default:
		warning("Unknown/unhandled kSave subop %d", argv[0].toUint16());
	}

	return NULL_REG;
}

#endif

} // End of namespace Sci
