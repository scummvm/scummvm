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

#include "common/timer.h"
#include "common/util.h"

#include "sci/sci.h"
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_cursor.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_windowmgr.h"
#include "sci/gui/gui_view.h"

#include "sci/gfx/operations.h"

namespace Sci {

SciGui::SciGui(EngineState *state, SciGuiScreen *screen, SciGuiPalette *palette, SciGuiCursor *cursor)
	: _s(state), _screen(screen), _palette(palette), _cursor(cursor) {

	_gfx = new SciGuiGfx(_s, _screen, _palette);
	_windowMgr = new SciGuiWindowMgr(_s, _screen, _gfx);
}

SciGui::SciGui() {
}

SciGui::~SciGui() {
}

void SciGui::init(bool usesOldGfxFunctions) {
}


void SciGui::wait(int16 ticks) {
	uint32 time;

	time = g_system->getMillis();
	_s->r_acc = make_reg(0, ((long)time - (long)_s->last_wait_time) * 60 / 1000);
	_s->last_wait_time = time;

	ticks *= g_debug_sleeptime_factor;
	gfxop_sleep(_s->gfx_state, ticks * 1000 / 60);


	// Reset speed throttler: Game is playing along nicely anyway
	if (ticks > 0)
		_s->speedThrottler->reset();
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

	_gfx->FillRect(_gfx->_menuRect, 1, colorBack);
	_gfx->PenColor(colorPen);
	_gfx->MoveTo(0, 1);
	_gfx->Draw_String(text);
	_gfx->SetPort(oldPort);
	// _gfx->ShowBits(*_theMenuBar, 1);
	_screen->copyToScreen();
}

void SciGui::drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);

	if (_windowMgr->isFrontWindow(_windowMgr->_picWind)) {
		_gfx->drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
	} else {
		_windowMgr->BeginUpdate(_windowMgr->_picWind);
		_gfx->drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_windowMgr->EndUpdate(_windowMgr->_picWind);
	}
	_screen->copyToScreen();

	_gfx->SetPort(oldPort);
	_screen->_picNotValid = true;
}

void SciGui::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo) {
	_gfx->drawCel(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo);
	_palette->setOnScreen();
	_screen->copyToScreen();
}

void SciGui::drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite) {
	if (!hilite) {
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
	} else {
		_gfx->InvertRect(rect);
	}
	_screen->copyToScreen();
}

void SciGui::drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, bool hilite) {
	if (!hilite) {
		rect.grow(1);
		_gfx->EraseRect(rect);
		rect.grow(-1);
		_gfx->TextBox(text, 0, rect, mode, fontId);
		if (style & 8) { // selected
			_gfx->FrameRect(rect);
		}
	} else {
		_gfx->InvertRect(rect);
	}
	_screen->copyToScreen();
}

void SciGui::drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 style, bool hilite) {
	if (!hilite) {
		_gfx->drawCel(viewId, loopNo, celNo, rect.left, rect.top, 255, 0);
		if (style & 0x20) {
			_gfx->FrameRect(rect);
		}
	} else {
		_gfx->InvertRect(rect);
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
	_gfx->drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control);
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
   _palette->setFromResource(resourceNo, flags);
}

int16 SciGui::paletteFind(int r, int g, int b) {
	return _palette->matchColor(&_palette->_sysPalette, r, g, b) & 0xFF;
}

void SciGui::paletteSetIntensity(int fromColor, int toColor, int intensity, bool setPalette) {
	_palette->setIntensity(fromColor, toColor, intensity, &_palette->_sysPalette);
	if (setPalette) {
		_palette->setOnScreen();
	}
}

void SciGui::paletteAnimate(int fromColor, int toColor, int speed) {
	_palette->animate(fromColor, toColor, speed);
}

int16 SciGui::onControl(byte screenMask, Common::Rect rect) {
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	int16 result;

	result = _gfx->onControl(screenMask, rect);
	_gfx->SetPort(oldPort);
	return result;
}

void SciGui::animate(reg_t listReference, bool cycle, int argc, reg_t *argv) {
	byte old_picNotValid = _screen->_picNotValid;

	if (listReference.isNull()) {
		_gfx->AnimateDisposeLastCast();
		if (_screen->_picNotValid) {
			//(this->*ShowPic)(_showMap, _showStyle);
			_screen->_picNotValid = false;
		}
		return;
	}

	List *list = _s->_segMan->lookupList(listReference);
	if (!list)
		error("kAnimate called with non-list as parameter");

	if (cycle)
		_gfx->AnimateInvoke(list, argc, argv);

	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	_gfx->AnimateDisposeLastCast();

	_gfx->AnimateFill(list, old_picNotValid);

	// _gfx->AnimateSort();
	if (old_picNotValid) {
		_windowMgr->BeginUpdate(_windowMgr->_picWind);
		_gfx->AnimateUpdate(list);
		_windowMgr->EndUpdate(_windowMgr->_picWind);
	}

	_gfx->AnimateDrawCels(list);

	if (_screen->_picNotValid) {
		//(this->*ShowPic)(_showMap, _showStyle);
		_screen->_picNotValid = false;
	}

	//_gfx->AnimateUpdateScreen();
	_screen->copyToScreen();
	_gfx->AnimateRestoreAndDelete(list, argc, argv);

	_gfx->SetPort(oldPort);
}

void SciGui::addToPicList(reg_t listReference, int argc, reg_t *argv) {
	List *list;
	Common::List<GuiAnimateList> *sortedList;

	_gfx->SetPort((GuiPort *)_windowMgr->_picWind);

	list = _s->_segMan->lookupList(listReference);
	if (!list)
		error("kAddToPic called with non-list as parameter");

//	sortedList = _gfx->AnimateMakeSortedList(list);

//	uint16 szList = list.getSize();
//	HEAPHANDLE*arrObj = new HEAPHANDLE[szList];
//	int16*arrY = new int16[szList];
//	HEAPHANDLE hnode = list.getFirst();
//	sciNode1*pnode;
//
//	for (int i = 0; i < szList; i++) {
//		pnode = (sciNode1*)heap2Ptr(hnode);
//		obj.set(pnode->value);
//		arrY[i] = obj.getProperty(3);
//arrZ[i] = obj.getProperty(
//		arrObj[i] = pnode->value;
//		hnode = pnode->next;
//	}
//	animSort(arrObj, arrY, szList);

	_gfx->AddToPicDrawCels(list);

	_screen->_picNotValid = 2;
//	delete sortedList;
}

void SciGui::addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	// FIXME: port over from gregs engine
}

void SciGui::setNowSeen(reg_t objectReference) {
	_gfx->SetNowSeen(objectReference);
}

void SciGui::hideCursor() {
	_cursor->hide();
}

void SciGui::showCursor() {
	_cursor->show();
}

void SciGui::setCursorShape(GuiResourceId cursorId) {
	_cursor->setShape(cursorId);
}

void SciGui::setCursorPos(Common::Point pos) {
	// FIXME: try to find out if we need to adjust position somehow, currently just forwarding to moveCursor()
	moveCursor(pos);
}

void SciGui::moveCursor(Common::Point pos) {
	pos.y += _windowMgr->_picWind->rect.top;
	pos.x += _windowMgr->_picWind->rect.left;
	
	pos.y = CLIP<int16>(pos.y, _windowMgr->_picWind->rect.top, _windowMgr->_picWind->rect.bottom - 1);
	pos.x = CLIP<int16>(pos.x, _windowMgr->_picWind->rect.left, _windowMgr->_picWind->rect.right - 1);

	if (pos.x > _screen->_width || pos.y > _screen->_height) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return; // Not fatal
	}

	_cursor->setPosition(pos);
	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	//gfxop_get_event(_s->gfx_state, SCI_EVT_PEEK);
	// FIXME!
}

bool SciGui::debugUndither(bool flag) {
	_screen->unditherSetState(flag);
	return false;
}

bool SciGui::debugShowMap(int mapNo) {
	_screen->debugShowMap(mapNo);
	return false;
}

} // End of namespace Sci
