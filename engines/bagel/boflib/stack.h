
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

#ifndef BAGEL_BOFLIB_STACK_H
#define BAGEL_BOFLIB_STACK_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/llist.h"

namespace Bagel {

class CStack : public CBofObject {
private:
	CLList *m_pStackList = nullptr;

public:
	/**
	 * Default constructor
	 */
	CStack();

	/**
	 * Constructor that adds a passed object onto new stack
	 */
	CStack(VOID *pObject);

	/**
	 * Constructor that copies over a source stack
	 */
	CStack(CStack *pStack);

	/**
	 * Constructor that copies over a source stack
	 */
	CStack(const CStack &cStack);

	/**
	 * Destructor
	 */
	virtual ~CStack();

	/**
	 * Pushes object onto stack
	 * @param pObject		Pointer to object to be pushed
	 */
	VOID Push(VOID *pObject);

	/**
	 * Pops 1 object off stack
	 * @return		Pointer to top object on stack
	 */
	VOID *Pop();
};

} // namespace Bagel

#endif
