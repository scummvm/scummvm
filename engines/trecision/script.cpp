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

#include "trecision/graphics.h"
#include "trecision/nl/define.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/message.h"
#include "trecision/trecision.h"

namespace Trecision {

void SScriptFrame::sendFrame() {
	doEvent(_class, _event, MP_DEFAULT, _u16Param1, _u16Param2, _u8Param, _u32Param);
}

void TrecisionEngine::endScript() {
	_curStack--;
	if (_curStack == 0) {
		_flagscriptactive = false;
		showCursor();
		redrawString();
	}
}

void TrecisionEngine::playScript(uint16 id) {
	_curStack++;
	_flagscriptactive = true;
	hideCursor();
	_curScriptFrame[_curStack] = _script[id]._firstFrame;

	SScriptFrame *curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
	// If the event is empty, terminate the script
	if (curFrame->isEmptyEvent()) {
		endScript();
		return;
	}

	bool loop = true;
	while (loop) {
		loop = false;
		curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
		SScriptFrame *nextFrame = &_scriptFrame[_curScriptFrame[_curStack] + 1];
		curFrame->sendFrame();
		if (curFrame->_noWait && !nextFrame->isEmptyEvent()) {
			_curScriptFrame[_curStack]++;
			loop = true;
		}
	}
}

void TrecisionEngine::evalScript() {
	if (_characterQueue.testEmptyCharacterQueue4Script() && _gameQueue.testEmptyQueue(MC_DIALOG) && _flagScreenRefreshed) {
		_curScriptFrame[_curStack]++;
		hideCursor();

		SScriptFrame *curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
		if (curFrame->isEmptyEvent()) {
			endScript();
			return;
		}

		bool loop = true;
		while (loop) {
			loop = false;
			curFrame = &_scriptFrame[_curScriptFrame[_curStack]];
			SScriptFrame *nextFrame = &_scriptFrame[_curScriptFrame[_curStack] + 1];
			curFrame->sendFrame();
			if (curFrame->_noWait && !nextFrame->isEmptyEvent()) {
				_curScriptFrame[_curStack]++;
				loop = true;
			}
		}
	}
}

bool TrecisionEngine::quitGame() {
	for (int a = 0; a < TOP; a++)
		memcpy(_zBuffer + a * MAXX, _screenBuffer + MAXX * a, MAXX * 2);

	for (int a = 0; a < TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	SDText SText;
	SText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageConfirmExit]
	);
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	freeKey();

	checkSystem();

	char ch = waitKey();

	bool exitFl = ((ch == 'y') || (ch == 'Y'));

	for (int a = 0; a < TOP; a++)
		memcpy(_screenBuffer + MAXX * a, _zBuffer + a * MAXX, MAXX * 2);

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	return exitFl;
}

void TrecisionEngine::demoOver() {
	for (int a = 0; a < TOP; a++)
		memset(_screenBuffer + MAXX * a, 0, MAXX * 2);

	SDText SText;
	SText.set(
		Common::Rect(0, TOP - 20, MAXX, CARHEI + (TOP - 20)),
		Common::Rect(0, 0, MAXX, CARHEI),
		MOUSECOL,
		MASKCOL,
		_sysText[kMessageDemoOver]
	);	
	SText.DText();

	_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	freeKey();
	waitKey();
	quitGame();
}

} // End of namespace Trecision
