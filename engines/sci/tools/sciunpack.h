/***************************************************************************
 sciunpack.h Copyright (C) 2001 Christoph Reichenbach


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

#ifndef SCI_UNPACK_H_
#define SCI_UNPACK_H_

#include <sciresource.h>

#define SORT_METHOD_ALPHA 0
#define SORT_METHOD_GROUP 1

#define DEFAULT_SORTING SORT_METHOD_ALPHA

extern int vocab_sort; /* Sorting strategy for vocab */
extern resource_mgr_t *resmgr;

int
vocab_print(void);
/* Prints vocab data
** Parameters: (void)
** Returns   : (int) 0 on success, 1 on failure
** Controlled by vocab_sort
*/

int
script_dump(void);
/* Prints all object information
** Parameters: (void)
** Returns   : (int) 0 on success, 1 on failure
*/

int
vocab_dump(void);
/* Prints full vocabulary information
** Parameters: (void)
** Returns   : (int) 0 on success, 1 on failure
*/

int
print_classes(void);
/* Prints full class information
** Parameters: (void)
** Returns   : (int) 0 on success, 1 otherwise
*/

#endif
