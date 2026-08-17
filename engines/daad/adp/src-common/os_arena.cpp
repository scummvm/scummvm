#include <os_mem.h>

#define BLOCK_SIZE 65536
static inline size_t AlignSize(size_t bytes)
{
	size_t align = sizeof(void*);
	return (bytes + (align - 1)) & ~(align - 1);
}

static Arena* AllocateArenaBlock(const char* reason, size_t bytes)
{
	Arena* block = Allocate<Arena>(reason);
	if (block == 0)
		return 0;

	size_t size = bytes > BLOCK_SIZE ? AlignSize(bytes) : BLOCK_SIZE;
	block->block = Allocate<uint8_t>(reason, (unsigned)size, false);
	if (block->block == 0)
	{
		Free(block);
		return 0;
	}

	block->name = reason;
	block->size = size;
	block->used = 0;
	block->current = block;
	block->next = 0;
	return block;
}

Arena* AllocateArena (const char* reason)
{
	Arena* arena = AllocateArenaBlock(reason, BLOCK_SIZE);
	if (arena != 0)
		arena->current = arena;
	return arena;
}

void FreeArena(Arena* arena)
{
	while (arena != 0)
	{
		Arena* next = arena->next;
		Free(arena->block);
		Free(arena);
		arena = next;
	}
}

void* AllocateBlock (Arena* arena, size_t bytes, bool zero)
{
	if (arena == 0)
		return 0;

	bytes = AlignSize(bytes);
	Arena* current = arena->current != 0 ? arena->current : arena;
	if (current->size - current->used < bytes)
	{
		Arena* next = AllocateArenaBlock(arena->name, bytes);
		if (next == 0)
			return 0;
		current->next = next;
		arena->current = next;
		current = next;
	}

	void* ptr = current->block + current->used;
	current->used += bytes;
	if (zero)
		MemClear(ptr, bytes);
	return ptr;
}
