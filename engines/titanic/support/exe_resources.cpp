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

#include "titanic/support/exe_resources.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/titanic.h"

namespace Titanic {

CExeResources::CExeResources() : _owner(nullptr), _field4(0), _field8(0),
		_fieldC(0), _field10(0), _field14(0), _vocabMode(VOCAB_MODE_NONE) {
}

void CExeResources::reset(CScriptHandler *owner, int val1, VocabMode vocabMode) {
	_owner = owner;
	_vocabMode = vocabMode;
}

} // End of namespace Titanic
