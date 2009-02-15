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
#include <engine.h>
#include <gfx_resource.h>
#include <sci_conf.h>
#include <stddef.h>

/* unistd override for GNU flex for non-UNIX systems */
#ifndef HAVE_UNISTD_H
#  define YY_NO_UNISTD_H
#endif

#ifdef _MSC_VER
#  include <ctype.h>
#  include <direct.h>
#endif

config_entry_t *conf;
int cur_section=0; /* Size-1 and current section in conf */
char *exported_conf_path; /* Path which the config file was found in */
int dospath; /* Use dos-style paths? */

typedef struct {
	const char *name;
	int value;
} name_value_pair;


struct {
	const char *name;
	void *(*check_driver)(char *name);
} freesci_subsystems[FREESCI_DRIVER_SUBSYSTEMS_NR] = {
	{"gfx", NULL},
#ifdef __GNUC__
#warning "sound"
#endif
#if 0
        {"pcm", parse_pcmout_driver},
	{"midiout", parse_midiout_driver}
#endif
};


typedef struct _file_stack {
	char *name;
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

static void
set_config_parameter(config_entry_t *conf, char *subsystem_name, char *driver_name,
		       char *option, char *value);


static int
parse_name(char *name, name_value_pair* nvps, const char *what, int oldval); /* Parses a string with a name value pair */

static void
copy_subsystem_options(config_entry_t *dest, config_entry_t *src);
/* Copies all subsystem options
** Parameters: (config_entry_t *) dest: The destination config struct
**             (config_entry_t *) src: Source struct
** Returns   : (void)
*/


static name_value_pair valid_modes[] = {
	{"default", GFXR_DITHER_MODE_D16},
	{"dither", GFXR_DITHER_MODE_D16},
	{"dither16", GFXR_DITHER_MODE_D16},
	{"dither_16", GFXR_DITHER_MODE_D16},
	{"dither-16", GFXR_DITHER_MODE_D16},
	{"d16", GFXR_DITHER_MODE_D16},
	{"flat", GFXR_DITHER_MODE_F256},
	{"interpol", GFXR_DITHER_MODE_F256},
	{"interpolate", GFXR_DITHER_MODE_F256},
	{"dither256", GFXR_DITHER_MODE_D256},
	{"dither_256", GFXR_DITHER_MODE_D256},
	{"d256", GFXR_DITHER_MODE_D256},
	{0, 0} /* Terminator */
};

static name_value_pair yesno[] = {
	{"yes", 1},
	{"no", 0},
	{"true", 1},
	{"false", 0},
	{"1", 1},
	{"0", 0},
	{"ok", 1},
	{"enable", 1},
	{"disable", 0},
	{"activate", 1},
	{"deactivate", 0},
	{"+", 1},
	{"-", 0},
	{"on", 1},
	{"off", 0},
	{0, 0}
};

static name_value_pair dither_pattern[] = {
	{"scaled", GFXR_DITHER_PATTERN_SCALED},
	{"unscaled", GFXR_DITHER_PATTERN_1},
	{"one", GFXR_DITHER_PATTERN_1},
	{"1", GFXR_DITHER_PATTERN_1},
	{0, 0}
};

static name_value_pair dirty_strategy[] = {
	{"1", GFXOP_DIRTY_FRAMES_ONE},
	{"one", GFXOP_DIRTY_FRAMES_ONE},
	{"cluster", GFXOP_DIRTY_FRAMES_CLUSTERS},
	{"clusters", GFXOP_DIRTY_FRAMES_CLUSTERS},
	{0, 0}
};

static name_value_pair brush_mode[] = {
	{"scaled", GFX_BRUSH_MODE_SCALED},
	{"ellipse", GFX_BRUSH_MODE_ELLIPSES},
	{"ellipses", GFX_BRUSH_MODE_ELLIPSES},
	{"rnd_ellipses", GFX_BRUSH_MODE_RANDOM_ELLIPSES},
	{"rnd-ellipses", GFX_BRUSH_MODE_RANDOM_ELLIPSES},
	{"random_ellipses", GFX_BRUSH_MODE_RANDOM_ELLIPSES},
	{"random-ellipses", GFX_BRUSH_MODE_RANDOM_ELLIPSES},
	{"morerandom", GFX_BRUSH_MODE_MORERANDOM},
	{"more-random", GFX_BRUSH_MODE_MORERANDOM},
	{"more_random", GFX_BRUSH_MODE_MORERANDOM},
	{0, 0}
};

static name_value_pair filter_mode[] = {
	{"none", GFX_XLATE_FILTER_NONE},
	{"linear", GFX_XLATE_FILTER_LINEAR},
	{"bilinear", GFX_XLATE_FILTER_LINEAR},
	{"bi-linear", GFX_XLATE_FILTER_LINEAR},
	{"trilinear", GFX_XLATE_FILTER_TRILINEAR},
	{"tri-linear", GFX_XLATE_FILTER_TRILINEAR},
	{0, 0}
};

static name_value_pair antialiasing_modes[] = {
	{"none", GFXR_ANTIALIASING_NONE},
	{"0", GFXR_ANTIALIASING_NONE},
	{"off", GFXR_ANTIALIASING_NONE},
	{"on", GFXR_ANTIALIASING_SIMPLE},
	{"basic", GFXR_ANTIALIASING_SIMPLE},
	{"simple", GFXR_ANTIALIASING_SIMPLE}
};

static name_value_pair line_mode[] = {
	{"correct", GFX_LINE_MODE_CORRECT},
	{"normal", GFX_LINE_MODE_CORRECT},
	{"fast", GFX_LINE_MODE_FAST},
	{"half", GFX_LINE_MODE_FAST},
	{"fine", GFX_LINE_MODE_FINE},
	{"thin", GFX_LINE_MODE_FINE},
	{0, 0}
};

#define BAD_INT_VALUE -33333333

/* Types of options */
#define OPTION_TYPE_NONE 0
#define OPTION_TYPE_INT 1
#define OPTION_TYPE_NVP 2
#define OPTION_TYPE_INVERSE_NVP 3
#define OPTION_TYPE_STATICREF 4
#define OPTION_TYPE_STRING 5
#define OPTION_TYPE_RECT 6

typedef struct {
	int type;
	const char *name;
	int min;
	int max;
	name_value_pair *nvp;
	void * (*parse_funct)(char *);
	int varoffset;
} standard_option;

#define OPT_END                               {OPTION_TYPE_NONE, NULL, 0, 0, NULL, 0}
/* Terminates */

#define OPT_INT(NAME, VARNAME, MIN, MAX)      {OPTION_TYPE_INT, NAME, MIN, MAX, NULL, NULL, offsetof (config_entry_t, VARNAME)}
/* Read INT from the interval [MIN, MAX] */

#define OPT_STRING(NAME, VARNAME)             {OPTION_TYPE_STRING, NAME, 0, 0, NULL, NULL, offsetof (config_entry_t, VARNAME)}
/* Read a string */

#define OPT_NVP(NAME, VARNAME, NVP)           {OPTION_TYPE_NVP, NAME, 0, 0, NVP, NULL, offsetof (config_entry_t, VARNAME)}
/* Read (name,value) pair from NVP */

#define OPT_INVERSE_NVP(NAME, VARNAME, NVP)   {OPTION_TYPE_INVERSE_NVP, NAME, 0, 0, NVP, NULL, offsetof (config_entry_t, VARNAME)}
/* Read NVP and negate result */

#define OPT_STATICREF(NAME, VARNAME, FUNCTN)  {OPTION_TYPE_STATICREF, NAME, 0, 0, NULL, FUNCTN, offsetof (config_entry_t, VARNAME)}
/* Call FUNCTN() with the specified value, store resulting NULL* */

#define OPT_RECT(NAME, VARNAME)  {OPTION_TYPE_RECT, NAME, 0, 0, NULL, NULL, offsetof (config_entry_t, VARNAME)}
/* Read a rectangle */


standard_option standard_options[] = {
	OPT_RECT("pic_port_bounds", gfx_options.pic_port_bounds),
	OPT_NVP("pic0_dither_mode", gfx_options.pic0_dither_mode, valid_modes),
	OPT_NVP("color_mode", gfx_options.pic0_dither_mode, valid_modes),
	OPT_NVP("pic0_dither_pattern", gfx_options.pic0_dither_pattern, dither_pattern),
	OPT_NVP("pic0_brush_mode", gfx_options.pic0_brush_mode, brush_mode),
	OPT_NVP("pic0_line_mode", gfx_options.pic0_line_mode, line_mode),
	OPT_NVP("dirty_strategy", gfx_options.dirty_frames, dirty_strategy),
	OPT_INVERSE_NVP("pic0_scaled", gfx_options.pic0_unscaled, yesno),
	OPT_NVP("pic_filter", gfx_options.pic_xlate_filter, filter_mode),
	OPT_NVP("pic_antialiasing", gfx_options.pic0_antialiasing, antialiasing_modes),
	OPT_NVP("text_filter", gfx_options.text_xlate_filter, filter_mode),
	OPT_NVP("view_filter", gfx_options.view_xlate_filter, filter_mode),
	OPT_NVP("cursor_filter", gfx_options.cursor_xlate_filter, filter_mode),
	OPT_NVP("mouse", mouse, yesno),
	OPT_NVP("reverse_stereo", reverse_stereo, yesno),
	OPT_INT("pic_buffer_size", gfx_options.buffer_pics_nr, 0, 4096),
	OPT_INT("alpha_threshold", alpha_threshold, 0, 255),
	OPT_INT("animation_delay", animation_delay, 0, 1000000),
	OPT_INT("animation_granularity", animation_granularity, 1, 160),
#ifdef __GNUC__
#warning "Re-enable sound server config"
#endif
#if 0
	OPT_STATICREF("midiout_driver", midiout_driver, parse_midiout_driver),
	OPT_STATICREF("midi_device", midi_device, parse_midi_device),
	OPT_STATICREF("sound_server", sound_server, parse_sound_server),
	OPT_STATICREF("pcmout_driver", pcmout_driver, parse_pcmout_driver),
	OPT_INT("pcmout_rate", pcmout_rate, 11025, 48000),
	OPT_INT("pcmout_stereo", pcmout_stereo, 0, 1),
#endif
	OPT_STRING("console_log", console_log),
	OPT_STRING("module_path", module_path),
	OPT_STRING("menu_dir", menu_dir),
	OPT_STRING("gfx_driver", gfx_driver_name),
	OPT_INT("scale_x", x_scale, 1, 256),
	OPT_INT("scale_y", y_scale, 1, 256),
	OPT_INT("scale", scale, 1, 256),
	OPT_INT("resource_version", res_version, 0, 6),
	OPT_INT("color_depth", color_depth, 8, 32),
	OPT_END
};


static void
parse_option(char *option, int optlen, char *value);

char *
crop_value(char *yytext);

char *
purge_comments(char *comments);

%}

DIGIT [0-9]
PARAMTOKEN [[:alnum:]"."_\:\/-]*
SCIVERSION {DIGIT}"."{DIGIT}{3}"."{DIGIT}{3}
NUMTOKEN {DIGIT}+
COORDTOKEN {NUMTOKEN},?[[:space:]]*
RECTTOKEN {COORDTOKEN}{3}{NUMTOKEN}
NUMPARAMTOKEN {NUMTOKEN}|{PARAMTOKEN}
PATHTOKEN [[:alnum:]"/""\\""."]*
NUMPATHPARAMTOKEN {NUMPARAMTOKEN}|{PATHTOKEN}
QUOTED_NUMPARAMTOKEN "\"".*"\""

%%

"["[_[:alnum:]]+"]" {
	char *cleanup;
	++yytext; /* Get over opening bracket */

	++cur_section; /* Start new section */

	/* Create new entry... */
	conf = (config_entry_t *) sci_realloc(conf, sizeof(config_entry_t) * (cur_section + 1));

	/* ...and initialize it */
	memcpy(&(conf[cur_section]), &(conf[0]), sizeof(config_entry_t));
	if (conf[0].console_log)
		conf[cur_section].console_log = sci_strdup (conf[0].console_log);

	/* Copy the subsystem init strings */
	copy_subsystem_options(conf + cur_section, conf);

	while (isspace(*yytext))
		yytext++;

	cleanup = strchr(yytext, ']');

	do {
		*cleanup-- = 0;
	} while (isblank(*cleanup));

	conf[cur_section].name = sci_strdup(yytext);

	conf[cur_section].resource_dir = sci_strdup(".");

}


version[[:blank:]]*"="[[:blank:]]*{SCIVERSION} {

	yytext = strchr(yytext, '=') + 1;

	while (isspace(*yytext))
		yytext++;

	version_parse(yytext, &conf[cur_section].version);
}

resource_dir[[:blank:]]*"="[[:blank:]]*.+	if (cur_section) {
	yytext = strchr(yytext, '=') + 1;
	while (isspace(*yytext))
		yytext++;

	sci_free(conf[cur_section].resource_dir);

	conf[cur_section].resource_dir = sci_strdup(yytext);
}

debug_mode[[:blank:]]*"="[[:blank:]]*.+  {
        yytext = strchr(yytext, '=') + 1;

        while (isspace(*yytext))
                yytext++;

	strcpy (conf[cur_section].debug_mode, yytext);
}

[[:alnum:]]+"."[[:alnum:]]+"."[[:alnum:]_]+[[:blank:]]*"="[[:blank:]]*({NUMPARAMTOKEN}|{QUOTED_NUMPARAMTOKEN})[[:blank:]]* {
/* driver parameters */
	char *subsys_name = yytext;
	char *driver_name;
	char *option, *value;
	char *p2;

	yytext = strchr(yytext, '.');
	*yytext++ = 0;
	driver_name = yytext;
	yytext = strchr(yytext, '.');
	*yytext++ = 0;


	option = yytext;
	yytext = strchr(yytext, '=');
	*yytext++ = 0;

	p2 = yytext-2;			/* trim right spaces */
	while (p2 > option && isspace (*p2))
		*p2-- = 0;

	value = crop_value(yytext); /* Get config value */

	set_config_parameter(conf + cur_section, subsys_name,
			     driver_name, option, value);

}


{PARAMTOKEN}[[:blank:]]*"="[[:blank:]]*({NUMPATHPARAMTOKEN}|{QUOTED_NUMPARAMTOKEN})[[:blank:]]* { /* Normal config option */
	char *option_str = yytext;
	char *value_str = yytext;
	int option_str_len;

	while (isalnum(*value_str) || *value_str == '_')
		++value_str;

	option_str_len = value_str - option_str;

	while (!(isalnum(*value_str) || *value_str == '_' || *value_str == '"' || *value_str == '/' || *value_str == '\\'))
		++value_str;

	value_str = crop_value(value_str);

	parse_option(option_str, option_str_len, value_str);
}

{PARAMTOKEN}[[:blank:]]*"="[[:blank:]]*\"{RECTTOKEN}\" { /* Normal config option */
	char *option_str = yytext;
	char *value_str = yytext;
	int option_str_len;

	while (isalnum(*value_str) || *value_str == '_')
		++value_str;

	option_str_len = value_str - option_str;

	while (!(isdigit(*value_str)||(*value_str == '"')))
		++value_str;

	value_str = crop_value(value_str);

	parse_option(option_str, option_str_len, value_str);
}


(view|pic|cursor)[^_A-Za-z0-9"("]*"("([^";"]|"#"[^\n]*\n)*";" {
	gfx_update_conf(&(conf[cur_section].gfx_options), purge_comments(yytext));
}

"%include"[^<\n]*<[^>\n]*> {
	char *filename = strchr(yytext, '<');
	char *end = strchr(filename, '>');

	*end-- = 0;
	while (isblank(*end))
		*end-- = 0;

	filename++;
	while (*filename && isblank(*filename))
		filename++;

	push_file(filename);
	YY_NEW_FILE;
}


"#".+$ /* Ignore comments */

[[:blank:]\n]+ /* Eat whitespace */

<<EOF>> {
	yy_delete_buffer( YY_CURRENT_BUFFER );
	yyterminate();
}

.* printf("Unrecognized option: '%s'\n", yytext);

%%

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
		newfs = (struct _file_stack *) malloc(sizeof(struct _file_stack));
		newfs->handle = yy_fsci_active_buffer;
		newfs->name = yy_filename;

		newfs->next = file_stack;
		file_stack = newfs;
	}

	yy_filename = strdup(name);
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


char *
crop_value(char *yytext)
{
	char *retval;

	while (isspace(*yytext))
		++yytext;

	retval = yytext;
	if (*yytext == '"') { /* Magic Quote Mode */
		++retval;
		++yytext;
		while (*yytext && (*yytext != '"'))
			++yytext;
		*yytext = 0; /* Terminate */
	} else {
		while (*yytext && !isspace(*yytext))
			++yytext;
		*yytext = 0; /* Terminate */
	}

	return retval;
}


int
config_init(config_entry_t **_conf, char *conffile)
{
	char *homedir = sci_get_homedir();
	char *conf_path;
	int i;

	conf = (config_entry_t *) sci_malloc(sizeof(config_entry_t));
#ifdef SATISFY_PURIFY
	memset(conf, 0, sizeof(config_entry_t));
#endif

/**** Default config: */
	conf->gfx_options.workarounds = 0;
	conf->gfx_options.buffer_pics_nr = 0;
	conf->gfx_options.correct_rendering = 1;
	conf->gfx_options.pic0_unscaled = 1;
	conf->gfx_options.pic0_dither_mode = GFXR_DITHER_MODE_D256;
	conf->gfx_options.pic0_dither_pattern = GFXR_DITHER_PATTERN_SCALED;
	conf->gfx_options.pic0_brush_mode = GFX_BRUSH_MODE_RANDOM_ELLIPSES;
	conf->gfx_options.pic0_line_mode = GFX_LINE_MODE_CORRECT;
	conf->gfx_options.cursor_xlate_filter = GFX_XLATE_FILTER_NONE;
	conf->gfx_options.view_xlate_filter = GFX_XLATE_FILTER_NONE;
	conf->gfx_options.pic_xlate_filter = GFX_XLATE_FILTER_NONE;
	conf->gfx_options.text_xlate_filter = GFX_XLATE_FILTER_NONE;
	conf->gfx_options.dirty_frames = GFXOP_DIRTY_FRAMES_CLUSTERS;
	conf->gfx_options.pic0_antialiasing = GFXR_ANTIALIASING_NONE;
	conf->gfx_options.pic_port_bounds = gfx_rect(0,10,320,190);
	for (i = 0; i < GFX_RESOURCE_TYPES_NR; i++) {
		conf->gfx_options.res_conf.assign[i] = NULL;
		conf->gfx_options.res_conf.mod[i] = NULL;
	}

	conf->gfx_driver_name = NULL;

#ifdef __GNUC__
#warning "Re-enable sound stuff"
#endif
#if 0
	conf->pcmout_driver = pcmout_find_driver(NULL);
	conf->pcmout_rate = 22050;
	conf->pcmout_stereo = 1;
	conf->midiout_driver = midiout_find_driver(NULL);
	conf->midi_device = midi_find_device(NULL);
	conf->sound_server = sound_server_find_driver(NULL);
#endif

	conf->x_scale = 0;
	conf->y_scale = 0;
	conf->scale = 0;
	conf->color_depth = 0;

	conf->mouse = 1;
	conf->reverse_stereo = 0;

	conf->version = 0;

	conf->alpha_threshold = 0x90;
	conf->animation_delay = 5;
	conf->animation_granularity = 4;
	conf->console_log = NULL;
	conf->debug_mode [0] = '\0';
	conf->name = NULL;
	conf->resource_dir = NULL;
        conf->module_path = sci_strdup(SCI_DEFAULT_MODULE_PATH);
	conf->res_version = SCI_VERSION_AUTODETECT;

	if (homedir) {
		conf->menu_dir = (char*)sci_malloc(strlen(homedir) + strlen(FREESCI_GAMEDIR)
			+ strlen(FREESCI_GAMES_DIR) + 2 * strlen(G_DIR_SEPARATOR_S) + 1);
		strcpy(conf->menu_dir, homedir);
		strcat(conf->menu_dir, G_DIR_SEPARATOR_S);
		strcat(conf->menu_dir, FREESCI_GAMEDIR);
		strcat(conf->menu_dir, G_DIR_SEPARATOR_S);
		strcat(conf->menu_dir, FREESCI_GAMES_DIR);
	}
	else
		conf->menu_dir = NULL;

	for (i = 0; i < FREESCI_DRIVER_SUBSYSTEMS_NR; i++)
		conf->driver_options[i] = NULL;
/**** Default config ends */


	if (conffile) {
		exported_conf_path = (char *) sci_malloc(PATH_MAX + 1);
		getcwd(exported_conf_path, PATH_MAX+1);

		conf_path = sci_strdup(conffile); /* Use config file if supplied */
	} else {
		if (!homedir) { /* We're probably not under UNIX if this happens */

			conf_path = sci_strdup(FREESCI_CONFFILE_DOS); /* Use DOS config style */

			exported_conf_path = (char *) sci_malloc(PATH_MAX + 1);
			getcwd(exported_conf_path, PATH_MAX+1);

			dospath = 1; /* Use DOS-style paths */

		} else {

			/* So we've got a home directory */
			if (chdir(homedir)) {
				fprintf(stderr,"Warning: Could not enter home directory: %s\n", homedir);
				*_conf = conf; /* Set the result variable */
				return 1;
			}

			if (chdir(FREESCI_GAMEDIR))
				if (scimkdir(FREESCI_GAMEDIR, 0700)) {

					fprintf(stderr,"Warning: Could not enter/create ~/"FREESCI_GAMEDIR"\n");
					*_conf = conf; /* Set the result variable */
					return 1;
				}

			conf_path = (char *) sci_malloc(strlen(homedir) + 3 + strlen(FREESCI_GAMEDIR) + strlen(FREESCI_CONFFILE));
			strcpy(conf_path, homedir);
			strcat(conf_path, "/");
			strcat(conf_path, FREESCI_GAMEDIR);

			exported_conf_path = sci_strdup(conf_path);

			strcat(conf_path, "/");
			strcat(conf_path, FREESCI_CONFFILE);

			dospath = 0; /* Use UN*X-style paths */
		}
	} /* !conffile */


	if ((push_file(conf_path))) {
		printf("No configuration file found; using defaults.\n");
		*_conf = conf; /* Set the result variable */
		sci_free(conf_path);
		sci_free(exported_conf_path);
		return 1;
	}

	printf("Reading configuration...\n");

	yylex(); /* Parse the file */

	while (!pop_file());  /* Ignore error conditions- might be lex implementation dependant */
	sci_free(conf_path);
	sci_free(exported_conf_path);

	*_conf = conf; /* Store the result */
	return cur_section + 1;
}


static void
config_free_driver_options(driver_option_t *option)
{
	if (option) {
		sci_free(option->option);
		sci_free(option->value);

		config_free_driver_options(option->next);
		sci_free(option);
	}
}

static void
config_free_driver_subsystem(subsystem_options_t *subsys)
{
	if (subsys) {
		sci_free(subsys->name);

		config_free_driver_options(subsys->options);

		config_free_driver_subsystem(subsys->next);
		sci_free(subsys);
	}
}


void
config_free(config_entry_t **conf, int entries)
{
	int i;

	if ((*conf)->console_log)
		sci_free((*conf)->console_log);

	if ((*conf)->module_path)
		sci_free((*conf)->module_path);

	if ((*conf)->menu_dir)
		sci_free((*conf)->menu_dir);

	for (i = 0; i < entries; i++) {
		int j;

		if (i >= 1) {
			sci_free((*conf)[i].name);
			if ((*conf)[i].resource_dir)
				sci_free((*conf)[i].resource_dir);
			if ((*conf)[i].console_log)
				sci_free((*conf)[i].console_log);
		}

		for (j = 0; j < FREESCI_DRIVER_SUBSYSTEMS_NR; j++) {
			if ((*conf)[i].driver_options[j])
				config_free_driver_subsystem((*conf)[i].driver_options[j]);
		}
	}

	sci_free(*conf);
}


static int
parse_name(char *name, name_value_pair *nvps, const char *what, int oldval)
{
	int i = 0;

	while (nvps[i].name) {
		if (0 == strcasecmp(name, nvps[i].name))
			return nvps[i].value;

		i++;
	}

	printf("Invalid %s mode %s\n", what, name);

	return oldval;
}


#ifdef __GNUC__
#warning "Re-enable sound stuff"
#endif
#if 0
void *
parse_sound_server(char *driver_name)
{
	sound_server_t *retval = sound_server_find_driver(driver_name);

	if (retval)
		return (void *) retval;
	/* not found - return default */

	printf ("Unknown sound server %s\n", driver_name);
	return (void *) conf->sound_server;
}

void *
parse_midiout_driver(char *driver_name)
{
	midiout_driver_t *retval = midiout_find_driver(driver_name);

	if (retval)
		return (void *) retval;
	/* not found - return default */

	printf ("Unknown midiout driver %s\n", driver_name);
	return (void *) conf->midiout_driver;
}


void *
parse_midi_device(char *driver_name)
{
	midi_device_t *retval = midi_find_device(driver_name);

	if (retval)
		return (void *) retval;
	/* not found - return default */

	printf ("Unknown MIDI device %s\n", driver_name);
	return (void *) conf->midi_device;
}

void *
parse_pcmout_driver(char *driver_name)
{
	pcmout_driver_t *retval = pcmout_find_driver(driver_name);

	if (retval)
		return (void *) retval;
	/* not found - return default */

	printf ("Unknown pcmout driver %s\n", driver_name);
	return (void *) conf->pcmout_driver;
}
#endif

static void
parse_option(char *option, int optlen, char *value)
{
	int optindex = 0;
	standard_option *opt = NULL;

	while (!opt && standard_options[optindex].type)
		if (optlen == strlen(standard_options[optindex].name)
		    && !strncmp(standard_options[optindex].name, option, optlen))
			opt = &(standard_options[optindex]);
		else
			optindex++;

	if (!opt) {
		fprintf(stderr,"Invalid option '%s'\n", option);
		return;
	}

	switch (opt->type) {

	case OPTION_TYPE_INT: {
		char *foo;
		int int_value = strtol(value, &foo, 0);

		if (*foo) {
			fprintf(stderr, "Option '%s' expects numeric value; encountered '%s'\n",
				opt->name, value);
			return;
		}

		if (int_value < opt->min) {
			fprintf(stderr, "Option '%s' expects value >= %d; encountered '%s'\n", opt->name, opt->min, value);
			return;
		}

		if (int_value > opt->max) {
			fprintf(stderr, "Option '%s' expects value <= %d; encountered '%s'\n", opt->name, opt->max, value);
			return;
		}

		*((int *)(((char *)&(conf[cur_section])) + opt->varoffset)) = int_value; /* Store value */

		break;
	}

	case OPTION_TYPE_STRING: {
		char **stringref = ((char **)(((char *)&(conf[cur_section])) + opt->varoffset));
		if (*stringref)
			sci_free(*stringref);
		*stringref = sci_strdup(value); /* Store value */
		break;
	}

	case OPTION_TYPE_INVERSE_NVP:
	case OPTION_TYPE_NVP: {
		int int_value = parse_name(value, opt->nvp, opt->name, BAD_INT_VALUE);

		if (int_value != BAD_INT_VALUE) {

			if (opt->type == OPTION_TYPE_INVERSE_NVP)
				int_value = !int_value;

/* FUCKED HERE: cur_section = 0, opt->varoffset = 205 */
			*((int *)(((char*)&(conf[cur_section])) + opt->varoffset)) = int_value; /* Store value */
		}
		break;
	}


	case OPTION_TYPE_RECT: {
		char *seeker = value;

		/* A rect_t is four integers */
		int *result = (int *) ((char *) &(conf[cur_section]) + opt->varoffset);
		int i;

		for (i=0;i<4;i++)
		{
			*(result++) = strtol(seeker, &seeker, 10);
			if (i < 3)
			{
				while (((*seeker == ',') || (*seeker == ' ')) &&
				       (*seeker != 0))
					seeker++;
				if ((*seeker < '0') || (*seeker > '9'))
				{
					fprintf(stderr, "Option '%s' expects a rectangle\n", opt->name);
					return;
				}
			}
		}

		break;
	}


	case OPTION_TYPE_STATICREF: {
		*((void **)(((char *)&(conf[cur_section])) + opt->varoffset)) = opt->parse_funct(value);
		break;
	}

	default:
		fprintf(stderr, "INTERNAL ERROR in %s, parse_option(), line %d\n", __FILE__, __LINE__);
	}
}


driver_option_t *
get_driver_options(config_entry_t *config, int subsystem, const char *name)
{
	subsystem_options_t *options;

	if (subsystem < 0 || subsystem >= FREESCI_DRIVER_SUBSYSTEMS_NR) {
		fprintf(stderr, "Attempt to get options from invalid subsystem #%d!\n", subsystem);
		return NULL;
	}

	if (!config)
		return NULL;


	options = config->driver_options[subsystem];

	while (options && strcasecmp(options->name, name))
		options = options->next;

	if (options)
		return options->options;

	return NULL;
}

static driver_option_t *
clone_driver_options(driver_option_t *options)
{
	driver_option_t *retval;

	if (!options)
		return NULL;

	retval = (driver_option_t *) sci_malloc(sizeof(driver_option_t));
	retval->option = sci_strdup(options->option);
	retval->value = sci_strdup(options->value);
	retval->next = clone_driver_options(options->next);

	return retval;
}

static subsystem_options_t *
clone_subsystem_options(subsystem_options_t *options)
{
	subsystem_options_t *retval;

	if (!options)
		return NULL;

	retval = (subsystem_options_t *) sci_malloc(sizeof(subsystem_options_t));
	retval->name = sci_strdup(options->name);
	retval->options = clone_driver_options(options->options);
	retval->next = clone_subsystem_options(options->next);

	return retval;
}

static void
copy_subsystem_options(config_entry_t *dest, config_entry_t *src)
{
	int i;
	for (i = 0; i < FREESCI_DRIVER_SUBSYSTEMS_NR; i++)
		dest->driver_options[i] = clone_subsystem_options(src->driver_options[i]);
}


char *
purge_comments(char *comments)
{
	char *c = comments;
	int overwrite = 0;
	char ch;

	/* Tear out all comments */
	while ((ch = *c)) {
		if (ch == '#')
			overwrite = 1;
		if (ch == '\n')
			overwrite = 0;
		if (overwrite)
			*c = ' ';

		c++;
	}

	return comments;
}

static void
set_config_parameter(config_entry_t *conf, char *subsystem_name, char *driver_name,
		     char *option, char *value)
{
	subsystem_options_t **subsys_optionsp;
	driver_option_t **driver_optionsp;
	int subsystem_nr = -1;
	int i = 0;

	while (subsystem_nr == -1 && i < FREESCI_DRIVER_SUBSYSTEMS_NR) {
		if (!strcasecmp(subsystem_name, freesci_subsystems[i].name))
			subsystem_nr = i;
		i++;
	}

	if (subsystem_nr == -1) {
		sciprintf("config file: There is no subsystem named '%s'\n", subsystem_name);
		return;
	}

#if 0
	if (!(freesci_subsystems[subsystem_nr].check_driver(driver_name))) {
		sciprintf("config file: There is no %s driver called '%s'\n", subsystem_name, driver_name);
		return;
	}
#endif

	subsys_optionsp = &(conf->driver_options[subsystem_nr]);

	while (*subsys_optionsp && strcasecmp((*subsys_optionsp)->name, driver_name))
		subsys_optionsp = &((*subsys_optionsp)->next);

	if (!*subsys_optionsp) {
		*subsys_optionsp = (subsystem_options_t *) sci_malloc(sizeof(subsystem_options_t));
		(*subsys_optionsp)->name = sci_strdup(driver_name);
		(*subsys_optionsp)->next = NULL;
		(*subsys_optionsp)->options = NULL;
	}

	driver_optionsp = &((*subsys_optionsp)->options);

	while (*driver_optionsp && strcasecmp((*driver_optionsp)->option, option))
		driver_optionsp = &((*driver_optionsp)->next);

	if (*driver_optionsp) {
		sci_free((*driver_optionsp)->value);
	} else {
		*driver_optionsp = (driver_option_t *) sci_malloc(sizeof(driver_option_t));
		(*driver_optionsp)->option = sci_strdup(option);
		(*driver_optionsp)->next = NULL;
	}

	(*driver_optionsp)->value = sci_strdup(value);
}
