/***************************************************************************
 modules.h Copyright (C) 2001 Christoph Reichenbach


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

#ifndef _FREESCI_MODULES_H_
#define _FREESCI_MODULES_H_

#include <sciresource.h>

#ifdef _WIN32
#  define MODULE_NAME_SUFFIX ".dll"
#else
#  define MODULE_NAME_SUFFIX ".so"
#endif


/* Modules must sub-type this structure. Oh, wait, C doesn't do sub-typing.
** Well, they have to start the same, anyway...  */
typedef struct {
	char *module_name; /* Module name */
	char *module_version; /* Module version */
	int class_magic; /* Magic ID to identify drivers of certain types */
	int class_version; /* Version of that particular driver class */

	/* More entries might be added later, but won't be checked if magic or
	** version don't match.  */

} sci_module_t;


void *
sci_find_module(char *path, char *name, char *type, char *struct_prefix,
		char *file_suffix, int magic, int version, void **handle);
/* Attempts to open a module with the specified parameters in the path
** Parameters: (char *) path: The path to look in; supports several directories
**             (char *) name: Module name
**             (char *) type: Module type string (see below)
**             (char *) struct_prefix: see below
**             (char *) file_suffix: see below
**             (int) magic: Magic number to identify the module type
**             (int) version: The only version to support
**             (void **) handle: Pointer to a void * which a handle is written
**                               to (for use with sci_close_module()). 
** Returns   : (void *) NULL if no module was found, a pointer to the structure
**                      otherwise
** This function looks for the structure 'struct_prefix + name' in a file called
** 'name + file_suffix + MODULE_NAME_SUFFIX' in the 'type' subdirectory.
** It only success if both the magic and the version number match.
*/

void
sci_close_module(void *module, char *type, char *name);
/* Closes a previously found module
** Parameters: (void *) module: Reference to the module to close
**             (char *) type: Type of the module (for debugging)
**             (char *) name: Module name (for debugging)
** Returns   : (void)
*/ 
	

#endif /* !_FREESCI_MODULES_H_ */
