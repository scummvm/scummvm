#include "enum_globals.h"

typedef unsigned char byte;
typedef UInt32 uint;
typedef Int32 int32;
typedef UInt32 uint32;


void writeRecord(MemPtr ptr, UInt32 size, UInt16 index, UInt16 dbID);
void buildAll();

void addNewGui();
void addGraphics();
/////////////////////////////////
void addDimuseTables();
void addAkos();
void addDimuseCodecs();
void addCodec47();
void addGfx();
void addDialogs();
void addCharset();
void addCostume();
void addPlayerV2();
void addScummTables();
/////////////////////////////////
void addSimon();
void Simon_addCharset();
/////////////////////////////////
void Queen_addTalk();
void Queen_addRestables();
void Queen_addGraphics();
void Queen_addDisplay();
void Queen_addMusicdata();
/////////////////////////////////
void Sky_addHufftext();
/////////////////////////////////
void Sword1_addStaticres();
