#ifndef __STARFORCE_H__
#define __STARFORCE_H__

#define USE_STARFORCE

#ifndef USE_STARFORCE

#include "securom_api.h"

#define STARFORCE_API 

#else // USE_STARFORCE

#define STARFORCE_API __declspec(dllexport)

#define SECUROM_MARKER_HIGH_SECURITY_ON(x)
#define SECUROM_MARKER_HIGH_SECURITY_OFF(x)
#define SecuROM_Tripwire() 1

#endif // USE_STARFORCE

STARFORCE_API void initConditions();
STARFORCE_API void initActions();
STARFORCE_API void initActionsEnvironmental();
STARFORCE_API void initActionsSound();

#endif //__STARFORCE_H__
