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
#include "sludge/colours.h"
#include "sludge/sprites.h"
#include "sludge/fonttext.h"
#include "sludge/moreio.h"
#include "sludge/stringy.h"
#include "sludge/newfatal.h"
#include "sludge/statusba.h"

namespace Sludge {

spritePalette verbLinePalette;
spritePalette litVerbLinePalette;

statusStuff mainStatus;
statusStuff *nowStatus = & mainStatus;
extern int fontHeight;
extern float cameraZoom;

void setLitStatus(int i) {
	nowStatus->litStatus = i;
}

void killLastStatus() {
	if (nowStatus->firstStatusBar) {
		statusBar *kill = nowStatus->firstStatusBar;
		nowStatus->firstStatusBar = kill->next;
		delete kill->text;
		delete kill;
	}
}

void clearStatusBar() {
	statusBar *stat = nowStatus->firstStatusBar;
	statusBar *kill;
	nowStatus->litStatus = -1;
	while (stat) {
		kill = stat;
		stat = stat->next;
		delete kill->text;
		delete kill;
	}
	nowStatus->firstStatusBar = NULL;
}

void addStatusBar() {
	statusBar *newStat = new statusBar;
	if (checkNew(newStat)) {
		newStat->next = nowStatus->firstStatusBar;
		newStat->text = copyString("");
		nowStatus->firstStatusBar = newStat;
	}
}

void setStatusBar(char *txt) {
	if (nowStatus->firstStatusBar) {
		delete nowStatus->firstStatusBar->text;
		nowStatus->firstStatusBar->text = copyString(txt);
	}
}

void positionStatus(int x, int y) {
	nowStatus->statusX = x;
	nowStatus->statusY = y;
}

void drawStatusBar() {
	int y = nowStatus->statusY, n = 0;
	statusBar *stat = nowStatus->firstStatusBar;
	fixFont(litVerbLinePalette);
	fixFont(verbLinePalette);
	while (stat) {
		switch (nowStatus->alignStatus) {
		case IN_THE_CENTRE:
			pasteString(stat->text, ((winWidth - stringWidth(stat->text)) >> 1) / cameraZoom, y / cameraZoom, (n ++ == nowStatus->litStatus) ? litVerbLinePalette : verbLinePalette);
			break;

		case 1001:
			pasteString(stat->text, (winWidth - stringWidth(stat->text)) - nowStatus->statusX / cameraZoom, y / cameraZoom, (n ++ == nowStatus->litStatus) ? litVerbLinePalette : verbLinePalette);
			break;

		default:
			pasteString(stat->text, nowStatus->statusX / cameraZoom, y / cameraZoom, (n ++ == nowStatus->litStatus) ? litVerbLinePalette : verbLinePalette);
		}
		stat = stat->next;
		y -= fontHeight;
	}
}

void statusBarColour(byte r, byte g, byte b) {
	setFontColour(verbLinePalette, r, g, b);
	nowStatus->statusR = r;
	nowStatus->statusG = g;
	nowStatus->statusB = b;
}

void statusBarLitColour(byte r, byte g, byte b) {
	setFontColour(litVerbLinePalette, r, g, b);
	nowStatus->statusLR = r;
	nowStatus->statusLG = g;
	nowStatus->statusLB = b;
}

statusStuff *copyStatusBarStuff(statusStuff *here) {

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

	statusStuff *old = nowStatus;
	nowStatus = here;

	return old;
}

void restoreBarStuff(statusStuff *here) {
	delete nowStatus;
	setFontColour(verbLinePalette, here->statusR, here->statusG, here->statusB);
	setFontColour(litVerbLinePalette, here->statusLR, here->statusLG, here->statusLB);
	nowStatus = here;
}


void initStatusBar() {
	mainStatus.firstStatusBar = NULL;
	mainStatus.alignStatus = IN_THE_CENTRE;
	mainStatus.litStatus = -1;
	mainStatus.statusX = 10;
	mainStatus.statusY = winHeight - 15;
	statusBarColour(255, 255, 255);
	statusBarLitColour(255, 255, 128);
}

const char *statusBarText() {
	if (nowStatus->firstStatusBar) {
		return nowStatus->firstStatusBar->text;
	} else {
		return "";
	}
}

void saveStatusBars(Common::WriteStream *stream) {
	statusBar *viewLine = nowStatus->firstStatusBar;

	stream->writeUint16BE(nowStatus->alignStatus);
	putSigned(nowStatus->litStatus, stream);
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
	nowStatus->litStatus = getSigned(stream);
	nowStatus->statusX = stream->readUint16BE();
	nowStatus->statusY = stream->readUint16BE();

	nowStatus->statusR = stream->readByte();
	nowStatus->statusG = stream->readByte();
	nowStatus->statusB = stream->readByte();
	nowStatus->statusLR = stream->readByte();
	nowStatus->statusLG = stream->readByte();
	nowStatus->statusLB = stream->readByte();

	setFontColour(verbLinePalette, nowStatus->statusR, nowStatus->statusG, nowStatus->statusB);
	setFontColour(litVerbLinePalette, nowStatus->statusLR, nowStatus->statusLG, nowStatus->statusLB);
	// Read what's being said
	statusBar * * viewLine = & (nowStatus->firstStatusBar);
	statusBar *newOne;
	while (stream->readByte()) {
		newOne = new statusBar;
		if (! checkNew(newOne)) return false;
		newOne->text = readString(stream);
		newOne->next = NULL;
		(*viewLine) = newOne;
		viewLine = &(newOne->next);
	}
	return true;
}

} // End of namespace Sludge
