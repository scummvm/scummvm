/***************************************************************************/
/*                                                                         */
/*  ftlru.h                                                                */
/*                                                                         */
/*    Simple LRU list-cache (specification).                               */
/*                                                                         */
/*  Copyright 2000-2001 by                                                 */
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
/* An LRU is a list that cannot hold more than a certain number of       */
/* elements (`max_elements').  All elements in the list are sorted in    */
/* least-recently-used order, i.e., the `oldest' element is at the tail  */
/* of the list.                                                          */
/*                                                                       */
/* When doing a lookup (either through `Lookup()' or `Lookup_Node()'),   */
/* the list is searched for an element with the corresponding key.  If   */
/* it is found, the element is moved to the head of the list and is      */
/* returned.                                                             */
/*                                                                       */
/* If no corresponding element is found, the lookup routine will try to  */
/* obtain a new element with the relevant key.  If the list is already   */
/* full, the oldest element from the list is discarded and replaced by a */
/* new one; a new element is added to the list otherwise.                */
/*                                                                       */
/* Note that it is possible to pre-allocate the element list nodes.      */
/* This is handy if `max_elements' is sufficiently small, as it saves    */
/* allocations/releases during the lookup process.                       */
/*                                                                       */
/*************************************************************************/


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*********                                                       *********/
/*********             WARNING, THIS IS BETA CODE.               *********/
/*********                                                       *********/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/


#ifndef __FTLRU_H__
#define __FTLRU_H__


#include "engines/ags/lib/freetype-2.1.3/include/ft2build.h"
#include "engines/ags/lib/freetype-2.1.3/include/freetype/freetype.h"


FT2_1_3_BEGIN_HEADER


/* generic list key type */
typedef FT2_1_3_Pointer  FT2_1_3_LruKey;

/* a list list handle */
typedef struct FT2_1_3_LruListRec_*  FT2_1_3_LruList;

/* a list class handle */
typedef const struct FT2_1_3_LruList_ClassRec_*  FT2_1_3_LruList_Class;

/* a list node handle */
typedef struct FT2_1_3_LruNodeRec_*  FT2_1_3_LruNode;

/* the list node structure */
typedef struct  FT2_1_3_LruNodeRec_ {
	FT2_1_3_LruNode  next;
	FT2_1_3_LruKey   key;

} FT2_1_3_LruNodeRec;


/* the list structure */
typedef struct  FT2_1_3_LruListRec_ {
	FT2_1_3_Memory         memory;
	FT2_1_3_LruList_Class  clazz;
	FT2_1_3_LruNode        nodes;
	FT2_1_3_UInt           max_nodes;
	FT2_1_3_UInt           num_nodes;
	FT2_1_3_Pointer        data;

} FT2_1_3_LruListRec;


/* initialize a list list */
typedef FT2_1_3_Error
(*FT2_1_3_LruList_InitFunc)( FT2_1_3_LruList  list );

/* finalize a list list */
typedef void
(*FT2_1_3_LruList_DoneFunc)( FT2_1_3_LruList  list );

/* this method is used to initialize a new list element node */
typedef FT2_1_3_Error
(*FT2_1_3_LruNode_InitFunc)( FT2_1_3_LruNode  node,
                        FT2_1_3_LruKey   key,
                        FT2_1_3_Pointer  data );

/* this method is used to finalize a given list element node */
typedef void
(*FT2_1_3_LruNode_DoneFunc)( FT2_1_3_LruNode  node,
                        FT2_1_3_Pointer  data );

/* If defined, this method is called when the list if full        */
/* during the lookup process -- it is used to change the contents */
/* of a list element node instead of calling `done_element()',    */
/* then `init_element()'.  Set it to 0 for default behaviour.     */
typedef FT2_1_3_Error
(*FT2_1_3_LruNode_FlushFunc)( FT2_1_3_LruNode  node,
                         FT2_1_3_LruKey   new_key,
                         FT2_1_3_Pointer  data );

/* If defined, this method is used to compare a list element node */
/* with a given key during a lookup.  If set to 0, the `key'      */
/* fields will be directly compared instead.                      */
typedef FT2_1_3_Bool
(*FT2_1_3_LruNode_CompareFunc)( FT2_1_3_LruNode  node,
                           FT2_1_3_LruKey   key,
                           FT2_1_3_Pointer  data );

/* A selector is used to indicate whether a given list element node */
/* is part of a selection for FT2_1_3_LruList_Remove_Selection().  The   */
/* functrion must return true (i.e., non-null) to indicate that the */
/* node is part of it.                                              */
typedef FT2_1_3_Bool
(*FT2_1_3_LruNode_SelectFunc)( FT2_1_3_LruNode  node,
                          FT2_1_3_Pointer  data,
                          FT2_1_3_Pointer  list_data );

/* LRU class */
typedef struct  FT2_1_3_LruList_ClassRec_ {
	FT2_1_3_UInt                 list_size;
	FT2_1_3_LruList_InitFunc     list_init;      /* optional */
	FT2_1_3_LruList_DoneFunc     list_done;      /* optional */

	FT2_1_3_UInt                 node_size;
	FT2_1_3_LruNode_InitFunc     node_init;     /* MANDATORY */
	FT2_1_3_LruNode_DoneFunc     node_done;     /* optional  */
	FT2_1_3_LruNode_FlushFunc    node_flush;    /* optional  */
	FT2_1_3_LruNode_CompareFunc  node_compare;  /* optional  */

} FT2_1_3_LruList_ClassRec;


/* The following functions must be exported in the case where */
/* applications would want to write their own cache classes.  */

FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_LruList_New( FT2_1_3_LruList_Class  clazz,
                FT2_1_3_UInt           max_elements,
                FT2_1_3_Pointer        user_data,
                FT2_1_3_Memory         memory,
                FT2_1_3_LruList       *alist );

FT2_1_3_EXPORT( void )
FT2_1_3_LruList_Reset( FT2_1_3_LruList  list );

FT2_1_3_EXPORT( void )
FT2_1_3_LruList_Destroy ( FT2_1_3_LruList  list );

FT2_1_3_EXPORT( FT2_1_3_Error )
FT2_1_3_LruList_Lookup( FT2_1_3_LruList  list,
                   FT2_1_3_LruKey   key,
                   FT2_1_3_LruNode *anode );

FT2_1_3_EXPORT( void )
FT2_1_3_LruList_Remove( FT2_1_3_LruList  list,
                   FT2_1_3_LruNode  node );

FT2_1_3_EXPORT( void )
FT2_1_3_LruList_Remove_Selection( FT2_1_3_LruList             list,
                             FT2_1_3_LruNode_SelectFunc  select_func,
                             FT2_1_3_Pointer             select_data );

/* */

FT2_1_3_END_HEADER


#endif /* __FTLRU_H__ */


/* END */
