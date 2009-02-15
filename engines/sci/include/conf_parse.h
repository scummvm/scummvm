/***************************************************************************
 conf_parse.h Copyright (C) 2007 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/* RAW parse (config layers zero and one) */

#ifndef CONF_PARSE_H_
#define CONF_PARSE_H_
#include <conf_extension.h>

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
