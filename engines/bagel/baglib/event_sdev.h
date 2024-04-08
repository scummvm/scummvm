
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

#ifndef BAGEL_BAGLIB_EVENT_SDEV_H
#define BAGEL_BAGLIB_EVENT_SDEV_H

#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {

class CBagEventSDev : public CBagStorageDev {
private:
	static bool m_bEvalTurnEvents;

public:
	CBagEventSDev() : CBagStorageDev() {}
	virtual ~CBagEventSDev() {}
	static void initialize() {
		m_bEvalTurnEvents = FALSE;
	}

	/**
	 * Called to overload new set backgrounds.  Calls the set backdrop pure
	 * virtual function by default.
	 */
	virtual ERROR_CODE Attach();

	virtual ERROR_CODE SetBackground(CBofBitmap * /*pBmp*/) {
		return ERR_NONE;
	}
	virtual CBofBitmap *GetBackground() {
		return nullptr;
	}

	/**
	 * Evaluate only the expression object of this storage device
	 * @return  Returns and error code if there is an invalid object in the list
	 */
	virtual ERROR_CODE EvaluateExpressions();

	// Gives timer code a method to launch tim
	static void SetEvalTurnEvents(bool b = TRUE) {
		m_bEvalTurnEvents = b;
	}
	static bool GetEvalTurnEvents() {
		return m_bEvalTurnEvents;
	}
};

class CBagTurnEventSDev : public CBagStorageDev {
public:
	CBagTurnEventSDev() : CBagStorageDev() {}
	virtual ~CBagTurnEventSDev() {}

	virtual ERROR_CODE SetBackground(CBofBitmap * /*pBmp*/) {
		return ERR_NONE;
	}
	virtual CBofBitmap *GetBackground() {
		return nullptr;
	}

	virtual ERROR_CODE EvaluateExpressions();
};

} // namespace Bagel

#endif
