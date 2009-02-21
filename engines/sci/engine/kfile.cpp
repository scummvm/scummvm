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

#ifdef WIN32
#  include <windows.h>
#elif defined (__DC__)
#  include <dc.h>
#endif

#include "common/str.h"
#include "common/savefile.h"

#include "sci/include/engine.h"
#include "sci/sci.h"

#include <errno.h>

namespace Sci {

#ifndef O_BINARY
#define O_BINARY 0
#endif

static int _savegame_indices_nr = -1; // means 'uninitialized'

static struct _savegame_index_struct {
	int id;
	int date;
	int time;
} _savegame_indices[MAX_SAVEGAME_NR];

// This assumes modern stream implementations. It may break on DOS.


/* Attempts to mirror a file by copying it from the resource firectory
** to the working directory. Returns NULL if the file didn't exist.
** Otherwise, the new file is then opened for reading or writing.
*/
static FILE *f_open_mirrored(EngineState *s, char *fname) {
	int fd;
	char *buf = NULL;
	int fsize;


	printf("f_open_mirrored(%s)\n", fname);
#if 0
	// TODO/FIXME: Use s->resource_dir to locate the file???
	File file;
	if (!file.open(fname))
		return NULL;

	fsize = file.size();
	if (fsize > 0) {
		buf = (char *)sci_malloc(fsize);
		file.read(buf, fsize);
	}

	file.close();

	....
	copy the file to a savegame -> only makes sense to perform this change
	if we at the same time change the code for loading files to look among the
	savestates, and also change *all* file writing code to write to savestates,
	as it should
	...
#endif

	chdir(s->resource_dir);
	fd = sci_open(fname, O_RDONLY | O_BINARY);
	if (!IS_VALID_FD(fd)) {
		chdir(s->work_dir);
		return NULL;
	}

	fsize = sci_fd_size(fd);
	if (fsize > 0) {
		buf = (char*)sci_malloc(fsize);
		read(fd, buf, fsize);
	}

	close(fd);

	chdir(s->work_dir);

	// Visual C++ doesn't allow to specify O_BINARY with creat()
#ifdef _MSC_VER
	fd = _open(fname, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
#else
	fd = open(fname, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);
#endif

	if (!IS_VALID_FD(fd) && buf) {
		free(buf);
		sciprintf("kfile.c: f_open_mirrored(): Warning: Could not create '%s' in '%s' (%d bytes to copy)\n", fname, s->work_dir, fsize);
		return NULL;
	}

	if (fsize) {
		int ret;
		ret = write(fd, buf, fsize);
		if (ret < fsize) {
			sciprintf("kfile.c: f_open_mirrored(): Warning: Could not write all %ld bytes to '%s' in '%s' (only wrote %d)\n",
			          (long)fsize, fname, s->work_dir, ret);
		}

		free(buf);
	}

	close(fd);

	return sci_fopen(fname, "r" FO_BINARY "+");
}

#define _K_FILE_MODE_OPEN_OR_CREATE 0
#define _K_FILE_MODE_OPEN_OR_FAIL 1
#define _K_FILE_MODE_CREATE 2

void file_open(EngineState *s, char *filename, int mode) {
	int retval = 1; // Ignore file_handles[0]
	FILE *file = NULL;

	SCIkdebug(SCIkFILE, "Opening file %s with mode %d\n", filename, mode);
	if ((mode == _K_FILE_MODE_OPEN_OR_FAIL) || (mode == _K_FILE_MODE_OPEN_OR_CREATE)) {
		file = sci_fopen(filename, "r" FO_BINARY "+"); // Attempt to open existing file
		SCIkdebug(SCIkFILE, "Opening file %s with mode %d\n", filename, mode);
		if (!file) {
			SCIkdebug(SCIkFILE, "Failed. Attempting to copy from resource dir...\n");
			file = f_open_mirrored(s, filename);
			if (file)
				SCIkdebug(SCIkFILE, "Success");
			else
				SCIkdebug(SCIkFILE, "Not found.\n");
		}
	}

	if ((!file) && ((mode == _K_FILE_MODE_OPEN_OR_CREATE) || (mode == _K_FILE_MODE_CREATE))) {
		file = sci_fopen(filename, "w" FO_BINARY "+"); /* Attempt to create file */
		SCIkdebug(SCIkFILE, "Creating file %s with mode %d\n", filename, mode);
	}
	if (!file) { // Failed
		SCIkdebug(SCIkFILE, "file_open() failed\n");
		s->r_acc = make_reg(0, 0xffff);
		return;
	}

	while (s->file_handles[retval] && (retval < s->file_handles_nr))
		retval++;

	if (retval == s->file_handles_nr) // Hit size limit => Allocate more space
		s->file_handles = (FILE**)sci_realloc(s->file_handles, sizeof(FILE *) * ++(s->file_handles_nr));

	s->file_handles[retval] = file;

	s->r_acc = make_reg(0, retval);
}

reg_t kFOpen(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *name = kernel_dereference_char_pointer(s, argv[0], 0);
	int mode = UKPV(1);

	file_open(s, name, mode);
	return s->r_acc;
}

static FILE *getFileFromHandle(EngineState *s, int handle) {
	if (handle == 0) {
		error("Attempt to use file handle 0");
		return 0;
	}

	if ((handle >= s->file_handles_nr) || (s->file_handles[handle] == NULL)) {
		error("Attempt to use invalid/unused file handle %d", handle);
		return 0;
	}
	
	return s->file_handles[handle];
}

void file_close(EngineState *s, int handle) {
	SCIkdebug(SCIkFILE, "Closing file %d\n", handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	fclose(f);

	s->file_handles[handle] = NULL;
}

reg_t kFClose(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	file_close(s, UKPV(0));
	return s->r_acc;
}

void fputs_wrapper(EngineState *s, int handle, int size, char *data) {
	SCIkdebug(SCIkFILE, "FPuts'ing \"%s\" to handle %d\n", data, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (f)
		fwrite(data, 1, size, f);
}

void fwrite_wrapper(EngineState *s, int handle, char *data, int length) {
	SCIkdebug(SCIkFILE, "fwrite()'ing \"%s\" to handle %d\n", data, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (f)
		fwrite(data, 1, length, f);
}

reg_t kFPuts(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int handle = UKPV(0);
	char *data = kernel_dereference_char_pointer(s, argv[1], 0);

	fputs_wrapper(s, handle, strlen(data), data);
	return s->r_acc;
}

static void fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle) {
	SCIkdebug(SCIkFILE, "FGets'ing %d bytes from handle %d\n", maxsize, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	fgets(dest, maxsize, f);

	SCIkdebug(SCIkFILE, "FGets'ed \"%s\"\n", dest);
}

static void fread_wrapper(EngineState *s, char *dest, int bytes, int handle) {
	SCIkdebug(SCIkFILE, "fread()'ing %d bytes from handle %d\n", bytes, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	s->r_acc = make_reg(0, fread(dest, 1, bytes, f));
}

static void fseek_wrapper(EngineState *s, int handle, int offset, int whence) {
	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	s->r_acc = make_reg(0, fseek(f, offset, whence));
}

#define TEST_DIR_OR_QUIT(dir) if (!dir) { return NULL_REG; }

reg_t kFGets(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *dest = kernel_dereference_char_pointer(s, argv[0], 0);
	int maxsize = UKPV(1);
	int handle = UKPV(2);

	fgets_wrapper(s, dest, maxsize, handle);
	return argv[0];
}

/* kGetCWD(address):
** Writes the cwd to the supplied address and returns the address in acc.
*/
reg_t kGetCWD(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *wd = sci_getcwd();
	char *targetaddr = kernel_dereference_char_pointer(s, argv[0], 0);

	strncpy(targetaddr, wd, MAX_SAVE_DIR_SIZE - 1);
	targetaddr[MAX_SAVE_DIR_SIZE - 1] = 0; // Terminate

	SCIkdebug(SCIkFILE, "Copying cwd='%s'(%d chars) to %p", wd, strlen(wd), targetaddr);

	free(wd);
	return argv[0];
}

void delete_savegame(EngineState *s, int savedir_nr) {
	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);

	sciprintf("Deleting savegame '%s'\n", filename.c_str());

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	saveFileMan->removeSavefile(filename.c_str());
}

#define K_DEVICE_INFO_GET_DEVICE 0
#define K_DEVICE_INFO_GET_CURRENT_DEVICE 1
#define K_DEVICE_INFO_PATHS_EQUAL 2
#define K_DEVICE_INFO_IS_FLOPPY 3
#define K_DEVICE_INFO_GET_SAVECAT_NAME 7
#define K_DEVICE_INFO_GET_SAVEFILE_NAME 8

#ifdef WIN32

reg_t kDeviceInfo_Win32(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char dir_buffer[MAXPATHLEN], dir_buffer2[MAXPATHLEN];
	int mode = UKPV(0);

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE: {
		char *input_s = (char*)kernel_dereference_bulk_pointer(s, argv[1], 0);
		char *output_s = (char*)kernel_dereference_bulk_pointer(s, argv[2], 0);

		GetFullPathName(input_s, sizeof(dir_buffer) - 1, dir_buffer, NULL);
		strncpy(output_s, dir_buffer, 2);
		output_s [2] = 0;
	}
	break;

	case K_DEVICE_INFO_GET_CURRENT_DEVICE: {
		char *output_s = (char*)kernel_dereference_bulk_pointer(s, argv[1], 0);

		_getcwd(dir_buffer, sizeof(dir_buffer) - 1);
		strncpy(output_s, dir_buffer, 2);
		output_s [2] = 0;
	}
	break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		char *path1_s = (char*)kernel_dereference_bulk_pointer(s, argv[1], 0);
		char *path2_s = (char*)kernel_dereference_bulk_pointer(s, argv[2], 0);

		GetFullPathName(path1_s, sizeof(dir_buffer) - 1, dir_buffer, NULL);
		GetFullPathName(path2_s, sizeof(dir_buffer2) - 1, dir_buffer2, NULL);

		return make_reg(0, !scumm_stricmp(path1_s, path2_s));
	}
	break;

	case K_DEVICE_INFO_IS_FLOPPY: {
		char *input_s = (char*)kernel_dereference_bulk_pointer(s, argv[1], 0);

		GetFullPathName(input_s, sizeof(dir_buffer) - 1, dir_buffer, NULL);
		dir_buffer [3] = 0;  /* leave X:\ */

		return make_reg(0, GetDriveType(dir_buffer) == DRIVE_REMOVABLE);
	}
	break;

	/* SCI uses these in a less-than-portable way to delete savegames.
	** Read http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		char *output_buffer = kernel_dereference_char_pointer(s, argv[1], 0);
		char *game_prefix = kernel_dereference_char_pointer(s, argv[2], 0);

		sprintf(output_buffer, "%s/__throwaway", s->work_dir);
	}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		char *output_buffer = kernel_dereference_char_pointer(s, argv[1], 0);
		char *game_prefix = kernel_dereference_char_pointer(s, argv[2], 0);
		int savegame_id = UKPV(3);
		sprintf(output_buffer, "%s/__throwaway", s->work_dir);
		delete_savegame(s, savegame_id);
	}
	break;
	default: {
		error("Unknown DeviceInfo() sub-command: %d", mode);
	}
	}
	return s->r_acc;
}

#else // !WIN32

reg_t kDeviceInfo_Unix(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int mode = UKPV(0);

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE: {
		char *output_s = kernel_dereference_char_pointer(s, argv[2], 0);

		strcpy(output_s, "/");
	}
	break;

	case K_DEVICE_INFO_GET_CURRENT_DEVICE: {
		char *output_s = kernel_dereference_char_pointer(s, argv[1], 0);

		strcpy(output_s, "/");
	}
	break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		char *path1_s = kernel_dereference_char_pointer(s, argv[1], 0);
		char *path2_s = kernel_dereference_char_pointer(s, argv[2], 0);

		//return make_reg(0, !strcmp(path1_s, path2_s));
		return make_reg(0, Common::matchString(path2_s, path1_s, true));
	}
	break;

	case K_DEVICE_INFO_IS_FLOPPY: {

		return NULL_REG; /* Never */
	}
	break;

	/* SCI uses these in a less-than-portable way to delete savegames.
	** Read http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		char *output_buffer = kernel_dereference_char_pointer(s, argv[1], 0);
		//char *game_prefix = kernel_dereference_char_pointer(s, argv[2], 0);

		sprintf(output_buffer, "%s/__throwaway", s->work_dir);
	}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		char *output_buffer = kernel_dereference_char_pointer(s, argv[1], 0);
		//char *game_prefix = kernel_dereference_char_pointer(s, argv[2], 0);
		int savegame_id = UKPV(3);
		sprintf(output_buffer, "%s/__throwaway", s->work_dir);
		delete_savegame(s, savegame_id);
	}
	break;
	default: {
		error("Unknown DeviceInfo() sub-command: %d", mode);
	}
	}

	return s->r_acc;
}

#endif // !WIN32

reg_t kGetSaveDir(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
}

reg_t kCheckFreeSpace(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *path = kernel_dereference_char_pointer(s, argv[0], 0);
	char *testpath = (char*)sci_malloc(strlen(path) + 15);
	char buf[1024];
	int fd;
	int failed = 0;
	int pathlen;

	strcpy(testpath, path);
	strcat(testpath, "freesci.foo");
	pathlen = strlen(testpath);

	while (IS_VALID_FD(fd = open(testpath, O_RDONLY))) {
		close(fd);
		if (testpath[pathlen - 2] == 'z') { // Failed.
			warning("Failed to find non-existing file for free space test");
			free(testpath);
			return NULL_REG;
		}

		/* If this file couldn't be created, try freesci.fop, freesci.foq etc.,
		** then freesci.fpa, freesci.fpb. Stop at freesci.fza.
		** Yes, this is extremely arbitrary and very strange.
		*/
		if (testpath[pathlen - 1] == 'z') {
			testpath[pathlen - 1] = 'a';
			++testpath[pathlen - 2];
		} else
			++testpath[pathlen - 1];
	}

	fd = open(testpath, O_CREAT | O_BINARY | O_RDWR, S_IREAD | S_IWRITE);

	if (!IS_VALID_FD(fd)) {
		warning("Could not test for disk space: %s", strerror(errno));
		warning("Test path was '%s'", testpath);
		free(testpath);
		return NULL_REG;
	}

	memset(buf, 0, sizeof(buf));
	for (int i = 0; i < 1024; i++) // Check for 1 MB
		if (write(fd, buf, 1024) < 1024)
			failed = 1;

	close(fd);
	remove(testpath);
	free(testpath);

	return make_reg(0, !failed);
}

int _k_check_file(char *filename, int minfilesize) {
	// Returns 0 if the file exists and is big enough
	return (sci_file_size(filename) < minfilesize);
}

static int _savegame_index_struct_compare(const void *a, const void *b) {
	struct _savegame_index_struct *A = (struct _savegame_index_struct *)a;
	struct _savegame_index_struct *B = (struct _savegame_index_struct *)b;

	if (B->date != A->date)
		return B->date - A->date;
	return B->time - A->time;
}

static void update_savegame_indices() {
	int i;

	_savegame_indices_nr = 0;

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (i = 0; i < MAX_SAVEGAME_NR; i++) {
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(i);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename.c_str()))) {
			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta)) {
				// invalid
				delete in;
				continue;
			}
			delete in;

			fprintf(stderr, "Savegame in %s file ok\n", filename.c_str());
			_savegame_indices[_savegame_indices_nr].id = i;
			_savegame_indices[_savegame_indices_nr].date = meta.savegame_date;
			_savegame_indices[_savegame_indices_nr].time = meta.savegame_time;
			_savegame_indices_nr++;
		}
	}

	qsort(_savegame_indices, _savegame_indices_nr, sizeof(struct _savegame_index_struct), _savegame_index_struct_compare);
}

reg_t kCheckSaveGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	//char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);

	if (_savegame_indices_nr < 0) {
		warning("Savegame index list not initialized");
		update_savegame_indices();
	}

	savedir_nr = _savegame_indices[savedir_nr].id;

	if (savedir_nr > MAX_SAVEGAME_NR - 1) {
		return NULL_REG;
	}

	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
	Common::SeekableReadStream *in;
	if ((in = saveFileMan->openForLoading(filename.c_str()))) {
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

reg_t kGetSaveFiles(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	//char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	char *nametarget = kernel_dereference_char_pointer(s, argv[1], 0);
	reg_t nametarget_base = argv[1];
	reg_t *nameoffsets = kernel_dereference_reg_pointer(s, argv[2], 0);
	int i;

	update_savegame_indices();

	s->r_acc = NULL_REG;
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();

	for (i = 0; i < _savegame_indices_nr; i++) {
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(_savegame_indices[i].id);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename.c_str()))) {
			// found a savegame file

			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta)) {
				// invalid
				delete in;
				continue;
			}

			char namebuf[SCI_MAX_SAVENAME_LENGTH]; // Save game name buffer
			strncpy(namebuf, meta.savegame_name, SCI_MAX_SAVENAME_LENGTH);
			namebuf[SCI_MAX_SAVENAME_LENGTH-1] = 0;
			
			if (strlen(namebuf) > 0) {
				if (namebuf[strlen(namebuf) - 1] == '\n')
					namebuf[strlen(namebuf) - 1] = 0; // Remove trailing newline

				*nameoffsets = s->r_acc; // Store savegame ID
				++s->r_acc.offset; // Increase number of files found

				nameoffsets++; // Make sure the next ID string address is written to the next pointer
				strncpy(nametarget, namebuf, SCI_MAX_SAVENAME_LENGTH); // Copy identifier string
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

reg_t kSaveGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	//char *game_id = (char*)kernel_dereference_bulk_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);
	int savedir_id; // Savegame ID, derived from savedir_nr and the savegame ID list
	char *game_description = (char*)kernel_dereference_bulk_pointer(s, argv[2], 0);
	char *version = argc > 3 ? strdup((char*)kernel_dereference_bulk_pointer(s, argv[3], 0)) : NULL;

	s->game_version = version;

	update_savegame_indices();

	fprintf(stderr, "savedir_nr = %d\n", savedir_nr);

	if (savedir_nr >= 0 && savedir_nr < _savegame_indices_nr)
		// Overwrite
		savedir_id = _savegame_indices[savedir_nr].id;
	else if (savedir_nr >= 0 && savedir_nr < MAX_SAVEGAME_NR) {
		int i = 0;

		fprintf(stderr, "searching for hole\n");

		savedir_id = 0;

		// First, look for holes
		while (i < _savegame_indices_nr) {
			if (_savegame_indices[i].id == savedir_id) {
				++savedir_id;
				i = 0;
			} else ++i;
		}
		if (savedir_id >= MAX_SAVEGAME_NR) {
			sciprintf("Internal error: Free savegame ID is %d, shouldn't happen", savedir_id);
			return NULL_REG;
		}

		// This loop terminates when savedir_id is not in [x | ex. n. _savegame_indices[n].id = x]
	} else {
		sciprintf("Savegame ID %d is not allowed", savedir_nr);
		return NULL_REG;
	}

	Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_id);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out;
	if (!(out = saveFileMan->openForSaving(filename.c_str()))) {
		sciprintf("Error opening savegame \"%s\" for writing\n", filename.c_str());
		s->r_acc = NULL_REG;
		return NULL_REG;
	}

	if (gamestate_save(s, out, game_description)) {
		sciprintf("Saving the game failed.\n");
		s->r_acc = NULL_REG;
	} else {
		out->finalize();
		if (out->err()) {
			delete out;
			sciprintf("Writing the savegame failed.\n");
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

reg_t kRestoreGame(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = (char*)kernel_dereference_bulk_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);

	if (_savegame_indices_nr < 0) {
		warning("Savegame index list not initialized");
		update_savegame_indices();
	}

	savedir_nr = _savegame_indices[savedir_nr].id;

	if (savedir_nr > -1) {
		Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
		Common::String filename = ((Sci::SciEngine*)g_engine)->getSavegameName(savedir_nr);
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename.c_str()))) {
			// found a savegame file

			EngineState *newstate = gamestate_restore(s, in);
			delete in;

			if (newstate) {
				s->successor = newstate;
				script_abort_flag = SCRIPT_ABORT_WITH_REPLAY; // Abort current game
				s->execution_stack_pos = s->execution_stack_base;
			} else {
				s->r_acc = make_reg(0, 1);
				sciprintf("Restoring failed (game_id = '%s').\n", game_id);
			}
			return s->r_acc;
		}
	}

	s->r_acc = make_reg(0, 1);
	sciprintf("Savegame #%d not found!\n", savedir_nr);

	return s->r_acc;
}

reg_t kValidPath(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	char *pathname = kernel_dereference_char_pointer(s, argv[0], 0);
	char cpath[MAXPATHLEN + 1];
	getcwd(cpath, MAXPATHLEN + 1);

	s->r_acc = make_reg(0, !chdir(pathname)); // Try to go there. If it works, return 1, 0 otherwise.
	chdir(cpath);

	return s->r_acc;
}

#define K_FILEIO_OPEN 		0
#define K_FILEIO_CLOSE		1
#define K_FILEIO_READ_RAW	2
#define K_FILEIO_WRITE_RAW	3
#define K_FILEIO_UNLINK		4
#define K_FILEIO_READ_STRING	5
#define K_FILEIO_WRITE_STRING	6
#define K_FILEIO_SEEK		7
#define K_FILEIO_FIND_FIRST	8
#define K_FILEIO_FIND_NEXT	9
#define K_FILEIO_STAT		10

char * write_filename_to_mem(EngineState *s, reg_t address, char *string) {
	char *mem = kernel_dereference_char_pointer(s, address, 0);

	if (string) {
		memset(mem, 0, 13);
		strncpy(mem, string, 12);
	}

	return string;
}

void next_file(EngineState *s) {
	if (write_filename_to_mem(s, s->dirseeker_outbuffer, sci_find_next(&(s->dirseeker))))
		s->r_acc = s->dirseeker_outbuffer;
	else
		s->r_acc = NULL_REG;
}

void first_file(EngineState *s, const char *dir, char *mask, reg_t buffer) {
	if (!buffer.segment) {
		sciprintf("Warning: first_file(state,\"%s\",\"%s\", 0) invoked", dir, mask);
		s->r_acc = NULL_REG;
		return;
	}

	if (strcmp(dir, ".")) {
		sciprintf("%s L%d: Non-local first_file: Not implemented yet\n", __FILE__, __LINE__);
		s->r_acc = NULL_REG;
		return;
	}

	// Get rid of the old find structure
	if (s->dirseeker_outbuffer.segment)
		sci_finish_find(&(s->dirseeker));

	s->dirseeker_outbuffer = buffer;

	if (write_filename_to_mem(s, s->dirseeker_outbuffer, sci_find_first(&(s->dirseeker), mask)))
		s->r_acc = s->dirseeker_outbuffer;
	else
		s->r_acc = NULL_REG;
}

reg_t kFileIO(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int func_nr = UKPV(0);

	switch (func_nr) {
	case K_FILEIO_OPEN : {
		char *name = kernel_dereference_char_pointer(s, argv[1], 0);
		int mode = UKPV(2);

		file_open(s, name, mode);
		break;
	}
	case K_FILEIO_CLOSE : {
		int handle = UKPV(1);

		file_close(s, handle);
		break;
	}
	case K_FILEIO_READ_RAW : {
		int handle = UKPV(1);
		char *dest = kernel_dereference_char_pointer(s, argv[2], 0);
		int size = UKPV(3);

		fread_wrapper(s, dest, size, handle);
		break;
	}
	case K_FILEIO_WRITE_RAW : {
		int handle = UKPV(1);
		char *buf = kernel_dereference_char_pointer(s, argv[2], 0);
		int size = UKPV(3);

		fwrite_wrapper(s, handle, buf, size);
		break;
	}
	case K_FILEIO_UNLINK : {
		char *name = kernel_dereference_char_pointer(s, argv[1], 0);

		unlink(name);
		break;
	}
	case K_FILEIO_READ_STRING : {
		char *dest = kernel_dereference_char_pointer(s, argv[1], 0);
		int size = UKPV(2);
		int handle = UKPV(3);

		fgets_wrapper(s, dest, size, handle);
		return argv[1];
	}
	case K_FILEIO_WRITE_STRING : {
		int handle = UKPV(1);
		int size = UKPV(3);
		char *buf = kernel_dereference_char_pointer(s, argv[2], size);

		if (buf)
			fputs_wrapper(s, handle, size, buf);
		break;
	}
	case K_FILEIO_SEEK : {
		int handle = UKPV(1);
		int offset = UKPV(2);
		int whence = UKPV(3);

		fseek_wrapper(s, handle, offset, whence);
		break;
	}
	case K_FILEIO_FIND_FIRST : {
		char *mask = kernel_dereference_char_pointer(s, argv[1], 0);
		reg_t buf = argv[2];
		// int attr = UKPV(3); */ /* We won't use this, Win32 might, though...

#ifndef WIN32
		if (strcmp(mask, "*.*") == 0)
			strcpy(mask, "*"); // For UNIX
#endif
		first_file(s, ".", mask, buf);

		break;
	}
	case K_FILEIO_FIND_NEXT : {
		next_file(s);
		break;
	}
	case K_FILEIO_STAT : {
		char *name = kernel_dereference_char_pointer(s, argv[1], 0);
		s->r_acc = make_reg(0, 1 - _k_check_file(name, 0));
		break;
	}
	default :
		error("Unknown FileIO() sub-command: %d", func_nr);
	}

	return s->r_acc;
}

} // End of namespace Sci
