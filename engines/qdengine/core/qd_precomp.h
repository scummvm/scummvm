#ifndef __QD_PRECOMP_H__
#define __QD_PRECOMP_H__

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/system.h"

#define FOR_EACH(list, iterator) \
    for(iterator = (list).begin(); iterator != (list).end(); ++iterator)

 // STUB: XZipStream
#define _NO_ZIP_

// STUB FIXME
#define APIENTRY
#define BOOL          bool
#define CALLBACK
#define HMMIO         void *
#define HWND          void *
#define LPARAM        int
#define LPSTR         char *
#define LRESULT       long
#define MMCKINFO      int
#define MSG           int
#define PASCAL
#define UINT          unsigned int
#define WPARAM        int
#define WAVEFORMATEX  int
#define _MAX_DRIVE    3
#define _MAX_DIR      256
#define _MAX_FNAME    256
#define _MAX_EXT      4
#define VK_ESCAPE     0x1B
#define VK_RETURN     0x0D
#define VK_SPACE      0x20
#define VK_LEFT       0x25
#define VK_UP         0x26
#define VK_RIGHT      0x27
#define VK_DOWN       0x28
#define VK_BACK       0x08
#define VK_DELETE     0x2E
#define VK_HOME       0x24
#define VK_END        0x23

// STUB FIXME
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
    warning("STUB: _splitpath");
}

#ifndef _QUEST_EDITOR
#else
/*  #define   _STLP_NO_NEW_IOSTREAMS 1

    #ifdef _STLP_NO_NEW_IOSTREAMS
        #define   _STLP_NO_OWN_NAMESPACE 1
    #else
        #define _STLP_USE_OWN_NAMESPACE 1
        #define _STLP_REDEFINE_STD 1
    #endif*/
#endif // _QUEST_EDITOR

#ifndef _FINAL_VERSION
#ifndef __QD_SYSLIB__
#define __QD_DEBUG_ENABLE__
#endif
#endif

#ifdef __QD_DEBUG_ENABLE_
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


#if _MSC_VER < 1300
#define for if(false); else for
#endif

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#include "qdengine/xlibs/xutil/xutil.h"
#include "qdengine/xlibs/util/xmath/xmath.h"
#include "qdengine/xlibs/util/xmath/XMathLib.h"

#ifndef _NO_ZIP_
#if 0
#include "qdengine/xlibs/XZip/XZip.h"
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
#include "qdengine/xlibs/STLPort/stlport/hash_map"
#endif
#include <algorithm>
#include <functional>

#if 0
#include "XMath\SafeMath.h"
#endif

#include "qdengine/core/system/app_core.h"
#include "qdengine/core/system/app_log.h"
#include "qdengine/core/system/app_error_handler.h"

#include "qdengine/core/system/input/mouse_input.h"
#include "qdengine/core/system/input/keyboard_input.h"

#include "qdengine/core/qdcore/util/qd_save_stream.h"
#include "engines/qdengine/core/qd_fwd.h"

#endif /* __QD_PRECOMP_H__ */
