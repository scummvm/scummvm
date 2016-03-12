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

#ifndef TITANIC_TT_SCRIPT_BASE_H
#define TITANIC_TT_SCRIPT_BASE_H

#include "titanic/true_talk/tt_string.h"

namespace Titanic {

class TTScriptBase {
private:
	void reset();
protected:
	int _field4;
	int _field8;
	int _fieldC;
	TTString _charName, _charClass;
	int _field20;
	int _field24;
	int _field28;
	int _field2C;
	int _field30;
	int _field34;
	int _field38;
	int _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _status;
public:
	TTScriptBase(int v1, const char *charClass, int v2, const char *charName,
		int v3, int v4, int v5, int v6, int v7);

	bool areNamesValid();

	int getStatus() const { return _status; }

	virtual void proc2(int v);

	virtual void proc3(int v);

	virtual void proc4(int v);

	virtual void proc5();
};


} // End of namespace Titanic

#endif /* TITANIC_TT_SCRIPT_BASE_H */
