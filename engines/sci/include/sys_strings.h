/***************************************************************************
 sys_strings.h Copyright (C) 2002 Christoph Reichenbach


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

#ifndef _FREESCI_SYSTEM_STRINGS_H_
#define _FREESCI_SYSTEM_STRINGS_H_

#define SYS_STRINGS_MAX 4

#define SYS_STRING_SAVEDIR 0
#define SYS_STRING_PARSER_BASE 1

#define MAX_PARSER_BASE 64

typedef struct {
	char *name;
	int max_size;
	char *value;
} sys_string_t;

typedef struct {
	sys_string_t strings[SYS_STRINGS_MAX];
} sys_strings_t;

void
sys_string_acquire(sys_strings_t *strings, int index, const char *name, int max_len);
/* Reserves a new system string
** Parameters: (sys_strings_t *) strings: The string table to reserve in
**             (int) index: Index number to reserve
**             (const char *) name: Name the entry should be tagged with
**             (int) max_len: Maximum string length in bytes
*/

int
sys_string_set(sys_strings_t *strings, int index, const char *value);
/* Sets the value of a system string
** Parameters: (sys_strings_t *) strings: The string table to use
**             (int) index: Index of the string to write to
**             (const char *) value: The value to copy
** Returns   : 0 on success, 1 on error
** Length clipping is performed.
*/

void
sys_strings_restore(sys_strings_t *new_strings, sys_strings_t *old_strings);
/* Cleanup system strings after a gamestate restore
** Parameters: (sys_strings_t *) The freshly loaded system strings to clean up
**             (sys_strings_t *) The old system strings to clean up
*/

void
sys_string_free_all(sys_strings_t *strings);
/* Deallocates all allocated system strings
** Parameters: (sys_strings_t *) strings: The string table to deallocate
*/

#endif /* !_FREESCI_SYSTEM_STRINGS_H_ */
