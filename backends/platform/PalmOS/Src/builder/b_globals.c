#include <PalmOS.h>
#include "b_globals.h"


//#define BUILD_COMMON
//#define BUILD_SIMON
#define BUILD_SCUMM
//#define BUILD_SKY
//#define BUILD_QUEEN
//#define BUILD_SWORD1

#ifdef BUILD_COMMON
#	define BUILD_RES	GBVARS_COMMON
#	define BUILD_NAME	"Common"
#else
#	define BUILD_RES	GBVARS_ENGINE
#	define BUILD_NAME	"Engine"
#endif

DmOpenRef dbP[GBVARS_COUNT];
///////////////////////////////////////////////////////////////////

void writeRecord(MemPtr ptr, UInt32 size, UInt16 index, UInt16 dbID) {
	Err e;
	
	MemHandle recordH = DmNewRecord(dbP[dbID], &index, size);
	MemPtr save = MemHandleLock(recordH);
	e = DmWrite(save, 0, ptr, size);
	MemHandleUnlock(recordH);
	DmReleaseRecord (dbP[dbID], index, 0);
}
///////////////////////////////////////////////////////////////////

void buildAll() {
	Err err;
	LocalID olddb;	

	olddb = DmFindDatabase(0, "Glbs::" BUILD_NAME);
	if (olddb) {
		DmDeleteDatabase(0,olddb);
		FrmCustomAlert(1000,"delete old " BUILD_NAME " DB",0,0);
	}
	err = DmCreateDatabase (0, "Glbs::" BUILD_NAME, 'ScVM', 'GLBS', false);
	olddb = DmFindDatabase(0, "Glbs::" BUILD_NAME);
	dbP[BUILD_RES] = DmOpenDatabase(0, olddb, dmModeReadWrite);

#if defined(BUILD_COMMON)
	addNewGui();

#elif defined(BUILD_SCUMM)
	addDimuseTables();
	// temp removed
	// TODO ::scummvm use sizeof(OLD256_MIDI_HACK) so i need to fix it
	// directly in the code or call MemHandleSize but it may slow down
	// code execution
	addAkos();
	addDimuseCodecs();
	addCodec47();
	addGfx();
	addDialogs();
	addCharset();
	addCostume();
	addPlayerV2();
	addScummTables();

#elif defined(BUILD_SIMON)
	addSimon();
	Simon_addCharset();

#elif defined(BUILD_SKY)
	Sky_addHufftext();

#elif defined(BUILD_QUEEN)
	Queen_addTalk();
	Queen_addRestables();
	Queen_addGraphics();
	Queen_addDisplay();
	Queen_addMusicdata();

#elif defined(BUILD_SWORD1)
	Sword1_addStaticres();
#endif

	DmCloseDatabase(dbP[BUILD_RES]);
}
