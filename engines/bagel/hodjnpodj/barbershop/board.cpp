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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/barbershop/board.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

CBoard::CBoard(CPaint *m_cPnt) {
	int i;

	m_pFound = new CStack(m_cPnt->m_cFound, fnd);

	for (i = 0; i < TAB_COUNT; i++)
		m_pTab[i] = new CStack(m_cPnt->m_cTab[i], tab + i);

	m_pStock = new CStack(m_cPnt->m_cStock, stock);
	m_pUsed  = new CStack(m_cPnt->m_cUsed, used);
}

CBoard::~CBoard() {
	int i;

	if (m_pUsed != nullptr) {
		delete m_pUsed;
		m_pUsed = nullptr;
	}

	if (m_pStock != nullptr) {
		delete m_pStock;
		m_pStock = nullptr;
	}

	for (i = 0; i < TAB_COUNT; i++) {
		if (m_pTab[i] != nullptr) {
			delete m_pTab[i];
			m_pTab[i] = nullptr;
		}
	}

	if (m_pFound != nullptr) {
		delete m_pFound;
		m_pFound = nullptr;
	}
}

CStack *CBoard::GetStack(loc nStack) {
	int i;

	switch (nStack) {

	case fnd:
		return m_pFound;

	case stock:
		return m_pStock;

	case used:
		return m_pUsed;

	default:
		for (i = 0; i < TAB_COUNT; i++)
			if (m_pTab[i]->GetID() == nStack)
				return m_pTab[i];
	}

	return nullptr;
}

bool CBoard::IsTabStack(int nStack) {
	switch (nStack) {

	case fnd:
		return false;

	case stock:
		return false;

	case used:
		return false;

	default:
		return true;
	}
}

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel
