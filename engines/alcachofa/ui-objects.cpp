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

#include "alcachofa.h"
#include "script.h"
#include "global-ui.h"
#include "menu.h"
#include "objects.h"
#include "rooms.h"

using namespace Common;

namespace Alcachofa {

enum class MainMenuAction : int32 {
	ContinueGame = 0,
	Save,
	Load,
	InternetMenu,
	OptionsMenu,
	Exit,
	NextSave,
	PrevSave,
	NewGame,
	AlsoExit // there seems to be no difference to Exit
};

const char *MenuButton::typeName() const { return "MenuButton"; }

MenuButton::MenuButton(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _actionId(stream.readSint32LE())
	, _graphicNormal(stream)
	, _graphicHovered(stream)
	, _graphicClicked(stream)
	, _graphicDisabled(stream) {
}

void MenuButton::draw() {
	if (!isEnabled())
		return;
	Graphic &graphic =
		!_isInteractable ? _graphicDisabled
		: _isClicked ? _graphicClicked
		: _isHovered ? _graphicHovered
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

void MenuButton::onHoverStart() {
	PhysicalObject::onHoverStart();
	_isHovered = true;
}

void MenuButton::onHoverEnd() {
	PhysicalObject::onHoverEnd();
	_isHovered = false;
}

void MenuButton::onClick() {
	if (_isInteractable) {
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
	: MenuButton(room, stream) {
}

const char *OptionsMenuButton::typeName() const { return "OptionsMenuButton"; }

OptionsMenuButton::OptionsMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {
}

const char *MainMenuButton::typeName() const { return "MainMenuButton"; }

MainMenuButton::MainMenuButton(Room *room, ReadStream &stream)
	: MenuButton(room, stream) {
}

void MainMenuButton::update() {
	MenuButton::update();
	const auto action = (MainMenuAction)actionId();
	if (g_engine->input().wasMenuKeyPressed() &&
		(action == MainMenuAction::ContinueGame || action == MainMenuAction::NewGame))
		onClick();
}

void MainMenuButton::trigger() {
	switch ((MainMenuAction)actionId()) {
	case MainMenuAction::ContinueGame:
		g_engine->menu().continueGame();
		break;
	case MainMenuAction::Save:
		warning("STUB: MainMenuAction Save");
		break;
	case MainMenuAction::Load:
		warning("STUB: MainMenuAction Load");
		break;
	case MainMenuAction::InternetMenu:
		g_system->messageBox(LogMessageType::kWarning, "Multiplayer is not implemented in this ScummVM version.");
		break;
	case MainMenuAction::OptionsMenu:
		g_engine->menu().openOptionsMenu();
		break;
	case MainMenuAction::Exit:
	case MainMenuAction::AlsoExit:
		// implemented in AlcachofaEngine as it has its own event loop
		g_engine->fadeExit();
		break;
	case MainMenuAction::NextSave:
		warning("STUB: MainMenuAction NextSave");
		break;
	case MainMenuAction::PrevSave:
		warning("STUB: MainMenuAction PrevSave");
		break;
	case MainMenuAction::NewGame:
		g_engine->menu().newGame();
		break;
	default:
		warning("Unknown main menu action: %d", actionId());
		break;
	}
}

const char *PushButton::typeName() const { return "PushButton"; }

PushButton::PushButton(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _alwaysVisible(readBool(stream))
	, _graphic1(stream)
	, _graphic2(stream)
	, _actionId(stream.readSint32LE()) {
}

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
	, _fontId(stream.readSint32LE()) {
}

const char *CheckBox::typeName() const { return "CheckBox"; }

CheckBox::CheckBox(Room *room, ReadStream &stream)
	: PhysicalObject(room, stream)
	, _isChecked(readBool(stream))
	, _graphicUnchecked(stream)
	, _graphicChecked(stream)
	, _graphicHovered(stream)
	, _graphicClicked(stream)
	, _actionId(stream.readSint32LE()) {
}

void CheckBox::update() {
	PhysicalObject::update();
	if (!isEnabled())
		return;
	Graphic &baseGraphic = _isChecked ? _graphicChecked : _graphicUnchecked;
	baseGraphic.update();
	g_engine->drawQueue().add<AnimationDrawRequest>(baseGraphic, true, BlendMode::AdditiveAlpha);

	if (_wasClicked) {
		if (g_system->getMillis() - _clickTime > 500) {
			_wasClicked = false;
			trigger();
		}
	}
	if (_isHovered) {
		Graphic &hoverGraphic = _wasClicked ? _graphicClicked : _graphicHovered;
		hoverGraphic.update();
		g_engine->drawQueue().add<AnimationDrawRequest>(hoverGraphic, true, BlendMode::AdditiveAlpha);
	}

	// the original engine would stall the application as click delay.
	// this would prevent bacterios arm in movie adventure being rendered twice for multiple checkboxes
	// we can instead check the hovered state and prevent the arm (clicked/hovered graphic) being drawn
}

void CheckBox::loadResources() {
	_wasClicked = _isHovered = false;
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

void CheckBox::onHoverStart() {
	PhysicalObject::onHoverStart();
	_isHovered = true;
}

void CheckBox::onHoverEnd() {
	PhysicalObject::onHoverEnd();
	_isHovered = false;
}

void CheckBox::onClick() {
	_wasClicked = true;
	_clickTime = g_system->getMillis();
}

void CheckBox::trigger() {
	debug("CheckBox %d", _actionId);
}

const char *CheckBoxAutoAdjustNoise::typeName() const { return "CheckBoxAutoAdjustNoise"; }

CheckBoxAutoAdjustNoise::CheckBoxAutoAdjustNoise(Room *room, ReadStream &stream)
	: CheckBox(room, stream) {
	stream.readByte(); // unused and ignored byte
}

const char *SlideButton::typeName() const { return "SlideButton"; }

SlideButton::SlideButton(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, i1(stream.readSint32LE())
	, p1(Shape(stream).firstPoint())
	, p2(Shape(stream).firstPoint())
	, _graph1(stream)
	, _graph2(stream)
	, _graph3(stream) {
}

const char *IRCWindow::typeName() const { return "IRCWindow"; }

IRCWindow::IRCWindow(Room *room, ReadStream &stream)
	: ObjectBase(room, stream)
	, _p1(Shape(stream).firstPoint())
	, _p2(Shape(stream).firstPoint()) {
}

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
