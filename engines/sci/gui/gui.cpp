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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_windowmgr.h"
#include "sci/gui/gui_memmgr.h"
#include "sci/gui/gui_view.h"

#include "sci/gfx/operations.h"

namespace Sci {

SciGUI::SciGUI(OSystem *system, EngineState *state)
	: _system(system), _s(state) {
	_screen = new SciGUIscreen(_system, _s);
	_gfx = new SciGUIgfx(_system, _s, _screen);
	_windowMgr = new SciGUIwindowMgr(_s, _gfx);
}

SciGUI::SciGUI() {
}

SciGUI::~SciGUI() {
}

void SciGUI::init(bool oldGfxFunctions) {
	_usesOldGfxFunctions = oldGfxFunctions;

	/* Set default SCI0 palette */
}

int16 SciGUI::getTimeTicks() {
	return _gfx->_sysTicks;
}

void SciGUI::wait(int16 ticks) {
	uint32 waitto = _gfx->_sysTicks + ticks;
	do {
		//eventMgr->pollEvents();
		_system->delayMillis(_gfx->_sysSpeed >> 11);
	} while (_gfx->_sysTicks < waitto);
}

void SciGUI::setPort(uint16 portPtr) {
	switch (portPtr) {
		case 0: _gfx->SetPort(_windowMgr->_wmgrPort); break;
		case 0xFFFF: _gfx->SetPort(_gfx->_menuPort); break;
		default:
			_gfx->SetPort((sciPort *)heap2Ptr(portPtr));
	};
}

void SciGUI::setPortPic(Common::Rect rect, int16 picTop, int16 picLeft) {
	_windowMgr->_picWind->rect = rect;
	_windowMgr->_picWind->top = picTop;
	_windowMgr->_picWind->left = picLeft;
	//if (argc >= 7)
		//InitPri(42,190);
}

reg_t SciGUI::getPort() {
	return make_reg(0, ptr2heap((byte *)_gfx->GetPort()));
}

void SciGUI::globalToLocal(int16 *x, int16 *y) {
	sciPort *curPort = _gfx->GetPort();
	*x = *x - curPort->left;
	*y = *y - curPort->top;
}

void SciGUI::localToGlobal(int16 *x, int16 *y) {
	sciPort *curPort = _gfx->GetPort();
	*x = *x + curPort->left;
	*y = *y + curPort->top;
}

reg_t SciGUI::newWindow(Common::Rect rect1, Common::Rect rect2, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title) {
	sciWnd *wnd = NULL;

	if (rect2.top != 0 && rect2.left != 0 && rect2.height() != 0 && rect2.width() != 0)
		wnd = _windowMgr->NewWindow(&rect1, &rect2, title, style, priority, 0);
	else
		wnd = _windowMgr->NewWindow(&rect1, NULL, title, style, priority, 0);
	wnd->penClr = colorPen;
	wnd->backClr = colorBack;
	_windowMgr->DrawWindow(wnd);
	return make_reg(0, ptr2heap((byte *)wnd));
}

void SciGUI::disposeWindow(uint16 windowPtr, int16 arg2) {
	sciWnd *wnd = (sciWnd *)heap2Ptr(windowPtr);
	_windowMgr->DisposeWindow(wnd, arg2);
}

void SciGUI::display(const char *text, int argc, reg_t *argv) {
	int displayArg;
	sciPort oldPort;
	int16 align = 0;
	int16 bgcolor = -1, width = -1, bRedraw = 1;
	byte bSaveUnder = false;
	Common::Rect rect, *orect = &((sciWnd *)_gfx->GetPort())->rect0;

	memcpy(&oldPort, _gfx->GetPort(), sizeof(sciPort));
	// setting defaults
	_gfx->PenMode(0);
	_gfx->PenColor(0);
	_gfx->TextFace(0);
	// processing codes in argv
	while (argc > 0) {
		displayArg = argv[0].toUint16();
		argc--; argv++;
		switch (displayArg - 100) {
		case 0:
			_gfx->MoveTo(argv[0].toUint16(), argv[1].toUint16());
			argc -= 2; argv += 2;
			break;// move pen
		case 1:
			align = argv[0].toUint16();
			argc--; argv++;
			break;// set alignment
		case 2:
			_gfx->PenColor(argv[0].toUint16());
			argc--; argv++;
			break;// set pen color
		case 3:
			bgcolor = argv[0].toUint16();
			argc--; argv++;
			break;
		case 4:
			_gfx->TextFace(argv[0].toUint16());
			argc--; argv++;
			break;// set text grayout flag
		case 5:
			_gfx->SetFont(argv[0].toUint16());
			argc--; argv++;
			break;// set font
		case 6:
			width = argv[0].toUint16();
			argc--; argv++;
			break;
		case 7:
			bSaveUnder = 1;
			break;
		case 8: // restore under
//			if (hunk2Ptr(*pArgs)) {
//				memcpy(&rect, hunk2Ptr(*pArgs), sizeof(Common::Rect));
//				// rect is now absolute. Have to move it to be port-relative
//				rect.translate(-_gfx->RGetPort()->left, -_gfx->RGetPort()->top);
//				_gfx->RestoreBits(*pArgs);
//				ReAnimate(&rect);
//			}
			// finishing loop
			argc = 0;
			break;
		case 0x15:
			bRedraw = 0;
			break;
		default:
			warning("Unknown kDisplay argument %X", displayArg);
			break;
		}
	}
	// now drawing the text
	_gfx->TextSize(rect, text, -1, width);
	_gfx->Move((orect->left <= 320 ? 0 : 320 - orect->left), (orect->top <= 200 ? 0 : 200 - orect->top)); // move port to (0,0)
	rect.moveTo(_gfx->GetPort()->curLeft, _gfx->GetPort()->curTop);
//	if (bSaveUnder)
//		_acc = _gfx->SaveBits(rect, 1);
	if (bgcolor != -1)
		_gfx->FillRect(rect, 1, bgcolor, 0, 0);
	_gfx->TextBox(text, 0, rect, align, -1);
//	if (_picNotValid == 0 && bRedraw)
//		_gfx->ShowBits(rect, 1);
	// restoring port and cursor pos
	sciPort *currport = _gfx->GetPort();
	uint16 tTop = currport->curTop;
	uint16 tLeft = currport->curLeft;
	memcpy(currport, &oldPort, sizeof(sciPort));
	currport->curTop = tTop;
	currport->curLeft = tLeft;

	_screen->UpdateWhole();
}

void SciGUI::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, *textWidth, *textHeight);
	_gfx->TextSize(rect, text, font, maxWidth);
	*textWidth = rect.width(); *textHeight = rect.height();
}

// Used SCI1+ for text codes
void SciGUI::textFonts(int argc, reg_t *argv) {
	_gfx->SetTextFonts(argc, argv);
}

// Used SCI1+ for text codes
void SciGUI::textColors(int argc, reg_t *argv) {
	_gfx->SetTextColors(argc, argv);
}

void SciGUI::drawPicture(sciResourceId pictureId, uint16 style, uint16 flags, int16 EGApaletteNo) {
	bool addToFlag = flags ? true : false;
	sciPort *oldPort = _gfx->SetPort((sciPort *)_windowMgr->_picWind);

	if (_windowMgr->isFrontWindow(_windowMgr->_picWind)) {
		_gfx->drawPicture(pictureId, style, addToFlag, EGApaletteNo);
	} else {
		_windowMgr->BeginUpdate(_windowMgr->_picWind);
		_gfx->drawPicture(pictureId, style, addToFlag, EGApaletteNo);
		_windowMgr->EndUpdate(_windowMgr->_picWind);
	}
	_screen->UpdateWhole();

	_gfx->SetPort(oldPort);
	_s->pic_not_valid = 1;
}

void SciGUI::drawCell(sciResourceId viewId, uint16 loopNo, uint16 cellNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo) {
	_gfx->drawCell(viewId, loopNo, cellNo, leftPos, topPos, priority, paletteNo);
	_gfx->SetCLUT(&_gfx->_sysPalette);
	_screen->UpdateWhole();
}

void SciGUI::drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool inverse) {
	rect.grow(1);
	_gfx->EraseRect(rect);
	_gfx->FrameRect(rect);
	rect.grow(-2);
	_gfx->TextFace(style & 1 ? 0 : 1);
	_gfx->TextBox(text, 0, rect, 1, fontId);
	_gfx->TextFace(0);
	if (style & 8) { // selected
		rect.grow(1);
		_gfx->FrameRect(rect);
	}
	_screen->UpdateWhole();
}

void SciGUI::drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool inverse) {
	rect.grow(1);
	_gfx->EraseRect(rect);
	rect.grow(-1);
	_gfx->TextBox(text, 0, rect, mode, fontId);
	if (style & 8) { // selected
		_gfx->FrameRect(rect);
	}
	_screen->UpdateWhole();
}

void SciGUI::graphFillBoxForeground(Common::Rect rect) {
	_gfx->PaintRect(rect);
	_screen->UpdateWhole();
}

void SciGUI::graphFillBoxBackground(Common::Rect rect) {
	_gfx->EraseRect(rect);
	_screen->UpdateWhole();
}

void SciGUI::graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
	_gfx->FillRect(rect, colorMask, color, priority, control);
	_screen->UpdateWhole();
}

void SciGUI::graphDrawLine(Common::Rect rect, int16 color, int16 priority, int16 control) {
	_gfx->Draw_Line(rect.left, rect.top, rect.right, rect.bottom, color, priority, control);
	_screen->UpdateWhole();
}

reg_t SciGUI::graphSaveBox(Common::Rect rect, uint16 flags) {
	return _gfx->SaveBits(rect, flags);
}

void SciGUI::graphRestoreBox(reg_t handle) {
	_gfx->RestoreBits(handle);
	_screen->UpdateWhole();
}

void SciGUI::paletteSet(int resourceNo, int flags) {
   _gfx->SetResPalette(resourceNo, flags);
}

int16 SciGUI::paletteFind(int r, int g, int b) {
	return _gfx->MatchColor(&_gfx->_sysPalette, r, g, b) & 0xFF;
}

void SciGUI::paletteAnimate(int fromColor, int toColor, int speed) {
	_gfx->animatePalette(fromColor, toColor, speed);
}

int16 SciGUI::onControl(byte screenMask, Common::Rect rect) {
	sciPort *oldPort = _gfx->SetPort((sciPort *)_windowMgr->_picWind);
	int16 result;

	result = _gfx->onControl(screenMask, rect);
	_gfx->SetPort(oldPort);
	return result;
}

void SciGUI::moveCursor(int16 x, int16 y) {
	Common::Point newPos;
	
	x += _windowMgr->_picWind->rect.left;
	y += _windowMgr->_picWind->rect.top;
	newPos.x = CLIP<int16>(x, _windowMgr->_picWind->rect.left, _windowMgr->_picWind->rect.right - 1);
	newPos.y = CLIP<int16>(y, _windowMgr->_picWind->rect.top, _windowMgr->_picWind->rect.bottom - 1);

	gfxop_set_pointer_position(_s->gfx_state, newPos);
}

} // End of namespace Sci
