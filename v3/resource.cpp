#include"../stdafx.h"
#include"../scumm.h"


void Scumm_v3::readIndexFile() {
       uint16 blocktype;
       uint32 itemsize;
       int numblock = 0;
       int num, i;
       byte* _oldClass; 

       debug(9, "readIndexFile()");

       openRoom(-1);
       openRoom(0);

       if (!(_features & GF_AFTER_V6)) {
               while (!fileEof(_fileHandle)) {
                       itemsize = fileReadDwordLE();
                       blocktype = fileReadWordLE();
                       if (fileReadFailed(_fileHandle))
                               break;

                       switch(blocktype) {
                       case 0x4E52:
                               fileReadWordLE();
                               break;
                       case 0x5230:
                               _numRooms = fileReadWordLE();
                               break;
                       case 0x5330:
                               _numScripts = fileReadWordLE();
                               break;
                       case 0x4E30:
                               _numSounds = fileReadWordLE();
                               break;
                       case 0x4330:
                               _numCostumes = fileReadWordLE();
                               break;
                       case 0x4F30:
                               _numGlobalObjects = fileReadWordLE();
                               break;
                       }
                       fileSeek(_fileHandle, itemsize-8,SEEK_CUR);
               }
               clearFileReadFailed(_fileHandle);
               fileSeek(_fileHandle, 0, SEEK_SET);
       }

       /* I'm not sure for those values yet, they will have to be rechecked */

       _numVariables = 800; /* 800 */
       _numBitVariables = 4096; /* 2048 */
       _numLocalObjects = 200; /* 200 */
       _numArray = 50;
       _numVerbs = 100;
       _numNewNames = 0;
       _objectRoomTable = NULL;
       _numCharsets = 9; /* 9 */
       _numInventory = 80; /* 80 */
       _numGlobalScripts = 200;

       _shadowPaletteSize = 256;
       _shadowPalette = (byte*)alloc(_shadowPaletteSize); // stupid for now. Need to be removed later
       _numFlObject = 50;
       allocateArrays();

       while (1) {
               itemsize = fileReadDwordLE();

               if (fileReadFailed(_fileHandle))
                       break;

               blocktype = fileReadWordLE();

               numblock++;

               switch(blocktype) {

               case 0x4E52:
                       fileSeek(_fileHandle, itemsize-6,SEEK_CUR);
                       break;

               case 0x5230:
                       readResTypeList(rtRoom,MKID('ROOM'),"room");
                       break;

               case 0x5330:
                       readResTypeList(rtScript,MKID('SCRP'),"script");
                       break;

               case 0x4E30:
                       readResTypeList(rtSound,MKID('SOUN'),"sound");
                       break;

               case 0x4330:
                       readResTypeList(rtCostume,MKID('COST'),"costume");
                       break;

               case 0x4F30:
                       num = fileReadWordLE();
                       assert(num == _numGlobalObjects);
                       for (i=0; i<num; i++) { /* not too sure about all that */
                                _oldClass=(byte*)&_classData[i];
                                _oldClass[0]=fileReadByte();
                                _oldClass[1]=fileReadByte();
                                _oldClass[2]=fileReadByte();
                                _objectOwnerTable[i] = fileReadByte();
                            //   _objectStateTable[i] = fileReadByte();
                               _objectOwnerTable[i] &= OF_OWNER_MASK;
                       }

#if defined(SCUMM_BIG_ENDIAN)
                       for (i=0; i<num; i++) {
                               _classData[i] = FROM_LE_32(_classData[i]);
                       }
#endif
                       break;

               default:
                       error("Bad ID %c%c found in directory!", blocktype&0xFF, blocktype>>8);
                       return;
               }
       }

       openRoom(-1);
}
