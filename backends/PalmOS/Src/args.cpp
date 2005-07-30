#include <PalmOS.h>
#include "args.h"

Char **ArgsInit() {
	MemHandle argvH = MemHandleNew(MAX_ARG * sizeof(Char *));
	Char **argvP = (Char **)MemHandleLock(argvH);

	for(UInt8 count = 0; count < MAX_ARG; count++)
		argvP[count] = NULL;

	return argvP;
}

void ArgsAdd(Char **argvP, const Char *argP, const Char *parmP, UInt8 *countArgP) {
	if (argP) {
		MemHandle newArg;
		UInt16 len2 = 0;
		UInt16 len1 = StrLen(argP);

		if (len1 > 0) {
			if (parmP)
				len2 = StrLen(parmP);

			(*countArgP)++;
			newArg = MemHandleNew(len1 + len2 + 1); // +1 = NULL CHAR
			*argvP = (Char *)MemHandleLock(newArg);
			StrCopy(*argvP, argP);

			if (parmP)
				StrCat(*argvP, parmP);
		}
	}
}

void ArgsFree(Char **argvP) {
	if (!argvP)
		return;

	MemHandle oldH;

	for(UInt8 count = 0; count < MAX_ARG; count++)
		if (argvP[count]) {
			oldH = MemPtrRecoverHandle(argvP[count]);
			MemHandleUnlock(oldH);
			MemHandleFree(oldH);
		}

	oldH = MemPtrRecoverHandle(argvP);
	MemHandleUnlock(oldH);
	MemHandleFree(oldH);
}

void ArgsSetOwner(Char **argvP, UInt16 owner) {
	if (!argvP)
		return;

	MemHandle oldH;

	for(UInt8 count = 0; count < MAX_ARG; count++)
		if (argvP[count]) {
			oldH = MemPtrRecoverHandle(argvP[count]);
			MemHandleSetOwner(oldH, owner);
//			MemPtrSetOwner(argvP[count], 0);
		}

	oldH = MemPtrRecoverHandle(argvP);
	MemHandleSetOwner(oldH, owner);
//	MemPtrSetOwner(argvP, 0);
}
