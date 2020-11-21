#ifndef ADDR_AND_MANAGER_H
#define ADDR_AND_MANAGER_H

#include "script/runtimescriptvalue.h"
#include "ac/dynobj/cc_dynamicobject.h"

extern ScriptValueType ccGetObjectAddressAndManagerFromHandle(
   int32_t handle, void *&object, ICCDynamicObject *&manager);

#endif

