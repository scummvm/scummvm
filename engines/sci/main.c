/***************************************************************************
 main.c Copyright (C) 1999,2000,01,02 Christoph Reichenbach


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

#include <sciresource.h>
#include <engine.h>
#include <uinput.h>
#include <console.h>
#include <gfx_operations.h>
#include <sci_conf.h>
#include <kdebug.h>
#include <sys/types.h>
#include <game_select.h>
#include "list.h"
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_FORK
#  include <sys/wait.h>
#endif

#if defined(HAVE_SDL) && defined(MACOSX)
#  include <SDL.h>
/* On OS X, SDL must #define main to something else in order to function */
#endif

#ifdef _MSC_VER
#define extern __declspec(dllimport) extern
#include <win32/getopt.h>
#endif

#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#ifdef HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif /* HAVE_READLINE_HISTORY_H */
#endif /* HAVE_READLINE_READLINE_H */

#ifdef HAVE_GETOPT_H
#  ifndef _MSC_VER
#    include <getopt.h>
#  else
#    include <win32\getopt.h>
#  endif
#endif /* HAVE_GETOPT_H */

#ifdef HAVE_GETOPT_LONG
#define EXPLAIN_OPTION(longopt, shortopt, description) "  " longopt "\t" shortopt "\t" description "\n"
#else /* !HAVE_GETOPT_H */
#define EXPLAIN_OPTION(longopt, shortopt, description) "  " shortopt "\t" description "\n"
#endif /* !HAVE_GETOPT_H */


#ifdef _WIN32
#  ifdef _MSC_VER
#    include <direct.h>
#    define PATH_MAX 255
#  endif
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#ifdef _DREAMCAST
#  include <selectgame.h>
#endif

#ifdef _MSC_VER
#  define MSVC_FUNCTYPECAST_KLUDGE (void *)
#else
#  define MSVC_FUNCTYPECAST_KLUDGE
#endif

#define ACTION_PLAY 0
#define ACTION_LIST_SAVEGAMES 1

static int sciv_action = ACTION_PLAY;

/*** HW/OS-dependant features ***/

static void
check_features()
{
#ifdef HAVE_ALPHA_EV6_SUPPORT
	int helper;
	printf("Checking for MVI instruction-set extension: ");

	helper = 0x100;
#ifdef __DECC
	axp_have_mvi = asm("amask %0, %v0", helper);
#else
	__asm__ ("amask %1, %0"
		 : "=r"(axp_have_mvi)
		 : "r"(helper));
#endif

	axp_have_mvi = !axp_have_mvi;

	if (axp_have_mvi)
		printf("found\n");
	else
		printf("not present\n");
#endif
}


static gfx_state_t static_gfx_state; /* see below */
static gfx_options_t static_gfx_options; /* see below */

static state_t *gamestate; /* The main game state */
static gfx_state_t *gfx_state = &static_gfx_state; /* The graphics state */
static gfx_options_t *gfx_options = &static_gfx_options; /* Graphics options */
static char *commandline_config_file = NULL;

int
c_quit(state_t *s)
{
	script_abort_flag = 1; /* Terminate VM */
	_debugstate_valid = 0;
	_debug_seeking = 0;
	_debug_step_running = 0;
	return 0;
}

int
c_die(state_t *s)
{
	exit(0); /* Die */
	return 0; /* ;-P (fixes warning) */
}


char *old_input = NULL;

#ifdef HAVE_READLINE_READLINE_H
const char *
get_readline_input(void)
{
	char *input;

	fflush(NULL);
	input = readline("> ");

	if (!input) { /* ^D */
		c_quit(NULL);
		return "";
	}

	if (strlen(input) == 0) {
		free (input);
	} else {

#ifdef HAVE_READLINE_HISTORY_H
		add_history(input);
#endif /* HAVE_READLINE_HISTORY_H */

		if (old_input) {
			free(old_input);
		}
		old_input = input;
	}

	return old_input? old_input : "";
}
#endif /* HAVE_READLINE_READLINE_H */


int
init_directories(char *work_dir, char *game_id)
{
	char *homedir = sci_get_homedir();

	printf("Initializing directories...\n");
	if (!homedir) { /* We're probably not under UNIX if this happens */

		if (!getcwd(work_dir, PATH_MAX)) {
			fprintf(stderr,"Cannot get the working directory!\n");
			return 1;
		}

		return 0;
	}

  /* So we've got a home directory */

	if (chdir(homedir)) {

#ifdef _WIN32
		if (!getcwd(work_dir, PATH_MAX)) {
			fprintf(stderr,"Cannot get the working directory: %s\n", work_dir);
			return 1;
		}
#else /* Assume UNIX-ish environment */
		fprintf(stderr,"Error: Could not enter home directory %s.\n", homedir);
		perror("Reason");
		return 1; /* If we get here, something really bad is happening */
#endif
	}

	if (strlen(homedir) > MAX_HOMEDIR_SIZE) {
		fprintf(stderr, "Your home directory path is too long. Re-compile FreeSCI with "
			"MAX_HOMEDIR_SIZE set to at least %i and try again.\n", (int)(strlen(homedir)));
		return 1;
	}

	if (chdir(FREESCI_GAMEDIR)) {
		if (scimkdir(FREESCI_GAMEDIR, 0700)) {

			fprintf(stderr, "Warning: Could not enter ~/"FREESCI_GAMEDIR"; save files"
				" will be written to ~/\n");

			getcwd(work_dir, PATH_MAX);
			return 0;

		}
		else /* mkdir() succeeded */
			chdir(FREESCI_GAMEDIR);
	}

	if (chdir(game_id)) {
		if (scimkdir(game_id, 0700)) {

			fprintf(stderr,"Warning: Could not enter ~/"FREESCI_GAMEDIR"/%s; "
				"save files will be written to ~/"FREESCI_GAMEDIR"\n", game_id);

			getcwd(work_dir, PATH_MAX);
			return 0;
		}
		else /* mkdir() succeeded */
			chdir(game_id);
	}

	getcwd(work_dir, PATH_MAX);

	return 0;
}


const char *
get_gets_input(void)
{
	static char input[1024] = "";

	putchar('>');

	fflush(NULL);
	while (!strchr(input, '\n'))
		fgets(input, 1024, stdin);

	if (!input) {
		c_quit(NULL);
		return "";
	}

	if (strlen(input))
		if (input[strlen(input)-1] == '\n');
	input[strlen(input)-1] = 0; /* Remove trailing '\n' */

	if (strlen(input) == 0) {
		return old_input? old_input : "";
	}

	if (old_input)
		free(old_input);

	old_input = (char *) sci_malloc(1024);
	strcpy(old_input, input);
	return input;
}




static void
list_graphics_drivers()
{
	int i = 0;
	while (gfx_get_driver_name(i)) {
		if (i != 0)
			printf(", ");

		printf(gfx_get_driver_name(i));

		i++;
	}
	printf("\n");
}

#ifdef __GNUC__
#warning "Re-enable sound stuff"
#endif
#if 0
static void
list_pcmout_drivers()
{
	int i = 0;
	while (pcmout_drivers[i]) {
		if (i != 0)
			printf(", ");
		printf(pcmout_drivers[i]->name);
		i++;
	}
	printf("\n");
}

static void
list_midiout_drivers()
{
	int i = 0;
	while (midiout_drivers[i]) {
		if (i != 0)
			printf(", ");
		printf(midiout_drivers[i]->name);
		i++;
	}
	printf("\n");
}


static void
list_midi_devices()
{
	int i = 0;
	while (midi_devices[i]) {
		if (i != 0)
			printf(", ");
		printf(midi_devices[i]->name);
		i++;
	}
	printf("\n");
}

static void
list_sound_servers()
{
	int i = 0;
	while (sound_servers[i]) {
		if (i != 0)
			printf(", ");
		printf(sound_servers[i]->name);
		i++;
	}
	printf("\n");
}
#endif


/**********************************************************/
/* Startup and config management                          */
/**********************************************************/

typedef struct {
	int script_debug_flag;
	int scale_x, scale_y, color_depth;
	int mouse;
	int master_sound;	/* on or off */
	int show_rooms;
	sci_version_t version;
	int res_version;
	char *gfx_driver_name;
	char *gamedir;
	char *gamemenu;
	char *midiout_driver_name;
	char *midi_device_name;
	char *sound_server_name;
	char *pcmout_driver_name;
} cl_options_t;

#define ON 1
#define OFF 0
#define DONTCARE -1

static int game_select(cl_options_t cl_options, config_entry_t *confs, int conf_entries, const char* freesci_dir, const char *games_dir);
static int game_select_resource_found();

static char *
parse_arguments(int argc, char **argv, cl_options_t *cl_options, char **savegame_name)
{
	int c;
#ifdef HAVE_GETOPT_LONG
	int optindex;

	struct option options[] = {
		{"run", no_argument, NULL, 0 },
		{"debug", no_argument, NULL, 1 },
		{"gamedir", required_argument, 0, 'd'},
		{"menudir", required_argument, 0, 'G'},
		{"no-sound", required_argument, 0, 'q'},
		{"sci-version", required_argument, 0, 'V'},
		{"graphics", required_argument, 0, 'g'},
		{"midiout", required_argument, 0, 'O'},
		{"pcmout", required_argument, 0, 'P'},
		{"sound-server", required_argument, 0, 'S'},
		{"mididevice", required_argument, 0, 'M'},
		{"version", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'},
		{"scale-x", required_argument, 0, 'x'},
		{"scale-y", required_argument, 0, 'y'},
		{"color-depth", required_argument, 0, 'c'},
		{"disable-mouse", no_argument, 0, 'm'},
		{"list-savegames", no_argument, 0, 'l'},
		{"show-rooms", no_argument, 0, 's'},
		{"config-file", required_argument, 0, 'f'},
		{0,0,0,0}
	};

	options[0].flag = &(cl_options->script_debug_flag);
	options[1].flag = &(cl_options->script_debug_flag);
#endif /* HAVE_GETOPT_H */

	cl_options->scale_x = cl_options->scale_y = cl_options->color_depth = 0;
	cl_options->version = 0;
	cl_options->script_debug_flag = 0;
	cl_options->gfx_driver_name = NULL;
	cl_options->gamedir = NULL;
	cl_options->gamemenu = NULL;
	cl_options->midiout_driver_name = NULL;
	cl_options->pcmout_driver_name = NULL;
	cl_options->midi_device_name = NULL;
	cl_options->sound_server_name = NULL;
	cl_options->mouse = ON;
	cl_options->master_sound = ON;
	cl_options->res_version = SCI_VERSION_AUTODETECT;
	cl_options->show_rooms = 0;

#ifdef HAVE_GETOPT_LONG
	while ((c = getopt_long(argc, argv, "qlvhmsDr:d:G:V:g:x:y:c:M:O:S:P:f:", options, &optindex)) > -1) {
#else /* !HAVE_GETOPT_LONG */
	while ((c = getopt(argc, argv, "qlvhmsDr:d:G:V:g:x:y:c:M:O:S:P:f:")) > -1) {
#endif /* !HAVE_GETOPT_LONG */
		switch (c) {

		case 'r':
			cl_options->res_version = atoi(optarg);
			break;

		case 's':
			cl_options->show_rooms = 1;
			break;

		case 'D':
			cl_options->script_debug_flag = 1;
			break;

		case 'd':
			if (cl_options->gamedir)
				free(cl_options->gamedir);

			cl_options->gamedir = sci_strdup(optarg);
			break;

		case 'G':
			if (cl_options->gamemenu)
				free(cl_options->gamemenu);

			cl_options->gamemenu = sci_strdup(optarg);
			break;

		case 'f':
			commandline_config_file = sci_strdup(optarg);
			break;

		case 'V': {
			int major = *optarg - '0'; /* One version digit */
			int minor = atoi(optarg + 2);
			int patchlevel = atoi(optarg + 6);

			cl_options->version = SCI_VERSION(major, minor, patchlevel);
		}
		break;

		case 'g':
			if (cl_options->gfx_driver_name)
				free(cl_options->gfx_driver_name);
			cl_options->gfx_driver_name = sci_strdup(optarg);
			break;
		case 'O':
			if (cl_options->midiout_driver_name)
				free(cl_options->midiout_driver_name);
			cl_options->midiout_driver_name = sci_strdup(optarg);
			break;
		case 'P':
			if (cl_options->pcmout_driver_name)
				free(cl_options->pcmout_driver_name);
			cl_options->pcmout_driver_name = sci_strdup(optarg);
			break;
		case 'M':
			if (cl_options->midi_device_name)
				free(cl_options->midi_device_name);
			cl_options->midi_device_name = sci_strdup(optarg);
			break;
		case 'S':
			if (cl_options->sound_server_name)
				free(cl_options->sound_server_name);
			cl_options->sound_server_name = sci_strdup(optarg);
			break;
		case '?':
			/* getopt_long already printed an error message. */
			exit(1);

		case 'x':
			cl_options->scale_x = atoi(optarg);
			break;

		case 'y':
			cl_options->scale_y = atoi(optarg);
			break;

		case 'c':
			cl_options->color_depth = (atoi(optarg) +7) >> 3;
			break;

		case 'm':
			cl_options->mouse = OFF;
			break;

		case 'q':
			cl_options->master_sound = OFF;
			break;

		case 0: /* getopt_long already did this for us */
			break;

		case 'v':
			printf("This is FreeSCI, version %s\n", VERSION);

			printf("Supported graphics drivers: ");
			list_graphics_drivers();

#ifdef __GNUC__
#warning "Re-enable sound stuff"
#endif
#if 0
			printf("Supported sound servers: ");
			list_sound_servers();

			printf("Supported midiout drivers: ");
			list_midiout_drivers();

			printf("Supported midi 'devices': ");
			list_midi_devices();

			printf("Supported pcmout drivers: ");
			list_pcmout_drivers();
#endif

			printf("\n");
			exit(0);

		case 'h':
			printf("Usage: freesci [options] [game name] [savegame ID]\n"
			       "Runs a Sierra SCI game.\n"
			       "\n"
			       EXPLAIN_OPTION("--gamedir dir\t", "-ddir", "read game resources from dir")
			       EXPLAIN_OPTION("--menudir dir\t", "-Gdir", "display menu for all games under dir") 
			       EXPLAIN_OPTION("--run\t\t", "-r", "do not start the debugger")
			       EXPLAIN_OPTION("--sci-version ver", "-Vver", "set the version for freesci to emulate")
			       EXPLAIN_OPTION("--version\t", "-v", "display version number and exit")
			       EXPLAIN_OPTION("--debug\t", "-D", "start up in debug mode")
			       EXPLAIN_OPTION("--help\t", "-h", "display this help text and exit")
			       EXPLAIN_OPTION("--graphics gfx", "-ggfx", "use the 'gfx' graphics driver")
			       EXPLAIN_OPTION("--scale-x\t", "-x", "Set horizontal scale factor")
			       EXPLAIN_OPTION("--scale-y\t", "-y", "Set vertical scale factor")
			       EXPLAIN_OPTION("--color-depth\t", "-c", "Specify color depth in bpp")
			       EXPLAIN_OPTION("--disable-mouse", "-m", "Disable support for pointing device")
			       EXPLAIN_OPTION("--midiout drv\t", "-Odrv", "use the 'drv' midiout driver")
			       EXPLAIN_OPTION("--mididevice drv", "-Mdrv", "use the 'drv' midi device (eg mt32 or adlib)")
			       EXPLAIN_OPTION("--pcmout drv\t", "-Pdrv", "use the 'drv' pcmout driver")
			       EXPLAIN_OPTION("--sound-server srv", "-Ssrv", "Specifies the asynchronous sound server to use")
			       EXPLAIN_OPTION("--no-sound\t", "-q", "disable sound output")
			       EXPLAIN_OPTION("--list-savegames", "-l", "Lists all savegame IDs")
			       EXPLAIN_OPTION("--show-rooms\t", "-s","Displays room numbers on the game console")
			       "\n"
			       "The game name, if provided, must be equal to a game name as specified in the\n"
			       "FreeSCI config file.\n"
			       "It is overridden by --gamedir.\n"
			       "\n"
			       );
			exit(0);

		case 'l':
			sciv_action = ACTION_LIST_SAVEGAMES;
			break;

		default:
			exit(1);
		}
	}
#if 0
	} /* Work around EMACS paren matching bug */
#endif

	if (optind+1 >= argc)
		*savegame_name = NULL;
	else
		*savegame_name = argv[optind + 1];

	if (optind == argc)
		return NULL;

	return
		argv[optind];
}

static int
find_config(char *game_name, config_entry_t *conf, int conf_entries,
	    sci_version_t *version)
{
	int i, conf_nr = 0;

	for (i = 1; i < conf_entries; i++)
		if (!strcasecmp(conf[i].name, game_name)) {
			conf_nr = i;
			if (version) 
				*version = conf[i].version;
		}

	return conf_nr;
}

static void
init_console()
{
#ifdef WANT_CONSOLE
	con_gfx_init();
#endif
	con_hook_command(&c_quit, "quit", "", "console: Quits gracefully");
	con_hook_command(&c_die, "die", "", "console: Quits ungracefully");

	con_hook_int(&(gfx_options->buffer_pics_nr), "buffer_pics_nr",
		     "Number of pics to buffer in LRU storage\n");
	con_hook_int(&(gfx_options->pic0_dither_mode), "pic0_dither_mode",
		     "Mode to use for pic0 dithering\n");
	con_hook_int(&(gfx_options->pic0_dither_pattern), "pic0_dither_pattern",
		     "Pattern to use for pic0 dithering\n");
	con_hook_int(&(gfx_options->pic0_unscaled), "pic0_unscaled",
		     "Whether pic0 should be drawn unscaled\n");
	con_hook_int(&(gfx_options->dirty_frames), "dirty_frames",
		     "Dirty frames management\n");
	con_hook_int(&gfx_crossblit_alpha_threshold, "alpha_threshold",
		     "Alpha threshold for crossblitting\n");
	con_hook_int(&sci0_palette, "sci0_palette",
		     "SCI0 palette- 0: EGA, 1:AGI/Amiga, 2:Grayscale\n");
	con_hook_int(&sci01_priority_table_flags, "sci01_priority_table_flags",
		     "SCI01 priority table debugging flags: 1:Disable, 2:Print on change\n");

	con_passthrough = 1; /* enables all sciprintf data to be sent to stdout */

#ifdef HAVE_READLINE_HISTORY_H
	using_history(); /* Activate history for readline */
#endif /* HAVE_READLINE_HISTORY_H */

#ifdef HAVE_READLINE_READLINE_H
	_debug_get_input = get_readline_input; /* Use readline for debugging input */
#else /* !HAVE_READLINE_READLINE_H */
	_debug_get_input = get_gets_input; /* Use gets for debug input */
#endif /* !HAVE_READLINE_READLINE_H */
}


static int
init_gamestate(state_t *gamestate, resource_mgr_t *resmgr, sci_version_t version)
{
	int errc;
	gamestate->resmgr = resmgr;

	if ((errc = script_init_engine(gamestate, version))) { /* Initialize game state */
		int recovered = 0;

		if (errc == SCI_ERROR_INVALID_SCRIPT_VERSION) {
			int tversion = SCI_VERSION_FTU_NEW_SCRIPT_HEADER - ((version < SCI_VERSION_FTU_NEW_SCRIPT_HEADER)? 0 : 1);

			while (!recovered && tversion) {
				printf("Trying version %d.%03x.%03d instead\n", SCI_VERSION_MAJOR(tversion),
				       SCI_VERSION_MINOR(tversion), SCI_VERSION_PATCHLEVEL(tversion));

				errc = script_init_engine(gamestate, tversion);

				if ((recovered = !errc))
					version = tversion;

				if (errc != SCI_ERROR_INVALID_SCRIPT_VERSION)
					break;

				switch (tversion) {

				case SCI_VERSION_FTU_NEW_SCRIPT_HEADER - 1:
					if (version >= SCI_VERSION_FTU_NEW_SCRIPT_HEADER)
						tversion = 0;
					else
						tversion = SCI_VERSION_FTU_NEW_SCRIPT_HEADER;
					break;

				case SCI_VERSION_FTU_NEW_SCRIPT_HEADER:
					tversion = 0;
					break;
				}
			}
			if (recovered)
				printf("Success.\n");
		}

		if (!recovered) {
			fprintf(stderr,"Script initialization failed. Aborting...\n");
			return 1;
		}
	}
	return 0;
}

static int
init_gfx(config_entry_t *conf, cl_options_t *cl_options, gfx_driver_t *driver, resource_mgr_t *resmgr)
{
	int scale_x = 0, scale_y = 0, color_depth = 0;

	if (conf) {
		if (conf->scale)
			scale_x = scale_y = conf->scale;

		if (conf->x_scale)
			scale_x = conf->x_scale;

		if (conf->y_scale)
			scale_y = conf->y_scale;

		if (conf->color_depth)
			color_depth = conf->color_depth >> 3; /* In there it's bpp */
	}

	gfx_state->driver = driver;
	gamestate->gfx_state = gfx_state;
	gfx_state->version = resmgr->sci_version;

	if (cl_options->scale_x > 0) {
		scale_x = cl_options->scale_x;

		if (!scale_y)
			scale_y = cl_options->scale_x;
	}

	if (cl_options->scale_y > 0) {
		scale_y = cl_options->scale_y;

		if (!scale_x)
			scale_x = cl_options->scale_y;
	}

	if (cl_options->color_depth > 0)
		color_depth = cl_options->color_depth;

	if (cl_options->color_depth > 0 && scale_x == 0)
		scale_x = scale_y = 2; /* Some default setting */

	if (scale_x > 0) {

		if (color_depth > 0) {
			if (gfxop_init(gfx_state, scale_x,
				       scale_y, (gfx_color_mode_t) color_depth,
				       gfx_options, resmgr)) {
				fprintf(stderr,"Graphics initialization failed. Aborting...\n");
				return 1;
			}
		} else {
			color_depth = 4;
			while (gfxop_init(gfx_state, scale_x,
					  scale_y, (gfx_color_mode_t) color_depth,
					  gfx_options, resmgr) && --color_depth);

			if (!color_depth) {
				fprintf(stderr,"Could not find a matching color depth. Aborting...\n");
				return 1;
			}
		}

	} else if (gfxop_init_default(gfx_state, gfx_options, resmgr)) {
		fprintf(stderr,"Graphics initialization failed. Aborting...\n");
		return 1;
	}

	return 0;
}


typedef void *old_lookup_funct_t(char *name);

typedef void *lookup_funct_t(const char *path, const char *name);


static void *
lookup_driver(lookup_funct_t lookup_func, void explain_func(void),
	      const char *driver_class, const char *driver_name, const char *path)
{
	void *retval = lookup_func(path, driver_name);

	if (!retval) {
		if (!driver_name)
			sciprintf("The default %s is not available; please choose"
				  " one explicitly.\n", driver_class);
		else
			sciprintf("The %s you requested, '%s', is not available.\n"
/*			  "Please choose one among the following: " */
			  ,
			  driver_class, driver_name);
/*		explain_func();  */
		exit(1);
	}

	return retval;
}


/*static void *
old_lookup_driver(old_lookup_funct_t lookup_func, void explain_func(void),
	      char *driver_class, char *driver_name)
{
	void *retval = lookup_func(driver_name);

	if (!retval) {
		sciprintf("The %s you requested, '%s', is not available.\n"
			  "Please choose one among the following: ",
			  driver_class, driver_name);
		explain_func();
		exit(1);
	}

	return retval;
}*/

#define NAMEBUF_LEN 30
static void
list_savegames(state_t *s)
{
	sci_dir_t dir;
	char *filename		= NULL;

	sci_init_dir(&dir);

	filename = sci_find_first(&dir, "*");

	sciprintf("\nSavegame listing:\n"
		  "-----------------\n");
	while (filename) {
		char namebuf[NAMEBUF_LEN + 1];
		if (test_savegame(s, filename, namebuf, NAMEBUF_LEN)) {
			if (namebuf[0])
				sciprintf("%s:\t\"%s\"\n", filename, namebuf);
			else
				sciprintf("%s\n", filename);
		}
		filename = sci_find_next(&dir);
	}
	sciprintf("-----------------\n");
}

void
get_file_directory(char* directory, const char* file)
{
	char* end;

	strcpy(directory, file);

	end = directory + strlen(directory) - 1;
	while ((end >= directory) && (end != 0))
	{
		if (*end == G_DIR_SEPARATOR)
		{
			*end = 0;
			break;
		}
		else 
		{
			end--;
		}
	}
}

static void
detect_versions(sci_version_t *version, int *res_version, cl_options_t *options, config_entry_t *conf)
{
	sci_version_t exe_version;
	sci_version_t hash_version;
	int hash_res_version;
	guint32 code;
	int got_exe_version;
	const char *game_name;

	sciprintf("Detecting interpreter and resource versions...\n");

	got_exe_version = !version_detect_from_executable(&exe_version);

	if (got_exe_version) {
		sciprintf("Interpreter version: %d.%03d.%03d (by executable scan)\n",
				  SCI_VERSION_MAJOR(exe_version),
				  SCI_VERSION_MINOR(exe_version),
				  SCI_VERSION_PATCHLEVEL(exe_version));

		if (SCI_VERSION_MAJOR(exe_version) >= 1) {
			sciprintf("FIXME: Implement version mapping (results of executable scan ignored)\n");
			got_exe_version = 0;
		}

	}

	game_name = version_guess_from_hashcode(&hash_version, &hash_res_version, &code);

	if (game_name) {
		sciprintf("Interpreter version: %d.%03d.%03d (by hash code %08X)\n",
				  SCI_VERSION_MAJOR(hash_version),
				  SCI_VERSION_MINOR(hash_version),
				  SCI_VERSION_PATCHLEVEL(hash_version), code);
		if (got_exe_version && exe_version != hash_version)
				sciprintf("UNEXPECTED INCONSISTENCY: Hash code %08X indicates interpreter version\n"
					  "  %d.%03d.%03d, but analysis of the executable yields %d.%03d.%03d (for game\n"
					  "  '%s'). Please report this!\n",
					  code,
					  SCI_VERSION_MAJOR(hash_version),
					  SCI_VERSION_MINOR(hash_version),
					  SCI_VERSION_PATCHLEVEL(hash_version),
					  SCI_VERSION_MAJOR(exe_version),
					  SCI_VERSION_MINOR(exe_version),
					  SCI_VERSION_PATCHLEVEL(exe_version), game_name);

		if (hash_res_version != SCI_VERSION_AUTODETECT)
			sciprintf("Resource version: %d (by hash code)\n", hash_res_version);

		sciprintf("Game identified as '%s'\n", game_name);
	} else {
		sciprintf("Could not identify game by hash code: %08X\n", code);

		if (got_exe_version)
			  sciprintf("Please report the preceding two lines and the name of the game you were trying\n"
			  	    "to run to the FreeSCI development team to help other users!\n",
			  code);
	}

	if (options->version)
		*version = options->version;
	else if (conf && conf->version)
		*version = conf->version;
	else if (game_name)
		*version = hash_version;
	else if (got_exe_version)
		*version = exe_version;
	else
		*version = 0;

	if (options->res_version != SCI_VERSION_AUTODETECT)
		*res_version = options->res_version;
	else if (conf && conf->res_version != SCI_VERSION_AUTODETECT)
		*res_version = conf->res_version;
	else if (game_name)
		*res_version = hash_res_version;
	else
		*res_version = SCI_VERSION_AUTODETECT;

	if (*version)
		sciprintf("Using interpreter version %d.%03d.%03d\n",
				  SCI_VERSION_MAJOR(*version),
				  SCI_VERSION_MINOR(*version),
				  SCI_VERSION_PATCHLEVEL(*version));

	if (*res_version != SCI_VERSION_AUTODETECT)
		sciprintf("Using resource version %d\n", *res_version);
}

int
main(int argc, char** argv)
{
	config_entry_t *active_conf = NULL;	/* Active configuration used */
	config_entry_t *confs = {0};	/* Configuration read from config file (if it exists) */
	cl_options_t cl_options;		/* Command line options */
	int conf_entries	= -1;		/* Number of config entries */
	int conf_nr			= -1;		/* Element of conf to use */
/*	FILE *console_logfile			= NULL; */
	char freesci_dir[PATH_MAX+1] = "";
	char startdir[PATH_MAX+1] = "";
	char resource_dir[PATH_MAX+1] = "";
	char work_dir[PATH_MAX+1] = "";
	char *cwd;
	char *gfx_driver_name = NULL;
/*	char *midiout_driver_name = NULL;
	char *midi_device_name = NULL;
	char *pcm_driver_name = NULL; */
	char *game_name	= NULL;
	char *savegame_name = NULL;
	sci_version_t version;
	int res_version;
	gfx_driver_t *gfx_driver = NULL;
#if 0
	sound_server_t *sound_server = NULL;
#endif
	const char *module_path			= SCI_DEFAULT_MODULE_PATH;
	resource_mgr_t *resmgr;
#ifdef _DREAMCAST
	/* Fake command line arguments. */
	char *args[] = {"/cd/freesci.bin", "-f/ram/config", NULL};
	argv = args;
	argc = 2;
	chdir("/ram");
#endif

	init_console(); /* So we can get any output */

	game_name = parse_arguments(argc, argv, &cl_options, &savegame_name);

	/* remember where freesci executable is located */
	get_file_directory(freesci_dir, argv[0]);

	getcwd(startdir, PATH_MAX);
	script_debug_flag = cl_options.script_debug_flag;

	printf("FreeSCI %s Copyright (C) 1999-2007\n", VERSION);
	printf(" Dmitry Jemerov, Christopher T. Lansdown, Sergey Lapin, Rickard Lind,\n"
		   " Carl Muckenhoupt, Christoph Reichenbach, Magnus Reftel, Lars Skovlund,\n"
		   " Rink Springer, Petr Vyhnak, Solomon Peachy, Matt Hargett, Alex Angas\n"
                   " Walter van Niftrik, Rainer Canavan, Ruediger Hanke, Hugues Valois\n"
	       "This program is free software. You can copy and/or modify it freely\n"
	       "according to the terms of the GNU general public license, v2.0\n"
	       "or any later version, at your option.\n"
	       "It comes with ABSOLUTELY NO WARRANTY.\n");

#ifdef _DREAMCAST
	choose_game();
	game_name = "game";
#endif

	conf_entries = config_init(&confs, commandline_config_file);

	/* working directory was changed by config_init so restore it */
	chdir(startdir);

	if (game_name) {

		conf_nr = find_config(game_name, confs, conf_entries, &version);
		active_conf = confs + conf_nr;

		if (!cl_options.gamedir)
			if (chdir(active_conf->resource_dir)) {
				if (conf_nr)
					fprintf(stderr,"Error entering '%s' to load resource data\n", active_conf->resource_dir);
				else
					fprintf(stderr,"Game '%s' isn't registered in your config file.\n", game_name);
				exit(1);
			}
	}

	if (cl_options.gamedir)
	{
		if (chdir(cl_options.gamedir)) {
			printf ("Error changing to game directory '%s'\n", cl_options.gamedir);
			exit(1);
		}
		free(cl_options.gamedir);
	}

	/* by now, if the user specified a game name or a game directory, the working dir has been changed */
	/* so if no resource are found in the working dir, invoke the game selection screen */
	if (!game_name && !game_select_resource_found())
	{
		char *menu_dir;

		if (cl_options.gamemenu)
			menu_dir = cl_options.gamemenu;
		else
			menu_dir = confs->menu_dir;

		chdir(startdir);
		conf_nr = game_select(cl_options, confs, conf_entries, freesci_dir, menu_dir);
		if (conf_nr < 0)
			return 1;
		if (conf_nr > 0)
			/* A game from the config file was chosen */
			active_conf = confs + conf_nr;
	}

	detect_versions(&version, &res_version, &cl_options, active_conf);

	getcwd(resource_dir, PATH_MAX); /* Store resource directory */

	sciprintf("Loading resources...\n");

	resmgr = scir_new_resource_manager(resource_dir, res_version, 1, 256*1024);
	
	if (!resmgr) {
		printf("No resources found in '%s'.\nAborting...\n",
		       resource_dir);
		exit(1);
	}

	script_adjust_opcode_formats(resmgr->sci_version);

	check_features();

	chdir(startdir);

#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	printf("Mapping instruments to General Midi\n");

	map_MIDI_instruments(resmgr);
#endif

	sciprintf("FreeSCI, version "VERSION"\n");

	gamestate = (state_t *) sci_calloc(sizeof(state_t), 1);

	if (init_gamestate(gamestate, resmgr, version))
		return 1;

	gamestate->gfx_state = NULL;
	if (game_init(gamestate)) { /* Initialize */
		fprintf(stderr,"Game initialization failed: Aborting...\n");
		return 1;
	}

	if (init_directories(work_dir, (char *) gamestate->game_name)) {
		fprintf(stderr,"Error resolving the working directory\n");
		exit(1);
	}

	/* Set the CWD as the savegame dir */
	cwd = sci_getcwd();
	script_set_gamestate_save_dir(gamestate, cwd);
	sci_free(cwd);

	if (sciv_action == ACTION_LIST_SAVEGAMES) {
		list_savegames(gamestate);
		exit(0);
	}
	gamestate->resource_dir = resource_dir;
	gamestate->work_dir = work_dir;
	gamestate->port_serial = 0;

	if (!game_name)
		game_name = (char *) gamestate->game_name;

	/* If no game-specific configuration has been read, then read the non-specific config from file */
	if (!active_conf) {
		conf_nr = find_config(game_name, confs, conf_entries, &version);
		active_conf = confs + conf_nr;
	}

	/* gcc doesn't warn about (void *)s being typecast. If your compiler doesn't like these
	** implicit casts, don't hesitate to typecast appropriately.  */
	if (cl_options.gfx_driver_name) {
		gfx_driver_name = sci_strdup(cl_options.gfx_driver_name);
		free(cl_options.gfx_driver_name);
	} /* else it's still NULL */

#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	if (cl_options.pcmout_driver_name)
		pcmout_driver = old_lookup_driver((old_lookup_funct_t *)pcmout_find_driver,
						   MSVC_FUNCTYPECAST_KLUDGE list_pcmout_drivers,
						   "pcmout driver", cl_options.pcmout_driver_name);

	if (cl_options.midiout_driver_name)
	{
		midiout_driver = old_lookup_driver((old_lookup_funct_t *)midiout_find_driver,
						   MSVC_FUNCTYPECAST_KLUDGE list_midiout_drivers,
						   "midiout driver", cl_options.midiout_driver_name);
		free(cl_options.midiout_driver_name);
	}

	if (cl_options.midi_device_name)
	{
		midi_device = old_lookup_driver((old_lookup_funct_t *)midi_find_device,
						MSVC_FUNCTYPECAST_KLUDGE list_midi_devices,
						"MIDI device", cl_options.midi_device_name);
		free(cl_options.midi_device_name);
	}

	if (cl_options.sound_server_name)
	{
		sound_server = old_lookup_driver((old_lookup_funct_t *)sound_server_find_driver,
						 MSVC_FUNCTYPECAST_KLUDGE list_sound_servers,
						 "sound server", cl_options.sound_server_name);
		free(cl_options.sound_server_name);
	}
#endif

	if (confs) {
		memcpy(gfx_options, &(active_conf->gfx_options), sizeof(gfx_options_t)); /* memcpy so that console works */
		if (!gfx_driver_name)
			gfx_driver_name = active_conf->gfx_driver_name;
#ifdef __GNUC__
#warning "sound"
#endif
#if 0
		if (!sound_server)
			sound_server = active_conf->sound_server;

		/* make sure we have sound drivers */
		if (!midiout_driver)
			midiout_driver = active_conf->midiout_driver;
		if (!midi_device)
			midi_device = active_conf->midi_device;
		if (!pcmout_driver)
		  pcmout_driver = active_conf->pcmout_driver;
#endif
	}

	if (confs) {
		module_path = active_conf->module_path;

		if (!gfx_driver_name)
			gfx_driver_name = active_conf->gfx_driver_name;
	}

	gfx_driver = (gfx_driver_t *)
		lookup_driver((lookup_funct_t *)gfx_find_driver,
				MSVC_FUNCTYPECAST_KLUDGE list_graphics_drivers,
				"graphics driver", gfx_driver_name, module_path);

	if (!gfx_driver) {
		if (gfx_driver_name)
			fprintf(stderr,"Failed to find graphics driver \"%s\"\n"
				"Please run 'freesci -v' to get a list of all "
				"available drivers.\n", gfx_driver_name);
		else
			fprintf(stderr,"No default gfx driver available.\n");

		return 1;
	}

	if (!gamestate->version_lock_flag)
		if (active_conf->version)
			gamestate->version = active_conf->version;

	if (strlen (active_conf->debug_mode))
		set_debug_mode (gamestate, 1, active_conf->debug_mode);


#if 0
	{
		int j;
		for (j =0; j < conf_entries; j++) {
			int i;
			config_entry_t *c = conf + j;
			fprintf(stderr, "[%s]\n", c->name);
			for (i = 0; i < 2; i++) {
				subsystem_options_t *subsys = c->driver_options[i];
				fprintf(stderr, "  <%s>\n", i? "midiout" : "gfx");

				while (subsys) {
					driver_option_t *opt;
					fprintf(stderr, "    {%p,%s}\n", subsys->name,subsys->name);
					opt = subsys->options;
					while (opt) {
						fprintf(stderr, "\t%p'%s' = %p'%s'\n", opt->option, opt->option, opt->value,opt->value);
						opt = opt->next;
					}
					subsys = subsys->next;
				}
			}
		}
	}
#endif /* 0 */

	/* Now configure the graphics driver with the specified options */
	{
		driver_option_t *option = get_driver_options(active_conf, FREESCI_DRIVER_SUBSYSTEM_GFX, gfx_driver->name);
		while (option) {
			if ((gfx_driver->set_parameter)(gfx_driver, option->option, option->value)) {
				fprintf(stderr, "Fatal error occured in graphics driver while processing \"%s = %s\"\n",
					option->option, option->value);
				exit(1);
			}

			option = option->next;
		}
	}

#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	/* Configure the pcmout driver */
	{
		pcmout_sample_rate = active_conf->pcmout_rate;
		pcmout_stereo = active_conf->pcmout_stereo;
	}

	/* Configure the midiout driver */
	{
		driver_option_t *option = get_driver_options(active_conf, FREESCI_DRIVER_SUBSYSTEM_MIDIOUT, midiout_driver->name);
		while (option) {
			if ((midiout_driver->set_parameter)(midiout_driver, option->option, option->value)) {
				fprintf(stderr, "Fatal error occured in midiout driver while processing \"%s = %s\"\n",
					option->option, option->value);
				exit(1);
			}

			option = option->next;
		}
	}
#endif

	/* Allows drivers to access files in the resource directory. */
	if (chdir(gamestate->resource_dir)) {
		fprintf(stderr,"Error entering resource directory '%s'\n",
			gamestate->resource_dir);
		exit(1);
	}

	if (init_gfx(active_conf, &cl_options, gfx_driver, resmgr))
		return 1;


	if (game_init_graphics(gamestate)) { /* Init interpreter graphics */
		fprintf(stderr,"Game initialization failed: Error in GFX subsystem. Aborting...\n");
		return 1;
	}

	if (game_init_sound(gamestate, (cl_options.master_sound == OFF)? SFX_STATE_FLAG_NOSOUND : 0)) {
		fprintf(stderr,"Game initialization failed: Error in sound subsystem. Aborting...\n");
		return 1;
	}

	if (chdir(gamestate->work_dir)) {
		fprintf(stderr,"Error entering working directory '%s'\n",
			gamestate->work_dir);
		exit(1);
	}

#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	if (!sound_server)
		sound_server = sound_server_find_driver(NULL);
#endif

	if (cl_options.show_rooms)
	  set_debug_mode(gamestate, 1, "r");

#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	gamestate->sound_server = sound_server;

	if (gamestate->sound_server) {
		int poly;
		if (gamestate->sound_server->init(
			gamestate,
			((active_conf->reverse_stereo) ? SOUNDSERVER_INIT_FLAG_REVERSE_STEREO : 0)))
		{

			fprintf(stderr,"Sound server initialization failed- aborting.\n");
			return 1;
		}
		sci_sched_yield();

		if (!soundserver_dead) {
		poly = gamestate->sound_server->command(gamestate, get_msg_value("SOUND_COMMAND_TEST"), 0, 0);

		printf("Sound server reports polyphony %d\n", poly);

		gamestate->sound_server->command(gamestate, get_msg_value("SOUND_COMMAND_SET_VOLUME"), 0, 0xc);

		}

		gamestate->sound_server->get_event(gamestate); /* Get init message */
		/* FIXME: memory allocated that is not freed */
	}
#endif

	if (active_conf && active_conf->console_log)
		open_console_file (active_conf->console_log);
	gamestate->animation_delay = active_conf->animation_delay;
	gamestate->animation_granularity = active_conf->animation_granularity;
	gfx_crossblit_alpha_threshold = active_conf->alpha_threshold;

	printf("Emulating SCI version %d.%03d.%03d\n",
	       SCI_VERSION_MAJOR(gamestate->version),
	       SCI_VERSION_MINOR(gamestate->version),
	       SCI_VERSION_PATCHLEVEL(gamestate->version));

	printf("Graphics: Using the %s driver %s\n",
	       gfx_driver->name, gfx_driver->version);
#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	printf("MIDI-out: Using the %s driver %s\n",
	       midiout_driver->name, midiout_driver->version);
	printf("MIDI-device: Using the %s driver %s\n",
	       midi_device->name, midi_device->version);
	printf("PCM-out: Using the %s driver %s\n",
	       pcmout_driver->name, pcmout_driver->version);

	if (sound_server)
		printf("Sound server: Using the %s sound server %s\n",
		       sound_server->name, sound_server->version);
	else
		printf("Sound server: Disabled.\n");
#endif

	gamestate->have_mouse_flag = (cl_options.mouse == DONTCARE)?
		active_conf->mouse : cl_options.mouse;

	if (savegame_name)
		game_restore(&gamestate, savegame_name);
	else
		game_run(&gamestate); /* Run the game */
#ifdef __GNUC__
#warning "sound"
#endif
#if 0
	if (gamestate->sound_server)
		gamestate->sound_server->exit(gamestate); /* Shutdown sound daemon first */
#endif

	game_exit(gamestate);
	script_free_engine(gamestate); /* Uninitialize game state */
	script_free_breakpoints(gamestate);

	scir_free_resource_manager(resmgr);

	if (conf_entries >= 0)
		config_free(&confs, conf_entries);

	close_console_file();

	chdir (startdir); /* ? */

#ifdef HAVE_FORK
	printf("Waiting for sound server to die...");
	wait(NULL); /* Wait for sound server process to die, if neccessary */
	printf(" OK.\n");
#endif

	gfxop_exit(gfx_state);

	sci_free(gamestate);

	if (commandline_config_file)
		sci_free(commandline_config_file);

#ifdef WITH_DMALLOC
	fprintf(stderr,"--- Everything but the two console buffers should have been freed now ---\n");
	dmalloc_log_unfreed();
/*	BREAKPOINT(); */
	((*(int *)NULL) = 42);
#endif
	return 0;
}


static int
game_select_resource_found()
{
	int fd;

	fd = sci_open("resource.map", O_RDONLY | O_BINARY);
	if (IS_VALID_FD(fd))
	{
		close(fd);
		return 1;
	}

	return 0;
}

static int
game_select_init_gfx(config_entry_t *conf, cl_options_t *cl_options, gfx_driver_t *driver, sci_version_t sci_version)
{
	int scale_x = 0, scale_y = 0;
	int color_depth = 0;

	if (conf) {
		if (conf->scale)
			scale_x = scale_y = conf->scale;

		if (conf->x_scale)
			scale_x = conf->x_scale;

		if (conf->y_scale)
			scale_y = conf->y_scale;

		if (conf->color_depth)
			color_depth = conf->color_depth >> 3; /* In there it's bpp */
	}

	gfx_state->driver = driver;
	gfx_state->version = sci_version;

	if (cl_options->scale_x > 0) {
		scale_x = cl_options->scale_x;

		if (!scale_y)
			scale_y = cl_options->scale_x;
	}

	if (cl_options->scale_y > 0) {
		scale_y = cl_options->scale_y;

		if (!scale_x)
			scale_x = cl_options->scale_y;
	}

	if (cl_options->color_depth > 0)
		color_depth = cl_options->color_depth;

	if (color_depth > 0 && scale_x == 0)
		scale_x = scale_y = 2; /* Some default setting */

/*	fprintf(stderr, "cd-conf=%d, cd-cl=%d, cd=%d\n",
		conf->color_depth, cl_options->color_depth, color_depth); */

	fprintf(stderr, "Checking byte depth %d\n", color_depth);

	if (scale_x > 0) {

		if (color_depth > 0) {
			if (game_select_gfxop_init(gfx_state, scale_x,
				       scale_y, (gfx_color_mode_t) color_depth,
				       gfx_options, 0)) {
				fprintf(stderr,"Graphics initialization failed. Aborting...\n");
				return 1;
			}
		} else {
			color_depth = 4;
			while (game_select_gfxop_init(gfx_state, scale_x,
					  scale_y, (gfx_color_mode_t) color_depth,
					  gfx_options, 0) && --color_depth);

			if (!color_depth) {
				fprintf(stderr,"Could not find a matching color depth. Aborting...\n");
				return 1;
			}
		}

	} else if (game_select_gfxop_init_default(gfx_state, gfx_options, 0)) {
		fprintf(stderr,"Graphics initialization failed. Aborting...\n");
		return 1;
	}

	return 0;
}

static int compare_games(const void* arg1, const void* arg2)
{
	game_t* game1 = (game_t *)arg1;
	game_t* game2 = (game_t *)arg2;

	return strcmp(game1->name, game2->name);
}

static gfx_bitmap_font_t* load_font(const char* font_dir, const char* filename)
{
	gfx_bitmap_font_t* font = NULL;
	int fh;
	int fsize;
	byte* buffer;
	char work_dir[256];

	getcwd(work_dir, 256);

	/* change to the font directory */
	chdir(font_dir);

	fh = sci_open(filename, O_RDONLY|O_BINARY);

	if (!IS_VALID_FD(fh))
		return NULL;

	fsize = sci_fd_size(fh);

	buffer = (byte *) sci_malloc(fsize);

	/* skip the header information, is present by default when using sciunpack */
	read(fh, buffer, 2);

	/* read the font data */
	read(fh, buffer, fsize);

	font = gfxr_read_font(0, buffer, fsize);

	sci_free(buffer);

	close(fh);

	chdir(work_dir);

	return font;
}

#define FONT_DEFAULT		"default.fnt"
#define FONT_SMALL			"small.fnt"

static void
add_games(const char *dir_name, games_list_head_t *head, int *games, gfx_driver_t *driver,
	  gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small)
{
	sci_dir_t dir;
	char *filename;
	int fd;

	if (chdir(dir_name))
		return;

	fd = sci_open("resource.map", O_RDONLY);

	if (IS_VALID_FD(fd)) {
		games_list_t *list;
		sci_version_t result;
		int res_version;
		const char *game_name;
		guint32 code;

		close(fd);

		list = (games_list_t*)sci_malloc(sizeof(games_list_t));
		getcwd(list->game.dir, PATH_MAX);
		game_name = version_guess_from_hashcode(&result, &res_version, &code);

		if (game_name)
			list->game.name = sci_strdup(game_name);
		else
			list->game.name = sci_strdup(dir_name);

		list->game.conf_nr = 0;
		LIST_INSERT_HEAD(head, list, entries);
		(*games)++;

		game_select_scan_info(driver, font_default, font_small, list->game.name, *games);
	}

	sci_init_dir(&dir);

	filename = sci_find_first(&dir, "*");

	while (filename) {
		add_games(filename, head, games, driver, font_default, font_small);
		filename = sci_find_next(&dir);
	}

	sci_finish_find(&dir);
	chdir("..");

	return;
}

static int
find_games(const char* dir, game_t **games, gfx_driver_t *driver, gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small)
{
	games_list_head_t head;
	int games_nr = 0;
	games_list_t *list;
	int i;

	game_select_scan_info(driver, font_default, font_small, NULL, 0);

	LIST_INIT(&head);
	add_games(dir, &head, &games_nr, driver, font_default, font_small);

	if (!games_nr)
		return 0;

	*games = (game_t*)sci_malloc(sizeof(game_t) * games_nr);

	i = 0;
	while ((list = LIST_FIRST(&head))) {
		(*games)[i++] = list->game;
		LIST_REMOVE(list, entries);
		sci_free(list);
	}

	return games_nr;
}

static int game_select(cl_options_t cl_options, config_entry_t *confs, int conf_entries, const char* freesci_dir, const char *games_dir) {
	char start_dir[PATH_MAX+1] = "";
	char *gfx_driver_name			= NULL;
	gfx_driver_t *gfx_driver		= NULL;
	const char *module_path			= SCI_DEFAULT_MODULE_PATH;
	int selected_game = -1;
	gfx_bitmap_font_t* font_default;
	gfx_bitmap_font_t* font_small;
	int font_default_allocated = 0;
	int font_small_allocated = 0;
	game_t *games = NULL;
	int games_nr;
	int i;

	getcwd(start_dir, PATH_MAX);
	script_debug_flag = cl_options.script_debug_flag;

	/* gcc doesn't warn about (void *)s being typecast. If your compiler doesn't like these
	** implicit casts, don't hesitate to typecast appropriately.  */
	if (cl_options.gfx_driver_name) {
		gfx_driver_name = sci_strdup(cl_options.gfx_driver_name);
		/* free(cl_options.gfx_driver_name); */
	} /* else it's still NULL */

	if (confs) {
		memcpy(gfx_options, &(confs->gfx_options), sizeof(gfx_options_t)); /* memcpy so that console works */
		if (!gfx_driver_name)
			gfx_driver_name = confs->gfx_driver_name;
	}

	if (confs) {
		module_path = confs->module_path;

		if (!gfx_driver_name)
			gfx_driver_name = confs->gfx_driver_name;
	}

	gfx_driver = (gfx_driver_t *)
		lookup_driver((lookup_funct_t *)gfx_find_driver,
				MSVC_FUNCTYPECAST_KLUDGE list_graphics_drivers,
				"graphics driver", gfx_driver_name, module_path);

	if (!gfx_driver) {
		if (gfx_driver_name)
			fprintf(stderr,"Failed to find graphics driver \"%s\"\n"
				"Please run 'freesci -v' to get a list of all "
				"available drivers.\n", gfx_driver_name);
		else
			fprintf(stderr,"No default gfx driver available.\n");

		return -2;
	}


	/* Now configure the graphics driver with the specified options */
	{
		driver_option_t *option = get_driver_options(confs, FREESCI_DRIVER_SUBSYSTEM_GFX, gfx_driver->name);
		while (option) {
			if ((gfx_driver->set_parameter)(gfx_driver, option->option, option->value)) {
				fprintf(stderr, "Fatal error occured in graphics driver while processing \"%s = %s\"\n",
					option->option, option->value);
				exit(1);
			}

			option = option->next;
		}
	}

	if (game_select_init_gfx(confs, &cl_options, gfx_driver, 0))
		return -2;

	/* load user supplied font from disk, if not found then use built-in font */
	font_default = load_font(freesci_dir, FONT_DEFAULT);
	if (!font_default)
		font_default = gfxr_get_font(NULL, GFX_FONT_BUILTIN_6x10, 0);
	else
		font_default_allocated = 1;

	/* load user supplied font from disk, if not found then use built-in font */
	font_small = load_font(freesci_dir, FONT_SMALL);
	if (!font_small)
		font_small = gfxr_get_font(NULL, GFX_FONT_BUILTIN_5x8, 0);
	else
		font_small_allocated = 1;

	chdir(start_dir);

	if (games_dir)
		games_nr = find_games(games_dir, &games, gfx_driver, font_default, font_small);
	else
		games_nr = 0;

	games = (game_t*)sci_realloc(games, sizeof(game_t) * (games_nr + conf_entries));

	for (i = 0; i < conf_entries; i++) {
		if (confs[i].name) {
			char *c;

			games[games_nr].name = sci_strdup(confs[i].name);
			games[games_nr].conf_nr = i;
			/* Replace all '_'with ' ' */

			c = games[games_nr].name;
			while (*c != 0)
			{
				if (*c == '_')
					*c = ' ';
				c++;
			}
			strncpy(games[games_nr].dir, confs[i].resource_dir, PATH_MAX - 1);
			games[games_nr++].dir[PATH_MAX - 1] = 0;
		}
	}

	/* Sort game list */
	qsort(games, games_nr, sizeof(game_t), compare_games);

	/* Index of game selected is returned - -1 means no selection (quit) */
	selected_game = game_select_display(gfx_driver, games, games_nr, font_default, font_small);
	if (selected_game >= 0)
	{
		chdir(games[selected_game].dir);
	}

	if (font_default_allocated == 1)
		gfxr_free_font(font_default);

	if (font_small_allocated == 1)
		gfxr_free_font(font_small);

	if (selected_game >= 0)
		selected_game = games[selected_game].conf_nr;

	for (i = 0; i < games_nr; i++)
		sci_free(games[i].name);
	sci_free(games);

	gfx_driver->exit(gfx_driver);

	return selected_game;
}
