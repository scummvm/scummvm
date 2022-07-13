#include <string.h>
#include "scripthelper.h"
#include <assert.h>
#include <stdio.h>
#include <fstream>
#include <set>
#include <stdlib.h>

using namespace std;

BEGIN_AS_NAMESPACE

int ExecuteString(asIScriptEngine *engine, const char *code, asIScriptModule *mod, asIScriptContext *ctx)
{
	return ExecuteString(engine, code, 0, asTYPEID_VOID, mod, ctx);
}

int ExecuteString(asIScriptEngine *engine, const char *code, void *ref, int refTypeId, asIScriptModule *mod, asIScriptContext *ctx)
{
	// Wrap the code in a function so that it can be compiled and executed
	string funcCode = " ExecuteString() {\n";
	funcCode += code;
	funcCode += "\n;}";

	// Determine the return type based on the type of the ref arg
	funcCode = engine->GetTypeDeclaration(refTypeId, true) + funcCode;

	// GetModule will free unused types, so to be on the safe side we'll hold on to a reference to the type
	asITypeInfo *type = 0;
	if( refTypeId & asTYPEID_MASK_OBJECT )
	{
		type = engine->GetTypeInfoById(refTypeId);
		if( type )
			type->AddRef();
	}

	// If no module was provided, get a dummy from the engine
	asIScriptModule *execMod = mod ? mod : engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);

	// Now it's ok to release the type
	if( type )
		type->Release();

	// Compile the function that can be executed
	asIScriptFunction *func = 0;
	int r = execMod->CompileFunction("ExecuteString", funcCode.c_str(), -1, 0, &func);
	if( r < 0 )
		return r;

	// If no context was provided, request a new one from the engine
	asIScriptContext *execCtx = ctx ? ctx : engine->RequestContext();
	r = execCtx->Prepare(func);
	if (r >= 0)
	{
		// Execute the function
		r = execCtx->Execute();

		// Unless the provided type was void retrieve it's value
		if (ref != 0 && refTypeId != asTYPEID_VOID)
		{
			if (refTypeId & asTYPEID_OBJHANDLE)
			{
				// Expect the pointer to be null to start with
				assert(*reinterpret_cast<void**>(ref) == 0);
				*reinterpret_cast<void**>(ref) = *reinterpret_cast<void**>(execCtx->GetAddressOfReturnValue());
				engine->AddRefScriptObject(*reinterpret_cast<void**>(ref), engine->GetTypeInfoById(refTypeId));
			}
			else if (refTypeId & asTYPEID_MASK_OBJECT)
			{
				// Use the registered assignment operator to do a value assign. 
				// This assumes that the ref is pointing to a valid object instance.
				engine->AssignScriptObject(ref, execCtx->GetAddressOfReturnValue(), engine->GetTypeInfoById(refTypeId));
			}
			else
			{
				// Copy the primitive value
				memcpy(ref, execCtx->GetAddressOfReturnValue(), engine->GetSizeOfPrimitiveType(refTypeId));
			}
		}
	}

	// Clean up
	func->Release();
	if( !ctx ) engine->ReturnContext(execCtx);

	return r;
}

END_AS_NAMESPACE
