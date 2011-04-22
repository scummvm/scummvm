#ifndef INCLUDED_FROM_BASE_VERSION_CPP
#error This file may only be included by base/version.cpp
#endif

// Loads revision number from an external file
#ifdef SCUMMVM_EXTERNAL_REVISION
#include "external_version.h"
#endif

#ifdef RELEASE_BUILD
#undef SCUMMVM_REVISION
#endif

#ifndef SCUMMVM_REVISION
#define SCUMMVM_REVISION
#endif

#define SCUMMVM_VERSION "1.3.0git" SCUMMVM_REVISION
