#ifndef INCLUDED_FROM_BASE_VERSION_CPP
#error This file may only be included by base/version.cpp
#endif

#ifndef RESIDUAL_SVN_REVISION
#define RESIDUAL_SVN_REVISION
#endif

#ifdef RELEASE_BUILD
#undef RESIDUAL_SVN_REVISION
#define RESIDUAL_SVN_REVISION
#endif

#define RESIDUAL_VERSION "0.0.6git" RESIDUAL_SVN_REVISION
