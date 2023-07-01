/***************************************************************************/
/*                                                                         */
/*  ftdbgmem.c                                                             */
/*                                                                         */
/*    Memory debugger (body).                                              */
/*                                                                         */
/*  Copyright 2001, 2002 by                                                */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/config/ftconfig.h"
#include FT2_1_3_INTERNAL_DEBUG_H
#include FT2_1_3_INTERNAL_MEMORY_H
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftsystem.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fterrors.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/fttypes.h"


#ifdef FT2_1_3_DEBUG_MEMORY


#include <stdio.h>
#include <stdlib.h>


typedef struct FT2_1_3_MemNodeRec_*   FT2_1_3_MemNode;
typedef struct FT2_1_3_MemTableRec_*  FT2_1_3_MemTable;

#define FT2_1_3_MEM_VAL( addr )  ((FT2_1_3_ULong)(FT2_1_3_Pointer)( addr ))

typedef struct  FT2_1_3_MemNodeRec_ {
	FT2_1_3_Byte*     address;
	FT2_1_3_Long      size;     /* < 0 if the block was freed */

	const char*  alloc_file_name;
	FT2_1_3_Long      alloc_line_no;

	const char*  free_file_name;
	FT2_1_3_Long      free_line_no;

	FT2_1_3_MemNode   link;

} FT2_1_3_MemNodeRec;


typedef struct  FT2_1_3_MemTableRec_ {
	FT2_1_3_ULong         size;
	FT2_1_3_ULong         nodes;
	FT2_1_3_MemNode*      buckets;

	FT2_1_3_ULong         alloc_total;
	FT2_1_3_ULong         alloc_current;
	FT2_1_3_ULong         alloc_max;

	const char*      file_name;
	FT2_1_3_Long          line_no;

	FT2_1_3_Memory        memory;
	FT2_1_3_Pointer       memory_user;
	FT2_1_3_Alloc_Func    alloc;
	FT2_1_3_Free_Func     free;
	FT2_1_3_Realloc_Func  realloc;

} FT2_1_3_MemTableRec;


#define FT2_1_3_MEM_SIZE_MIN  7
#define FT2_1_3_MEM_SIZE_MAX  13845163

#define FT2_1_3_FILENAME( x )  ((x) ? (x) : "unknown file")


static const FT2_1_3_UInt  ft_mem_primes[] = {
	7,
	11,
	19,
	37,
	73,
	109,
	163,
	251,
	367,
	557,
	823,
	1237,
	1861,
	2777,
	4177,
	6247,
	9371,
	14057,
	21089,
	31627,
	47431,
	71143,
	106721,
	160073,
	240101,
	360163,
	540217,
	810343,
	1215497,
	1823231,
	2734867,
	4102283,
	6153409,
	9230113,
	13845163,
};



extern void
ft_mem_debug_panic( const char*  fmt, ... ) {
	va_list  ap;


	printf( "FreeType.Debug: " );

	va_start( ap, fmt );
	vprintf( fmt, ap );
	va_end( ap );

	printf( "\n" );
	exit( EXIT_FAILURE );
}


static FT2_1_3_ULong
ft_mem_closest_prime( FT2_1_3_ULong  num ) {
	FT2_1_3_UInt  i;


	for ( i = 0;
	        i < sizeof ( ft_mem_primes ) / sizeof ( ft_mem_primes[0] ); i++ )
		if ( ft_mem_primes[i] > num )
			return ft_mem_primes[i];

	return FT2_1_3_MEM_SIZE_MAX;
}


static FT2_1_3_Pointer
ft_mem_table_alloc( FT2_1_3_MemTable  table,
                    FT2_1_3_Long      size ) {
	FT2_1_3_Memory   memory = table->memory;
	FT2_1_3_Pointer  block;


	memory->user = table->memory_user;
	block = table->alloc( memory, size );
	memory->user = table;

	return block;
}


static void
ft_mem_table_free( FT2_1_3_MemTable  table,
                   FT2_1_3_Pointer   block ) {
	FT2_1_3_Memory  memory = table->memory;


	memory->user = table->memory_user;
	table->free( memory, block );
	memory->user = table;
}


static void
ft_mem_table_resize( FT2_1_3_MemTable  table ) {
	FT2_1_3_ULong  new_size;


	new_size = ft_mem_closest_prime( table->nodes );
	if ( new_size != table->size ) {
		FT2_1_3_MemNode*  new_buckets ;
		FT2_1_3_ULong     i;


		new_buckets = (FT2_1_3_MemNode *)
		              ft_mem_table_alloc( table,
		                                  new_size * sizeof ( FT2_1_3_MemNode ) );
		if ( new_buckets == NULL )
			return;

		FT2_1_3_MEM_ZERO( new_buckets, sizeof ( FT2_1_3_MemNode ) * new_size );

		for ( i = 0; i < table->size; i++ ) {
			FT2_1_3_MemNode  node, next, *pnode;
			FT2_1_3_ULong    hash;


			node = table->buckets[i];
			while ( node ) {
				next  = node->link;
				hash  = FT2_1_3_MEM_VAL( node->address ) % new_size;
				pnode = new_buckets + hash;

				node->link = pnode[0];
				pnode[0]   = node;

				node = next;
			}
		}

		if ( table->buckets )
			ft_mem_table_free( table, table->buckets );

		table->buckets = new_buckets;
		table->size    = new_size;
	}
}


static FT2_1_3_MemTable
ft_mem_table_new( FT2_1_3_Memory  memory ) {
	FT2_1_3_MemTable  table;


	table = (FT2_1_3_MemTable)memory->alloc( memory, sizeof ( *table ) );
	if ( table == NULL )
		goto Exit;

	FT2_1_3_MEM_ZERO( table, sizeof ( *table ) );

	table->size  = FT2_1_3_MEM_SIZE_MIN;
	table->nodes = 0;

	table->memory = memory;

	table->memory_user = memory->user;

	table->alloc   = memory->alloc;
	table->realloc = memory->realloc;
	table->free    = memory->free;

	table->buckets = (FT2_1_3_MemNode *)
	                 memory->alloc( memory,
	                                table->size * sizeof ( FT2_1_3_MemNode ) );
	if ( table->buckets )
		FT2_1_3_MEM_ZERO( table->buckets, sizeof ( FT2_1_3_MemNode ) * table->size );
	else {
		memory->free( memory, table );
		table = NULL;
	}

Exit:
	return table;
}


static void
ft_mem_table_destroy( FT2_1_3_MemTable  table ) {
	FT2_1_3_ULong  i;


	if ( table ) {
		FT2_1_3_Long    leak_count = 0;
		FT2_1_3_ULong   leaks = 0;


		for ( i = 0; i < table->size; i++ ) {
			FT2_1_3_MemNode  *pnode = table->buckets + i, next, node = *pnode;


			while ( node ) {
				next       = node->link;
				node->link = 0;

				if ( node->size > 0 ) {
					printf(
					    "leaked memory block at address %p, size %8ld in (%s:%ld)\n",
					    node->address, node->size,
					    FT2_1_3_FILENAME( node->alloc_file_name ),
					    node->alloc_line_no );

					leak_count++;
					leaks += node->size;

					ft_mem_table_free( table, node->address );
				}

				node->address = NULL;
				node->size    = 0;

				free( node );
				node = next;
			}
			table->buckets[i] = 0;
		}
		ft_mem_table_free( table, table->buckets );
		table->buckets = NULL;

		table->size   = 0;
		table->nodes  = 0;

		printf(
		    "FreeType: total memory allocations = %ld\n", table->alloc_total );
		printf(
		    "FreeType: maximum memory footprint = %ld\n", table->alloc_max );

		free( table );

		if ( leak_count > 0 )
			ft_mem_debug_panic(
			    "FreeType: %ld bytes of memory leaked in %ld blocks\n",
			    leaks, leak_count );
		printf( "FreeType: No memory leaks detected!\n" );
	}
}


static FT2_1_3_MemNode*
ft_mem_table_get_nodep( FT2_1_3_MemTable  table,
                        FT2_1_3_Byte*     address ) {
	FT2_1_3_ULong     hash;
	FT2_1_3_MemNode  *pnode, node;


	hash  = FT2_1_3_MEM_VAL( address );
	pnode = table->buckets + ( hash % table->size );

	for (;;) {
		node = pnode[0];
		if ( !node )
			break;

		if ( node->address == address )
			break;

		pnode = &node->link;
	}
	return pnode;
}


static void
ft_mem_table_set( FT2_1_3_MemTable  table,
                  FT2_1_3_Byte*     address,
                  FT2_1_3_ULong     size ) {
	FT2_1_3_MemNode  *pnode, node;


	if ( table ) {
		pnode = ft_mem_table_get_nodep( table, address );
		node  = *pnode;
		if ( node ) {
			if ( node->size < 0 ) {
				/* this block was already freed.  This means that our memory is */
				/* now completely corrupted!                                    */
				ft_mem_debug_panic(
				    "memory heap corrupted (allocating freed block)" );
			} else {
				/* this block was already allocated.  This means that our memory */
				/* is also corrupted!                                            */
				ft_mem_debug_panic(
				    "memory heap corrupted (re-allocating allocated block)" );
			}
		}

		/* we need to create a new node in this table */
		node = (FT2_1_3_MemNode)ft_mem_table_alloc( table, sizeof ( *node ) );
		if ( node == NULL )
			ft_mem_debug_panic( "not enough memory to run memory tests" );

		node->address = address;
		node->size    = size;

		node->alloc_file_name = table->file_name;
		node->alloc_line_no   = table->line_no;

		node->free_file_name = NULL;
		node->free_line_no   = 0;

		node->link = pnode[0];

		pnode[0] = node;
		table->nodes++;

		table->alloc_total   += size;
		table->alloc_current += size;
		if ( table->alloc_current > table->alloc_max )
			table->alloc_max = table->alloc_current;

		if ( table->nodes * 3 < table->size  ||
		        table->size  * 3 < table->nodes )
			ft_mem_table_resize( table );
	}
}


static void
ft_mem_table_remove( FT2_1_3_MemTable  table,
                     FT2_1_3_Byte*     address ) {
	if ( table ) {
		FT2_1_3_MemNode  *pnode, node;


		pnode = ft_mem_table_get_nodep( table, address );
		node  = *pnode;
		if ( node ) {
			if ( node->size < 0 )
				ft_mem_debug_panic(
				    "freeing memory block at %p more than once at (%s:%ld)\n"
				    "block allocated at (%s:%ld) and released at (%s:%ld)",
				    address,
				    FT2_1_3_FILENAME( table->file_name ), table->line_no,
				    FT2_1_3_FILENAME( node->alloc_file_name ), node->alloc_line_no,
				    FT2_1_3_FILENAME( node->free_file_name ), node->free_line_no );

			/* we simply invert the node's size to indicate that the node */
			/* was freed.  We also change its contents.                   */
			FT2_1_3_MEM_SET( address, 0xF3, node->size );

			table->alloc_current -= node->size;
			node->size            = -node->size;
			node->free_file_name  = table->file_name;
			node->free_line_no    = table->line_no;
		} else
			ft_mem_debug_panic(
			    "trying to free unknown block at %p in (%s:%ld)\n",
			    address,
			    FT2_1_3_FILENAME( table->file_name ), table->line_no );
	}
}


extern FT2_1_3_Pointer
ft_mem_debug_alloc( FT2_1_3_Memory  memory,
                    FT2_1_3_Long    size ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;
	FT2_1_3_Byte*     block;


	if ( size <= 0 )
		ft_mem_debug_panic( "negative block size allocation (%ld)", size );

	block = (FT2_1_3_Byte *)ft_mem_table_alloc( table, size );
	if ( block )
		ft_mem_table_set( table, block, (FT2_1_3_ULong)size );

	table->file_name = NULL;
	table->line_no   = 0;

	return (FT2_1_3_Pointer) block;
}


extern void
ft_mem_debug_free( FT2_1_3_Memory   memory,
                   FT2_1_3_Pointer  block ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;


	if ( block == NULL )
		ft_mem_debug_panic( "trying to free NULL in (%s:%ld)",
		                    FT2_1_3_FILENAME( table->file_name ),
		                    table->line_no );

	ft_mem_table_remove( table, (FT2_1_3_Byte*)block );

	/* we never really free the block */
	table->file_name = NULL;
	table->line_no   = 0;
}


extern FT2_1_3_Pointer
ft_mem_debug_realloc( FT2_1_3_Memory   memory,
                      FT2_1_3_Long     cur_size,
                      FT2_1_3_Long     new_size,
                      FT2_1_3_Pointer  block ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;
	FT2_1_3_MemNode   node, *pnode;
	FT2_1_3_Pointer   new_block;

	const char*  file_name = FT2_1_3_FILENAME( table->file_name );
	FT2_1_3_Long      line_no   = table->line_no;


	if ( block == NULL || cur_size == 0 )
		ft_mem_debug_panic( "trying to reallocate NULL in (%s:%ld)",
		                    file_name, line_no );

	if ( new_size <= 0 )
		ft_mem_debug_panic(
		    "trying to reallocate %p to size 0 (current is %ld) in (%s:%ld)",
		    block, cur_size, file_name, line_no );

	/* check 'cur_size' value */
	pnode = ft_mem_table_get_nodep( table, (FT2_1_3_Byte*)block );
	node  = *pnode;
	if ( !node )
		ft_mem_debug_panic(
		    "trying to reallocate unknown block at %p in (%s:%ld)",
		    block, file_name, line_no );

	if ( node->size <= 0 )
		ft_mem_debug_panic(
		    "trying to reallocate freed block at %p in (%s:%ld)",
		    block, file_name, line_no );

	if ( node->size != cur_size )
		ft_mem_debug_panic( "invalid ft_realloc request for %p. cur_size is "
		                    "%ld instead of %ld in (%s:%ld)",
		                    block, cur_size, node->size, file_name, line_no );

	new_block = ft_mem_debug_alloc( memory, new_size );
	if ( new_block == NULL )
		return NULL;

	ft_memcpy( new_block, block, cur_size < new_size ? cur_size : new_size );

	table->file_name = file_name;
	table->line_no   = line_no;

	ft_mem_debug_free( memory, (FT2_1_3_Byte*)block );

	return new_block;
}


extern FT2_1_3_Int
ft_mem_debug_init( FT2_1_3_Memory  memory ) {
	FT2_1_3_MemTable  table;
	FT2_1_3_Int       result = 0;


	if ( getenv( "FT2_1_3_DEBUG_MEMORY" ) ) {
		table = ft_mem_table_new( memory );
		if ( table ) {
			memory->user    = table;
			memory->alloc   = ft_mem_debug_alloc;
			memory->realloc = ft_mem_debug_realloc;
			memory->free    = ft_mem_debug_free;
			result = 1;
		}
	}
	return result;
}


extern void
ft_mem_debug_done( FT2_1_3_Memory  memory ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;


	if ( table ) {
		memory->free    = table->free;
		memory->realloc = table->realloc;
		memory->alloc   = table->alloc;

		ft_mem_table_destroy( table );
		memory->user = NULL;
	}
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Alloc_Debug( FT2_1_3_Memory    memory,
                FT2_1_3_Long      size,
                void*       *P,
                const char*  file_name,
                FT2_1_3_Long      line_no ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;


	if ( table ) {
		table->file_name = file_name;
		table->line_no   = line_no;
	}
	return FT2_1_3_Alloc( memory, size, P );
}


FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Realloc_Debug( FT2_1_3_Memory    memory,
                  FT2_1_3_Long      current,
                  FT2_1_3_Long      size,
                  void*       *P,
                  const char*  file_name,
                  FT2_1_3_Long      line_no ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;


	if ( table ) {
		table->file_name = file_name;
		table->line_no   = line_no;
	}
	return FT2_1_3_Realloc( memory, current, size, P );
}


FT2_1_3_BASE_DEF( void )
FT2_1_3_Free_Debug( FT2_1_3_Memory    memory,
               FT2_1_3_Pointer   block,
               const char*  file_name,
               FT2_1_3_Long      line_no ) {
	FT2_1_3_MemTable  table = (FT2_1_3_MemTable)memory->user;


	if ( table ) {
		table->file_name = file_name;
		table->line_no   = line_no;
	}
	FT2_1_3_Free( memory, (void **)block );
}


#else  /* !FT2_1_3_DEBUG_MEMORY */

/* ANSI C doesn't like empty source files */
const FT2_1_3_Byte  _debug_mem_dummy = 0;

#endif /* !FT2_1_3_DEBUG_MEMORY */


/* END */
