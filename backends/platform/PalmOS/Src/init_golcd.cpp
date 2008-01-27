#include <PalmOS.h>
#include <PalmGoLCD.h>

#include "init_golcd.h"

// you can use this handle directly
MemHandle gGoLcdH;

Err GoLCDInit(MemHandle *goLcdH) {
	if (!goLcdH)
		return sysErrParamErr;

	Err e;
	UInt16 refNum;
	Boolean loaded = false;
	*goLcdH = NULL;

	if (e = SysLibFind(goLcdLibName, &refNum))
		loaded = !(e = SysLibLoad(goLcdLibType, goLcdLibCreator, &refNum));

	if (!e)
		if ((e = GoLcdLibOpen(refNum)))
			SysLibRemove(refNum);

	if (!e) {
		MemHandle lcdH = MemHandleNew(sizeof(GoLCDType));
		GoLCDType *lcdP = (GoLCDType *)MemHandleLock(lcdH);
		MemSet(lcdP, MemHandleSize(lcdH), 0);
		lcdP->refNum = refNum;
		lcdP->timeout = GoLcdGetTimeout(refNum, goLcdPenTapMode);
		MemPtrUnlock(lcdP);
		*goLcdH = lcdH;
	}

	return e;
}

Err GoLCDRelease(MemHandle goLcdH) {
	if (!goLcdH)
		return sysErrParamErr;

	Err e;
	GoLCDType *lcdP = (GoLCDType *)MemHandleLock(goLcdH);
	if (lcdP->refNum != sysInvalidRefNum)
		if (!(e = GoLcdLibClose(lcdP->refNum)))
			e = SysLibRemove(lcdP->refNum);

	MemPtrUnlock(lcdP);
	MemHandleFree(goLcdH);

	return e;
}

Boolean GoLCDPointInBounds(MemHandle goLcdH, Coord x, Coord y) {
	Boolean inBounds = false;

	if (!goLcdH)
		return inBounds;

	GoLCDType *lcdP = (GoLCDType *)MemHandleLock(goLcdH);
	inBounds = (lcdP->active && RctPtInRectangle(x, y, &(lcdP->bounds)));
	MemPtrUnlock(lcdP);

	return inBounds;
}

void GoLCDSetInk(MemHandle goLcdH, RGBColorType *inkP) {
	if (!goLcdH)
		return;

	GoLCDType *lcdP = (GoLCDType *)MemHandleLock(goLcdH);
	MemMove(&(lcdP->ink), inkP, sizeof(RGBColorType));
	MemPtrUnlock(lcdP);
}

void GoLCDSetBounds(MemHandle goLcdH, RectangleType *boundsP) {
	if (!goLcdH)
		return;

	GoLCDType *lcdP = (GoLCDType *)MemHandleLock(goLcdH);
	MemMove(&(lcdP->bounds), boundsP, sizeof(RectangleType));
	MemPtrUnlock(lcdP);
}

void GoLCDActivate(MemHandle goLcdH, Boolean active) {
	if (!goLcdH)
		return;

	GoLCDType *lcdP = (GoLCDType *)MemHandleLock(goLcdH);

	if (active) {
		if (!lcdP->active) {
			lcdP->active = true;
			GoLcdSetInkState(lcdP->refNum, goLcdInkEnabled, goLcdColorOverride, &(lcdP->ink));
			GoLcdSetBounds(lcdP->refNum, &(lcdP->bounds));
			GoLcdSetGsiState(lcdP->refNum, goLcdGsiNormal, goLcdColorDefault, 0);
			GoLcdSetTimeout(lcdP->refNum, goLcdPenTapMode, lcdP->timeout);
			GoLcdSetStatus(lcdP->refNum, goLcdEnabled);
		}

	} else {
			lcdP->active = false;
			GoLcdSetInkState(lcdP->refNum, goLcdInkDisabled, goLcdColorDefault, 0);
			GoLcdSetStatus(lcdP->refNum, goLcdDisabled);
	}

	MemPtrUnlock(lcdP);
}

Boolean GoLCDToggle(MemHandle goLcdH) {
	if (!goLcdH)
		return;

	Boolean active;
	GoLCDType *lcdP = (GoLCDType *)MemHandleLock(goLcdH);
	active = lcdP->active;
	MemPtrUnlock(lcdP);

	GoLCDActivate(goLcdH, !active);
	return (!active);
}
