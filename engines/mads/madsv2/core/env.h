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

#ifndef MADS_CORE_ENV_H
#define MADS_CORE_ENV_H

#include "common/stream.h"
#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define MADS_ENV        "MADS"
#define MADS_PRIV_ENV   "MADSPRIV"
#define MADS_SOUND_ENV  "MADSOUND"

#define MADS_PRIV_ARTIST        0x01
#define MADS_PRIV_DESIGNER      0x02
#define MADS_PRIV_PROGRAM       0x04
#define MADS_PRIV_SYSTEM        0x08

#define ENV_CONFIGNAME  "*options.cfg"

#define ENV_SEARCH_MADS_PATH            0
#define ENV_SEARCH_CONCAT_FILES         1

#define ENV_OBJECTS                     16
#define ENV_INTERFACE                   17
#define ENV_TEXT                        18
#define ENV_QUOTES                      19
#define ENV_FONT                        20
#define ENV_ART                         21
#define ENV_SOUND                       22
#define ENV_CONV                        23
#define ENV_SPEECH                      24


struct env_options_type
{
	int     linemode;          /* 50/25/0=auto */
	int     compile_final;     /* TRUE if compilation is to final set */
	int     compile_debug;     /* TRUE for debugging mode */
	int     current_room;      /* User's current room */
	long    game_revision;     /* Version of game */
	char    game_name[25];     /* Game's working title */
	int     mono_on_right;     /* Is monochrome monitor on right? */
};


extern int env_search_mode;
extern int env_search_cd;
extern char env_cd_drive;
extern int env_privileges;
extern int env_sound_override;
extern long env_concat_file_size; /* Size of last concat file opened */
extern char env_null[7];


extern int env_verify(void);
extern Common::SeekableReadStream *env_open(const char *file_path, const char *options = "rb");
extern int env_exist(const char *file_name);
extern long env_get_file_size(Common::SeekableReadStream *handle);
extern char *env_get_path(char *madspath, const char *infile);
extern char *env_catint(char *out, int value, int digits);
extern char *env_fill_path(char *path, int env_mode, int env_room);
extern char *env_get_level_path(char *out, int item_type, const char *file_spec,
	int first_level, int second_level);
extern Common::SeekableReadStream *env_open_level(int item_type, const char *file_spec,
	int level_one, int level_two, const char *options);
extern char *env_get(char *target, const char *env);

/**
 * Given a pointer to an environment variable, returns a pointer
 * to the next variable in the environment list (if pointer is
 * to a null string, the end of the list has been reached).
 */
char *env_next(char *variable);

/**
 * Given a pointer to an environment, returns a pointer to one
 * byte after the end of the entire environment.
 */
char *env_find_end(char *environment);

/**
 * Given a pointer to the start of an environment, returns the
 * size in bytes of that environment.
 */
int env_size(char *environment);

int env_free(char *environment);

/**
 * Given a pointer to an environment and a variable, returns a
 * pointer to that variable name in the environment.  Returns
 * NULL if no match found.
 */
char *env_find(char *environment, const char *variable);

/**
 * Given an environment and a variable name, deletes that
 * variable from the environment table.
 */
void env_delete(char *environment, const char *variable);

/**
 * Given a pointer to an environment, a variable name, and a value,
 * inserts that variable into the environment table with the
 * designated value.  Returns 0 for success, -1 for failure.
 */
int env_insert(char *environment, const char *variable, const char *value);

} // namespace MADSV2
} // namespace MADS

#endif
