/*
 This file is part of WME Lite.
 http://dead-code.org/redir.php?target=wmelite
 
 Copyright (c) 2011 Jan Nedoma
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef WINTERMUTE_PERSISTENT_H
#define WINTERMUTE_PERSISTENT_H

#include "wintypes.h"

namespace WinterMute {
	
	class CBPersistMgr;
	
	// persistence support
	typedef void *(WINAPI *PERSISTBUILD)(void);
	typedef HRESULT(WINAPI *PERSISTLOAD)(void *, CBPersistMgr *);
	typedef void (*SYS_INSTANCE_CALLBACK)(void *Instance, void *Data);
} // end of namespace WinterMute

#include "SysClass.h"
#include "SysClassRegistry.h"
namespace WinterMute {
	

#define DECLARE_PERSISTENT(class_name, parent_class)\
static const char m_ClassName[];\
static void* WINAPI PersistBuild(void);\
virtual const char* GetClassName();\
static HRESULT WINAPI PersistLoad(void* Instance, CBPersistMgr* PersistMgr);\
class_name(TDynamicConstructor p1, TDynamicConstructor p2):parent_class(p1, p2){ /*memset(this, 0, sizeof(class_name));*/ };\
virtual HRESULT Persist(CBPersistMgr* PersistMgr);\
void* operator new (size_t size);\
void operator delete(void* p);\

	
#define IMPLEMENT_PERSISTENT(class_name, persistent_class)\
const char class_name::m_ClassName[] = #class_name;\
void* class_name::PersistBuild(){\
return ::new class_name(DYNAMIC_CONSTRUCTOR, DYNAMIC_CONSTRUCTOR);\
}\
\
HRESULT class_name::PersistLoad(void* Instance, CBPersistMgr* PersistMgr){\
return ((class_name*)Instance)->Persist(PersistMgr);\
}\
\
const char* class_name::GetClassName(){\
return #class_name;\
}\
\
CSysClass Register##class_name(class_name::m_ClassName, class_name::PersistBuild, class_name::PersistLoad, persistent_class);\
\
void* class_name::operator new (size_t size){\
void* ret = ::operator new(size);\
CSysClassRegistry::GetInstance()->RegisterInstance(#class_name, ret);\
return ret;\
}\
\
void class_name::operator delete (void* p){\
CSysClassRegistry::GetInstance()->UnregisterInstance(#class_name, p);\
::operator delete(p);\
}\

#define TMEMBER(member_name) #member_name, &member_name
#define TMEMBER_INT(member_name) #member_name, (int*)&member_name
	
} // end of namespace WinterMute

#endif // WINTERMUTE_PERSISTENT_H
