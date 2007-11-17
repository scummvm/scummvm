#include <PalmOS.h>
#include "palmdefs.h"
#include "args.h"
#include "pace.h"
#include "endianutils.h"

Char **ArgsInit() {
	// first 4 bytes keep the real address of the chunk
	MemHandle argvH = MemHandleNew(sizeof(Char *) + (MAX_ARG * sizeof(Char **) + 2));
	// real addr
	Char *lockP = (Char *)MemHandleLock(argvH);
	// 4byte aligned
	Char **argvP = (Char **)ALIGN_4BYTE(lockP);
	// save real addr
	argvP[0] = lockP;
	// initial position
	argvP++;

	for (UInt8 count = 0; count < MAX_ARG; count++)
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

	for (UInt8 count = 0; count < MAX_ARG; count++)
		if (argvP[count]) {
			oldH = MemPtrRecoverHandle(argvP[count]);
			MemHandleUnlock(oldH);
			MemHandleFree(oldH);
		}

	argvP--;
	oldH = MemPtrRecoverHandle(argvP[0]);
	MemHandleUnlock(oldH);
	MemHandleFree(oldH);
}

void ArgsSetOwner(Char **argvP, UInt16 owner) {
	if (!argvP)
		return;

	MemHandle oldH;

	for (UInt8 count = 0; count < MAX_ARG; count++)
		if (argvP[count]) {
			oldH = MemPtrRecoverHandle(argvP[count]);
			MemHandleSetOwner(oldH, owner);
		}

	argvP--;
	oldH = MemPtrRecoverHandle(argvP[0]);
	MemHandleSetOwner(oldH, owner);
}

void ArgsExportInit(Char **argvP, UInt32 countArg, Boolean arm) {
	if (arm) {
		for (UInt8 count = 0; count < MAX_ARG; count++)
			if (argvP[count])
				argvP[count] = (Char *)ByteSwap32(argvP[count]);
	}

	FtrSet(appFileCreator, ftrArgsData , (UInt32)argvP);
	FtrSet(appFileCreator, ftrArgsCount, (UInt32)countArg);
}

void ArgsExportRelease(Boolean arm) {
	if (arm) {
		Char **argvP;
		Err e = FtrGet(appFileCreator, ftrArgsData, (UInt32 *)&argvP);

		if (argvP)
			for (UInt8 count = 0; count < MAX_ARG; count++)
				if (argvP[count])
					argvP[count] = (Char *)ByteSwap32(argvP[count]);
	}

	FtrUnregister(appFileCreator, ftrArgsCount);
	FtrUnregister(appFileCreator, ftrArgsData);
}
