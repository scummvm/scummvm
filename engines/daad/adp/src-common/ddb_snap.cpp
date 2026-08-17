#if HAS_SNAPSHOTS

#define NOZIP_IMPLEMENTATION

#include <nozip.h>
#include <ddb.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>
#include <os_bito.h>

static uint8_t* snapshotRAM = 0;
static uint32_t snapshotSize = 0;
static uint8_t* snapshotDDB = 0;

static bool DetachSnapshot(uint8_t** ram, size_t* size, DDB_Machine* machine, DDB_Machine snapshotMachine)
{
	if (machine) *machine = snapshotMachine;
	if (ram) *ram = snapshotRAM;
	if (size) *size = snapshotSize;
	snapshotRAM = 0;
	snapshotSize = 0;
	snapshotDDB = 0;
	return true;
}

static bool CheckExtension(const char* filename, const char* ext)
{
	const char* p = StrRChr(filename, '.');
	if (!p) return false;
	return StrIComp(p + 1, ext) == 0;
}

static bool AllocateSnapshot(uint32_t size)
{
	if (snapshotRAM != 0 && snapshotSize >= size)
		return true;
	if (snapshotRAM != 0)
	{
		Free(snapshotRAM);
		snapshotRAM = 0;
		snapshotSize = 0;
		snapshotDDB = 0;
	}
	snapshotRAM = Allocate<uint8_t>("Snapshot RAM", size);
	if (snapshotRAM == 0)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}
	snapshotSize = size;
	return true;
}

// ----------------------------------------------------------------------------
//  STA Snapshot support (BlueMSX)
// ----------------------------------------------------------------------------

static bool LoadSnapshotFromSTA(File* file)
{
	ZIP_Entry* entries;
	int count = ZIP_Read(&entries, file);

	for (int n = 0; n < count; n++)
	{
		if (StrComp(entries[n].filename, "mapperNormalRam_00") == 0 &&
			entries[n].uncompressed_size == 65556)
		{
			snapshotRAM = Allocate<uint8_t>("Snapshot", 65556);
			if (snapshotRAM == 0)
			{
				DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
				return false;
			}
			snapshotSize = 65536;
			if (!ZIP_Extract(file, entries + n, snapshotRAM, 65556))
			{
				Free(snapshotRAM);
				snapshotRAM = 0;
				DDB_SetError(DDB_ERROR_READING_FILE);
				return false;
			}
			MemMove(snapshotRAM, snapshotRAM+20, 65536);
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------------
//  VSF Snapshot support
// ----------------------------------------------------------------------------

static bool LoadSnapshotFromVSF(File* file)
{
	static const char* VSFHeader = "VICE Snapshot File\x1A";

	char header[58];
	
	File_Seek(file, 0);
	if (File_Read(file, header, 58) != 58) 
	{
		DDB_SetError(DDB_ERROR_READING_FILE);
		return false;
	}
	if (StrComp(header, VSFHeader, 0x13) != 0)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}
	
	bool is128k = false;
	if (StrComp(header+0x15, "C128", 4) == 0)
		is128k = true;
	else if (StrComp(header+0x15, "C64", 3) != 0)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	uint64_t fileSize = File_GetSize(file);
	uint64_t position = File_GetPosition(file);
	for (;;)
	{
		if (position >= fileSize)
			return false;

		char moduleHeader[22];
		if (File_Read(file, moduleHeader, 22) != 22)
		{
			DDB_SetError(DDB_ERROR_READING_FILE);
			return false;
		}
		uint32_t moduleSize = read32LE((uint8_t*)moduleHeader + 18);
		if (StrComp(moduleHeader, "C64MEM") == 0)
		{
			File_Seek(file, position + 0x1A);
			snapshotRAM = Allocate<uint8_t>("Snapshot", 65536);
			if (snapshotRAM == 0)
			{
				DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
				return false;
			}
			snapshotSize = 65536;
			if (File_Read(file, snapshotRAM, 65536) != 65536)
			{
				Free(snapshotRAM);
				snapshotRAM = 0;
				DDB_SetError(DDB_ERROR_READING_FILE);
				return false;
			}
			return true;
		}
		else if (StrComp(moduleHeader, "C128MEM") == 0)
		{
			File_Seek(file, position + 0x1A);
			snapshotRAM = Allocate<uint8_t>("Snapshot", 131072);
			if (snapshotRAM == 0)
			{
				DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
				return false;
			}
			snapshotSize = 131072;
			if (File_Read(file, snapshotRAM, 131072) != 131072)
			{
				Free(snapshotRAM);
				snapshotRAM = 0;
				DDB_SetError(DDB_ERROR_READING_FILE);
				return false;
			}
			return true;
		}
		position += moduleSize;
		File_Seek(file, position);
	}
}

// ----------------------------------------------------------------------------
//  SNA Snapshot support
// ----------------------------------------------------------------------------

static bool LoadCPCSnapshotFromSNA(File* file)
{
	char header[8];
	
	File_Seek(file, 0);
	if (File_Read(file, header, 8) != 8)
		return false;
	if (StrComp(header, "MV - SNA", 8) != 0)
		return false;

	uint16_t memsize;
	File_Seek(file, 0x6b);
	if (File_Read(file, &memsize, 2) != 2)
	{
		DDB_SetError(DDB_ERROR_READING_FILE);
		return false;
	}
	memsize = fix16(memsize, true);
	if (memsize != 64 && memsize != 128)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	snapshotRAM = Allocate<uint8_t>("Snapshot", 65536);
	if (snapshotRAM == 0)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}
	snapshotSize = 65536;
	File_Seek(file, 0x100);
	if (File_Read(file, snapshotRAM, 65536) != 65536)
	{
		Free(snapshotRAM);
		snapshotRAM = 0;
		DDB_SetError(DDB_ERROR_READING_FILE);
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------------
//  ZX Spectrum SNA Snapshot support
// ----------------------------------------------------------------------------

#pragma pack(push, 1)
struct SNASnapshotHeader
{
	uint8_t	I;
	uint16_t HLp, DEp, BCp, AFp, HL, DE, BC, IY, IX;
	uint8_t interrupt, R;
	uint16_t AF, SP;
	uint8_t IM, border;
};
struct SNAP128KHeader
{
	uint16_t PC;
	uint8_t  pager, trDOSPage;
};
#pragma pack(pop)

static bool LoadSnapshotFromSNA (File* file)
{
	uint64_t fileSize = File_GetSize(file);

	if (fileSize == 131103 || fileSize == 147487)
	{
		// This is a 128K snapshot
		SNAP128KHeader header;

		AllocateSnapshot(0x30000);
		File_Seek(file, sizeof(SNASnapshotHeader));
		File_Read(file, snapshotRAM + 16384, 49152);
		File_Read(file, &header, sizeof(header));

		uint8_t page = header.pager & 7;
		MemCopy(snapshotRAM + 0x24000 /* page  5 */, snapshotRAM + 0x4000, 0x4000);
		MemCopy(snapshotRAM + 0x18000 /* page  2 */, snapshotRAM + 0x8000, 0x4000);
		MemCopy(snapshotRAM + 0x10000 + page*0x4000, snapshotRAM + 0xC000, 0x4000);
		for (int i = 0; i < 8; i++) 
		{
			if (i != page && i != 2 && i != 5)
				File_Read(file, snapshotRAM + 0x10000 + i * 0x4000, 0x4000);
		}

		// Select page 0
		MemCopy(snapshotRAM + 0xC000, snapshotRAM + 0x10000, 0x4000);

		return true;
	}

	if (fileSize != 49179)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	AllocateSnapshot(65536);
	File_Seek(file, sizeof(SNASnapshotHeader));
	File_Read(file, snapshotRAM + 16384, 49152);
	return true;
}

// ----------------------------------------------------------------------------
//  Z80 Snapshot support
// ----------------------------------------------------------------------------

#pragma pack(push, 1)
struct Z80SnapshotHeader
{
	uint8_t	A, F;
	uint8_t	C, B;
	uint8_t	L, H;
	uint8_t	PC_L, PC_H;
	uint8_t	SP_L, SP_H;
	uint8_t	I, R;
	uint8_t flags;
	uint8_t	E, D;
	uint8_t	C_, B_;
	uint8_t	E_, D_;
	uint8_t	L_, H_;
	uint8_t	A_, F_;
	uint8_t	IY_L, IY_H;
	uint8_t	IX_L, IX_H;
	uint8_t	IFF1, IFF2;
	uint8_t	flags2;	
};
struct Z80SnapshotExtraHeader
{
	uint16_t extraHeaderLength;
	uint16_t PC;
	uint8_t  mode;
	uint8_t  out7ffd;
	uint8_t  _if1paged;
	uint8_t  flags;
	
	// There are more info after this, but we don't care about it
};
#pragma pack(pop)

const int Z80FLAG_COMPRESSED = 0x20;

static void DecompressZ80Block (const uint8_t* ptr, const uint8_t* end, uint8_t* out, uint8_t* outEnd)
{
	while (ptr < end && out < outEnd)
	{
		if (ptr[0] == 0x00 && ptr <= end-4 && ptr[1] == 0xED && ptr[2] == 0xED && ptr[3] == 0x00)
			break;
		if (ptr[0] == 0xED && ptr <= end-4 && ptr[1] == 0xED)
		{
			uint8_t count = ptr[2];
			if (count == 0 || ptr == end-3)
				break;

			uint8_t value = ptr[3];
			for (int i = 0; i < count && out < outEnd; i++)
				*out++ = value;
			ptr += 4;
		}
		else
		{
			*out++ = *ptr++;
		}
	}
}

static bool IsZ8048K (int version, int mode)
{
	return version == 1 || (version == 2 && mode < 3) || (version == 3 && mode < 4);
}

static uint8_t* GetZ80RAMPage (int version, int mode, int page)
{
	// We don't care about SAMRAM stuff
	if (IsZ8048K(version, mode))
	{
		switch (page)
		{
			case 4: return snapshotRAM + 0x8000;
			case 5: return snapshotRAM + 0xC000;
			case 8: return snapshotRAM + 0x4000;
			default: return 0;
		}
	}
	else
	{
		switch (page)
		{
			case  3: return snapshotRAM + 0x10000;
			case  4: return snapshotRAM + 0x14000;
			case  5: return snapshotRAM + 0x18000;
			case  6: return snapshotRAM + 0x1C000;
			case  7: return snapshotRAM + 0x20000;
			case  8: return snapshotRAM + 0x24000;
			case  9: return snapshotRAM + 0x28000;
			case 10: return snapshotRAM + 0x2C000;
			default: return 0;
		}
	}

}

static bool LoadSnapshotFromZ80 (File* file)
{
	int version = 1;

	uint64_t fileSize = File_GetSize(file);
	if (fileSize > 200 * 1024)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	File_Seek(file, 0);

	uint8_t* data = Allocate<uint8_t>("Snapshot file", (size_t)fileSize);
	if (data == 0)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}
	if (File_Read(file, data, fileSize) != fileSize)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	Z80SnapshotHeader* header = (Z80SnapshotHeader*)data;
	Z80SnapshotExtraHeader* extraHeader = (Z80SnapshotExtraHeader*)(data + 30);
	uint8_t* start = data + 30;
	int mode = 0;
	if (header->flags == 0xFF)
		header->flags = 1;
	if (header->PC_H == 0 && header->PC_L == 0)
	{
		extraHeader->extraHeaderLength = fix16(extraHeader->extraHeaderLength, true);
		mode = extraHeader->mode;
		start += extraHeader->extraHeaderLength + 2;
		version = 2;
		if (extraHeader->extraHeaderLength > 23)
			version = 3;
	}

	// This allocates space for the ROM just for convenience
	AllocateSnapshot((IsZ8048K(version, mode) ? 64 : 192) * 1024);

	if (version == 1)
	{
		DecompressZ80Block(start, data + fileSize, snapshotRAM + 16384, snapshotRAM + 65536);
	}
	else
	{
		uint8_t* ptr = start;
		uint8_t* end = data + fileSize;
		while (ptr < end)
		{
			uint32_t length = read16(ptr, true);
			uint8_t page = ptr[2];
			uint8_t* out = GetZ80RAMPage(version, mode, page);
			bool compressed = true;

			ptr += 3;
			if (length == 0xFFFF)
			{
				compressed = false;
				length = 16384;
			}
			if (ptr + length > end)
				break;
			if (out != 0)
			{
				if (compressed)
					DecompressZ80Block(ptr, ptr + length, out, out + 16384);
				else
					DecompressZ80Block(ptr, ptr + length, out, out + 16384);
					// MemCopy(out, ptr, length);
			}
			ptr += length;
		}
	}

	if (!IsZ8048K(version, mode))
	{
		// Copy the 128K pages into 48K address space
		MemCopy(snapshotRAM + 0x4000, snapshotRAM + 0x24000, 0x4000);	// Page 5
		MemCopy(snapshotRAM + 0x8000, snapshotRAM + 0x18000, 0x4000);	// Page 2
		MemCopy(snapshotRAM + 0xC000, snapshotRAM + 0x10000, 0x4000);	// Page 0
	}

	Free(data);
	return true;
}

// ----------------------------------------------------------------------------
//  TAP Tape file support
// ----------------------------------------------------------------------------

#pragma pack(push, 1)
struct TAPBlockHeader
{
	uint8_t flag;
	uint8_t type;
	uint8_t name[10];
	uint16_t length;
	uint16_t param0;
	uint16_t param1;
	uint8_t checksum;
};
#pragma pack(pop)

static bool LoadSnapshotFromTAP (File* file)
{
	uint64_t fileSize = File_GetSize(file);
	uint64_t position = File_GetPosition(file);
	int count = 0;

	AllocateSnapshot(64 * 1024);

	while (position < fileSize)
	{
		uint16_t headerSize;
		if (File_Read(file, &headerSize, 2) != 2) 
		{
			DDB_SetError(DDB_ERROR_READING_FILE);
			return false;
		}
		fix16(headerSize, true);
		position += headerSize + 2;
		if (headerSize != sizeof(TAPBlockHeader))
		{
			File_Seek(file, position);
			continue;
		}

		TAPBlockHeader header;
		if (File_Read(file, &header, headerSize) != headerSize)
		{
			DDB_SetError(DDB_ERROR_READING_FILE);
			return false;
		}
		if (header.flag != 0)
			continue;

		const uint8_t* ptr = (const uint8_t*)&header;
		uint8_t checksum = 0;
		for (int i = 0; i < headerSize-1; i++)
			checksum ^= ptr[i];
		if (checksum != header.checksum)
		{
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			return false;
		}

		if (header.flag == 0 && header.type == 3)	// Header block for a CODE file
		{
			uint16_t dataAddress = fix16(header.param0, true);
			uint16_t length = fix16(header.length, true);
			if (dataAddress < 0x4000 || dataAddress + length > 65536)
			{
				DDB_SetError(DDB_ERROR_INVALID_FILE);
				return false;
			}

			File_Seek(file, position);
			uint16_t dataSize;
			if (File_Read(file, &dataSize, 2) != 2) 
			{
				DDB_SetError(DDB_ERROR_READING_FILE);
				return false;
			}
			fix16(dataSize, true);
			if (dataSize != length + 2)
			{
				DDB_SetError(DDB_ERROR_INVALID_FILE);
				return false;
			}

			File_Seek(file, position + 3);
			if (File_Read(file, snapshotRAM + dataAddress, length) != length)
			{
				DDB_SetError(DDB_ERROR_READING_FILE);
				return false;
			}
			position += dataSize + 2;
			File_Seek(file, position);
			count++;
		}
	}

	return count > 0;
}

// ----------------------------------------------------------------------------
//  TZX Tape file support
// ----------------------------------------------------------------------------
//
// We only support the bare minimum set of features from a TZX file.
// No turbo or fancy loaders! We just want a dump of the DDB data,
// without having to emulate the Spectrum to handle custom loaders,
// so we just load any CODE blocks in the tape and hope for the best.
// Unfortunately, a BASIC loader will often load code blocks in
// a different address compared to the one specified in its header,
// so even that may not work.
//
// There is currently a hack to load Cozumel data specifically (ugh).
// Jabato won't work at all since it has a custom loader.

static bool LoadSnapshotFromTZX (File* file)
{
	char header[10];

	File_Read(file, header, 10);
	if (StrComp(header, "ZXTape!\x1A", 8) != 0)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	AllocateSnapshot(128 * 1024);

	bool expectingDataBlock = false;
	bool loadingScreenFound = false;
	uint32_t dataAddress = 0;
	int blockCount = 0;

	uint64_t fileSize = File_GetSize(file);
	while (File_GetPosition(file) < fileSize)
	{
		uint8_t blockType;
		if (File_Read(file, &blockType, 1) != 1)
		{
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			return false;
		}

		switch (blockType)
		{
			case 0x10: // Normal speed data block
			{
				uint16_t pause;
				uint16_t length;
				File_Read(file, &pause, 2);
				File_Read(file, &length, 2);
				pause = fix16(pause, true);
				length = fix16(length, true);
				if (expectingDataBlock)
				{
					if (loadingScreenFound == false && length == 6914)
					{
						loadingScreenFound = true;
						dataAddress = 16384;
					}

					uint8_t flags, checksum;
					File_Read(file, &flags, 1);
					File_Read(file, snapshotRAM + dataAddress, length-2);
					File_Read(file, &checksum, 1);
					expectingDataBlock = false;
					blockCount++;
				}
				else if (length == 19)
				{
					uint8_t data[19];
					if (File_Read(file, data, 19) != 19)
					{
						DDB_SetError(DDB_ERROR_INVALID_FILE);
						return false;
					}
					if (data[0] == 0 && data[1] == 3)	// Header block for a CODE file
					{
						dataAddress = read16(data + 14, true);
						expectingDataBlock = true;
					}
				}
				else
				{
					if (length >= 32768 && blockCount > 0)
					{
						// Desperate attempt to find game data in a headerless
						// block. If the payload tail carries the SDG footer
						// signature (FF FF count 00 00) the block loads flush
						// against the top of memory, landing the footer at
						// 0xFFED (Espacial); otherwise use the historical
						// one-below placement (Cozumel)
						uint8_t tail[5];
						uint64_t payloadStart = File_GetPosition(file) + 1;
						File_Seek(file, payloadStart + (length-2) - 5);
						File_Read(file, tail, 5);
						File_Seek(file, payloadStart - 1);
						bool flushTop = tail[0] == 0xFF && tail[1] == 0xFF &&
						                tail[3] == 0x00 && tail[4] == 0x00;
						dataAddress = (flushTop ? 65536 : 65535) - (length-2);
						uint8_t flags, checksum;
						File_Read(file, &flags, 1);
						File_Read(file, snapshotRAM + dataAddress, length-2);
						File_Read(file, &checksum, 1);
					}
					else
					{
						File_Seek(file, File_GetPosition(file) + length);
					}
				}
				break;
			}

			case 0x12: // Pure tone
			{
				File_Seek(file, File_GetPosition(file) + 4);
				break;
			}

			case 0x13: // Pulse sequence
			{
				uint8_t pulseCount;
				File_Read(file, &pulseCount, 1);
				File_Seek(file, File_GetPosition(file) + pulseCount * 2);
				break;
			}

			case 0x14: // Pure data block (custom speed loaders)
			{
				// Same payload conventions as a standard block, but the
				// blocks carry two trailing bytes after the data instead
				// of a single checksum
				uint8_t blockHeader[10];
				File_Read(file, blockHeader, 10);
				uint32_t length = blockHeader[7] | (blockHeader[8] << 8) | (blockHeader[9] << 16);
				if (expectingDataBlock)
				{
					uint8_t flags;
					File_Read(file, &flags, 1);
					File_Read(file, snapshotRAM + dataAddress, length-3);
					File_Seek(file, File_GetPosition(file) + 2);
					expectingDataBlock = false;
					blockCount++;
				}
				else if (length == 20)
				{
					uint8_t data[20];
					if (File_Read(file, data, 20) != 20)
					{
						DDB_SetError(DDB_ERROR_INVALID_FILE);
						return false;
					}
					if (data[0] == 0 && data[1] == 3)	// Header block for a CODE file
					{
						dataAddress = read16(data + 14, true);
						expectingDataBlock = true;
					}
				}
				else if (length >= 32768 && blockCount > 0)
				{
					// Desperate attempt to find game data in a headerless block
					// (Cozumel part 2): assume it fills memory up to the top
					dataAddress = 65536 - (length-3);
					uint8_t flags;
					File_Read(file, &flags, 1);
					File_Read(file, snapshotRAM + dataAddress, length-3);
					File_Seek(file, File_GetPosition(file) + 2);
					blockCount++;
				}
				else
				{
					File_Seek(file, File_GetPosition(file) + length);
				}
				break;
			}

			case 0x20: // Pause / stop the tape
			{
				File_Seek(file, File_GetPosition(file) + 2);
				break;
			}

			case 0x2A: // Stop the tape in 48K mode
			{
				File_Seek(file, File_GetPosition(file) + 4);
				break;
			}

			case 0x2B: // Set signal level
			{
				File_Seek(file, File_GetPosition(file) + 5);
				break;
			}

			case 0x30: // Text description
			{
				// [1 byte: length][length bytes: text]
				uint8_t length;
				File_Read(file, &length, 1);
				File_Seek(file, File_GetPosition(file) + length);
				break;
			}

			case 0x31: // Message block
			{
				uint8_t length;
				File_Read(file, &length, 1);
				File_Seek(file, File_GetPosition(file) + length + 2);
				break;
			}

			case 0x32: // Archive info
			{
				uint16_t blockLength;
				File_Read(file, &blockLength, 2);
				blockLength = fix16(blockLength, true);
				File_Seek(file, File_GetPosition(file) + blockLength);
				break;
			}
			
			case 0x33: // Hardware type
			{
				uint8_t machineCount;
				File_Read(file, &machineCount, 1);
				File_Seek(file, File_GetPosition(file) + machineCount * 3);
				break;
			}

			default:
				DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
				return false;
		}
	}

	return blockCount > 0;
}

// ----------------------------------------------------------------------------
//  CDT Tape file support (Amstrad CPC)
// ----------------------------------------------------------------------------
//
// A .cdt is a TZX-family tape. CPC DAAD games use a custom turbo loader whose
// payload is a single large data block holding, contiguously, the text database
// (DDB) followed by the graphics database (GDB). At runtime the interpreter
// keeps the DDB at 0x2880 and relocates the GDB upward so it ends flush at
// 0xA600, placing the graphics footer at 0x9DEF and the charset at 0x9E00.
// We reconstruct that final RAM image so the
// standard snapshot loaders (GuessDDBOffsetFromSnapshot + DDB_LoadVectorGraphics)
// read both text and graphics unchanged.

#define CPC_DDB_BASE    0x2880
#define CPC_GFX_FOOTER  0x9DEF   // 'start' field of the graphics footer
#define CPC_GFX_ENDING  0x9DFB   // 0xFFFF sentinel
#define CPC_GFX_CHARS   0x9E00   // charset (fixed in this interpreter version)
#define CPC_GFX_END     0xA600   // First byte after the graphics database

// Mirror of DDB_LoadVectorGraphics()'s CPC validation applied at the fixed footer
// address: a candidate that passes here is guaranteed to load as graphics.
static bool CPC_GraphicsFooterValid(const uint8_t* mem, uint32_t size)
{
	if (size < CPC_GFX_END)
		return false;
	uint16_t start   = read16(mem + CPC_GFX_FOOTER, true);
	uint16_t table   = read16(mem + CPC_GFX_FOOTER + 2, true);
	uint16_t windefs = read16(mem + CPC_GFX_FOOTER + 4, true);
	uint16_t coltab  = read16(mem + CPC_GFX_FOOTER + 10, true);
	uint16_t ending  = read16(mem + CPC_GFX_ENDING, true);
	uint8_t  count   = mem[CPC_GFX_FOOTER + 14];
	if (ending != 0xFFFF) return false;
	if (table < start || CPC_GFX_CHARS < start || coltab < start) return false;
	if ((uint32_t)windefs + 8*count >= size) return false;
	if (mem[windefs + 8*count] != 0xFF) return false;
	return true;
}

static bool LoadSnapshotFromCDT(File* file)
{
	char header[10];
	File_Seek(file, 0);
	if (File_Read(file, header, 10) != 10 || StrComp(header, "ZXTape!\x1A", 8) != 0)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	// Scan the tape and remember the largest data block: the custom loader's
	// payload. The firmware records that carry the BASIC loader are all small.
	uint64_t fileSize = File_GetSize(file);
	uint64_t payloadOffset = 0;
	uint32_t payloadLength = 0;
	bool done = false;
	while (!done && File_GetPosition(file) < fileSize)
	{
		uint8_t blockType;
		if (File_Read(file, &blockType, 1) != 1)
			break;

		uint32_t dataLength = 0;
		uint32_t skipBefore = 0;
		bool     isData = false;
		switch (blockType)
		{
			case 0x10: { uint8_t h[4];  File_Read(file, h, 4);  dataLength = h[2] | (h[3]<<8); isData = true; break; }
			case 0x11: { uint8_t h[18]; File_Read(file, h, 18); dataLength = h[15] | (h[16]<<8) | (h[17]<<16); isData = true; break; }
			case 0x12: skipBefore = 4; break;                                              // pure tone
			case 0x13: { uint8_t n; File_Read(file,&n,1); skipBefore = 2*n; break; }        // pulse sequence
			case 0x14: { uint8_t h[10]; File_Read(file,h,10); dataLength = h[7]|(h[8]<<8)|(h[9]<<16); isData = true; break; } // pure data
			case 0x20: skipBefore = 2; break;                                              // pause
			case 0x21: { uint8_t l; File_Read(file,&l,1); skipBefore = l; break; }          // group start
			case 0x22: break;                                                              // group end
			case 0x2A: skipBefore = 4; break;                                              // stop the tape (48K)
			case 0x2B: skipBefore = 5; break;                                              // set signal level
			case 0x30: { uint8_t l; File_Read(file,&l,1); skipBefore = l; break; }          // text description
			case 0x31: { uint8_t d; File_Read(file,&d,1); uint8_t l; File_Read(file,&l,1); skipBefore = l; break; } // message
			case 0x32: { uint8_t l[2]; File_Read(file,l,2); skipBefore = l[0]|(l[1]<<8); break; } // archive info
			case 0x33: { uint8_t n; File_Read(file,&n,1); skipBefore = 3*n; break; }        // hardware type
			case 0x35: { uint8_t nm[16]; File_Read(file,nm,16); uint8_t l[4]; File_Read(file,l,4); skipBefore = l[0]|(l[1]<<8)|(l[2]<<16)|(l[3]<<24); break; } // custom info
			default:
				// Unknown block; stop scanning and use whatever payload we found.
				done = true;
				break;
		}
		if (done)
			break;

		uint64_t dataPos = File_GetPosition(file) + skipBefore;
		if (isData && dataLength > payloadLength)
		{
			payloadLength = dataLength;
			payloadOffset = dataPos;
		}
		File_Seek(file, dataPos + dataLength);
	}

	if (payloadLength == 0)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	uint8_t* payload = Allocate<uint8_t>("CDT payload", payloadLength);
	if (payload == 0)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}
	File_Seek(file, payloadOffset);
	if (File_Read(file, payload, payloadLength) != payloadLength)
	{
		Free(payload);
		DDB_SetError(DDB_ERROR_READING_FILE);
		return false;
	}

	// Locate the DDB signature (version 1/2, CPC machine nibble, 0x5F).
	uint32_t ddbOffset = 0;
	bool ddbFound = false;
	for (uint32_t o = 0; o + 3 < payloadLength; o++)
	{
		if ((payload[o] == 1 || payload[o] == 2) &&
		    (payload[o+1] >> 4) == DDB_MACHINE_CPC &&
		    payload[o+2] == 0x5F)
		{
			ddbOffset = o;
			ddbFound = true;
			break;
		}
	}
	if (!ddbFound || ddbOffset > CPC_DDB_BASE)
	{
		Free(payload);
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	// Place the payload so the DDB lands at 0x2880; the graphics sit contiguously
	// after it, not yet relocated.
	if (!AllocateSnapshot(65536))
	{
		Free(payload);
		return false;
	}
	MemClear(snapshotRAM, 65536);
	uint32_t base = CPC_DDB_BASE - ddbOffset;
	uint32_t copyLen = payloadLength;
	if (base + copyLen > 65536)
		copyLen = 65536 - base;
	MemCopy(snapshotRAM + base, payload, copyLen);
	Free(payload);

	// Replay the interpreter's LDDR relocation. Look for the graphics footer (the
	// 0xFFFF sentinel preceded by an ascending address table) above the DDB, shift
	// the graphics up so the sentinel lands at CPC_GFX_ENDING, and keep the first
	// placement that passes the graphics validation.
	uint8_t* work = Allocate<uint8_t>("CDT reloc", 65536);
	if (work == 0)
		return true; // text is still usable without the relocation
	for (uint32_t e = CPC_DDB_BASE + 12; e + 3 < CPC_GFX_ENDING; e++)
	{
		if (snapshotRAM[e] != 0xFF || snapshotRAM[e+1] != 0xFF)
			continue;
		uint16_t start   = read16(snapshotRAM + e - 12, true);
		uint16_t table   = read16(snapshotRAM + e - 10, true);
		uint16_t windefs = read16(snapshotRAM + e - 8, true);
		// The footer's address fields all point into the graphics DB (>= start,
		// <= charset); table and windefs are not ordered relative to each other.
		if (!(start >= 0x400 && start <= table && start <= windefs &&
		      table <= CPC_GFX_CHARS && windefs <= CPC_GFX_CHARS))
			continue;
		uint32_t shift = CPC_GFX_ENDING - e;
		if (shift == 0 || shift > 0x1000 || start < shift)
			continue;

		MemCopy(work, snapshotRAM, 65536);
		// Move [start-shift .. CPC_GFX_END-shift) up by shift (high bytes first).
		for (uint32_t i = CPC_GFX_END - start; i-- > 0; )
			work[start + i] = work[start - shift + i];
		if (CPC_GraphicsFooterValid(work, 65536))
		{
			MemCopy(snapshotRAM, work, 65536);
			break;
		}
	}
	Free(work);
	return true;
}

// ----------------------------------------------------------------------------
//  RAW file support
// ----------------------------------------------------------------------------

static void RelocateCPCGraphicsFromRAW(size_t ddbOffset)
{
	const uint16_t ddbBaseOffset = 0x2880;
	const uint16_t interpreterBase = 0x0840;
	const uint16_t graphicsTop = 0xA600;
	const uint16_t spareFieldOffset = 0x20;

	if (ddbOffset < ddbBaseOffset - interpreterBase)
		return;

	size_t loaderOffset = ddbOffset - (ddbBaseOffset - interpreterBase);
	if (loaderOffset + 5 > snapshotSize)
		return;

	// CPC interpreter binaries store the appended graphics length right after
	// the initial JP, patched in by the original builder.
	if (snapshotRAM[loaderOffset] != 0xC3)
		return;

	uint16_t gdlen = read16LE(snapshotRAM + loaderOffset + 3);
	if (gdlen == 0)
		return;

	if (ddbBaseOffset + spareFieldOffset + 2 > snapshotSize)
		return;

	uint16_t spare = read16LE(snapshotRAM + ddbBaseOffset + spareFieldOffset);
	uint16_t target = graphicsTop - gdlen;
	if (spare < ddbBaseOffset || spare > graphicsTop)
		return;
	if (target < ddbBaseOffset || target > graphicsTop)
		return;
	if ((size_t)spare + gdlen > snapshotSize || (size_t)target + gdlen > snapshotSize)
		return;

	if (spare != target)
		MemMove(snapshotRAM + target, snapshotRAM + spare, gdlen);
}

// MSX cassette image. DAAD tapes carry the database and the graphics database
// as raw blocks after the BASIC/loader ones. Two layouts exist: standard BIOS
// binary blocks whose payloads are staged and then relocated by a small LDIR
// stub at each block's exec address (emulated here), and custom loader tapes
// whose blocks hold the database directly and the graphics as a single image
// located by its footer.

#define MSX_GFX_FOOTER 0xAFED

static bool LoadSnapshotFromCAS(File* file)
{
	static const uint8_t sync[8] = { 0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74 };

	uint64_t fileSize = File_GetSize(file);
	if (fileSize < 64 || fileSize > 512*1024)
		return false;

	uint8_t* data = Allocate<uint8_t>("CAS contents", fileSize);
	if (data == 0)
		return false;
	File_Seek(file, 0);
	if (File_Read(file, data, fileSize) != fileSize || MemComp(data, sync, 8) != 0)
	{
		Free(data);
		return false;
	}

	// Collect sync block offsets
	uint32_t blocks[64];
	int blockCount = 0;
	for (uint32_t o = 0; o + 8 <= fileSize && blockCount < 64; o++)
	{
		if (MemComp(data + o, sync, 8) == 0)
		{
			blocks[blockCount++] = o;
			o += 7;
		}
	}

	if (!AllocateSnapshot(65536))
	{
		Free(data);
		return false;
	}
	MemClear(snapshotRAM, 65536);

	// Emulate the BIOS load: place each binary block (announced by a 0xD0
	// header block) at its load address, then apply the relocations performed
	// by the LDIR stub at its exec address.
	bool standardBlocks = false;
	for (int b = 1; b < blockCount; b++)
	{
		bool headerBefore = true;
		for (int k = 0; k < 10; k++)
		{
			if (data[blocks[b-1] + 8 + k] != 0xD0)
			{
				headerBefore = false;
				break;
			}
		}
		if (!headerBefore || blocks[b] + 14 >= fileSize)
			continue;

		uint32_t next = b + 1 < blockCount ? blocks[b+1] : (uint32_t)fileSize;
		uint16_t load = read16(data + blocks[b] + 8, true);
		uint16_t end  = read16(data + blocks[b] + 10, true);
		uint16_t exec = read16(data + blocks[b] + 12, true);
		if (end < load)
			continue;

		uint32_t avail = next - (blocks[b] + 14);
		uint32_t count = (uint32_t)end - load + 1;
		if (count > avail) count = avail;
		if ((uint32_t)load + count > 65536) count = 65536 - load;
		MemCopy(snapshotRAM + load, data + blocks[b] + 14, count);
		standardBlocks = true;

		if (exec > load && exec <= end)
		{
			// Look for LDIR stubs: LD HL,src / LD DE,dst / LD BC,len / LDIR
			for (uint32_t o = exec; o + 10 <= (uint32_t)end && o + 10 < 65536; o++)
			{
				if (snapshotRAM[o] == 0x21 && snapshotRAM[o+3] == 0x11 &&
				    snapshotRAM[o+6] == 0x01 && snapshotRAM[o+9] == 0xED && snapshotRAM[o+10] == 0xB0)
				{
					uint16_t src = read16(snapshotRAM + o + 1, true);
					uint16_t dst = read16(snapshotRAM + o + 4, true);
					uint16_t len = read16(snapshotRAM + o + 7, true);
					if (len != 0 && (uint32_t)src + len <= 65536 && (uint32_t)dst + len <= 65536)
						MemMove(snapshotRAM + dst, snapshotRAM + src, len);
					o += 10;
				}
			}
		}
	}

	// Check the result: a database signature at the base address means the
	// emulated load produced a usable memory image
	if (standardBlocks &&
	    (snapshotRAM[0x100] == 1 || snapshotRAM[0x100] == 2) &&
	    (snapshotRAM[0x101] >> 4) == DDB_MACHINE_MSX &&
	    snapshotRAM[0x102] == 0x5F)
	{
		Free(data);
		return true;
	}

	// Custom loader tapes: the database block holds the DDB directly and the
	// graphics block is located by the footer that, placed at the fixed
	// address, yields a start pointer equal to the resulting load base
	MemClear(snapshotRAM, 65536);

	uint32_t ddbOffset = 0, ddbEnd = 0;
	for (int b = 0; b < blockCount && ddbEnd == 0; b++)
	{
		uint32_t end = b + 1 < blockCount ? blocks[b+1] : (uint32_t)fileSize;
		for (uint32_t o = blocks[b] + 8; o < blocks[b] + 17 && o + 3 < end; o++)
		{
			if ((data[o] == 1 || data[o] == 2) &&
			    (data[o+1] >> 4) == DDB_MACHINE_MSX &&
			    data[o+2] == 0x5F)
			{
				ddbOffset = o;
				ddbEnd = end;
				break;
			}
		}
	}
	if (ddbEnd == 0)
	{
		Free(data);
		return false;
	}

	uint32_t ddbSize = ddbEnd - ddbOffset;
	uint16_t eof     = read16(data + ddbOffset + (data[ddbOffset] == 2 ? 0x20 : 0x1E), true);
	if (eof > 0x0100 + 32 && (uint32_t)eof - 0x0100 <= ddbSize)
		ddbSize = eof - 0x0100;
	else if ((uint32_t)0x0100 + ddbSize > 65536)
		ddbSize = 65536 - 0x0100;

	// Locate the graphics block and its load base
	uint32_t gfxOffset = 0, gfxSize = 0;
	uint16_t gfxBase = 0;
	for (int b = 0; b < blockCount && gfxOffset == 0; b++)
	{
		uint32_t end = b + 1 < blockCount ? blocks[b+1] : (uint32_t)fileSize;
		static const uint8_t payloadOffsets[3] = { 8, 9, 14 };
		for (int po = 0; po < 3 && gfxOffset == 0; po++)
		{
			uint32_t start = blocks[b] + payloadOffsets[po];
			uint32_t size = end - start;
			if (size < 32 || size > 65536)
				continue;
			for (uint32_t f = 0; f + 17 < size; f++)
			{
				if (data[start + f + 14] != 0xFF || data[start + f + 15] != 0xFF)
					continue;
				if (f > MSX_GFX_FOOTER)
					continue;
				uint32_t base = MSX_GFX_FOOTER - f;
				if (base + size > 65536)
					continue;
				const uint8_t* p = data + start + f;
				uint16_t gstart  = read16(p + 2,  true);
				uint16_t table   = read16(p + 4,  true);
				uint16_t windefs = read16(p + 6,  true);
				uint16_t chset   = read16(p + 10, true);
				uint16_t coltab  = read16(p + 12, true);
				if (gstart != base)
					continue;
				if (table < gstart || windefs < gstart || chset < gstart || coltab < gstart)
					continue;
				if (table >= base + size || windefs >= base + size || chset >= base + size)
					continue;
				gfxOffset = start;
				gfxSize   = size;
				gfxBase   = (uint16_t)base;
				break;
			}
		}
	}
	if (gfxOffset == 0)
	{
		Free(data);
		return false;
	}

	MemCopy(snapshotRAM + 0x0100, data + ddbOffset, ddbSize);
	MemCopy(snapshotRAM + gfxBase, data + gfxOffset, gfxSize);
	Free(data);
	return true;
}

// This loads a binary file into memory and tries to search for a valid DDB
// file inside. It is not very reliable, but may be the only option for
// some games which are stored in uncompressed formats.

bool LoadSnapshotFromRAW(File* file)
{
	uint64_t fileSize64 = File_GetSize(file);
	if (fileSize64 > 128 * 1024)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}
	if (fileSize64 < 32)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}
	size_t fileSize = (size_t)fileSize64;

	snapshotSize = fileSize > 65536 ? (size_t)fileSize : 65536;
	if (!AllocateSnapshot(snapshotSize))
		return false;
	MemClear(snapshotRAM, snapshotSize);

	if (File_Read(file, snapshotRAM, fileSize) != fileSize)
	{
		DDB_SetError(DDB_ERROR_READING_FILE);
		return false;
	}

	uint8_t* fileEnd = snapshotRAM + fileSize;
	for (size_t offset = 0; offset + 32 <= fileSize; offset++)
	{
		uint8_t *ptr = snapshotRAM + offset;
		if ((ptr[0] == 1 || ptr[0] == 2) && (ptr[2] == 0x5F))
		{
			DDB_Machine platform = (DDB_Machine)(ptr[1] >> 4);
			uint16_t baseOffset  = 0;
			switch (platform)
			{
				case DDB_MACHINE_SPECTRUM: baseOffset = 0x8400; break;
				case DDB_MACHINE_C64:      baseOffset = 0x3880; break;
				case DDB_MACHINE_CPC:      baseOffset = 0x2880; break;
				case DDB_MACHINE_MSX:      baseOffset = 0x0100; break;
				case DDB_MACHINE_PLUS4:    baseOffset = 0x7080; break;
				default: break;
			}
			if (baseOffset != 0)
			{
				if (offset + 0x18 > fileSize)
					continue;

				// Check the vocabulary table
				uint16_t voc = read16(ptr + 0x16, true);
				if (voc < baseOffset)
					continue;
				uint8_t* vocdata = ptr + voc - baseOffset;
				if (vocdata + 7 > fileEnd)
					continue;
				bool valid = true;
				bool spaces = false;
				bool endingZero = false;
				int wordCount = 0;
				while (vocdata + 7 <= fileEnd)
				{
					if (vocdata[0] == 0)	// Vocabulary must end with a zero
					{
						endingZero = true;
						break;
					}
					if (vocdata[6] > 6)		// Word type must be 0-6
					{
						valid = false;
						break;
					}
					if (vocdata[4] == 0xDF)	// Vocabulary must have at least one short word
						spaces = true;
					vocdata += 7;
					wordCount++;
				}
				if (wordCount < 16 || !spaces || !valid || !endingZero)
					continue;

				// Move the data into its base address. The image may end
				// before the top of RAM (e.g. a program file holding just
				// the interpreter and the databases); move what is there
				uint32_t remaining = 65536 - baseOffset;
				if (offset + remaining > fileSize)
					remaining = fileSize - offset;
				snapshotDDB = snapshotRAM + baseOffset;
				MemMove(snapshotDDB, ptr, remaining);
				if (platform == DDB_MACHINE_CPC)
					RelocateCPCGraphicsFromRAW(offset);
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------------
//  Loader
// ----------------------------------------------------------------------------

bool DDB_LoadSnapshot (File* file, const char* filename, uint8_t** ram, size_t* size, DDB_Machine* machine)
{
	if (file == 0)
		return 0;

	if (CheckExtension(filename, "z80"))
	{
		if (!LoadSnapshotFromZ80(file))
			return false;

		return DetachSnapshot(ram, size, machine, DDB_MACHINE_SPECTRUM);
	}
	else if (CheckExtension(filename, "sna"))
	{
		if (LoadCPCSnapshotFromSNA(file))
			return DetachSnapshot(ram, size, machine, DDB_MACHINE_CPC);
		if (LoadSnapshotFromSNA(file))
			return DetachSnapshot(ram, size, machine, DDB_MACHINE_SPECTRUM);
		return false;
	}
	else if (CheckExtension(filename, "tzx"))
	{
		if (!LoadSnapshotFromTZX(file))
			return false;

		return DetachSnapshot(ram, size, machine, DDB_MACHINE_SPECTRUM);
	}
	else if (CheckExtension(filename, "cdt"))
	{
		if (!LoadSnapshotFromCDT(file))
			return false;

		return DetachSnapshot(ram, size, machine, DDB_MACHINE_CPC);
	}
	else if (CheckExtension(filename, "sta"))
	{
		if (!LoadSnapshotFromSTA(file))
			return false;

		return DetachSnapshot(ram, size, machine, DDB_MACHINE_MSX);
	}
	else if (CheckExtension(filename, "vsf"))
	{
		if (!LoadSnapshotFromVSF(file))
			return false;

		return DetachSnapshot(ram, size, machine, DDB_MACHINE_C64);
	}
	else if (CheckExtension(filename, "tap"))
	{
		if (!LoadSnapshotFromTAP(file))
			return false;

		return DetachSnapshot(ram, size, machine, DDB_MACHINE_SPECTRUM);
	}
	else if (CheckExtension(filename, "cas") ||
			 CheckExtension(filename, "bin") ||
			 CheckExtension(filename, "rom") ||
			 CheckExtension(filename, "raw"))
	{
		if (CheckExtension(filename, "cas") && LoadSnapshotFromCAS(file))
			return DetachSnapshot(ram, size, machine, DDB_MACHINE_MSX);

		if (!LoadSnapshotFromRAW(file))
			return false;

		return DetachSnapshot(ram, size, machine, (DDB_Machine)(snapshotDDB[1] >> 4));
	}
	else
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_SUPPORTED);
		return false;
	}

	return false;
}

#endif