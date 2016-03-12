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

#ifndef TITANIC_TT_SCRIPTS_H
#define TITANIC_TT_SCRIPTS_H

#include "titanic/core/list.h"
#include "titanic/true_talk/tt_named_script.h"
#include "titanic/true_talk/tt_unnamed_script.h"

namespace Titanic {

class CTitleEngine;

PTR_LIST_ITEM(TTNamedScript);
PTR_LIST_ITEM(TTUnnamedScript);

class TTNamedScriptList : public List<TTNamedScriptListItem> {
};

class TTUnamedScriptList : public List<TTUnnamedScriptListItem> {
};

class TTScripts {
private:
	TTNamedScriptList _namedScripts;
	TTUnamedScriptList _unnamedScripts;
	CTitleEngine *_titleEngine;
	int _field24;
	int _field28;
private:
	/**
	 * Add a named script to the named scripts list
	 */
	void addScript(TTNamedScript *script);

	/**
	 * Add an unnamed script to the unnamed scripts list
	 */
	void addScript(TTUnnamedScript *script);
public:
	TTScripts(CTitleEngine *titleEngine);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_CHARACTERS_H */
