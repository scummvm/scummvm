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

#include "ultima/ultima8/misc/pent_include.h"

#ifdef USE_TIMIDITY_MIDI

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "timidity.h"
#include "timidity_common.h"
#include "timidity_output.h"
#include "timidity_controls.h"

namespace Ultima {
namespace Ultima8 {

static int errno = 0;
static char *strerror(int _errno) {
	return "";
}

#ifdef NS_TIMIDITY
namespace NS_TIMIDITY {
#endif

/* I guess "rb" should be right for any libc */
#define OPEN_MODE "rb"

char current_filename[1024];

#ifdef DEFAULT_TIMIDITY_PATH
/* The paths in this list will be tried whenever we're reading a file */
static PathList defaultpathlist = {DEFAULT_TIMIDITY_PATH, 0};
static PathList *pathlist = &defaultpathlist; /* This is a linked list */
#else
static PathList *pathlist = 0;
#endif

/* Try to open a file for reading. If the filename ends in one of the
   defined compressor extensions, pipe the file through the decompressor */
static FILE *try_to_open(char *name, int decompress, int noise_mode) {
	FILE *fp;

	fp = fopen(name, OPEN_MODE); /* First just check that the file exists */

	if (!fp)
		return 0;

#ifdef DECOMPRESSOR_LIST
	if (decompress) {
		int l, el;
		static char *decompressor_list[] = DECOMPRESSOR_LIST, **dec;
		char tmp[1024], tmp2[1024], *cp, *cp2;
		/* Check if it's a compressed file */
		l = strlen(name);
		for (dec = decompressor_list; *dec; dec += 2) {
			el = strlen(*dec);
			if ((el >= l) || (strcmp(name + l - el, *dec)))
				continue;

			/* Yes. Close the file, open a pipe instead. */
			fclose(fp);

			/* Quote some special characters in the file name */
			cp = name;
			cp2 = tmp2;
			while (*cp) {
				switch (*cp) {
				case '\'':
				case '\\':
				case ' ':
				case '`':
				case '!':
				case '"':
				case '&':
				case ';':
					*cp2++ = '\\';
				}
				*cp2++ = *cp++;
			}
			*cp2 = 0;

			sprintf(tmp, *(dec + 1), tmp2);
			fp = popen(tmp, "r");
			break;
		}
	}
#endif

	return fp;
}

/* This is meant to find and open files for reading, possibly piping
   them through a decompressor. */
FILE *open_file(const char *name, int decompress, int noise_mode) {
	FILE *fp;
	PathList *plp = pathlist;
	int l;

	if (!name || !(*name)) {
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "Attempted to open nameless file.");
		return 0;
	}

	/* First try the given name */

	strncpy(current_filename, name, 1023);
	current_filename[1023] = '\0';

	ctl->cmsg(CMSG_INFO, VERB_DEBUG, "Trying to open %s", current_filename);
	if ((fp = try_to_open(current_filename, decompress, noise_mode)))
		return fp;

#ifdef ENOENT
	if (noise_mode && (errno != ENOENT)) {
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: %s",
		          current_filename, strerror(errno));
		return 0;
	}
#endif

#ifndef __WIN32__
	if (name[0] != PATH_SEP)
#else
	if (name[0] != '\\' && name[0] != '/' && name[1] != ':')
#endif
		while (plp) { /* Try along the path then */
			*current_filename = 0;
			l = static_cast<int>(strlen(plp->path));
			if (l) {
				strcpy(current_filename, plp->path);
#ifndef __WIN32__
				if (current_filename[l - 1] != PATH_SEP)
#else
				if (current_filename[l - 1] != '\\' && current_filename[l - 1] != '/')
#endif
					strcat(current_filename, PATH_STRING);
			}
			strcat(current_filename, name);
			ctl->cmsg(CMSG_INFO, VERB_DEBUG, "Trying to open %s", current_filename);
			if ((fp = try_to_open(current_filename, decompress, noise_mode)))
				return fp;
#ifdef ENOENT
			if (noise_mode && (errno != ENOENT)) {
				ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: %s",
				          current_filename, strerror(errno));
				return 0;
			}
#endif
			plp = plp->next;
		}

	/* Nothing could be opened. */

	*current_filename = 0;

	if (noise_mode >= 2)
		ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: %s", name, strerror(errno));

	return 0;
}

/* This closes files opened with open_file */
void close_file(FILE *fp) {
#ifdef DECOMPRESSOR_LIST
	if (pclose(fp)) /* Any better ideas? */
#endif
		fclose(fp);
}

/* This is meant for skipping a few bytes in a file or fifo. */
void skip(FILE *fp, size_t len) {
	size_t c;
	char tmp[1024];
	while (len > 0) {
		c = len;
		if (c > 1024) c = 1024;
		len -= c;
		if (c != fread(tmp, 1, c, fp))
			ctl->cmsg(CMSG_ERROR, VERB_NORMAL, "%s: skip: %s",
			          current_filename, strerror(errno));
	}
}

/* This'll allocate memory or die. */
void *safe_malloc(size_t count) {
	void *p;
	if (count > (1 << 21)) {
		ctl->cmsg(CMSG_FATAL, VERB_NORMAL,
		          "Strange, I feel like allocating %d bytes. This must be a bug.",
		          count);
	} else if ((p = malloc(count)))
		return p;
	else
		ctl->cmsg(CMSG_FATAL, VERB_NORMAL, "Sorry. Couldn't malloc %d bytes.", count);

	ctl->close();
	exit(10);
	return (NULL);
}

/* This adds a directory to the path list */
void add_to_pathlist(const char *s) {
	PathList *plp = safe_Malloc<PathList>();
	char *tmp = safe_Malloc<char>(strlen(s) + 1);
	strcpy(tmp, s);
	plp->path = tmp;
	plp->next = pathlist;
	pathlist = plp;
}

#ifdef NS_TIMIDITY
};
#endif

} // End of namespace Ultima8
} // End of namespace Ultima

#endif //USE_TIMIDITY_MIDI
