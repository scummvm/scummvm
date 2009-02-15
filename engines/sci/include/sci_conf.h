/***************************************************************************
 sci_conf.h Copyright (C) 1999,2000,01 Christoph Reichenbach


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
/* Configuration and setup stuff for FreeSCI */

#ifndef _SCI_CONFIG_H_
#define _SCI_CONFIG_H_

#include <versions.h>
#include <gfx_options.h>

#define FREESCI_DRIVER_SUBSYSTEMS_NR 1

#define FREESCI_DRIVER_SUBSYSTEM_GFX 0
#define FREESCI_DRIVER_SUBSYSTEM_PCM 1
#define FREESCI_DRIVER_SUBSYSTEM_MIDIOUT 2

#ifdef _WIN32
#  define SCI_DEFAULT_MODULE_PATH "."
#else
#  define SCI_DEFAULT_MODULE_PATH "/usr/local/lib/freesci/:/usr/lib/freesci/"
#endif

typedef struct _driver_option {
	char *option;
	char *value;
	struct _driver_option *next;
} driver_option_t;

typedef struct _subsystem_options {
	char *name; /* Driver name */
	driver_option_t *options;
	struct _subsystem_options *next; /* next driver */
} subsystem_options_t;

typedef struct {
	/* IMPORTANT: these values correspond directly to what's specified in
	** config.l. Where an option is of type OPTION_TYPE_NVP or
	** OPTION_TYPE_INVERSE_NVP, it is cast as an _integer_. Therefore it
	** should not be any other type except for int here.
	*/

	char *name; /* Game identifier */
	sci_version_t version; /* The version to emulate */

	gfx_options_t gfx_options;

	subsystem_options_t *driver_options[FREESCI_DRIVER_SUBSYSTEMS_NR];

	int x_scale, y_scale, scale, color_depth; /* GFX subsystem initialization values */

	int animation_delay; /* Number of microseconds to wait between each pic transition animation cycle */
	int animation_granularity; /* Granularity for pic transition animations */
	int alpha_threshold; /* Crossblitting alpha threshold */
	int unknown_count; /* The number of "unknown" kernel functions */
	char *resource_dir; /* Resource directory */
	char *gfx_driver_name; /* The graphics driver to use */
	char *console_log; /* The file to which console output should be echoed */
	char *menu_dir; /* Directory where the game menu searches for games */
	char debug_mode [80]; /* Characters specifying areas for which debug output should be enabled */
	int mouse; /* Whether the mouse should be active */
	int reverse_stereo;
	int res_version;
	
#ifdef __GNUC__
#  warning "Re-enable sound stuff"
#endif
#if 0
	midiout_driver_t *midiout_driver; /* the midiout method to use */
	midi_device_t *midi_device; /* the midi device to use */
	
	pcmout_driver_t *pcmout_driver; /* the pcm driver to use */
	sound_server_t *sound_server; /* The sound server */
#endif
        guint16 pcmout_rate;  /* Sample rate */
        guint8 pcmout_stereo;  /* Stereo? */

	char *module_path; /* path to directories modules are loaded from */
	void *dummy; /* This is sad... */
} config_entry_t;

int
config_init(config_entry_t **conf, char *conffil);
/* Initializes the config entry structurre based on information found in the config file.
** Parameters: (config_entry_t **) conf: See below
**             (char *) conffile: Filename of the config file, or NULL to use the default name
** Returns   : (int) The number of config file entries found
** This function reads the ~/.freesci/config file, parses it, and inserts the appropriate
** data into *conf. *conf will be malloc'd to be an array containing default information in [0]
** and game-specific data in each of the subsequent record entries.
** Not threadsafe. Uses flex-generated code.
*/

void
config_free(config_entry_t **conf, int entries);
/* Frees a config entry structure
** Parameters: (config_entry_t **) conf: Pointer to the pointer to the first entry of the list
**             (int) entries: Number of entries to free
** Returns   : (void)
*/


void *
parse_gfx_driver(char *driver_name);
/* Parses a string and looks up an appropriate driver structure
** Parameters: (char *) driver_name: Name of the driver to look up
** Returns   : (void *) A matching driver, or NULL on failure
*/

void *
parse_midiout_driver(char *driver_name);
/* Parses a string and looks up an appropriate driver structure
** Parameters: (char *) driver_name: Name of the driver to look up
** Returns   : (void *) A matching driver, or NULL on failure
*/

void *
parse_midi_device(char *driver_name);
/* Parses a string and looks up an appropriate driver structure
** Parameters: (char *) driver_name: Name of the driver to look up
** Returns   : (void *) A matching driver, or NULL on failure
*/

void *
parse_pcmout_driver(char *driver_name);
/* Parses a string and looks up an appropriate driver structure
** Parameters: (char *) driver_name: Name of the driver to look up
** Returns   : (void *) A matching driver, or NULL on failure
*/

void *
parse_sound_server(char *driver_name);
/* Parses a string and looks up an appropriate driver structure
** Parameters: (char *) driver_name: Name of the driver to look up
** Returns   : (void *) A matching sound server, or NULL on failure
*/

driver_option_t *
get_driver_options(config_entry_t *config, int subsystem, const char *name);
/* Retreives the driver options for one specific driver in a subsystem
** Parameters: (config_entry_t *) config: The config entry to search in
**             (int) subsystem: Any of the FREESCI_DRIVER_SUBSYSTEMs
**             (const char *) name: Name of the driver to look for
** Returns   : (driver_option_t *) A pointer to the first option in
**             a singly-linked list of options, or NULL if none was
**             found
*/

#if 0
void *
find_module(char *path, char *module_name, char *module_suffix);
/* Tries to find a module in the specified path
** Parameters: (char *) path: The path to search in (specified in config)
**             (char *) module_name: Name of the module to look for
**             (char *) module_suffix: Module structure to look for
** More precisely, the module "module_name" + MODULE_NAME_SUFFIX is
** looked for in all members of the path. If it is found,

** FIXME: First need to add generic module architecture

*/
#endif

#endif /* !_SCI_CONFIG_H */
