/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "cruise/cruise_main.h"

namespace Cruise {

#ifdef DUMP_SCRIPT

#define numMaxLignes 100000
#define lineMaxSize 10000

int currentLineType = 0;

struct decompileLineStruct {
	int lineOffset;
	char line[lineMaxSize];
	int indent;
	int type;
	int pendingElse;
};

struct decompileLineStruct decompileLineTable[numMaxLignes];

int positionInDecompileLineTable;

int failed;

char *currentDecompScript;
scriptInstanceStruct dummy;
scriptInstanceStruct *currentDecompScriptPtr = &dummy;

uint8 *getDataFromData3(ovlData3Struct * ptr, int param);

opcodeTypeFunction decompOpcodeTypeTable[64];

int currentLineIdx = 0;

unsigned long int currentOffset;

unsigned long int dumpIdx = 0;

FILE *fHandle = NULL;	// FIXME: Use Common::DumpFile instead of FILE

#define DECOMPILER_STACK_DEPTH 100
#define DECOMPILER_STACK_ENTRY_SIZE 5000

char tempbuffer[5000];

char decompileStack[DECOMPILER_STACK_DEPTH][DECOMPILER_STACK_ENTRY_SIZE];

unsigned long int decompileStackPosition = 0;

uint8 stringName[256];

ovlData3Struct *currentScript;

ovlDataStruct *currentDecompOvl;
int currentDecompScriptIdx;

char decompSaveOpcodeVar[256];

uint8 *getStringNameFromIdx(uint16 stringTypeIdx, char *offset) {
	switch (stringTypeIdx & 7) {
	case 2: {
		Common::sprintf_s(stringName, "\"%s\"",
		        currentScript->dataPtr +
		        currentScript->offsetToSubData3 + atoi(offset));
		break;
	}
	case 5: {
		Common::sprintf_s(stringName, "vars[%s]", offset);
		break;
	}
	default: {
		Common::sprintf_s(stringName, "string[%d][%s]",
		        stringTypeIdx & 7, offset);
		break;
	}
	}

	return stringName;
}

char *resolveMessage(char *messageIdxString) {
	char buffer[500];
	int variable;

	variable = atoi(messageIdxString);
	Common::sprintf_s(buffer, "%d", variable);

	if (strcmp(buffer, messageIdxString)) {
		return messageIdxString;
	} else {
		return currentDecompOvl->stringTable[atoi(messageIdxString)].
		       string;
	}
}

void pushDecomp(char *string, ...) {
	va_list va;

	va_start(va, string);
	Common::vsprintf_s(decompileStack[decompileStackPosition], string, va);
	va_end(va);

	// fprintf(fHandle, "----> %s\n",decompileStack[decompileStackPosition]);

	decompileStackPosition++;
}

void resolveDecompShort(char *buffer) {
	ovlData3Struct *data3Ptr = currentScript;

	importScriptStruct *importEntry =
	    (importScriptStruct *)(data3Ptr->dataPtr +
	                           data3Ptr->offsetToImportData);

	for (int i = 0; i < data3Ptr->numRelocGlob; i++) {
		switch (importEntry->type) {
		case 20:	// script
		case 30:
		case 40:
		case 50: {
			if (importEntry->offset == currentDecompScriptPtr->var4 - 3) {	// param1
				Common::sprintf_s(buffer,
				        data3Ptr->dataPtr +
				        data3Ptr->
				        offsetToImportName +
				        importEntry->offsetToName);
				return;
			}
			if (importEntry->offset == currentDecompScriptPtr->var4 - 6) {	// param2
				Common::sprintf_s(buffer, "linkedIdx");
				return;
			}
			break;
		}
		default: {
			if (importEntry->offset ==
			        currentDecompScriptPtr->var4 - 4) {
				Common::sprintf_s(buffer,
				        data3Ptr->dataPtr +
				        data3Ptr->
				        offsetToImportName +
				        importEntry->offsetToName);
				return;
			}
		}
		}
		importEntry++;
	}

	buffer[0] = 0;

}

void resolveDecompChar(char *buffer) {
	ovlData3Struct *data3Ptr = currentScript;

	importScriptStruct *importEntry =
	    (importScriptStruct *)(data3Ptr->dataPtr +
	                           data3Ptr->offsetToImportData);

	for (int i = 0; i < data3Ptr->numRelocGlob; i++) {
		switch (importEntry->type) {
		default: {
			if (importEntry->offset ==
			        currentDecompScriptPtr->var4 - 2) {
				Common::sprintf_s(buffer,
				        data3Ptr->dataPtr +
				        data3Ptr->
				        offsetToImportName +
				        importEntry->offsetToName);
				return;
			}
		}
		}
		importEntry++;
	}

	buffer[0] = 0;

}

char *popDecomp() {
	//  printf("<----\n");

	if (!decompileStackPosition) {
		return ("");
	}

	decompileStackPosition--;

	return decompileStack[decompileStackPosition];
}

void getByteFromDecompScript(char *buffer) {
	short int var = currentDecompScript[currentDecompScriptPtr->var4];

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 1;

	if (var == -1) {
		resolveDecompChar(buffer);

		if (buffer[0])
			return;
	}

	Common::sprintf_s(buffer, "%d", var);
}

char getByteFromDecompScriptReal() {
	short int var = currentDecompScript[currentDecompScriptPtr->var4];

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 1;

	return var;
}

void getShortFromDecompScript(char *buffer) {
	int16 var = (int16)READ_BE_UINT16(currentDecompScript + currentDecompScriptPtr->var4);

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 2;

	if (var == -1) {
		resolveDecompShort(buffer);

		if (buffer[0])
			return;
	}

	Common::sprintf_s(buffer, "%d", var);
}

int16 getShortFromDecompScriptReal() {
	int16 var = (int16)READ_BE_UINT16(currentDecompScript + currentDecompScriptPtr->var4);

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 2;

	return var;
}

void addDecomp(char *string, ...) {
	va_list va;

	/* fprintf(fHandle,"%d: ",currentLineIdx);
	 *
	 * va_start(va,string);
	 * vfprintf(fHandle,string,va);
	 * va_end(va);
	 *
	 * fprintf(fHandle,"\n"); */

	struct decompileLineStruct *pLineStruct =
				    &decompileLineTable[positionInDecompileLineTable++];

	pLineStruct->lineOffset = currentLineIdx;
	pLineStruct->indent = 0;
	pLineStruct->type = currentLineType;
	pLineStruct->line[0] = 0;
	pLineStruct->pendingElse = 0;

	va_start(va, string);
	Common::vsprintf_s(pLineStruct->line, string, va);
	va_end(va);

	currentLineIdx = currentDecompScriptPtr->var4;
	currentLineType = 0;

	/*printf("%d: ",currentOffset);
	 *
	 * va_start(va,string);
	 * vprintf(string,va);
	 * va_end(va);
	 *
	 * printf("\n"); */
}

void resolveVarName(char *ovlIdxString, int varType, char *varIdxString,
					char *outputName) {
	// outputName is 256 length
	int varIdx = atoi(varIdxString);

	outputName[0] = '\0';

	if (varType == 2) {
		Common::strlcpy(outputName, 256, getStringNameFromIdx(varType,
		                                        varIdxString));
		return;
	}
	if (varType == 1) {
		Common::sprintf_s(outputName, "localVar_%s", varIdxString);
		return;
	}

	if (!strcmp(ovlIdxString, "0")) {
		for (int i = 0; i < currentDecompOvl->numSymbGlob; i++) {
			if (varIdx == currentDecompOvl->arraySymbGlob[i].idx) {
				if (((currentDecompOvl->arraySymbGlob[i].var4 & 0xF0) == 0) && varType != 0x20) {	// var
					strcpy(outputName,
					       currentDecompOvl->arrayNameSymbGlob +
					       currentDecompOvl->arraySymbGlob[i].
					       offsetToName);
					return;
				}
				if ((currentDecompOvl->arraySymbGlob[i].var4) == 20 && varType == 0x20) {	// script
					strcpy(outputName,
					       currentDecompOvl->arrayNameSymbGlob +
					       currentDecompOvl->arraySymbGlob[i].
					       offsetToName);
					return;
				}
			}
		}
		Common::sprintf_s(outputName, "ovl(%s).[%d][%s]", ovlIdxString, varType,
		        varIdxString);
	} else {
		Common::strcpy_s(outputName, 256,  ovlIdxString);
	}
}

int decompLoadVar() {
	switch (currentScriptOpcodeType) {
	case 0: {
		char buffer[256];

		getShortFromDecompScript(buffer);

		pushDecomp(buffer);

		return (0);
	}
	// string
	case 1: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		resolveVarName(buffer2, atoi(buffer1) & 7, buffer3,
		               varName);

		pushDecomp("%s", varName);
		return (0);
	}
	case 2: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		resolveVarName(buffer2, atoi(buffer1) & 7, buffer3,
		               varName);

		pushDecomp("%s", varName);
		return (0);
	}
	case 5: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		resolveVarName(buffer2, atoi(buffer1) & 7, buffer3,
		               varName);

		pushDecomp("%s[%s]", varName, decompSaveOpcodeVar);
		return (0);
	}
	default: {
		debug("Unsupported type %d in opcodeType0",
		       currentScriptOpcodeType);
		failed = 1;
	}
	}
}

int decompSaveVar() {
//      int var = popVar();

	switch (currentScriptOpcodeType) {
	case 0: {
		addDecomp(popDecomp());
		return (0);
	}
	// modify string
	case 1: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];
		uint8 type;

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		type = atoi(buffer1) & 7;

		resolveVarName(buffer2, type, buffer3, varName);

		addDecomp("%s = %s", varName, popDecomp());
		break;
	}
	case 2: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		addDecomp("ovl(%s).setVar(%s,%s) = %s", buffer2,
		          buffer1, buffer3, popDecomp());
		break;
	}
	case 4: {
		Common::strcpy_s(decompSaveOpcodeVar,  popDecomp());
		break;
	}
	case 5: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];
		uint8 type;

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		type = atoi(buffer1) & 7;

		resolveVarName(buffer2, type, buffer3, varName);

		addDecomp("%s[%s] = %s", varName, decompSaveOpcodeVar,
		          popDecomp());
		break;
	}
	default: {
		debug("Unsupported type %d in opcodeType1",
		       currentScriptOpcodeType);
		failed = 1;
	}
	}

	return (0);
}

int decompOpcodeType2() {
	switch (currentScriptOpcodeType) {
	case 1: {
		char buffer3[256];
		char varName[256];
		int byte1 = getByteFromDecompScriptReal();
		getByteFromDecompScriptReal();
		getShortFromDecompScript(buffer3);

		resolveVarName("0", byte1 & 7, buffer3, varName);

		pushDecomp(varName);

		break;
	}
	case 5: {
		int byte1 = getByteFromDecompScriptReal();
		getByteFromDecompScriptReal();
		short int short1 = getShortFromDecompScriptReal();

		int8 *ptr = scriptDataPtrTable[byte1 & 7] + short1;

		if ((byte1 & 7) == 2) {
			pushDecomp("\"%s\"[%s]", ptr,
			           decompSaveOpcodeVar);
		} else if ((byte1 & 7) == 5) {
			pushDecomp("freeString[%d][%s]", short1,
			           decompSaveOpcodeVar);
		} else {
			debug("Unsupported type %d in opcodeType2",
			       byte1 & 7);
			failed = 1;
		}
		break;
	}
	default: {
		debug("Unsupported type %d in opcodeType2",
		       currentScriptOpcodeType);
		failed = 1;
	}
	}
	return (0);
}

int decompMath() {
	char *param1 = popDecomp();
	char *param2 = popDecomp();

	switch (currentScriptOpcodeType) {
	case 0: {
		Common::sprintf_s(tempbuffer, "%s+%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 1: {
		Common::sprintf_s(tempbuffer, "%s/%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 2: {
		Common::sprintf_s(tempbuffer, "%s-%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 3: {
		Common::sprintf_s(tempbuffer, "%s*%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 4: {
		Common::sprintf_s(tempbuffer, "%s % %s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 5:
	case 7: {
		Common::sprintf_s(tempbuffer, "%s|%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 6: {
		Common::sprintf_s(tempbuffer, "%s&%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}

	default: {
		Common::sprintf_s(tempbuffer, "decompMath(%d,%s,%s)",
		        currentScriptOpcodeType, param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	}
	return (0);
}

int decompBoolCompare() {
	char *param1;
	char *param2;

	param1 = popDecomp();
	param2 = popDecomp();

	Common::sprintf_s(tempbuffer, "compare(%s,%s)", param1, param2);
	pushDecomp(tempbuffer);

	return 0;
}

int decompTest() {
	unsigned long int oldOffset = currentDecompScriptPtr->var4;
	short int offset = getShortFromDecompScriptReal();

	switch (currentScriptOpcodeType) {
	case 0: {
		currentLineType = 1;
		addDecomp("test '!(bitMask & 1)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 1: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 1)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 2: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 2)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 3: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 3)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 4: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 4)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 5: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 5)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 6: {
		currentLineType = 2;
		addDecomp("test 'never' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 7: {
		currentLineType = 3;
		addDecomp("goto %d", offset + oldOffset);
		break;
	}

	}

	return 0;
}

int decompCompare() {
	char *param;

	param = popDecomp();

	addDecomp("sign(%s)", param);

	/*
	  if (!pop)
	    si = 1;

	  if (pop<0) {
	    si |= 4;
	  }

	  if (pop>0) {
	    si |= 2;
	  }

	  currentScriptPtr->bitMask = si;
	*/

	return 0;
}

int decompSwapStack() {
	char *stack1;
	char *stack2;
	char buffer1[4000];
	char buffer2[4000];

	stack1 = popDecomp();
	stack2 = popDecomp();

	Common::strcpy_s(buffer1,  stack1);
	Common::strcpy_s(buffer2,  stack2);

	pushDecomp(buffer1);
	pushDecomp(buffer2);

	return 0;
}

int decompFunction() {
	currentScriptOpcodeType = getByteFromDecompScriptReal();
	switch (currentScriptOpcodeType) {
	case 0x1:
		pushDecomp("Op_FadeIn()");
		break;

	case 0x2:
		pushDecomp("Op_FadeOut()");
		break;

	case 0x3:
		Common::sprintf_s(tempbuffer, "Op_loadBackground(%s,%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x4:
		Common::sprintf_s(tempbuffer, "Op_LoadAbs(%s,%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x5:
		Common::sprintf_s(tempbuffer, "Op_AddCell(%s,%s,%s)", popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x6: {
		unsigned long int numArg = atoi(popDecomp());
		char functionName[256];

		char *idxStr = popDecomp();
		char *ovlStr = popDecomp();

		resolveVarName(ovlStr, 0x20, idxStr, functionName);
		Common::sprintf_s(tempbuffer, "Op_AddProc(%s", functionName);

		for (int i = 0; i < numArg; i++) {
			strcatuint8(tempbuffer, ",");
			strcatuint8(tempbuffer, popDecomp());
		}

		strcatuint8(tempbuffer, ")");

		pushDecomp(tempbuffer);
		break;
		}

	case 0x7: {
		char *var1 = popDecomp();
		char *objIdxStr = popDecomp();
		char *ovlStr = popDecomp();

		Common::sprintf_s(tempbuffer, "Op_InitializeState(ovl:%s,dataIdx:%s,%s)", ovlStr, objIdxStr, var1);
		pushDecomp(tempbuffer);
		break;
		}

	case 0x8:
		Common::sprintf_s(tempbuffer, "Op_RemoveCell(%s,%s,%s)", popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x9:
		pushDecomp("Op_FreeCell()");
		break;

	case 0xA:
		Common::sprintf_s(tempbuffer, "Op_RemoveProc(ovl(%s),%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0xB:
		Common::sprintf_s(tempbuffer, "Op_RemoveFrame(%s,%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0xC:
		Common::sprintf_s(tempbuffer, "Op_LoadOverlay(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0xD:
		Common::sprintf_s(tempbuffer, "Op_SetColor(%s,%s,%s,%s,%s)", popDecomp(), popDecomp(), popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0xE:
		Common::sprintf_s(tempbuffer, "Op_PlayFX(%s,%s,%s,%s)", popDecomp(), popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x10:
		Common::sprintf_s(tempbuffer, "Op_FreeOverlay(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x11:
		Common::sprintf_s(tempbuffer, "Op_FindOverlay(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x13:
		Common::sprintf_s(tempbuffer, "Op_AddMessage(%s,\"%s\",%s,%s,%s,%s)", popDecomp(),
			resolveMessage(popDecomp()), popDecomp(),  popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x14:
		Common::sprintf_s(tempbuffer, "Op_RemoveMessage(ovl(%s),%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x15:
		pushDecomp("Op_UserWait()");
		break;

	case 0x16:
		Common::sprintf_s(tempbuffer, "Op_FreezeCell(%s,%s,%s,%s,%s,%s)", popDecomp(), popDecomp(),
			popDecomp(), popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x17:
		Common::sprintf_s(tempbuffer, "Op_LoadCt(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x18:
		Common::sprintf_s(tempbuffer, "Op_AddAnimation(%s,%s,%s,%s,%s,%s,%s)", popDecomp(), popDecomp(),
			popDecomp(), popDecomp(), popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x19:
		Common::sprintf_s(tempbuffer, "Op_RemoveAnimation(%s,%s,%s)", popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x1A:
		Common::sprintf_s(tempbuffer, "Op_SetZoom(%s,%s,%s,%s)", popDecomp(), popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x1E:
		Common::sprintf_s(tempbuffer, "Op_TrackAnim(%s,%s,%s,%s,%s,%s)", popDecomp(), popDecomp(),
			popDecomp(), popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x21:
		Common::sprintf_s(tempbuffer, "Op_EndAnim(%s,%s,%s)", popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x22:
		Common::sprintf_s(tempbuffer, "Op_GetZoom(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x23:
		Common::sprintf_s(tempbuffer, "Op_GetStep(%s,%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x24:
		Common::sprintf_s(tempbuffer, "Op_SetStringColors(%s,%s,%s,%s)", popDecomp(), popDecomp(),
			popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x27:
		Common::sprintf_s(tempbuffer, "Op_getPixel(%s,%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x28:
		Common::sprintf_s(tempbuffer, "Op_UserOn(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x29:
		pushDecomp("Op_FreeCT()");
		break;

	case 0x2B:
		Common::sprintf_s(tempbuffer, "Op_FindProc(%s,%s)", popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x2C:
		Common::sprintf_s(tempbuffer, "Op_WriteObject(%s,%s,%s)", popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x2E:
		Common::sprintf_s(tempbuffer, "Op_RemoveOverlay(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x2F:
		Common::sprintf_s(tempbuffer, "Op_AddBackgroundIncrust(%s,%s,%s)", popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;

	case 0x30: {
		Common::sprintf_s(tempbuffer, "_removeBackgroundIncrust(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x31: {
		Common::sprintf_s(tempbuffer, "_op_31(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x32: {
		pushDecomp("_freeBackgroundInscrustList()");
		break;
	}
	case 0x35: {
		Common::sprintf_s(tempbuffer, "_op35(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x37: {
		Common::sprintf_s(tempbuffer, "_op37(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x38: {
		Common::sprintf_s(tempbuffer, "_removeBackground(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x39: {
		Common::sprintf_s(tempbuffer, "_SetActiveBackgroundPlane(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3A: {
		Common::sprintf_s(tempbuffer, "_setVar49(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3B: {
		pushDecomp("_op3B()");
		break;
	}
	case 0x3C: {
		Common::sprintf_s(tempbuffer, "_rand(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3D: {
		Common::sprintf_s(tempbuffer, "_loadMusic(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3E: {
		pushDecomp("_op_3E()");
		break;
	}
	case 0x3F: {
		pushDecomp("_op_3F()");
		break;
	}
	case 0x40: {
		pushDecomp("_op_40()");
		break;
	}
	case 0x41: {
		Common::sprintf_s(tempbuffer, "_isFileLoaded2(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x45: {
		pushDecomp("_stopSound()");
		break;
	}
	case 0x49: {
		Common::sprintf_s(tempbuffer, "_op49(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x54: {
		Common::sprintf_s(tempbuffer, "_setFontVar(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x56: {
		Common::sprintf_s(tempbuffer, "_changeCutSceneState(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x57: {
		pushDecomp("_getMouseX()");
		break;
	}
	case 0x58: {
		pushDecomp("_getMouseY()");
		break;
	}
	case 0x59: {
		pushDecomp("_getMouse3()");
		break;
	}
	case 0x5A: {
		Common::sprintf_s(tempbuffer, "_isFileLoaded(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x5B: {
		pushDecomp("_regenerateBackgroundIncrust()");
		break;
	}
	case 0x5C: {
		Common::sprintf_s(tempbuffer, "_Op_AddCellC(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x5E: {
		Common::sprintf_s(tempbuffer, "_Op_AddCellE(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x60: {
		Common::sprintf_s(tempbuffer, "_op_60(%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x61: {
		Common::sprintf_s(tempbuffer, "_op61(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x62: {
		pushDecomp("_pauseCallingScript()");
		break;
	}
	case 0x63: {
		pushDecomp("_resumeScript()");
		break;
	}
	case 0x64: {
		unsigned long int numArg = atoi(popDecomp());
		char *ovlStr;
		char *idxStr;
		char functionName[256];

		idxStr = popDecomp();
		ovlStr = popDecomp();

		resolveVarName(ovlStr, 0x20, idxStr, functionName);

		Common::sprintf_s(tempbuffer, "%s(", functionName);

		for (int i = 0; i < numArg; i++) {
			if (i)
				strcatuint8(tempbuffer, ",");
			strcatuint8(tempbuffer, popDecomp());
		}

		strcatuint8(tempbuffer, ")");

		pushDecomp(tempbuffer);
		break;
	}
	case 0x65: {
		Common::sprintf_s(tempbuffer,
		        "_addWaitObject(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x66: {
		Common::sprintf_s(tempbuffer, "_op_66(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x67: {
		Common::sprintf_s(tempbuffer, "_loadAudioResource(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x68: {
		pushDecomp("_freeMediumVar()");
		break;
	}
	case 0x6A: {
		Common::sprintf_s(tempbuffer, "_op_6A(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6B: {
		Common::sprintf_s(tempbuffer, "_loadData(%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6C: {
		Common::sprintf_s(tempbuffer, "_op_6C(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6D: {
		Common::sprintf_s(tempbuffer, "_strcpy(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6E: {
		Common::sprintf_s(tempbuffer, "_op_6E(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6F: {
		unsigned long int numArg = atoi(popDecomp());
		char *ovlStr;
		char *idxStr;

		idxStr = popDecomp();
		ovlStr = popDecomp();

		Common::sprintf_s(tempbuffer, "_op_6F(%s,%s", idxStr, ovlStr);

		for (int i = 0; i < numArg; i++) {
			strcatuint8(tempbuffer, ",");
			strcatuint8(tempbuffer, popDecomp());
		}

		strcatuint8(tempbuffer, ")");

		pushDecomp(tempbuffer);
		break;
	}
	case 0x70: {
		Common::sprintf_s(tempbuffer, "_comment(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x71: {
		Common::sprintf_s(tempbuffer, "_op71(%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x72: {
		Common::sprintf_s(tempbuffer, "_op72(%s,%s)", popDecomp(),
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x73: {
		Common::sprintf_s(tempbuffer, "_op73(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x74: {
		Common::sprintf_s(tempbuffer, "_getlowMemory()");
		pushDecomp(tempbuffer);
		break;
	}
	case 0x76: {
		Common::sprintf_s(tempbuffer, "_Op_InitializeState6(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x77: {
		Common::sprintf_s(tempbuffer, "_Op_InitializeState7(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x78: {
		Common::sprintf_s(tempbuffer, "_Op_InitializeState8(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x79: {
		Common::sprintf_s(tempbuffer, "_EnterPlayerMenu(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x7B: {
		Common::sprintf_s(tempbuffer, "_Op_InitializeStateB(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x7C: {
		Common::sprintf_s(tempbuffer, "_Op_InitializeStateC(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x7D: {
		pushDecomp("_freeAllMenu()");
		break;
	}
	default: {
		addDecomp("OP_%X", currentScriptOpcodeType);
		debug("OPCODE: %X", currentScriptOpcodeType);
		failed = 1;
		break;
	}
	}

//    pushDecomp("functionDummyPush");

	return (0);
}

uint8 stop = 0;

int decompStop() {
	stop = 1;
	addDecomp("stop\n");
	return 0;
}

int decompBreak() {
	addDecomp("break");
	return 0;
}

void generateIndentation() {
	for (int i = 0; i < positionInDecompileLineTable; i++) {
		if (decompileLineTable[i].type != 0) {
			char *gotoStatement;
			int destLine;
			int destLineIdx;

			gotoStatement =
			    strstr(decompileLineTable[i].line, "goto");
			assert(gotoStatement);
			gotoStatement = strchr(gotoStatement, ' ') + 1;

			destLine = atoi(gotoStatement);
			destLineIdx = -1;

			for (int j = 0; j < positionInDecompileLineTable; j++) {
				if (decompileLineTable[j].lineOffset == destLine) {
					destLineIdx = j;
					break;
				}
			}

			assert(destLineIdx != -1);

			if (destLineIdx > i) {
				for (int j = i + 1; j < destLineIdx; j++) {
					decompileLineTable[j].indent++;
				}

				if (strstr(decompileLineTable[destLineIdx - 1].line, "goto") ==
				        decompileLineTable[destLineIdx - 1].line) {
					//decompileLineTable[destLineIdx-1].pendingElse = 1;
				}
			}
		}
	}
}

void dumpScript(uint8 *ovlName, ovlDataStruct *ovlData, int idx) {
	uint8 opcodeType;
	char buffer[256];

	char temp[256];
	char scriptName[256];

	Common::sprintf_s(temp, "%d", idx);

	failed = 0;

	currentScript = &ovlData->arrayProc[idx];

	currentDecompScript = currentScript->dataPtr;
	currentDecompScriptPtr->var4 = 0;

	currentDecompOvl = ovlData;
	currentDecompScriptIdx = idx;

	currentLineIdx = 0;
	positionInDecompileLineTable = 0;
	currentLineType = 0;

	resolveVarName("0", 0x20, temp, scriptName);

	debug("decompiling script %d - %s", idx, scriptName);

	// return;

//      scriptDataPtrTable[1] = *(char**)(ptr+0x6);
	scriptDataPtrTable[2] = getDataFromData3(currentScript, 1);	// strings
	scriptDataPtrTable[5] = ovlData->data4Ptr;	// free strings
	scriptDataPtrTable[6] = ovlData->ptr8;

	stop = 0;

	Common::sprintf_s(buffer, "%s-%02d-%s.txt", ovlName, idx, scriptName);
	fHandle = fopen(buffer, "w+");

	decompileStackPosition = 0;

	for (int i = 0; i < 64; i++)
		decompOpcodeTypeTable[i] = NULL;

	decompOpcodeTypeTable[1] = decompLoadVar;
	decompOpcodeTypeTable[2] = decompSaveVar;
	decompOpcodeTypeTable[3] = decompOpcodeType2;
	decompOpcodeTypeTable[4] = decompMath;
	decompOpcodeTypeTable[5] = decompBoolCompare;
	decompOpcodeTypeTable[6] = decompTest;
	decompOpcodeTypeTable[7] = decompCompare;
	decompOpcodeTypeTable[8] = decompSwapStack;
	decompOpcodeTypeTable[9] = decompFunction;
	decompOpcodeTypeTable[10] = decompStop;
	decompOpcodeTypeTable[12] = decompBreak;

	do {
		currentOffset = currentDecompScriptPtr->var4;

		opcodeType = getByteFromDecompScriptReal();

		currentScriptOpcodeType = opcodeType & 7;

		if (!decompOpcodeTypeTable[(opcodeType & 0xFB) >> 3]) {
			debug("Unsupported opcode type %d in decomp",
			       (opcodeType & 0xFB) >> 3);
			return;
		}

		//debug("Optype: %d",(opcodeType&0xFB)>>3);

		decompOpcodeTypeTable[(opcodeType & 0xFB) >> 3]();

		if (failed) {
			debug("Aborting decompilation..");
			fclose(fHandle);
			return;
		}

	} while (!stop);

	dumpIdx++;

	generateIndentation();

	for (int i = 0; i < positionInDecompileLineTable; i++) {
		if (decompileLineTable[i].pendingElse) {
			fprintf(fHandle, "%05d:\t",
			        decompileLineTable[i].lineOffset);
			fprintf(fHandle, "else %s\n", decompileLineTable[i].line);
		}

		fprintf(fHandle, "%05d:\t", decompileLineTable[i].lineOffset);
		for (int j = 0; j < decompileLineTable[i].indent; j++)
			fprintf(fHandle, "\t");

		fprintf(fHandle, "%s\n", decompileLineTable[i].line);
	}

	fclose(fHandle);
}

#endif

} // End of namespace Cruise
