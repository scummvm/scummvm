/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_TRACE_H
#define COMMON_TRACE_H

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"

#define traceValueScoped(name, x) ZoneNamedN(__tracy_scoped_##name, #name, true); ZoneValueV(__tracy_scoped_##name, (uint64_t)(x))
#define tracePlot(name, x) TracyPlot(name, x)
#define traceMsg(message) TracyMessageL(message)
#define traceThread(name) tracy::SetThreadName(name)
#define traceMutex(type, varname, name) type varname { tracy::SourceLocationData{ nullptr, name, __FILE__, __LINE__, 0 } }

#define traceBeginFrame(name) (void)0
#define traceEndFrame(name) FrameMark
#define traceScoped(name) ZoneNamedN(__tracy_scoped_##name, #name, true)
#define traceScopedFunc() ZoneNamedN(__tracy_scoped_function_, __FUNCTION__, true)
#else
#define traceValueScoped(name, x) (void)0
#define tracePlot(name, x) (void)0
#define traceMsg(message) (void)0
#define traceThread(name) (void)0
#define traceMutex(type, varname, name) type varname

#define traceBeginFrame(name) (void)0
#define traceEndFrame(name) (void)0
#define traceScoped(name) (void)0
#define traceScopedFunc() (void)0
#endif

#endif
