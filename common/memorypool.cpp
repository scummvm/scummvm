#include "common/memorypool.h"
#include <algorithm>

namespace Common
{

static const size_t CHUNK_PAGE_SIZE = 32;

void* MemoryPool::allocPage() {
  void* result = ::malloc(CHUNK_PAGE_SIZE * _chunkSize);
  _pages.push_back(result);
  void* current = result;
  for(size_t i=1; i<CHUNK_PAGE_SIZE; ++i)
  {
    void* next    = ((char*)current + _chunkSize);
    *(void**)current = next;
    
    current = next;
  }
  *(void**)current = NULL;
  return result;
}

MemoryPool::MemoryPool(size_t chunkSize) {
   // You must at least fit the pointer in the node (technically unneeded considering the next rounding statement)
  _chunkSize = std::max(chunkSize, sizeof(void*));
  // There might be an alignment problem on some platforms when trying to load a void* on a non natural boundary
  // so we round to the next sizeof(void*)
  _chunkSize = (_chunkSize + sizeof(void*) - 1) & (~(sizeof(void*) - 1));

  _next = NULL;
}

MemoryPool::~MemoryPool() {
  for(size_t i=0; i<_pages.size(); ++i)
    ::free(_pages[i]);
}

void* MemoryPool::malloc() {
#if 1
  if(!_next)
    _next = allocPage();

  void* result = _next;
  _next = *(void**)result;
  return result;
#else
  return ::malloc(_chunkSize);
#endif
}

void MemoryPool::free(void* ptr) {
#if 1
  *(void**)ptr = _next;
  _next = ptr;
#else
  ::free(ptr);
#endif
}

// Technically not compliant C++ to compare unrelated pointers. In practice...
bool MemoryPool::isPointerInPage(void* ptr, void* page) {
    return (ptr >= page) && (ptr < (char*)page + CHUNK_PAGE_SIZE * _chunkSize);
}

void MemoryPool::freeUnusedPages() {
  //std::sort(_pages.begin(), _pages.end());
  Array<size_t> numberOfFreeChunksPerPage;
  numberOfFreeChunksPerPage.resize(_pages.size());
  for(size_t i=0; i<numberOfFreeChunksPerPage.size(); ++i) {
      numberOfFreeChunksPerPage[i] = 0;
  }
  
  void* iterator = _next;
  while(iterator) {
    // This should be a binary search
    for(size_t i=0; i<_pages.size(); ++i) {
      if(isPointerInPage(iterator, _pages[i])) {
	++numberOfFreeChunksPerPage[i];
	break;
      }
    }
    iterator = *(void**)iterator;
  }

  size_t freedPagesCount = 0;
  for(size_t i=0; i<_pages.size(); ++i) {
    if(numberOfFreeChunksPerPage[i] == CHUNK_PAGE_SIZE) {
      ::free(_pages[i]);
      _pages[i] = NULL; // TODO : Remove NULL values
      ++freedPagesCount;
    }
  }

  printf("%d freed pages\n", freedPagesCount); 
}

}
