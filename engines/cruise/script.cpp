/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL:https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2007-fsnode/engines/cruise/script.cpp $
 * $Id:script.cpp 26949 2007-05-26 20:23:24Z david_corrales $
 *
 */

#include "cruise/cruise_main.h"

namespace Cruise {

scriptInstanceStruct relHead;
scriptInstanceStruct procHead;

scriptInstanceStruct *currentScriptPtr;

uint8 getByteFromScript(void) {
	uint8 var = currentData3DataPtr[currentScriptPtr->var4];

	currentScriptPtr->var4 = currentScriptPtr->var4 + 1;

	return (var);
}

short int getShortFromScript(void) {
	short int var = *(int16 *) (currentData3DataPtr + currentScriptPtr->var4);

	currentScriptPtr->var4 = currentScriptPtr->var4 + 2;

	flipShort(&var);

	return (var);
}

// load opcode
int32 opcodeType0(void) {
	switch (currentScriptOpcodeType) {
	case 0:
		{
			pushVar(getShortFromScript());
			return (0);
		}
	case 1:
		{
			uint8 *ptr = 0;
			int byte1 = getByteFromScript();
			int byte2 = getByteFromScript();
			short int short1 = getShortFromScript();

			int var_E = byte1 & 7;

			if (!var_E) {
				return (-10);
			}

			if (!byte2) {
				ptr = scriptDataPtrTable[var_E] + short1;
			} else	// TODO: 
			{
				if (!overlayTable[byte2].alreadyLoaded) {
					return (-7);
				}

				if (!overlayTable[byte2].ovlData) {
					return (-4);
				}

				if (var_E == 5) {
					ptr =
					    overlayTable[byte2].ovlData->
					    data4Ptr + short1;
				} else {
					assert(0);
				}
			}

			if (((byte1 & 0x18) >> 3) == 1) {
				pushVar(loadShort(ptr));
				return (0);
			} else if (((byte1 & 0x18) >> 3) == 2) {
				pushVar(*ptr);
				return (0);
			} else {
				printf
				    ("Unsupported code in opcodeType0 case 1!\n");
				exit(1);
			}

			return (0);
		}
	case 2:
		{
			int16 var_16;
			int di = getByteFromScript();
			int si = getByteFromScript();
			int var_2 = getShortFromScript();

			if (!si) {
				si = currentScriptPtr->overlayNumber;
			}

			if (getSingleObjectParam(si, var_2, di, &var_16)) {
				return -10;
			}

			pushVar(var_16);
			return (0);

			break;
		}
	case 5:
		{
			int byte1 = getByteFromScript();
			int byte2 = getByteFromScript();
			short int short1 = getShortFromScript();

			short int var_12 = short1;
			// short int var_10 = saveOpcodeVar;

			int var_E = byte1 & 7;

			uint8 *ptr = 0;

			if (!var_E) {
				return (-10);
			}

			if (!byte2) {
				ptr = scriptDataPtrTable[var_E] + var_12;
			} else	// TODO: 
			{
				if (!overlayTable[byte2].alreadyLoaded) {
					return (-7);
				}

				if (!overlayTable[byte2].ovlData) {
					return (-4);
				}

				if (var_E == 5) {
					ptr =
					    overlayTable[byte2].ovlData->
					    data4Ptr + var_12;
				} else {
					assert(0);
				}
			}

			if (((byte1 & 0x18) >> 3) == 1) {
				pushVar(loadShort(ptr + saveOpcodeVar * 2));	// TODO: check this !
				return (0);
			} else if (((byte1 & 0x18) >> 3) == 2) {
				pushVar(*(ptr + saveOpcodeVar));
				return (0);
			} else {
				printf
				    ("Unsupported code in opcodeType0 case 1!\n");
				exit(1);
			}

			return (0);
		}
	default:
		{
			printf("Unsupported type %d in opcodeType0\n",
			    currentScriptOpcodeType);
			exit(1);
		}
	}

	return 0;
}

// save opcode
int32 opcodeType1(void)	{
	int var = popVar();
	int offset = 0;

	switch (currentScriptOpcodeType) {
	case 0:
		{
			return (0);	// strange, but happens also in original interpreter
		}
	case 5:
		{
			offset = saveOpcodeVar;
		}
	case 1:
		{
			int var_A = 0;

			int byte1 = getByteFromScript();
			int byte2 = getByteFromScript();

			int short1 = getShortFromScript();

			int var_6 = byte1 & 7;

			int var_C = short1;

			uint8 *ptr = 0;
			int type2;

			if (!var_6)
				return (-10);

			var_C = short1;

			if (byte2) {
				if (!overlayTable[byte2].alreadyLoaded) {
					return (-7);
				}

				if (!overlayTable[byte2].ovlData) {
					return (-4);
				}

				if (var_6 == 5) {
					ptr =
					    overlayTable[byte2].ovlData->
					    data4Ptr + var_C;
				} else {
					ASSERT(0);
				}
			} else {
				ptr = scriptDataPtrTable[var_6] + var_C;
			}

			type2 = ((byte1 & 0x18) >> 3);

			switch (type2) {
			case 1:
				{
					saveShort(ptr + var_A + offset * 2,
					    var);
					return 0;
				}
			case 2:
				{
					*(ptr + var_A + offset) = var;
					return (0);
				}
			default:
				{
					printf
					    ("Unsupported code in opcodeType1 case 1!\n");
					exit(1);
				}
			}

			break;
		}
	case 2:
		{
			int mode = getByteFromScript();
			int di = getByteFromScript();
			int var_4 = getShortFromScript();

			if (!di) {
				di = currentScriptPtr->overlayNumber;
			}

			if (var == 0x85)	// Special case to handle...
			{
				ASSERT(0);
			}

			setObjectPosition(di, var_4, mode, var);

			break;
		}
	case 4:
		{
			saveOpcodeVar = var;
			break;
		}
	default:
		{
			printf("Unsupported type %d in opcodeType1\n",
			    currentScriptOpcodeType);
			exit(1);
		}
	}

	return (0);
}

int32 opcodeType2(void) {
	int offset = saveOpcodeVar;
	int byte1 = getByteFromScript();
	int byte2 = getByteFromScript();
	short int short1 = getShortFromScript();

	ASSERT(currentScriptOpcodeType == 1 || currentScriptOpcodeType == 5);

	if (currentScriptOpcodeType == 5)
		short1 += saveOpcodeVar;

	ASSERT(byte1 & 7);

	if (!(byte1 & 7)) {
		return (-10);
	}

	if (!byte2) {
		int type2;
		uint8 *ptr = scriptDataPtrTable[byte1 & 7] + short1;

		type2 = ((byte1 & 0x18) >> 3);

		ASSERT(type2 == 1 || type2 == 2);

		switch (type2) {
		case 1:
			{
				pushPtr(ptr + offset);
				return (0);
			}
		case 2:
			{
				pushPtr(ptr);
				return (0);
			}
		default:
			{
				return (-10);
			}
		}
	} else {
		printf("Unsupported code in opcodeType2 case 1!\n");
		exit(1);
	}

	return 0;
}

int32 opcodeType10(void) {	// break
	return (0);
}

int32 opcodeType11(void) {	// break
	return (1);
}

int32 opcodeType4(void) {		// test
	int boolVar = 0;

	var1 = popVar();
	var2 = popVar();

	switch (currentScriptOpcodeType) {
	case 0:
		{
			if (var2 != var1)
				boolVar = 1;
			break;
		}
	case 1:
		{
			if (var2 == var1)
				boolVar = 1;
			break;
		}
	case 2:
		{
			if (var2 < var1)
				boolVar = 1;
			break;
		}
	case 3:
		{
			if (var2 <= var1)
				boolVar = 1;
			break;
		}
	case 4:
		{
			if (var2 > var1)
				boolVar = 1;
			break;
		}
	case 5:
		{
			if (var2 >= var1)
				boolVar = 1;
			break;
		}

	}

	pushVar(boolVar);

	return (0);
}

int32 opcodeType6(void) {
	int si = 0;

	int pop = popVar();

	if (!pop)
		si = 1;

	if (pop < 0) {
		si |= 4;
	}

	if (pop > 0) {
		si |= 2;
	}

	currentScriptPtr->bitMask = si;

	return (0);
}

int32 opcodeType7(void) {
	var1 = popVar();
	var2 = popVar();

	pushVar(var1);
	pushVar(var2);

	return (0);
}

int32 opcodeType5(void) {
	int offset = currentScriptPtr->var4;
	int short1 = getShortFromScript();
	int newSi = short1 + offset;
	int bitMask = currentScriptPtr->bitMask;

	switch (currentScriptOpcodeType) {
	case 0:
		{
			if (!(bitMask & 1)) {
				currentScriptPtr->var4 = newSi;
			}
			break;
		}
	case 1:
		{
			if (bitMask & 1) {
				currentScriptPtr->var4 = newSi;
			}
			break;
		}
	case 2:
		{
			if (bitMask & 2) {
				currentScriptPtr->var4 = newSi;
			}
			break;
		}
	case 3:
		{
			if (bitMask & 3) {
				currentScriptPtr->var4 = newSi;
			}
			break;
		}
	case 4:
		{
			if (bitMask & 4) {
				currentScriptPtr->var4 = newSi;
			}
			break;
		}
	case 5:
		{
			if (bitMask & 5) {
				currentScriptPtr->var4 = newSi;
			}
			break;
		}
	case 6:
		{
			break;	// never
		}
	case 7:
		{
			currentScriptPtr->var4 = newSi;	//always
		}
	}

	return (0);
}

int32 opcodeType3(void)	{	// math
	int pop1 = popVar();
	int pop2 = popVar();

	switch (currentScriptOpcodeType) {
	case 0:
		{
			pushVar(pop1 + pop2);
			return (0);
		}
	case 1:
		{
			pushVar(pop1 / pop2);
			return (0);
		}
	case 2:
		{
			pushVar(pop1 - pop2);
			return (0);
		}
	case 3:
		{
			pushVar(pop1 * pop2);
			return (0);
		}
	case 4:
		{
			pushVar(pop1 % pop2);
			return (0);
		}
	case 7:
	case 5:
		{
			pushVar(pop2 | pop1);
			return (0);
		}
	case 6:
		{
			pushVar(pop2 & pop1);
			return (0);
		}
	}

	return 0;
}

int32 opcodeType9(void) {		// stop script
	//printf("Stop a script of overlay %s\n",overlayTable[currentScriptPtr->overlayNumber].overlayName);
	currentScriptPtr->scriptNumber = -1;
	return (1);
}

void setupFuncArray() {
	int i;

	for (i = 0; i < 64; i++) {
		opcodeTypeTable[i] = NULL;
	}

	opcodeTypeTable[1] = opcodeType0;
	opcodeTypeTable[2] = opcodeType1;
	opcodeTypeTable[3] = opcodeType2;
	opcodeTypeTable[4] = opcodeType3;
	opcodeTypeTable[5] = opcodeType4;
	opcodeTypeTable[6] = opcodeType5;
	opcodeTypeTable[7] = opcodeType6;
	opcodeTypeTable[8] = opcodeType7;
	opcodeTypeTable[9] = opcodeType8;
	opcodeTypeTable[10] = opcodeType9;
	opcodeTypeTable[11] = opcodeType10;
	opcodeTypeTable[12] = opcodeType11;
}

int removeScript(int overlay, int idx, scriptInstanceStruct *headPtr) {
	scriptInstanceStruct *scriptPtr;

	scriptPtr = headPtr->nextScriptPtr;

	if (scriptPtr) {
		do {
			if (scriptPtr->overlayNumber == overlay
			    && (scriptPtr->scriptNumber == idx || idx == -1)) {
				scriptPtr->scriptNumber = -1;
			}

			scriptPtr = scriptPtr->nextScriptPtr;
		}
		while (scriptPtr);
	}

	return (0);
}

uint8 *attacheNewScriptToTail(int16 overlayNumber,
		  scriptInstanceStruct *scriptHandlePtr, int16 param, int16 arg0,
		  int16 arg1, int16 arg2, scriptTypeEnum scriptType) {
	int useArg3Neg = 0;
	ovlData3Struct *data3Ptr;
	scriptInstanceStruct *tempPtr;
	int var_C;
	scriptInstanceStruct *oldTail;

	//printf("Starting script %d of overlay %s\n",param,overlayTable[overlayNumber].overlayName);

	if (scriptType < 0) {
		useArg3Neg = 1;
		scriptType = (scriptTypeEnum) - scriptType;
	}

	if (scriptType == 20) {
		data3Ptr = getOvlData3Entry(overlayNumber, param);
	} else {
		if (scriptType == 30) {
			data3Ptr = scriptFunc1Sub2(overlayNumber, param);
		} else {
			return (NULL);
		}
	}

	if (!data3Ptr) {
		return (NULL);
	}

	if (!data3Ptr->dataPtr) {
		return (NULL);
	}

	var_C = data3Ptr->sysKey;

	oldTail = scriptHandlePtr;

	while (oldTail->nextScriptPtr)	// go to the end of the list
	{
		oldTail = oldTail->nextScriptPtr;
	}

	tempPtr =
	    (scriptInstanceStruct *)
	    mallocAndZero(sizeof(scriptInstanceStruct));

	if (!tempPtr)
		return (NULL);

	tempPtr->var6 = NULL;

	if (var_C) {
		tempPtr->var6 = (uint8 *) mallocAndZero(var_C);
	}

	tempPtr->varA = var_C;
	tempPtr->nextScriptPtr = NULL;
	tempPtr->var4 = 0;

	tempPtr->scriptNumber = param;
	tempPtr->overlayNumber = overlayNumber;

	if (scriptType == 20)	// Obj or not ?
	{
		tempPtr->sysKey = useArg3Neg;
	} else {
		tempPtr->sysKey = 1;
	}

	tempPtr->freeze = 0;
	tempPtr->type = scriptType;
	tempPtr->var18 = arg2;
	tempPtr->var16 = arg1;
	tempPtr->var1A = arg0;
	tempPtr->nextScriptPtr = oldTail->nextScriptPtr;	// should always be NULL as it's the tail

	oldTail->nextScriptPtr = tempPtr;	// attache the new node to the list

	return (tempPtr->var6);
}

int executeScripts(scriptInstanceStruct *ptr) {
	int numScript2;
	ovlData3Struct *ptr2;
	ovlDataStruct *ovlData;
	uint8 opcodeType;

	numScript2 = ptr->scriptNumber;

	if (ptr->type == 20) {
		ptr2 = getOvlData3Entry(ptr->overlayNumber, numScript2);

		if (!ptr2) {
			return (-4);
		}
	} else {
		if (ptr->type == 30) {
			ptr2 = scriptFunc1Sub2(ptr->overlayNumber, numScript2);

			if (!ptr2) {
				return (-4);
			}
		} else {
			return (-6);
		}
	}

	if (!overlayTable[ptr->overlayNumber].alreadyLoaded) {
		return (-7);
	}

	ovlData = overlayTable[ptr->overlayNumber].ovlData;

	if (!ovlData)
		return (-4);

	currentData3DataPtr = ptr2->dataPtr;

	scriptDataPtrTable[1] = (uint8 *) ptr->var6;
	scriptDataPtrTable[2] = getDataFromData3(ptr2, 1);
	scriptDataPtrTable[5] = ovlData->data4Ptr;	// free strings
	scriptDataPtrTable[6] = ovlData->ptr8;

	currentScriptPtr = ptr;

	positionInStack = 0;

	do {
		if (currentScriptPtr->var4 == 290
		    && currentScriptPtr->overlayNumber == 4
		    && currentScriptPtr->scriptNumber == 0) {
			currentScriptPtr->var4 = 923;
		}
		opcodeType = getByteFromScript();

		//printf("opType: %d\n",(opcodeType&0xFB)>>3);

		currentScriptOpcodeType = opcodeType & 7;

		if (!opcodeTypeTable[(opcodeType & 0xFB) >> 3]) {
			printf("Unsupported opcode type %d\n",
			    (opcodeType & 0xFB) >> 3);
			exit(1);
			return (-21);
		}
	} while (!opcodeTypeTable[(opcodeType & 0xFB) >> 3] ());

	return (0);
}

void manageScripts(scriptInstanceStruct *scriptHandle) {
	scriptInstanceStruct *ptr = scriptHandle;

	if (ptr) {
		do {
			if (!overlayTable[ptr->overlayNumber].executeScripts) {
				if ((ptr->scriptNumber != -1) && (ptr->freeze == 0) && (ptr->sysKey != 0)) {
					executeScripts(ptr);
				}

				if (ptr->sysKey == 0) {
					ptr->sysKey = 1;
				}
			}

			ptr = ptr->nextScriptPtr;

		} while (ptr);
	}
}

} // End of namespace Cruise
