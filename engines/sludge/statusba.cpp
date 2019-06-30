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

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/fonttext.h"
#include "sludge/graphics.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sprites.h"
#include "sludge/statusba.h"

namespace Sludge {

SpritePalette verbLinePalette;
SpritePalette litVerbLinePalette;

StatusStuff mainStatus;
StatusStuff *nowStatus = & mainStatus;

void setLitStatus(int i) {
	nowStatus->litStatus = i;
}

void killLastStatus() {
	if (nowStatus->firstStatusBar) {
		StatusBar *kill = nowStatus->firstStatusBar;
		nowStatus->firstStatusBar = kill->next;
		delete kill;
	}
}

void clearStatusBar() {
	StatusBar *stat = nowStatus->firstStatusBar;
	StatusBar *kill;
	nowStatus->litStatus = -1;
	while (stat) {
		kill = stat;
		stat = stat->next;
		delete kill;
	}
	nowStatus->firstStatusBar = NULL;
}

void addStatusBar() {
	StatusBar *newStat = new StatusBar;
	if (checkNew(newStat)) {
		newStat->next = nowStatus->firstStatusBar;
		newStat->text.clear();
		nowStatus->firstStatusBar = newStat;
	}
}

void setStatusBar(Common::String &txt) {
	if (nowStatus->firstStatusBar) {
		nowStatus->firstStatusBar->text.clear();
		nowStatus->firstStatusBar->text = txt;
	}
}

void positionStatus(int x, int y) {
	nowStatus->statusX = x;
	nowStatus->statusY = y;
}

void drawStatusBar() {
	float cameraZoom = g_sludge->_gfxMan->getCamZoom();
	int y = nowStatus->statusY, n = 0;
	StatusBar *stat = nowStatus->firstStatusBar;
	while (stat) {
		switch (nowStatus->alignStatus) {
		case IN_THE_CENTRE:
			g_sludge->_txtMan->pasteString(stat->text,
					((g_system->getWidth() - g_sludge->_txtMan->stringWidth(stat->text)) >> 1) / cameraZoom, y / cameraZoom,
					(n++ == nowStatus->litStatus) ? litVerbLinePalette : verbLinePalette);
			break;

		case 1001:
			g_sludge->_txtMan->pasteString(stat->text,
					(g_system->getWidth() - g_sludge->_txtMan->stringWidth(stat->text)) - nowStatus->statusX / cameraZoom, y / cameraZoom,
					(n ++ == nowStatus->litStatus) ? litVerbLinePalette : verbLinePalette);
			break;

		default:
			g_sludge->_txtMan->pasteString(stat->text,
					nowStatus->statusX / cameraZoom, y / cameraZoom,
					(n ++ == nowStatus->litStatus) ? litVerbLinePalette : verbLinePalette);
		}
		stat = stat->next;
		y -= g_sludge->_txtMan->getFontHeight();
	}
}

void statusBarColour(byte r, byte g, byte b) {
	verbLinePalette.setColor(r, g, b);
	nowStatus->statusR = r;
	nowStatus->statusG = g;
	nowStatus->statusB = b;
}

void statusBarLitColour(byte r, byte g, byte b) {
	litVerbLinePalette.setColor(r, g, b);
	nowStatus->statusLR = r;
	nowStatus->statusLG = g;
	nowStatus->statusLB = b;
}

StatusStuff *copyStatusBarStuff(StatusStuff  *here) {

	// Things we want to keep
	here->statusLR = nowStatus->statusLR;
	here->statusLG = nowStatus->statusLG;
	here->statusLB = nowStatus->statusLB;
	here->statusR = nowStatus->statusR;
	here->statusG = nowStatus->statusG;
	here->statusB = nowStatus->statusB;
	here->alignStatus = nowStatus->alignStatus;
	here->statusX = nowStatus->statusX;
	here->statusY = nowStatus->statusY;

	// Things we want to clear
	here->litStatus = -1;
	here->firstStatusBar = NULL;

	StatusStuff *old = nowStatus;
	nowStatus = here;

	return old;
}

void restoreBarStuff(StatusStuff *here) {
	delete nowStatus;
	verbLinePalette.setColor((byte)here->statusR, (byte)here->statusG, (byte)here->statusB);
	litVerbLinePalette.setColor((byte)here->statusLR, (byte)here->statusLG, (byte)here->statusLB);
	nowStatus = here;
}


void initStatusBar() {
	mainStatus.firstStatusBar = NULL;
	mainStatus.alignStatus = IN_THE_CENTRE;
	mainStatus.litStatus = -1;
	mainStatus.statusX = 10;
	mainStatus.statusY = g_system->getHeight() - 15;
	statusBarColour(255, 255, 255);
	statusBarLitColour(255, 255, 128);
}

const Common::String statusBarText() {
	if (nowStatus->firstStatusBar) {
		return nowStatus->firstStatusBar->text;
	} else {
		return "";
	}
}

void saveStatusBars(Common::WriteStream *stream) {
	StatusBar *viewLine = nowStatus->firstStatusBar;

	stream->writeUint16BE(nowStatus->alignStatus);
	stream->writeSint16LE(nowStatus->litStatus);
	stream->writeUint16BE(nowStatus->statusX);
	stream->writeUint16BE(nowStatus->statusY);

	stream->writeByte(nowStatus->statusR);
	stream->writeByte(nowStatus->statusG);
	stream->writeByte(nowStatus->statusB);
	stream->writeByte(nowStatus->statusLR);
	stream->writeByte(nowStatus->statusLG);
	stream->writeByte(nowStatus->statusLB);

	// Write what's being said
	while (viewLine) {
		stream->writeByte(1);
		writeString(viewLine->text, stream);
		viewLine = viewLine->next;
	}
	stream->writeByte(0);
}

bool loadStatusBars(Common::SeekableReadStream *stream) {
	clearStatusBar();

	nowStatus->alignStatus = stream->readUint16BE();
	nowStatus->litStatus = stream->readSint16LE();
	nowStatus->statusX = stream->readUint16BE();
	nowStatus->statusY = stream->readUint16BE();

	nowStatus->statusR = stream->readByte();
	nowStatus->statusG = stream->readByte();
	nowStatus->statusB = stream->readByte();
	nowStatus->statusLR = stream->readByte();
	nowStatus->statusLG = stream->readByte();
	nowStatus->statusLB = stream->readByte();

	verbLinePalette.setColor((byte)nowStatus->statusR, (byte)nowStatus->statusG, (byte)nowStatus->statusB);
	litVerbLinePalette.setColor((byte)nowStatus->statusLR, (byte)nowStatus->statusLG, (byte)nowStatus->statusLB);
	// Read what's being said
	StatusBar **viewLine = & (nowStatus->firstStatusBar);
	StatusBar *newOne;
	while (stream->readByte()) {
		newOne = new StatusBar;
		if (! checkNew(newOne)) return false;
		newOne->text = readString(stream);
		newOne->next = NULL;
		(*viewLine) = newOne;
		viewLine = &(newOne->next);
	}
	return true;
}

} // End of namespace Sludge
