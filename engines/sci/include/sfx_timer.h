/***************************************************************************
 sfx_timer.h Copyright (C) 2002..04 Christoph Reichenbach


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

#ifndef _FREESCI_SFX_TIMER_H_
#define _FREESCI_SFX_TIMER_H_

#include "sci/include/sfx_core.h"

typedef struct {
	const char *name;
	const char *version;

	int delay_ms; /* Approximate delay (in milliseconds) between calls */
	int flags;

	int
	(*set_option)(char *name, char *value);
	/* Sets an option for the timing mechanism
	** Parameters: (char *) name: The name describing what to set
	**             (char *) value: The value to set
	** Returns   : (int) SFX_OK, or SFX_ERROR if the name wasn't understood
	** May be NULL
	*/

	int
	(*init)(void (*callback)(void *data), void *data);
	/* Initializes the timer
	** Parameters: (void* -> void) callback:
	**                                   'data' must contain the next argument:
	**             (void *) data: Must always be passed to the callback
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** This does not start the timer yet, it just specifies and initializes it.
	** This function is called exactly once (provided that the timer is used at all).
	*/

	int
	(*exit)(void);
	/* Stops the timer
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** All resources allocated with the timer should be freed as an effect
	** of this.
	*/

	int
	(*block)(void);
	/* Blocks the timer
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** When this function returns it is guaranteed that no timer callback is
	** currently being executed and that no new timer callback will occur
	** until unblock() is called.
	*/

	int
	(*unblock)(void);
	/* Unblocks the timer
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** Any callbacks that were blocked should be executed immediately when
	** possible.
	*/
} sfx_timer_t;

extern sfx_timer_t *
	sfx_find_timer(char *name);
/* Finds a timer by name
** Parameters: (char *) name: Name of the timer to look up, or NULL for default
** Returns   : (sfx_timer_t *) The timer of matching name, or NULL
**                             if not found
** This does not consider timers provided by PCM devices; there must be
** retrieved externally.
*/

#endif /* !_FREESCI_SFX_TIMER_H_ */
