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

#include "sci/include/engine.h"

#include <errno.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

static int _savegame_indices_nr = -1; // means 'uninitialized'

static struct _savegame_index_struct {
	int id;
	long timestamp;
} _savegame_indices[MAX_SAVEGAME_NR];

// This assumes modern stream implementations. It may break on DOS.


/* Attempts to mirror a file by copying it from the resource firectory
** to the working directory. Returns NULL if the file didn't exist.
** Otherwise, the new file is then opened for reading or writing.
*/
static FILE *f_open_mirrored(state_t *s, char *fname) {
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
	fd = creat(fname, 0600);
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
			sciprintf("kfile.c: f_open_mirrored(): Warning: Could not write all %ld bytes to '%s' in '%s' (only wrote %ld)\n",
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

void file_open(state_t *s, char *filename, int mode) {
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
				SCIkdebug(SCIkFILE, "Success!\n");
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

reg_t kFOpen(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *name = kernel_dereference_char_pointer(s, argv[0], 0);
	int mode = UKPV(1);

	file_open(s, name, mode);
	return s->r_acc;
}

static FILE *getFileFromHandle(state_t *s, int handle) {
	if (handle == 0) {
		SCIkwarn(SCIkERROR, "Attempt to use file handle 0\n");
		return 0;
	}

	if ((handle >= s->file_handles_nr) || (s->file_handles[handle] == NULL)) {
		SCIkwarn(SCIkERROR, "Attempt to use invalid/unused file handle %d\n", handle);
		return 0;
	}
	
	return s->file_handles[handle];
}

void file_close(state_t *s, int handle) {
	SCIkdebug(SCIkFILE, "Closing file %d\n", handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	fclose(f);

	s->file_handles[handle] = NULL;
}

reg_t kFClose(state_t *s, int funct_nr, int argc, reg_t *argv) {
	file_close(s, UKPV(0));
	return s->r_acc;
}

void fputs_wrapper(state_t *s, int handle, int size, char *data) {
	SCIkdebug(SCIkFILE, "FPuts'ing \"%s\" to handle %d\n", data, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (f)
		fwrite(data, 1, size, f);
}

void fwrite_wrapper(state_t *s, int handle, char *data, int length) {
	SCIkdebug(SCIkFILE, "fwrite()'ing \"%s\" to handle %d\n", data, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (f)
		fwrite(data, 1, length, f);
}

reg_t kFPuts(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int handle = UKPV(0);
	char *data = kernel_dereference_char_pointer(s, argv[1], 0);

	fputs_wrapper(s, handle, strlen(data), data);
	return s->r_acc;
}

static void fgets_wrapper(state_t *s, char *dest, int maxsize, int handle) {
	SCIkdebug(SCIkFILE, "FGets'ing %d bytes from handle %d\n", maxsize, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	fgets(dest, maxsize, f);

	SCIkdebug(SCIkFILE, "FGets'ed \"%s\"\n", dest);
}

static void fread_wrapper(state_t *s, char *dest, int bytes, int handle) {
	SCIkdebug(SCIkFILE, "fread()'ing %d bytes from handle %d\n", bytes, handle);

	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	s->r_acc = make_reg(0, fread(dest, 1, bytes, f));
}

static void fseek_wrapper(state_t *s, int handle, int offset, int whence) {
	FILE *f = getFileFromHandle(s, handle);
	if (!f)
		return;

	s->r_acc = make_reg(0, fseek(f, offset, whence));
}

static char *_chdir_savedir(state_t *s) {
	char *cwd = sci_getcwd();
	char *save_dir = kernel_dereference_char_pointer(s, make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR), 0);

	if (chdir(save_dir) && sci_mkpath(save_dir)) {
		sciprintf(__FILE__": Can't chdir to savegame dir '%s' or create it\n", save_dir);
		free(cwd);
		return NULL;
	}

	if (!cwd)
		cwd = strdup(s->work_dir);

	return cwd; // Potentially try again
}

static void _chdir_restoredir(char *dir) {
	if (chdir(dir)) {
		sciprintf(__FILE__": Can't seem to return to previous homedir '%s'\n", dir);
	}
	free(dir);
}

#define TEST_DIR_OR_QUIT(dir) if (!dir) { return NULL_REG; }

reg_t kFGets(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *dest = kernel_dereference_char_pointer(s, argv[0], 0);
	int maxsize = UKPV(1);
	int handle = UKPV(2);

	fgets_wrapper(s, dest, maxsize, handle);
	return argv[0];
}

/* kGetCWD(address):
** Writes the cwd to the supplied address and returns the address in acc.
*/
reg_t kGetCWD(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *wd = sci_getcwd();
	char *targetaddr = kernel_dereference_char_pointer(s, argv[0], 0);

	strncpy(targetaddr, wd, MAX_SAVE_DIR_SIZE - 1);
	targetaddr[MAX_SAVE_DIR_SIZE - 1] = 0; // Terminate

	SCIkdebug(SCIkFILE, "Copying cwd='%s'(%d chars) to %p", wd, strlen(wd), targetaddr);

	free(wd);
	return argv[0];
}

// Returns a dynamically allocated pointer to the name of the requested save dir
char *_k_get_savedir_name(int nr) {
	char suffices[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char *savedir_name = (char*)sci_malloc(strlen(FREESCI_SAVEDIR_PREFIX) + 2);
	assert(nr >= 0);
	assert(nr < MAX_SAVEGAME_NR);
	strcpy(savedir_name, FREESCI_SAVEDIR_PREFIX);
	savedir_name[strlen(FREESCI_SAVEDIR_PREFIX)] = suffices[nr];
	savedir_name[strlen(FREESCI_SAVEDIR_PREFIX) + 1] = 0;

	return savedir_name;
}

void delete_savegame(state_t *s, int savedir_nr) {
	char *workdir = _chdir_savedir(s);
	char *savedir = _k_get_savedir_name(savedir_nr);
	char buffer[256];

	sciprintf("Deleting savegame '%s'\n", savedir);

	sprintf(buffer, "%s/%s.id", savedir, s->game_name);
	sci_unlink(buffer);

	sprintf(buffer, "%s/state", savedir);
	sci_unlink(buffer);

	sci_rmdir(savedir);

	free(savedir);
	_chdir_restoredir(workdir);
}

#define K_DEVICE_INFO_GET_DEVICE 0
#define K_DEVICE_INFO_GET_CURRENT_DEVICE 1
#define K_DEVICE_INFO_PATHS_EQUAL 2
#define K_DEVICE_INFO_IS_FLOPPY 3
#define K_DEVICE_INFO_GET_SAVECAT_NAME 7
#define K_DEVICE_INFO_GET_SAVEFILE_NAME 8

#ifdef WIN32

reg_t kDeviceInfo_Win32(state_t *s, int funct_nr, int argc, reg_t *argv) {
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
		SCIkwarn(SCIkERROR, "Unknown DeviceInfo() sub-command: %d\n", mode);
	}
	}
	return s->r_acc;
}

#else // !WIN32

reg_t kDeviceInfo_Unix(state_t *s, int funct_nr, int argc, reg_t *argv) {
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
		SCIkwarn(SCIkERROR, "Unknown DeviceInfo() sub-command: %d\n", mode);
	}
	}

	return s->r_acc;
}

#endif // !WIN32

reg_t kGetSaveDir(state_t *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(s->sys_strings_segment, SYS_STRING_SAVEDIR);
}

reg_t kCheckFreeSpace(state_t *s, int funct_nr, int argc, reg_t *argv) {
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
			SCIkwarn(SCIkWARNING, "Failed to find non-existing file for free space test\n");
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

	fd = creat(testpath, 0600);

	if (!IS_VALID_FD(fd)) {
		SCIkwarn(SCIkWARNING, "Could not test for disk space: %s\n", strerror(errno));
		SCIkwarn(SCIkWARNING, "Test path was '%s'\n", testpath);
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

int _k_find_savegame_by_name(char *game_id_file, char *name) {
	int savedir_nr = -1;
	int i;
	char *buf = NULL;

	for (i = 0; i < MAX_SAVEGAME_NR; i++) {
		if (!chdir((buf = _k_get_savedir_name(i)))) {
			char namebuf[32]; // Save game name buffer
			FILE *idfile = sci_fopen(game_id_file, "r");

			if (idfile) {
				fgets(namebuf, 31, idfile);
				if (strlen(namebuf) > 0)
					if (namebuf[strlen(namebuf) - 1] == '\n')
						namebuf[strlen(namebuf) - 1] = 0; // Remove trailing newlines

				if (strcmp(name, namebuf) == 0) {
					sciprintf("Save game name matched entry %d\n", i);
					savedir_nr = i;
				}
				fclose(idfile);
			}
			chdir("..");
		}
		free(buf);
	}
	return 0;
}

#ifdef __DC__
static long get_file_mtime(int fd) {
	/* FIXME (Dreamcast): Not yet implemented */
	return 0;
}

#else

#define get_file_mtime(fd) get_file_mtime_Unix(fd)
/* Returns the time of the specified file's last modification
** Parameters: (int) fd: The file descriptor of the file in question
** Returns   : (long) An integer value describing the time of the
**                    file's last modification.
** The only thing that must be ensured is that
** get_file_mtime(f1) > get_file_mtime(f2)
**   <=>
** if f1 was modified at a later point in time than the last time
** f2 was modified.
*/

static long get_file_mtime_Unix(int fd) {
	struct stat fd_stat;
	fstat(fd, &fd_stat);

	return fd_stat.st_ctime;
}
#endif

static int _savegame_index_struct_compare(const void *a, const void *b) {
	return ((struct _savegame_index_struct *)b)->timestamp - ((struct _savegame_index_struct *)a)->timestamp;
}

static void update_savegame_indices(const char *gfname) {
	int i;

	_savegame_indices_nr = 0;

	for (i = 0; i < MAX_SAVEGAME_NR; i++) {
		char *dirname = _k_get_savedir_name(i);
		int fd;

		if (!chdir(dirname)) {

			if (IS_VALID_FD(fd = sci_open(gfname, O_RDONLY))) {
				_savegame_indices[_savegame_indices_nr].id = i;
				_savegame_indices[_savegame_indices_nr++].timestamp = get_file_mtime(fd);
				close(fd);
			}
			chdir("..");
		}

		free(dirname);
	}

	qsort(_savegame_indices, _savegame_indices_nr, sizeof(struct _savegame_index_struct), _savegame_index_struct_compare);
}

int test_savegame(state_t *s, char *savegame_id, char *savegame_name, int savegame_name_length) {
	FILE *f;
	char buffer[80];
	int version = -1;

	chdir(savegame_id);
	f = fopen("state", "r");

	if (!f) return 0;
	while (!feof(f)) {
		char *seeker;
		fgets(buffer, sizeof(buffer), f);
		if ((seeker = strstr(buffer, "savegame_version = ")) != NULL) {
			seeker += strlen("savegame_version = ");
			version = strtol(seeker, NULL, 10);
			break;
		}
	}

	fclose(f);
	return version >= FREESCI_MINIMUM_SAVEGAME_VERSION && version <= FREESCI_CURRENT_SAVEGAME_VERSION;
}

reg_t kCheckSaveGame(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);
	char *buf = NULL;
	char *workdir = _chdir_savedir(s);
	TEST_DIR_OR_QUIT(workdir);

	if (_savegame_indices_nr < 0) {
		char *game_id_file_name = (char*)sci_malloc(strlen(game_id) + strlen(FREESCI_ID_SUFFIX) + 1);

		strcpy(game_id_file_name, game_id);
		strcat(game_id_file_name, FREESCI_ID_SUFFIX);
		SCIkwarn(SCIkWARNING, "Savegame index list not initialized!\n");
		update_savegame_indices(game_id_file_name);
	}

	savedir_nr = _savegame_indices[savedir_nr].id;

	if (savedir_nr > MAX_SAVEGAME_NR - 1) {
		_chdir_restoredir(workdir);
		return NULL_REG;
	}

	s->r_acc = make_reg(0, test_savegame(s, (buf = _k_get_savedir_name(savedir_nr)), NULL, 0));

	_chdir_restoredir(workdir);
	free(buf);

	return s->r_acc;
}

reg_t kGetSaveFiles(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = kernel_dereference_char_pointer(s, argv[0], 0);
	char *nametarget = kernel_dereference_char_pointer(s, argv[1], 0);
	reg_t nametarget_base = argv[1];
	reg_t *nameoffsets = kernel_dereference_reg_pointer(s, argv[2], 0);
	int gfname_len = strlen(game_id) + strlen(FREESCI_ID_SUFFIX) + 1;
	char *gfname = (char*)sci_malloc(gfname_len);
	int i;
	char *workdir = _chdir_savedir(s);
	TEST_DIR_OR_QUIT(workdir);

	strcpy(gfname, game_id);
	strcat(gfname, FREESCI_ID_SUFFIX); // This file is used to identify in-game savegames

	update_savegame_indices(gfname);

	s->r_acc = NULL_REG;

	for (i = 0; i < _savegame_indices_nr; i++) {
		char *savedir_name = _k_get_savedir_name(_savegame_indices[i].id);
		FILE *idfile;

		if (!chdir(savedir_name)) {
			if ((idfile = sci_fopen(gfname, "r"))) { // Valid game ID file: Assume valid game
				char namebuf[SCI_MAX_SAVENAME_LENGTH]; // Save game name buffer
				fgets(namebuf, SCI_MAX_SAVENAME_LENGTH - 1, idfile);
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
					fclose(idfile);
				}
			}
			chdir("..");
		}
		free(savedir_name);
	}

	free(gfname);
	*nametarget = 0; // Terminate list

	_chdir_restoredir(workdir);
	return s->r_acc;
}

reg_t kSaveGame(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = (char*)kernel_dereference_bulk_pointer(s, argv[0], 0);
	char *savegame_dir;
	int savedir_nr = UKPV(1);
	int savedir_id; // Savegame ID, derived from savedir_nr and the savegame ID list
	char *game_id_file_name = (char*)sci_malloc(strlen(game_id) + strlen(FREESCI_ID_SUFFIX) + 1);
	char *game_description = (char*)kernel_dereference_bulk_pointer(s, argv[2], 0);
	char *workdir = _chdir_savedir(s);
	char *version = argc > 3 ? strdup((char*)kernel_dereference_bulk_pointer(s, argv[3], 0)) : NULL;
	TEST_DIR_OR_QUIT(workdir);

	s->game_version = version;

	strcpy(game_id_file_name, game_id);
	strcat(game_id_file_name, FREESCI_ID_SUFFIX);

	update_savegame_indices(game_id_file_name);

	if (savedir_nr >= 0 && savedir_nr < _savegame_indices_nr)
		// Overwrite
		savedir_id = _savegame_indices[savedir_nr].id;
	else if (savedir_nr >= 0 && savedir_nr < MAX_SAVEGAME_NR) {
		int i = 0;

		savedir_id = 0;

		// First, look for holes
		while (i < _savegame_indices_nr) {
			if (_savegame_indices[i].id == savedir_id) {
				++savedir_id;
				i = 0;
			} else ++i;
		}
		if (savedir_id >= MAX_SAVEGAME_NR) {
			sciprintf("Internal error: Free savegame ID is %d, shouldn't happen!\n", savedir_id);
			return NULL_REG;
		}

		// This loop terminates when savedir_id is not in [x | ex. n. _savegame_indices[n].id = x]
	} else {
		sciprintf("Savegame ID %d is not allowed!\n", savedir_nr);
		return NULL_REG;
	}

	savegame_dir = _k_get_savedir_name(savedir_id);

	if (gamestate_save(s, savegame_dir)) {
		sciprintf("Saving the game failed.\n");
		s->r_acc = NULL_REG;
	} else {
		FILE *idfile;

		chdir(savegame_dir);

		if ((idfile = sci_fopen(game_id_file_name, "w"))) {
			fprintf(idfile, "%s", game_description);
			fclose(idfile);
		} else {
			sciprintf("Creating the game ID file failed.\n");
			sciprintf("You can still restore from inside the debugger with \"restore_game %s\"\n", savegame_dir);
			s->r_acc = NULL_REG;
		}

		chdir("..");
		s->r_acc = make_reg(0, 1);
	}
	free(game_id_file_name);
	_chdir_restoredir(workdir);

	free(s->game_version);
	s->game_version = NULL;

	return s->r_acc;
}

reg_t kRestoreGame(state_t *s, int funct_nr, int argc, reg_t *argv) {
	char *game_id = (char*)kernel_dereference_bulk_pointer(s, argv[0], 0);
	int savedir_nr = UKPV(1);
	char *workdir = _chdir_savedir(s);
	TEST_DIR_OR_QUIT(workdir);

	if (_savegame_indices_nr < 0) {
		char *game_id_file_name = (char*)sci_malloc(strlen(game_id) + strlen(FREESCI_ID_SUFFIX) + 1);

		strcpy(game_id_file_name, game_id);
		strcat(game_id_file_name, FREESCI_ID_SUFFIX);
		SCIkwarn(SCIkWARNING, "Savegame index list not initialized!\n");
		update_savegame_indices(game_id_file_name);
	}

	savedir_nr = _savegame_indices[savedir_nr].id;

	if (savedir_nr > -1) {
		char *savedir_name = _k_get_savedir_name(savedir_nr);
		state_t *newstate = gamestate_restore(s, savedir_name);

		free(savedir_name);
		if (newstate) {
			s->successor = newstate;
			script_abort_flag = SCRIPT_ABORT_WITH_REPLAY; // Abort current game
			s->execution_stack_pos = s->execution_stack_base;
		} else {
			s->r_acc = make_reg(0, 1);
			sciprintf("Restoring failed (game_id = '%s').\n", game_id);
		}
	} else {
		s->r_acc = make_reg(0, 1);
		sciprintf("Savegame #%d not found!\n", savedir_nr);
	}

	_chdir_restoredir(workdir);

	return s->r_acc;
}

reg_t kValidPath(state_t *s, int funct_nr, int argc, reg_t *argv) {
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

char * write_filename_to_mem(state_t *s, reg_t address, char *string) {
	char *mem = kernel_dereference_char_pointer(s, address, 0);

	if (string) {
		memset(mem, 0, 13);
		strncpy(mem, string, 12);
	}

	return string;
}

void next_file(state_t *s) {
	if (write_filename_to_mem(s, s->dirseeker_outbuffer, sci_find_next(&(s->dirseeker))))
		s->r_acc = s->dirseeker_outbuffer;
	else
		s->r_acc = NULL_REG;
}

void first_file(state_t *s, const char *dir, char *mask, reg_t buffer) {
	if (!buffer.segment) {
		sciprintf("Warning: first_file(state,\"%s\",\"%s\", 0) invoked!\n", dir, mask);
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

reg_t kFileIO(state_t *s, int funct_nr, int argc, reg_t *argv) {
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
		SCIkwarn(SCIkERROR, "Unknown FileIO() sub-command: %d\n", func_nr);
	}

	return s->r_acc;
}
