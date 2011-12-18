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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "kyra/gui.h"
#include "kyra/kyra_v1.h"
#include "kyra/util.h"

#include "common/savefile.h"
#include "common/system.h"


namespace Kyra {

GUI::GUI(KyraEngine_v1 *kyra) : _vm(kyra), _screen(kyra->screen()) {
	_savegameListUpdateNeeded = false;
	_savegameListSize = 0;
	_savegameList = 0;
}

GUI::~GUI() {
	if (_savegameList) {
		for (int i = 0; i < _savegameListSize; i++)
			delete[] _savegameList[i];
		delete[] _savegameList;
		_savegameList = 0;
	}
}

void GUI::updateSaveList(bool excludeQuickSaves) {
	Common::String pattern = _vm->_targetName + ".???";
	Common::StringArray saveFileList = _vm->_saveFileMan->listSavefiles(pattern);
	_saveSlots.clear();

	for (Common::StringArray::const_iterator i = saveFileList.begin(); i != saveFileList.end(); ++i) {
		char s1 = 0, s2 = 0, s3 = 0;
		s1 = (*i)[i->size()-3];
		s2 = (*i)[i->size()-2];
		s3 = (*i)[i->size()-1];
		if (!isdigit(static_cast<unsigned char>(s1)) || !isdigit(static_cast<unsigned char>(s2)) || !isdigit(static_cast<unsigned char>(s3)))
			continue;
		s1 -= '0';
		s2 -= '0';
		s3 -= '0';
		if (excludeQuickSaves && s1 == 9 && s2 == 9)
			continue;
		_saveSlots.push_back(s1*100+s2*10+s3);
	}

	if (_saveSlots.begin() == _saveSlots.end())
		return;

	sortSaveSlots();
}

void GUI::sortSaveSlots() {
	Common::sort(_saveSlots.begin(), _saveSlots.end(), Common::Less<int>());
	if (_saveSlots.size() > 2)
		Common::sort(_saveSlots.begin()+1, _saveSlots.end(), Common::Greater<int>());
}

int GUI::getNextSavegameSlot() {
	Common::InSaveFile *in;

	int start = _vm->game() == GI_LOL ? 0 : 1;

	for (int i = start; i < 990; i++) {
		if ((in = _vm->_saveFileMan->openForLoading(_vm->getSavegameFilename(i))))
			delete in;
		else
			return i;
	}
	warning("Didn't save: Ran out of saveGame filenames");
	return 0;
}

void GUI::updateSavegameList() {
	if (!_savegameListUpdateNeeded)
		return;

	_savegameListUpdateNeeded = false;

	if (_savegameList) {
		for (int i = 0; i < _savegameListSize; i++)
			delete[] _savegameList[i];
		delete[] _savegameList;
	}

	updateSaveList(true);
	_savegameListSize = _saveSlots.size();

	if (_savegameListSize) {
		if (_vm->game() == GI_EOB1 || _vm->game() == GI_EOB2) {
			Common::sort(_saveSlots.begin(), _saveSlots.end(),  Common::Less<int>());
			_savegameListSize = _saveSlots.back() + 1;
		} else {
			Common::sort(_saveSlots.begin(), _saveSlots.end(),  Common::Greater<int>());
		}

		KyraEngine_v1::SaveHeader header;
		Common::InSaveFile *in;

		_savegameList = new char *[_savegameListSize];

		for (int i = 0; i < _savegameListSize; i++) {
			in = _vm->openSaveForReading(_vm->getSavegameFilename(i), header);
			if (in) {
				_savegameList[i] = new char[header.description.size() + 1];
				Common::strlcpy(_savegameList[i], header.description.c_str(), header.description.size() + 1);
				Util::convertISOToDOS(_savegameList[i]);
				delete in;
			} else {
				_savegameList[i] = 0;
			}
		}

	} else {
		_savegameList = 0;
	}
}

} // End of namespace Kyra


