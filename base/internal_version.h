#ifndef INCLUDED_FROM_BASE_VERSION_CPP
#error This file may only be included by base/version.cpp
#endif

#ifdef RELEASE_BUILD
#undef SCUMMVM_SVN_REVISION
#endif

#ifndef SCUMMVM_SVN_REVISION
#define SCUMMVM_SVN_REVISION
#endif

#define SCUMMVM_VERSION "1.3.0svn" SCUMMVM_SVN_REVISION
