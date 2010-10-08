#ifndef INCLUDED_FROM_BASE_VERSION_CPP
#error This file may only be included by base/version.cpp
#endif

#ifndef SCUMMVM_SVN_REVISION
#define SCUMMVM_SVN_REVISION
#endif

#ifdef RELEASE_BUILD
#undef SCUMMVM_SVN_REVISION
#define SCUMMVM_SVN_REVISION
#endif

#define SCUMMVM_VERSION "1.2.0" SCUMMVM_SVN_REVISION
