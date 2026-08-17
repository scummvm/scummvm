#include <ddb_paw.h>

#if HAS_PAWS

#include <ddb_scr.h>
#include <ddb_data.h>
#include <ddb_vid.h>
#include <os_bito.h>
#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

enum
{
	SDB_HEADER_SIZE = 24,
	SDB_ENTRY_SIZE = 16,
};

uint32_t DDB_PAWSSDBCRC32(const uint8_t* data, size_t size)
{
	uint32_t crc = 0xFFFFFFFFU;
	for (size_t n = 0; n < size; n++)
	{
		crc ^= data[n];
		for (int bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ (0xEDB88320U & (uint32_t)-(int32_t)(crc & 1));
	}
	return crc ^ 0xFFFFFFFFU;
}

static bool SDBReadAt(File* file, uint64_t offset, void* data, size_t size)
{
	return File_Seek(file, offset) && File_Read(file, data, size) == size;
}

bool DDB_PAWSIsSDB(File* file)
{
	uint8_t magic[4];
	uint64_t position = File_GetPosition(file);
	bool result = SDBReadAt(file, 0, magic, sizeof(magic)) &&
		magic[0] == 'S' && magic[1] == 'D' && magic[2] == 'B' && magic[3] == 0x1A;
	File_Seek(file, position);
	return result;
}

bool DDB_PAWSLoadSDB(File* file, uint8_t** outputMemory, size_t* outputSize,
	uint8_t* outputModel, uint8_t* outputSegmentCount)
{
	if (file == 0 || outputMemory == 0 || outputSize == 0)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}
	uint8_t header[SDB_HEADER_SIZE];
	if (!SDBReadAt(file, 0, header, sizeof(header)) ||
		header[0] != 'S' || header[1] != 'D' || header[2] != 'B' || header[3] != 0x1A)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	uint8_t major = header[4];
	uint8_t minor = header[5];
	uint8_t model = header[6];
	uint8_t flags = header[7];
	uint16_t headerSize = read16LE(header + 8);
	uint16_t entrySize = read16LE(header + 10);
	uint16_t segmentCount = read16LE(header + 12);
	uint16_t reserved = read16LE(header + 14);
	uint32_t directoryOffset = read32LE(header + 16);
	uint32_t declaredSize = read32LE(header + 20);
	uint64_t fileSize = File_GetSize(file);

	if (major != 1 || minor != 0 || flags != 0 || reserved != 0 ||
		(model != DDB_SDB_48K && model != DDB_SDB_128K) ||
		headerSize != SDB_HEADER_SIZE || entrySize != SDB_ENTRY_SIZE ||
		segmentCount == 0 || segmentCount > DDB_SDB_MAX_SEGMENTS ||
		directoryOffset != SDB_HEADER_SIZE || declaredSize != fileSize ||
		(uint64_t)directoryOffset + (uint64_t)entrySize * segmentCount > fileSize)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	size_t memorySize = model == DDB_SDB_128K ? 0x30000 : 0x10000;
	uint8_t* memory = Allocate<uint8_t>("SDB Spectrum RAM", memorySize, true);
	uint8_t* used = Allocate<uint8_t>("SDB segment map", memorySize, true);
	if (memory == 0 || used == 0)
	{
		if (memory) Free(memory);
		if (used) Free(used);
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}

	bool seenLow[9] = { false };
	bool seenHigh[9] = { false };
	uint16_t lowLoad[9] = { 0 }, lowLength[9] = { 0 };
	uint16_t highLoad[9] = { 0 }, highLength[9] = { 0 };
	uint32_t nextPayloadOffset = directoryOffset + entrySize * segmentCount;
	bool valid = true;
	for (uint16_t n = 0; n < segmentCount && valid; n++)
	{
		uint8_t entry[SDB_ENTRY_SIZE];
		if (!SDBReadAt(file, directoryOffset + n * entrySize, entry, sizeof(entry)))
		{
			valid = false;
			break;
		}
		uint8_t bank = entry[0];
		uint8_t role = entry[1];
		uint16_t load = read16LE(entry + 2);
		uint16_t length = read16LE(entry + 4);
		uint16_t entryReserved = read16LE(entry + 6);
		uint32_t payloadOffset = read32LE(entry + 8);
		uint32_t expectedCRC = read32LE(entry + 12);
		int slot = model == DDB_SDB_48K ? 8 : bank;

		if (entryReserved != 0 || length == 0 || load < 0x4000 ||
			(uint32_t)load + length > 0x10000 ||
			(uint64_t)payloadOffset + length > fileSize ||
			payloadOffset != nextPayloadOffset ||
			(role != DDB_SDB_SEGMENT_LOW && role != DDB_SDB_SEGMENT_HIGH) ||
			(model == DDB_SDB_48K ? bank != 0xFF : bank > 7) ||
			(model == DDB_SDB_128K && bank != 0 && load < 0xC000) ||
			(role == DDB_SDB_SEGMENT_LOW ? seenLow[slot] : seenHigh[slot]))
		{
			valid = false;
			break;
		}
		nextPayloadOffset += length;

		uint8_t* payload = Allocate<uint8_t>("SDB segment", length);
		if (payload == 0 || !SDBReadAt(file, payloadOffset, payload, length) ||
			DDB_PAWSSDBCRC32(payload, length) != expectedCRC)
		{
			if (payload) Free(payload);
			valid = false;
			break;
		}

		for (uint32_t p = 0; p < length && valid; p++)
		{
			uint32_t address = load + p;
			uint32_t destination = address;
			if (model == DDB_SDB_128K && address >= 0xC000)
				destination = 0x10000 + bank * 0x4000 + address - 0xC000;
			if (used[destination])
				valid = false;
			else
			{
				used[destination] = 1;
				memory[destination] = payload[p];
			}
		}
		Free(payload);
		if (role == DDB_SDB_SEGMENT_LOW)
		{
			seenLow[slot] = true;
			lowLoad[slot] = load;
			lowLength[slot] = length;
		}
		else
		{
			seenHigh[slot] = true;
			highLoad[slot] = load;
			highLength[slot] = length;
		}
	}

	if (model == DDB_SDB_48K)
		valid = valid && seenLow[8] && seenHigh[8];
	else
	{
		valid = valid && seenLow[0] && seenHigh[0];
		for (int bank = 0; bank < 8; bank++)
			if (seenLow[bank] != seenHigh[bank]) valid = false;
		if (valid)
			MemCopy(memory + 0xC000, memory + 0x10000, 0x4000);
	}

	for (int slot = 0; slot < 9 && valid; slot++)
	{
		if (!seenLow[slot]) continue;
		uint8_t bank = model == DDB_SDB_48K ? 0 : (uint8_t)slot;
		const uint8_t* base = memory;
		if (model == DDB_SDB_128K && bank != 0)
			base = memory + 0x10000 + bank * 0x4000 - 0xC000;
		uint16_t expectedBase = bank == 0 ? 0x9300 : 0xC000;
		valid = lowLoad[slot] == expectedBase &&
			(uint32_t)lowLoad[slot] + lowLength[slot] == read16LE(base + 0xFFED) &&
			highLoad[slot] == read16LE(base + 0xFFEF) &&
			(uint32_t)highLoad[slot] + highLength[slot] == 0x10000 &&
			read16LE(base + 0xFFFD) == expectedBase && base[0xFFFF] == bank;
	}

	Free(used);
	if (!valid || nextPayloadOffset != fileSize || read16LE(memory + 0xFFFD) != 0x9300)
	{
		Free(memory);
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	*outputMemory = memory;
	*outputSize = memorySize;
	if (outputModel) *outputModel = model;
	if (outputSegmentCount) *outputSegmentCount = (uint8_t)segmentCount;
	return true;
}

bool DDB_PAWSWriteSDB(const char* filename, uint8_t model,
	const DDB_SDBSegment* segments, uint8_t segmentCount)
{
	uint8_t* data = 0;
	size_t size = 0;
	if (!DDB_PAWSBuildSDB(model, segments, segmentCount, &data, &size))
		return false;
	File* file = File_Create(filename);
	if (file == 0)
	{
		Free(data);
		DDB_SetError(DDB_ERROR_CREATING_FILE);
		return false;
	}
	bool ok = File_Write(file, data, size) == size;
	File_Close(file);
	Free(data);
	if (!ok) DDB_SetError(DDB_ERROR_WRITING_FILE);
	return ok;
}

bool DDB_PAWSBuildSDB(uint8_t model, const DDB_SDBSegment* segments,
	uint8_t segmentCount, uint8_t** outputData, size_t* outputSize)
{
	if (outputData == 0 || outputSize == 0)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}
	if ((model != DDB_SDB_48K && model != DDB_SDB_128K) || segments == 0 ||
		segmentCount == 0 || segmentCount > DDB_SDB_MAX_SEGMENTS)
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}
	for (uint8_t n = 0; n < segmentCount; n++)
	{
		const DDB_SDBSegment& segment = segments[n];
		if (segment.data == 0 || segment.length == 0 || segment.loadAddress < 0x4000 ||
			(uint32_t)segment.loadAddress + segment.length > 0x10000 ||
			(segment.role != DDB_SDB_SEGMENT_LOW && segment.role != DDB_SDB_SEGMENT_HIGH) ||
			(model == DDB_SDB_48K ? segment.bank != 0xFF : segment.bank > 7) ||
			(model == DDB_SDB_128K && segment.bank != 0 && segment.loadAddress < 0xC000))
		{
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			return false;
		}
	}

	uint32_t payloadOffset = SDB_HEADER_SIZE + SDB_ENTRY_SIZE * segmentCount;
	uint32_t fileSize = payloadOffset;
	for (uint8_t n = 0; n < segmentCount; n++)
		fileSize += segments[n].length;
	uint8_t* data = Allocate<uint8_t>("SDB file", fileSize, true);
	if (data == 0)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}

	uint8_t* header = data;
	header[0] = 'S'; header[1] = 'D'; header[2] = 'B'; header[3] = 0x1A;
	header[4] = 1; header[5] = 0; header[6] = model; header[7] = 0;
	write16(header + 8, SDB_HEADER_SIZE, true);
	write16(header + 10, SDB_ENTRY_SIZE, true);
	write16(header + 12, segmentCount, true);
	write16(header + 14, 0, true);
	write32(header + 16, SDB_HEADER_SIZE, true);
	write32(header + 20, fileSize, true);
	uint32_t nextPayload = payloadOffset;
	for (uint8_t n = 0; n < segmentCount; n++)
	{
		const DDB_SDBSegment& segment = segments[n];
		uint8_t* entry = data + SDB_HEADER_SIZE + n * SDB_ENTRY_SIZE;
		entry[0] = segment.bank;
		entry[1] = segment.role;
		write16(entry + 2, segment.loadAddress, true);
		write16(entry + 4, segment.length, true);
		write32(entry + 8, nextPayload, true);
		write32(entry + 12, DDB_PAWSSDBCRC32(segment.data, segment.length), true);
		MemCopy(data + nextPayload, segment.data, segment.length);
		nextPayload += segment.length;
	}
	*outputData = data;
	*outputSize = fileSize;
	return true;
}

void DDB_PAWSResetState(DDB_Interpreter* i)
{
	// A finished PAW game copies the Spectrum frame counter into $5C76 on
	// every game start/restart. Use ADP's normally time-seeded random source
	// for equivalent entropy; --random-seed keeps scripted tests exact.
	i->pawsRandomSeed = (uint16_t)RandInt(0, 0xFFFF);

	// A set bit means that the corresponding automatic location picture has
	// not yet been drawn. Drawing it clears the bit.
	MemSet(i->visited, 0xFF, DDB_PAWS_GRAPHIC_BYTES);
	i->flags[Flag_GraphicFlags] = 0;
	i->flags[Flag_TopLine] = 24;
	i->flags[Flag_PAWMode] = 0;
	i->flags[Flag_SplitLine] = 12;
	i->pawsControlParams = 0;
	i->pawsPermanentInk = i->ddb->defaultInk;
	i->pawsPermanentPaper = i->ddb->defaultPaper;
	i->pawsPermanentCharset = i->ddb->defaultCharset;
	i->pawsPermanentFlags = 0;
	i->pawsBorder = i->ddb->defaultBorder & 7;
	i->pauseTickRemainderMs = 0;
	i->timeoutTickRemainderMs = 0;
}

void DDB_PAWSSetMode(DDB_Interpreter* i, uint8_t mode, uint8_t options)
{
	i->flags[Flag_PAWMode] = (mode & 0x3F) | ((options & 0x03) << 6);
}

void DDB_PAWSSetLine(DDB_Interpreter* i, uint8_t line)
{
	i->flags[Flag_SplitLine] = (uint8_t)(24 - line);
}

void DDB_PAWSProtect(DDB_Interpreter* i)
{
	DDB_Flush(i);
	int row = i->win.posY / lineHeight;
	if (row < 0) row = 0;
	if (row > 24) row = 24;
	i->flags[Flag_TopLine] = (uint8_t)(24 - row);
}

void DDB_PAWSSetGraphic(DDB_Interpreter* i, uint8_t option)
{
	i->flags[Flag_GraphicFlags] = (option & 0x03) << 5;
}

void DDB_PAWSSetTime(DDB_Interpreter* i, uint8_t duration, uint8_t options)
{
	i->flags[Flag_Timeout] = duration;
	i->flags[Flag_TimeoutFlags] =
		(i->flags[Flag_TimeoutFlags] & 0xF8) | (options & 0x07);
}

void DDB_PAWSSetInput(DDB_Interpreter* i, uint8_t options)
{
	i->flags[Flag_TimeoutFlags] =
		(i->flags[Flag_TimeoutFlags] & 0xC7) | ((options & 0x07) << 3);
	// Generic bits 1/2 have the same accepted-line/timeout echo meanings.
	// PAW bit 0 selects bottom-of-screen input; it is not "clear window".
	i->inputFlags = options & 0x06;
}

uint8_t DDB_PAWSRandom(DDB_Interpreter* i)
{
	// B03 routine $8546 updates the Spectrum seed at $5C76 using only its
	// previous low byte. A zero high byte is rejected, with the updated seed
	// feeding the next attempt, so the public result is always in 1..100.
	uint8_t result;
	do
	{
		i->pawsRandomSeed = (uint16_t)(101u *
			(uint16_t)(((i->pawsRandomSeed & 0x00FFu) + 1u)));
		result = (uint8_t)(i->pawsRandomSeed >> 8);
	}
	while (result == 0);
	return result;
}

static void SetFullScreen(DDB_Interpreter* i, int row)
{
	DDB_Window* w = &i->win;
	w->x = 0;
	w->y = 0;
	w->width = screenWidth;
	w->height = screenHeight;
	w->posX = 0;
	w->posY = row * lineHeight;
	i->curwin = 0;
	i->windef[0] = *w;
	DDB_CalculateCells(i, w, &i->cellX, &i->cellW);
}

static uint8_t SetPAWSVideoAttributes(DDB_Interpreter* i, DDB_Window* w)
{
	uint8_t ink, paper;
	DDB_GetCurrentColors(i->ddb, w, &ink, &paper);
	uint8_t attributes = (ink & 0x07) | ((paper & 0x07) << 3) |
		((ink & 0x08) << 3) | ((ink & 0x10) << 3);
	SCR_SetAttributes(attributes);
	return paper;
}

void DDB_PAWSClear(DDB_Interpreter* i)
{
	// PAW's clear/open routine restores the permanent display state before
	// clearing.  The resulting top-left position also replaces the single
	// SAVEAT slot used by BACKAT.
	DDB_ResetPAWSColors(i, &i->win);
	SetFullScreen(i, 0);

	SetPAWSVideoAttributes(i, &i->win);
	DDB_ClearWindow(i, &i->win);
	// Spectrum ROM CLS upper ($0DAF) stores 1 in SCR_CT ($5C8C).  The first
	// attempted scroll must therefore expire the counter and enter PAW's
	// MODE-controlled pagination path.  Zero means "unknown" to ADP and would
	// incorrectly reload a whole page only after the screen already overflowed.
	i->win.scrollCount = 1;

	i->win.saveX = i->win.posX;
	i->win.saveY = i->win.posY;
	i->flags[Flag_TopLine] = 24;
	i->windef[0] = i->win;
}

void DDB_PAWSOutputAnyKeyMessage(DDB_Interpreter* i)
{
	// ANYKEY prints system message 16 through the Spectrum lower screen.  Its
	// leading newline advances from row 22 to the bottom row; using the main
	// PAW window here would instead invoke its protected-scroll boundary and
	// move the prompt up into row 21.
	DDB_Flush(i);
	DDB_Window* w = &i->windef[1];
	w->x = 0;
	w->y = screenHeight - 2 * lineHeight;
	w->width = screenWidth;
	w->height = 2 * lineHeight;
	w->posX = 0;
	w->posY = w->y;
	w->ink = i->win.ink;
	w->paper = i->win.paper;
	w->flags = i->win.flags | Win_NoMorePrompt;
	w->graphics = false;
	w->smooth = false;
	w->scrollCount = 0;
	DDB_OutputMessageToWindow(i, DDB_SYSMSG, 16, w);
	DDB_FlushWindow(i, w);
}

bool DDB_PAWSPrepareDescription(DDB_Interpreter* i, uint8_t location)
{
	SetFullScreen(i, 0);
	const uint8_t mode = i->flags[Flag_PAWMode] & 0x07;
	const uint8_t mask = (uint8_t)(1u << (location & 7));
	uint8_t* drawn = i->visited + (location >> 3);
	const bool redraw = (i->flags[Flag_GraphicFlags] & Graphics_Once) != 0;
	const bool off = (i->flags[Flag_GraphicFlags] & Graphics_Off) != 0;
	const bool always = (i->flags[Flag_GraphicFlags] & Graphics_On) != 0;
	const bool normal = (*drawn & mask) != 0;
	const bool eligible = DDB_HasVectorPicture(location);
	const bool draw = !off && mode != 1 && mode != 4 && eligible &&
		(always || redraw || normal);

	// Bit 7 requests one automatic attempt, not one successful drawing.
	i->flags[Flag_GraphicFlags] &= ~Graphics_Once;

	if (draw)
	{
		SCR_Clear(0, 0, screenWidth, screenHeight, VID_GetPaper());
		SCR_DrawVectorPicture(location);
		*drawn &= (uint8_t)~mask;

		if (mode == 0)
		{
			SCR_ConsumeBuffer();
			SCR_WaitForKey();
			DDB_PAWSClear(i);
			return true;
		}

		const int internal = DDB_PAWSValidatedInternalRow(i->flags[Flag_SplitLine]);
		const int row = 24 - internal;
		uint8_t clearPaper = SetPAWSVideoAttributes(i, &i->win);
		SCR_Clear(0, row * lineHeight, screenWidth,
			screenHeight - row * lineHeight, clearPaper);
		SetFullScreen(i, row);
		// The successful automatic-picture branch jumps over the flag-39 reset.
		// It captures the current internal cursor row before opening the stream;
		// at this point that is the LINE-selected split row.
		i->flags[Flag_TopLine] = (uint8_t)internal;
		i->win.scrollCount = 1;
		return true;
	}

	if (mode != 1)
	{
		DDB_PAWSClear(i);
	}
	return false;
}

void DDB_PAWSFinishDescription(DDB_Interpreter* i, bool automaticPictureDrawn)
{
	DDB_Flush(i);
	if (automaticPictureDrawn)
		return;
	const uint8_t mode = i->flags[Flag_PAWMode] & 0x07;
	if (mode == 4)
	{
		int row = i->win.posY / lineHeight;
		if (row < 0) row = 0;
		if (row > 24) row = 24;
		i->flags[Flag_TopLine] = (uint8_t)(24 - row);
	}
	else
	{
		i->flags[Flag_TopLine] = 24;
	}
}

int DDB_PAWSValidatedInternalRow(uint8_t row)
{
	return row >= 4 && row <= 24 ? row : 12;
}

int DDB_PAWSUserRow(uint8_t internalRow)
{
	return 24 - DDB_PAWSValidatedInternalRow(internalRow);
}

static void GetPAWSScrollRegion(DDB_Interpreter* i, int* topY, int* height)
{
	// Stream 2 uses the Spectrum's 22-row upper screen. Rows 22-23 belong to
	// the lower screen and are not part of ordinary PAW wrapping/scrolling.
	const int textHeight = screenHeight - 2 * lineHeight;
	int top = 0;
	if ((i->flags[Flag_PAWMode] & 0x07) >= 3)
		top = DDB_PAWSUserRow(i->flags[Flag_TopLine]) * lineHeight;
	if (top < 0) top = 0;
	if (top > textHeight - lineHeight) top = textHeight - lineHeight;
	*topY = top;
	*height = textHeight - top;
}

void DDB_PAWSRefreshWindow(DDB_Interpreter* i, DDB_Window* w)
{
	// The text region is derived from live flags: flag 39 is game-writable
	// and consulted at scroll time, so the window rect is refreshed here
	// instead of being cached when a condact changes it
	int topY, height;
	GetPAWSScrollRegion(i, &topY, &height);
	w->x = 0;
	w->width = screenWidth;
	w->y = (int16_t)topY;
	w->height = (int16_t)height;
}

void DDB_PAWSReloadScrollCounter(DDB_Interpreter* i, DDB_Window* w)
{
	// $8ACB subtracts the current internal S_POSN row (after its two lower
	// screen rows) from the normal/protected bottom boundary. In host row
	// coordinates this is the number of occupied rows in the usable region.
	int topY, height;
	GetPAWSScrollRegion(i, &topY, &height);
	int rows = (w->posY - topY) / lineHeight + 1;
	if (rows < 1) rows = 1;
	const int regionRows = height / lineHeight;
	if (rows > regionRows) rows = regionRows;
	w->scrollCount = (uint8_t)rows;
}

void DDB_PAWSStartPause(DDB_Interpreter* i, uint8_t ticks)
{
	i->pauseFrames = ticks == 0 ? 256 : ticks;
	i->pauseTickRemainderMs = 0;
	i->saveKeyToFlags = false;
	i->state = DDB_PAUSED;
}

bool DDB_PAWSAdvancePause(DDB_Interpreter* i, uint32_t elapsedMs)
{
	uint32_t total = i->pauseTickRemainderMs + elapsedMs;
	uint32_t ticks = total / DDB_PAWS_TICK_MS;
	i->pauseTickRemainderMs = total % DDB_PAWS_TICK_MS;
	if (ticks >= (uint32_t)i->pauseFrames)
	{
		i->pauseFrames = 0;
		return true;
	}
	i->pauseFrames -= ticks;
	return false;
}

void DDB_PAWSStartTimeout(DDB_Interpreter* i)
{
	i->timeout = true;
	i->timeoutRemainingMs = (int32_t)i->flags[Flag_Timeout] * DDB_PAWS_TIME_TICKS;
	i->timeoutTickRemainderMs = 0;
}

bool DDB_PAWSAdvanceTimeout(DDB_Interpreter* i, uint32_t elapsedMs)
{
	uint32_t total = i->timeoutTickRemainderMs + elapsedMs;
	uint32_t ticks = total / DDB_PAWS_TICK_MS;
	i->timeoutTickRemainderMs = total % DDB_PAWS_TICK_MS;
	if (ticks >= (uint32_t)i->timeoutRemainingMs)
	{
		i->timeoutRemainingMs = 0;
		return true;
	}
	i->timeoutRemainingMs -= ticks;
	return false;
}

#endif
