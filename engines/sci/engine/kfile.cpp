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



/*
 * Note on how file I/O is implemented: In ScummVM, one can not create/write
 * arbitrary data files, simply because many of our target platforms do not
 * support this. The only files on can create are savestates. But SCI has an
 * opcode to create and write to seemingly 'arbitrary' files.
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
 *
 * There are some caveats to this: First off, SCI apparently has no way
 * to signal that a file is supposed to be opened for reading only. For now,
 * we hackishly just assume that this is what _K_FILE_MODE_OPEN_OR_FAIL is for.
 *
 * Secondly, at least in theory, a file could be opened for both reading and
 * writing. We currently do not support this. If it turns out that we *have*
 * to support it, we could do it as follows: Initially open the file for
 * reading. If a write is attempted, store the file offset, close the file,
 * if necessary create a mirror clone (i.e., clone it into a suitably named
 * savefile), then open the file (resp. its clone for writing) and seek to the
 * correct position. If later a read is attempted, we again close and re-open.
 *
 * However, before putting any effort into implementing such an error-prone
 * scheme, we are well advised to first determine whether any game needs this
 * at all, and for what. Based on that, we can maybe come up with a better waybill
 * to provide this functionality.
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
		if (!inFile)
			warning("file_open(_K_FILE_MODE_OPEN_OR_FAIL) failed to open file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_CREATE) {
		// Create the file, destroying any content it might have had
		outFile = saveFileMan->openForSaving(wrappedName);
		if (!outFile)
			warning("file_open(_K_FILE_MODE_CREATE) failed to create file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_OPEN_OR_CREATE) {
		// Try to open file, create it if it doesn't exist

		// FIXME: I am disabling this for now, as it's not quite clear what
		// should happen if the given file already exists... open it for appending?
		// Or (more likely), open it for reading *and* writing? We may have to
		// clone the file for that, etc., see also the long comment at the start
		// of this file.
		// We really need some examples on how this is used.
		error("file_open(_K_FILE_MODE_OPEN_OR_CREATE) File creation currently not supported");
	} else {
		error("file_open: unsupported mode %d", mode);
	}

	if (!inFile && !outFile) { // Failed
		debug(3, "file_open() failed");
		s->r_acc = make_reg(0, 0xffff);
		return;
	}


#if 0
	// FIXME: The old FreeSCI code for opening a file. Left as a reference, as apparently
	// the implementation below used to work well enough.

	debugC(2, kDebugLevelFile, "Opening file %s with mode %d\n", englishName.c_str(), mode);
	if ((mode == _K_FILE_MODE_OPEN_OR_FAIL) || (mode == _K_FILE_MODE_OPEN_OR_CREATE)) {
		file = sci_fopen(englishName.c_str(), "r" FO_BINARY "+"); // Attempt to open existing file
		debugC(2, kDebugLevelFile, "Opening file %s with mode %d\n", englishName.c_str(), mode);
		if (!file) {
			debugC(2, kDebugLevelFile, "Failed. Attempting to copy from resource dir...\n");
			file = f_open_mirrored(s, englishName.c_str());
			if (file)
				debugC(2, kDebugLevelFile, "Success!\n");
			else
				debugC(2, kDebugLevelFile, "Not found.\n");
		}
	}

	if ((!file) && ((mode == _K_FILE_MODE_OPEN_OR_CREATE) || (mode == _K_FILE_MODE_CREATE))) {
		file = sci_fopen(englishName.c_str(), "w" FO_BINARY "+"); /* Attempt to create file */
		debugC(2, kDebugLevelFile, "Creating file %s with mode %d\n", englishName.c_str(), mode);
	}
	if (!file) { // Failed
		debugC(2, kDebugLevelFile, "file_open() failed\n");
		s->r_acc = make_reg(0, 0xffff);
		return;
	}
#endif

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

reg_t kFOpen(EngineState *s, int, int argc, reg_t *argv) {
	char *name = kernelDerefString(s->segMan, argv[0]);
	int mode = argv[1].toUint16();

	debug(3, "kFOpen(%s,0x%x)", name, mode);
	file_open(s, name, mode);
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

void file_close(EngineState *s, int handle) {
	debugC(2, kDebugLevelFile, "Closing file %d\n", handle);

	FileHandle *f = getFileFromHandle(s, handle);
	if (f)
		f->close();
}

reg_t kFClose(EngineState *s, int, int argc, reg_t *argv) {
	debug(3, "kFClose(%d)", argv[0].toUint16());
	file_close(s, argv[0].toUint16());
	return s->r_acc;
}

void fwrite_wrapper(EngineState *s, int handle, char *data, int length) {
	debugC(2, kDebugLevelFile, "fwrite()'ing \"%s\" to handle %d\n", data, handle);

	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	if (!f->_out) {
		error("fgets_wrapper: Trying to write to file '%s' opened for reading", f->_name.c_str());
		return;
	}

	f->_out->write(data, length);
}

reg_t kFPuts(EngineState *s, int, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	char *data = kernelDerefString(s->segMan, argv[1]);

	fwrite_wrapper(s, handle, data, strlen(data));
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

static void fread_wrapper(EngineState *s, char *dest, int bytes, int handle) {
	debugC(2, kDebugLevelFile, "fread()'ing %d bytes from handle %d\n", bytes, handle);

	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	if (!f->_in) {
		error("fread_wrapper: Trying to read from file '%s' opened for writing", f->_name.c_str());
		return;
	}

	s->r_acc = make_reg(0, f->_in->read(dest, bytes));
}

static void fseek_wrapper(EngineState *s, int handle, int offset, int whence) {
	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	if (!f->_in) {
		error("fseek_wrapper: Trying to seek in file '%s' opened for writing", f->_name.c_str());
		return;
	}

	s->r_acc = make_reg(0, f->_in->seek(offset, whence));
}

reg_t kFGets(EngineState *s, int, int argc, reg_t *argv) {
	char *dest = kernelDerefString(s->segMan, argv[0]);
	int maxsize = argv[1].toUint16();
	int handle = argv[2].toUint16();

	debug(3, "kFGets(%d,%d)", handle, maxsize);
	fgets_wrapper(s, dest, maxsize, handle);
	return argv[0];
}

/**
 * Writes the cwd to the supplied address and returns the address in acc.
 */
reg_t kGetCWD(EngineState *s, int, int argc, reg_t *argv) {
	char *targetaddr = kernelDerefString(s->segMan, argv[0]);

	// We do not let the scripts see the file system, instead pretending
	// we are always in the same directory.
	// TODO/FIXME: Is "/" a good value? Maybe "" or "." or "C:\" are better?
	strcpy(targetaddr, "/");

	debug(3, "kGetCWD() -> %s", targetaddr);

	return argv[0];
}

void delete_savegame(EngineState *s, int savedir_nr) {
	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);

	//printf("Deleting savegame '%s'\n", filename.c_str());

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	saveFileMan->removeSavefile(filename);
}

enum {
	K_DEVICE_INFO_GET_DEVICE = 0,
	K_DEVICE_INFO_GET_CURRENT_DEVICE = 1,
	K_DEVICE_INFO_PATHS_EQUAL = 2,
	K_DEVICE_INFO_IS_FLOPPY = 3,
	K_DEVICE_INFO_GET_SAVECAT_NAME = 7,
	K_DEVICE_INFO_GET_SAVEFILE_NAME = 8
};

reg_t kDeviceInfo(EngineState *s, int, int argc, reg_t *argv) {
	int mode = argv[0].toUint16();
	char *game_prefix, *input_s, *output_s;

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE:
		input_s = kernelDerefString(s->segMan, argv[1]);
		output_s = kernelDerefString(s->segMan, argv[2]);
		assert(input_s != output_s);

		strcpy(output_s, "/");
		debug(3, "K_DEVICE_INFO_GET_DEVICE(%s) -> %s", input_s, output_s);
		break;

	case K_DEVICE_INFO_GET_CURRENT_DEVICE:
		output_s = kernelDerefString(s->segMan, argv[1]);

		strcpy(output_s, "/");
		debug(3, "K_DEVICE_INFO_GET_CURRENT_DEVICE() -> %s", output_s);
		break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		char *path1_s = kernelDerefString(s->segMan, argv[1]);
		char *path2_s = kernelDerefString(s->segMan, argv[2]);
		debug(3, "K_DEVICE_INFO_PATHS_EQUAL(%s,%s)", path1_s, path2_s);

		return make_reg(0, Common::matchString(path2_s, path1_s, true));
		}
		break;

	case K_DEVICE_INFO_IS_FLOPPY:
		input_s = kernelDerefString(s->segMan, argv[1]);
		debug(3, "K_DEVICE_INFO_IS_FLOPPY(%s)", input_s);
		return NULL_REG; /* Never */

	/* SCI uses these in a less-than-portable way to delete savegames.
	** Read http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		output_s = kernelDerefString(s->segMan, argv[1]);
		game_prefix = kernelDerefString(s->segMan, argv[2]);

		sprintf(output_s, "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVECAT_NAME(%s) -> %s", game_prefix, output_s);
		}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		output_s = kernelDerefString(s->segMan, argv[1]);
		game_prefix = kernelDerefString(s->segMan, argv[2]);
		int savegame_id = argv[3].toUint16();
		sprintf(output_s, "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVEFILE_NAME(%s,%d) -> %s", game_prefix, savegame_id, output_s);
		delete_savegame(s, savegame_id);
		}
		break;

	default:
		// TODO: Not all sub-commands are handled. E.g. KQ5CD calls sub-command 5
		warning("Unknown DeviceInfo() sub-command: %d", mode);
		break;
	}

	return s->r_acc;
}

reg_t kGetSaveDir(EngineState *s, int, int argc, reg_t *argv) {
#ifdef ENABLE_SCI32
	// TODO: SCI32 uses a parameter here.
	if (argc > 0)
		warning("kGetSaveDir called with a parameter");
#endif

	return make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
}

reg_t kCheckFreeSpace(EngineState *s, int, int argc, reg_t *argv) {
	char *path = kernelDerefString(s->segMan, argv[0]);

	debug(3, "kCheckFreeSpace(%s)", path);
	// We simply always pretend that there is enough space.
	// The alternative would be to write a big test file, which is not nice
	// on systems where doing so is very slow.
	return make_reg(0, 1);
}


struct SavegameDesc {
	int id;
	int date;
	int time;
};

static int _savegame_index_struct_compare(const void *a, const void *b) {
	SavegameDesc *A = (SavegameDesc *)a;
	SavegameDesc *B = (SavegameDesc *)b;

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

reg_t kCheckSaveGame(EngineState *s, int, int argc, reg_t *argv) {
	char *game_id = kernelDerefString(s->segMan, argv[0]);
	int savedir_nr = argv[1].toUint16();

	debug(3, "kCheckSaveGame(%s, %d)", game_id, savedir_nr);

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

reg_t kGetSaveFiles(EngineState *s, int, int argc, reg_t *argv) {
	char *game_id = kernelDerefString(s->segMan, argv[0]);
	char *nametarget = kernelDerefString(s->segMan, argv[1]);
	reg_t nametarget_base = argv[1];
	reg_t *nameoffsets = kernelDerefRegPtr(s->segMan, argv[2], 0);

	debug(3, "kGetSaveFiles(%s,%s)", game_id, nametarget);

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
				strncpy(nametarget, meta.savegame_name.c_str(), SCI_MAX_SAVENAME_LENGTH); // Copy identifier string
				*(nametarget + SCI_MAX_SAVENAME_LENGTH - 1) = 0; // Make sure it's terminated
				nametarget += SCI_MAX_SAVENAME_LENGTH; // Increase name offset pointer accordingly
				nametarget_base.offset += SCI_MAX_SAVENAME_LENGTH;
			}
			delete in;
		}
	}

	//free(gfname);
	*nametarget = 0; // Terminate list

	return s->r_acc;
}

reg_t kSaveGame(EngineState *s, int, int argc, reg_t *argv) {
	char *game_id = kernelDerefString(s->segMan, argv[0]);
	int savedir_nr = argv[1].toUint16();
	int savedir_id; // Savegame ID, derived from savedir_nr and the savegame ID list
	char *game_description = kernelDerefString(s->segMan, argv[2]);
	char *version = argc > 3 ? strdup(kernelDerefString(s->segMan, argv[3])) : NULL;

	debug(3, "kSaveGame(%s,%d,%s,%s)", game_id, savedir_nr, game_description, version);
	s->game_version = version;

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

	if (gamestate_save(s, out, game_description)) {
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
	free(s->game_version);
	s->game_version = NULL;

	return s->r_acc;
}

reg_t kRestoreGame(EngineState *s, int, int argc, reg_t *argv) {
	char *game_id = kernelDerefString(s->segMan, argv[0]);
	int savedir_nr = argv[1].toUint16();

	debug(3, "kRestoreGame(%s,%d)", game_id, savedir_nr);

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
				warning("Restoring failed (game_id = '%s')", game_id);
			}
			return s->r_acc;
		}
	}

	s->r_acc = make_reg(0, 1);
	warning("Savegame #%d not found", savedir_nr);

	return s->r_acc;
}

reg_t kValidPath(EngineState *s, int, int argc, reg_t *argv) {
	const char *path = kernelDerefString(s->segMan, argv[0]);

	// FIXME: For now, we only accept the (fake) root dir "/" as a valid path.
	s->r_acc = make_reg(0, 0 == strcmp(path, "/"));

	debug(3, "kValidPath(%s) -> %d", path, s->r_acc.offset);

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
	K_FILEIO_FILE_EXISTS	= 10
};


void DirSeeker::firstFile(const char *mask, reg_t buffer) {

	// Verify that we are given a valid buffer
	if (!buffer.segment) {
		error("DirSeeker::firstFile('%s') invoked with invalid buffer", mask);
		_vm->r_acc = NULL_REG;
		return;
	}
	_outbuffer = buffer;

	// Obtain a list of all savefiles matching the given mask
	// TODO: Modify the mask, e.g. by prefixing "TARGET-".
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	_savefiles = saveFileMan->listSavefiles(mask);

	// Reset the list iterator and write the first match to the output buffer, if any.
	_iter = _savefiles.begin();
	nextFile();
}

void DirSeeker::nextFile() {
	if (_iter == _savefiles.end()) {
		_vm->r_acc = NULL_REG;
		return;
	}

	char *mem = kernelDerefString(_vm->segMan, _outbuffer);
	memset(mem, 0, 13);

	// TODO: Transform the string back into a format usable by the SCI scripts.
	// I.e., strip any TARGET- prefix.
	const char *string = _iter->c_str();
	assert(string);
	strncpy(mem, string, 12);

	// Return the result and advance the list iterator :)
	_vm->r_acc = _outbuffer;
	++_iter;
}



reg_t kFileIO(EngineState *s, int, int argc, reg_t *argv) {
	int func_nr = argv[0].toUint16();

	switch (func_nr) {
	case K_FILEIO_OPEN : {
		char *name = kernelDerefString(s->segMan, argv[1]);
		int mode = argv[2].toUint16();

		file_open(s, name, mode);
		debug(3, "K_FILEIO_OPEN(%s,0x%x)", name, mode);
		break;
	}
	case K_FILEIO_CLOSE : {
		int handle = argv[1].toUint16();
		debug(3, "K_FILEIO_CLOSE(%d)", handle);

		file_close(s, handle);
		break;
	}
	case K_FILEIO_READ_RAW : {
		int handle = argv[1].toUint16();
		char *dest = kernelDerefString(s->segMan, argv[2]);
		int size = argv[3].toUint16();
		debug(3, "K_FILEIO_READ_RAW(%d,%d)", handle, size);

		fread_wrapper(s, dest, size, handle);
		break;
	}
	case K_FILEIO_WRITE_RAW : {
		int handle = argv[1].toUint16();
		char *buf = kernelDerefString(s->segMan, argv[2]);
		int size = argv[3].toUint16();
		debug(3, "K_FILEIO_WRITE_RAW(%d,%d)", handle, size);

		fwrite_wrapper(s, handle, buf, size);
		break;
	}
	case K_FILEIO_UNLINK : {
		char *name = kernelDerefString(s->segMan, argv[1]);
		debug(3, "K_FILEIO_UNLINK(%s)", name);

		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		const Common::String wrappedName = ((Sci::SciEngine*)g_engine)->wrapFilename(name);
		saveFileMan->removeSavefile(wrappedName);
		// TODO/FIXME: Should we return something (like, a bool indicating
		// whether deleting the save succeeded or failed)?
		break;
	}
	case K_FILEIO_READ_STRING : {
		char *dest = kernelDerefString(s->segMan, argv[1]);
		int size = argv[2].toUint16();
		int handle = argv[3].toUint16();
		debug(3, "K_FILEIO_READ_STRING(%d,%d)", handle, size);

		fgets_wrapper(s, dest, size, handle);
		return argv[1];
	}
	case K_FILEIO_WRITE_STRING : {
		int handle = argv[1].toUint16();
		int size = argv[3].toUint16();
		char *buf = kernelDerefString(s->segMan, argv[2], size);
		debug(3, "K_FILEIO_WRITE_STRING(%d,%d)", handle, size);

		// FIXME: What is the difference between K_FILEIO_WRITE_STRING and
		// K_FILEIO_WRITE_RAW? Normally, I would expect the difference to
		// be that the former doesn't receive a 'size' parameter. But here
		// it does. Are we missing something?
		if (buf)
			fwrite_wrapper(s, handle, buf, size);
		break;
	}
	case K_FILEIO_SEEK : {
		int handle = argv[1].toUint16();
		int offset = argv[2].toUint16();
		int whence = argv[3].toUint16();
		debug(3, "K_FILEIO_SEEK(%d,%d,%d)", handle, offset, whence);

		fseek_wrapper(s, handle, offset, whence);
		break;
	}
	case K_FILEIO_FIND_FIRST : {
		char *mask = kernelDerefString(s->segMan, argv[1]);
		reg_t buf = argv[2];
		int attr = argv[3].toUint16(); // We won't use this, Win32 might, though...
		debug(3, "K_FILEIO_FIND_FIRST(%s,0x%x)", mask, attr);

#ifndef WIN32
		if (strcmp(mask, "*.*") == 0)
			strcpy(mask, "*"); // For UNIX
#endif
		s->_dirseeker.firstFile(mask, buf);

		break;
	}
	case K_FILEIO_FIND_NEXT : {
		debug(3, "K_FILEIO_FIND_NEXT()");
		s->_dirseeker.nextFile();
		break;
	}
	case K_FILEIO_FILE_EXISTS : {
		char *name = kernelDerefString(s->segMan, argv[1]);

		// Check for regular file
		bool exists = Common::File::exists(name);

		if (!exists) {
			// TODO: Transform the name given by the scripts to us, e.g. by
			// prepending TARGET-
			Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
			exists = !saveFileMan->listSavefiles(name).empty();
		}

		debug(3, "K_FILEIO_FILE_EXISTS(%s) -> %d", name, exists);
		return make_reg(0, exists);
	}
	default :
		error("Unknown FileIO() sub-command: %d", func_nr);
	}

	return s->r_acc;
}

} // End of namespace Sci
