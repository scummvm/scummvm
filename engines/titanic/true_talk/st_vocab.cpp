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

#include "common/file.h"
#include "titanic/true_talk/st_vocab.h"
#include "titanic/titanic.h"

namespace Titanic {

STVocab::STVocab(int val): _field0(0), _field4(0), _word(nullptr),
		_fieldC(0), _field10(0), _field18(val) {
	_field14 = load("STVOCAB.TXT");
}

int STVocab::load(const CString &name) {
	SimpleFile *file = g_vm->_fileReader._owner->openResource(name);
	int result = 0;
	bool skipFlag;

	while (!result && !file->eos()) {
		skipFlag = false;
		int mode = file->readNumber();
		TTString space(" ");

		switch (mode) {
		case 0: {
			if (_word)
				result = _word->readSyn(file);
			skipFlag = true;
			break;
		}

		case 1: {
			TTWord2 *word = new TTWord2(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 2: {
			TTWord3 *word = new TTWord3(space, 0, 0, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 3:
		case 9: {
			TTWord1 *word = new TTWord1(space, 0, 0, 0);
			result = word->load(file, mode);
			_word = word;
			break;
		}

		case 4:
		case 5:
		case 7: {
			TTWord *word = new TTWord(space, 0, 0);
			result = word->load(file, mode);
			_word = word;
			break;
		}

		case 8: {
			TTWord4 *word = new TTWord4(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		case 6: {
			TTWord5 *word = new TTWord5(space, 0, 0, 0, 0);
			result = word->load(file);
			_word = word;
			break;
		}

		default:
			result = 4;
			break;
		}
	}

	delete file;
	return result;
}

} // End of namespace Titanic
