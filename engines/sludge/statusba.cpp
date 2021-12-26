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

#include "common/file.h"
#include "common/system.h"

#include "sludge/fonttext.h"
#include "sludge/graphics.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/statusba.h"

namespace Sludge {

StatusBarManager::StatusBarManager(SludgeEngine *sludge) {
	_nowStatus = &_mainStatus;
	_sludge = sludge;
}

void StatusBarManager::init() {
	_mainStatus.firstStatusBar = NULL;
	_mainStatus.alignStatus = IN_THE_CENTRE;
	_mainStatus.litStatus = -1;
	_mainStatus.statusX = 10;
	_mainStatus.statusY = g_system->getHeight() - 15;
	statusBarColour(255, 255, 255);
	statusBarLitColour(255, 255, 128);
}

void StatusBarManager::setLitStatus(int i) {
	_nowStatus->litStatus = i;
}

void StatusBarManager::killLastStatus() {
	if (_nowStatus->firstStatusBar) {
		StatusBar *kill = _nowStatus->firstStatusBar;
		_nowStatus->firstStatusBar = kill->next;
		delete kill;
	}
}

void StatusBarManager::clear() {
	StatusBar *stat = _nowStatus->firstStatusBar;
	StatusBar *kill;
	_nowStatus->litStatus = -1;
	while (stat) {
		kill = stat;
		stat = stat->next;
		delete kill;
	}
	_nowStatus->firstStatusBar = NULL;
}

void StatusBarManager::addStatusBar() {
	StatusBar *newStat = new StatusBar;
	if (checkNew(newStat)) {
		newStat->next = _nowStatus->firstStatusBar;
		newStat->text.clear();
		_nowStatus->firstStatusBar = newStat;
	}
}

void StatusBarManager::set(Common::String &txt) {
	if (_nowStatus->firstStatusBar) {
		_nowStatus->firstStatusBar->text.clear();
		_nowStatus->firstStatusBar->text = txt;
	}
}

void StatusBarManager::positionStatus(int x, int y) {
	_nowStatus->statusX = x;
	_nowStatus->statusY = y;
}

void StatusBarManager::draw() {
	float cameraZoom = _sludge->_gfxMan->getCamZoom();
	int y = _nowStatus->statusY, n = 0;
	StatusBar *stat = _nowStatus->firstStatusBar;
	while (stat) {
		switch (_nowStatus->alignStatus) {
		case IN_THE_CENTRE:
			_sludge->_txtMan->pasteString(stat->text,
					((g_system->getWidth() - _sludge->_txtMan->stringWidth(stat->text)) >> 1) / cameraZoom, y / cameraZoom,
					(n++ == _nowStatus->litStatus) ? _litVerbLinePalette : _verbLinePalette);
			break;

		case 1001:
			_sludge->_txtMan->pasteString(stat->text,
					(g_system->getWidth() - _sludge->_txtMan->stringWidth(stat->text)) - _nowStatus->statusX / cameraZoom, y / cameraZoom,
					(n ++ == _nowStatus->litStatus) ? _litVerbLinePalette : _verbLinePalette);
			break;

		default:
			_sludge->_txtMan->pasteString(stat->text,
					_nowStatus->statusX / cameraZoom, y / cameraZoom,
					(n ++ == _nowStatus->litStatus) ? _litVerbLinePalette : _verbLinePalette);
		}
		stat = stat->next;
		y -= _sludge->_txtMan->getFontHeight();
	}
}

void StatusBarManager::statusBarColour(byte r, byte g, byte b) {
	_verbLinePalette.setColor(r, g, b);
	_nowStatus->statusR = r;
	_nowStatus->statusG = g;
	_nowStatus->statusB = b;
}

void StatusBarManager::statusBarLitColour(byte r, byte g, byte b) {
	_litVerbLinePalette.setColor(r, g, b);
	_nowStatus->statusLR = r;
	_nowStatus->statusLG = g;
	_nowStatus->statusLB = b;
}

StatusStuff *StatusBarManager::copyStatusBarStuff(StatusStuff *here) {

	// Things we want to keep
	here->statusLR = _nowStatus->statusLR;
	here->statusLG = _nowStatus->statusLG;
	here->statusLB = _nowStatus->statusLB;
	here->statusR = _nowStatus->statusR;
	here->statusG = _nowStatus->statusG;
	here->statusB = _nowStatus->statusB;
	here->alignStatus = _nowStatus->alignStatus;
	here->statusX = _nowStatus->statusX;
	here->statusY = _nowStatus->statusY;

	// Things we want to clear
	here->litStatus = -1;
	here->firstStatusBar = NULL;

	StatusStuff *old = _nowStatus;
	_nowStatus = here;

	return old;
}

void StatusBarManager::restoreBarStuff(StatusStuff *here) {
	delete _nowStatus;
	_verbLinePalette.setColor((byte)here->statusR, (byte)here->statusG, (byte)here->statusB);
	_litVerbLinePalette.setColor((byte)here->statusLR, (byte)here->statusLG, (byte)here->statusLB);
	_nowStatus = here;
}


const Common::String StatusBarManager::statusBarText() {
	if (_nowStatus->firstStatusBar) {
		return _nowStatus->firstStatusBar->text;
	} else {
		return "";
	}
}

void StatusBarManager::saveStatusBars(Common::WriteStream *stream) {
	StatusBar *viewLine = _nowStatus->firstStatusBar;

	stream->writeUint16BE(_nowStatus->alignStatus);
	stream->writeSint16LE(_nowStatus->litStatus);
	stream->writeUint16BE(_nowStatus->statusX);
	stream->writeUint16BE(_nowStatus->statusY);

	stream->writeByte(_nowStatus->statusR);
	stream->writeByte(_nowStatus->statusG);
	stream->writeByte(_nowStatus->statusB);
	stream->writeByte(_nowStatus->statusLR);
	stream->writeByte(_nowStatus->statusLG);
	stream->writeByte(_nowStatus->statusLB);

	// Write what's being said
	while (viewLine) {
		stream->writeByte(1);
		writeString(viewLine->text, stream);
		viewLine = viewLine->next;
	}
	stream->writeByte(0);
}

bool StatusBarManager::loadStatusBars(Common::SeekableReadStream *stream) {
	clear();

	_nowStatus->alignStatus = stream->readUint16BE();
	_nowStatus->litStatus = stream->readSint16LE();
	_nowStatus->statusX = stream->readUint16BE();
	_nowStatus->statusY = stream->readUint16BE();

	_nowStatus->statusR = stream->readByte();
	_nowStatus->statusG = stream->readByte();
	_nowStatus->statusB = stream->readByte();
	_nowStatus->statusLR = stream->readByte();
	_nowStatus->statusLG = stream->readByte();
	_nowStatus->statusLB = stream->readByte();

	_verbLinePalette.setColor((byte)_nowStatus->statusR, (byte)_nowStatus->statusG, (byte)_nowStatus->statusB);
	_litVerbLinePalette.setColor((byte)_nowStatus->statusLR, (byte)_nowStatus->statusLG, (byte)_nowStatus->statusLB);
	// Read what's being said
	StatusBar **viewLine = &(_nowStatus->firstStatusBar);
	StatusBar *newOne;
	while (stream->readByte()) {
		newOne = new StatusBar;
		if (!checkNew(newOne))
			return false;
		newOne->text = readString(stream);
		newOne->next = NULL;
		(*viewLine) = newOne;
		viewLine = &(newOne->next);
	}
	return true;
}

} // End of namespace Sludge
