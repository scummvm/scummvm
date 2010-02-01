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
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/gui.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/controls.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/portrait.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/transitions.h"
#include "sci/graphics/view.h"
#include "sci/sound/audio.h"

namespace Sci {

SciGui::SciGui(EngineState *state, GfxScreen *screen, GfxPalette *palette, GfxCache *cache, Cursor *cursor, GfxPorts *ports, AudioPlayer *audio)
	: _s(state), _screen(screen), _palette(palette), _cache(cache), _cursor(cursor), _ports(ports), _audio(audio) {

	_compare = new GfxCompare(_s->_segMan, _s->_kernel, _cache, _screen);
	_paint16 = new GfxPaint16(_s->resMan, _s->_segMan, _s->_kernel, _cache, _ports, _screen, _palette);
	_transitions = new Transitions(this, _screen, _palette, _s->resMan->isVGA());
	_animate = new GfxAnimate(_s, _cache, _ports, _paint16, _screen, _palette, _cursor, _transitions);
	_s->_gfxAnimate = _animate;
	_text16 = new GfxText16(_s->resMan, _cache, _ports, _paint16, _screen);
	_controls = new GfxControls(_s->_segMan, _ports, _paint16, _text16, _screen);
	_s->_gfxControls = _controls;
	_menu = new GfxMenu(_s->_event, _s->_segMan, this, _ports, _paint16, _text16, _screen, _cursor);
	_s->_gfxMenu = _menu;
}

SciGui::~SciGui() {
	delete _menu;
	delete _controls;
	delete _text16;
	delete _animate;
	delete _transitions;
	delete _paint16;
	delete _ports;
	delete _compare;
	delete _cache;
}

void SciGui::resetEngineState(EngineState *s) {
	_s = s;
	_animate->resetEngineState(s);
}

void SciGui::init(bool usesOldGfxFunctions) {
	_usesOldGfxFunctions = usesOldGfxFunctions;

	_ports->init(this, _paint16, _text16, _s->_gameId);
	_paint16->init(_text16);
	initPriorityBands();
}

void SciGui::initPriorityBands() {
	if (_usesOldGfxFunctions) {
		_ports->priorityBandsInit(15, 42, 200);
	} else {
		if (getSciVersion() >= SCI_VERSION_1_1)
			_ports->priorityBandsInit(14, 0, 190);
		else
			_ports->priorityBandsInit(14, 42, 190);
	}
}

void SciGui::wait(int16 ticks) {
	uint32 time;

	time = g_system->getMillis();
	_s->r_acc = make_reg(0, ((long)time - (long)_s->last_wait_time) * 60 / 1000);
	_s->last_wait_time = time;

	ticks *= g_debug_sleeptime_factor;
	kernel_sleep(_s->_event, ticks * 1000 / 60);
}

void SciGui::globalToLocal(int16 *x, int16 *y) {
	Port *curPort = _ports->getPort();
	*x = *x - curPort->left;
	*y = *y - curPort->top;
}

void SciGui::localToGlobal(int16 *x, int16 *y) {
	Port *curPort = _ports->getPort();
	*x = *x + curPort->left;
	*y = *y + curPort->top;
}

int16 SciGui::coordinateToPriority(int16 y) {
	return _ports->coordinateToPriority(y);
}

int16 SciGui::priorityToCoordinate(int16 priority) {
	return _ports->priorityToCoordinate(priority);
}

#define SCI_DISPLAY_MOVEPEN				100
#define SCI_DISPLAY_SETALIGNMENT		101
#define SCI_DISPLAY_SETPENCOLOR			102
#define SCI_DISPLAY_SETBACKGROUNDCOLOR	103
#define SCI_DISPLAY_SETGREYEDOUTPUT		104
#define SCI_DISPLAY_SETFONT				105
#define SCI_DISPLAY_WIDTH				106
#define SCI_DISPLAY_SAVEUNDER			107
#define SCI_DISPLAY_RESTOREUNDER		108
#define SCI_DISPLAY_DONTSHOWBITS		121

void SciGui::display(const char *text, int argc, reg_t *argv) {
	int displayArg;
	TextAlignment alignment = SCI_TEXT16_ALIGNMENT_LEFT;
	int16 colorPen = -1, colorBack = -1, width = -1, bRedraw = 1;
	bool doSaveUnder = false;
	Common::Rect rect;

	// Make a "backup" of the port settings
	Port oldPort = *_ports->getPort();

	// setting defaults
	_ports->penMode(0);
	_ports->penColor(0);
	_ports->textGreyedOutput(false);
	// processing codes in argv
	while (argc > 0) {
		displayArg = argv[0].toUint16();
		argc--; argv++;
		switch (displayArg) {
		case SCI_DISPLAY_MOVEPEN:
			_ports->moveTo(argv[0].toUint16(), argv[1].toUint16());
			argc -= 2; argv += 2;
			break;
		case SCI_DISPLAY_SETALIGNMENT:
			alignment = argv[0].toSint16();
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETPENCOLOR:
			colorPen = argv[0].toUint16();
			_ports->penColor(colorPen);
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETBACKGROUNDCOLOR:
			colorBack = argv[0].toUint16();
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETGREYEDOUTPUT:
			_ports->textGreyedOutput(argv[0].isNull() ? false : true);
			argc--; argv++;
			break;
		case SCI_DISPLAY_SETFONT:
			_text16->SetFont(argv[0].toUint16());
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
			_paint16->bitsGetRect(argv[0], &rect);
			rect.translate(-_ports->getPort()->left, -_ports->getPort()->top);
			_paint16->bitsRestore(argv[0]);
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
	_text16->Size(rect, text, -1, width);
	rect.moveTo(_ports->getPort()->curLeft, _ports->getPort()->curTop);
	if (getSciVersion() >= SCI_VERSION_1_LATE) {
		int16 leftPos = rect.right <= _screen->getWidth() ? 0 : _screen->getWidth() - rect.right;
		int16 topPos = rect.bottom <= _screen->getHeight() ? 0 : _screen->getHeight() - rect.bottom;
		_ports->move(leftPos, topPos);
		rect.moveTo(_ports->getPort()->curLeft, _ports->getPort()->curTop);
	}

	if (doSaveUnder)
		_s->r_acc = _paint16->bitsSave(rect, SCI_SCREEN_MASK_VISUAL);
	if (colorBack != -1)
		_paint16->fillRect(rect, SCI_SCREEN_MASK_VISUAL, colorBack, 0, 0);
	_text16->Box(text, 0, rect, alignment, -1);
	if (_screen->_picNotValid == 0 && bRedraw)
		_paint16->bitsShow(rect);
	// restoring port and cursor pos
	Port *currport = _ports->getPort();
	uint16 tTop = currport->curTop;
	uint16 tLeft = currport->curLeft;
	*currport = oldPort;
	currport->curTop = tTop;
	currport->curLeft = tLeft;
}

void SciGui::textSize(const char *text, int16 font, int16 maxWidth, int16 *textWidth, int16 *textHeight) {
	Common::Rect rect(0, 0, *textWidth, *textHeight);
	_text16->Size(rect, text, font, maxWidth);
	*textWidth = rect.width();
	*textHeight = rect.height();
}

// Used SCI1+ for text codes
void SciGui::textFonts(int argc, reg_t *argv) {
	_text16->CodeSetFonts(argc, argv);
}

// Used SCI1+ for text codes
void SciGui::textColors(int argc, reg_t *argv) {
	_text16->CodeSetColors(argc, argv);
}

void SciGui::drawStatus(const char *text, int16 colorPen, int16 colorBack) {
	Port *oldPort = _ports->setPort(_ports->_menuPort);

	_paint16->fillRect(_ports->_menuBarRect, 1, colorBack);
	_ports->penColor(colorPen);
	_ports->moveTo(0, 1);
	_text16->Draw_String(text);
	_paint16->bitsShow(_ports->_menuBarRect);
	_ports->setPort(oldPort);
}

void SciGui::drawMenuBar(bool clear) {
	if (!clear) {
		Port *oldPort = _ports->setPort(_ports->_menuPort);
		_menu->drawBar();
		_paint16->bitsShow(_ports->_menuBarRect);
		_ports->setPort(oldPort);
	} else {
		drawStatus("", 0, 0);
	}
}

void SciGui::drawPicture(GuiResourceId pictureId, int16 animationNr, bool animationBlackoutFlag, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);

	if (_ports->isFrontWindow(_ports->_picWind)) {
		_screen->_picNotValid = 1;
		_paint16->drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_transitions->setup(animationNr, animationBlackoutFlag);
	} else {
		_ports->beginUpdate(_ports->_picWind);
		_paint16->drawPicture(pictureId, animationNr, mirroredFlag, addToFlag, EGApaletteNo);
		_ports->endUpdate(_ports->_picWind);
	}
	_ports->setPort(oldPort);
}

void SciGui::drawCel(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, int16 priority, uint16 paletteNo, bool hiresMode, reg_t upscaledHiresHandle) {
	// some calls are hiresMode even under kq6 DOS, that's why we check for upscaled hires here
	if ((!hiresMode) || (!_screen->getUpscaledHires())) {
		_paint16->drawCelAndShow(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo);
	} else {
		_paint16->drawHiresCelAndShow(viewId, loopNo, celNo, leftPos, topPos, priority, paletteNo, upscaledHiresHandle);
	}
	_palette->setOnScreen();
}

void SciGui::graphFillBoxForeground(Common::Rect rect) {
	_paint16->paintRect(rect);
}

void SciGui::graphFillBoxBackground(Common::Rect rect) {
	_paint16->eraseRect(rect);
}

void SciGui::graphFillBox(Common::Rect rect, uint16 colorMask, int16 color, int16 priority, int16 control) {
	_paint16->fillRect(rect, colorMask, color, priority, control);
}

void SciGui::graphFrameBox(Common::Rect rect, int16 color) {
	int16 oldColor = _ports->getPort()->penClr;
	_ports->penColor(color);
	_paint16->frameRect(rect);
	_ports->penColor(oldColor);
}

void SciGui::graphDrawLine(Common::Point startPoint, Common::Point endPoint, int16 color, int16 priority, int16 control) {
	_ports->offsetLine(startPoint, endPoint);
	_screen->drawLine(startPoint.x, startPoint.y, endPoint.x, endPoint.y, color, priority, control);
}

reg_t SciGui::graphSaveBox(Common::Rect rect, uint16 screenMask) {
	return _paint16->bitsSave(rect, screenMask);
}

reg_t SciGui::graphSaveUpscaledHiresBox(Common::Rect rect) {
	return _paint16->bitsSave(rect, SCI_SCREEN_MASK_DISPLAY);
}

void SciGui::graphRestoreBox(reg_t handle) {
	_paint16->bitsRestore(handle);
}

void SciGui::graphUpdateBox(Common::Rect rect, bool hiresMode) {
	// some calls are hiresMode even under kq6 DOS, that's why we check for upscaled hires here
	if ((!hiresMode) || (!_screen->getUpscaledHires()))
		_paint16->bitsShow(rect);
	else
		_paint16->bitsShowHires(rect);
}

void SciGui::graphRedrawBox(Common::Rect rect) {
	localToGlobal(&rect.left, &rect.top);
	localToGlobal(&rect.right, &rect.bottom);
	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);
	globalToLocal(&rect.left, &rect.top);
	globalToLocal(&rect.right, &rect.bottom);

	_animate->reAnimate(rect);

	_ports->setPort(oldPort);
}

void SciGui::graphAdjustPriority(int top, int bottom) {
	if (_usesOldGfxFunctions) {
		_ports->priorityBandsInit(15, top, bottom);
	} else {
		_ports->priorityBandsInit(14, top, bottom);
	}
}

int16 SciGui::picNotValid(int16 newPicNotValid) {
	int16 oldPicNotValid;

	if (getSciVersion() >= SCI_VERSION_1_1) {
		oldPicNotValid = _screen->_picNotValidSci11;

		if (newPicNotValid != -1)
			_screen->_picNotValidSci11 = newPicNotValid;
	} else {
		oldPicNotValid = _screen->_picNotValid;

		if (newPicNotValid != -1)
			_screen->_picNotValid = newPicNotValid;
	}

	return oldPicNotValid;
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
	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);
	Common::Rect adjustedRect(rect.left, rect.top, rect.right, rect.bottom);
	uint16 result;

	adjustedRect.clip(_ports->getPort()->rect);
	_ports->offsetRect(adjustedRect);
	result = _compare->onControl(screenMask, adjustedRect);

	_ports->setPort(oldPort);
	return result;
}

void SciGui::setNowSeen(reg_t objectReference) {
	_compare->SetNowSeen(objectReference);
}

bool SciGui::canBeHere(reg_t curObject, reg_t listReference) {
	Port *oldPort = _ports->setPort((Port *)_ports->_picWind);
	Common::Rect checkRect;
	Common::Rect adjustedRect;
	uint16 signal, controlMask;
	bool result;

	checkRect.left = GET_SEL32V(_s->_segMan, curObject, brLeft);
	checkRect.top = GET_SEL32V(_s->_segMan, curObject, brTop);
	checkRect.right = GET_SEL32V(_s->_segMan, curObject, brRight);
	checkRect.bottom = GET_SEL32V(_s->_segMan, curObject, brBottom);

	adjustedRect = checkRect;
	adjustedRect.clip(_ports->getPort()->rect);
	_ports->offsetRect(adjustedRect);

	signal = GET_SEL32V(_s->_segMan, curObject, signal);
	controlMask = GET_SEL32V(_s->_segMan, curObject, illegalBits);
	result = (_compare->onControl(SCI_SCREEN_MASK_CONTROL, adjustedRect) & controlMask) ? false : true;
	if ((result) && (signal & (kSignalIgnoreActor | kSignalRemoveView)) == 0) {
		List *list = _s->_segMan->lookupList(listReference);
		if (!list)
			error("kCanBeHere called with non-list as parameter");

		result = _compare->CanBeHereCheckRectList(curObject, checkRect, list);
	}
	_ports->setPort(oldPort);
	return result;
}

bool SciGui::isItSkip(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Point position) {
	View *tmpView = _cache->getView(viewId);
	CelInfo *celInfo = tmpView->getCelInfo(loopNo, celNo);
	position.x = CLIP<int>(position.x, 0, celInfo->width - 1);
	position.y = CLIP<int>(position.y, 0, celInfo->height - 1);
	byte *celData = tmpView->getBitmap(loopNo, celNo);
	bool result = (celData[position.y * celInfo->width + position.x] == celInfo->clearKey);
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

		View *tmpView = _cache->getView(viewId);
		Common::Rect celRect;

		tmpView->getCelRect(loopNo, celNo, x, y, z, &celRect);
		celRect.bottom = y + 1;
		celRect.top = celRect.bottom - yStep;

		PUT_SEL32V(_s->_segMan, object, brLeft, celRect.left);
		PUT_SEL32V(_s->_segMan, object, brRight, celRect.right);
		PUT_SEL32V(_s->_segMan, object, brTop, celRect.top);
		PUT_SEL32V(_s->_segMan, object, brBottom, celRect.bottom);
	}
}

void SciGui::hideCursor() {
	_cursor->hide();
}

void SciGui::showCursor() {
	_cursor->show();
}

bool SciGui::isCursorVisible() {
	return _cursor->isVisible();
}

void SciGui::setCursorShape(GuiResourceId cursorId) {
	_cursor->setShape(cursorId);
}

void SciGui::setCursorView(GuiResourceId viewNum, int loopNum, int cellNum, Common::Point *hotspot) {
	_cursor->setView(viewNum, loopNum, cellNum, hotspot);
}

void SciGui::setCursorPos(Common::Point pos) {
	pos.y += _ports->getPort()->top;
	pos.x += _ports->getPort()->left;
	moveCursor(pos);
}

Common::Point SciGui::getCursorPos() {
	return _cursor->getPosition();
}

void SciGui::moveCursor(Common::Point pos) {
	pos.y += _ports->_picWind->rect.top;
	pos.x += _ports->_picWind->rect.left;

	pos.y = CLIP<int16>(pos.y, _ports->_picWind->rect.top, _ports->_picWind->rect.bottom - 1);
	pos.x = CLIP<int16>(pos.x, _ports->_picWind->rect.left, _ports->_picWind->rect.right - 1);

	if (pos.x > _screen->getWidth() || pos.y > _screen->getHeight()) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return;
	}

	_cursor->setPosition(pos);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	_s->_event->get(SCI_EVENT_PEEK);
}

void SciGui::setCursorZone(Common::Rect zone) {
	_cursor->setMoveZone(zone);
}

reg_t SciGui::portraitLoad(Common::String resourceName) {
	//Portrait *myPortrait = new Portrait(_s->resMan, _screen, _palette, resourceName);
	return NULL_REG;
}

void SciGui::portraitShow(Common::String resourceName, Common::Point position, uint16 resourceId, uint16 noun, uint16 verb, uint16 cond, uint16 seq) {
	Portrait *myPortrait = new Portrait(_s->resMan, _s->_event, this, _screen, _palette, _audio, resourceName);
	// TODO: cache portraits
	// adjust given coordinates to curPort (but dont adjust coordinates on upscaledHires_Save_Box and give us hires coordinates
	//  on kDrawCel, yeah this whole stuff makes sense)
	position.x += _ports->getPort()->left; position.y += _ports->getPort()->top;
	position.x *= 2; position.y *= 2;
	myPortrait->doit(position, resourceId, noun, verb, cond, seq);
	delete myPortrait;
}

void SciGui::portraitUnload(uint16 portraitId) {
}

void SciGui::startPalVary(uint16 paletteId, uint16 ticks) {
	if (_palVaryId >= 0)	// another palvary is taking place, return
		return;

	_palVaryId = paletteId;
	_palVaryStart = g_system->getMillis();
	_palVaryEnd = _palVaryStart + ticks * 1000 / 60;
	((SciEngine*)g_engine)->getTimerManager()->installTimerProc(&palVaryCallback, 1000 / 60, this);
}

void SciGui::togglePalVary(bool pause) {
	// this call is actually counting states, so calling this 3 times with true will require calling it later
	// 3 times with false to actually remove pause

	// TODO
}

void SciGui::stopPalVary() {
	((SciEngine*)g_engine)->getTimerManager()->removeTimerProc(&palVaryCallback);
	_palVaryId = -1;	// invalidate the target palette

	// HACK: just set the target palette
	_palette->kernelSetFromResource(_palVaryId, true);
}

void SciGui::palVaryCallback(void *refCon) {
	((SciGui *)refCon)->doPalVary();
}

void SciGui::doPalVary() {
	// TODO: do palette transition here...
}

bool SciGui::debugUndither(bool flag) {
	_screen->unditherSetState(flag);
	return false;
}

bool SciGui::debugShowMap(int mapNo) {
	_screen->debugShowMap(mapNo);
	return false;
}

bool SciGui::debugEGAdrawingVisualize(bool state) {
	_paint16->setEGAdrawingVisualize(state);
	return false;
}

} // End of namespace Sci
