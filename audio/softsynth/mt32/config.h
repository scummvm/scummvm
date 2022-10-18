/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011-2021 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MT32EMU_CONFIG_H
#define MT32EMU_CONFIG_H

#define MT32EMU_VERSION      "2.7.0"
#define MT32EMU_VERSION_MAJOR 2
#define MT32EMU_VERSION_MINOR 7
#define MT32EMU_VERSION_PATCH 0

/* Library Exports Configuration
 *
 * This reflects the API types actually provided by the library build.
 * 0: The full-featured C++ API is only available in this build. The client application may ONLY use MT32EMU_API_TYPE 0.
 * 1: The C-compatible API is only available. The library is built as a shared object, only C functions are exported,
 *    and thus the client application may NOT use MT32EMU_API_TYPE 0.
 * 2: The C-compatible API is only available. The library is built as a shared object, only the factory function
 *    is exported, and thus the client application may ONLY use MT32EMU_API_TYPE 2.
 * 3: All the available API types are provided by the library build.
 */
#define MT32EMU_EXPORTS_TYPE 0
#define MT32EMU_API_TYPE 0

/* Type of library build.
 *
 * For shared library builds, MT32EMU_SHARED is defined, so that compiler-specific attributes are assigned
 * to all the exported symbols as appropriate. MT32EMU_SHARED is undefined for static library builds.
 */
#undef MT32EMU_SHARED

/* Whether the library is built as a shared object with a version tag to enable runtime version checks. */
#define MT32EMU_WITH_VERSION_TAGGING 0

/* Automatic runtime check of the shared library version in client applications.
 *
 * When the shared library is built with version tagging enabled, the client application may rely on an automatic
 * version check that ensures forward compatibility. See VersionTagging.h for more info.
 * 0: Disables the automatic runtime version check in the client application. Implied for static library builds
 *    and when version tagging is not used in a shared object.
 * 1: Enables an automatic runtime version check in client applications that utilise low-level C++ API,
 *    i.e. when MT32EMU_API_TYPE 0. Client applications that rely on the C-compatible API are supposed
 *    to check the version of the shared object by other means (e.g. using versioned C symbols, etc.).
 * 2: Enables an automatic runtime version check for C++ and C client applications.
 */
#if MT32EMU_WITH_VERSION_TAGGING
#  ifndef MT32EMU_RUNTIME_VERSION_CHECK
#    define MT32EMU_RUNTIME_VERSION_CHECK @libmt32emu_RUNTIME_VERSION_CHECK@
#  endif
#else
#  undef MT32EMU_RUNTIME_VERSION_CHECK
#endif

#endif /* #ifndef MT32EMU_CONFIG_H */
