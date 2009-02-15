#ifndef EVENT_H
#define EVENT_H

#include "sci/include/uinput.h"

struct _state;

sci_event_t getEvent(struct _state *s);
/* Returns the next SCI_EV_* event
** Parameters: (struct state *) Current game state
** Returns   : (sci_event_t) The next event, which may be any of the
**             existing events.
*/

#endif
