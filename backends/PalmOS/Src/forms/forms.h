#ifndef __FORMS_H__
#define __FORMS_H__

#include "StarterRsc.h"
#include "formUtil.h"

#include "formEditGame.h"
#include "formCards.h"


#define HANDLER(x)	Boolean  x##FormHandleEvent(EventPtr eventP);

HANDLER(EditGame)
HANDLER(Volume)
HANDLER(SystemInfo)
HANDLER(Sound)
HANDLER(MiscOptions)
HANDLER(CardSlot)
HANDLER(Skins)

#endif