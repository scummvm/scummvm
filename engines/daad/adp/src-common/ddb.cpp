#include <ddb.h>
#include <ddb_condact_defs.h>
#include <ddb_paw.h>
#include <ddb_xmsg.h>
#include <ddb_vid.h>
#include <dmg.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>
#include <os_bito.h>

#if _STDCLIB
#include <stdio.h>
#include <stdarg.h>
#endif

DDB_Interpreter*    interpreter;

static DDB_Error 	ddbError = DDB_ERROR_NONE;
static void 		(*warningHandler)(const char* message) = 0;

#if HAS_PSG
void DDB_DetectPSGExternalTable(DDB* ddb, uint32_t firstSectionOffset);
#endif

static uint16_t DDB_GetBaseOffsetForMachine(DDB_Machine target)
{
	switch (target)
	{
		case DDB_MACHINE_SPECTRUM: return 0x8400;
		case DDB_MACHINE_C64:      return 0x3880;
		case DDB_MACHINE_CPC:      return 0x2880;
		case DDB_MACHINE_MSX:      return 0x0100;
		case DDB_MACHINE_PCW:      return 0x0100;
		case DDB_MACHINE_PLUS4:    return 0x7080;
		default:                   return 0;
	}
}

static uint32_t DDB_GetMinimumHeaderSize(DDB_Version version)
{
	return version == DDB_VERSION_1 ? 34 : 36;
}

static uint32_t DDB_GetExternFieldOffset(DDB_Version version)
{
	return version >= DDB_VERSION_2 ? 34 : 32;
}

static uint32_t DDB_GetFirstSectionOffset(const DDB* ddb,
	uint16_t tokensOffset,
	uint16_t processTableOffset,
	uint16_t objNamTableOffset,
	uint16_t locDescTableOffset,
	uint16_t msgTableOffset,
	uint16_t sysMsgTableOffset,
	uint16_t conTableOffset,
	uint16_t vocabularyOffset,
	uint16_t objLocTableOffset,
	uint16_t objWordsTableOffset,
	uint16_t objAttrTableOffset,
	uint16_t objExAttrTableOffset)
{
	uint16_t rawOffsets[] = {
		tokensOffset,
		processTableOffset,
		objNamTableOffset,
		locDescTableOffset,
		msgTableOffset,
		sysMsgTableOffset,
		conTableOffset,
		vocabularyOffset,
		objLocTableOffset,
		objWordsTableOffset,
		objAttrTableOffset,
		objExAttrTableOffset,
	};

	uint32_t firstOffset = ddb->dataSize;
	for (size_t n = 0; n < sizeof(rawOffsets) / sizeof(rawOffsets[0]); n++)
	{
		uint16_t rawOffset = rawOffsets[n];
		uint32_t offset = 0;
		if (!DDB_DecodeStoredOffset(ddb, rawOffset, ddb->dataSize, true, &offset) || offset == 0)
			continue;
		if (offset < firstOffset)
			firstOffset = offset;
	}
	return firstOffset;
}

static bool DDB_IsHeaderOffsetValid(const DDB* ddb, uint16_t rawOffset, uint32_t minimumSize, const char* name)
{
	uint32_t offset = 0;
	if (!DDB_DecodeStoredOffset(ddb, rawOffset, ddb->dataSize, false, &offset))
	{
		DDB_Warning("Invalid DDB header: %s offset 0x%04X outside file", name, rawOffset);
		ddbError = DDB_ERROR_INVALID_FILE;
		return false;
	}

	if (offset > ddb->dataSize || (minimumSize != 0 && (offset >= ddb->dataSize || minimumSize > ddb->dataSize - offset)))
	{
		DDB_Warning("Invalid DDB header: %s offset 0x%04X outside file", name, rawOffset);
		ddbError = DDB_ERROR_INVALID_FILE;
		return false;
	}

	return true;
}

static uint32_t DDB_NormalizeDeclaredSize(const DDB* ddb, uint16_t declaredSize)
{
	if (ddb->baseOffset != 0 && declaredSize >= ddb->baseOffset)
		return declaredSize - ddb->baseOffset;
	return declaredSize;
}

static uint32_t DDB_GetLogicalSizeForHeaderGuess(const DDB* ddb, uint16_t declaredSize)
{
	if (declaredSize == 0)
		return ddb->dataSize;
	return DDB_NormalizeDeclaredSize(ddb, declaredSize);
}

static bool DDB_HasOnlyPaddingBytes(const uint8_t* data, uint32_t dataSize, uint32_t logicalSize)
{
	if (logicalSize > dataSize)
		return false;

	for (uint32_t n = logicalSize; n < dataSize; n++)
	{
		if (data[n] != 0x00 && data[n] != 0xE5)
			return false;
	}
	return true;
}

static bool DDB_IsPlausibleHeaderLayout(const DDB* ddb, bool littleEndian, uint16_t declaredSize)
{
	uint32_t logicalSize = DDB_GetLogicalSizeForHeaderGuess(ddb, declaredSize);
	uint32_t minimumHeaderSize = DDB_GetMinimumHeaderSize(ddb->version);
	if (logicalSize < minimumHeaderSize || logicalSize > ddb->dataSize)
		return false;

	for (int n = 8; n < 30; n += 2)
	{
		uint16_t rawOffset = read16(ddb->data + n, littleEndian);
		uint32_t offset = 0;
		if (n == 8 && rawOffset == 0)
			continue;

		if (!DDB_DecodeStoredOffset(ddb, rawOffset, logicalSize, false, &offset))
			return false;
	}

	return true;
}

static bool DDB_ParseHeader(DDB* ddb, uint8_t* data, uint32_t dataSize, uint32_t loadedSize)
{
	ddb->data = data;
	ddb->dataSize = dataSize;
	ddb->version = (DDB_Version)data[0];
	ddb->language = (DDB_Language)(data[1] & 0x0F);
	ddb->target = (DDB_Machine)(data[1] >> 4);
	// drb.php emits target 0x0D for the new PC VGA256 interpreter.
	// The native SDL runtime needs to treat that target as IBM PC so it
	// enables DAT loading and PC intro-screen handling.
	if ((uint8_t)ddb->target == 0x0D)
		ddb->target = DDB_MACHINE_IBMPC;
	ddb->nullWordChar = data[2];
	ddb->firstToken = 128;

	if (ddb->baseOffset == 0)
		ddb->baseOffset = DDB_GetBaseOffsetForMachine(ddb->target);

	if (ddb->version > DDB_VERSION_3)
	{
		DDB_Warning("Invalid DDB header: unsupported version %d", ddb->version);
		ddbError = DDB_ERROR_INVALID_FILE;
		return false;
	}

	uint32_t minimumHeaderSize = DDB_GetMinimumHeaderSize(ddb->version);
	if (ddb->dataSize < minimumHeaderSize || loadedSize < minimumHeaderSize)
	{
		DDB_Warning("Invalid DDB header: file too small (%u bytes)", (unsigned)ddb->dataSize);
		ddbError = DDB_ERROR_INVALID_FILE;
		return false;
	}

	uint16_t declaredSizeBE = read16(data + (ddb->version == 1 ? 30 : 32), false);
	uint16_t declaredSizeLE = read16(data + (ddb->version == 1 ? 30 : 32), true);
	uint32_t declaredLogicalSizeBE = DDB_GetLogicalSizeForHeaderGuess(ddb, declaredSizeBE);
	uint32_t declaredLogicalSizeLE = DDB_GetLogicalSizeForHeaderGuess(ddb, declaredSizeLE);
	bool bigEndianValid = DDB_IsPlausibleHeaderLayout(ddb, false, declaredSizeBE);
	bool littleEndianValid = DDB_IsPlausibleHeaderLayout(ddb, true, declaredSizeLE);
	bool bigEndianMatch = bigEndianValid && declaredLogicalSizeBE == ddb->dataSize;
	bool littleEndianMatch = littleEndianValid && declaredLogicalSizeLE == ddb->dataSize;

	if (bigEndianMatch && !littleEndianMatch)
		ddb->littleEndian = false;
	else if (!bigEndianMatch && littleEndianMatch)
		ddb->littleEndian = true;
	else if (bigEndianValid && !littleEndianValid)
		ddb->littleEndian = false;
	else if (!bigEndianValid && littleEndianValid)
		ddb->littleEndian = true;
	else
	{
		DDB_Warning("Invalid DDB header: unable to guess endianess");
		ddbError = DDB_ERROR_INVALID_FILE;
		return false;
	}

	uint32_t declaredLogicalSize = ddb->littleEndian ? declaredLogicalSizeLE : declaredLogicalSizeBE;
	if (loadedSize == dataSize &&
		declaredLogicalSize >= minimumHeaderSize &&
		declaredLogicalSize < ddb->dataSize &&
		DDB_HasOnlyPaddingBytes(data, ddb->dataSize, declaredLogicalSize))
	{
		ddb->dataSize = declaredLogicalSize;
	}

	ddb->numObjects = data[3];
	ddb->numLocations = data[4];
	ddb->numMessages = data[5];
	ddb->numSystemMessages = data[6];
	ddb->numProcesses = data[7];

	uint16_t tokensOffset = read16(data + 8, ddb->littleEndian);
	uint16_t processTableOffset = read16(data + 10, ddb->littleEndian);
	uint16_t objNamTableOffset = read16(data + 12, ddb->littleEndian);
	uint16_t locDescTableOffset = read16(data + 14, ddb->littleEndian);
	uint16_t msgTableOffset = read16(data + 16, ddb->littleEndian);
	uint16_t sysMsgTableOffset = read16(data + 18, ddb->littleEndian);
	uint16_t conTableOffset = read16(data + 20, ddb->littleEndian);
	uint16_t vocabularyOffset = read16(data + 22, ddb->littleEndian);
	uint16_t objLocTableOffset = read16(data + 24, ddb->littleEndian);
	uint16_t objWordsTableOffset = read16(data + 26, ddb->littleEndian);
	uint16_t objAttrTableOffset = read16(data + 28, ddb->littleEndian);
	uint32_t tokensFileOffset = 0;
	uint32_t processTableFileOffset = 0;
	uint32_t objNamTableFileOffset = 0;
	uint32_t locDescTableFileOffset = 0;
	uint32_t msgTableFileOffset = 0;
	uint32_t sysMsgTableFileOffset = 0;
	uint32_t conTableFileOffset = 0;
	uint32_t vocabularyFileOffset = 0;
	uint32_t objLocTableFileOffset = 0;
	uint32_t objWordsTableFileOffset = 0;
	uint32_t objAttrTableFileOffset = 0;

	if ((tokensOffset != 0 && !DDB_IsHeaderOffsetValid(ddb, tokensOffset, 1, "token block")) ||
		!DDB_IsHeaderOffsetValid(ddb, processTableOffset, ddb->numProcesses * 2, "process table") ||
		!DDB_IsHeaderOffsetValid(ddb, objNamTableOffset, ddb->numObjects * 2, "object names table") ||
		!DDB_IsHeaderOffsetValid(ddb, locDescTableOffset, ddb->numLocations * 2, "location descriptions table") ||
		!DDB_IsHeaderOffsetValid(ddb, msgTableOffset, ddb->numMessages * 2, "messages table") ||
		!DDB_IsHeaderOffsetValid(ddb, sysMsgTableOffset, ddb->numSystemMessages * 2, "system messages table") ||
		!DDB_IsHeaderOffsetValid(ddb, conTableOffset, ddb->numLocations * 2, "connections table") ||
		!DDB_IsHeaderOffsetValid(ddb, vocabularyOffset, 1, "vocabulary") ||
		!DDB_IsHeaderOffsetValid(ddb, objLocTableOffset, ddb->numObjects, "object locations table") ||
		!DDB_IsHeaderOffsetValid(ddb, objWordsTableOffset, ddb->numObjects, "object words table") ||
		!DDB_IsHeaderOffsetValid(ddb, objAttrTableOffset, ddb->numObjects, "object attributes table"))
		return false;

	if ((tokensOffset != 0 && !DDB_DecodeStoredOffset(ddb, tokensOffset, ddb->dataSize, false, &tokensFileOffset)) ||
		!DDB_DecodeStoredOffset(ddb, processTableOffset, ddb->dataSize, false, &processTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, objNamTableOffset, ddb->dataSize, false, &objNamTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, locDescTableOffset, ddb->dataSize, false, &locDescTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, msgTableOffset, ddb->dataSize, false, &msgTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, sysMsgTableOffset, ddb->dataSize, false, &sysMsgTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, conTableOffset, ddb->dataSize, false, &conTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, vocabularyOffset, ddb->dataSize, false, &vocabularyFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, objLocTableOffset, ddb->dataSize, false, &objLocTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, objWordsTableOffset, ddb->dataSize, false, &objWordsTableFileOffset) ||
		!DDB_DecodeStoredOffset(ddb, objAttrTableOffset, ddb->dataSize, false, &objAttrTableFileOffset))
		return false;

	ddb->tokens = tokensOffset == 0 ? data : data + tokensFileOffset;
	ddb->processTable = (uint16_t*)(data + processTableFileOffset);
	ddb->objNamTable = (uint16_t*)(data + objNamTableFileOffset);
	ddb->locDescTable = (uint16_t*)(data + locDescTableFileOffset);
	ddb->msgTable = (uint16_t*)(data + msgTableFileOffset);
	ddb->sysMsgTable = (uint16_t*)(data + sysMsgTableFileOffset);
	ddb->conTable = (uint16_t*)(data + conTableFileOffset);
	ddb->vocabulary = data + vocabularyFileOffset;
	ddb->objLocTable = data + objLocTableFileOffset;
	ddb->objWordsTable = data + objWordsTableFileOffset;
	ddb->objAttrTable = data + objAttrTableFileOffset;

	if (ddb->version >= 2)
	{
		uint16_t objExAttrTableOffset = read16(data + 30, ddb->littleEndian);
		uint32_t objExAttrFileOffset = 0;
		if (!DDB_IsHeaderOffsetValid(ddb, objExAttrTableOffset, ddb->numObjects * 2, "extended object attributes table"))
			return false;
		if (!DDB_DecodeStoredOffset(ddb, objExAttrTableOffset, ddb->dataSize, false, &objExAttrFileOffset))
			return false;

		ddb->objExAttrTable = (uint16_t*)(data + objExAttrFileOffset);
		uint32_t objExAttrOffset = objExAttrFileOffset;
		if (objExAttrOffset < loadedSize && ddb->numObjects * 2 <= loadedSize - objExAttrOffset)
		{
			for (int n = 0; n < ddb->numObjects; n++)
				ddb->objExAttrTable[n] = read16((const uint8_t*)&ddb->objExAttrTable[n], !ddb->littleEndian);
		}
	}

	uint16_t objExAttrTableOffset = ddb->version >= 2 ? read16(data + 30, ddb->littleEndian) : 0;
	uint32_t firstSectionOffset = DDB_GetFirstSectionOffset(ddb,
		tokensOffset,
		processTableOffset,
		objNamTableOffset,
		locDescTableOffset,
		msgTableOffset,
		sysMsgTableOffset,
		conTableOffset,
		vocabularyOffset,
		objLocTableOffset,
		objWordsTableOffset,
		objAttrTableOffset,
		objExAttrTableOffset);
	uint32_t externFieldOffset = DDB_GetExternFieldOffset(ddb->version);
	bool externFieldFits = externFieldOffset + 1 < firstSectionOffset && externFieldOffset + 1 < ddb->dataSize;
	if (externFieldFits)
	{
		uint16_t externOffset = read16(data + externFieldOffset, ddb->littleEndian);
		uint32_t externFileOffset = 0;
		if (externOffset != 0)
		{
			// An extern pointer outside the database is not fatal: in memory
			// snapshots it may point at data loaded elsewhere in RAM, which is
			// simply unavailable here
			if (DDB_DecodeStoredOffset(ddb, externOffset, ddb->dataSize, false, &externFileOffset) &&
			    externFileOffset < ddb->dataSize)
				ddb->externData = data + externFileOffset;
		}
	}

	#if HAS_PSG
	DDB_DetectPSGExternalTable(ddb, firstSectionOffset);
	#else
	ddb->externPsgTable = 0;
	ddb->externPsgCount = 0;
	#endif

	return true;
}

static bool DDB_ValidateOffsetTablePrefix(const DDB* ddb, const uint8_t* data, uint32_t loadedSize, uint16_t tableOffset, uint8_t entries, const char* tableName)
{
	if (entries == 0)
		return true;

	uint32_t minimumHeaderSize = DDB_GetMinimumHeaderSize(ddb->version);

	uint32_t offset = 0;
	if (!DDB_DecodeStoredOffset(ddb, tableOffset, ddb->dataSize, false, &offset))
	{
		ddbError = DDB_ERROR_INVALID_FILE;
		DDB_Warning("Invalid table offset 0x%04X in %s", tableOffset, tableName);
		return false;
	}

	uint32_t tableSize = entries * 2;
	if (offset >= loadedSize || tableSize > loadedSize - offset)
		return true;

	const uint8_t* table = data + offset;
	for (int n = 0; n < entries; n++)
	{
		uint16_t rawOffset = read16(table + n * 2, ddb->littleEndian);
		uint32_t relativeOffset = 0;
		if (!DDB_DecodeStoredOffset(ddb, rawOffset, ddb->dataSize, false, &relativeOffset))
		{
			ddbError = DDB_ERROR_INVALID_FILE;
			DDB_Warning("Invalid internal offset 0x%04X (entry %d in %s)", rawOffset, n, tableName);
			return false;
		}

		if (relativeOffset >= ddb->dataSize || relativeOffset < minimumHeaderSize)
		{
			ddbError = DDB_ERROR_INVALID_FILE;
			DDB_Warning("Invalid internal offset 0x%04X (entry %d in %s)", relativeOffset, n, tableName);
			return false;
		}
	}

	return true;
}

#define DDB_CONDACT_MAP_ENTRY(opcode, condact, parameters) { condact, parameters },
static DDB_CondactMap version1Condacts[128] = {
	DDB_VERSION1_CONDACTS(DDB_CONDACT_MAP_ENTRY)
};
#undef DDB_CONDACT_MAP_ENTRY

static DDB_CondactMap version2Condacts[128] = {
	{ CONDACT_AT,     		  1 },		// 0x00
	{ CONDACT_NOTAT,  		  1 },		// 0x01
	{ CONDACT_ATGT,           1 },		// 0x02
	{ CONDACT_ATLT,   		  1 },		// 0x03
	{ CONDACT_PRESENT,		  1 },		// 0x04
	{ CONDACT_ABSENT, 		  1 },		// 0x05
	{ CONDACT_WORN,   		  1 },		// 0x06
	{ CONDACT_NOTWORN,		  1 },		// 0x07
	{ CONDACT_CARRIED,		  1 },		// 0x08
	{ CONDACT_NOTCARR,		  1 },		// 0x09
	{ CONDACT_CHANCE, 		  1 },		// 0x0A
	{ CONDACT_ZERO,   		  1 },		// 0x0B
	{ CONDACT_NOTZERO,		  1 },		// 0x0C
	{ CONDACT_EQ,     		  2 },		// 0x0D
	{ CONDACT_GT,     		  2 },		// 0x0E
	{ CONDACT_LT,     		  2 },		// 0x0F
	{ CONDACT_ADJECT1,		  1 },		// 0x10
	{ CONDACT_ADVERB, 		  1 },		// 0x11
	{ CONDACT_SFX,    		  2 },		// 0x12
	{ CONDACT_DESC,   		  1 },		// 0x13
	{ CONDACT_QUIT,   		  0 },		// 0x14
	{ CONDACT_END,    		  0 },		// 0x15
	{ CONDACT_DONE,   		  0 },		// 0x16
	{ CONDACT_OK,     		  0 },		// 0x17
	{ CONDACT_ANYKEY, 		  0 },		// 0x18
	{ CONDACT_SAVE,   		  1 },		// 0x19
	{ CONDACT_LOAD,   		  1 },		// 0x1A
	{ CONDACT_DPRINT,   	  1 },		// 0x1B
	{ CONDACT_DISPLAY,  	  1 },		// 0x1C
	{ CONDACT_CLS,    		  0 },		// 0x1D
	{ CONDACT_DROPALL,		  0 },		// 0x1E
	{ CONDACT_AUTOG,  		  0 },		// 0x1F
	{ CONDACT_AUTOD,  		  0 },		// 0x20
	{ CONDACT_AUTOW,  		  0 },		// 0x21
	{ CONDACT_AUTOR,  		  0 },		// 0x22
	{ CONDACT_PAUSE,  		  1 },		// 0x23
	{ CONDACT_SYNONYM, 	 	  2 },		// 0x24
	{ CONDACT_GOTO,   		  1 },		// 0x25
	{ CONDACT_MESSAGE,		  1 },		// 0x26
	{ CONDACT_REMOVE, 		  1 },		// 0x27
	{ CONDACT_GET,    		  1 },		// 0x28
	{ CONDACT_DROP,   		  1 },		// 0x29
	{ CONDACT_WEAR,   		  1 },		// 0x2A
	{ CONDACT_DESTROY,		  1 },		// 0x2B
	{ CONDACT_CREATE, 		  1 },		// 0x2C
	{ CONDACT_SWAP,   		  2 },		// 0x2D
	{ CONDACT_PLACE,  		  2 },		// 0x2E
	{ CONDACT_SET,    		  1 },		// 0x2F
	{ CONDACT_CLEAR,  		  1 },		// 0x30
	{ CONDACT_PLUS,   		  2 },		// 0x31
	{ CONDACT_MINUS,  		  2 },		// 0x32
	{ CONDACT_LET,    		  2 },		// 0x33
	{ CONDACT_NEWLINE,		  0 },		// 0x34
	{ CONDACT_PRINT,  		  1 },		// 0x35
	{ CONDACT_SYSMESS,		  1 },		// 0x36
	{ CONDACT_ISAT,   		  2 },		// 0x37
	{ CONDACT_SETCO,  		  1 },		// 0x38
	{ CONDACT_SPACE,  		  0 },		// 0x39
	{ CONDACT_HASAT,  		  1 },		// 0x3A
	{ CONDACT_HASNAT, 		  1 },		// 0x3B
	{ CONDACT_LISTOBJ,		  0 },		// 0x3C
	{ CONDACT_EXTERN, 		  2 },		// 0x3D
	{ CONDACT_RAMSAVE,		  0 },		// 0x3E
	{ CONDACT_RAMLOAD,		  1 },		// 0x3F
	{ CONDACT_BEEP,   		  2 },		// 0x40
	{ CONDACT_PAPER,  		  1 },		// 0x41
	{ CONDACT_INK,    		  1 },		// 0x42
	{ CONDACT_BORDER, 		  1 },		// 0x43
	{ CONDACT_PREP,   		  1 },		// 0x44
	{ CONDACT_NOUN2,  		  1 },		// 0x45
	{ CONDACT_ADJECT2,		  1 },		// 0x46
	{ CONDACT_ADD,    		  2 },		// 0x47
	{ CONDACT_SUB,    		  2 },		// 0x48
	{ CONDACT_PARSE,  		  1 },		// 0x49
	{ CONDACT_LISTAT, 		  1 },		// 0x4A
	{ CONDACT_PROCESS,		  1 },		// 0x4B
	{ CONDACT_SAME,   		  2 },		// 0x4C
	{ CONDACT_MES,    		  1 },		// 0x4D
	{ CONDACT_WINDOW, 		  1 },		// 0x4E
	{ CONDACT_NOTEQ,  		  2 },		// 0x4F
	{ CONDACT_NOTSAME,		  2 },		// 0x50
	{ CONDACT_MODE,   		  1 },		// 0x51
	{ CONDACT_WINAT,  		  2 },		// 0x52
	{ CONDACT_TIME,   		  2 },		// 0x53
	{ CONDACT_PICTURE,		  1 },		// 0x54
	{ CONDACT_DOALL,  		  1 },		// 0x55
	{ CONDACT_MOUSE,  		  2 },		// 0x56
	{ CONDACT_GFX,    		  2 },		// 0x57
	{ CONDACT_ISNOTAT,		  2 },		// 0x58
	{ CONDACT_WEIGH,  		  2 },		// 0x59
	{ CONDACT_PUTIN,  		  2 },		// 0x5A
	{ CONDACT_TAKEOUT,		  2 },		// 0x5B
	{ CONDACT_NEWTEXT,		  0 },		// 0x5C
	{ CONDACT_ABILITY,		  2 },		// 0x5D
	{ CONDACT_WEIGHT, 		  1 },		// 0x5E
	{ CONDACT_RANDOM, 		  1 },		// 0x5F
	{ CONDACT_INPUT,  		  2 },		// 0x60
	{ CONDACT_SAVEAT, 		  0 },		// 0x61
	{ CONDACT_BACKAT, 		  0 },		// 0x62
	{ CONDACT_PRINTAT,		  2 },		// 0x63
	{ CONDACT_WHATO,  		  0 },		// 0x64
	{ CONDACT_CALL,   		  2 },		// 0x65 (a 16 bit address, in two single-byte parameters)
	{ CONDACT_PUTO,   		  1 },		// 0x66
	{ CONDACT_NOTDONE,		  0 },		// 0x67
	{ CONDACT_AUTOP,  		  1 },		// 0x68
	{ CONDACT_AUTOT,  		  1 },		// 0x69
	{ CONDACT_MOVE,   		  1 },		// 0x6A
	{ CONDACT_WINSIZE,		  2 },		// 0x6B
	{ CONDACT_REDO,   		  0 },		// 0x6C
	{ CONDACT_CENTRE, 		  0 },		// 0x6D
	{ CONDACT_EXIT,   		  1 },		// 0x6E
	{ CONDACT_INKEY,  		  0 },		// 0x6F
	{ CONDACT_BIGGER, 		  2 },		// 0x70
	{ CONDACT_SMALLER,		  2 },		// 0x71
	{ CONDACT_ISDONE, 		  0 },		// 0x72
	{ CONDACT_ISNDONE,		  0 },		// 0x73
	{ CONDACT_SKIP,   		  1 },		// 0x74
	{ CONDACT_RESTART,		  0 },		// 0x75
	{ CONDACT_TAB,    		  1 },		// 0x76
	{ CONDACT_COPYOF, 		  2 },		// 0x77
	{ CONDACT_XMESSAGE,  	  2 },		// 0x78		- New in DAAD V3
	{ CONDACT_COPYOO, 		  2 },		// 0x79
	{ CONDACT_INDIR, 		  1 },		// 0x7A		- New in DAAD V3
	{ CONDACT_COPYFO, 		  2 },		// 0x7B
	{ CONDACT_SETAT, 	      2 },		// 0x7C		- New in DAAD V3
	{ CONDACT_COPYFF, 		  2 },		// 0x7D
	{ CONDACT_COPYBF, 		  2 },		// 0x7E
	{ CONDACT_RESET,   		  0 },		// 0x7F
};

#if HAS_PAWS

static DDB_CondactMap pawsCondacts[128] = {
	#define DDB_PAWS_CONDACT_ENTRY(code, condact, parameters) { condact, parameters },
	DDB_PAWS_CONDACTS(DDB_PAWS_CONDACT_ENTRY)
	#undef DDB_PAWS_CONDACT_ENTRY

	// 0x6C-0x7F are not valid PAW opcodes
	{ CONDACT_INVALID,        0 },		// 0x6C
	{ CONDACT_INVALID,        0 },		// 0x6D
	{ CONDACT_INVALID,        0 },		// 0x6E
	{ CONDACT_INVALID,        0 },		// 0x6F
	{ CONDACT_INVALID,        0 },		// 0x70
	{ CONDACT_INVALID,        0 },		// 0x71
	{ CONDACT_INVALID,        0 },		// 0x72
	{ CONDACT_INVALID,        0 },		// 0x73
	{ CONDACT_INVALID,        0 },		// 0x74
	{ CONDACT_INVALID,        0 },		// 0x75
	{ CONDACT_INVALID,        0 },		// 0x76
	{ CONDACT_INVALID,        0 },		// 0x77
	{ CONDACT_INVALID,        0 },		// 0x78
	{ CONDACT_INVALID,        0 },		// 0x79
	{ CONDACT_INVALID,        0 },		// 0x7A
	{ CONDACT_INVALID,        0 },		// 0x7B
	{ CONDACT_INVALID,        0 },		// 0x7C
	{ CONDACT_INVALID,        0 },		// 0x7D
	{ CONDACT_INVALID,        0 },		// 0x7E
	{ CONDACT_INVALID,        0 },		// 0x7F
};

#endif

void DDB_SetError(DDB_Error error)
{
	ddbError = error;
}

DDB_Error DDB_GetError()
{
	return ddbError;
}

const char* DDB_GetErrorString()
{
	switch (ddbError)
	{
		case DDB_ERROR_NONE:               return "No error";
		case DDB_ERROR_FILE_NOT_FOUND:     return "File not found";
		case DDB_ERROR_READING_FILE:       return "I/O error reading file";
		case DDB_ERROR_SEEKING_FILE:       return "I/O error seeking file";
		case DDB_ERROR_CREATING_FILE:	   return "I/O error creating file";
		case DDB_ERROR_WRITING_FILE:       return "I/O error writing file";
		case DDB_ERROR_OUT_OF_MEMORY:      return "Out of memory";
		case DDB_ERROR_INVALID_FILE:       return "Corrupted or invalid DDB file";
		case DDB_ERROR_FILE_NOT_SUPPORTED: return "Unsupported file format";
		case DDB_ERROR_VIDEO_MODE_NOT_SUPPORTED: return "Selected video mode is not supported by this build";
		case DDB_ERROR_VIDEO_HARDWARE_NOT_SUPPORTED: return "Required video hardware was not detected";
		case DDB_ERROR_SDL:                return "SDL error";
		case DDB_ERROR_NO_DDBS_FOUND:      return "No DDBs found";
		default:                           return "Unknown error";
	}
}

void DDB_SetWarningHandler(void (*handler)(const char* message))
{
	warningHandler = handler;
}

void DDB_Warning(const char* format, ...)
{
	#if _STDCLIB && !defined(NO_PRINTF)
		char buffer[256];
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);
	#elif defined(AMIGA)
		char buffer[256];
		va_list args;
		va_start(args, format);
		vsnprintf_(buffer, sizeof(buffer), format, args);
		va_end(args);
	#else
		const char* buffer = format;
	#endif
	if (warningHandler != 0)
		warningHandler(buffer);
	else
	{
		DebugPrintf("%s\n", buffer);
	}
}

/* ─────────────────────────────────── ────────────────────────────────────── */

const char* DDB_GetDebugMessage (DDB* ddb, DDB_MsgType type, uint8_t msgId)
{
	static char buffer[64];
	DDB_GetMessage (ddb, type, msgId, buffer, sizeof(buffer));
	if (buffer[60] != 0)
		StrCopy(buffer+60, 4, "...");
	return buffer;
}

const char* DDB_GetMessage (DDB* ddb, DDB_MsgType type, uint8_t msgId, char* buffer, size_t bufferSize)
{
	uint8_t* ptr;

	if (bufferSize == 0 || buffer == 0)
		return buffer;

	*buffer = 0;

	switch (type)
	{
		case DDB_MSG:
			ptr = ddb->messages[msgId];
			break;
		case DDB_SYSMSG:
			if (msgId >= ddb->numSystemMessages)
				return buffer;
			ptr = ddb->data + ddb->sysMsgTable[msgId];
			break;
		case DDB_OBJNAME:
			if (msgId >= ddb->numObjects)
				return buffer;
			ptr = ddb->data + ddb->objNamTable[msgId];
			break;
		case DDB_LOCDESC:
			ptr = ddb->locDescriptions[msgId];
			break;
		default:
			DDB_Warning("Invalid message type %d", type);
			return buffer;
	}

	if (ptr <= ddb->data || ptr >= ddb->data + ddb->dataSize)
		return buffer;

	uint8_t endMarker = ddb->version == DDB_VERSION_PAWS ? 0x1F : 0x0A;

	bufferSize--;
	while (bufferSize > 0)
	{
		uint8_t c = *ptr++ ^ 0xFF;
		if (c == endMarker)
		{
			*buffer = 0;
			return buffer;
		}
		if (c >= ddb->firstToken)
		{
			if (!ddb->hasTokens)
			{
				DDB_Warning("Message contains token 0x%02X but DDB has no tokens!", c);
				continue;
			}
			uint8_t* token = ddb->tokensPtr[c - ddb->firstToken];
			if (token == 0)
			{
				DDB_Warning("Message contains token 0x%02X but it's not defined in the DDB!", c);
				continue;
			}
			while (bufferSize > 0)
			{
				*buffer++ = *token & 0x7F;
				bufferSize--;
				if ((*token & 0x80) != 0)
					break;
				token++;
			}
		}
		else
		{
			*buffer++ = c;
			bufferSize--;
		}
	}
	if (bufferSize > 0)
		*buffer = 0;
	return buffer;
}

/* ───────────────────────────────────────────────────────────────────────── */

void DDB_FixOffsets (DDB* ddb)
{
	int n;
	uint32_t minimumHeaderSize = DDB_GetMinimumHeaderSize(ddb->version);

#ifndef _WEB
#ifdef _BIG_ENDIAN
	if (!ddb->littleEndian)
		return;
#endif
#ifdef _LITTLE_ENDIAN
	if (ddb->littleEndian)
		return;
#endif
#endif

	uint16_t* tables[] = {
		ddb->msgTable,
		ddb->sysMsgTable,
		ddb->objNamTable,
		ddb->locDescTable,
		ddb->processTable,
		ddb->conTable
	};
	uint8_t counts[] = {
		ddb->numMessages,
		ddb->numSystemMessages,
		ddb->numObjects,
		ddb->numLocations,
		ddb->numProcesses,
		ddb->numLocations
	};
	const char* tableName[] = {
		"Messages",
		"System messages",
		"Object names",
		"Location descriptions",
		"Processes",
		"Connections"
	};
	const int numTables = sizeof(tables) / sizeof(tables[0]);

	for (n = 0; n < numTables; n++)
	{
		uint16_t* table = tables[n];
		uint8_t entries = counts[n];
		int m;
		if (table == 0)
			continue;
		for (m = 0; m < entries; m++)
		{
			uint16_t rawOffset = read16((uint8_t*)table + m * 2, ddb->littleEndian);
			uint32_t offset = 0;
			if (!DDB_DecodeStoredOffset(ddb, rawOffset, ddb->dataSize, false, &offset) || offset < minimumHeaderSize)
			{
				DDB_Warning("Invalid internal offset 0x%04X (entry %d in %s)", offset, m, tableName[n]);
				ddbError = DDB_ERROR_INVALID_FILE;
				table[m] = 0;
				continue;
			}
			table[m] = offset;
		}
	}

	// Fix processes

	for (n = 0; n < ddb->numProcesses; n++)
	{
		int entryIndex = 0;
		uint16_t offset = ddb->processTable[n];
		uint8_t* entry = ddb->data + offset;
		uint8_t* end = ddb->data + ddb->dataSize;
		if (offset == 0)
			continue;
		while (entry + 4 <= end)
		{
			// End of process marker
			if (entry[0] == 0)
				break;

			// PAW '*' entries (value 1) are left untouched so dumps stay
			// byte-faithful; the runtime matcher treats 1 as a wildcard.

			uint32_t entryOffset = 0;
			if (!DDB_DecodeStoredOffset(ddb, read16(entry + 2, ddb->littleEndian), ddb->dataSize, false, &entryOffset) || entryOffset < minimumHeaderSize)
			{
				DDB_Warning("Invalid entry %d offset 0x%04X in process %d", entryIndex, entryOffset, n);
				ddbError = DDB_ERROR_INVALID_FILE;
				uint16_t zero = 0;
				MemCopy(entry + 2, &zero, sizeof(zero));
				break;
			}
			uint16_t normalizedOffset = (uint16_t)entryOffset;
			MemCopy(entry + 2, &normalizedOffset, sizeof(normalizedOffset));

			entry += 4;
			entryIndex++;
		}

		if (entry + 4 > end)
		{
			DDB_Warning("Truncated process %d table", n);
			ddbError = DDB_ERROR_INVALID_FILE;
		}
	}
}

bool DDB_CheckVideoMode(const char* fileName, DDB_ScreenMode* mode)
{
	File* file = File_Open(ChangeExtension(fileName, ".dat"), ReadOnly);
	if (file == NULL)
    {
		file = File_Open(ChangeExtension(fileName, ".ega"), ReadOnly);
        if (file != NULL)
        {
            *mode = ScreenMode_EGA;
            File_Close(file);
            return true;
        }
		file = File_Open(ChangeExtension(fileName, ".cga"), ReadOnly);
        if (file != NULL)
        {
            *mode = ScreenMode_CGA;
            File_Close(file);
            return true;
        }
		#if HAS_PCX
		file = File_Open(ChangeExtension(fileName, ".VGA"), ReadOnly);
		if (file == NULL)
			file = File_Open(ChangeExtension(fileName, ".vga"), ReadOnly);
		if (file != NULL)
		{
			*mode = ScreenMode_VGA;
			File_Close(file);
			return true;
		}
		#endif
        return false;
    }
    
    uint8_t header[16];
    if (File_Read(file, header, sizeof(header)) != sizeof(header))
    {
        File_Close(file);
        return false;
    }
    uint16_t width  = read16BE(header + 0x6);
    uint16_t height = read16BE(header + 0x8);
    
	// uint16_t flags  = read16BE(header + 0xE);
	// DAT5_ColorMode colorMode = (DAT5_ColorMode)(flags & 0x000F);

    if (width == 320 && height == 200)
        *mode = ScreenMode_VGA;
    else if (width == 640 && height == 200)
        *mode = ScreenMode_HiRes;
    else if (width == 640 && height == 400)
        *mode = ScreenMode_SHiRes;

    File_Close(file);
    return true;
}

static uint32_t DDB_GetDataFileModeFlag(DDB_ScreenMode mode)
{
	switch (mode)
	{
		case ScreenMode_CGA:    return DDB_DataFileMode_CGA;
		case ScreenMode_EGA:    return DDB_DataFileMode_EGA;
		case ScreenMode_VGA16:  return DDB_DataFileMode_VGA16;
		case ScreenMode_VGA:    return DDB_DataFileMode_VGA;
		case ScreenMode_HiRes:  return DDB_DataFileMode_HiRes;
		case ScreenMode_SHiRes: return DDB_DataFileMode_SHiRes;
		default:                return 0;
	}
}

static bool DDB_IsClassicDATSignature(uint16_t signature)
{
	return signature == 0x0004 || signature == 0x0300 || signature == 0xFFFF;
}

// Data file probes are memoized: the startup path probes the same handful
// of candidate names several times (mode detection, file resolution, video
// configuration), and on floppy media every probe costs seeks between the
// directory and data cylinders. The cache is flushed whenever the file
// list is re-enumerated (startup, part transitions, disk swaps).
#define PROBE_CACHE_ENTRIES 12

struct DDB_ProbeCacheEntry
{
	char           path[FILE_MAX_PATH];
	bool           used;
	bool           result;
	uint32_t       maskContribution;
	DDB_ScreenMode nativeMode;
	uint8_t        nativePlanes;
};

static DDB_ProbeCacheEntry probeCache[PROBE_CACHE_ENTRIES];
static int probeCacheNext = 0;

void DDB_FlushDataFileProbeCache()
{
	for (int n = 0; n < PROBE_CACHE_ENTRIES; n++)
		probeCache[n].used = false;
	probeCacheNext = 0;
}

static bool DDB_ProbeDataFileUncached(const char* path, DDB_Machine target, DDB_ScreenMode extensionHint, uint32_t* modeMask, DDB_ScreenMode* nativeMode, uint8_t* nativePlanes);

static bool DDB_ProbeDataFile(const char* path, DDB_Machine target, DDB_ScreenMode extensionHint, uint32_t* modeMask, DDB_ScreenMode* nativeMode, uint8_t* nativePlanes)
{
	for (int n = 0; n < PROBE_CACHE_ENTRIES; n++)
	{
		if (probeCache[n].used && StrComp(probeCache[n].path, path) == 0)
		{
			if (probeCache[n].result)
			{
				if (modeMask)
					*modeMask |= probeCache[n].maskContribution;
				if (nativeMode)
					*nativeMode = probeCache[n].nativeMode;
				if (nativePlanes)
					*nativePlanes = probeCache[n].nativePlanes;
			}
			return probeCache[n].result;
		}
	}

	uint32_t contribution = 0;
	DDB_ScreenMode mode = ScreenMode_Default;
	uint8_t planes = 4;
	bool result = DDB_ProbeDataFileUncached(path, target, extensionHint, &contribution, &mode, &planes);

	DDB_ProbeCacheEntry* entry = &probeCache[probeCacheNext];
	probeCacheNext = (probeCacheNext + 1) % PROBE_CACHE_ENTRIES;
	StrCopy(entry->path, sizeof(entry->path), path);
	entry->used = true;
	entry->result = result;
	entry->maskContribution = contribution;
	entry->nativeMode = mode;
	entry->nativePlanes = planes;

	if (result)
	{
		if (modeMask)
			*modeMask |= contribution;
		if (nativeMode)
			*nativeMode = mode;
		if (nativePlanes)
			*nativePlanes = planes;
	}
	return result;
}

static bool DDB_ProbeDataFileUncached(const char* path, DDB_Machine target, DDB_ScreenMode extensionHint, uint32_t* modeMask, DDB_ScreenMode* nativeMode, uint8_t* nativePlanes)
{
	File* file = File_Open(path, ReadOnly);
	if (file == 0)
		return false;

	uint64_t fileSize = File_GetSize(file);
	uint8_t header[16];
	bool ok = File_Read(file, header, sizeof(header)) == sizeof(header);
	File_Close(file);
	if (!ok)
		return false;

	DDB_ScreenMode mode = ScreenMode_Default;
	uint8_t planes = 4;
	uint32_t mask = 0;
	const char* extension = StrRChr(path, '.');

	if (header[0] == 'D' && header[1] == 'A' && header[2] == 'T' && header[3] == 0 && header[4] == 0 && header[5] == 5)
	{
		uint16_t width = read16BE(header + 0x06);
		uint16_t height = read16BE(header + 0x08);
		uint8_t colorMode = header[0x0E];
		#ifdef _DOS
		if (!DMG_DAT5ModeIsDOSSupported(colorMode))
		{
			// DebugPrintf("DDB_ProbeDataFile: rejecting DOS-unsupported DAT5 %s colorMode=%u size=%ux%u\n",
			// 	path,
			// 	(unsigned)colorMode,
			// 	(unsigned)width,
			// 	(unsigned)height);
			return false;
		}
		#endif
		planes = DMG_DAT5ModePlaneCount(colorMode);
		if (planes == 0)
			planes = 4;

		if (width == 320 && height == 200)
		{
			mode =
				(colorMode == DMG_DAT5_COLORMODE_CGA) ? ScreenMode_CGA :
				(colorMode == DMG_DAT5_COLORMODE_EGA) ? ScreenMode_EGA :
				(DMG_DAT5ModePlaneCount(colorMode) >= 8) ? ScreenMode_VGA :
				ScreenMode_VGA16;
		}
		else if (width == 640 && height == 200)
			mode = ScreenMode_HiRes;
		else if (width == 640 && height == 400)
			mode = ScreenMode_SHiRes;

		mask = DDB_GetDataFileModeFlag(mode);
		// DebugPrintf("DDB_ProbeDataFile: %s DAT5 colorMode=%u size=%ux%u screen=%u planes=%u mask=0x%08lX\n",
		// 	path,
		// 	(unsigned)colorMode,
		// 	(unsigned)width,
		// 	(unsigned)height,
		// 	(unsigned)mode,
		// 	(unsigned)planes,
		// 	(unsigned long)mask);
	}
	else if (extension != 0)
	{
		if (StrIComp(extension, ".cga") == 0)
		{
			mode = ScreenMode_CGA;
			mask = DDB_DataFileMode_CGA;
		}
		else if (StrIComp(extension, ".ega") == 0)
		{
			mode = ScreenMode_EGA;
			mask = DDB_DataFileMode_EGA;
		}
	}
	// The PCW data file (little-endian, machine=0/mode=4 header) is a format
	// DMG_Open handles directly; recognise it here too, otherwise this probe
	// rejects it and the loader reports the data file as "not found".
	if (mask == 0 && target == DDB_MACHINE_PCW &&
		DMG_LooksLikePCWDataFile(header, fileSize, extension))
	{
		mode = ScreenMode_HiRes;
		mask = DDB_DataFileMode_HiRes;
		planes = 1;
	}

	if (mask == 0)
	{
		uint16_t signature = read16BE(header);
		if (DDB_IsClassicDATSignature(signature))
		{
			uint16_t storedMode = read16BE(header + 2);
			if (target == DDB_MACHINE_IBMPC && storedMode == 0)
			{
				mask = DDB_DataFileMode_CGA | DDB_DataFileMode_EGA | DDB_DataFileMode_VGA16;
				mode = extensionHint != ScreenMode_Default ? extensionHint : ScreenMode_VGA16;
			}
			else if (storedMode == ScreenMode_CGA)
			{
				mode = ScreenMode_CGA;
				mask = DDB_DataFileMode_CGA;
			}
			else if (storedMode == ScreenMode_EGA)
			{
				mode = ScreenMode_EGA;
				mask = DDB_DataFileMode_EGA;
			}
			else if (storedMode == ScreenMode_HiRes)
			{
				mode = ScreenMode_HiRes;
				mask = DDB_DataFileMode_HiRes;
			}
			else if (storedMode == ScreenMode_SHiRes)
			{
				mode = ScreenMode_SHiRes;
				mask = DDB_DataFileMode_SHiRes;
			}
			else
			{
				mode = extensionHint != ScreenMode_Default ? extensionHint : ScreenMode_VGA16;
				mask = DDB_GetDataFileModeFlag(mode);
			}
		}
	}

	if (mask == 0)
		return false;

	if (modeMask)
		*modeMask |= mask;
	if (nativeMode)
		*nativeMode = mode;
	if (nativePlanes)
		*nativePlanes = planes;
	return true;
}

// Data file extensions are probed in lowercase/uppercase pairs. Platforms
// whose filesystems are case-insensitive (floppy-based targets, where the
// redundant probes are also the most expensive) only probe one of each.
#if defined(_AMIGA) || defined(_DOS) || defined(_ATARIST)
#define DDB_PROBE_CASE_TWINS 0
#else
#define DDB_PROBE_CASE_TWINS 1
#endif

uint32_t DDB_GetDataFileModes(const char* fileName, DDB_Machine target)
{
	uint32_t mask = 0;
	DDB_ProbeDataFile(ChangeExtension(fileName, ".dat"), target, ScreenMode_Default, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".ega"), target, ScreenMode_EGA, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".cga"), target, ScreenMode_CGA, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".vga"), target, ScreenMode_VGA16, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".sga"), target, ScreenMode_SHiRes, &mask, 0, 0);
	#if DDB_PROBE_CASE_TWINS
	DDB_ProbeDataFile(ChangeExtension(fileName, ".DAT"), target, ScreenMode_Default, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".EGA"), target, ScreenMode_EGA, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".CGA"), target, ScreenMode_CGA, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".VGA"), target, ScreenMode_VGA16, &mask, 0, 0);
	DDB_ProbeDataFile(ChangeExtension(fileName, ".SGA"), target, ScreenMode_SHiRes, &mask, 0, 0);
	#endif
	return mask;
}

bool DDB_ResolveDataFile(const char* fileName, DDB_Machine target, DDB_ScreenMode requestedMode, char* resolvedFileName, size_t resolvedFileNameSize, DDB_ScreenMode* resolvedMode, uint8_t* planes)
{
	struct Candidate
	{
		const char* extension;
		DDB_ScreenMode hint;
	};

	static const Candidate candidates[] =
	{
		{ ".dat", ScreenMode_Default },
		{ ".ega", ScreenMode_EGA },
		{ ".cga", ScreenMode_CGA },
		{ ".vga", ScreenMode_VGA16 },
		{ ".sga", ScreenMode_SHiRes },
		#if DDB_PROBE_CASE_TWINS
		{ ".DAT", ScreenMode_Default },
		{ ".EGA", ScreenMode_EGA },
		{ ".CGA", ScreenMode_CGA },
		{ ".VGA", ScreenMode_VGA16 },
		{ ".SGA", ScreenMode_SHiRes },
		#endif
		{ 0, ScreenMode_Default },
	};

	uint32_t requestedFlag = DDB_GetDataFileModeFlag(requestedMode);
	for (const Candidate* candidate = candidates; candidate->extension != 0; candidate++)
	{
		char path[FILE_MAX_PATH];
		StrCopy(path, sizeof(path), ChangeExtension(fileName, candidate->extension));
		uint32_t mask = 0;
		DDB_ScreenMode nativeMode = ScreenMode_Default;
		uint8_t nativePlanes = 4;
		if (!DDB_ProbeDataFile(path, target, candidate->hint, &mask, &nativeMode, &nativePlanes))
			continue;
		if (requestedMode != ScreenMode_Default && (mask & requestedFlag) == 0 && nativeMode != requestedMode)
			continue;

		if (resolvedFileName && resolvedFileNameSize > 0)
			StrCopy(resolvedFileName, resolvedFileNameSize, path);
		if (resolvedMode)
		{
			if (requestedMode != ScreenMode_Default && (mask & requestedFlag) != 0)
				*resolvedMode = requestedMode;
			else
				*resolvedMode = nativeMode;
		}
		if (planes)
			*planes = nativePlanes;
		// DebugPrintf("DDB_ResolveDataFile: selected %s requested=%u native=%u planes=%u mask=0x%08lX\n",
		// 	path,
		// 	(unsigned)requestedMode,
		// 	(unsigned)nativeMode,
		// 	(unsigned)nativePlanes,
		// 	(unsigned long)mask);
		return true;
	}

	return false;
}

DDB_ScreenMode DDB_GetDefaultScreenMode(DDB_Machine machine)
{
	return machine == DDB_MACHINE_PCW ? ScreenMode_HiRes : ScreenMode_VGA16;
}

#if HAS_SNAPSHOTS

static bool DDB_HasSnapshotExtension(const char* filename)
{
	const char* dot = (const char*)StrRChr(filename, '.');
	if (dot == 0)
		return false;

	return 
		StrIComp(dot, ".z80") == 0 ||
		StrIComp(dot, ".sna") == 0 ||
		StrIComp(dot, ".tzx") == 0 ||
		StrIComp(dot, ".cdt") == 0 ||
		StrIComp(dot, ".sta") == 0 ||
		StrIComp(dot, ".vsf") == 0 ||
		StrIComp(dot, ".tap") == 0 ||
		StrIComp(dot, ".cas") == 0 ||
		StrIComp(dot, ".bin") == 0 ||
		StrIComp(dot, ".rom") == 0 ||
		StrIComp(dot, ".raw") == 0;
}

#endif

bool DDB_CheckDataFileConfig(const char* fileName, DDB_Machine target, DDB_ScreenMode* mode, uint8_t* planes)
{
	DDB_ScreenMode requestedMode = mode ? *mode : ScreenMode_Default;
	DDB_ScreenMode resolvedMode = ScreenMode_Default;
	uint8_t resolvedPlanes = 4;
	if (!DDB_ResolveDataFile(fileName, target, requestedMode, 0, 0, &resolvedMode, &resolvedPlanes))
		return false;
	if (mode)
		*mode = resolvedMode;
	if (planes)
		*planes = resolvedPlanes;
	return true;
}

#if HAS_SNAPSHOTS
static uint32_t GuessDDBOffsetFromSnapshot(uint8_t* memory, size_t size, DDB_Machine target, DDB* ddb);
#if HAS_PAWS
static bool LoadPAWS(DDB* ddb, uint8_t* memory, size_t size);
#endif

// Check a memory snapshot (or a program file holding a memory image, like
// the .BIN files found in CPC disks): load it, locate the database inside
// and validate its header
static bool DDB_CheckSnapshot(File* file, const char* filename, DDB_Machine* target, DDB_Language* language, DDB_Version* version)
{
	uint8_t* memory = 0;
	size_t ramSize = 0;
	DDB_Machine snapshotMachine;
	if (!DDB_LoadSnapshot(file, filename, &memory, &ramSize, &snapshotMachine))
		return false;

	DDB* check = Allocate<DDB>("DDB Check", 1, true);
	if (check == 0)
	{
		Free(memory);
		return false;
	}

	bool result = false;
	#if HAS_PAWS
	if (LoadPAWS(check, memory, ramSize))
	{
		if (language != 0)
			*language = check->language;
		if (target != 0)
			*target = check->target;
		if (version != 0)
			*version = DDB_VERSION_PAWS;
		result = true;
	}
	else
	#endif
	if (GuessDDBOffsetFromSnapshot(memory, ramSize, snapshotMachine, check))
	{
		ddbError = DDB_ERROR_NONE;
		if (DDB_ParseHeader(check, check->data, check->dataSize, check->dataSize) &&
		    ddbError == DDB_ERROR_NONE)
		{
			if (language != 0)
				*language = check->language;
			if (target != 0)
				*target = check->target;
			if (version != 0)
				*version = check->version;
			result = true;
		}
	}
	Free(check);
	Free(memory);
	return result;
}
#endif

bool DDB_Check(const char* filename, DDB_Machine* target, DDB_Language* language, DDB_Version* version)
{
	const uint32_t checkBufferSize = 512;
	uint8_t* buffer = Allocate<uint8_t>("DDB Check buffer", checkBufferSize);
	DDB* check = Allocate<DDB>("DDB Check", 1, true);
	if (buffer == 0 || check == 0)
	{
		ddbError = DDB_ERROR_OUT_OF_MEMORY;
		if (buffer != 0)
			Free(buffer);
		if (check != 0)
			Free(check);
		return false;
	}

	File* file = File_Open(filename, ReadOnly);
	if (file == 0)
	{
		Free(check);
		Free(buffer);
		return false;
	}

	#if HAS_PAWS
	if (DDB_PAWSIsSDB(file))
	{
		uint8_t* sdbMemory = 0;
		size_t sdbSize = 0;
		bool valid = DDB_PAWSLoadSDB(file, &sdbMemory, &sdbSize, 0, 0) &&
			LoadPAWS(check, sdbMemory, sdbSize);
		if (valid)
		{
			if (target) *target = DDB_MACHINE_SPECTRUM;
			if (language) *language = check->language;
			if (version) *version = DDB_VERSION_PAWS;
		}
		File_Close(file);
		if (sdbMemory) Free(sdbMemory);
		Free(check);
		Free(buffer);
		return valid;
	}
	#endif

	#if HAS_SNAPSHOTS
	if (DDB_HasSnapshotExtension(filename))
	{
		if (DDB_CheckSnapshot(file, filename, target, language, version))
		{
			File_Close(file);
			Free(check);
			Free(buffer);
			return true;
		}
		File_Seek(file, 0);
	}
	#endif

	uint64_t fileSize = File_GetSize(file);
	if (fileSize < 34 || fileSize > MAX_DDB_SIZE)
	{
		DDB_Warning("Rejecting DDB %s due to size %lu (max %u)", filename, (unsigned long)fileSize, (unsigned)MAX_DDB_SIZE);
		File_Close(file);
		Free(check);
		Free(buffer);
		return false;
	}

	uint32_t readSize = fileSize < checkBufferSize ? (uint32_t)fileSize : checkBufferSize;
	if (File_Read(file, buffer, readSize) != readSize)
	{
		File_Close(file);
		Free(check);
		Free(buffer);
		return false;
	}
	File_Close(file);

	ddbError = DDB_ERROR_NONE;
	if (!DDB_ParseHeader(check, buffer, fileSize, readSize))
	{
		Free(check);
		Free(buffer);
		return false;
	}

	if (!DDB_ValidateOffsetTablePrefix(check, buffer, readSize, read16(buffer + 10, check->littleEndian), check->numProcesses, "Processes") ||
		!DDB_ValidateOffsetTablePrefix(check, buffer, readSize, read16(buffer + 16, check->littleEndian), check->numMessages, "Messages") ||
		!DDB_ValidateOffsetTablePrefix(check, buffer, readSize, read16(buffer + 18, check->littleEndian), check->numSystemMessages, "System messages") ||
		!DDB_ValidateOffsetTablePrefix(check, buffer, readSize, read16(buffer + 12, check->littleEndian), check->numObjects, "Object names") ||
		!DDB_ValidateOffsetTablePrefix(check, buffer, readSize, read16(buffer + 14, check->littleEndian), check->numLocations, "Location descriptions") ||
		!DDB_ValidateOffsetTablePrefix(check, buffer, readSize, read16(buffer + 20, check->littleEndian), check->numLocations, "Connections"))
	{
		Free(check);
		Free(buffer);
		return false;
	}

	if (ddbError != DDB_ERROR_NONE)
	{
		Free(check);
		Free(buffer);
		return false;
	}

	if (language != 0)
		*language = check->language;
	if (target != 0)
		*target = check->target;
	if (version != 0)
		*version = check->version;

	Free(check);
	Free(buffer);

	return true;
}

bool DDB_SupportsDataFile(DDB_Version version, DDB_Machine target)
{
	#if HAS_PAWS
	if (version == DDB_VERSION_PAWS)
		return false;
	#endif

	// Spectrum is not here: its graphics ship as a .SDG loaded by DDB_Load
	// (drawString), never as a DMG .dat, so probing one just prints a
	// misleading "data file not found" for every tape-based game
	return
		target == DDB_MACHINE_AMIGA ||
		target == DDB_MACHINE_ATARIST ||
		target == DDB_MACHINE_PCW ||
		target == DDB_MACHINE_IBMPC;
}

#if HAS_SNAPSHOTS
static uint32_t GuessDDBOffsetFromSnapshot(uint8_t* memory, size_t size, DDB_Machine target, DDB* ddb)
{
	uint32_t offset = 0;

	// This is actually the minimum offset for the platform

	switch (target)
	{
		case DDB_MACHINE_SPECTRUM: offset = 0x5B00; break;
		case DDB_MACHINE_C64:      offset = 0x3880; break;
		case DDB_MACHINE_CPC:      offset = 0x2880; break;
		case DDB_MACHINE_MSX:      offset = 0x0100; break;
		case DDB_MACHINE_PLUS4:    offset = 0x7080; break;
		default:                   offset = 0; break;
	}

	// Exploring the entire memory is a bit overkill, but...

	while (offset < size)
	{
		if (   (memory[offset] == 1 || memory[offset] == 2) // DAAD Version
		    && (memory[offset+1] & 0xF0) == (target << 4)   // Machine
			&& (memory[offset+2] == 0x5F))
		{
			uint16_t eofOffset = offset + 0x1E;
			if (memory[offset] == 2) eofOffset += 2;
			uint16_t eof = read16(memory + eofOffset, true);
			if (eof <= size && eof > offset + 32)
			{
				// The signature is only three bytes and graphics data can
				// match it by chance: trial-parse a scratch copy (the parser
				// byte-swaps in place) and keep scanning on failure
				uint32_t candidateSize = eof - offset;
				uint8_t* copy = Allocate<uint8_t>("DDB candidate", candidateSize);
				DDB* trial = Allocate<DDB>("DDB candidate header", 1, true);
				bool valid = false;
				if (copy != 0 && trial != 0)
				{
					MemCopy(copy, memory + offset, candidateSize);
					trial->baseOffset = offset;
					void (*savedHandler)(const char*) = warningHandler;
					warningHandler = 0;
					DDB_Error savedError = ddbError;
					valid = DDB_ParseHeader(trial, copy, candidateSize, candidateSize);
					ddbError = savedError;
					warningHandler = savedHandler;
				}
				if (copy != 0)
					Free(copy);
				if (trial != 0)
					Free(trial);
				if (valid)
				{
					ddb->baseOffset = offset;
					ddb->data = memory + offset;
					ddb->dataSize = candidateSize;
					ddb->version = (DDB_Version)memory[offset];
					return true;
				}
			}
		}

		if (offset == 65535)
			break;
		offset++;
	}
	return 0;
}
#endif

static void DDB_FillTokenPointers(DDB* ddb)
{
	// An offset 0 means no tokens in file. Otherwise,
	// update the token pointers in the DDB structure

	if (ddb->tokens == ddb->data)
	{
		ddb->hasTokens = false;
		ddb->tokens = 0;
	}
	else
	{
		int n;
		uint8_t* ptr = ddb->tokens + 1;
		uint8_t* end = ddb->data + ddb->dataSize;
        uint8_t  eof = ddb->version == DDB_VERSION_PAWS ? 0xFF : 0x00;
		for (n = ddb->firstToken; n <= 255; n++)
		{
			ddb->tokensPtr[n - ddb->firstToken] = ptr;
			while (ptr < end && (*ptr & 0x80) == 0 && *ptr != 0)
				ptr++;
			if (ptr == end || *ptr == eof)
				break;
			ptr++;
		}
		while (n < 256)
			ddb->tokensPtr[n++ - ddb->firstToken] = 0;
		ddb->hasTokens = true;
		ddb->tokenBlockSize = ptr - ddb->tokens;
	}
}

static void DDB_FillMsgPointers(DDB* ddb)
{
	for (int i = 0; i < ddb->numLocations; i++)
	{
		ddb->locConnections[i] = ddb->data + ddb->conTable[i];
		ddb->locDescriptions[i] = ddb->data + ddb->locDescTable[i];
	}
	for (int i = 0; i < ddb->numMessages; i++)
	{
		ddb->messages[i] = ddb->data + ddb->msgTable[i];
	}
}

bool DDB_RequiresBackBuffer(DDB* ddb)
{
	if (ddb == 0)
		return false;

	for (int process = 0; process < ddb->numProcesses; process++)
	{
		uint8_t* entry = ddb->data + ddb->processTable[process];
		while (entry < ddb->data + ddb->dataSize)
		{
			if (*entry == 0)
				break;

			uint16_t offset = *(uint16_t*)(entry + 2);
			uint8_t* code = ddb->data + offset;
			if (code >= ddb->data + ddb->dataSize || code == ddb->data)
				break;
			entry += 4;

			while (*code != 0xFF)
			{
				uint8_t condactIndex = *code & 0x7F;
				uint8_t condact = ddb->condactMap[condactIndex].condact;
				int parameters = ddb->condactMap[condactIndex].parameters;
				uint8_t param1 = parameters > 1 ? code[2] : 0;

				if (condact == CONDACT_GFX && param1 <= 8)
					return true;

				if (condact == CONDACT_INDIR)
				{
					uint8_t* nextCode = code + parameters + 1;
					if (*nextCode != 0xFF)
					{
						uint8_t nextCondactIndex = *nextCode & 0x7F;
						uint8_t nextCondact = ddb->condactMap[nextCondactIndex].condact;
						if (nextCondact == CONDACT_GFX)
							return true;
					}
				}

				code += parameters + 1;

				if (condact == CONDACT_DONE || 
					condact == CONDACT_NOTDONE ||
					condact == CONDACT_OK ||
					condact == CONDACT_SKIP ||
					condact == CONDACT_RESTART ||
					condact == CONDACT_REDO)
					break;
			}
		}
	}

	return false;
}

#if HAS_PAWS

static bool CheckPAWSignature(uint8_t* memory, uint16_t base, uint16_t attr)
{
	return (
		base < 65535-321 &&
		base > 16384-311 &&
		memory[attr]    == 16 &&
		memory[attr+2]  == 17 &&
		memory[attr+4]  == 18 &&
		memory[attr+6]  == 19 &&
		memory[attr+8]  == 20 &&
		memory[attr+10] == 21
	);
}

static bool LoadPAWS(DDB* ddb, uint8_t* memory, size_t size)
{
	if (size < 65536) return false;

	uint16_t base = read16LE(memory + 65533);
	uint16_t attr = base + 311;
	if (CheckPAWSignature(memory, base, attr))
	{
		// PAWS
		ddb->data         = memory;
		ddb->dataSize     = size;
		ddb->memory       = memory;
		ddb->version      = DDB_VERSION_PAWS;
		// System message 30 holds the letter the interpreter accepts as a
		// positive answer, so it doubles as a reliable language marker:
		// English releases store "Y", Spanish ones "S".
		ddb->language     = DDB_SPANISH;
		if (memory[base + 327] > 30)
		{
			uint16_t sysMsg30 = read16LE(memory + read16LE(memory + 65505) + 30 * 2);
			if ((uint8_t)(memory[sysMsg30] ^ 0xFF) == 'Y')
				ddb->language = DDB_ENGLISH;
		}
		ddb->target       = DDB_MACHINE_SPECTRUM;
		ddb->machine      = DDB_MACHINE_SPECTRUM;
		ddb->condactMap   = pawsCondacts;
		ddb->littleEndian = true;
		ddb->firstToken   = 164;

		ddb->defaultBorder     = memory[base + 323];
		ddb->defaultInk		   = memory[base + 312];
		ddb->defaultPaper	   = memory[base + 314];
		ddb->defaultCharset	   = memory[base + 281];
		ddb->numObjects        = memory[base + 324];
		ddb->numLocations      = memory[base + 325];
		ddb->numMessages       = memory[base + 326];
		ddb->numSystemMessages = memory[base + 327];
		ddb->numProcesses      = memory[base + 328];
		ddb->numCharsets       = memory[base + 329];

		ddb->charsets          = memory + read16LE(memory + base + 330);
		ddb->tokens			   = memory + read16LE(memory + base + 332);
		ddb->processTable	   = (uint16_t*)(memory + read16LE(memory + 65497));
		ddb->objNamTable	   = (uint16_t*)(memory + read16LE(memory + 65499));
		ddb->locDescTable	   = (uint16_t*)(memory + read16LE(memory + 65501));
		ddb->msgTable		   = (uint16_t*)(memory + read16LE(memory + 65503));
		ddb->sysMsgTable	   = (uint16_t*)(memory + read16LE(memory + 65505));
		ddb->conTable	       = (uint16_t*)(memory + read16LE(memory + 65507));
		ddb->vocabulary		   = memory + read16LE(memory + 65509);
		ddb->objLocTable	   = memory + read16LE(memory + 65511);
		ddb->objWordsTable	   = memory + read16LE(memory + 65513);
		ddb->objAttrTable	   = memory + read16LE(memory + 65515);

		if (size > 65536)
		{
			int from = 0;
			for (int i = 0; i < 8 && from < ddb->numLocations ; i++)
			{
				uint8_t page = memory[base + 297 + i*2];
				uint8_t to   = memory[base + 298 + i*2];
				if (to == 255) to = ddb->numLocations;
				DebugPrintf("Page %d contains locations from %d to %d\n", page, from, to-1);
				uint8_t* base = ddb->data + (page > 0 ? 0x10000 + page * 0x4000 - 0xC000: 0);
				uint16_t locDescTable = read16LE(base + 65501);
				uint16_t locConnTable = read16LE(base + 65507);
				for (int n = from; n < to; n++)
				{
					ddb->locConnections[n] = base + read16LE(base + locConnTable + (n-from)*2);
					ddb->locDescriptions[n] = base + read16LE(base + locDescTable + (n-from)*2);
				}
				from = to;
			}
			from = 0;
			for (int i = 0; i < 8 && from < ddb->numMessages; i++)
			{
				uint8_t page = memory[base + 283 + i*2];
				uint8_t to   = memory[base + 284 + i*2];
				if (to == 255) to = ddb->numMessages;
				DebugPrintf("Page %d contains messages from %d to %d\n", page, from, to-1);
				uint8_t* base = ddb->data + (page > 0 ? 0x10000 + page * 0x4000 - 0xC000 : 0);
				uint16_t msgTable = read16LE(base + 65503);
				for (int n = from; n < to; n++)
					ddb->messages[n] = base + read16LE(base + msgTable + (n-from)*2);
				from = to;
			}

			ddb->conTable = 0;
			ddb->msgTable = 0;
			ddb->locDescTable = 0;
		}
		else
		{
			DDB_FillMsgPointers(ddb);
		}

		return true;
	}
	if (CheckPAWSignature(memory, base = 26931, attr = 27908))
	{
		// TODO: QUILL Version A
		return false;
	}
	if (CheckPAWSignature(memory, base = 27356, attr = 27525))
	{
		// TODO: QUILL Version C
		return false;
	}

	return false;
}

#endif

// Scan a raw container (a disk/tape image, or a loader binary read off a
// mounted disk) for an embedded DDB and, if a valid one is found, report its
// byte range and load address.
//
// A DDB is plain, position-fixed data: its internal offset tables are absolute
// addresses relative to a per-machine load address, and it stores its own end
// pointer, so it can be lifted out of any container where it sits contiguous
// and uncompressed. That is exactly how the original 8-bit disks and tapes
// store it -- so this recovers the database regardless of the copy-protected or
// unusual loader in front of it, which we never need to run.
static bool DDB_FindEmbeddedDDB(const uint8_t* container, uint32_t containerSize,
                                uint32_t* outOffset, uint32_t* outSize, uint16_t* outBaseOffset)
{
	if (containerSize < 36)
		return false;

	for (uint32_t o = 0; o + 36 <= containerSize; o++)
	{
		// DAAD signature in memory: version byte (1 or 2), machine nibble in the
		// high bits of the next byte, then the null-word char 0x5F ('_').
		uint8_t version = container[o];
		if ((version != 1 && version != 2) || container[o+2] != 0x5F)
			continue;

		DDB_Machine machine = (DDB_Machine)(container[o+1] >> 4);
		uint16_t base = DDB_GetBaseOffsetForMachine(machine);
		if (base == 0)
			continue;	// no known load address for this target (or IBM PC)

		uint32_t sizeFieldOffset = o + (version == 1 ? 30 : 32);
		if (sizeFieldOffset + 2 > containerSize)
			continue;

		// The database carries its end address; the length is that minus the
		// load address. All 8-bit targets are little-endian, but try both.
		for (int endianTry = 0; endianTry < 2; endianTry++)
		{
			bool littleEndian = (endianTry == 0);
			uint16_t declaredEnd = read16(container + sizeFieldOffset, littleEndian);
			if (declaredEnd <= base)
				continue;

			uint32_t logicalSize = (uint32_t)declaredEnd - base;
			if (logicalSize < DDB_GetMinimumHeaderSize((DDB_Version)version) ||
			    (uint64_t)o + logicalSize > containerSize)
				continue;

			// Validate the header offset tables against this candidate; this is
			// what rejects the false positives a 3-byte signature would let
			// through, and disambiguates when a disk holds several copies.
			DDB probe;
			MemClear(&probe, sizeof(probe));
			probe.version    = (DDB_Version)version;
			probe.target     = machine;
			probe.data       = (uint8_t*)container + o;
			probe.dataSize   = logicalSize;
			probe.baseOffset = base;
			probe.littleEndian = littleEndian;
			if (DDB_IsPlausibleHeaderLayout(&probe, littleEndian, declaredEnd))
			{
				*outOffset     = o;
				*outSize       = logicalSize;
				*outBaseOffset = base;
				return true;
			}
		}
	}
	return false;
}

DDB* DDB_Load(const char* filename)
{
	ddbError = DDB_ERROR_NONE;

	File* file = File_Open(filename, ReadOnly);
	if (file == 0)
	{
		ddbError = DDB_ERROR_FILE_NOT_FOUND;
		return 0;
	}

	uint64_t fileSize = File_GetSize(file);
	uint8_t* memory = 0;
	uint8_t* data = 0;
	bool containerLoaded = false;

	DDB* ddb = Allocate<DDB>("DDB", 1, true);
	if (ddb == 0)
	{
		ddbError = DDB_ERROR_OUT_OF_MEMORY;
		File_Close(file);
		Free(memory);
		return 0;
	}
	ddb->machine = DDB_MACHINE_IBMPC;

	#if HAS_PAWS
	if (DDB_PAWSIsSDB(file))
	{
		size_t sdbSize = 0;
		if (!DDB_PAWSLoadSDB(file, &memory, &sdbSize,
			&ddb->sdbMemoryModel, &ddb->sdbSegmentCount))
		{
			File_Close(file);
			Free(ddb);
			return 0;
		}
		File_Close(file);
		if (!LoadPAWS(ddb, memory, sdbSize))
		{
			Free(memory);
			Free(ddb);
			ddbError = DDB_ERROR_INVALID_FILE;
			return 0;
		}
		#if HAS_DRAWSTRING
		if (DDB_LoadPAWSGraphics(memory, sdbSize))
		{
			ddb->drawString = true;
			ddb->defaultInk = VID_GetInk();
			ddb->defaultPaper = VID_GetPaper();
		}
		#endif
		DDB_FixOffsets(ddb);
		DDB_FillTokenPointers(ddb);
		ddb->oldMainLoop = true;
		return ddb;
	}
	#endif

	#if HAS_SNAPSHOTS

	size_t ramSize;
	DDB_Machine snapshotMachine;
	bool shouldProbeSnapshot = DDB_HasSnapshotExtension(filename);
	if (shouldProbeSnapshot && DDB_LoadSnapshot(file, filename, &memory, &ramSize, &snapshotMachine))
	{
		File_Close(file);

		#if HAS_PAWS
		if (LoadPAWS(ddb, memory, ramSize))
		{
			#if HAS_DRAWSTRING
			if (DDB_LoadPAWSGraphics(memory, ramSize))
			{
				ddb->drawString = true;
				ddb->defaultInk = VID_GetInk();
				ddb->defaultPaper = VID_GetPaper();
				// DebugPrintf("Default Ink: %d, Paper: %d\n", ddb->defaultInk, ddb->defaultPaper);
			}
			#endif

			DDB_FixOffsets(ddb);
			DDB_FillTokenPointers(ddb);
			ddb->oldMainLoop = true;
			return ddb;
		}
		#endif

		if (GuessDDBOffsetFromSnapshot(memory, ramSize, snapshotMachine, ddb) == false)
		{
			DebugPrintf("Unable to guess offset from snapshot\n");
			ddbError = DDB_ERROR_FILE_NOT_SUPPORTED;
			Free(memory);
			Free(ddb);
			return 0;
		}

		#if HAS_DRAWSTRING
		if (DDB_LoadVectorGraphics(snapshotMachine, ddb->version, memory, ramSize))
			ddb->drawString = true;
		#endif

		data = ddb->data;
		ddb->machine = snapshotMachine;
	}
	else

	#endif

	{
		ddbError = DDB_ERROR_NONE;

		// Allow files larger than a single DDB: they may be a disk/tape image or
		// a loader binary with the database embedded inside, which we carve out
		// below when the plain offset-0 parse fails.
		if (fileSize > MAX_CONTAINER_SIZE)
		{
			ddbError = DDB_ERROR_INVALID_FILE;
			DebugPrintf("Invalid DDB file size for %s: %lu bytes (max %u)\n", filename, (unsigned long)fileSize, (unsigned)MAX_CONTAINER_SIZE);
			DDB_Warning("Invalid DDB file: too big (max size: %d)\n", MAX_CONTAINER_SIZE);
			File_Close(file);
			Free(ddb);
			return 0;
		}

		if (memory == 0)
			memory = Allocate<uint8_t>("DDB Contents", fileSize);
		if (memory == 0)
		{
			ddbError = DDB_ERROR_OUT_OF_MEMORY;
			File_Close(file);
			return 0;
		}
		// A failed snapshot probe (e.g. a .cas/.tap that isn't a real snapshot)
		// leaves the file position at EOF; rewind before reading the container.
		File_Seek(file, 0);
		if (File_Read(file, memory, fileSize) != fileSize)
		{
			ddbError = DDB_ERROR_READING_FILE;
			File_Close(file);
			Free(memory);
			Free(ddb);
			return 0;
		}
		File_Close(file);

		data          = memory;
		ddb->memory   = memory;
		ddb->data     = data;
		ddb->dataSize = fileSize;
		containerLoaded = true;
	}

	// Parse the header exactly once. DDB_ParseHeader byte-swaps parts of the data
	// in place (e.g. the extended object-attributes table for v2+), so calling it
	// twice would corrupt the database. For a container (or an oversized file)
	// that is not a bare DDB at offset 0, scan it for an embedded database and
	// parse the carved range instead.
	bool parsed = false;
	if (!containerLoaded || fileSize <= MAX_DDB_SIZE)
		parsed = DDB_ParseHeader(ddb, data, ddb->dataSize, ddb->dataSize);

	if (!parsed && containerLoaded)
	{
		uint32_t carveOffset = 0, carveSize = 0;
		uint16_t carveBase = 0;
		if (DDB_FindEmbeddedDDB(memory, (uint32_t)fileSize, &carveOffset, &carveSize, &carveBase))
		{
			ddbError        = DDB_ERROR_NONE;
			data            = memory + carveOffset;
			ddb->data       = data;
			ddb->dataSize   = carveSize;
			ddb->baseOffset = carveBase;
			DebugPrintf("Carved embedded DDB at offset 0x%X (%u bytes) from %s\n",
				(unsigned)carveOffset, (unsigned)carveSize, filename);
			parsed = DDB_ParseHeader(ddb, data, ddb->dataSize, ddb->dataSize);
		}
	}

	if (!parsed)
	{
		ddbError = ddbError == DDB_ERROR_NONE ? DDB_ERROR_INVALID_FILE : ddbError;
		Free(memory);
		Free(ddb);
		return 0;
	}
	ddb->condactMap = ddb->version == DDB_VERSION_1 ? version1Condacts : version2Condacts;

	DDB_FixOffsets(ddb);
	DDB_FillTokenPointers(ddb);
	DDB_FillMsgPointers(ddb);

	#if HAS_XMSG
	DDB_OpenXMessageFile(filename);
	#endif

	// Old databases may need a PAWS style flow, try to
	// detect if process table 0 is a responses table
	//
	// TODO: *All* version 1 databases use a old style
	// loop, so there should be no need for this. Double check!

	if (ddb->version == 1 && ddb->numProcesses > 0)
	{
		uint16_t offset = ddb->processTable[0];
		if (offset != 0)
		{
			uint8_t* ptr = ddb->data + offset;
			int entriesWithVerb = 0;
			while (*ptr != 0)
			{
				if (ptr[0] != 255)
					entriesWithVerb++;
				ptr += 4;
			}
			if (entriesWithVerb > 10)
				ddb->oldMainLoop = true;
		}
	}

	if (ddbError != DDB_ERROR_NONE)
	{
		Free(memory);
		Free(ddb);
		return 0;
	}

	#if HAS_DRAWSTRING
	// Commodore 64 vector graphics as a separate .CDG file next to the
	// database, loading flush against the color RAM base at 0xCC00 (the
	// graphics footer then lands at its fixed 0xCBED position)
	if (ddb->target == DDB_MACHINE_C64 && !ddb->drawString)
	{
		char cdgName[FILE_MAX_PATH];
		StrCopy(cdgName, sizeof(cdgName), filename);
		char* ext = (char*)StrRChr(cdgName, '.');
		if (ext != 0)
		{
			StrCopy(ext, sizeof(cdgName) - (ext - cdgName), ".CDG");
			File* cdg = File_Open(cdgName, ReadOnly);
			if (cdg == 0)
			{
				StrCopy(ext, sizeof(cdgName) - (ext - cdgName), ".cdg");
				cdg = File_Open(cdgName, ReadOnly);
			}
			if (cdg != 0)
			{
				uint64_t cdgSize = File_GetSize(cdg);
				if (cdgSize > 0 && cdgSize <= 0xCC00)
				{
					uint8_t* ram = Allocate<uint8_t>("C64 graphics RAM", 65536, true);
					if (ram != 0)
					{
						uint32_t base = (uint32_t)(0xCC00 - cdgSize);
						if (File_Read(cdg, ram + base, cdgSize) == cdgSize &&
						    DDB_LoadVectorGraphics(DDB_MACHINE_C64, ddb->version, ram, 65536))
							ddb->drawString = true;
						else
							Free(ram);
					}
				}
				File_Close(cdg);
			}
		}
	}

	// MSX releases ship the vector graphics either as an extracted .MDG
	// (a memory slice ending at 0xAFFF, like the one DDB_WriteVectorDatabase
	// produces) or, on original disks, as a companion BLOAD image next to
	// the database (PART11.BIN + PART12.BIN): the BLOAD payload ends in a
	// relocation stub that places the graphics flush against 0xAFFF, so
	// the database slice is recovered by finding the footer whose start
	// field equals its own base address
	if (ddb->target == DDB_MACHINE_MSX && !ddb->drawString)
	{
		char mdgName[FILE_MAX_PATH];
		StrCopy(mdgName, sizeof(mdgName), filename);
		char* ext = (char*)StrRChr(mdgName, '.');
		if (ext != 0)
		{
			StrCopy(ext, sizeof(mdgName) - (ext - mdgName), ".MDG");
			File* mdg = File_Open(mdgName, ReadOnly);
			if (mdg == 0)
			{
				StrCopy(ext, sizeof(mdgName) - (ext - mdgName), ".mdg");
				mdg = File_Open(mdgName, ReadOnly);
			}
			if (mdg != 0)
			{
				uint64_t mdgSize = File_GetSize(mdg);
				if (mdgSize > 0 && mdgSize <= 0xB000)
				{
					uint8_t* ram = Allocate<uint8_t>("MSX graphics RAM", 65536, true);
					if (ram != 0)
					{
						uint32_t base = (uint32_t)(0xB000 - mdgSize);
						if (File_Read(mdg, ram + base, mdgSize) == mdgSize &&
						    DDB_LoadVectorGraphics(DDB_MACHINE_MSX, ddb->version, ram, 65536))
							ddb->drawString = true;
						else
							Free(ram);
					}
				}
				File_Close(mdg);
			}
		}
		if (!ddb->drawString && ext != 0)
		{
			// Companion BLOAD image: same name with the last character of
			// the stem incremented (PART11.BIN -> PART12.BIN)
			StrCopy(mdgName, sizeof(mdgName), filename);
			ext = (char*)StrRChr(mdgName, '.');
			if (ext != 0 && ext > mdgName && ext[-1] == '1')
			{
				ext[-1] = '2';
				File* bin = File_Open(mdgName, ReadOnly);
				if (bin != 0)
				{
					uint64_t binSize = File_GetSize(bin);
					if (binSize > 26 && binSize <= 65536)
					{
						uint8_t* raw = Allocate<uint8_t>("MSX graphics BIN", (size_t)binSize);
						if (raw != 0)
						{
							if (File_Read(bin, raw, binSize) == binSize && raw[0] == 0xFE)
							{
								const uint8_t* payload = raw + 7;
								uint32_t psize = (uint32_t)(binSize - 7);
								for (uint32_t n = psize; n >= 0x13 + 32; n--)
								{
									uint16_t base = (uint16_t)(0xB000 - n);
									const uint8_t* footer = payload + n - 0x13;
									uint16_t fstart  = read16LE(footer + 2);
									uint16_t fending = read16LE(footer + 14);
									if (fstart != base || fending != 0xFFFF)
										continue;
									uint8_t* ram = Allocate<uint8_t>("MSX graphics RAM", 65536, true);
									if (ram != 0)
									{
										MemCopy(ram + base, payload, n);
										if (DDB_LoadVectorGraphics(DDB_MACHINE_MSX, ddb->version, ram, 65536))
										{
											ddb->drawString = true;
											break;
										}
										Free(ram);
									}
								}
							}
							Free(raw);
						}
					}
					File_Close(bin);
				}
			}
		}
	}

	// Spectrum DAAD releases may ship a separate .SDG. PAWS graphics are part
	// of the PAWS database and are binary-incompatible with SDG.
	if (ddb->target == DDB_MACHINE_SPECTRUM && ddb->version != DDB_VERSION_PAWS && !ddb->drawString)
	{
		char sdgName[FILE_MAX_PATH];
		StrCopy(sdgName, sizeof(sdgName), filename);
		char* ext = (char*)StrRChr(sdgName, '.');
		if (ext != 0)
		{
			StrCopy(ext, sizeof(sdgName) - (ext - sdgName), ".SDG");
			File* sdg = File_Open(sdgName, ReadOnly);
			if (sdg == 0)
			{
				StrCopy(ext, sizeof(sdgName) - (ext - sdgName), ".sdg");
				sdg = File_Open(sdgName, ReadOnly);
			}
			if (sdg != 0)
			{
				uint64_t sdgSize = File_GetSize(sdg);
				if (sdgSize > 0 && sdgSize <= 65536)
				{
					uint8_t* ram = Allocate<uint8_t>("Spectrum graphics RAM", 65536, true);
					if (ram != 0)
					{
						uint32_t base = (uint32_t)(65536 - sdgSize);
						if (File_Read(sdg, ram + base, sdgSize) == sdgSize &&
						    DDB_LoadVectorGraphics(DDB_MACHINE_SPECTRUM, ddb->version, ram, 65536))
							ddb->drawString = true;
						else
							Free(ram);
					}
				}
				File_Close(sdg);
			}
		}
	}
	#endif

	return ddb;
}

void DDB_Close(DDB* ddb)
{
	if (ddb == 0)
		return;
	#if HAS_XMSG
	DDB_CloseXMessageFile();
	DDB_FreeXMessageCache();
	#endif
	if (ddb->memory != 0)
		Free(ddb->memory);
	Free(ddb);
}

const char* DDB_DescribeLanguage(DDB_Language lang)
{
	switch (lang)
	{
		case DDB_ENGLISH: return "English"; break;
		case DDB_SPANISH: return "Spanish"; break;
		default:          return "Unknown"; break;
	}
}

const char* DDB_DescribeMachine(DDB_Machine machine)
{
	switch (machine)
	{
		case DDB_MACHINE_IBMPC:    return "IBM PC"; break;
		case DDB_MACHINE_SPECTRUM: return "ZX Spectrum"; break;
		case DDB_MACHINE_C64:      return "Commodore 64"; break;
		case DDB_MACHINE_CPC:      return "Amstrad CPC"; break;
		case DDB_MACHINE_MSX:      return "MSX"; break;
		case DDB_MACHINE_ATARIST:  return "Atari ST"; break;
		case DDB_MACHINE_AMIGA:    return "Amiga"; break;
		case DDB_MACHINE_PCW:      return "Amstrad PCW"; break;
		case DDB_MACHINE_PLUS4:    return "Commodore Plus/4"; break;
		case DDB_MACHINE_MSX2:     return "MSX2"; break;
		default:                   return "Unknown machine"; break;
	}
}

const char* DDB_DescribeVersion (DDB_Version version)
{
	switch (version)
	{
		case DDB_VERSION_PAWS: return "PAWS"; break;
		case DDB_VERSION_1: return "DAAD v1"; break;
		case DDB_VERSION_2: return "DAAD v2"; break;
		case DDB_VERSION_3: return "DAAD v3"; break;
		default:            return "Unknown version"; break;
	}
}
