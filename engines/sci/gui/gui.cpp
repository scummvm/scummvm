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
#include "sci/gui/gui.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_cursor.h"
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_windowmgr.h"
#include "sci/gui/gui_animate.h"
#include "sci/gui/gui_controls.h"
#include "sci/gui/gui_text.h"
#include "sci/gui/gui_transitions.h"
#include "sci/gui/gui_view.h"

#include "sci/gfx/operations.h"

namespace Sci {

// for debug purposes
// class SciGui32 : public SciGui {
// public:
// 	SciGui32(EngineState *s, SciGuiScreen *screen, SciGuiPalette *palette, SciGuiCursor *cursor);
// 	~SciGui32();
// };

SciGui::SciGui(EngineState *state, SciGuiScreen *screen, SciGuiPalette *palette, SciGuiCursor *cursor)
	: _s(state), _screen(screen), _palette(palette), _cursor(cursor) {

	_gfx = new SciGuiGfx(_s, _screen, _palette);
	_transitions = new SciGuiTransitions(this, _screen, _palette, _s->resMan->isVGA());
	_animate = new SciGuiAnimate(_s, _gfx, _screen, _palette);
	_text = new SciGuiText(_s->resMan, _gfx, _screen);
	_windowMgr = new SciGuiWindowMgr(this, _screen, _gfx, _text);
	_controls = new SciGuiControls(_s->_segMan, _gfx, _text);
//  	_gui32 = new SciGui32(_s, _screen, _palette, _cursor); // for debug purposes
}

SciGui::SciGui() {
}

SciGui::~SciGui() {
}

void SciGui::resetEngineState(EngineState *s) {
	_s = s;
	_gfx->resetEngineState(s);
	_animate->resetEngineState(s);
}

void SciGui::init(bool usesOldGfxFunctions) {
	_usesOldGfxFunctions = usesOldGfxFunctions;

	_gfx->init(_text);
	_windowMgr->init(_s->_gameName);
	initPriorityBands();
}

void SciGui::initPriorityBands() {
	if (_usesOldGfxFunctions) {
		_gfx->PriorityBandsInit(15, 42, 200);
	} else {
		_gfx->PriorityBandsInit(14, 42, 190);
	}
}

void SciGui::wait(int16 ticks) {
	uint32 time;

	time = g_system->getMillis();
	_s->r_acc = make_reg(0, ((long)time - (long)_s->last_wait_time) * 60 / 1000);
	_s->last_wait_time = time;

	ticks *= g_debug_sleeptime_factor;
	gfxop_sleep(_s->gfx_state, ticks * 1000 / 60);
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

void SciGui::setPortPic(Common::Rect rect, int16 picTop, int16 picLeft, bool initPriorityBandsFlag) {
	_windowMgr->_picWind->rect = rect;
	_windowMgr->_picWind->top = picTop;
	_windowMgr->_picWind->left = picLeft;
	if (initPriorityBandsFlag)
		initPriorityBands();
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

int16 SciGui::coordinateToPriority(int16 y) {
	return _gfx->CoordinateToPriority(y);
}

int16 SciGui::priorityToCoordinate(int16 priority) {
	return _gfx->PriorityToCoordinate(priority);
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

#define SCI_DISPLAY_MOVEPEN				100
#define SCI_DISPLAY_SETALIGNMENT		101
#define SCI_DISPLAY_SETPENCOLOR			102
#define SCI_DISPLAY_SETBACKGROUNDCOLOR	103
#define SCI_DISPLAY_SETTEXTFACE			104
#define SCI_DISPLAY_SETFONT				105
#define SCI_DISPLAY_WIDTH				106
#define SCI_DISPLAY_SAVEUNDER			107
#define SCI_DISPLAY_RESTOREUNDER		108
#define SCI_DISPLAY_DONTSHOWBITS		121

void SciGui::display(const char *text, int argc, reg_t *argv) {
	int displayArg;
	GuiTextAlignment alignment = SCI_TEXT_ALIGNMENT_LEFT;
	int16 bgcolor = -1, width = -1, bRedraw = 1;
	bool doSaveUnder = false;
	Common::Rect rect;

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
		switch (displayArg) {
		case SCI_DISPLAY_MOVEPEN:
			_gfx->MoveTo(argv[0].toUint16(), argv[1].toUint16());
			argc -= 2; argv += 2;
			break;
		case SCI_DISPLAY_SETALIGNMENT:
			alignment = argv[0].toSint16();
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETPENCOLOR:
			_gfx->PenColor(argv[0].toUint16());
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETBACKGROUNDCOLOR:
			bgcolor = argv[0].toUint16();
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETTEXTFACE:
			_gfx->TextFace(argv[0].toUint16());
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETFONT:
			_text->SetFont(argv[0].toUint16());
			argc--; argv++;
			break;
		case SCI_DISPLAY_WIDTH:
			width = argv[0].toUint16();
			argc--; argv++;
			break;
		case SCI_DISPLAY_SAVEUNDER:
			doSaveUnder = true;
			break;
		case SCI_DISPLAY_RESTOREUNDER:
			_gfx->BitsGetRect(argv[0], &rect);
			rect.translate(-_gfx->GetPort()->left, -_gfx->GetPort()->top);
			_gfx->BitsRestore(argv[0]);
			graphRedrawBox(rect);
			// finishing loop
			argc = 0;
			break;
		case SCI_DISPLAY_DONTSHOWBITS:
			bRedraw = 0;
			break;
		default:
			warning("Unknown kDisplay argument %X", displayArg);
			break;
		}
	}

	// now drawing the text
	_text->Size(rect, text, -1, width);
	rect.moveTo(_gfx->GetPort()->curLeft, _gfx->GetPort()->curTop);
	_gfx->Move(rect.right <= _screen->_width ? 0 : _screen->_width - rect.right, rect.bottom <= _screen->_height ? 0 : _screen->_width - rect.bottom);
	rect.moveTo(_gfx->GetPort()->curLeft, _gfx->GetPort()->curTop);

	if (doSaveUnder)
		_s->r_acc = _gfx->BitsSave(rect, SCI_SCREEN_MASK_VISUAL);
	if (bgcolor != -1)
		_gfx->FillRect(rect, SCI_SCREEN_MASK_VISUAL, bgcolor, 0, 0);
	_text->Box(text, 0, rect, alignment, -1);
	if (_screen->_picNotValid == 0 && bRedraw)
		_gfx->BitsShow(rect);
	// restoring port and cursor pos
	GuiPort *currport = _gfx->GetPort();
	uint16 tTop = currport->curTop;
	uint16 tLeft = currport->curLeft;
	*currport = oldPort;
	currport->curTop = tTop;
	currport->curLeft = tLeft;
}

void SciGui::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, *textWidth, *textHeight);
	_text->Size(rect, text, font, maxWidth);
	*textWidth = rect.width();
	*textHeight = rect.height();
}

// Used SCI1+ for text codes
void SciGui::textFonts(int argc, reg_t *argv) {
	_text->CodeSetFonts(argc, argv);
}

// Used SCI1+ for text codes
void SciGui::textColors(int argc, reg_t *argv) {
	_text->CodeSetColors(argc, argv);
}

void SciGui::drawStatus(const char *text, int16 colorPen, int16 colorBack) {
	GuiPort *oldPort = _gfx->SetPort(_gfx->_menuPort);

	_gfx->FillRect(_gfx->_menuRect, 1, colorBack);
	_gfx->PenColor(colorPen);
	_gfx->MoveTo(0, 1);
	_text->Draw_String(text);
	_gfx->BitsShow(_gfx->_menuRect);
	_gfx->SetPort(oldPort);
}

void SciGui::drawMenuBar() {
	// TODO: Implement menubar
	warning("TODO: drawMenuBar()");
}

void SciGui::clearMenuBar() {
	// TODO: Implement menubar
	warning("TODO: clearMenuBar()");
}

void SciGui::drawPicture(GuiResourceId pictureId, int16 animationNr, bool animationBlackoutFlag, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);

	if (_windowMgr->isFrontWindow(_windowMgr->_picWind)) {
		_screen->_picNotValid = 1;
		_gfx->drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_transitions->setup(animationNr, animationBlackoutFlag);
	} else {
		_windowMgr->BeginUpdate(_windowMgr->_picWind);
		_gfx->drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_windowMgr->EndUpdate(_windowMgr->_picWind);
	}
	_gfx->SetPort(oldPort);
}

void SciGui::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo) {
	_gfx->drawCel(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo);
	_palette->setOnScreen();
}

void SciGui::drawControlButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite) {
	if (!hilite) {
		rect.grow(1);
		_gfx->EraseRect(rect);
		_gfx->FrameRect(rect);
		rect.grow(-2);
		_gfx->TextFace(style & 1 ? 0 : 1);
		_text->Box(text, 0, rect, SCI_TEXT_ALIGNMENT_CENTER, fontId);
		_gfx->TextFace(0);
		rect.grow(1);
		if (style & 8) // selected
			_gfx->FrameRect(rect);
		if (_screen->_picNotValid == 0) {
			rect.grow(1);
			_gfx->BitsShow(rect);
		}
	} else {
		_gfx->InvertRect(rect);
		_gfx->BitsShow(rect);
	}
}

void SciGui::drawControlText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, GuiTextAlignment alignment, int16 style, bool hilite) {
	if (!hilite) {
		rect.grow(1);
		_gfx->EraseRect(rect);
		rect.grow(-1);
		_text->Box(text, 0, rect, alignment, fontId);
		if (style & 8) { // selected
			_gfx->FrameRect(rect);
		}
		rect.grow(1);
		if (_screen->_picNotValid == 0)
			_gfx->BitsShow(rect);
	} else {
		_gfx->InvertRect(rect);
		_gfx->BitsShow(rect);
	}
}

void SciGui::drawControlTextEdit(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, int16 cursorPos, int16 maxChars, bool hilite) {
	Common::Rect textRect = rect;
	uint16 oldFontId = _text->GetFontId();

	rect.grow(1);
	_controls->TexteditCursorErase();
	_gfx->EraseRect(rect);
	_text->Box(text, 0, textRect, SCI_TEXT_ALIGNMENT_LEFT, fontId);
	_gfx->FrameRect(rect);
	if (style & 8) {
		_text->SetFont(fontId);
		rect.grow(-1);
		_controls->TexteditCursorDraw(rect, text, cursorPos);
		_text->SetFont(oldFontId);
		rect.grow(1);
	}
	if (_screen->_picNotValid == 0)
		_gfx->BitsShow(rect);
}

void SciGui::drawControlIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 style, bool hilite) {
	if (!hilite) {
		_gfx->drawCel(viewId, loopNo, celNo, rect.left, rect.top, 255, 0);
		if (style & 0x20) {
			_gfx->FrameRect(rect);
		}
		if (_screen->_picNotValid == 0)
			_gfx->BitsShow(rect);
	} else {
		_gfx->InvertRect(rect);
		_gfx->BitsShow(rect);
	}
}

void SciGui::drawControlList(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 style, int16 upperPos, int16 cursorPos, bool isAlias, bool hilite) {
	if (!hilite) {
		_controls->drawListControl(rect, obj, maxChars, count, entries, fontId, upperPos, cursorPos, isAlias);
		rect.grow(1);
		if (isAlias && (style & 8)) {
			_gfx->FrameRect(rect);
		}
		if (_screen->_picNotValid == 0)
			_gfx->BitsShow(rect);
	}
}

void SciGui::editControl(reg_t controlObject, reg_t eventObject) {
	int16 controlType = GET_SEL32V(_s->_segMan, controlObject, type);

	switch (controlType) {
	case SCI_CONTROLS_TYPE_TEXTEDIT:
		// Only process textedit controls in here
		_controls->TexteditChange(controlObject, eventObject);
		return;
	}
}

void SciGui::graphFillBoxForeground(Common::Rect rect) {
	_gfx->PaintRect(rect);
}

void SciGui::graphFillBoxBackground(Common::Rect rect) {
	_gfx->EraseRect(rect);
}

void SciGui::graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
	_gfx->FillRect(rect, colorMask, color, priority, control);
}

void SciGui::graphFrameBox(Common::Rect rect, int16 color) {
	int16 oldColor = _gfx->GetPort()->penClr;
	_gfx->PenColor(color);
	_gfx->FrameRect(rect);
	_gfx->PenColor(oldColor);
}

void SciGui::graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control) {
	_gfx->OffsetLine(startPoint, endPoint);
	_screen->drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control);
}

reg_t SciGui::graphSaveBox(Common::Rect rect, uint16 flags) {
	return _gfx->BitsSave(rect, flags);
}

void SciGui::graphRestoreBox(reg_t handle) {
	_gfx->BitsRestore(handle);
}

void SciGui::graphUpdateBox(Common::Rect rect) {
	_gfx->BitsShow(rect);
}

void SciGui::graphRedrawBox(Common::Rect rect) {
	localToGlobal(&rect.left, &rect.top);
	localToGlobal(&rect.right, &rect.bottom);
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	globalToLocal(&rect.left, &rect.top);
	globalToLocal(&rect.right, &rect.bottom);

	_animate->reAnimate(rect);

	_gfx->SetPort(oldPort);
}

int16 SciGui::picNotValid(int16 newPicNotValid) {
	int16 oldPicNotValid = _screen->_picNotValid;

	if (newPicNotValid != -1)
		_screen->_picNotValid = newPicNotValid;

	return oldPicNotValid;
}


void SciGui::paletteSet(GuiResourceId resourceId, uint16 flags) {
	// we are also called on EGA games as well, this doesnt make sense. doing this would actually break the system EGA palette
	if (!_s->resMan->isVGA())
		return;
   _palette->setFromResource(resourceId, flags);
}

void SciGui::paletteSetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	_palette->setFlag(fromColor, toColor, flag);
}

void SciGui::paletteUnsetFlag(uint16 fromColor, uint16 toColor, uint16 flag) {
	_palette->unsetFlag(fromColor, toColor, flag);
}

int16 SciGui::paletteFind(uint16 r, uint16 g, uint16 b) {
	return _palette->matchColor(&_palette->_sysPalette, r, g, b) & 0xFF;
}

void SciGui::paletteSetIntensity(uint16 fromColor, uint16 toColor, uint16 intensity, bool setPalette) {
	_palette->setIntensity(fromColor, toColor, intensity, setPalette);
}

void SciGui::paletteAnimate(uint16 fromColor, uint16 toColor, int16 speed) {
	// we are also called on Amiga as well, but for colors above 32, so it doesnt make sense
	if (!_s->resMan->isVGA())
		return;

	_palette->animate(fromColor, toColor, speed);
}

void SciGui::shakeScreen(uint16 shakeCount, uint16 directions) {
	while (shakeCount--) {
		if (directions & SCI_SHAKE_DIRECTION_VERTICAL)
			_screen->setVerticalShakePos(10);
		// TODO: horizontal shakes
		g_system->updateScreen();
		wait(3);
		if (directions & SCI_SHAKE_DIRECTION_VERTICAL)
			_screen->setVerticalShakePos(0);
		g_system->updateScreen();
		wait(3);
	}
}

uint16 SciGui::onControl(byte screenMask, Common::Rect rect) {
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	uint16 result;

	result = _gfx->onControl(screenMask, rect);
	_gfx->SetPort(oldPort);
	return result;
}

void SciGui::animateShowPic() {
	GuiPort *picPort = _windowMgr->_picWind;
	Common::Rect picRect = picPort->rect;

	_cursor->hide();
	// Adjust picRect to become relative to screen
	picRect.translate(picPort->left, picPort->top);
	_transitions->doit(picRect);
	_cursor->show();
}

void SciGui::animate(reg_t listReference, bool cycle, int argc, reg_t *argv) {
	byte old_picNotValid = _screen->_picNotValid;

	if (listReference.isNull()) {
		_animate->disposeLastCast();
		if (_screen->_picNotValid)
			animateShowPic();
		return;
	}

	List *list = _s->_segMan->lookupList(listReference);
	if (!list)
		error("kAnimate called with non-list as parameter");

	if (cycle) {
		if (!_animate->invoke(list, argc, argv))
			return;
	}

	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	_animate->disposeLastCast();

	_animate->makeSortedList(list);
	_animate->fill(old_picNotValid);

	if (old_picNotValid) {
		_windowMgr->BeginUpdate(_windowMgr->_picWind);
		_animate->update();
		_windowMgr->EndUpdate(_windowMgr->_picWind);
	}

	_animate->drawCels();

	if (_screen->_picNotValid)
		animateShowPic();

	_animate->updateScreen(old_picNotValid);
	_animate->restoreAndDelete(argc, argv);

	_gfx->SetPort(oldPort);
}

void SciGui::addToPicSetPicNotValid() {
	if (getSciVersion() <= SCI_VERSION_1_EARLY)
		_screen->_picNotValid = 1;
	else
		_screen->_picNotValid = 2;
}

void SciGui::addToPicList(reg_t listReference, int argc, reg_t *argv) {
	List *list;

	_gfx->SetPort((GuiPort *)_windowMgr->_picWind);

	list = _s->_segMan->lookupList(listReference);
	if (!list)
		error("kAddToPic called with non-list as parameter");

	_animate->makeSortedList(list);
	_animate->addToPicDrawCels();

	addToPicSetPicNotValid();
}

void SciGui::addToPicView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	_gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	_animate->addToPicDrawView(viewId, loopNo, celNo, leftPos, topPos, priority, control);
	addToPicSetPicNotValid();
}

void SciGui::setNowSeen(reg_t objectReference) {
	_gfx->SetNowSeen(objectReference);
}

bool SciGui::canBeHere(reg_t curObject, reg_t listReference) {
	SegManager *segMan = _s->_segMan;
	GuiPort *oldPort = _gfx->SetPort((GuiPort *)_windowMgr->_picWind);
	Common::Rect checkRect;
	uint16 signal, controlMask;
	bool result;

	checkRect.left = GET_SEL32V(segMan, curObject, brLeft);
	checkRect.top = GET_SEL32V(segMan, curObject, brTop);
	checkRect.right = GET_SEL32V(segMan, curObject, brRight);
	checkRect.bottom = GET_SEL32V(segMan, curObject, brBottom);
	signal = GET_SEL32V(segMan, curObject, signal);
	controlMask = GET_SEL32V(segMan, curObject, illegalBits);
	result = (_gfx->onControl(SCI_SCREEN_MASK_CONTROL, checkRect) & controlMask) ? false : true;
	if ((result) && (signal & (kSignalIgnoreActor | kSignalRemoveView)) == 0) {
		List *list = _s->_segMan->lookupList(listReference);
		if (!list)
			error("kCanBeHere called with non-list as parameter");

		result = _gfx->CanBeHereCheckRectList(curObject, checkRect, list);
	}
	_gfx->SetPort(oldPort);
	return result;
}

bool SciGui::isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position) {
	SciGuiView *tmpView = new SciGuiView(_s->resMan, NULL, NULL, viewId);
	sciViewCelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	position.x = CLIP<int>(position.x, 0, celInfo->width - 1);
	position.y = CLIP<int>(position.y, 0, celInfo->height - 1);
	byte *celData = tmpView->getBitmap(loopNo, celNo);
	bool result = (celData[position.y * celInfo->width + position.x] == celInfo->clearKey);
	delete tmpView;
	return result;
}

void SciGui::baseSetter(reg_t object) {
	if (lookup_selector(_s->_segMan, object, _s->_kernel->_selectorCache.brLeft, NULL, NULL) == kSelectorVariable) {
		int16 x = GET_SEL32V(_s->_segMan, object, x);
		int16 y = GET_SEL32V(_s->_segMan, object, y);
		int16 z = (_s->_kernel->_selectorCache.z > -1) ? GET_SEL32V(_s->_segMan, object, z) : 0;
		int16 yStep = GET_SEL32V(_s->_segMan, object, yStep);
		GuiResourceId viewId = GET_SEL32V(_s->_segMan, object, view);
		int16 loopNo = GET_SEL32V(_s->_segMan, object, loop);
		int16 celNo = GET_SEL32V(_s->_segMan, object, cel);

		SciGuiView *tmpView = new SciGuiView(_s->resMan, NULL, NULL, viewId);
		sciViewCelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
		int16 left = x + celInfo->displaceX - (celInfo->width >> 1);
		int16 right = left + celInfo->width;
		int16 bottom = y + celInfo->displaceY - z + 1;
		int16 top = bottom - yStep;

		debugC(2, kDebugLevelBaseSetter, "(%d,%d)+/-(%d,%d), (%d x %d) -> (%d, %d) to (%d, %d)\n",
				x, y, celInfo->displaceX, celInfo->displaceY, celInfo->width, celInfo->height, left, top, bottom, right);

		delete tmpView;

		PUT_SEL32V(_s->_segMan, object, brLeft, left);
		PUT_SEL32V(_s->_segMan, object, brRight, right);
		PUT_SEL32V(_s->_segMan, object, brTop, top);
		PUT_SEL32V(_s->_segMan, object, brBottom, bottom);
	}
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

void SciGui::setCursorView(GuiResourceId viewNum, int loopNum, int cellNum, Common::Point *hotspot) {
	_cursor->setView(viewNum, loopNum, cellNum, hotspot);
}

void SciGui::setCursorPos(Common::Point pos) {
	pos.y += _gfx->GetPort()->top;
	pos.x += _gfx->GetPort()->left;
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

int16 SciGui::getCelWidth(GuiResourceId viewId, int16 loopNo, int16 celNo) {
	SciGuiView *tmpView = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	sciViewCelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	int16 celWidth = celInfo->width;
	delete tmpView;

	return celWidth;
}

int16 SciGui::getCelHeight(GuiResourceId viewId, int16 loopNo, int16 celNo) {
	SciGuiView *tmpView = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	sciViewCelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	int16 celHeight = celInfo->height;
	delete tmpView;

	return celHeight;
}

int16 SciGui::getLoopCount(GuiResourceId viewId) {
	SciGuiView *tmpView = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	if (!tmpView)
		return -1;

	uint16 loopCount = tmpView->getLoopCount();
	delete tmpView;

	return loopCount;
}

int16 SciGui::getCelCount(GuiResourceId viewId, int16 loopNo) {
	SciGuiView *tmpView = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	if (!tmpView)
		return -1;

	uint16 celCount = tmpView->getLoopInfo(loopNo)->celCount;
	delete tmpView;

	return celCount;
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
