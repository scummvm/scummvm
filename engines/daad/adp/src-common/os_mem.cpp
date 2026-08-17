#include <os_types.h>
#include <os_lib.h>
#include <os_mem.h>
#include <ddb_scr.h>
#include <ddb_vid.h>

#if DEBUG_ALLOCS

#define BLOCK_SIGNATURE 0xCAADDAAD

struct BlockInfo
{
	uint32_t signature;
	const char* reason;
	size_t size;
	BlockInfo* next;
	BlockInfo* prev;
};

BlockInfo* firstBlock = 0;
BlockInfo* lastBlock = 0;

#if DUMP_MEMORY
void CorruptedBlockChain();

static BlockInfo* FindLowestAddressBlockAbove(unsigned long address)
{
	BlockInfo* candidate = 0;
	for (BlockInfo* block = firstBlock; block != 0; block = block->next)
	{
		if (block->signature != BLOCK_SIGNATURE)
			CorruptedBlockChain();

		unsigned long blockAddress = (unsigned long)block;
		if (blockAddress <= address)
			continue;
		if (candidate == 0 || blockAddress < (unsigned long)candidate)
			candidate = block;
	}
	return candidate;
}

static void DumpMemoryMapToDebugPrintf(uint32_t totalFree, uint32_t largestBlock)
{
	uint32_t blockCount = 0;
	uint32_t payloadBytes = 0;
	uint32_t trackedBytes = 0;
	uint32_t gapBytes = 0;
	uint32_t largestGap = 0;
	unsigned long firstAddress = 0;
	unsigned long lastAddress = 0;
	unsigned long previousEnd = 0;

	DebugPrintf("Memory map begin\n");
	for (BlockInfo* block = FindLowestAddressBlockAbove(0); block != 0; block = FindLowestAddressBlockAbove((unsigned long)block))
	{
		unsigned long headerAddress = (unsigned long)block;
		unsigned long payloadAddress = (unsigned long)(block + 1);
		unsigned long blockSize = (unsigned long)block->size;
		unsigned long blockEnd = payloadAddress + blockSize;
		unsigned long trackedSize = blockEnd - headerAddress;

		if (blockCount == 0)
			firstAddress = headerAddress;
		else if (headerAddress > previousEnd)
		{
			uint32_t gap = (uint32_t)(headerAddress - previousEnd);
			gapBytes += gap;
			if (gap > largestGap)
				largestGap = gap;
			DebugPrintf("  GAP   %08lX..%08lX  size=%lu\n",
				previousEnd,
				headerAddress - 1,
				(unsigned long)gap);
		}

		DebugPrintf("  BLOCK %08lX..%08lX  payload=%08lX..%08lX  size=%lu  reason=%s\n",
			headerAddress,
			blockEnd - 1,
			payloadAddress,
			blockEnd - 1,
			blockSize,
			block->reason);

		blockCount++;
		payloadBytes += (uint32_t)blockSize;
		trackedBytes += (uint32_t)trackedSize;
		previousEnd = blockEnd;
		lastAddress = blockEnd;
	}

	if (blockCount == 0)
	{
		DebugPrintf("Memory map: no tracked allocation blocks\n");
	}
	else
	{
		DebugPrintf("Memory map summary: blocks=%lu payload=%lu tracked=%lu gaps=%lu largest-gap=%lu span=%lu\n",
			(unsigned long)blockCount,
			(unsigned long)payloadBytes,
			(unsigned long)trackedBytes,
			(unsigned long)gapBytes,
			(unsigned long)largestGap,
			(unsigned long)(lastAddress - firstAddress));
	}

	if (totalFree > 0 || largestBlock > 0)
	{
		DebugPrintf("Memory availability: free=%lu largest=%lu\n",
			(unsigned long)totalFree,
			(unsigned long)largestBlock);
	}
	DebugPrintf("Memory map end\n");
}
#endif

static void PrintDebugText(int x, int y, const char* text)
{
	VID_DrawTextSpan(x, y, (const uint8_t*)text, (uint16_t)StrLen(text), 15, 255);
}

static int MeasureDebugTextWidth(const char* text)
{
	int width = 0;
	for (int n = 0; text[n]; n++)
		width += charWidth[(uint8_t)text[n]];
	return width;
}

size_t GetMaxAllocatableBlockSize()
{
	size_t free = OSGetFree();
	if (free == 0)
		return 0;

	free &= ~(size_t)15;
	if (free <= sizeof(BlockInfo))
		return 0;

	// DEBUG_ALLOCS wraps every payload in a BlockInfo header before it reaches OSAlloc.
	return free - sizeof(BlockInfo);
}

void CorruptedBlockChain()
{
	PrintDebugText(0, 0, "Corrupted memory block chain");
	int y = 8;

	char buf[16];
	BlockInfo*r = firstBlock;
	while (r->signature == BLOCK_SIGNATURE)
	{
		LongToChar((size_t)r, buf, 16);
		PrintDebugText(0, y, "-");
		PrintDebugText(12, y, buf);
		PrintDebugText(32, y, r->reason);
		r = r->next;
		y += 8;
		if (y == 200) y = 8;
	}
	LongToChar((size_t)r, buf, 16);
	PrintDebugText(0, y, "*");
	PrintDebugText(12, y, buf);
	PrintDebugText(32, y, "*** CORRUPTED ***");
	Abort();
}

void SanityCheck()
{
	BlockInfo* r = firstBlock;
	while (r)
	{
		if (r->signature != BLOCK_SIGNATURE)
			CorruptedBlockChain();
		r = r->next;
	}
}

void* AllocateBlockInPool(const char* reason, size_t bytes, bool zero, OSMemoryPool pool)
{
	SanityCheck();

	BlockInfo* r = (BlockInfo*)OSAlloc(bytes + sizeof(BlockInfo), pool);
	if (r == 0)
	{
		DebugPrintf("ALLOC FAIL size=%lu zero=%d pool=%lu reason=%s\n",
			(unsigned long)bytes,
			zero ? 1 : 0,
			(unsigned long)pool,
			reason ? reason : "(null)");
		return r;
	}

	r->signature = BLOCK_SIGNATURE;
	r->reason    = reason;
	r->size      = bytes;
	r->next      = 0;
	r->prev      = lastBlock;
	lastBlock    = r;

	if (r->prev)
		r->prev->next = r;
	if (firstBlock == 0)
		firstBlock = r;

	if (zero)
		MemClear(r+1, bytes);

	#if DUMP_ALLOCS
	DebugPrintf("ALLOC head=%08lX payload=%08lX size=%lu zero=%d pool=%lu reason=%s\n",
		(unsigned long)r,
		(unsigned long)(r + 1),
		(unsigned long)bytes,
		zero ? 1 : 0,
		(unsigned long)pool,
		reason ? reason : "(null)");
	#endif

	return r+1;
}

void* AllocateBlock(const char* reason, size_t bytes, bool zero)
{
	return AllocateBlockInPool(reason, bytes, zero, OSMemoryPool_Any);
}

void Free(void* block)
{
	SanityCheck();

	BlockInfo* r = (BlockInfo*)block - 1;
	if (r->signature == BLOCK_SIGNATURE)
	{
		#if DUMP_ALLOCS
		DebugPrintf("FREE  head=%08lX payload=%08lX size=%lu reason=%s\n",
			(unsigned long)r,
			(unsigned long)(r + 1),
			(unsigned long)r->size,
			r->reason ? r->reason : "(null)");
		#endif

		if (r->prev) r->prev->next = r->next;
		if (r->next) r->next->prev = r->prev;
		if (firstBlock == r) firstBlock = r->next;
		if (lastBlock == r) lastBlock = r->prev;
		OSFree(r);
		return;
	}

	//fputs("WRONG free: no block!\n\r", stderr);
	Abort();
}

void DumpMemory(uint32_t totalFree, uint32_t largestBlock, uint32_t stackUsed, uint32_t stackTotal)
{
	SanityCheck();
	
	#if DUMP_MEMORY
	DumpMemoryMapToDebugPrintf(totalFree, largestBlock);
	#endif

	BlockInfo* r = firstBlock;
	uint32_t totalAllocated = 0;

	int y = 16;

	while (r)
	{
		if (r->signature != BLOCK_SIGNATURE)
			CorruptedBlockChain();

		totalAllocated += (uint32_t)r->size;

		BlockInfo* prev = r->prev;
		while (prev != 0)
		{
			if (prev->reason == r->reason)
				break;
			prev = prev->prev;
		}

		if (prev == 0)
		{
			uint32_t count = 1;
			uint32_t total = (uint32_t)r->size;
			BlockInfo* next = r->next;
			while (next != 0)
			{
				if (next->signature != BLOCK_SIGNATURE)
					CorruptedBlockChain();

				if (next->reason == r->reason)
				{
					count++;
					total += (uint32_t)next->size;
				}
				next = next->next;
			}

			VID_Clear(6, y, 230, 8, 0, Clear_All);
			
			char buf[16];
			LongToChar(count, buf, 10);
			PrintDebugText(8, y, r->reason);
			LongToChar(count, buf, 10);
			PrintDebugText(180 - MeasureDebugTextWidth(buf), y, buf);
			LongToChar(total, buf, 10);
			PrintDebugText(234 - MeasureDebugTextWidth(buf), y, buf);
			y += 8;
			if (y >= 180) break;
		}

		r = r->next;
	}

	VID_Clear(  5,  14,   3, y-12, 0, Clear_All);
	VID_Clear(234,  14,   3, y-12, 0, Clear_All);
	VID_Clear(  5,  13, 232,    3, 0, Clear_All);
	VID_Clear(  5,   y, 232,    3, 0, Clear_All);

	VID_Clear(  6,  15,   1, y-14, 15, Clear_All);
	VID_Clear(235,  15,   1, y-14, 15, Clear_All);
	VID_Clear(  6,  14, 230,    1, 15, Clear_All);
	VID_Clear(  6, y+1, 230,    1, 15, Clear_All);

	y += 10;

	char buf[64];
	buf[0] = 0;
	if (totalFree > 0)
	{
		LongToChar(totalFree / 1024, buf, 10);
		StrCat(buf, sizeof(buf), "K free");
	}
	if (totalAllocated > 0)
	{
		if (buf[0] != 0)
			StrCat(buf, sizeof(buf), " | ");
		LongToChar(totalAllocated / 1024, buf + StrLen(buf), 10);
		StrCat(buf, sizeof(buf), "K alloc");
	}
	if (largestBlock > 0 && largestBlock != totalFree)
	{
		if (buf[0] != 0)
			StrCat(buf, sizeof(buf), " | ");
		LongToChar(largestBlock / 1024, buf + StrLen(buf), 10);
		StrCat(buf, sizeof(buf), "K largest");
	}
	if (stackTotal > 0)
	{
		if (buf[0] != 0)
			StrCat(buf, sizeof(buf), " | ");
		LongToChar(stackUsed / 1024, buf + StrLen(buf), 10);
		StrCat(buf, sizeof(buf), "/");
		LongToChar(stackTotal / 1024, buf + StrLen(buf), 10);
		StrCat(buf, sizeof(buf), "K stack");
	}
	VID_Clear(6, 2, MeasureDebugTextWidth(buf), 8, 0, Clear_All);
	VID_DrawTextSpan(6, 2, (const uint8_t*)buf, (uint16_t)StrLen(buf), 0, 15);
}

#endif
