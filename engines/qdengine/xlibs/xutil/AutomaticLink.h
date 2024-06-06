/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// define parameter _LIB_NAME

#if defined(_AFXDLL)
#define _MT_DLL_SUFFIX "Dll"
#elif defined(_DLL) || defined(_MTD)
#define _MT_DLL_SUFFIX "Dll"
#elif defined(_MT)
#define _MT_DLL_SUFFIX "Mt"
#else
#define _MT_DLL_SUFFIX
#endif

#ifdef _DEBUG
#define _DEBUG_SUFFIX "Dbg"
#else
#define _DEBUG_SUFFIX
#endif

#define _FULL_NAME_ _LIB_NAME _MT_DLL_SUFFIX _DEBUG_SUFFIX ".lib"
#pragma message("Automatically linking with " _FULL_NAME_)
#pragma comment(lib, _FULL_NAME_)

#undef _FULL_NAME_
#undef _DEBUG_SUFFIX
#undef _MT_DLL_SUFFIX
#undef _LIB_NAME
