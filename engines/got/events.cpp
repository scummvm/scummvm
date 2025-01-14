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

#include "got/events.h"
#include "common/config-manager.h"
#include "got/gfx/palette.h"
#include "got/got.h"
#include "got/views/views.h"
#include "graphics/screen.h"

namespace Got {

Events *g_events;

// Index and RGB63 values used for palette animation
// like water effects and gems sparkling
static const uint16 PAL_CLR1[] = {0xf300, 0x003b, 0xf000, 0x003b, 0xf100, 0x003b, 0xf200, 0x003b};
static const uint16 PAL_SET1[] = {0xf027, 0x273f, 0xf127, 0x273f, 0xf227, 0x273f, 0xf327, 0x273f};
static const uint16 PAL_CLR2[] = {0xf73b, 0x0000, 0xf43b, 0x0000, 0xf53b, 0x0000, 0xf63b, 0x0000};
static const uint16 PAL_SET2[] = {0xf43f, 0x2727, 0xf53f, 0x2727, 0xf63f, 0x2727, 0xf73f, 0x2727};

Events::Events() : UIElement("Root", nullptr) {
	g_events = this;
}

Events::~Events() {
	g_events = nullptr;
}

void Events::runGame() {
	uint nextFrameTime = 0;
	int palCycleCtr = 0;
	_screen = new Graphics::Screen();
	Views::Views views; // Loads all views in the structure

	// Set up the initial game view
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1) {
		if (g_engine->loadGameState(saveSlot).getCode() != Common::kNoError)
			saveSlot = -1;
	}
	if (saveSlot == -1)
		addView(isDemo() ? "Game" : "SplashScreen");

	// Main game loop
	Common::Event e;
	while (!_views.empty() && !shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_QUIT ||
				e.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_views.clear();
				break;
			}

			processEvent(e);
		}

		if (_views.empty())
			break;

		g_system->delayMillis(10);

		// Rotate the palette
		if (++palCycleCtr == 2) {
			palCycleCtr = 0;

			bool gameVisible = false;
			for (uint i = 0; i < _views.size() && !gameVisible; ++i)
				gameVisible = _views[i]->_name == "Game";

			if (gameVisible)
				rotatePalette();
		}
		uint currTime = g_system->getMillis();
		if (currTime >= nextFrameTime) {
			nextFrameTime = currTime + FRAME_DELAY;
			nextFrame();
		}
	}

	delete _screen;
}

void Events::nextFrame() {
	// Update state variables
	_G(rand1) = getRandomNumber(99);
	_G(rand2) = getRandomNumber(99);
	_G(pge) = _G(pge) ^ 1;
	_G(shot_ok) = true;
	_G(magic_cnt++);

	// In demo mode, handle the next key
	if (_G(demo) && focusedView()->getName() == "Game") {
		if (_G(demoKeys).empty()) {
			_G(demo) = false;
			send("TitleBackground", GameMessage("MAIN_MENU"));
			return;
		}

		processDemoEvent(_G(demoKeys).pop());
	}

	// Check if any script needs resuming
	_G(scripts).runIfResuming();

	// Do tick action to the views to handle gameplay logic
	tick();

	// Draw the current view's elements as needed, and update screen
	drawElements();
	_screen->update();
}

#define LOOP_THRESHOLD 5

void Events::rotatePalette() {
	++_palLoop;

	if (_palLoop > LOOP_THRESHOLD) {
		_palLoop = 0;
	} else {
		const uint16 *entry;
		switch (_palLoop) {
		case LOOP_THRESHOLD - 4:
			entry = &PAL_CLR2[_palCnt2];
			break;
		case LOOP_THRESHOLD - 3:
			entry = &PAL_SET2[_palCnt2];

			_palCnt2 += 2;
			if (_palCnt2 >= 8)
				_palCnt2 = 0;
			break;
		case LOOP_THRESHOLD - 2:
			entry = &PAL_CLR1[_palCnt1];
			break;
		case LOOP_THRESHOLD - 1:
			entry = &PAL_SET1[_palCnt1];

			_palCnt1 += 2;
			if (_palCnt1 >= 8)
				_palCnt1 = 0;
			break;
		default:
			return;
		}

		Gfx::xsetpal(entry[0] >> 8, (entry[0] & 0xff) << 2,
					 (entry[1] >> 8) << 2, (entry[1] & 0xff) << 2);
	}
}

void Events::processEvent(Common::Event &ev) {
	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
		if (!_G(demo)) {
			if (ev.kbd.keycode < 100)
				_G(key_flag[ev.kbd.keycode]) = true;

			if (ev.kbd.keycode < Common::KEYCODE_NUMLOCK)
				msgKeypress(KeypressMessage(ev.kbd));
		}
		break;
	case Common::EVENT_KEYUP:
		if (!_G(demo) && ev.kbd.keycode < 100)
			_G(key_flag[ev.kbd.keycode]) = false;
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		if (!_G(demo)) {
			_G(key_flag[actionToKeyFlag(ev.customType)]) = true;
			msgAction(ActionMessage(ev.customType));
		} else if (ev.customType == KEYBIND_ESCAPE) {
			// The Escape action will return to main menu from demo
			_G(demo) = false;
			send("TitleBackground", GameMessage("MAIN_MENU"));
		}
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		if (!_G(demo))
			_G(key_flag[actionToKeyFlag(ev.customType)]) = false;
		break;
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
		if (!_G(demo))
			msgMouseDown(MouseDownMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONUP:
		if (!_G(demo))
			msgMouseUp(MouseUpMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_MOUSEMOVE:
		if (!_G(demo))
			msgMouseMove(MouseMoveMessage(ev.type, ev.mouse));
		break;
	default:
		break;
	}
}

void Events::processDemoEvent(byte ev) {
	if (!ev)
		return;

	bool flag = ev & 0x80;
	ev &= 0x7f;

	int action = -1;
	switch (ev) {
	case 72:
		ev = key_up;
		action = KEYBIND_UP;
		break;
	case 80:
		ev = key_down;
		break;
	case 75:
		ev = key_left;
		break;
	case 77:
		ev = key_right;
		break;
	case 56:
		ev = key_fire;
		action = KEYBIND_FIRE;
		break;
	case 29:
		ev = key_magic;
		action = KEYBIND_MAGIC;
		break;
	case 39:
		ev = key_select;
		action = KEYBIND_SELECT;
		break;
	default:
		break;
	}

	_G(key_flag[ev]) = flag;

	if (flag && action != -1)
		msgAction(ActionMessage(action));
}

int Events::actionToKeyFlag(int action) const {
	return (action == KEYBIND_ESCAPE) ? Common::KEYCODE_ESCAPE : (int)action;
}

void Events::replaceView(UIElement *ui, bool replaceAllViews, bool fadeOutIn) {
	assert(ui);
	UIElement *oldView = focusedView();

	if (fadeOutIn)
		Gfx::fade_out();

	if (replaceAllViews) {
		clearViews();

	} else if (!_views.empty()) {
		oldView->msgUnfocus(UnfocusMessage());
		_views.pop();
	}

	// Redraw any prior views to erase the removed view
	for (uint i = 0; i < _views.size(); ++i) {
		_views[i]->redraw();
		_views[i]->draw();
	}

	// Add the new view
	_views.push(ui);

	ui->redraw();
	ui->msgFocus(FocusMessage(oldView));
	ui->draw();

	if (fadeOutIn)
		Gfx::fade_in();
}

void Events::replaceView(const Common::String &name, bool replaceAllViews, bool fadeOutIn) {
	replaceView(findView(name), replaceAllViews, fadeOutIn);
}

void Events::addView(UIElement *ui) {
	assert(ui);
	UIElement *oldView = focusedView();

	if (!_views.empty())
		oldView->msgUnfocus(UnfocusMessage());

	_views.push(ui);
	ui->redraw();
	ui->msgFocus(FocusMessage(oldView));
}

void Events::addView(const Common::String &name) {
	addView(findView(name));
}

void Events::popView() {
	UIElement *oldView = focusedView();
	oldView->msgUnfocus(UnfocusMessage());
	_views.pop();

	for (uint i = 0; i < _views.size(); ++i) {
		_views[i]->redraw();
		_views[i]->draw();
	}

	if (!_views.empty()) {
		UIElement *view = focusedView();
		view->msgFocus(FocusMessage(oldView));
		view->redraw();
		view->draw();
	}
}

bool Events::isPresent(const Common::String &name) const {
	for (uint i = 0; i < _views.size(); ++i) {
		if (_views[i]->_name == name)
			return true;
	}

	return false;
}

void Events::drawElements() {
	if (!_views.empty())
		focusedView()->drawElements();
}

void Events::clearViews() {
	if (!_views.empty())
		focusedView()->msgUnfocus(UnfocusMessage());

	_views.clear();
}

/*------------------------------------------------------------------------*/

Bounds::Bounds(Common::Rect &innerBounds) : _bounds(0, 0, 320, 240),
											_innerBounds(innerBounds),
											left(_bounds.left), top(_bounds.top),
											right(_bounds.right), bottom(_bounds.bottom) {
}

Bounds &Bounds::operator=(const Common::Rect &r) {
	_bounds = r;
	_innerBounds = r;
	_innerBounds.grow(-_borderSize);
	return *this;
}

void Bounds::setBorderSize(size_t borderSize) {
	_borderSize = borderSize;
	_innerBounds = *this;
	_innerBounds.grow(-_borderSize);
}

/*------------------------------------------------------------------------*/

UIElement::UIElement(const Common::String &name) : _name(name), _parent(g_engine), _bounds(_innerBounds) {
	g_engine->_children.push_back(this);
}

UIElement::UIElement(const Common::String &name, UIElement *uiParent) : _name(name), _parent(uiParent), _bounds(_innerBounds) {
	if (_parent)
		_parent->_children.push_back(this);
}

void UIElement::redraw() {
	_needsRedraw = true;

	for (size_t i = 0; i < _children.size(); ++i)
		_children[i]->redraw();
}

void UIElement::drawElements() {
	if (_needsRedraw) {
		draw();
		_needsRedraw = false;
	}

	for (size_t i = 0; i < _children.size(); ++i)
		_children[i]->drawElements();
}

UIElement *UIElement::findViewGlobally(const Common::String &name) {
	return g_events->findView(name);
}

void UIElement::close() {
	assert(g_events->focusedView() == this);
	g_events->popView();
}

void UIElement::draw() {
	for (size_t i = 0; i < _children.size(); ++i) {
		_children[i]->draw();
	}
}

bool UIElement::tick() {
	if (_timeoutCtr && --_timeoutCtr == 0) {
		timeout();
	}

	for (size_t i = 0; i < _children.size(); ++i) {
		if (_children[i]->tick())
			return true;
	}

	return false;
}

UIElement *UIElement::findView(const Common::String &name) {
	if (_name.equalsIgnoreCase(name))
		return this;

	for (size_t i = 0; i < _children.size(); ++i) {
		UIElement *result = _children[i]->findView(name);
		if (result != nullptr)
			return result;
	}

	return nullptr;
}

void UIElement::replaceView(UIElement *ui, bool replaceAllViews, bool fadeOutIn) {
	g_events->replaceView(ui, replaceAllViews, fadeOutIn);
}

void UIElement::replaceView(const Common::String &name, bool replaceAllViews, bool fadeOutIn) {
	g_events->replaceView(name, replaceAllViews, fadeOutIn);
}

void UIElement::addView(UIElement *ui) {
	g_events->addView(ui);
}

void UIElement::addView(const Common::String &name) {
	g_events->addView(name);
}

void UIElement::addView() {
	g_events->addView(this);
}

GfxSurface UIElement::getSurface(bool innerBounds) const {
	return GfxSurface(*g_events->getScreen(),
					  innerBounds ? _innerBounds : _bounds);
}

int UIElement::getRandomNumber(int minNumber, int maxNumber) {
	return g_engine->getRandomNumber(maxNumber - minNumber + 1) + minNumber;
}

int UIElement::getRandomNumber(int maxNumber) {
	return g_engine->getRandomNumber(maxNumber);
}

void UIElement::timeout() {
	redraw();
}

} // namespace Got
