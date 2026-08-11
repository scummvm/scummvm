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

#include "mohawk/livingbooks.h"

#include "common/textconsole.h"

namespace Mohawk {

void MohawkEngine_LivingBooks::handleUIMenuClick(uint controlId) {
	LBItem *item;

	switch (controlId) {
	case 1:
		if (getFeatures() & GF_LB_10) {
			if (!tryLoadPageStart(kLBControlMode, 2))
				error("couldn't load options page");
		} else {
			if (!tryLoadPageStart(kLBControlMode, 3))
				error("couldn't load options page");
		}
		break;

	case 2:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById(199 + _curLanguage);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 3:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById(12);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 4:
		if (getFeatures() & GF_LB_10) {
			if (!tryLoadPageStart(kLBControlMode, 3))
				error("couldn't load quit page");
		} else {
			if (!tryLoadPageStart(kLBControlMode, 2))
				error("couldn't load quit page");
		}
		break;

	case 10:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false);
		}
		break;

	case 11:
		item = getItemById(11);
		if (item)
			item->togglePlaying(false, true);
		break;

	case 12:
		// start game, in play mode
		if (!tryLoadPageStart(kLBPlayMode, 1))
			error("couldn't start play mode");
		break;

	default:
		if (controlId >= 100 && controlId < 100 + (uint)_numLanguages) {
			uint newLanguage = controlId - 99;
			if (newLanguage == _curLanguage)
				break;
			item = getItemById(99 + _curLanguage);
			if (item)
				item->seek(1);
			_curLanguage = newLanguage;
		} else if (controlId >= 200 && controlId < 200 + (uint)_numLanguages) {
			// start game, in read mode
			if (!tryLoadPageStart(kLBReadMode, 1))
				error("couldn't start read mode");
		}
		break;
	}
}

void MohawkEngine_LivingBooks::handleUIPoetryMenuClick(uint controlId) {
	LBItem *item;

	// the menu UI in New Kid on the Block is a hybrid of the normal menu
	// and the normal options screen

	// TODO: this is mostly untested

	switch (controlId) {
	case 2:
	case 3:
		handleUIOptionsClick(controlId);
		break;

	case 4:
		handleUIMenuClick(controlId);
		break;

	case 6:
		handleUIMenuClick(2);
		break;

	case 7:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById(12);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 0xA:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false);
		}
		break;

	case 0xB:
		item = getItemById(11);
		if (item)
			item->togglePlaying(false, true);
		break;

	case 0xC:
		if (!tryLoadPageStart(kLBPlayMode, _curSelectedPage))
			error("failed to load page %d", _curSelectedPage);
		break;

	default:
		if (controlId < 100) {
			handleUIMenuClick(controlId);
		} else {
			if (!tryLoadPageStart(kLBReadMode, _curSelectedPage))
				error("failed to load page %d", _curSelectedPage);
		}
	}
}

void MohawkEngine_LivingBooks::handleUIQuitClick(uint controlId) {
	LBItem *item;

	switch (controlId) {
	case 1:
	case 2:
		// button clicked, run animation
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(11);
		if (item)
			item->destroySelf();
		item = getItemById((controlId == 1) ? 12 : 13);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false);
		}
		break;

	case 10:
	case 11:
		item = getItemById(11);
		if (item)
			item->togglePlaying(false, true);
		break;

	case 12:
		// 'yes', I want to quit
		quitGame();
		break;

	case 13:
		// 'no', go back to menu
		if (!tryLoadPageStart(kLBControlMode, 1))
			error("couldn't return to menu");
		break;

	default:
		break;
	}
}

void MohawkEngine_LivingBooks::handleUIOptionsClick(uint controlId) {
	LBItem *item;

	switch (controlId) {
	case 1:
		item = getItemById(10);
		if (item)
			item->destroySelf();
		item = getItemById(202);
		if (item) {
			item->setVisible(true);
			item->togglePlaying(false, true);
		}
		break;

	case 2:
		// back
		item = getItemById(2);
		if (item)
			item->seek(1);
		if (_curSelectedPage == 1) {
			_curSelectedPage = _numPages;
		} else {
			_curSelectedPage--;
		}
		for (uint i = 0; i < _numPages; i++) {
			item = getItemById(1000 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
			item = getItemById(1100 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
		}
		break;

	case 3:
		// forward
		item = getItemById(3);
		if (item)
			item->seek(1);
		if (_curSelectedPage == _numPages) {
			_curSelectedPage = 1;
		} else {
			_curSelectedPage++;
		}
		for (uint i = 0; i < _numPages; i++) {
			item = getItemById(1000 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
			item = getItemById(1100 + i);
			if (item)
				item->setVisible(_curSelectedPage == i + 1);
		}
		break;

	case 4:
		if (!tryLoadPageStart(kLBCreditsMode, 1))
			error("failed to start credits");
		break;

	case 5:
		if (!tryLoadPageStart(kLBPreviewMode, 1))
			error("failed to start preview");
		break;

	case 202:
		if (!tryLoadPageStart(kLBPlayMode, _curSelectedPage))
			error("failed to load page %d", _curSelectedPage);
		break;

	default:
		break;
	}
}

} // End of namespace Mohawk
