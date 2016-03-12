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

#ifndef TITANIC_TT_UNNAMED_SCRIPT_H
#define TITANIC_TT_UNNAMED_SCRIPT_H

#include "titanic/true_talk/tt_script_base.h"

namespace Titanic {

class TTUnnamedScriptBase : public TTScriptBase {
protected:
	int _scriptId;
public:
	TTUnnamedScriptBase(int scriptId, const char *charClass, const char *charName,
		int v3, int v4, int v5, int v6, int v2, int v7);

	virtual void proc6() = 0;
	virtual void proc7() = 0;
	virtual void proc8() = 0;
	virtual void proc9() = 0;
	virtual void proc10() = 0;
	virtual void proc11() = 0;
};


class TTUnnamedScript : public TTUnnamedScriptBase {
private:
	int _field54;
public:
	TTUnnamedScript(int scriptId);

	virtual void proc6();
	virtual void proc7();
	virtual void proc8();
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
};

} // End of namespace Titanic

#endif /* TITANIC_TT_UNNAMED_SCRIPT_H */
