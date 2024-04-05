#ifndef __QD_PRECOMP_H__
#define __QD_PRECOMP_H__

#ifndef _QUEST_EDITOR
	#include <my_stl.h>
#else
/*	#define   _STLP_NO_NEW_IOSTREAMS 1

	#ifdef _STLP_NO_NEW_IOSTREAMS
		#define   _STLP_NO_OWN_NAMESPACE 1
	#else
		#define _STLP_USE_OWN_NAMESPACE 1
		#define _STLP_REDEFINE_STD 1
	#endif*/
#endif // _QUEST_EDITOR

#include <crtdbg.h>

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

#include <Additional\addition.h>
#include <assert.h>

#if _MSC_VER < 1300
#define for if(false); else for
#endif

#pragma warning(disable : 4786)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)

#include <windows.h>
#include <XUtil.h>
#include "XMath\XMath.h"
#include "XMath\XMathLib.h"

#ifndef _NO_ZIP_
#include <XZip.h>
#else
class XZipStream : public XStream
{
	int dummy_;
};
#endif

#include <list>
#include <stack>
#include <vector>
#include <string>
#include <hash_map>
#include <algorithm>
#include <functional>

#include "XMath\SafeMath.h"

#include "app_core.h"
#include "app_log.h"
#include "app_error_handler.h"

#include "mouse_input.h"
#include "keyboard_input.h"

#include "qd_save_stream.h"

#include "qd_fwd.h"

#endif /* __QD_PRECOMP_H__ */
