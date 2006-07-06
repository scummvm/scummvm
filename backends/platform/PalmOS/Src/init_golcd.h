#ifndef INIT_GOLCD_H
#define INIT_GOLCD_H

typedef struct {
	Boolean active;
	UInt16 refNum;
	RectangleType bounds;
	RGBColorType ink;
	UInt32 timeout;
} GoLCDType;

extern MemHandle gGoLcdH;

Err GoLCDInit(MemHandle *goLcdH);
Err GoLCDRelease(MemHandle goLcdH);
void GoLCDSetInk(MemHandle goLcdH, RGBColorType *inkP);
void GoLCDSetBounds(MemHandle goLcdH, RectangleType *boundsP);
void GoLCDActivate(MemHandle goLcdH, Boolean active);
Boolean GoLCDToggle(MemHandle goLcdH);
Boolean GoLCDPointInBounds(MemHandle goLcdH, Coord x, Coord y);

#endif
