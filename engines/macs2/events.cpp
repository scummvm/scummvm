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

#include "macs2/events.h"
#include "common/config-manager.h"
#include "graphics/screen.h"
#include "macs2/detection.h"
#include "macs2/macs2.h"
#include "macs2/macs2_constants.h"
#include "macs2/view1.h"

namespace Macs2 {

Events *g_events;

Events::Events() : UIElement("Root", nullptr) {
	g_events = this;
}

Events::~Events() {
	g_events = nullptr;
}

void Events::runGame() {
	_screen = new Graphics::Screen();
	View1 view1;
	addView(&view1);

	// Run the game
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		g_engine->loadGameState(saveSlot);

	// DOS timer ISR fires at ~21.6Hz (PIT divisor 0xD7B0 = 55216, 1193182/55216).
	// Main loop processes a game frame when g_wTimerTickCounter > 1 (every ~2 ticks).
	// Effective game frame rate: ~10.8fps (every ~92ms).
	// We use wall-clock timing to match the original rate precisely.
	uint32 lastTickTime = g_system->getMillis();
	// One timer tick = 1000/21.6 ≈ 46.3ms. Game ticks every 2 timer ticks = ~92.6ms.
	static constexpr uint32 kTimerTickMs = 46;
	uint16 timerTickCounter = 0;

	Common::Event e;
	while (!_views.empty() && !shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_QUIT ||
				e.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				_views.clear();
				break;
			} else {
				processEvent(e);
			}
		}

		if (_views.empty())
			break;

		currentMillis = g_system->getMillis();

		// Accumulate timer ticks based on elapsed wall-clock time
		const uint32 elapsed = currentMillis - lastTickTime;
		if (elapsed >= kTimerTickMs) {
			const uint16 ticks = elapsed / kTimerTickMs;
			timerTickCounter += ticks;
			lastTickTime += ticks * kTimerTickMs;
		}

		bool doTick = false;
		switch (g_engine->_gameSpeedMode) {
		case 1: // fast mode: tick every frame
			doTick = true;
			break;
		case 2: // slow mode: tick when counter >= 0x12
			doTick = (timerTickCounter >= 0x12); // TODO: this is running at different speed compared to running this in dosbox
			break;
		default: // normal: tick when counter > 1
			doTick = (timerTickCounter > 1);
			break;
		}

		if (doTick) {
			timerTickCounter = 0;
			tick();
			drawElements();
			_screen->update();
		}

		g_system->delayMillis(10); // Short sleep for responsiveness; timing is wall-clock based
	}

	delete _screen;
}

void Events::processEvent(Common::Event &ev) {
	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
		if (ev.kbd.keycode < Common::KEYCODE_NUMLOCK)
			msgKeypress(KeypressMessage(ev.kbd));
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		msgAction(ActionMessage(ev.customType));
		break;
	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		// Gamepad buttons map to custom actions; mouse uses raw EVENT_*BUTTON* below.
		switch (ev.customType) {
		case kMacs2ActionInteract:
			msgMouseUp(MouseUpMessage(MouseUpMessage::MB_LEFT, ev.mouse));
			break;
		case kMacs2ActionCursorMode:
			msgMouseUp(MouseUpMessage(MouseUpMessage::MB_RIGHT, ev.mouse));
			break;
		default:
			break;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		msgMouseDown(MouseDownMessage(MouseDownMessage::MB_LEFT, ev.mouse));
		break;
	case Common::EVENT_RBUTTONDOWN:
		// case Common::EVENT_MBUTTONDOWN:
		msgMouseDown(MouseDownMessage(MouseDownMessage::MB_RIGHT, ev.mouse));
		break;
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		// case Common::EVENT_MBUTTONUP:
		msgMouseUp(MouseUpMessage(ev.type, ev.mouse));
		break;
	case Common::EVENT_MOUSEMOVE:
		msgMouseMove(MouseMoveMessage(ev.type, ev.mouse));
	default:
		break;
	}
}

void Events::addView(UIElement *ui) {
	assert(ui);
	UIElement *priorView = focusedView();

	if (!_views.empty())
		priorView->msgUnfocus(UnfocusMessage());

	_views.push(ui);
	ui->redraw();
	ui->msgFocus(FocusMessage(priorView));
}

/*------------------------------------------------------------------------*/

Bounds::Bounds(Common::Rect &innerBounds) : _bounds(0, 0, kScreenWidth, kGameHeight),
											_innerBounds(innerBounds) {
}

Bounds &Bounds::operator=(const Common::Rect &r) {
	_bounds = r;
	_innerBounds = r;
	return *this;
}

/*------------------------------------------------------------------------*/

UIElement::UIElement(const Common::String &name) : _name(name), _bounds(_innerBounds) {
	g_engine->_children.push_back(this);
}

UIElement::UIElement(const Common::String &name, UIElement *uiParent) : _name(name), _bounds(_innerBounds) {
	if (uiParent)
		uiParent->_children.push_back(this);
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

void UIElement::draw() {
	for (size_t i = 0; i < _children.size(); ++i) {
		_children[i]->draw();
	}
}

bool UIElement::tick() {
	for (size_t i = 0; i < _children.size(); ++i) {
		if (_children[i]->tick())
			return true;
	}

	return false;
}

UIElement *UIElement::findView(const Common::String &name) {
	if (_name.equalsIgnoreCase(name))
		return this;

	UIElement *result;
	for (size_t i = 0; i < _children.size(); ++i) {
		if ((result = _children[i]->findView(name)) != nullptr)
			return result;
	}

	return nullptr;
}

Graphics::ManagedSurface UIElement::getSurface() const {
	return Graphics::ManagedSurface(*g_events->getScreen(), _bounds);
}

} // namespace Macs2
