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
#include "sci/gui/gui.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_windowmgr.h"
#include "sci/gui/gui_view.h"

#include "sci/gfx/operations.h"

namespace Sci {

SciGui::SciGui(OSystem *system, EngineState *state)
	: _system(system), _s(state) {
	_screen = new SciGuiScreen(_system, _s);
	_gfx = new SciGuiGfx(_system, _s, _screen);
	_windowMgr = new SciGuiWindowMgr(_s, _gfx);
}

SciGui::SciGui() {
}

SciGui::~SciGui() {
}

void SciGui::init(bool oldGfxFunctions) {
	_usesOldGfxFunctions = oldGfxFunctions;

	
}

int16 SciGui::getTimeTicks() {
	return _gfx->_sysTicks;
}

void SciGui::wait(int16 ticks) {
	uint32 waitto = _gfx->_sysTicks + ticks;
	do {
		//eventMgr->pollEvents();
		_system->delayMillis(_gfx->_sysSpeed >> 11);
	} while (_gfx->_sysTicks < waitto);
}

void SciGui::setPort(uint16 portPtr) {
	switch (portPtr) {
	case 0:
		_gfx->SetPort(_windowMgr->_wmgrPort);
		break;
	case 0xFFFF:
		_gfx->SetPort(_gfx->_menuPort);
		break;
	default:
		_gfx->SetPort(_windowMgr->getPortById(portPtr));
	};
}

void SciGui::setPortPic(Common::Rect rect, int16 picTop, int16 picLeft) {
	_windowMgr->_picWind->rect = rect;
	_windowMgr->_picWind->top = picTop;
	_windowMgr->_picWind->left = picLeft;
	//if (argc >= 7)
		//InitPri(42,190);
}

reg_t SciGui::getPort() {
	return make_reg(0, _gfx->GetPort()->id);
}

void SciGui::globalToLocal(int16 *x, int16 *y) {
	GuiPort *curPort = _gfx->GetPort();
	*x = *x - curPort->left;
	*y = *y - curPort->top;
}

void SciGui::localToGlobal(int16 *x, int16 *y) {
	GuiPort *curPort = _gfx->GetPort();
	*x = *x + curPort->left;
	*y = *y + curPort->top;
}

reg_t SciGui::newWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title) {
	GuiWindow *wnd = NULL;

	if (restoreRect.top != 0 && restoreRect.left != 0 && restoreRect.height() != 0 && restoreRect.width() != 0)
		wnd = _windowMgr->NewWindow(dims, &restoreRect, title, style, priority, false);
	else
		wnd = _windowMgr->NewWindow(dims, NULL, title, style, priority, false);
	wnd->penClr = colorPen;
	wnd->backClr = colorBack;
	_windowMgr->DrawWindow(wnd);

	return make_reg(0, wnd->id);
}

void SciGui::disposeWindow(uint16 windowPtr, int16 arg2) {
	GuiWindow *wnd = (GuiWindow *)_windowMgr->getPortById(windowPtr);
	_windowMgr->DisposeWindow(wnd, arg2);
}

void SciGui::display(const char *text, int argc, reg_t *argv) {
	int displayArg;
	int16 align = 0;
	int16 bgcolor = -1, width = -1, bRedraw = 1;
	byte bSaveUnder = false;
	Common::Rect rect, *orect = &((GuiWindow *)_gfx->GetPort())->dims;

	// Make a "backup" of the port settings
	GuiPort oldPort = *_gfx->GetPort();

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
	GuiPort *currport = _gfx->GetPort();
	uint16 tTop = currport->curTop;
	uint16 tLeft = currport->curLeft;
	*currport = oldPort;
	currport->curTop = tTop;
	currport->curLeft = tLeft;

	_screen->copyToScreen();
}

void SciGui::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, *textWidth, *textHeight);
	_gfx->TextSize(rect, text, font, maxWidth);
	*textWidth = rect.width(); *textHeight = rect.height();
}

// Used SCI1+ for text codes
void SciGui::textFonts(int argc, reg_t *argv) {
	_gfx->SetTextFonts(argc, argv);
}

// Used SCI1+ for text codes
void SciGui::textColors(int argc, reg_t *argv) {
	_gfx->SetTextColors(argc, argv);
}

void SciGui::drawStatus(const char *text, int16 colorPen, int16 colorBack) {
	GuiPort *oldPort = _gfx->SetPort(_gfx->_menuPort);
	if (text) {
		_gfx->FillRect(_gfx->_menuRect, 1, colorBack);
		_gfx->PenColor(colorPen);
		_gfx->MoveTo(0, 1);
		_gfx->Draw_String(text);
		_gfx->SetPort(oldPort);
		// _gfx->ShowBits(*_theMenuBar, 1);
		_screen->copyToScreen();
	}
}

void SciGui::drawPicture(GuiResourceId pictureId, uint16 style, uint16 flags, int16 EGApaletteNo) {
	bool addToFlag = flags ? true : false;
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);

	if (_windowMgr->isFrontWindow(_windowMgr->_picWind)) {
		_gfx->drawPicture(pictureId, style, addToFlag, EGApaletteNo);
	} else {
		_windowMgr->BeginUpdate(_windowMgr->_picWind);
		_gfx->drawPicture(pictureId, style, addToFlag, EGApaletteNo);
		_windowMgr->EndUpdate(_windowMgr->_picWind);
	}
	_screen->copyToScreen();

	_gfx->SetPort(oldPort);
	_gfx->_picNotValid = true;
}

void SciGui::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo) {
	_gfx->drawCel(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo);
	_gfx->setScreenPalette(&_gfx->_sysPalette);
	_screen->copyToScreen();
}

void SciGui::drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool inverse) {
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
	_screen->copyToScreen();
}

void SciGui::drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool inverse) {
	rect.grow(1);
	_gfx->EraseRect(rect);
	rect.grow(-1);
	_gfx->TextBox(text, 0, rect, mode, fontId);
	if (style & 8) { // selected
		_gfx->FrameRect(rect);
	}
	_screen->copyToScreen();
}

void SciGui::drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 style, bool inverse) {
	_gfx->drawCel(viewId, loopNo, celNo, rect.left, rect.top, 255, 0);
	if (style & 0x20) {
		_gfx->FrameRect(rect);
	}
	_screen->copyToScreen();
}

void SciGui::graphFillBoxForeground(Common::Rect rect) {
	_gfx->PaintRect(rect);
	_screen->copyToScreen();
}

void SciGui::graphFillBoxBackground(Common::Rect rect) {
	_gfx->EraseRect(rect);
	_screen->copyToScreen();
}

void SciGui::graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
	_gfx->FillRect(rect, colorMask, color, priority, control);
	_screen->copyToScreen();
}

void SciGui::graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control) {
	_gfx->Draw_Line(startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control);
	_screen->copyToScreen();
}

reg_t SciGui::graphSaveBox(Common::Rect rect, uint16 flags) {
	return _gfx->SaveBits(rect, flags);
}

void SciGui::graphRestoreBox(reg_t handle) {
	_gfx->RestoreBits(handle);
	_screen->copyToScreen();
}

void SciGui::paletteSet(int resourceNo, int flags) {
   _gfx->SetResPalette(resourceNo, flags);
}

int16 SciGui::paletteFind(int r, int g, int b) {
	return _gfx->MatchColor(&_gfx->_sysPalette, r, g, b) & 0xFF;
}

void SciGui::paletteSetIntensity(int fromColor, int toColor, int intensity, bool setPalette) {
	_gfx->PaletteSetIntensity(fromColor, toColor, intensity, &_gfx->_sysPalette);
	if (setPalette) {
		_gfx->setScreenPalette(&_gfx->_sysPalette);
	}
}

void SciGui::paletteAnimate(int fromColor, int toColor, int speed) {
	_gfx->PaletteAnimate(fromColor, toColor, speed);
}

int16 SciGui::onControl(byte screenMask, Common::Rect rect) {
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	int16 result;

	result = _gfx->onControl(screenMask, rect);
	_gfx->SetPort(oldPort);
	return result;
}

void SciGui::animate(reg_t listReference, bool cycle, int argc, reg_t *argv) {
	bool old_picNotValid = _gfx->_picNotValid;

	if (listReference.isNull()) {
		_gfx->AnimateDisposeLastCast();
		if (_gfx->_picNotValid) {
			//(this->*ShowPic)(_showMap, _showStyle);
			_gfx->_picNotValid = false;
		}
		return;
	}

	List *list = lookup_list(_s, listReference);
	if (!list) {
		error("kAnimate called with non-list as parameter");
	}

	if (cycle) {
		_gfx->AnimateInvoke(list, argc, argv);
	}

	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	_gfx->AnimateDisposeLastCast();

	_gfx->AnimateFill();

	_gfx->AnimateSort();
	if (old_picNotValid) {
		_gfx->AnimateUpdate();
	}

	_gfx->AnimateDrawCels();

	if (_gfx->_picNotValid) {
		//(this->*ShowPic)(_showMap, _showStyle);
		_gfx->_picNotValid = false;
	}

	//_gfx->AnimateUpdateScreen();
	_screen->copyToScreen();
	_gfx->AnimateRestoreAndDelete();

	_gfx->SetPort(oldPort);
}

void SciGui::addToPicList(reg_t listReference, int argc, reg_t *argv) {
	// FIXME: port over from gregs engine
}

void SciGui::addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	// FIXME: port over from gregs engine
}

void SciGui::setNowSeen(reg_t objectReference) {
	_gfx->SetNowSeen(objectReference);
}

void SciGui::moveCursor(int16 x, int16 y, int16 scaleFactor) {
	Common::Point newPos;
	
	x += _windowMgr->_picWind->rect.left;
	y += _windowMgr->_picWind->rect.top;
	newPos.x = CLIP<int16>(x, _windowMgr->_picWind->rect.left, _windowMgr->_picWind->rect.right - 1);
	newPos.y = CLIP<int16>(y, _windowMgr->_picWind->rect.top, _windowMgr->_picWind->rect.bottom - 1);

	if (x > _screen->_width || y > _screen->_height) {
		debug("[GFX] Attempt to place pointer at invalid coordinates (%d, %d)\n", x, y);
		return; // Not fatal
	}

	g_system->warpMouse(x * scaleFactor, y * scaleFactor);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	gfxop_get_event(_s->gfx_state, SCI_EVT_PEEK);
}

} // End of namespace Sci
