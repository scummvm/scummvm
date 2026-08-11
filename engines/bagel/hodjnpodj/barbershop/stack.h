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

#ifndef HODJNPODJ_BARBERSHOP_STACK_H
#define HODJNPODJ_BARBERSHOP_STACK_H

#include "bagel/hodjnpodj/barbershop/card.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

#define STACKS  8


class CStack {
private:    // vars
	int     m_nID;
	CCard   *m_pTopCard;
	CCard   *m_pBottomCard;
	int     m_nSize;

public:     // functions

	CStack(CRect, int nID);
	~CStack();

	void    Reset();
	void    Unlink(CCard*);
	void    Push(CCard*);
	CCard *Pop();
	CCard *Top() const {
		return m_pTopCard;
	}
	CCard *Bottom() const {
		return m_pBottomCard;
	}
	bool IsEmpty() const {
		return (bool)(m_pTopCard == nullptr);
	}
	int GetID() const {
		return m_nID;
	}
	bool    IsTab() {
		return (bool)(m_nID >= tab && m_nID < stock);
	}
	int Size();

public:
	CRect   m_cRect;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
