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
 * $URL$
 * $Id$
 *
 */

#include "common/archive.h"
#include "common/file.h"
#include "common/str.h"
#include "common/savefile.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"

namespace Sci {

enum {
	MAX_SAVEGAME_NR = 20 /**< Maximum number of savegames */
};

struct SavegameDesc {
	int id;
	int date;
	int time;
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



void file_open(EngineState *s, const char *filename, int mode) {
	// QfG3 character import prepends /\ to the filenames.
	if (filename[0] == '/' && filename[1] == '\\')
		filename += 2;

	Common::String englishName = s->getLanguageString(filename, K_LANG_ENGLISH);
	const Common::String wrappedName = ((Sci::SciEngine*)g_engine)->wrapFilename(englishName);
	Common::SeekableReadStream *inFile = 0;
	Common::WriteStream *outFile = 0;
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	if (mode == _K_FILE_MODE_OPEN_OR_FAIL) {
		// Try to open file, abort if not possible
		inFile = saveFileMan->openForLoading(wrappedName);
		// If no matching savestate exists: fall back to reading from a regular file
		if (!inFile)
			inFile = SearchMan.createReadStreamForMember(englishName);

		// Special case for LSL3: It tries to create a new dummy file, LARRY3.DRV
		// Apparently, if the file doesn't exist here, it should be created. The game
		// scripts then go ahead and fill its contents with data. It seems to be a similar
		// case as the dummy MEMORY.DRV file in LSL5, but LSL5 creates the file if it can't
		// find it with a separate call to file_open()
		if (!inFile && englishName == "LARRY3.DRV") {
			outFile = saveFileMan->openForSaving(wrappedName);
			outFile->finalize();
			delete outFile;
			outFile = 0;
			inFile = SearchMan.createReadStreamForMember(wrappedName);
		}

		if (!inFile)
			warning("file_open(_K_FILE_MODE_OPEN_OR_FAIL) failed to open file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_CREATE) {
		// Create the file, destroying any content it might have had
		outFile = saveFileMan->openForSaving(wrappedName);
		if (!outFile)
			warning("file_open(_K_FILE_MODE_CREATE) failed to create file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_OPEN_OR_CREATE) {
		// Try to open file, create it if it doesn't exist

		// This has been disabled, as it's not used anywhere. Furthermore, it's not
		// quite clear what should happen if the given file already exists... open
		// it for appending? Or (more likely), open it for reading *and* writing?
		// We may have to clone the file for that, etc., see also the long comment
		// at the start of this file.
		error("file_open(_K_FILE_MODE_OPEN_OR_CREATE) File creation currently not supported (filename '%s')", englishName.c_str());
	} else {
		error("file_open: unsupported mode %d (filename '%s')", mode, englishName.c_str());
	}

	if (!inFile && !outFile) { // Failed
		debug(3, "file_open() failed");
		s->r_acc = SIGNAL_REG;
		return;
	}

	// Find a free file handle
	uint handle = 1; // Ignore _fileHandles[0]
	while ((handle < s->_fileHandles.size()) && s->_fileHandles[handle].isOpen())
		handle++;

	if (handle == s->_fileHandles.size()) { // Hit size limit => Allocate more space
		s->_fileHandles.resize(s->_fileHandles.size() + 1);
	}

	s->_fileHandles[handle]._in = inFile;
	s->_fileHandles[handle]._out = outFile;
	s->_fileHandles[handle]._name = englishName;

	s->r_acc = make_reg(0, handle);

	debug(3, " -> opened file '%s' with handle %d", englishName.c_str(), handle);
}

reg_t kFOpen(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	int mode = argv[1].toUint16();

	debug(3, "kFOpen(%s,0x%x)", name.c_str(), mode);
	file_open(s, name.c_str(), mode);
	return s->r_acc;
}

static FileHandle *getFileFromHandle(EngineState *s, uint handle) {
	if (handle == 0) {
		error("Attempt to use file handle 0");
		return 0;
	}

	if ((handle >= s->_fileHandles.size()) || !s->_fileHandles[handle].isOpen()) {
		error("Attempt to use invalid/unused file handle %d", handle);
		return 0;
	}

	return &s->_fileHandles[handle];
}

reg_t kFClose(EngineState *s, int argc, reg_t *argv) {
	debug(3, "kFClose(%d)", argv[0].toUint16());
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
	getFileFromHandle(s, handle)->_out->write(data.c_str(), data.size());
	return s->r_acc;
}

static void fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle) {
	debugC(2, kDebugLevelFile, "FGets'ing %d bytes from handle %d\n", maxsize, handle);

	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	if (!f->_in) {
		error("fgets_wrapper: Trying to read from file '%s' opened for writing", f->_name.c_str());
		return;
	}
	f->_in->readLine_NEW(dest, maxsize);
	// The returned string must not have an ending LF
	int strSize = strlen(dest);
	if (strSize > 0) {
		if (dest[strSize - 1] == 0x0A)
			dest[strSize - 1] = 0;
	}

	debugC(2, kDebugLevelFile, "FGets'ed \"%s\"\n", dest);
}

static int _savegame_index_struct_compare(const void *a, const void *b) {
	const SavegameDesc *A = (const SavegameDesc *)a;
	const SavegameDesc *B = (const SavegameDesc *)b;

	if (B->date != A->date)
		return B->date - A->date;
	return B->time - A->time;
}

void listSavegames(Common::Array<SavegameDesc> &saves) {
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	// Load all saves
	Common::StringList saveNames = saveFileMan->listSavefiles(((SciEngine *)g_engine)->getSavegamePattern());

	for (Common::StringList::const_iterator iter = saveNames.begin(); iter != saveNames.end(); ++iter) {
		Common::String filename = *iter;
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename))) {
			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta)) {
				// invalid
				delete in;
				continue;
			}
			delete in;

			SavegameDesc desc;
			desc.id = strtol(filename.end() - 3, NULL, 10);
			desc.date = meta.savegame_date;
			desc.time = meta.savegame_time;
			debug(3, "Savegame in file %s ok, id %d", filename.c_str(), desc.id);

			saves.push_back(desc);
		}
	}

	// Sort the list by creation date of the saves
	qsort(saves.begin(), saves.size(), sizeof(SavegameDesc), _savegame_index_struct_compare);
}

reg_t kFGets(EngineState *s, int argc, reg_t *argv) {
	int maxsize = argv[1].toUint16();
	char *buf = new char[maxsize];
	int handle = argv[2].toUint16();

	debug(3, "kFGets(%d,%d)", handle, maxsize);
	fgets_wrapper(s, buf, maxsize, handle);
	s->_segMan->memcpy(argv[0], (const byte*)buf, maxsize);
	return argv[0];
}

/**
 * Writes the cwd to the supplied address and returns the address in acc.
 */
reg_t kGetCWD(EngineState *s, int argc, reg_t *argv) {
	// We do not let the scripts see the file system, instead pretending
	// we are always in the same directory.
	// TODO/FIXME: Is "/" a good value? Maybe "" or "." or "C:\" are better?
	s->_segMan->strcpy(argv[0], "/");

	debug(3, "kGetCWD() -> %s", "/");

	return argv[0];
}

enum {
	K_DEVICE_INFO_GET_DEVICE = 0,
	K_DEVICE_INFO_GET_CURRENT_DEVICE = 1,
	K_DEVICE_INFO_PATHS_EQUAL = 2,
	K_DEVICE_INFO_IS_FLOPPY = 3,
	K_DEVICE_INFO_GET_SAVECAT_NAME = 7,
	K_DEVICE_INFO_GET_SAVEFILE_NAME = 8
};

reg_t kDeviceInfo(EngineState *s, int argc, reg_t *argv) {
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
		int savegame_id = argv[3].toUint16();
		s->_segMan->strcpy(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVEFILE_NAME(%s,%d) -> %s", game_prefix.c_str(), savegame_id, "__throwaway");
		Common::Array<SavegameDesc> saves;
		listSavegames(saves);
		int savedir_nr = saves[savegame_id].id;
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		saveFileMan->removeSavefile(filename);
		}
		break;

	default:
		// TODO: Not all sub-commands are handled. E.g. KQ5CD calls sub-command 5
		warning("Unknown DeviceInfo() sub-command: %d", mode);
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

	return make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
}

reg_t kCheckFreeSpace(EngineState *s, int argc, reg_t *argv) {
	Common::String path = s->_segMan->getString(argv[0]);

	debug(3, "kCheckFreeSpace(%s)", path.c_str());
	// We simply always pretend that there is enough space.
	// The alternative would be to write a big test file, which is not nice
	// on systems where doing so is very slow.
	return make_reg(0, 1);
}

reg_t kCheckSaveGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);
	int savedir_nr = argv[1].toUint16();

	debug(3, "kCheckSaveGame(%s, %d)", game_id.c_str(), savedir_nr);

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	savedir_nr = saves[savedir_nr].id;

	if (savedir_nr > MAX_SAVEGAME_NR - 1) {
		return NULL_REG;
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
	Common::SeekableReadStream *in;
	if ((in = saveFileMan->openForLoading(filename))) {
		// found a savegame file

		SavegameMetadata meta;
		if (!get_savegame_metadata(in, &meta)) {
			// invalid
			s->r_acc = make_reg(0, 0);
		} else {
			s->r_acc = make_reg(0, 1);
		}
		delete in;
	} else {
		s->r_acc = make_reg(0, 1);
	}

	return s->r_acc;
}

reg_t kGetSaveFiles(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);
	reg_t nametarget = argv[1];
	reg_t *nameoffsets = s->_segMan->derefRegPtr(argv[2], 0);

	debug(3, "kGetSaveFiles(%s)", game_id.c_str());

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	s->r_acc = NULL_REG;
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (uint i = 0; i < saves.size(); i++) {
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(saves[i].id);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename))) {
			// found a savegame file

			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta)) {
				// invalid
				delete in;
				continue;
			}

			if (!meta.savegame_name.empty()) {
				if (meta.savegame_name.lastChar() == '\n')
					meta.savegame_name.deleteLastChar();

				*nameoffsets = s->r_acc; // Store savegame ID
				++s->r_acc.offset; // Increase number of files found

				nameoffsets++; // Make sure the next ID string address is written to the next pointer
				Common::String name = meta.savegame_name;
				if (name.size() > SCI_MAX_SAVENAME_LENGTH-1)
					name = Common::String(meta.savegame_name.c_str(), SCI_MAX_SAVENAME_LENGTH-1);
				s->_segMan->strcpy(nametarget, name.c_str());

				// Increase name offset pointer accordingly
				nametarget.offset += SCI_MAX_SAVENAME_LENGTH;
			}
			delete in;
		}
	}

	//free(gfname);
	s->_segMan->strcpy(nametarget, ""); // Terminate list

	return s->r_acc;
}

reg_t kSaveGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);
	int savedir_nr = argv[1].toUint16();
	int savedir_id; // Savegame ID, derived from savedir_nr and the savegame ID list
	Common::String game_description = s->_segMan->getString(argv[2]);
	Common::String version;
	if (argc > 3)
		version = s->_segMan->getString(argv[3]);

	debug(3, "kSaveGame(%s,%d,%s,%s)", game_id.c_str(), savedir_nr, game_description.c_str(), version.c_str());

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	fprintf(stderr, "savedir_nr = %d\n", savedir_nr);

	if (savedir_nr >= 0 && (uint)savedir_nr < saves.size()) {
		// Overwrite
		savedir_id = saves[savedir_nr].id;
	} else if (savedir_nr >= 0 && savedir_nr < MAX_SAVEGAME_NR) {
		uint i = 0;

		fprintf(stderr, "searching for hole\n");

		savedir_id = 0;

		// First, look for holes
		while (i < saves.size()) {
			if (saves[i].id == savedir_id) {
				++savedir_id;
				i = 0;
			} else
				++i;
		}
		if (savedir_id >= MAX_SAVEGAME_NR) {
			warning("Internal error: Free savegame ID is %d, shouldn't happen", savedir_id);
			return NULL_REG;
		}

		// This loop terminates when savedir_id is not in [x | ex. n. saves	[n].id = x]
	} else {
		warning("Savegame ID %d is not allowed", savedir_nr);
		return NULL_REG;
	}

	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_id);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out;
	if (!(out = saveFileMan->openForSaving(filename))) {
		warning("Error opening savegame \"%s\" for writing", filename.c_str());
		s->r_acc = NULL_REG;
		return NULL_REG;
	}

	if (gamestate_save(s, out, game_description.c_str(), version.c_str())) {
		warning("Saving the game failed.");
		s->r_acc = NULL_REG;
	} else {
		out->finalize();
		if (out->err()) {
			delete out;
			warning("Writing the savegame failed.");
			s->r_acc = NULL_REG;
		} else {
			delete out;
			s->r_acc = make_reg(0, 1);
		}
	}

	return s->r_acc;
}

reg_t kRestoreGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);
	int savedir_nr = argv[1].toUint16();

	debug(3, "kRestoreGame(%s,%d)", game_id.c_str(), savedir_nr);

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	savedir_nr = saves[savedir_nr].id;

	if (savedir_nr > -1) {
		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename))) {
			// found a savegame file

			EngineState *newstate = gamestate_restore(s, in);
			delete in;

			if (newstate) {
				s->successor = newstate;
				script_abort_flag = 2; // Abort current game with replay
				shrink_execution_stack(s, s->execution_stack_base + 1);
			} else {
				s->r_acc = make_reg(0, 1);
				warning("Restoring failed (game_id = '%s')", game_id.c_str());
			}
			return s->r_acc;
		}
	}

	s->r_acc = make_reg(0, 1);
	warning("Savegame #%d not found", savedir_nr);

	return s->r_acc;
}

reg_t kValidPath(EngineState *s, int argc, reg_t *argv) {
	Common::String path = s->_segMan->getString(argv[0]);

	// FIXME: For now, we only accept the (fake) root dir "/" as a valid path.
	s->r_acc = make_reg(0, path == "/");

	debug(3, "kValidPath(%s) -> %d", path.c_str(), s->r_acc.offset);

	return s->r_acc;
}

enum {
	K_FILEIO_OPEN			= 0,
	K_FILEIO_CLOSE			= 1,
	K_FILEIO_READ_RAW		= 2,
	K_FILEIO_WRITE_RAW		= 3,
	K_FILEIO_UNLINK			= 4,
	K_FILEIO_READ_STRING	= 5,
	K_FILEIO_WRITE_STRING	= 6,
	K_FILEIO_SEEK			= 7,
	K_FILEIO_FIND_FIRST		= 8,
	K_FILEIO_FIND_NEXT		= 9,
	K_FILEIO_FILE_EXISTS	= 10,
	// SCI1.1
	K_FILEIO_RENAME         = 11,
	// SCI32
	// 12?
	K_FILEIO_READ_BYTE      = 13,
	K_FILEIO_WRITE_BYTE     = 14,
	K_FILEIO_READ_WORD      = 15,
	K_FILEIO_WRITE_WORD     = 16
};


reg_t DirSeeker::firstFile(const Common::String &mask, reg_t buffer) {
	// Verify that we are given a valid buffer
	if (!buffer.segment) {
		error("DirSeeker::firstFile('%s') invoked with invalid buffer", mask.c_str());
		return NULL_REG;
	}
	_outbuffer = buffer;

	// Obtain a list of all savefiles matching the given mask
	// TODO: Modify the mask, e.g. by prefixing "TARGET-".
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	_savefiles = saveFileMan->listSavefiles(mask);

	// Reset the list iterator and write the first match to the output buffer, if any.
	_iter = _savefiles.begin();
	return nextFile();
}

reg_t DirSeeker::nextFile() {
	if (_iter == _savefiles.end()) {
		return NULL_REG;
	}

	// TODO: Transform the string back into a format usable by the SCI scripts.
	// I.e., strip any TARGET- prefix.
	Common::String string = *_iter;
	if (string.size() > 12)
		string = Common::String(string.c_str(), 12);
	_state->_segMan->strcpy(_outbuffer, string.c_str());

	// Return the result and advance the list iterator :)
	++_iter;
	return _outbuffer;
}



reg_t kFileIO(EngineState *s, int argc, reg_t *argv) {
	int func_nr = argv[0].toUint16();

	switch (func_nr) {
	case K_FILEIO_OPEN : {
		Common::String name = s->_segMan->getString(argv[1]);
		int mode = argv[2].toUint16();

		// SQ4 floppy prepends /\ to the filenames
		if (name.hasPrefix("/\\")) {
			name.deleteChar(0);
			name.deleteChar(0);
		}

		// SQ4 floppy attempts to update the savegame index file sq4sg.dir
		// when deleting saved games. We don't use an index file for saving
		// or loading, so just stop the game from modifying the file here
		// in order to avoid having it saved in the ScummVM save directory
		if (name == "sq4sg.dir") {
			debugC(2, kDebugLevelFile, "Not opening unused file sq4sg.dir");
			return SIGNAL_REG;
		}

		if (name.empty()) {
			warning("Attempted to open a file with an empty filename");
			return SIGNAL_REG;
		}
		file_open(s, name.c_str(), mode);
		debug(3, "K_FILEIO_OPEN(%s,0x%x)", name.c_str(), mode);
		break;
	}
	case K_FILEIO_CLOSE : {
		debug(3, "K_FILEIO_CLOSE(%d)", argv[1].toUint16());

		FileHandle *f = getFileFromHandle(s, argv[1].toUint16());
		if (f)
			f->close();
		break;
	}
	case K_FILEIO_READ_RAW : {
		int handle = argv[1].toUint16();
		int size = argv[3].toUint16();
		char *buf = new char[size];
		debug(3, "K_FILEIO_READ_RAW(%d,%d)", handle, size);

		s->r_acc = make_reg(0, getFileFromHandle(s, handle)->_in->read(buf, size));
		s->_segMan->memcpy(argv[2], (const byte*)buf, size);
		delete[] buf;
		break;
	}
	case K_FILEIO_WRITE_RAW : {
		int handle = argv[1].toUint16();
		int size = argv[3].toUint16();
		char *buf = new char[size];
		s->_segMan->memcpy((byte*)buf, argv[2], size);
		debug(3, "K_FILEIO_WRITE_RAW(%d,%d)", handle, size);

		getFileFromHandle(s, handle)->_out->write(buf, size);
		delete[] buf;
		break;
	}
	case K_FILEIO_UNLINK : {
		Common::String name = s->_segMan->getString(argv[1]);
		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		// SQ4 floppy prepends /\ to the filenames
		if (name.hasPrefix("/\\")) {
			name.deleteChar(0);
			name.deleteChar(0);
		}

		// Special case for SQ4 floppy: This game has hardcoded names for all of its
		// savegames, and they are all named "sq4sg.xxx", where xxx is the slot. We just
		// take the slot number here, and delete the appropriate save game
		if (name.hasPrefix("sq4sg.")) {
			// Special handling for SQ4... get the slot number and construct the save game name
			int slotNum = atoi(name.c_str() + name.size() - 3);
			Common::Array<SavegameDesc> saves;
			listSavegames(saves);
			int savedir_nr = saves[slotNum].id;
			name = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
			saveFileMan->removeSavefile(name);
		} else {
			const Common::String wrappedName = ((Sci::SciEngine*)g_engine)->wrapFilename(name);
			saveFileMan->removeSavefile(wrappedName);
		}

		debug(3, "K_FILEIO_UNLINK(%s)", name.c_str());

		// TODO/FIXME: Should we return something (like, a bool indicating
		// whether deleting the save succeeded or failed)?
		break;
	}
	case K_FILEIO_READ_STRING : {
		int size = argv[2].toUint16();
		char *buf = new char[size];
		int handle = argv[3].toUint16();
		debug(3, "K_FILEIO_READ_STRING(%d,%d)", handle, size);

		fgets_wrapper(s, buf, size, handle);
		s->_segMan->memcpy(argv[1], (const byte*)buf, size);
		delete[] buf;
		return argv[1];
	}
	case K_FILEIO_WRITE_STRING : {
		int handle = argv[1].toUint16();
		int size = argv[3].toUint16();
		Common::String str = s->_segMan->getString(argv[2]);
		debug(3, "K_FILEIO_WRITE_STRING(%d,%d)", handle, size);

		// CHECKME: Is the size parameter used at all?
		// In the LSL5 password protection it is zero, and we should
		// then write a full string. (Not sure if it should write the
		// terminating zero.)
		getFileFromHandle(s, handle)->_out->write(str.c_str(), str.size());
		break;
	}
	case K_FILEIO_SEEK : {
		int handle = argv[1].toUint16();
		int offset = argv[2].toUint16();
		int whence = argv[3].toUint16();
		debug(3, "K_FILEIO_SEEK(%d,%d,%d)", handle, offset, whence);

		s->r_acc = make_reg(0, getFileFromHandle(s, handle)->_in->seek(offset, whence));
		break;
	}
	case K_FILEIO_FIND_FIRST : {
		Common::String mask = s->_segMan->getString(argv[1]);
		reg_t buf = argv[2];
		int attr = argv[3].toUint16(); // We won't use this, Win32 might, though...
		debug(3, "K_FILEIO_FIND_FIRST(%s,0x%x)", mask.c_str(), attr);

		// QfG3 uses this mask for the character import
		if (mask == "/\\*.*")
			mask = "*.*";
#ifndef WIN32
		if (mask == "*.*")
			mask = "*"; // For UNIX
#endif
		s->r_acc = s->_dirseeker.firstFile(mask, buf);

		break;
	}
	case K_FILEIO_FIND_NEXT : {
		debug(3, "K_FILEIO_FIND_NEXT()");
		s->r_acc = s->_dirseeker.nextFile();
		break;
	}
	case K_FILEIO_FILE_EXISTS : {
		Common::String name = s->_segMan->getString(argv[1]);

		// Check for regular file
		bool exists = Common::File::exists(name);
		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		const Common::String wrappedName = ((Sci::SciEngine*)g_engine)->wrapFilename(name);

		if (!exists)
			exists = !saveFileMan->listSavefiles(name).empty();

		if (!exists) {
			// Try searching for the file prepending target-
			Common::SeekableReadStream *inFile = saveFileMan->openForLoading(wrappedName);
			exists = (inFile != 0);
			delete inFile;
		}

		// Special case for non-English versions of LSL5: The English version of LSL5 calls
		// kFileIO(), case K_FILEIO_OPEN for reading to check if memory.drv exists (which is
		// where the game's password is stored). If it's not found, it calls kFileIO() again,
		// case K_FILEIO_OPEN for writing and creates a new file. Non-English versions call
		// kFileIO(), case K_FILEIO_FILE_EXISTS instead, and fail if memory.drv can't be found.
		// We create a default memory.drv file with no password, so that the game can continue
		if (!exists && name == "memory.drv") {
			// Create a new file, and write the bytes for the empty password string inside
			byte defaultContent[] = { 0xE9, 0xE9, 0xEB, 0xE1, 0x0D, 0x0A, 0x31, 0x30, 0x30, 0x30 };
			Common::WriteStream *outFile = saveFileMan->openForSaving(wrappedName);
			for (int i = 0; i < 10; i++)
				outFile->writeByte(defaultContent[i]);
			outFile->finalize();
			delete outFile;
			exists = true;
		}

		debug(3, "K_FILEIO_FILE_EXISTS(%s) -> %d", name.c_str(), exists);
		return make_reg(0, exists);
	}
	case K_FILEIO_RENAME: {
		Common::String oldName = s->_segMan->getString(argv[1]);
		Common::String newName = s->_segMan->getString(argv[2]);

		// SCI1.1 returns 0 on success and a DOS error code on fail. SCI32 returns -1 on fail.
		// We just return -1 for all versions.
		if (g_engine->getSaveFileManager()->renameSavefile(oldName, newName))
			return NULL_REG;
		else 
			return SIGNAL_REG;
	}
#ifdef ENABLE_SCI32
	case K_FILEIO_READ_BYTE:
		// Read the byte into the low byte of the accumulator
		return make_reg(0, (s->r_acc.toUint16() & 0xff00) | getFileFromHandle(s, argv[1].toUint16())->_in->readByte());
	case K_FILEIO_WRITE_BYTE:
		getFileFromHandle(s, argv[1].toUint16())->_out->writeByte(argv[2].toUint16() & 0xff);
		break;
	case K_FILEIO_READ_WORD:
		return make_reg(0, getFileFromHandle(s, argv[1].toUint16())->_in->readUint16LE());
	case K_FILEIO_WRITE_WORD:
		getFileFromHandle(s, argv[1].toUint16())->_out->writeUint16LE(argv[2].toUint16());
		break;
#endif
	default:
		error("Unknown FileIO() sub-command: %d", func_nr);
	}

	return s->r_acc;
}

} // End of namespace Sci
