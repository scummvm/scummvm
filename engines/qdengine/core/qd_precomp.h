#ifndef __QD_PRECOMP_H__
#define __QD_PRECOMP_H__

#ifndef _QUEST_EDITOR
#if 0
#include <my_stl.h>
#endif
#else
/*  #define   _STLP_NO_NEW_IOSTREAMS 1

    #ifdef _STLP_NO_NEW_IOSTREAMS
        #define   _STLP_NO_OWN_NAMESPACE 1
    #else
        #define _STLP_USE_OWN_NAMESPACE 1
        #define _STLP_REDEFINE_STD 1
    #endif*/
#endif // _QUEST_EDITOR

#if 0
#include <crtdbg.h>
#endif

#ifndef _FINAL_VERSION
#ifndef __QD_SYSLIB__
#define __QD_DEBUG_ENABLE__
#endif
#endif

#ifdef __QD_DEBUG_ENABLE__
#define __QDBG(a) a
#else
#define __QDBG(a)
#endif

#ifdef _DEBUG
#define DBGCHECK _ASSERTE(_CrtCheckMemory())
#else
#define DBGCHECK
#endif

//#define _GR_ENABLE_ZBUFFER
#if 0
#include "qdengine/xlibs/Heap/Additional/addition.h"
#endif

#include <assert.h>

#if _MSC_VER < 1300
#define for if(false); else for
#endif

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#if 0
#include <windows.h>
#endif

#if 0
#include "qdengine/xlibs/xutil/xutil.h"
#include "qdengine/xlibs/Util/XMath/xmath.h"
#include "qdengine/xlibs/Util/XMath/XMathLib.h"
#endif

#ifndef _NO_ZIP_
#if 0
#include "../xlibs/XZip/XZip.h"
#endif
#else
class XZipStream : public XStream {
int dummy_;
};
#endif

#include <list>
#include <stack>
#include <vector>
#include <string>
#if 0
#include "../xlibs/STLPort/stlport/hash_map"
#endif
#include <algorithm>
#include <functional>

#if 0
#include "XMath\SafeMath.h"
#endif

#include "System/app_core.h"
#include "System/app_log.h"
#include "System/app_error_handler.h"

#include "System/Input/mouse_input.h"
#include "System/Input/keyboard_input.h"

#include "qdCore/Util/qd_save_stream.h"

#include "qd_fwd.h"

#endif /* __QD_PRECOMP_H__ */
