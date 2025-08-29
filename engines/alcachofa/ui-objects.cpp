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

#include "alcachofa/alcachofa.h"
#include "alcachofa/script.h"
#include "alcachofa/global-ui.h"
#include "alcachofa/menu.h"
#include "alcachofa/objects.h"
#include "alcachofa/rooms.h"

using namespace Common;

namespace Alcachofa {

const char *MenuButton::typeName() const { return "MenuButton"; }

MenuButton::MenuButton(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _actionId(stream.readSint32LE())
	, _graphicNormal(stream)
	, _graphicHovered(stream)
	, _graphicClicked(stream)
	, _graphicDisabled(stream) {}

void MenuButton::draw() {
	if (!isEnabled())
		return;
	Graphic &graphic =
		!_isInteractable ? _graphicDisabled
		: _isClicked ? _graphicClicked
		: wasSelected() ? _graphicHovered
		: _graphicNormal;
	graphic.update();
	g_engine->drawQueue().add<AnimationDrawRequest>(graphic, true, BlendMode::AdditiveAlpha);
}

void MenuButton::update() {
	PhysicalObject::update();
	if (!_isClicked)
		return;

	_graphicClicked.update();
	if (!_graphicClicked.isPaused())
		return;

	if (!_triggerNextFrame) {
		// another delay probably to show the last frame of animation
		_triggerNextFrame = true;
		return;
	}

	_interactionLock.release();
	_triggerNextFrame = false;
	_isClicked = false;
	trigger();
}

void MenuButton::loadResources() {
	_graphicNormal.loadResources();
	_graphicHovered.loadResources();
	_graphicClicked.loadResources();
	_graphicDisabled.loadResources();
}

void MenuButton::freeResources() {
	_graphicNormal.freeResources();
	_graphicHovered.freeResources();
	_graphicClicked.freeResources();
	_graphicDisabled.freeResources();
}

void MenuButton::onHoverUpdate() {}

void MenuButton::onClick() {
	if (_isInteractable && _interactionLock.isReleased()) {
		_interactionLock = FakeLock("button", g_engine->menu().interactionSemaphore());
		_isClicked = true;
		_triggerNextFrame = false;
		_graphicClicked.start(false);
	}
}

void MenuButton::trigger() {
	// all menu buttons should be inherited and override trigger
	warning("Unimplemented %s %s action %d", typeName(), name().c_str(), _actionId);
}

const char *InternetMenuButton::typeName() const { return "InternetMenuButton"; }

InternetMenuButton::InternetMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {}

const char *OptionsMenuButton::typeName() const { return "OptionsMenuButton"; }

OptionsMenuButton::OptionsMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {}

void OptionsMenuButton::update() {
	MenuButton::update();
	const auto action = (OptionsMenuAction)actionId();
	if (action == OptionsMenuAction::MainMenu && g_engine->input().wasMenuKeyPressed())
		onClick();
}

void OptionsMenuButton::trigger() {
	g_engine->menu().triggerOptionsAction((OptionsMenuAction)actionId());
}

const char *MainMenuButton::typeName() const { return "MainMenuButton"; }

MainMenuButton::MainMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {}

void MainMenuButton::update() {
	MenuButton::update();
	const auto action = (MainMenuAction)actionId();
	if (g_engine->input().wasMenuKeyPressed() &&
		(action == MainMenuAction::ContinueGame || action == MainMenuAction::NewGame))
		onClick();
}

void MainMenuButton::trigger() {
	g_engine->menu().triggerMainMenuAction((MainMenuAction)actionId());
}

const char *PushButton::typeName() const { return "PushButton"; }

PushButton::PushButton(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _alwaysVisible(readBool(stream))
	, _graphic1(stream)
	, _graphic2(stream)
	, _actionId(stream.readSint32LE()) {}

const char *EditBox::typeName() const { return "EditBox"; }

EditBox::EditBox(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, i1(stream.readSint32LE())
	, p1(Shape(stream).firstPoint())
	, _labelId(readVarString(stream))
	, b1(readBool(stream))
	, i3(stream.readSint32LE())
	, i4(stream.readSint32LE())
	, i5(stream.readSint32LE())
	, _fontId(0) {

	if (g_engine->version() == EngineVersion::V3_1)
		_fontId = stream.readSint32LE();
}

const char *CheckBox::typeName() const { return "CheckBox"; }

CheckBox::CheckBox(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _isChecked(readBool(stream))
	, _graphicUnchecked(stream)
	, _graphicChecked(stream)
	, _graphicHovered(stream)
	, _graphicClicked(stream)
	, _actionId(stream.readSint32LE()) {}

void CheckBox::draw() {
	if (!isEnabled())
		return;
	Graphic &baseGraphic = _isChecked ? _graphicChecked : _graphicUnchecked;
	baseGraphic.update();
	g_engine->drawQueue().add<AnimationDrawRequest>(baseGraphic, true, BlendMode::AdditiveAlpha);

	if (wasSelected()) {
		Graphic &hoverGraphic = _wasClicked ? _graphicClicked : _graphicHovered;
		hoverGraphic.update();
		g_engine->drawQueue().add<AnimationDrawRequest>(hoverGraphic, true, BlendMode::AdditiveAlpha);
	}
}

void CheckBox::update() {
	PhysicalObject::update();
	if (_wasClicked) {
		if (g_engine->getMillis() - _clickTime > 500) {
			_wasClicked = false;
			trigger();
		}
	}

	// the original engine would stall the application as click delay.
	// this would prevent bacterios arm in movie adventure being rendered twice for multiple checkboxes
	// we can instead check the hovered state and prevent the arm (clicked/hovered graphic) being drawn
}

void CheckBox::loadResources() {
	_wasClicked = false;
	_graphicUnchecked.loadResources();
	_graphicChecked.loadResources();
	_graphicHovered.loadResources();
	_graphicClicked.loadResources();
}

void CheckBox::freeResources() {
	_graphicUnchecked.freeResources();
	_graphicChecked.freeResources();
	_graphicHovered.freeResources();
	_graphicClicked.freeResources();
}

void CheckBox::onHoverUpdate() {}

void CheckBox::onClick() {
	_wasClicked = true;
	_clickTime = g_engine->getMillis();
}

void CheckBox::trigger() {
	g_engine->menu().triggerOptionsAction((OptionsMenuAction)actionId());
}

const char *CheckBoxAutoAdjustNoise::typeName() const { return "CheckBoxAutoAdjustNoise"; }

CheckBoxAutoAdjustNoise::CheckBoxAutoAdjustNoise(Room *room, ReadStream &stream)
	: CheckBox(room, stream) {
	stream.readByte(); // unused and ignored byte
}

const char *SlideButton::typeName() const { return "SlideButton"; }

SlideButton::SlideButton(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _valueId(stream.readSint32LE())
	, _minPos(Shape(stream).firstPoint())
	, _maxPos(Shape(stream).firstPoint())
	, _graphicIdle(stream)
	, _graphicHovered(stream)
	, _graphicClicked(stream) {}

void SlideButton::draw() {
	auto *optionsMenu = dynamic_cast<OptionsMenu *>(room());
	scumm_assert(optionsMenu != nullptr);

	Graphic *activeGraphic;
	if (optionsMenu->currentSlideButton() == this && g_engine->input().isMouseLeftDown())
		activeGraphic = &_graphicClicked;
	else
		activeGraphic = isMouseOver() ? &_graphicHovered : &_graphicIdle;
	activeGraphic->update();
	g_engine->drawQueue().add<AnimationDrawRequest>(*activeGraphic, true, BlendMode::AdditiveAlpha);
}

void SlideButton::update() {
	const auto mousePos = g_engine->input().mousePos2D();
	auto *optionsMenu = dynamic_cast<OptionsMenu *>(room());
	scumm_assert(optionsMenu != nullptr);

	if (optionsMenu->currentSlideButton() == this) {
		if (!g_engine->input().isMouseLeftDown()) {
			optionsMenu->currentSlideButton() = nullptr;
			g_engine->menu().triggerOptionsValue((OptionsMenuValue)_valueId, _value);
			update(); // to update the position
		} else {
			int clippedMousePosY = CLIP(mousePos.y, _minPos.y, _maxPos.y);
			_value = (_maxPos.y - clippedMousePosY) / (float)(_maxPos.y - _minPos.y);
			_graphicClicked.topLeft() = Point((_minPos.x + _maxPos.x) / 2, clippedMousePosY);
		}
	} else {
		_graphicIdle.topLeft() = Point(
			(_minPos.x + _maxPos.x) / 2,
			(int16)(_maxPos.y - _value * (_maxPos.y - _minPos.y)));
		if (!isMouseOver())
			return;
		_graphicHovered.topLeft() = _graphicIdle.topLeft();
		if (g_engine->input().wasMouseLeftPressed())
			optionsMenu->currentSlideButton() = this;
		optionsMenu->clearLastSelectedObject();
		g_engine->player().selectedObject() = nullptr;
	}
}

void SlideButton::loadResources() {
	_graphicIdle.loadResources();
	_graphicHovered.loadResources();
	_graphicClicked.loadResources();
}

void SlideButton::freeResources() {
	_graphicIdle.freeResources();
	_graphicHovered.freeResources();
	_graphicClicked.freeResources();
}

bool SlideButton::isMouseOver() const {
	const auto mousePos = g_engine->input().mousePos2D();
	return
		mousePos.x >= _minPos.x && mousePos.y >= _minPos.y &&
		mousePos.x <= _maxPos.x && mousePos.y <= _maxPos.y;
}

const char *IRCWindow::typeName() const { return "IRCWindow"; }

IRCWindow::IRCWindow(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _p1(Shape(stream).firstPoint())
	, _p2(Shape(stream).firstPoint()) {}

const char *MessageBox::typeName() const { return "MessageBox"; }

MessageBox::MessageBox(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _graph1(stream)
	, _graph2(stream)
	, _graph3(stream)
	, _graph4(stream)
	, _graph5(stream) {
	_graph1.start(true);
	_graph2.start(true);
	_graph3.start(true);
	_graph4.start(true);
	_graph5.start(true);
}

const char *VoiceMeter::typeName() const { return "VoiceMeter"; }

VoiceMeter::VoiceMeter(Room *room, ReadStream &stream)
	: GraphicObject(room, stream) {
	stream.readByte(); // unused and ignored byte
}

}
