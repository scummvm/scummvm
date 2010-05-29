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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_ENGINE_SELECTOR_H
#define SCI_ENGINE_SELECTOR_H

#include "common/scummsys.h"

#include "sci/engine/vm_types.h"	// for reg_t
#include "sci/engine/vm.h"
#include "sci/engine/kernel.h"		// for Kernel::_selectorCache

namespace Sci {

enum SelectorInvocation {
	kStopOnInvalidSelector = 0,
	kContinueOnInvalidSelector = 1
};

/**
 * Map a selector name to a selector id. Shortcut for accessing the selector cache.
 */
#define SELECTOR(_slc_)		(g_sci->getKernel()->_selectorCache._slc_)

/**
 * Retrieves a selector from an object.
 * @param segMan	the segment mananger
 * @param _obj_		the address of the object which the selector should be read from
 * @param _slc_		the selector to refad
 * @return			the selector value as a reg_t
 * This macro halts on error. 'selector' must be a selector name registered in vm.h's
 * SelectorCache and mapped in script.cpp.
 */
reg_t readSelector(SegManager *segMan, reg_t object, Selector selectorId);
#define readSelectorValue(segMan, _obj_, _slc_) (readSelector(segMan, _obj_, _slc_).offset)

/**
 * Writes a selector value to an object.
 * @param segMan	the segment mananger
 * @param _obj_		the address of the object which the selector should be written to
 * @param _slc_		the selector to read
 * @param _val_		the value to write
 * This macro halts on error. 'selector' must be a selector name registered in vm.h's
 * SelectorCache and mapped in script.cpp.
 */
void writeSelector(SegManager *segMan, reg_t object, Selector selectorId, reg_t value);
#define writeSelectorValue(segMan, _obj_, _slc_, _val_) writeSelector(segMan, _obj_, _slc_, make_reg(0, _val_))

/**
 * Invokes a selector from an object.
 */
int invokeSelector(EngineState *s, reg_t object, int selectorId, SelectorInvocation noinvalid,
	int k_argc, StackPtr k_argp, int argc, ...);
int invokeSelectorArgv(EngineState *s, reg_t object, int selectorId, SelectorInvocation noinvalid,
	int k_argc, StackPtr k_argp, int argc, const reg_t *argv);

/**
 * Kludge for use with invokeSelector(). Used for compatibility with compilers
 * that cannot handle vararg macros.
 */
#define INV_SEL(s, _object_, _selector_, _noinvalid_) \
	s, _object_,  g_sci->getKernel()->_selectorCache._selector_, _noinvalid_, argc, argv


} // End of namespace Sci

#endif // SCI_ENGINE_KERNEL_H
