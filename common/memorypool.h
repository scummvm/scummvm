#ifndef COMMON_MEMORYPOOL_H
#define COMMON_MEMORYPOOL_H

#include <cstring>
#include "common/array.h"

namespace Common
{

class MemoryPool
{
 private:
  MemoryPool(const MemoryPool&);
  MemoryPool& operator=(const MemoryPool&);

  size_t       _chunkSize;
  Array<void*> _pages;
  void*        _next;

  void* allocPage();
  bool  isPointerInPage(void* ptr, void* page);
 public:
  MemoryPool(size_t chunkSize);
  ~MemoryPool();

  void* malloc();
  void  free(void* ptr);

  void freeUnusedPages();
};

}

#endif
