#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "sci/include/engine.h"
#include "sci/include/console.h"
#include "sci/engine/heap.h"

#define assert_in_range(pos) assert(pos>=1000 && pos<=0xffff)

static void set_size(heap_t *h, int block_pos, int size)
{
	assert_in_range(block_pos);
	assert(size<=0xffff-1000);
	putInt16(h->start+block_pos, size);
}

static void set_next(heap_t* h, int block_pos, int next)
{
	assert_in_range(block_pos);
	assert_in_range(next);
	putInt16(h->start+block_pos+2, next);
}


static unsigned int get_size(heap_t* h, int block_pos)
{
	assert_in_range(block_pos);
	return (guint16)getInt16(h->start+block_pos);
}

static unsigned int get_next(heap_t* h, int block_pos)
{
	assert_in_range(block_pos);
	return (guint16)getInt16(h->start+block_pos+2);
}

/*Allocates a new heap*/
heap_t* heap_new()
{
	heap_t* h;
	if((h= (heap_t*)sci_malloc(sizeof(heap_t)))==0) return 0;

	if((h->start= sci_calloc(SCI_HEAP_SIZE, 1))==0)
	{
		free(h);
		return 0;
	}

	h->base=h->start+1000;
	h->first_free=1000;
	h->old_ff=-1;
	set_size(h, 1000, 0xffff-1000);
	set_next(h, 1000, 0xffff);

	return h;
}

/*Deletes a heap*/
void heap_del(heap_t* h)
{
	free(h->start);
	free(h);
}


int heap_meminfo(heap_t* h)
{
	heap_ptr current = h->first_free;
	int total = 0;

	while (current != 0xffff) {
		total += get_size(h, current);
		current = get_next(h, current);
	}

	return total;
}


int heap_largest(heap_t* h)
{
	int current=h->first_free;
	int best_pos=-1, best_size=0;

	while(current!=0xffff)
	{
		int size=get_size(h, current);
		int next=get_next(h, current);

		if(size>best_size)
		{
			best_pos=current;
			best_size=size;
		}

		current=next;
	}

	return best_size;
}

heap_ptr heap_allocate(heap_t* h, int size)
{
	unsigned int previous=h->first_free;
	unsigned int current=previous;

	if (!size) {
		fprintf(stderr,"Warning: heap_alloc'd zero bytes!\n");
		size += 2;
	}

	size+=2+(size&1);

	while(current<0xffff)
	{
		int block_size=get_size(h, current);
		int next=get_next(h, current);

		/*Is this block large enough?*/
		if(block_size>=size)
		{
			/*Swallow the block whole*/
			if(block_size<=size+4)
			{
				size=block_size;
				set_next(h, previous, next);
			}
			else {
				/*Split the block*/
				int rest=current+size;

				set_next(h, previous, rest);
				set_size(h, rest, block_size-size);
				set_next(h, rest, next);
				next=rest;
			}
			set_size(h, current, size);
			if(current==h->first_free) h->first_free=next;
			return current;
		}
		previous=current;
		current=next;
	}

	/*No large enough block was found.*/
	return 0;
}

void heap_free(heap_t* h, unsigned int m)
{
	unsigned int previous, next;
	assert_in_range(m);
	previous=next=h->first_free;

	/*Find the previous and next blocks*/
	while(next < m)
	{
		previous = next;
		assert(previous<0xffff);
		next=get_next(h, previous);
		if (next <= previous) {
		  sciprintf("Heap corrupt. Aborting heap_free()...\n");
		  return;
		}
	}

	if (h->first_free > m)
	  h->first_free = m; /* Guarantee that first_free is correct */

	if(previous==next)
	{
		if(m<previous)
		{
			h->first_free=m;
			if(m+get_size(h, m)==previous)
			{
				set_size(h, m, get_size(h, m)+get_size(h, previous));
				set_next(h, m, get_next(h, previous));
			}
			else set_next(h, m, previous);
		}
		else
		{
			if(previous+get_size(h, previous)==m)
			{
				set_size(h, previous, get_size(h, previous)+get_size(h, m));
				set_next(h, previous, 0xffff);
			}
			else
			{
				set_next(h, previous, m);
				set_next(h, m, next);
			}
		}
	}
	else
	{
		set_next(h, previous, m);
		set_next(h, m, next);

		/*Try to merge with previous*/
		if(previous+get_size(h, previous)==m)
		{
			set_size(h, previous, get_size(h, previous)+get_size(h, m));
			set_next(h, previous, next);
			m=previous;
		}

		/*Try to merge with next*/
		if(m+get_size(h, m)==next)
		{
			set_size(h, m, get_size(h, m)+get_size(h, next));
			set_next(h, m, get_next(h, next));
		}
	}
}

void save_ff(heap_t* h)
{
	h->old_ff=h->first_free;
}

void restore_ff(heap_t* h)
{
	h->first_free=h->old_ff;
	set_size(h, h->first_free, 0xffff-h->first_free);
	set_next(h, h->first_free, 0xffff);
}



void heap_dump_free(heap_t *h)
{
	int freedomseeker;

	printf("\tfirst_free= %#x (oldff= %#x)\n\tFree Blocks:\n", h->first_free, h->old_ff);

	freedomseeker = h->first_free;
	while (freedomseeker != 0xffff) {
		printf("\t   %#04x: size: %#04x\n", freedomseeker, get_size(h, freedomseeker));
		freedomseeker = get_next(h, freedomseeker);
	}
}

void heap_dump_all(heap_t *h)
{
	int seeker = 1000;
	int free_seeker = h->first_free;

	while (seeker < 0xffff) {
		int is_free = (seeker == free_seeker);
		int size = get_size(h, seeker);

		if (is_free)
			free_seeker = get_next(h, free_seeker);

		printf("%04x\t%d\t%s\n", seeker, size, is_free? "FREE": "");
		seeker += size;
	}
}

/*

int main(int argc, char **argv) {
  heap_t *h = heap_new();
  int a,b,c,d,e;

  printf("Running heap tests:\nHeap initialization:\n");
  heap_dump_free(h);

  printf("[a] Allocating 0x1: position is %#x\n", a = heap_allocate(h, 1));
  heap_dump_free(h);

  printf("[b] Allocating 0x10: position is %#x\n", b = heap_allocate(h, 0x10));
  printf("[c] Allocating 0x10: position is %#x\n", c = heap_allocate(h, 0x10));
  printf("[d] Allocating 0x10: position is %#x\n", d = heap_allocate(h, 0x10));
  printf("[e] Allocating 0x1000: position is %#x\n", e = heap_allocate(h, 0x1000));
  heap_dump_free(h);

  printf("Freeing [b]:\n");
  heap_free(h, b);
  heap_dump_free(h);

  printf("Freeing [d]:\n");
  heap_free(h, d);
  heap_dump_free(h);

  printf("Freeing [c]:\n");
  heap_free(h, c);
  heap_dump_free(h);

  heap_del(h);

  return 0;
}

*/
