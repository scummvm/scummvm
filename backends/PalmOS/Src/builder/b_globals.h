#include "enum_globals.h"

// define this to limit memory request on Simon
#define SMALL_MEMORY

void writeRecord(MemPtr ptr, UInt32 size, UInt16 index, UInt16 dbID);
void buildAll();

void addImuseDigi();
void addNewGui();
void addAkos();
void addBundle();
void addCodec47();
void addGfx();
void addDialogs();
void addCharset();
void addCostume();
void addPlayerV2();
/////////////////////////////////
void addSimon();
void Simon_addCharset();
/////////////////////////////////
void addRestables();
