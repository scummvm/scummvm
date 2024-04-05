/*----------------------------------------------------------------------
John Robbins
Microsoft Systems Journal, October 1997 - BugSlayer Column!
----------------------------------------------------------------------*/

#ifndef _MEMDUMPERVALIDATOR_H
#define _MEMDUMPERVALIDATOR_H

// This file should not be included directly, include Bugslayer.h
//  instead.
#ifndef _BUGSLAYERUTIL_H
#error "Include BugslayerUtil.h instead of this file directly!"
#endif  // _BUGSLAYERUTIL_H

// Include the main header.
#include "MSJDBG.h"

#ifdef __cplusplus
extern "C" {
#endif      // __cplusplus

// This library can only be used in _DEBUG builds.
#ifdef _DEBUG

////////////////////////////////////////////////////////////////////////
// The typedefs for the dumper and validator functions.
////////////////////////////////////////////////////////////////////////
// The memory dumper function.  The only parameter is a pointer to the
//  block of memory.  This function can output the memory data for the
//  block any way it likes but it might be nice if it uses the same
//  Debug CRT reporting mechanism that everything else in the runtime
//  uses.
typedef void (*PFNMEMDUMPER)(const void *) ;
// The validator function.  The first parameter is the memory block to
//  validate and the second parameter is the context information passed
//  to the ValidateAllBlocks function.
typedef void (*PFNMEMVALIDATOR)(const void * , const void *) ;

////////////////////////////////////////////////////////////////////////
// Useful Macros.
////////////////////////////////////////////////////////////////////////
// The macro used to set a client block value.  This is the ONLY
//  approved means of setting a value for the dwValue field in the
//  DVINFO structure below.
#define CLIENT_BLOCK_VALUE(x) (_CLIENT_BLOCK|(x<<16))
// A macro to pick out the subtype.
#define CLIENT_BLOCK_SUBTYPE(x) ((x >> 16) & 0xFFFF)

////////////////////////////////////////////////////////////////////////
// The header used to initialize the dumper and validator for a specific
//  type of client block.
////////////////////////////////////////////////////////////////////////
typedef struct tag_DVINFO
{
    // The value for the client blocks.  This must be set with the
    //  CLIENT_BLOCK_VALUE macro above.  See the AddClientDV function
    //  for how to have the library assign this number.
    unsigned long   dwValue      ;
    // The pointer to the dumper function.
    PFNMEMDUMPER    pfnDump     ;
    // The pointer to the dumper function.
    PFNMEMVALIDATOR pfnValidate ;
} DVINFO , * LPDVINFO ;

/*----------------------------------------------------------------------
FUNCTION        :   AddClientDV
DISCUSSION      :
    Adds a client block dumper and validator to the list.  If the
dwValue field in the DVINFO structure is ZERO, then the next value in
the list is assigned.  This means that the value returned must always be
passed to _malloc_dbg as the value of the client block.
    If the value is set with CLIENT_BLOCK_VALUE, then a macro can be
used for the value to _malloc_dbg.
    No, there is no corresponding remove function.  Why possibly
introduce bugs in debugging code?  Performance is a non issue when it
comes to finding errors.
PARAMETERS      :
    lpDVInfo - The pointer to the DVINFO structure.
RETURNS         :
    1 - The client block dumper and validator was properly added.
    0 - The client block dumper and validator could not be added.
----------------------------------------------------------------------*/
    int BUGSUTIL_DLLINTERFACE __stdcall AddClientDV (LPDVINFO lpDVInfo);

/*----------------------------------------------------------------------
FUNCTION        :   ValidateAllBlocks
DISCUSSION      :
    Checks all the memory allocated out of the local heap.  Also goes
through all client blocks and calls the special validator function for
the different types of client blocks.
    It is probably best to call this function with the VALIDATEALLBLOCKS
macro below.
PARAMETERS      :
    pContext - The context information that will be passed to each
               call to the validator function.
RETURNS         :
    None.
----------------------------------------------------------------------*/
    void BUGSUTIL_DLLINTERFACE __stdcall
                                 ValidateAllBlocks ( void * pContext ) ;

#ifdef __cplusplus
////////////////////////////////////////////////////////////////////////
// Helper C++ class macros.
////////////////////////////////////////////////////////////////////////
// Declare this macro in your class just like the MFC ones.
#define DECLARE_MEMDEBUG(classname)                                 \
public   :                                                          \
    static DVINFO  m_stDVInfo ;                                     \
    static void ClassDumper ( const void * pData ) ;                \
    static void ClassValidator ( const void * pData ,               \
                                     const void * pContext )       ;\
    static void * operator new ( size_t nSize )                     \
    {                                                               \
        if ( 0 == m_stDVInfo.dwValue )                              \
        {                                                           \
            m_stDVInfo.pfnDump     = classname::ClassDumper ;       \
            m_stDVInfo.pfnValidate = classname::ClassValidator ;    \
            AddClientDV ( &m_stDVInfo ) ;                           \
        }                                                           \
        return ( _malloc_dbg ( nSize                   ,            \
                               (int)m_stDVInfo.dwValue ,            \
                               __FILE__                ,            \
                               __LINE__                 ) ) ;       \
    }                                                               \
    static void * operator new ( size_t nSize        ,              \
                                 char * lpszFileName ,              \
                                 int    nLine         )             \
    {                                                               \
        if ( 0 == m_stDVInfo.dwValue )                              \
        {                                                           \
            m_stDVInfo.pfnDump     = classname::ClassDumper ;       \
            m_stDVInfo.pfnValidate = classname::ClassValidator ;    \
            AddClientDV ( &m_stDVInfo ) ;                           \
        }                                                           \
        return ( _malloc_dbg ( nSize                   ,            \
                               (int)m_stDVInfo.dwValue ,            \
                               lpszFileName            ,            \
                               nLine                    ) ) ;       \
    }                                                               \
    static void operator delete ( void * pData )                    \
    {                                                               \
        _free_dbg ( pData , (int)m_stDVInfo.dwValue ) ;             \
    }

// Declare this one at the top of the CPP file.
#define IMPLEMENT_MEMDEBUG(classname)                               \
    DVINFO  classname::m_stDVInfo

// The macro for memory debugging allocations.  If DEBUG_NEW is defined,
//  then it can be used.
#ifdef DEBUG_NEW
#define MEMDEBUG_NEW DEBUG_NEW
#else
#define MEMDEBUG_NEW new ( __FILE__ , __LINE__ )
#endif

#endif      // __cplusplus defined.

////////////////////////////////////////////////////////////////////////
// Helper C macros.
////////////////////////////////////////////////////////////////////////

// For C style allocations, here is the macro to use.  Unfortunately,
//  with C it is not so easy to use the auto-increment feature of
//  AddClientDV.
#define INITIALIZE_MEMDEBUG(bType , pfnD , pfnV )   \
    {                                               \
        DVINFO dvInfo ;                             \
        dvInfo.dwValue = bType ;                    \
        dvInfo.pfnDump = pfnD ;                     \
        dvInfo.pfnValidate = pfnV ;                 \
        AddClientDV ( &dvInfo ) ;                   \
    }

// The macros that map the C-style allocations.  It might be easier if
//  you use macros to wrap these so you don't have to remember which
//  client block value to drag around with each memory usage function.
#define MEMDEBUG_MALLOC(bType , nSize)  \
            _malloc_dbg ( nSize , bType , __FILE__ , __LINE__ )
#define MEMDEBUG_REALLOC(bType , pBlock , nSize)    \
            _realloc_dbg( pBlock , nSize , bType , __FILE__ , __LINE__ )
#define MEMDEBUG_EXPAND(bType , pBlock , nSize )    \
            _expand_dbg( pBlock , nSize , bType , __FILE__ , __LINE__ )
#define MEMDEBUG_FREE(bType , pBlock)   \
            _free_dbg ( pBlock , bType )
#define MEMDEBUG_MSIZE(bType , pBlock)  \
            _msize_dbg ( pBlock , bType )

// Macro to call ValidateAllBlocks
#define VALIDATEALLBLOCKS(x)   ValidateAllBlocks ( x )

#else       // _DEBUG is NOT defined

#ifdef __cplusplus
#define DECLARE_MEMDEBUG(classname)
#define IMPLEMENT_MEMDEBUG(classname)
#define MEMDEBUG_NEW new
#endif      // __cplusplus

#define MEMDEBUG_MALLOC(bType , nSize)              malloc ( nSize )
#define MEMDEBUG_REALLOC(bType , pBlock , nSize)    \
                                              realloc ( pBlock , nSize )
#define MEMDEBUG_EXPAND(bType , pBlock , nSize)     \
                                              _expand ( pBlock , nSize )
#define MEMDEBUG_FREE(bType , pBlock)               free ( pBlock )
#define MEMDEBUG_MSIZE(bType , pBlock)              _msize ( pBlock )

#define VALIDATEALLBLOCKS(x)

#endif      // _DEBUG

#ifdef __cplusplus
}
#endif      // __cplusplus

#endif      // _MEMDUMPERVALIDATOR_H




