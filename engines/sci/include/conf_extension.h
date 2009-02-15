/***************************************************************************
  Copyright (C) 2008 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantability,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

/* Config extensions handle special-purpose configuration options such as the
** graphics operations.  To add new special-purpose operations, modify this file,
** the lexer, and conf_extensions.c.
*/

#ifndef CONF_EXTENSION_H_
#define CONF_EXTENSION_H_

#include "common/scummsys.h"

#define CONF_EXT_TYPE_INVALID	-1
#define CONF_EXT_TYPE_GFX	0

typedef struct {
	int type;	/* CONF_EXT_TYPE_* */
	void *data;
} conf_extension_t;



int
conf_extension_supercedes(conf_extension_t *a, conf_extension_t *b);
/* Determines whether conf extension b shadows extension a
** Parameters: (conf_extension_t *) a: The ``earlier'' extension
**             (conf_extension_t *) b: The ``later'' extension
** Returns   : nonzero iff shadowing does occur
*/

void
conf_extension_print(FILE *file, conf_extension_t *a);
/* Prints out a config extension
** Parameters: (FILE *) file: The file to print to
**             (conf_extension_t *) a: The extension to print out
*/

void
conf_extension_free(conf_extension_t *a);
/* Frees up all data associated with a conf extension
** Parameters: (conf_extension_t *) a: The extension to free
** This also frees up a itself.
*/

#endif /* !defined CONF_EXTENSION_H_ */
