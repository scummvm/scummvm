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

#ifndef HODJNPODJ_BARBERSHOP_UNDO_H
#define HODJNPODJ_BARBERSHOP_UNDO_H

#include "bagel/hodjnpodj/barbershop/paint.h"
#include "bagel/hodjnpodj/barbershop/board.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

constexpr int NONE = -1;

#define UNDO_LEF        432         // Undo hotspot rect
#define UNDO_TOP        80
#define UNDO_RIG        (UNDO_LEF + 98)
#define UNDO_BOT        (UNDO_TOP + 94)


class CUndo {
public:
	CUndo();
	~CUndo();

	void    Record(CStack*, CCard*);
	void    Record(int);
	void    Reset();
	bool    Undo(CDC*, CBoard*, CPaint*);

	CRect   m_cUndoRect;
private:
	CStack  *m_pStack;
	CCard   *m_pCard;
	int     m_nStock;
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
