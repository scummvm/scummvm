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

/* RAW parse (config layers zero and one) */

#ifndef CONF_PARSE_H_
#define CONF_PARSE_H_
#include "sci/include/conf_extension.h"

#define CONF_PARSE_TYPE_WHITESPACE		1	/* Whitespace, comments */
#define CONF_PARSE_TYPE_SUBSECTION		2	/* Subsection division */
#define CONF_PARSE_TYPE_OPTION			3	/* Option or driver assignment */
#define CONF_PARSE_TYPE_INCLUDE			4	/* External include file */
#define CONF_PARSE_TYPE_EXTENSION		5	/* An ``extension'' */
#define CONF_PARSE_TYPE_LEXERROR		255	/* Something lexically broken (stored in ``whitespace'') */

/* Base options */
#define CONF_PARSE_BASEOPTION_VERSION		0
#define CONF_PARSE_BASEOPTION_RESOURCEDIR	1
#define CONF_PARSE_BASEOPTION_DEBUGMODE		2

typedef struct _conf_parse_struct {
	int type;	/* c. CONF_PARSE_TYPE */
	int line_nr;	/* Line number in the current input file */

	struct _conf_parse_struct *prev;
	struct _conf_parse_struct *next;
	struct _conf_parse_struct *overridden;	/* In case we overrode something */

	union {
		char *whitespace;
		struct {
			char *subsystem;	/* may be NULL if driver==NULL */
			char *driver;		/* may be NULL */
			char *option;
			char *centre_whitespace;
			char *value;
			char *terminal_whitespace;
		} assignment; /* driver assignment, option */
		struct {
			char *pre_whitespace;
			char *name;
			char *post_whitespace;
		} subsection;
		struct {
			char *include_prefix;	/* NULL for system includes */
			char *filename;		/* Extracted from include_stmt, also a copy */
			char *include_suffix;	/* NULL for system includes */
			int modifiable;		/* May we write to this file? */
			struct _conf_parse_struct *options_head;
			struct _conf_parse_struct *options_end;
		} include;
		conf_extension_t *extension; /* Extended information, cf. conf_extension.h */
	} data;
} conf_parse_t;



conf_parse_t *
conf_read_file(char *config_file_name, int modifiable, conf_parse_t *previous);
/* Read configuration file
** Parameters: (char *) config_file_name:  File name for the source config file
**             (int) modifiable: Whether to treat the result as modifiable
**             (conf_parse_t *) previous:  Head of the previous configuration, or NULL if none
** Returns   : (conf_parse_t *) Head of the combined config parse, or NULL on failure
** Effects   : (stderr) Error message if the specified file was found but parsing failed
*/

void
conf_free_parse(conf_parse_t *raw_config);
/* Deallocates a conf_parse_t
** Parameters: (conf_parse_t *) raw_config: The raw parse to deallocate
** Returns   : usually.
*/

void
conf_write_parse(conf_parse_t *raw_config);
/* Writes out all writeable parts of the configuration parse to the source files, if writeable
** Parameters: (conf_parse_t *) raw_config
** Effects   : (stderr) Prints error message if ``modifiable'' file could not be written to
**             (filesystem) All ``modifiable'' config files are overwritten
*/

#endif /* !defined (CONF_PARSE_H_) */
