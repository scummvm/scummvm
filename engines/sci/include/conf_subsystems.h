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
