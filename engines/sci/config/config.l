/***************************************************************************
 config.l (C) 1999 Christoph Reichenbach


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

    Christoph Reichenbach (CJR) [jameson@linuxgames.com]

***************************************************************************/

%{
#include <stdio.h>
#include <stddef.h>
#include "conf_parse.h"
#include "conf_extension.h"
#include "sci_memory.h"
#include <assert.h>
#include <ctype.h>


/* unistd override for GNU flex for non-UNIX systems */
#ifndef HAVE_UNISTD_H
#  define YY_NO_UNISTD_H
#endif

#ifdef _MSC_VER
#  include <ctype.h>
#  include <direct.h>

#  define strcasecmp stricmp

#endif

char * cur_section = NULL; /* dynamically allocated */

typedef struct _file_stack {
	char *name;
	int line_nr; /* Which line were we included _from_? */
	conf_parse_t *input_spec; /* The config_parse_t that included this file */
	YY_BUFFER_STATE handle;
	struct _file_stack *next;
} file_stack_t;

static file_stack_t *file_stack = NULL;
static char *yy_filename = NULL;
static YY_BUFFER_STATE yy_fsci_active_buffer;

static int
push_file(char *name);

static int
pop_file(void);

FILE *initial_file;

static const char *
skip_while(const char *start, int (*predicate)(int));
/* Returns the first character after ``start'' which is 0 or for which ``predicate'' fails
** Parameters: (char *) start: Start position for the search
**             (int -> int) predicate: Termination predicate
** Returns   : (char *) The first character which is 0 or for which ``prediate'' fails
*/

static const char *
skip_until(const char *start, int (*predicate)(int));
/* Returns the first character after ``start'' which is 0 or for which ``predicate'' succeeds
** Parameters: (char *) start: Start position for the search
**             (int -> int) predicate: Termination predicate
** Returns   : (char *) The first character which is 0 or for which ``prediate'' succeeds
*/

static char *
string_between (const char *start, const char *end);
/* Allocates a copy of the string between ``start'' and ``end'' (exluding ``end'')
** Parameters: (char *) start: Start position for copying
**             (char *) end: End position for copying
*/

static int
is_separator_or_alnum(int character);
/* Determines whether ``character'' is an underscore, dash, period, or alphanumeric character
** Parameters: (int) character:  The char to test
** Returns   : (int) nonzero iff isalnum(character) or character equals '-' or '_' or ':' or '.'
*/

static int
is_double_quote(int character);
/* Determines whether ``character'' is '"'
** Parameters: (int) character: The char to test
** Returns   : (int) nonzero iff character = '"'
*/

static int
is_period(int character);
/* Determines whether ``character'' is '.'
** Parameters: (int) character: The char to test
** Returns   : (int) nonzero iff character = '.'
*/

static conf_parse_t *
next_conf_parse(int type);
/* Allocates a new conf_parse_t, tagged as being next in the input record
** Parameters: (int) type: The type of this section
** Returns   : (conf_parse_t *) An appropriate conf parse entry
** Note that the subsection is implicitly chained in with the present subsection.
*/

static void
record_whitespace(const char *yytext);
/* Records an instance of whitespace
** Parameters: (char *) yytext: The whitespace to copy and represent
** Returns   : (void)
*/

conf_extension_t *
conf_extension_alloc(int type);
/* Allocates a new chunk of configuration extension memory
** Parameters: (int) type: The type to pre-set
** Returns   : (conf_extension_t *) : Fresh memory, with only the type initialised
*/


static int
pop_file(void);


static conf_parse_t *current_confparse = NULL;
static conf_parse_t **current_confparse_start = NULL; /* Non-NULL if we request a pointer to the next current_confparse to be set; used for include files */
static int conf_line_nr = 1;

#define BAD_INT_VALUE -33333333

%}

DIGIT [0-9]
PARAMTOKEN [[:alnum:]_\:\/-]*
SCIVERSION {DIGIT}"."{DIGIT}{3}"."{DIGIT}{3}
NUMTOKEN {DIGIT}+
COORDTOKEN {NUMTOKEN},?[[:space:]]*
RECTTOKEN {COORDTOKEN}{3}{NUMTOKEN}
QUOTED_RECTTOKEN "\""{RECTTOKEN}"\""
NUMPARAMTOKEN {NUMTOKEN}|{PARAMTOKEN}
PATHTOKEN [[:alnum:]"/""\\""."]*
NUMPATHPARAMTOKEN {NUMPARAMTOKEN}|{PATHTOKEN}
QUOTED_NUMPARAMTOKEN "\"".*"\""

%%

"["[_[:alnum:]-]+"]"	{
	const char *start = yytext;
	const char *textstart = skip_until(start+1, is_separator_or_alnum);
	const char *textend = skip_while(textstart+1, is_separator_or_alnum);

	char *pre_pad = string_between(start, textstart);
	char *section_name = string_between(textstart, textend);
	char *post_pad = sci_strdup(textend);

	conf_parse_t *confdata = next_conf_parse(CONF_PARSE_TYPE_SUBSECTION);
	confdata->data.subsection.pre_whitespace = pre_pad;
	confdata->data.subsection.name = section_name;
	confdata->data.subsection.post_whitespace = post_pad;
}


({PARAMTOKEN}|.)+[[:blank:]]*"="[[:blank:]]*({NUMPATHPARAMTOKEN}|{QUOTED_NUMPARAMTOKEN}|{RECTTOKEN}|{QUOTED_RECTTOKEN})[[:blank:]]* {
	const char *name_start = yytext;
	const char *name_end = skip_while(name_start + 1, is_separator_or_alnum);

	const char *name_interperiod_2 = NULL;
	const char *name_interperiod_1 = skip_until(name_start + 1, is_period);

	if (*name_interperiod_1 == 0
	    || name_interperiod_1 > name_end)
		name_interperiod_1 = NULL; /* No separator */
	else {
		name_interperiod_2 = skip_until(name_interperiod_1 + 1, is_period);
		if (*name_interperiod_2 == 0)
			name_interperiod_2 = NULL; /* No separator */
	}

	const char *value_start = skip_until(name_end + 1, is_separator_or_alnum); /* Note that '=' is not a separator or alnum */
	const char *value_end = is_double_quote(*(value_start - 1))
		? skip_until(value_start + 1, is_double_quote) /* double quote:  Look for terminator */
		: skip_while(value_start + 1, is_separator_or_alnum); /* no double quote: Look for end of ``identifier'' like thing */

	char *subsystem_name = NULL;
	char *driver_name = NULL;
	char *option_name = NULL;

	if (name_interperiod_2) {
		subsystem_name = string_between(name_start, name_interperiod_1);
		driver_name = string_between(name_interperiod_1 + 1, name_interperiod_2);
		option_name = string_between(name_interperiod_2 + 1, name_end);
	} else if (name_interperiod_1) {
		subsystem_name = string_between(name_start, name_interperiod_1);
		option_name = string_between(name_interperiod_1 + 1, name_end);
	} else
		option_name = string_between(name_start, name_end);

	char *whitespace = string_between(name_end, value_start);
	char *value = string_between(value_start, value_end);
	fprintf(stderr, "value_end = %p\n", value_end);
	char *terminal_whitespace = sci_strdup(value_end);

	conf_parse_t *confdata = next_conf_parse(CONF_PARSE_TYPE_OPTION);
	confdata->data.assignment.subsystem = subsystem_name;
	confdata->data.assignment.driver = driver_name;
	confdata->data.assignment.option = option_name;
	confdata->data.assignment.centre_whitespace = whitespace;
	confdata->data.assignment.value = value;
	confdata->data.assignment.terminal_whitespace = terminal_whitespace;
}


(view|pic|cursor)[^_A-Za-z0-9"("]*"("([^";"]|"#"[^\n]*\n)*";" {

	conf_parse_t *confdata = next_conf_parse(CONF_PARSE_TYPE_EXTENSION);
	conf_extension_t *ext = conf_extension_alloc(CONF_EXT_TYPE_GFX);
	ext->data = sci_strdup(yytext);
	confdata->data.extension = ext;
}

"%include"[^<\n]*<[^>\n]*> {
	const char *file_start = strchr(yytext, '<') + 1;
	const char *file_end = strchr(file_start, '>');

	conf_parse_t *confdata = next_conf_parse(CONF_PARSE_TYPE_INCLUDE);
	char *include_file = string_between(file_start, file_end);
	confdata->data.include.include_prefix = string_between(yytext, file_start);
	confdata->data.include.include_suffix = sci_strdup(file_end);
	confdata->data.include.filename = include_file;
	confdata->data.include.modifiable = 0;

	if (push_file(include_file)) {
		YY_NEW_FILE;
	}
}


"#".+$ {/* Ignore comments */
	record_whitespace(yytext);
}

[[:blank:]\n]+ { /* Eat whitespace */
	record_whitespace(yytext);
	char *ypos = yytext - 1;
	while ((ypos = strchr(ypos + 1, '\n')))
	       ++conf_line_nr;
}

<<EOF>> {
	yy_delete_buffer( YY_CURRENT_BUFFER );
	yyterminate();
}

.*  {
	char *text = sci_strdup(yytext);
	conf_parse_t *parse = next_conf_parse(CONF_PARSE_TYPE_LEXERROR);
	parse->data.whitespace = text;
}

%%

static const char *
skip_while(const char *start, int (*predicate)(int))
{
	const char *pos = start;

	while (*pos && predicate(*pos))
		++pos;

	return pos;
}

static const char *
skip_until(const char *start, int (*predicate)(int))
{
	const char *pos = start;

	while (*pos && (!predicate(*pos)))
		++pos;

	return pos;
}

static char *
string_between(const char *start, const char *end)
{
	int length = end - start;
	assert(length >= 0);
	char * result = sci_malloc(length + 1);
	memcpy(result, start, length);
	result[length] = 0;	/* terminate */

	return result;
}

static int
is_separator_or_alnum(int character)
{
	return isalnum(character)
		|| (character == '.')
		|| (character == '_')
		|| (character == '-');
}

static int
is_double_quote(int character)
{
	return character == '"';
}

static int
is_period(int character)
{
	return character == '.';
}

int
yywrap(void)
{
	return pop_file(); /* no further input */
}

static int
push_file(char *name)
{
	file_stack_t *newfs;
	file_stack_t *s;
	FILE *newfile;

	if (yy_filename && !strcmp(name, yy_filename)) {
			fprintf(stderr, "[conf] Warning: Attempted immediate circular inclusion of config file '%s'\n",
				name);
		return 1;
	}

	s = file_stack;
	while (s) {
		if (!strcmp(name, s->name)) {
			fprintf(stderr, "[conf] Warning: Attempted circular inclusion of config file '%s'\n",
				name);
			return 1;
		}
		s = s->next;
	}

	if (!(newfile = fopen(name, "r"))) {
		fprintf(stderr, "[conf] Warning: Could not open configuration file '%s'\n", name);
		return 1;
	}

	if (yyin) {
		newfs = malloc(sizeof(struct _file_stack));
		newfs->handle = yy_fsci_active_buffer;
		newfs->name = yy_filename;

		newfs->next = file_stack;
		newfs->line_nr = conf_line_nr;
		newfs->input_spec = current_confparse;
		current_confparse = NULL;
		assert(newfs->input_spec->type == CONF_PARSE_TYPE_INCLUDE);
		current_confparse_start = &(newfs->input_spec->data.include.options_head);

		file_stack = newfs;
	}

	yy_filename = sci_strdup(name);
	yy_fsci_active_buffer = yy_create_buffer(newfile, YY_BUF_SIZE);
	yy_switch_to_buffer(yy_fsci_active_buffer);

	return 0;
}

static int
pop_file(void)
{
	if (file_stack) {
		void *goner = file_stack;
		yy_delete_buffer(yy_fsci_active_buffer);
		fclose(yyin);
		yy_fsci_active_buffer = file_stack->handle;
		yy_switch_to_buffer(yy_fsci_active_buffer);

		free(yy_filename);
		yy_filename = file_stack->name;
		conf_line_nr = file_stack->line_nr;

		/* Recover confparse */
		assert(file_stack->input_spec->type == CONF_PARSE_TYPE_INCLUDE);
		file_stack->input_spec->data.include.options_end = current_confparse;
		current_confparse = file_stack->input_spec;
		current_confparse_start = NULL;

		file_stack = file_stack->next;

		free(goner);
		return 0;
	} else {
		if (yy_filename) {
			free(yy_filename);
			yy_filename = NULL;
		}
		if (yyin) {
			yy_delete_buffer(yy_fsci_active_buffer);
			fclose(yyin);
			yyin = NULL;
		}
		return 1; /* Done */
	}
}


static void
record_whitespace(const char *yytext)
{
	char *text = sci_strdup(yytext);
	conf_parse_t *parse = next_conf_parse(CONF_PARSE_TYPE_WHITESPACE);
	parse->data.whitespace = text;
}


static conf_parse_t *
next_conf_parse (int type)
{
	conf_parse_t *result;

	fprintf(stderr, "Allocing entry...\n");
	result = sci_malloc(sizeof(conf_parse_t));
	fprintf(stderr, "Done...\n");

	if (!type) {
		BREAKPOINT();
	}

	result->type = type;
	result->line_nr = conf_line_nr;
	result->next = NULL;
	result->overridden = NULL;

	if (current_confparse) {
		current_confparse->next = result;
		result->prev = current_confparse;
	} else {
		result->prev = NULL;
	}

	if (current_confparse_start) {
		*current_confparse_start = result;
		current_confparse_start = NULL;
	}

	current_confparse = result;

	fprintf(stderr, "Final...\n");
	return result;
}

/* -------------- */
/* API operations */
/* -------------- */

conf_parse_t *
conf_read_file(char *config_file_name, int modifiable, conf_parse_t *previous)
{
	char *conf_path = sci_strdup(config_file_name);

	conf_parse_t *confdata = next_conf_parse(CONF_PARSE_TYPE_INCLUDE);
	confdata->data.include.include_prefix = NULL;
	confdata->data.include.include_suffix = NULL;
	confdata->data.include.filename = conf_path;
	confdata->data.include.modifiable = modifiable;

	if (previous) {
		/* Make sure to hook confdata in after previous */
		conf_parse_t **termp = &previous;

		while ((*termp)->next)
			termp = &((*termp)->next);

		(*termp)->next = confdata; /* Hook into the final pointer */
	}

	if ((push_file(conf_path))) {
		sci_free(conf_path);
		return NULL;
	}


	current_confparse_start = &confdata->data.include.options_head;
	yylex(); /* Parse the file */
	confdata->data.include.options_end = current_confparse;
	confdata->next = NULL;

	while (!pop_file());  /* Ignore error conditions- might be lex implementation dependant */

	if (previous)
		return previous;
	else
		return confdata;
}

#define XFREE(x) if (x) sci_free(x);

void
conf_free_parse(conf_parse_t *raw_config)
{
	while (raw_config) {
		switch (raw_config->type) {

		case CONF_PARSE_TYPE_WHITESPACE:
			sci_free(raw_config->data.whitespace);
			break;

		case CONF_PARSE_TYPE_SUBSECTION:
			sci_free(raw_config->data.subsection.pre_whitespace);
			sci_free(raw_config->data.subsection.name);
			sci_free(raw_config->data.subsection.post_whitespace);
			break;

		case CONF_PARSE_TYPE_OPTION:
			XFREE(raw_config->data.assignment.subsystem);
			XFREE(raw_config->data.assignment.driver);
			sci_free(raw_config->data.assignment.option);
			sci_free(raw_config->data.assignment.centre_whitespace);
			sci_free(raw_config->data.assignment.value);
			sci_free(raw_config->data.assignment.terminal_whitespace);
			break;

		case CONF_PARSE_TYPE_INCLUDE:
			XFREE(raw_config->data.include.include_prefix);
			XFREE(raw_config->data.include.include_suffix);
			sci_free(raw_config->data.include.filename);
			conf_free_parse(raw_config->data.include.options_head); /* recurse */
			break;

		case CONF_PARSE_TYPE_EXTENSION:
			conf_extension_free(raw_config->data.extension);
			raw_config->data.extension = NULL;
			break;

		case CONF_PARSE_TYPE_LEXERROR:
			sci_free(raw_config->data.whitespace);
			break;

		default:
			fprintf(stderr, "[conf] INTERNAL ERROR: Trying to free config segment of invalid type %d\n", raw_config->type);
			BREAKPOINT();
		}

		raw_config->type = -1;
		conf_parse_t *next = raw_config->next;
		sci_free(raw_config);
		raw_config = next;
	}
}

#undef DEBUG_CONF_PRINTING

#ifdef DEBUG_CONF_PRINTING
#define fputs(data, file) my_fputs(data, file)
#define fopen(filename, mode) my_fopen(filename, mode)
#define fclose(file) my_fclose(file)

FILE *
my_fopen(const char *filename, const char *mode)
{
	printf(">>> Opening ``%s'' for [%s]\n", filename, mode);
	return (FILE *) filename;
}

int
my_fputs(const char *data, const FILE *file)
{
	return puts(data);
}

int
fclose(const FILE *file)
{
	printf(">>> Closing ``%s''\n", file);
	return 0;
}
#endif

static void
conf_write_parse_file(conf_parse_t *confparse, FILE *file)
{
	while (confparse) {
		switch (confparse->type) {

		case CONF_PARSE_TYPE_LEXERROR:
		case CONF_PARSE_TYPE_WHITESPACE:
			fputs(confparse->data.whitespace, file);
			break;

		case CONF_PARSE_TYPE_SUBSECTION:
			fputs(confparse->data.subsection.pre_whitespace, file);
			fputs(confparse->data.subsection.name, file);
			fputs(confparse->data.subsection.post_whitespace, file);
			break;

		case CONF_PARSE_TYPE_OPTION:
			if (confparse->data.assignment.subsystem) {
				fputs(confparse->data.assignment.subsystem, file);
				fputs(".", file);
			}
			if (confparse->data.assignment.driver) {
				fputs(confparse->data.assignment.driver, file);
				fputs(".", file);
			}
			fputs(confparse->data.assignment.option, file);
			fputs(confparse->data.assignment.centre_whitespace, file);
			fputs(confparse->data.assignment.value, file);
			fputs(confparse->data.assignment.terminal_whitespace, file);
			break;

		case CONF_PARSE_TYPE_INCLUDE:
			if (confparse->data.include.include_prefix) {
				fputs(confparse->data.include.include_prefix, file);
				fputs(confparse->data.include.filename, file);
				fputs(confparse->data.include.include_suffix, file);
			}
			conf_write_parse(confparse); /* Recurse indirectly */
			break;

		case CONF_PARSE_TYPE_EXTENSION:
			conf_extension_print(file, confparse->data.extension);
			break;

		default:
			fprintf(stderr, "[conf] INTERNAL ERROR: Trying to print config segment of invalid type %d\n", confparse->type);
			BREAKPOINT();
		}

		confparse = confparse->next;
	}
}

void
conf_write_parse(conf_parse_t *raw_config)
{
	while (raw_config) {
		switch (raw_config->type) {

		case CONF_PARSE_TYPE_WHITESPACE:
		case CONF_PARSE_TYPE_SUBSECTION:
		case CONF_PARSE_TYPE_OPTION:
		case CONF_PARSE_TYPE_EXTENSION:
		case CONF_PARSE_TYPE_LEXERROR:
			break;

		case CONF_PARSE_TYPE_INCLUDE:
			if (raw_config->data.include.modifiable) {
				const char *filename = raw_config->data.include.filename;
				FILE *file = fopen(filename, "w");
				if (!file) {
					fprintf(stderr, "[conf] Could not write back to file ``%s'': ", filename);
					perror(NULL);
				} else {
					conf_write_parse_file(raw_config->data.include.options_head, file);
					fclose(file);
				}
			}
#ifdef DEBUG_CONF_PRINTING
			else printf(">>> Skipping file ``%s'' for output since it's tagged read-only\n", raw_config->data.include.filename);
#endif
			break;

		default:
			fprintf(stderr, "[conf] INTERNAL ERROR: Trying to print config segment of invalid type %d (on top-level)\n", raw_config->type);
			BREAKPOINT();
		}

		raw_config = raw_config->next;
	}
}

conf_extension_t *
conf_extension_alloc(int type)
{
	conf_extension_t *retval = sci_malloc(sizeof(conf_extension_t));
	retval->type = type;
	return retval;
}
