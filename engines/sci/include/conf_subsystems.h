/***************************************************************************
  Copyright (C) 2007 Christoph Reichenbach


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

#ifndef CONF_SUBSYSTEMS_H_
#define CONF_SUBSYSTEMS_H_

#include "conf_parse.h"
#include "conf_driver.h"

conf_parse_t *
conf_validate(conf_main_t *main, conf_parse_t *raw_config);
/* Validates raw configuration data (including subsections) and fills in ``overridden'' and ``prev'' pointers
** Parameters: (conf_parse_t *) raw_config:  The raw config data
** Returns   : (conf_parse_t *) The updated raw_config on success, or NULL if validation failed
**                              (raw_config is not freed in that case)
** Effects   : (stderr) Error message(s) if validation failed
*/

int
conf_init(conf_main_t *main, conf_parse_t *config, const char *id);
/* Validates all parameters, sets all parameters, loads modules, initialises drivers and subsystems and the main module
** Parameters: (conf_main_t *) The main config element
**             (conf_parse_t *) The parse
**             (char *) id: The game ID to choose for initialisation, or NULL for default
** Effects   : (stderr) Error message(s) if validation failed
*/

char *
conf_set_main_option(conf_main_t *main, const char *option, const char *value);
/* Sets a value for the main module
** Parameters: (char *) option: Name of the option to set
**             (char *) value: Value to set
** Returns   : (char *) NULL on success, an error message otherwise
*/

char *
conf_set_subsystem_option(conf_main_t *main, const char *subsystem, const char *option, const char *value);
/* Sets a value for the main module
** Parameters: (char *) option: Name of the option to set
**             (char *) value: Value to set
**	       (char *) subsystem: Name of the subsystem to manipulate
** Returns   : (char *) NULL on success, an error message otherwise
*/

char *
conf_set_driver_option(conf_main_t *main, const char *subsystem, const char *driver, const char *option, const char *value);
/* Sets a value for the main module
** Parameters: (char *) option: Name of the option to set
**             (char *) value: Value to set
**	       (char *) subsystem: Name of the subsystem to manipulate
**	       (char *) driver: Name of the driver to manipulate
** Returns   : (char *) NULL on success, an error message otherwise
** Automatically loads the driver, if neccessary
*/

#endif /* !defined(CONF_SUBSYSTEMS_H_) */
