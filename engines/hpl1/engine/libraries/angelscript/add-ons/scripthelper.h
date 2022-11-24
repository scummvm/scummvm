#ifndef SCRIPTHELPER_H
#define SCRIPTHELPER_H

#ifndef ANGELSCRIPT_H
// Avoid having to inform include path if header is already include before
#include "hpl1/engine/libraries/angelscript/angelscript.h"
#endif


BEGIN_AS_NAMESPACE

// Compile and execute simple statements
// The module is optional. If given the statements can access the entities compiled in the module.
// The caller can optionally provide its own context, for example if a context should be reused.
int ExecuteString(asIScriptEngine *engine, const char *code, asIScriptModule *mod = 0, asIScriptContext *ctx = 0);

// Compile and execute simple statements with option of return value
// The module is optional. If given the statements can access the entitites compiled in the module.
// The caller can optionally provide its own context, for example if a context should be reused.
int ExecuteString(asIScriptEngine *engine, const char *code, void *ret, int retTypeId, asIScriptModule *mod = 0, asIScriptContext *ctx = 0);

END_AS_NAMESPACE

#endif
