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

#ifndef CONF_DRIVER_H_
#define CONF_DRIVER_H_


/* Possible values for enum types */
typedef struct {
	char *name;
	int value;
} conf_value_t;

typedef struct {
	char *name;
	char *description;
	unsigned int type;
	unsigned int flags;
	union { char * str;
		int nr
	} default;	/* Optional default value */
	int min;			/* For subrange types */
	int max;			/* For subrange types; for enum types */
	conf_value_t *values;		/* For enum types; NULL-terminated */
	void *data;			/* User data */
} conf_option_t;

#define CONF_OPTION_TYPE_INT		0
#define CONF_OPTION_TYPE_SUBRANGE	1	/* ``interval'' type, i.e. bounded int */
#define CONF_OPTION_TYPE_STRING		2
#define CONF_OPTION_TYPE_BOOL		3	/* special built-in enum */
#define CONF_OPTION_TYPE_ENUM		4

#define CONF_OPTION_FLAG_MANDATORY	(1<<0)	/* Option MUST be set */
#define CONF_OPTION_FLAG_HAS_DEFAULT	(1<<1)	/* Option has a default value */
#define CONF_OPTION_FLAG_DYNAMIC	(1<<2)	/* Option may be altered after initialisation */


#define CONF_DRIVER_FAIL	1
#define CONF_DRIVER_SUCCESS	0

typedef struct conf_header {
	char [16] magic;		/* MUST be CONF_DRIVER_MAGIC */
	int freesci_version;		/* MUST be CONF_DRIVER_VERSION */
	int subsystem_id;		/* CONF_SUBSYSTEM_*, if appropriate */
	int subsystem_version;
	char *name;			/* Long driver name */
	char *version;			/* Long driver version */
	unsigned int dependencies;	/* CONF_DRIVER_DEPENDENCY_* */
	conf_option_t *options;		/* Last option has name of NULL */
	char * (*set_option)(void * self, /* points to base struct */
	                     conf_option_t *,
	                     union { char *str;
	                             int nr
	                           });	/* Set option, return static error (if applicable) or NULL on success */
} conf_header_t; /* Universal driver header */

struct conf_driver;
struct conf_subsystem;
struct conf_main;

#define CONF_SUBSYSTEM_GFX		0
#define CONF_SUBSYSTEM_SOUND		1	/* The sfx/player subsystem */
#define CONF_SUBSYSTEM_PCM_DEVICE	2
#define CONF_SUBSYSTEM_MIXER		3
#define CONF_SUBSYSTEM_SEQ		4	/* HW sequencer or generic softseq interface */
#define CONF_SUBSYSTEM_SOFTSEQ		5
#define CONF_SUBSYSTEM_TIMER		6

#define CONF_SUBSYSTEMS_NR		7

typedef struct conf_driver {
	conf_header_t header;
	char * (*init)(struct conf_driver *self,
	               struct conf_subsystem *owner);	/* Initialise, return static error message on error or NULL on success.
							** The owner is guaranteed to have been configured and guaranteed NOT to have
							** been initialised. */

	void (*exit)(void);
} conf_driver_t;

typedef struct conf_subsystem {
	conf_header_t header;
	char * (*init)(struct conf_subsystem *self,
	               struct conf_main *main,
	               struct conf_driver *driver);	/* Initialise, return static error message on error or NULL on success.
							** The driver is configured and initialised, the main reference configured but
							** not initialised. */
	void (*exit)(void);
	char *(*get_default_driver)(struct conf_subsystem *self,
	                            int index);	/* Get the nth default driver name, or NULL if there is none.  These are tried in order if
						** there is no explicit choice. */
	conf_driver_t **builtin_drivers;	/* NULL terminated list of built-in drivers */
	char *dynamic_driver_prefix;		/* string prefix to dynamically loaded drivers for this subsystem */
} conf_subsystem_t;


typedef struct conf_main {
	conf_header_t header;
	conf_subsystem_t *[CONF_SUBSYSTEMS_NR];
	char **dynamic_driver_locations; /* NULL terminated list of locations to check for dynamically loadable drivers */
	char * (*init)(struct conf_main *self);	/* Return static error message, or NULL on success. */
	void (*exit)(void);
} conf_main_t;


#define CONF_DRIVER_MAGIC "FreeSCI driver"

#define CONF_DRIVER_VERSION 1

/* Dependency flags */
#define CONF_DRIVER_DEPENDENCY_GFX		(1 << CONF_SUBSYSTEM_GFX)
#define CONF_DRIVER_DEPENDENCY_SOUND		(1 << CONF_SUBSYSTEM_SOUND)
#define CONF_DRIVER_DEPENDENCY_PCM_DEVICE	(1 << CONF_SUBSYSTEM_PCM_DEVICE)
#define CONF_DRIVER_DEPENDENCY_MIXER		(1 << CONF_SUBSYSTEM_MIXER)
#define CONF_DRIVER_DEPENDENCY_SEQ		(1 << CONF_SUBSYSTEM_SEQ)
#define CONF_DRIVER_DEPENDENCY_SOFTSEQ		(1 << CONF_SUBSYSTEM_SOFTSEQ)
#define CONF_DRIVER_DEPENDENCY_TIMER		(1 << CONF_SUBSYSTEM_TIMER)

#define CONF_BUILTIN_DEPENDENCIES	(CONF_DRIVER_DEPENDENCY_GFX | CONF_DRIVER_DEPENDENCY_SOUND)

/* ---------------------------------------------- */
/* -- convenience macros for driver developers -- */
/* ---------------------------------------------- */


char *
conf_default_set_option(void * self, conf_option_t *option,
                        union { char *str;
                                int nr
                              } value);
/* Default implementation of the option setting function
** Parameters: (void *) self: Reference to the structure we should be accessing
**             (conf_option_t *) option: The option to set
**             (char * | nr) value: The value to set
** Returns   : (char *) NULL on success, a static error string otherwise
*/


#define CONF_MK_HEADER(NAME, VERSION, SUBSYSTEM, DEPENDENCIES, OPTIONS, SETOPTION) \
	{								\
		CONF_DRIVER_MAGIC,					\
		CONF_DRIVER_VERSION,					\
		SUBSYSTEM,						\
		0,							\
		NAME,							\
		VERSION,						\
		DEPENDENCIES,						\
		OPTIONS,						\
		SETOPTION }

#define CONF_OPT_ANY(STRUCTURE, FIELD, NAME, TYPE, DEFAULT, MIN, MAX, VALUES, FLAGS, DESCRIPTION) \
	{								\
		NAME,							\
		DESCRIPTION,						\
		TYPE,							\
		FLAGS,							\
		DEFAULT,						\
		MIN,							\
		MAX,							\
		VALUES,							\
		(void *)(offsetof (STRUCTURE, FIELD))			\
	}

#define CONF_OPT_INT(S, F, NAME, DEFAULT, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_INT, {.nr = DEFAULT}, 0, 0, NULL, (FLAGS) | CONF_OPTION_HAS_DEFAULT, DESCR)
#define CONF_OPT_INT_NODEFAULT(S, F, NAME, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_INT, {.nr = 0}, 0, 0, NULL, (FLAGS), DESCR)

#define CONF_OPT_SUBRANGE(S, F, NAME, DEFAULT, MIN, MAX, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_SUBRANGE, {.nr = DEFAULT}, MIN, MAX, NULL, (FLAGS) | CONF_OPTION_HAS_DEFAULT, DESCR)
#define CONF_OPT_SUBRANGE_NODEFAULT(S, F, NAME, MIN, MAX, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_SUBRANGE, {.nr = 0}, MIN, MAX, NULL, (FLAGS), DESCR)

#define CONF_OPT_STRING(S, F, NAME, DEFAULT, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_STRING, {.str = DEFAULT}, 0, 0, NULL, (FLAGS) | CONF_OPTION_HAS_DEFAULT, DESCR)
#define CONF_OPT_STRING_NODEFAULT(S, F, NAME, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_STRING, {.str = NULL}, 0, 0, NULL, (FLAGS), DESCR)

#define CONF_OPT_BOOL(S, F, NAME, DEFAULT, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_INT, {.nr = DEFAULT}, 0, 0, NULL, (FLAGS) | CONF_OPTION_HAS_DEFAULT, DESCR)
#define CONF_OPT_BOOL_NODEFAULT(S, F, NAME, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_INT, {.nr = 0}, 0, 0, NULL, (FLAGS), DESCR)

#define CONF_OPT_ENUM(S, F, NAME, DEFAULT, CHOICES, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_INT, {.nr = DEFAULT}, 0, 0, CHOICES, (FLAGS) | CONF_OPTION_HAS_DEFAULT, DESCR)
#define CONF_OPT_ENUM_NODEFAULT(S, F, NAME, CHOICES, FLAGS, DESCR)  CONF_OPT_ANY(S, F, NAME, CONF_OPTION_TYPE_INT, {.nr = 0}, 0, 0, CHOICES, (FLAGS), DESCR)

#define CONF_LAST_OPTION { NULL, NULL, -1, 0, 0, 0, 0, NULL }


#endif /* !defined(CONF_DRIVER_H_) */
