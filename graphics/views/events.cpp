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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/text-to-speech.h"
#include "engines/engine.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/screen.h"
#include "graphics/views/events.h"

namespace Graphics {
namespace Views {

#define FRAME_RATE 20
#define FRAME_DELAY (1000 / FRAME_RATE)

Events *g_events;

Events::Events(const Common::String &engineName) : UIElement("Root", nullptr), _randomSource(engineName) {
	g_events = this;
}

Events::~Events() {
	g_events = nullptr;
}

void Events::runGame() {
	uint nextFrameTime = 0;
	_screen = new Graphics::Screen();

	// Run the game
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		g_engine->loadGameState(saveSlot);

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

		uint currTime = g_system->getMillis();
		if (currTime >= nextFrameTime) {
			nextFrameTime = currTime + FRAME_DELAY;
			nextFrame();
		}
	}

	delete _screen;
}

int Events::getRandomNumber(int minNumber, int maxNumber) {
	return _randomSource.getRandomNumberRng(minNumber, maxNumber);
}

int Events::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void Events::nextFrame() {
	// Do tick action to the views to handle gameplay logic
	tick();

	// Draw the current view's elements as needed, and update screen
	drawElements();
	updateScreen();
}

void Events::updateScreen() {
	_screen->update();
}

#define LOOP_THRESHOLD 5

void Events::processEvent(Common::Event &ev) {
	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
		if (ev.kbd.keycode < Common::KEYCODE_NUMLOCK)
			msgKeypress(KeypressMessage(ev.kbd));
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		msgAction(ActionMessage(ev.customType));
		break;
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
		msgMouseDown(MouseDownMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONUP:
		msgMouseUp(MouseUpMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_MOUSEMOVE:
		msgMouseMove(MouseMoveMessage(ev.type, ev.mouse));
		break;
	default:
		break;
	}
}

void Events::replaceView(UIElement *ui, bool replaceAllViews) {
	assert(ui);
	UIElement *oldView = focusedView();

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
}

void Events::replaceView(const Common::String &name, bool replaceAllViews) {
	replaceView(findView(name), replaceAllViews);
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

Graphics::Font *Events::createFont() const {
	return new Graphics::DosFont();
}

/*------------------------------------------------------------------------*/

Bounds::Bounds(Common::Rect &innerBounds) : _bounds(0, 0, 320, 200), _innerBounds(innerBounds),
		left(_bounds.left), top(_bounds.top), right(_bounds.right), bottom(_bounds.bottom) {
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

UIElement::UIElement(const Common::String &name) : _name(name), _parent(g_events), _bounds(_innerBounds) {
	g_events->_children.push_back(this);
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
#ifdef USE_TTS
	stopTextToSpeech();
#endif
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

void UIElement::replaceView(UIElement *ui, bool replaceAllViews) {
	g_events->replaceView(ui, replaceAllViews);
}

void UIElement::replaceView(const Common::String &name, bool replaceAllViews) {
	g_events->replaceView(name, replaceAllViews);
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
	return g_events->getRandomNumber(minNumber, maxNumber);
}

int UIElement::getRandomNumber(int maxNumber) {
	return g_events->getRandomNumber(maxNumber);
}

void UIElement::delaySeconds(uint seconds) {
	_timeoutCtr = seconds * FRAME_RATE;
}

void UIElement::delayFrames(uint frames) {
	_timeoutCtr = frames;
}

void UIElement::timeout() {
	redraw();
}

#ifdef USE_TTS

void UIElement::stopTextToSpeech() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan && ConfMan.getBool("tts_enabled") && ttsMan->isSpeaking()) {
		ttsMan->stop();
	}

	_previousSaid.clear();
}

#endif

} // namespace Views
} // namespace Graphics
