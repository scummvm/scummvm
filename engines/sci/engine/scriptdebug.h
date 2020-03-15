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

#ifndef SCI_ENGINE_SCRIPTDEBUG_H
#define SCI_ENGINE_SCRIPTDEBUG_H

namespace Sci {

//#ifndef REDUCE_MEMORY_USAGE
extern const char *opcodeNames[];
//#endif

void debugSelectorCall(reg_t send_obj, Selector selector, int argc, StackPtr argp, ObjVarRef &varp, reg_t funcp, SegManager *segMan, SelectorType selectorType);

void debugPropertyAccess(Object *obj, reg_t objp, unsigned int index, Selector selector, reg_t curValue, reg_t newValue, SegManager *segMan, BreakpointType breakpointType);

void logKernelCall(const KernelFunction *kernelCall, const KernelSubFunction *kernelSubCall, EngineState *s, int argc, reg_t *argv, reg_t result);

void logExportCall(uint16 script, uint16 pubfunct, EngineState *s, int argc, reg_t *argv);

void logBacktrace();

bool printObject(reg_t obj);

bool matchKernelBreakpointPattern(const Common::String &pattern, const Common::String &name);

}

#endif
