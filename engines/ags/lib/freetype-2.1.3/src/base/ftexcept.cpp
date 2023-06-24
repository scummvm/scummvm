#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include FT2_1_3_EXCEPT_H


FT2_1_3_BASE_DEF( void )
ft_cleanup_stack_init( FT2_1_3_CleanupStack  stack,
                       FT2_1_3_Memory        memory ) {
	stack->chunk = &stack->chunk_0;
	stack->top   = stack->chunk->items;
	stack->limit = stack->top + FT2_1_3_CLEANUP_CHUNK_SIZE;
	stack->chunk_0.link = NULL;

	stack->memory = memory;
}



FT2_1_3_BASE_DEF( void )
ft_cleanup_stack_done( FT2_1_3_CleanupStack  stack ) {
	FT2_1_3_Memory        memory = stack->memory;
	FT2_1_3_CleanupChunk  chunk, next;

	for (;;) {
		chunk = stack->chunk;
		if ( chunk == &stack->chunk_0 )
			break;

		stack->chunk = chunk->link;

		FT2_1_3_Free( chunk, memory );
	}

	stack->memory = NULL;
}



FT2_1_3_BASE_DEF( void )
ft_cleanup_stack_push( FT2_1_3_CleanupStack  stack,
                       FT2_1_3_Pointer       item,
                       FT2_1_3_CleanupFunc   item_func,
                       FT2_1_3_Pointer       item_data ) {
	FT2_1_3_CleanupItem  top;


	FT2_1_3_ASSERT( stack && stack->chunk && stack->top );
	FT2_1_3_ASSERT( item  && item_func );

	top = stack->top;

	top->item      = item;
	top->item_func = item_func;
	top->item_data = item_data;

	top ++;

	if ( top == stack->limit ) {
		FT2_1_3_CleanupChunk  chunk;

		chunk = FT2_1_3_QAlloc( sizeof(*chunk), stack->memory );

		chunk->link  = stack->chunk;
		stack->chunk = chunk;
		stack->limit = chunk->items + FT2_1_3_CLEANUP_CHUNK_SIZE;
		top          = chunk->items;
	}

	stack->top = top;
}



FT2_1_3_BASE_DEF( void )
ft_cleanup_stack_pop( FT2_1_3_CleanupStack   stack,
                      FT2_1_3_Int            destroy ) {
	FT2_1_3_CleanupItem  top;


	FT2_1_3_ASSERT( stack && stack->chunk && stack->top );
	top = stack->top;

	if ( top == stack->chunk->items ) {
		FT2_1_3_CleanupChunk  chunk;

		chunk = stack->chunk;

		if ( chunk == &stack->chunk_0 ) {
			FT2_1_3_ERROR(( "cleanup.pop: empty cleanup stack !!\n" ));
			ft_cleanup_throw( stack, FT2_1_3_Err_EmptyCleanupStack );
		}

		chunk = chunk->link;
		FT2_1_3_QFree( stack->chunk, stack->memory );

		stack->chunk = chunk;
		stack->limit = chunk->items + FT2_1_3_CLEANUP_CHUNK_SIZE;
		top          = stack->limit;
	}

	top --;

	if ( destroy )
		top->item_func( top->item, top->item_data );

	top->item      = NULL;
	top->item_func = NULL;
	top->item_data = NULL;

	stack->top = top;
}



FT2_1_3_BASE_DEF( FT2_1_3_CleanupItem )
ft_cleanup_stack_peek( FT2_1_3_CleanupStack  stack ) {
	FT2_1_3_CleanupItem   top;
	FT2_1_3_CleanupChunk  chunk;


	FT2_1_3_ASSERT( stack && stack->chunk && stack->top );

	top   = stack->top;
	chunk = stack->chunk;

	if ( top > chunk->items )
		top--;
	else {
		chunk = chunk->link;
		top   = NULL;
		if ( chunk != NULL )
			top = chunk->items + FT2_1_3_CLEANUP_CHUNK_SIZE - 1;
	}
	return top;
}



FT2_1_3_BASE_DEF( void )
ft_xhandler_enter( FT2_1_3_XHandler  xhandler,
                   FT2_1_3_Memory    memory ) {
	FT2_1_3_CleanupStack  stack = FT2_1_3_MEMORY__CLEANUP(memory);

	xhandler->previous = stack->xhandler;
	xhandler->cleanup  = stack->top;
	xhandler->error    = 0;
	stack->xhandler    = xhandler;
}



FT2_1_3_BASE_DEF( void )
ft_xhandler_exit( FT2_1_3_XHandler  xhandler ) {
	FT2_1_3_CleanupStack  stack = FT2_1_3_MEMORY__CLEANUP(memory);

	stack->xhandler    = xhandler->previous;
	xhandler->previous = NULL;
	xhandler->error    = error;
	xhandler->cleanup  = NULL;
}



FT2_1_3_BASE_DEF( void )
ft_cleanup_throw( FT2_1_3_CleanupStack  stack,
                  FT2_1_3_Error         error ) {
	FT2_1_3_XHandler  xhandler = stack->xhandler;

	if ( xhandler == NULL ) {
		/* no exception handler was registered. this  */
		/* means that we have an un-handled exception */
		/* the only thing we can do is _PANIC_ and    */
		/* halt the current program..                 */
		/*                                            */
		FT2_1_3_ERROR(( "FREETYPE PANIC: An un-handled exception occured. Program aborted" ));
		ft_exit(1);
	}

	/* cleanup the stack until we reach the handler's */
	/* starting stack location..                      */

	xhandler->error = error;
	longmp( xhandler->jump_buffer, 1 );
}