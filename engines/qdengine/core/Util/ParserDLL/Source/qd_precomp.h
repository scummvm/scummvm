#ifndef __QD_PRECOMP_H__
#define __QD_PRECOMP_H__

#include <crtdbg.h>

#ifdef _DEBUG
#define DBGCHECK _ASSERTE(_CrtCheckMemory())
#else
#define DBGCHECK
#endif

//#define _GR_ENABLE_ZBUFFER

#include <Additional\addition.h>
#include <assert.h>

//#define   _STLP_NO_NEW_IOSTREAMS 1

#ifdef _STLP_NO_NEW_IOSTREAMS
#define   _STLP_NO_OWN_NAMESPACE 1
#else
#  define _STLP_USE_OWN_NAMESPACE 1
# define _STLP_REDEFINE_STD 1
#endif

#if _MSC_VER < 1300
#define for if(false); else for
#endif

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#include <windows.h>
#include <XUtil.h>

#endif /* __QD_PRECOMP_H__ */
