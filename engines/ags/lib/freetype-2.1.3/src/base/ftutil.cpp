/***************************************************************************/
/*                                                                         */
/*  ftutil.c                                                               */
/*                                                                         */
/*    FreeType utility file for memory and list management (body).         */
/*                                                                         */
/*  Copyright 2002 by                                                      */
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
#include FT2_1_3_INTERNAL_DEBUG_H
#include FT2_1_3_INTERNAL_MEMORY_H
#include "engines/ags/lib/freetype-2.1.3/include/freetype/ftlist.h"


/*************************************************************************/
/*                                                                       */
/* The macro FT2_1_3_COMPONENT is used in trace mode.  It is an implicit      */
/* parameter of the FT2_1_3_TRACE() and FT2_1_3_ERROR() macros, used to print/log  */
/* messages during execution.                                            */
/*                                                                       */
#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_memory


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                                                               *****/
/*****               M E M O R Y   M A N A G E M E N T               *****/
/*****                                                               *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

/* documentation is in ftmemory.h */

FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Alloc( FT2_1_3_Memory  memory,
          FT2_1_3_Long    size,
          void*     *P ) {
	FT2_1_3_ASSERT( P != 0 );

	if ( size > 0 ) {
		*P = memory->alloc( memory, size );
		if ( !*P ) {
			FT2_1_3_ERROR(( "FT2_1_3_Alloc:" ));
			FT2_1_3_ERROR(( " Out of memory? (%ld requested)\n",
			           size ));

			return FT2_1_3_Err_Out_Of_Memory;
		}
		FT2_1_3_MEM_ZERO( *P, size );
	} else
		*P = NULL;

	FT2_1_3_TRACE7(( "FT2_1_3_Alloc:" ));
	FT2_1_3_TRACE7(( " size = %ld, block = 0x%08p, ref = 0x%08p\n",
	            size, *P, P ));

	return FT2_1_3_Err_Ok;
}


/* documentation is in ftmemory.h */

FT2_1_3_BASE_DEF( FT2_1_3_Error )
FT2_1_3_Realloc( FT2_1_3_Memory  memory,
            FT2_1_3_Long    current,
            FT2_1_3_Long    size,
            void**     P ) {
	void*  Q;


	FT2_1_3_ASSERT( P != 0 );

	/* if the original pointer is NULL, call FT2_1_3_Alloc() */
	if ( !*P )
		return FT2_1_3_Alloc( memory, size, P );

	/* if the new block if zero-sized, clear the current one */
	if ( size <= 0 ) {
		FT2_1_3_Free( memory, P );
		return FT2_1_3_Err_Ok;
	}

	Q = memory->realloc( memory, current, size, *P );
	if ( !Q )
		goto Fail;

	if ( size > current )
		FT2_1_3_MEM_ZERO( (char*)Q + current, size - current );

	*P = Q;
	return FT2_1_3_Err_Ok;

Fail:
	FT2_1_3_ERROR(( "FT2_1_3_Realloc:" ));
	FT2_1_3_ERROR(( " Failed (current %ld, requested %ld)\n",
	           current, size ));
	return FT2_1_3_Err_Out_Of_Memory;
}


/* documentation is in ftmemory.h */

FT2_1_3_BASE_DEF( void )
FT2_1_3_Free( FT2_1_3_Memory  memory,
         void**     P ) {
	FT2_1_3_TRACE7(( "FT2_1_3_Free:" ));
	FT2_1_3_TRACE7(( " Freeing block 0x%08p, ref 0x%08p\n",
	            P, P ? *P : (void*)0 ));

	if ( P && *P ) {
		memory->free( memory, *P );
		*P = 0;
	}
}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*****                                                               *****/
/*****                                                               *****/
/*****            D O U B L Y   L I N K E D   L I S T S              *****/
/*****                                                               *****/
/*****                                                               *****/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

#undef  FT2_1_3_COMPONENT
#define FT2_1_3_COMPONENT  trace_list

/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_ListNode )
FT2_1_3_List_Find( FT2_1_3_List  list,
              void*    data ) {
	FT2_1_3_ListNode  cur;


	cur = list->head;
	while ( cur ) {
		if ( cur->data == data )
			return cur;

		cur = cur->next;
	}

	return (FT2_1_3_ListNode)0;
}


/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_List_Add( FT2_1_3_List      list,
             FT2_1_3_ListNode  node ) {
	FT2_1_3_ListNode  before = list->tail;


	node->next = 0;
	node->prev = before;

	if ( before )
		before->next = node;
	else
		list->head = node;

	list->tail = node;
}


/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_List_Insert( FT2_1_3_List      list,
                FT2_1_3_ListNode  node ) {
	FT2_1_3_ListNode  after = list->head;


	node->next = after;
	node->prev = 0;

	if ( !after )
		list->tail = node;
	else
		after->prev = node;

	list->head = node;
}


/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_List_Remove( FT2_1_3_List      list,
                FT2_1_3_ListNode  node ) {
	FT2_1_3_ListNode  before, after;


	before = node->prev;
	after  = node->next;

	if ( before )
		before->next = after;
	else
		list->head = after;

	if ( after )
		after->prev = before;
	else
		list->tail = before;
}


/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_List_Up( FT2_1_3_List      list,
            FT2_1_3_ListNode  node ) {
	FT2_1_3_ListNode  before, after;


	before = node->prev;
	after  = node->next;

	/* check whether we are already on top of the list */
	if ( !before )
		return;

	before->next = after;

	if ( after )
		after->prev = before;
	else
		list->tail = before;

	node->prev       = 0;
	node->next       = list->head;
	list->head->prev = node;
	list->head       = node;
}


/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( FT2_1_3_Error )
FT2_1_3_List_Iterate( FT2_1_3_List            list,
                 FT2_1_3_List_Iterator   iterator,
                 void*              user ) {
	FT2_1_3_ListNode  cur   = list->head;
	FT2_1_3_Error     error = FT2_1_3_Err_Ok;


	while ( cur ) {
		FT2_1_3_ListNode  next = cur->next;


		error = iterator( cur, user );
		if ( error )
			break;

		cur = next;
	}

	return error;
}


/* documentation is in ftlist.h */

FT2_1_3_EXPORT_DEF( void )
FT2_1_3_List_Finalize( FT2_1_3_List             list,
                  FT2_1_3_List_Destructor  destroy,
                  FT2_1_3_Memory           memory,
                  void*               user ) {
	FT2_1_3_ListNode  cur;


	cur = list->head;
	while ( cur ) {
		FT2_1_3_ListNode  next = cur->next;
		void*        data = cur->data;


		if ( destroy )
			destroy( memory, data, user );

		FT2_1_3_FREE( cur );
		cur = next;
	}

	list->head = 0;
	list->tail = 0;
}


/* END */
