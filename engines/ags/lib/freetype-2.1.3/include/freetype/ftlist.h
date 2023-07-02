/***************************************************************************/
/*                                                                         */
/*  ftlist.h                                                               */
/*                                                                         */
/*    Generic list support for FreeType (specification).                   */
/*                                                                         */
/*  Copyright 1996-2001 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


/*************************************************************************/
/*                                                                       */
/*  This file implements functions relative to list processing.  Its     */
/*  data structures are defined in `freetype.h'.                         */
/*                                                                       */
/*************************************************************************/


#ifndef AGS_LIB_FREETYPE_FTLIST_H
#define AGS_LIB_FREETYPE_FTLIST_H


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"

namespace AGS3 {
namespace FreeType213 {

FT2_1_3_BEGIN_HEADER


/*************************************************************************/
/*                                                                       */
/* <Section>                                                             */
/*    list_processing                                                    */
/*                                                                       */
/* <Title>                                                               */
/*    List Processing                                                    */
/*                                                                       */
/* <Abstract>                                                            */
/*    Simple management of lists.                                        */
/*                                                                       */
/* <Description>                                                         */
/*    This section contains various definitions related to list          */
/*    processing using doubly-linked nodes.                              */
/*                                                                       */
/* <Order>                                                               */
/*    FT2_1_3_List                                                            */
/*    FT2_1_3_ListNode                                                        */
/*    FT2_1_3_ListRec                                                         */
/*    FT2_1_3_ListNodeRec                                                     */
/*                                                                       */
/*    FT2_1_3_List_Add                                                        */
/*    FT2_1_3_List_Insert                                                     */
/*    FT2_1_3_List_Find                                                       */
/*    FT2_1_3_List_Remove                                                     */
/*    FT2_1_3_List_Up                                                         */
/*    FT2_1_3_List_Iterate                                                    */
/*    FT2_1_3_List_Iterator                                                   */
/*    FT2_1_3_List_Finalize                                                   */
/*    FT2_1_3_List_Destructor                                                 */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Find                                                       */
/*                                                                       */
/* <Description>                                                         */
/*    Finds the list node for a given listed object.                     */
/*                                                                       */
/* <Input>                                                               */
/*    list :: A pointer to the parent list.                              */
/*    data :: The address of the listed object.                          */
/*                                                                       */
/* <Return>                                                              */
/*    List node.  NULL if it wasn't found.                               */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_ListNode )
FT2_1_3_List_Find( FT2_1_3_List  list,
			  void*    data );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Add                                                        */
/*                                                                       */
/* <Description>                                                         */
/*    Appends an element to the end of a list.                           */
/*                                                                       */
/* <InOut>                                                               */
/*    list :: A pointer to the parent list.                              */
/*    node :: The node to append.                                        */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_List_Add( FT2_1_3_List      list,
			 FT2_1_3_ListNode  node );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Insert                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    Inserts an element at the head of a list.                          */
/*                                                                       */
/* <InOut>                                                               */
/*    list :: A pointer to parent list.                                  */
/*    node :: The node to insert.                                        */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_List_Insert( FT2_1_3_List      list,
				FT2_1_3_ListNode  node );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Remove                                                     */
/*                                                                       */
/* <Description>                                                         */
/*    Removes a node from a list.  This function doesn't check whether   */
/*    the node is in the list!                                           */
/*                                                                       */
/* <Input>                                                               */
/*    node :: The node to remove.                                        */
/*                                                                       */
/* <InOut>                                                               */
/*    list :: A pointer to the parent list.                              */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_List_Remove( FT2_1_3_List      list,
				FT2_1_3_ListNode  node );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Up                                                         */
/*                                                                       */
/* <Description>                                                         */
/*    Moves a node to the head/top of a list.  Used to maintain LRU      */
/*    lists.                                                             */
/*                                                                       */
/* <InOut>                                                               */
/*    list :: A pointer to the parent list.                              */
/*    node :: The node to move.                                          */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_List_Up( FT2_1_3_List      list,
			FT2_1_3_ListNode  node );


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_List_Iterator                                                   */
/*                                                                       */
/* <Description>                                                         */
/*    An FT2_1_3_List iterator function which is called during a list parse   */
/*    by FT2_1_3_List_Iterate().                                              */
/*                                                                       */
/* <Input>                                                               */
/*    node :: The current iteration list node.                           */
/*                                                                       */
/*    user :: A typeless pointer passed to FT2_1_3_List_Iterate().            */
/*            Can be used to point to the iteration's state.             */
/*                                                                       */
typedef FT2_1_3_Error
(*FT2_1_3_List_Iterator)( FT2_1_3_ListNode  node,
					 void*        user );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Iterate                                                    */
/*                                                                       */
/* <Description>                                                         */
/*    Parses a list and calls a given iterator function on each element. */
/*    Note that parsing is stopped as soon as one of the iterator calls  */
/*    returns a non-zero value.                                          */
/*                                                                       */
/* <Input>                                                               */
/*    list     :: A handle to the list.                                  */
/*    iterator :: An interator function, called on each node of the      */
/*                list.                                                  */
/*    user     :: A user-supplied field which is passed as the second    */
/*                argument to the iterator.                              */
/*                                                                       */
/* <Return>                                                              */
/*    The result (a FreeType error code) of the last iterator call.      */
/*                                                                       */
FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_List_Iterate( FT2_1_3_List           list,
				 FT2_1_3_List_Iterator  iterator,
				 void*             user );


/*************************************************************************/
/*                                                                       */
/* <FuncType>                                                            */
/*    FT2_1_3_List_Destructor                                                 */
/*                                                                       */
/* <Description>                                                         */
/*    An FT2_1_3_List iterator function which is called during a list         */
/*    finalization by FT2_1_3_List_Finalize() to destroy all elements in a    */
/*    given list.                                                        */
/*                                                                       */
/* <Input>                                                               */
/*    system :: The current system object.                               */
/*                                                                       */
/*    data   :: The current object to destroy.                           */
/*                                                                       */
/*    user   :: A typeless pointer passed to FT2_1_3_List_Iterate().  It can  */
/*              be used to point to the iteration's state.               */
/*                                                                       */
typedef void
(*FT2_1_3_List_Destructor)( FT2_1_3_Memory  memory,
					   void*      data,
					   void*      user );


/*************************************************************************/
/*                                                                       */
/* <Function>                                                            */
/*    FT2_1_3_List_Finalize                                                   */
/*                                                                       */
/* <Description>                                                         */
/*    Destroys all elements in the list as well as the list itself.      */
/*                                                                       */
/* <Input>                                                               */
/*    list    :: A handle to the list.                                   */
/*                                                                       */
/*    destroy :: A list destructor that will be applied to each element  */
/*               of the list.                                            */
/*                                                                       */
/*    memory  :: The current memory object which handles deallocation.   */
/*                                                                       */
/*    user    :: A user-supplied field which is passed as the last       */
/*               argument to the destructor.                             */
/*                                                                       */
FT2_1_3_EXPORT( void )
FT2_1_3_List_Finalize( FT2_1_3_List             list,
				  FT2_1_3_List_Destructor  destroy,
				  FT2_1_3_Memory           memory,
				  void*               user );


/* */


FT2_1_3_END_HEADER

} // End of namespace FreeType213
} // End of namespace AGS3

#endif /* AGS_LIB_FREETYPE_FTLIST_H */


/* END */
