#ifndef __FORMS_H__
#define __FORMS_H__

#include "StarterRsc.h"
#include "formUtil.h"

#include "formEditGame.h"
#include "formCards.h"


#define HANDLER(x)	Boolean  x##FormHandleEvent(EventPtr eventP);

HANDLER(Main)
HANDLER(EditGame)
HANDLER(SystemInfo)
HANDLER(Misc)
HANDLER(CardSlot)
HANDLER(Skins)
HANDLER(Music)
HANDLER(Info)
HANDLER(Selector)

#endif
