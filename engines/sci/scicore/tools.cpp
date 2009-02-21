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

#ifdef _MSC_VER
#  include <sys/timeb.h>
#  include <windows.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  undef ARRAYSIZE
#endif

#include "common/archive.h"
#include "common/file.h"
#include "common/util.h"

#ifdef WIN32
#  include <windows.h>
#  include <errno.h>
#  include <mmsystem.h>

#	ifdef sleep
#		undef sleep
#	endif

#	define sleep(x) \
	do { \
		if (x == 0) { \
			Sleep(0); \
		} else { \
			if (timeBeginPeriod(1) != TIMERR_NOERROR) \
				error("timeBeginPeriod(1) failed\n"); \
			Sleep(x); \
			if (timeEndPeriod(1) != TIMERR_NOERROR) \
				error("timeEndPeriod(1) failed\n"); \
		} \
	} while (0);
#endif

#include "common/scummsys.h"
#include "common/str.h"

#ifdef UNIX
#include <fnmatch.h>
#endif

#include "sci/include/engine.h"

namespace Sci {

// FIXME: Get rid of G_DIR_SEPARATOR  / G_DIR_SEPARATOR_S
#if _MSC_VER
#  define G_DIR_SEPARATOR_S "\\"
#  define G_DIR_SEPARATOR '\\'
#else
#  define G_DIR_SEPARATOR_S "/"
#  define G_DIR_SEPARATOR '/'
#endif


#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#ifdef __DC__
#  include <kos/thread.h>
#endif

int script_debug_flag = 0; /* Defaulting to running mode */
int sci_debug_flags = 0; /* Special flags */

#ifndef con_file
#	define con_file 0
#endif

int sci_ffs(int _mask) {
	int retval = 0;

	if (!_mask)
		return 0;
	retval++;
	while (!(_mask & 1)) {
		retval++;
		_mask >>= 1;
	}

	return retval;
}


/******************** Debug functions ********************/



/* Functions for internal macro use */
void _SCIkvprintf(FILE *file, const char *format, va_list args);
void _SCIkprintf(FILE *file, const char *format, ...)  GCC_PRINTF(2, 3);

void _SCIkvprintf(FILE *file, const char *format, va_list args) {
	vfprintf(file, format, args);
	if (con_file) vfprintf(con_file, format, args);
}

void _SCIkdebug(EngineState *s, const char *file, int line, int area, const char *format, ...) {
	va_list args;

	if (s->debug_mode & (1 << area)) {
		fprintf(stdout, " kernel: (%s L%d): ", file, line);
		va_start(args, format);
		_SCIkvprintf(stdout, format, args);
		va_end(args);
		fflush(NULL);
	}
}

void _SCIGNUkdebug(const char *funcname, EngineState *s, const char *file, int line, int area, const char *format, ...) {
	va_list xargs;
	int error = ((area == SCIkWARNING_NR) || (area == SCIkERROR_NR));

	if (error || (s->debug_mode & (1 << area))) { /* Is debugging enabled for this area? */

		fprintf(stderr, "FSCI: ");

		if (area == SCIkERROR_NR)
			fprintf(stderr, "ERROR in %s ", funcname);
		else if (area == SCIkWARNING_NR)
			fprintf(stderr, "%s: Warning ", funcname);
		else
			fprintf(stderr, funcname);

		fprintf(stderr, "(%s L%d): ", file, line);

		va_start(xargs, format);
		_SCIkvprintf(stderr, format, xargs);
		va_end(xargs);

	}
}


#if defined(HAVE_GETTIMEOFDAY)
void
sci_gettime(long *seconds, long *useconds) {
	struct timeval tv;

	assert(!gettimeofday(&tv, NULL));
	*seconds = tv.tv_sec;
	*useconds = tv.tv_usec;
}
#elif defined (WIN32)

/*WARNING(Incorrect)*/
/* Warning: This function only retrieves the amount of mseconds since the start of
** the Win32 kernel; it does /not/ provide the number of seconds since the epoch!
** There are no known cases where this causes problems, though.  */
void sci_gettime(long *seconds, long *useconds) {
	DWORD tm;

	if (TIMERR_NOERROR != timeBeginPeriod(1)) {
		error("timeBeginPeriod(1) failed in sci_gettime\n");
	}

	tm = timeGetTime();

	if (TIMERR_NOERROR != timeEndPeriod(1)) {
		error("timeEndPeriod(1) failed in sci_gettime\n");
	}

	*seconds = tm / 1000;
	*useconds = (tm % 1000) * 1000;
}
#else
#  error "You need to provide a microsecond resolution sci_gettime implementation for your platform!"
#endif


void
sci_get_current_time(GTimeVal *val) {
	long foo, bar;
	sci_gettime(&foo, &bar);
	val->tv_sec = foo;
	val->tv_usec = bar;
}


/************* Directory entities *************/
#if defined(WIN32)
/******** Dir: Win32 CODE ********/

void
sci_init_dir(sci_dir_t *dir) {
	dir->search = -1;
}

char *
sci_find_first(sci_dir_t *dir, const char *mask) {
	dir->search = _findfirst(mask, &(dir->fileinfo));

	if (dir->search != -1) {
		if (dir->fileinfo.name == NULL) {
			return NULL;
		}

		if (strcmp(dir->fileinfo.name, ".") == 0 ||
		        strcmp(dir->fileinfo.name, "..") == 0) {
			if (sci_find_next(dir) == NULL) {
				return NULL;
			}
		}

		return dir->fileinfo.name;
	} else {
		switch (errno) {
		case ENOENT: {
#ifdef _DEBUG
			printf("_findfirst errno = ENOENT: no match\n");

			if (mask)
				printf(" in: %s\n", mask);
			else
				printf(" - searching in undefined directory\n");
#endif
			break;
		}
		case EINVAL: {
			printf("_findfirst errno = EINVAL: invalid filename\n");
			break;
		}
		default:
			printf("_findfirst errno = unknown (%d)", errno);
		}
	}

	return NULL;
}

char *
sci_find_next(sci_dir_t *dir) {
	if (dir->search == -1)
		return NULL;

	if (_findnext(dir->search, &(dir->fileinfo)) < 0) {
		_findclose(dir->search);
		dir->search = -1;
		return NULL;
	}

	if (strcmp(dir->fileinfo.name, ".") == 0 ||
	        strcmp(dir->fileinfo.name, "..") == 0) {
		if (sci_find_next(dir) == NULL) {
			return NULL;
		}
	}

	return dir->fileinfo.name;
}

void
sci_finish_find(sci_dir_t *dir) {
	if (dir->search != -1) {
		_findclose(dir->search);
		dir->search = -1;
	}
}

#else /* !WIN32 */
/******** Dir: UNIX CODE ********/

void
sci_init_dir(sci_dir_t *dir) {
	dir->dir = NULL;
	dir->mask_copy = NULL;
}

char *
sci_find_first(sci_dir_t *dir, const char *mask) {
	if (dir->dir)
		closedir(dir->dir);

	if (!(dir->dir = opendir("."))) {
		sciprintf("%s, L%d: opendir(\".\") failed", __FILE__, __LINE__);
		return NULL;
	}

	dir->mask_copy = sci_strdup(mask);

	return sci_find_next(dir);
}

#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD 0
#warning "File searches will not be case-insensitive!"
#endif

char *
sci_find_next(sci_dir_t *dir) {
	struct dirent *match;

	while ((match = readdir(dir->dir))) {
		if (match->d_name[0] == '.')
			continue;

		if (!fnmatch(dir->mask_copy, match->d_name, FNM_CASEFOLD))
			return match->d_name;
	}

	sci_finish_find(dir);
	return NULL;
}

void
sci_finish_find(sci_dir_t *dir) {
	if (dir->dir) {
		closedir(dir->dir);
		dir->dir = NULL;
		free(dir->mask_copy);
		dir->mask_copy = NULL;
	}
}

#endif /* !WIN32 */

/************* /Directory entities *************/


int
sci_mkpath(const char *path) {
	const char *path_position = path;
	char *next_separator = NULL;

	if (chdir(G_DIR_SEPARATOR_S)) { /* Go to root */
		sciprintf("Error: Could not change to root directory '%s'",
		          G_DIR_SEPARATOR_S);
		return -1;
	}

	do {
		if (next_separator)
			*next_separator = G_DIR_SEPARATOR_S[0];
		next_separator = (char *)strchr(path_position, G_DIR_SEPARATOR_S[0]);

		if (next_separator)
			*next_separator = 0;

		if (*path_position) { /* Unless we're at the first slash... */
			if (chdir(path_position)) {
				if (scimkdir(path_position, 0700) || chdir(path_position)) {
					sciprintf("Error: Could not create subdirectory '%s' in",
					          path_position);
					if (next_separator)
						*next_separator = G_DIR_SEPARATOR_S[0];
					sciprintf(" '%s'", path);
					return -2;
				}
			}
		}
		path_position = next_separator + 1;

	} while (next_separator);

	return 0;
}

/*-- Yielding to the scheduler --*/

#ifdef HAVE_SCHED_YIELD
#  include <sched.h>

void
sci_sched_yield(void) {
	sched_yield();
}

#elif defined (__DC__)

void
sci_sched_yield() {
	thd_pass();
}

#elif defined (WIN32)

void
sci_sched_yield() {
	sleep(1);
}

#else

void
sci_sched_yield() {
}

#endif /* !HAVE_SCHED_YIELD */


/* Returns the case-sensitive filename of a file.
** Expects *dir to be uninitialized and the caller to free it afterwards.
** Parameters: (const char *) fname: Name of file to get case-sensitive.
**             (sci_dir_t *) dir: Directory to find file within.
** Returns   : (char *) Case-sensitive filename of the file.
*/
Common::String _fcaseseek(const char *fname) {
/* Expects *dir to be uninitialized and the caller to
 ** free it afterwards  */

	if (strchr(fname, G_DIR_SEPARATOR)) {
		error("_fcaseseek() does not support subdirs\n");
		BREAKPOINT();
	}

	// Look up the file, ignoring case
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, fname);

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		if (name.equalsIgnoreCase(fname))
			return name;
	}
	return Common::String();
}


FILE *sci_fopen(const char *fname, const char *mode) {
	Common::String name = _fcaseseek(fname);
	FILE *file = NULL;

	if (!name.empty())
		file = fopen(name.c_str(), mode);
	else if (strchr(mode, 'w'))
		file = fopen(fname, mode);

	return file;
}

int sci_open(const char *fname, int flags) {
	int file = SCI_INVALID_FD;
	Common::String name = _fcaseseek(fname);
	if (!name.empty())
		file = open(name.c_str(), flags);

	return file;
}

char *
sci_getcwd(void) {
	int size = 0;
	char *cwd = NULL;

	while (size < 8192) {
		size += 256;
		cwd = (char*)sci_malloc(size);
		if (getcwd(cwd, size - 1))
			return cwd;

		free(cwd);
	}

	error("Could not determine current working directory");
	return NULL;
}

#ifdef __DC__

int
sci_fd_size(int fd) {
	return fs_total(fd);
}

int
sci_file_size(const char *fname) {
	int fd = fs_open(fname, O_RDONLY);
	int retval = -1;

	if (fd != 0) {
		retval = sci_fd_size(fd);
		fs_close(fd);
	}

	return retval;
}

#else

int
sci_fd_size(int fd) {
	struct stat fd_stat;
	if (fstat(fd, &fd_stat)) return -1;
	return fd_stat.st_size;
}

int
sci_file_size(const char *fname) {
	struct stat fn_stat;
	if (stat(fname, &fn_stat)) return -1;
	return fn_stat.st_size;
}

#endif

/* Simple heuristic to work around array handling peculiarity in SQ4:
It uses StrAt() to read the individual elements, so we must determine
whether a string is really a string or an array. */
int is_print_str(char *str) {
	int printable = 0;
	int len = strlen(str);

	if (len == 0) return 1;

	while (*str) {
		if (isprint(*str)) printable++;
		str++;
	}

	return ((float) printable / (float) len >= 0.5);
}

} // End of namespace Sci
