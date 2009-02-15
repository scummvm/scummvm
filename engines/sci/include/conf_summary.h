/***************************************************************************
 conf_summary.h Copyright (C) 2007 Christoph Reichenbach


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

#ifndef CONF_SUMMARY_H_
#define CONF_SUMMARY_H_

#include<conf_parse.h>
#include<conf_driver.h>

typedef struct {
	conf_option_t *option;	/* NULL if this is a terminator */
	conf_parse_t *parse;	/* Source location we got this information from,
				** or NULL for command-line or default */
	union { char *str;
		int nr } choice;
	int origin;
	int flags;
} conf_option_choice_t;

#define CONF_CHOICE_ORIGIN_DEFAULT	0	/* Built-in default */
#define CONF_CHOICE_ORIGIN_CONFDEFAULT	1	/* Config file option */
#define CONF_CHOICE_ORIGIN_CONFGAME	2	/* Game-specific option */
#define CONF_CHOICE_ORIGIN_COMMANDLINE	3

#define CONF_CHOICE_FLAG_WRITEABLE	(1<<0)	/* 'parse' can be written to */
#define CONF_CHOICE_FLAG_DISABLED	(1<<1)	/* Option isn't set, only listed for completeness */

typedef struct {
	conf_option_choice_t *options;	/* Driver-specific */
	conf_driver_t *driver;
} conf_driver_choice_t;

typedef struct {
	conf_option_choice_t *options;	/* Subsystem-specific */
	int driver_origin;		/* CONF_CHOICE_ORIGIN_* */
	conf_driver_choice_t *driver;	/* The particular driver selected */
	conf_driver_choice_t *drivers;	/* All available drivers, with their options */
} conf_subsystem_choice_t;

typedef struct {
	char *game_id;			/* NULL for default */
	conf_option_choice_t *options;	/* Global */
	int flags;
	conf_parse_t **append_location;	/* Into here we can add new configuration options to override */

	conf_subsystem_choice_t[CONF_SUBSYSTEMS_NR] subsystems;
} conf_game_choice_t;

#define CONF_GAME_CHOICE_FLAG_OVERRIDE_IN_SECTION	(1<<0)	/* Override section already exists, need not be re-created */

typedef struct {
	conf_game_choice_t *default;

	int game_choices_nr;
	conf_game_choice_t *game_choices;
} conf_summary_t;


conf_summary_t *
conf_summary(conf_parse_t *raw_config);
/* Summarises a config parse in a conf_summary_t
** Parameters: (conf_parse_t *) raw_config: The parse to summarise
** Returns   : (conf_summary_t *) A summarised configuration
** The summary includes (default) values for all driver/subsystem options
*/

void
conf_free_summary(conf_summary_t *summary);
/* Deallocates a configuration summary
** Parameters: (conf_summary_t *) summary: The summary to deallocate
** This only affects the summary, not the underlying parse.
*/

conf_game_choice_t *
conf_find_choice(conf_summary_t *choices, char *game_id);
/* Finds the correct game choice by game ID
** Parameters: (conf_summary_t *) summary: The summary to peruse
**             (char *) game_id:  Identified game ID.
** We search by exact match or otherwise default.
*/

#endif /* !defined (CONF_SUMMARY_H_) */
