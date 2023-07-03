/***************************************************************************/
/*                                                                         */
/*  ftlru.c                                                                */
/*                                                                         */
/*    Simple LRU list-cache (body).                                        */
/*                                                                         */
/*  Copyright 2000-2001, 2002 by                                           */
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
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftcache.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/cache/ftlru.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftlist.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/internal/ftobjs.h"

#include "ftcerror.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_LruList_New( FT2_1_3_LruList_Class  clazz,
				FT2_1_3_UInt           max_nodes,
				FT2_1_3_Pointer        user_data,
				FT2_1_3_Memory         memory,
				FT2_1_3_LruList       *alist ) {
	FT2_1_3_Error    error;
	FT2_1_3_LruList  list;


	if ( !alist || !clazz )
		return FTC_Err_Invalid_Argument;

	*alist = NULL;
	if ( !FT2_1_3_ALLOC( list, clazz->list_size ) ) {
		/* initialize common fields */
		list->clazz      = clazz;
		list->memory     = memory;
		list->max_nodes  = max_nodes;
		list->data       = user_data;

		if ( clazz->list_init ) {
			error = clazz->list_init( list );
			if ( error ) {
				if ( clazz->list_done )
					clazz->list_done( list );

				FT2_1_3_FREE( list );
			}
		}

		*alist = list;
	}

	return error;
}


FT2_1_3_EXPORT_DEF( void )
FT2_1_3_LruList_Destroy( FT2_1_3_LruList  list ) {
	FT2_1_3_Memory         memory;
	FT2_1_3_LruList_Class  clazz;


	if ( !list )
		return;

	memory = list->memory;
	clazz  = list->clazz;

	FT2_1_3_LruList_Reset( list );

	if ( clazz->list_done )
		clazz->list_done( list );

	FT2_1_3_FREE( list );
}


FT2_1_3_EXPORT_DEF( void )
FT2_1_3_LruList_Reset( FT2_1_3_LruList  list ) {
	FT2_1_3_LruNode        node;
	FT2_1_3_LruList_Class  clazz;
	FT2_1_3_Memory         memory;


	if ( !list )
		return;

	node   = list->nodes;
	clazz  = list->clazz;
	memory = list->memory;

	while ( node ) {
		FT2_1_3_LruNode  next = node->next;


		if ( clazz->node_done )
			clazz->node_done( node, list->data );

		FT2_1_3_FREE( node );
		node = next;
	}

	list->nodes     = NULL;
	list->num_nodes = 0;
}


FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_LruList_Lookup( FT2_1_3_LruList   list,
				   FT2_1_3_LruKey    key,
				   FT2_1_3_LruNode  *anode ) {
	FT2_1_3_Error          error = 0;
	FT2_1_3_LruNode        node, *pnode;
	FT2_1_3_LruList_Class  clazz;
	FT2_1_3_LruNode*       plast;
	FT2_1_3_LruNode        result = NULL;
	FT2_1_3_Memory         memory;


	if ( !list || !key || !anode )
		return FTC_Err_Invalid_Argument;

	pnode  = &list->nodes;
	plast  = pnode;
	node   = NULL;
	clazz  = list->clazz;
	memory = list->memory;

	if ( clazz->node_compare ) {
		for (;;) {
			node = *pnode;
			if ( node == NULL )
				break;

			if ( clazz->node_compare( node, key, list->data ) )
				break;

			plast = pnode;
			pnode = &(*pnode)->next;
		}
	} else {
		for (;;) {
			node = *pnode;
			if ( node == NULL )
				break;

			if ( node->key == key )
				break;

			plast = pnode;
			pnode = &(*pnode)->next;
		}
	}

	if ( node ) {
		/* move element to top of list */
		if ( list->nodes != node ) {
			*pnode      = node->next;
			node->next  = list->nodes;
			list->nodes = node;
		}
		result = node;
		goto Exit;
	}

	/* we haven't found the relevant element.  We will now try */
	/* to create a new one.                                    */
	/*                                                         */

	/* first, check if our list if full, when appropriate */
	if ( list->max_nodes > 0 && list->num_nodes >= list->max_nodes ) {
		/* this list list is full; we will now flush */
		/* the oldest node, if there's one!          */
		FT2_1_3_LruNode  last = *plast;


		if ( last ) {
			if ( clazz->node_flush ) {
				error = clazz->node_flush( last, key, list->data );
			} else {
				if ( clazz->node_done )
					clazz->node_done( last, list->data );

				last->key  = key;
				error = clazz->node_init( last, key, list->data );
			}

			if ( !error ) {
				/* move it to the top of the list */
				*plast      = NULL;
				last->next  = list->nodes;
				list->nodes = last;

				result = last;
				goto Exit;
			}

			/* in case of error during the flush or done/init cycle, */
			/* we need to discard the node                           */
			if ( clazz->node_done )
				clazz->node_done( last, list->data );

			*plast = NULL;
			list->num_nodes--;

			FT2_1_3_FREE( last );
			goto Exit;
		}
	}

	/* otherwise, simply allocate a new node */
	if ( FT2_1_3_ALLOC( node, clazz->node_size ) )
		goto Exit;

	node->key = key;
	error = clazz->node_init( node, key, list->data );
	if ( error ) {
		FT2_1_3_FREE( node );
		goto Exit;
	}

	result      = node;
	node->next  = list->nodes;
	list->nodes = node;
	list->num_nodes++;

Exit:
	*anode = result;
	return error;
}


FT2_1_3_EXPORT_DEF( void )
FT2_1_3_LruList_Remove( FT2_1_3_LruList  list,
				   FT2_1_3_LruNode  node ) {
	FT2_1_3_LruNode  *pnode;


	if ( !list || !node )
		return;

	pnode = &list->nodes;
	for (;;) {
		if ( *pnode == node ) {
			FT2_1_3_Memory         memory = list->memory;
			FT2_1_3_LruList_Class  clazz  = list->clazz;


			*pnode     = node->next;
			node->next = NULL;

			if ( clazz->node_done )
				clazz->node_done( node, list->data );

			FT2_1_3_FREE( node );
			list->num_nodes--;
			break;
		}

		pnode = &(*pnode)->next;
	}
}


FT2_1_3_EXPORT_DEF( void )
FT2_1_3_LruList_Remove_Selection( FT2_1_3_LruList             list,
							 FT2_1_3_LruNode_SelectFunc  select_func,
							 FT2_1_3_Pointer             select_data ) {
	FT2_1_3_LruNode       *pnode, node;
	FT2_1_3_LruList_Class  clazz;
	FT2_1_3_Memory         memory;


	if ( !list || !select_func )
		return;

	memory = list->memory;
	clazz  = list->clazz;
	pnode  = &list->nodes;

	for (;;) {
		node = *pnode;
		if ( node == NULL )
			break;

		if ( select_func( node, select_data, list->data ) ) {
			*pnode     = node->next;
			node->next = NULL;

			if ( clazz->node_done )
				clazz->node_done( node, list );

			FT2_1_3_FREE( node );
		} else
			pnode = &(*pnode)->next;
	}
}

} // End of namespace FreeType213
} // End of namespace AGS3

/* END */
