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
	int delay_ms; /* Approximate delay (in milliseconds) between calls */

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
} sfx_timer_t;

#endif /* !_FREESCI_SFX_TIMER_H_ */
