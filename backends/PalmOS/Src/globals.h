#ifndef GLOBALS_H
#define GLOBALS_H

typedef struct {
	DmOpenRef globals[3];

	UInt16 HRrefNum;
	UInt16 volRefNum;
	FileRef	logFile;

	Boolean screenLocked;
	Boolean vibrator;
	Boolean stdPalette;
/*
	struct {
		UInt16 speaker;
		UInt16 headphone;
	} volume;
*/
	struct {
		UInt8 on;
		UInt8 off;
	} indicator;
	
	struct {
		UInt8 *pageAddr1;
		UInt8 *pageAddr2;
	} flipping;

} GlobalsDataType;

extern GlobalsDataType *gVars;


#endif