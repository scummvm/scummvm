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

#include "common/textconsole.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/game.h"
#include "tetraedge/game/objectif.h"
#include "tetraedge/te/te_vector2f32.h"
#include "tetraedge/te/te_text_layout.h"

namespace Tetraedge {

/*static*/
bool Objectif::_layoutsDirty = false;

Objectif::Objectif() : _helpButtonVisible(false) {
}

void Objectif::enter() {
	_gui1.buttonLayoutChecked("helpButton")->setVisible(true);
	_helpButtonVisible = true;
}

bool Objectif::isMouseIn(const TeVector2s32 &mousept) {
	TeLayout *bg = _gui1.layoutChecked("background");
	if (bg->visible()) {
		TeLayout *calepin = _gui1.layoutChecked("Calepin");
		if (calepin->isMouseIn(mousept))
			return true;
		// otherwise check the helpButton
	}
	TeButtonLayout *btn = _gui2.buttonLayoutChecked("helpButton");
	if (btn->visible())
		return btn->isMouseIn(mousept);
	return false;
}

void Objectif::load() {
	Application *app = g_engine->getApplication();
	_gui1.load("menus/objectif.lua");
	_gui2.load("menus/helpButton.lua");

	TeButtonLayout *btn = _gui2.buttonLayoutChecked("helpButton");
	app->frontLayout().addChild(btn);
	btn->setVisible(true);
	_helpButtonVisible = true;
	btn->onMouseClickValidated().add(this, &Objectif::onHelpButtonValidated);

	btn = _gui1.buttonLayoutChecked("helpQuit");
	btn->onMouseClickValidated().add(this, &Objectif::onHelpButtonValidated);

	_gui1.buttonLayoutChecked("background")->setVisible(false);

	_gui2.spriteLayoutChecked("newUp")->setVisible(false);
	_gui2.spriteLayoutChecked("newDown")->setVisible(false);
	_gui2.spriteLayoutChecked("notNewUp")->setVisible(true);
	_gui2.spriteLayoutChecked("notNewDown")->setVisible(true);

	_layoutsDirty = true;
}

void Objectif::leave() {
	TeLayout *layout;
	layout = _gui1.layout("background");
	if (layout)
		layout->setVisible(false);
	layout = _gui2.layout("helpButton");
	if (layout) {
		layout->setVisible(false);
		_helpButtonVisible = false;
	}
}

bool Objectif::onHelpButtonValidated() {
	if (!_helpButtonVisible) {
		_gui1.buttonLayoutChecked("background")->setVisible(false);
		_gui2.buttonLayoutChecked("helpButton")->setVisible(true);
		_helpButtonVisible = true;
	} else {
		_gui1.buttonLayoutChecked("background")->setVisible(true);
		_gui2.spriteLayoutChecked("newUp")->setVisible(false);
		_gui2.spriteLayoutChecked("newDown")->setVisible(false);
		_gui2.spriteLayoutChecked("notNewUp")->setVisible(true);
		_gui2.spriteLayoutChecked("notNewUp")->setVisible(true);
		_gui2.spriteLayoutChecked("helpButton")->setVisible(false);
		_helpButtonVisible = false;
	}
	return false;
}

void Objectif::pushObjectif(Common::String const &head, Common::String const &sub) {
	for (const Task &t : _tasks) {
		if (t._headTask == head && t._subTask == sub)
			return;
	}

	_layoutsDirty = true;
	_tasks.resize(_tasks.size() + 1);
	_tasks.back()._headTask = head;
	_tasks.back()._subTask = sub;
	_tasks.back()._taskFlag = true;
}

void Objectif::deleteObjectif(Common::String const &head, Common::String const &sub) {
	for (Task &t : _tasks) {
		if (t._taskFlag && t._headTask == head && t._subTask == sub) {
			t._taskFlag = false;
			return;
		}
	}
}

void Objectif::reattachLayout(TeLayout *layout) {
	TeButtonLayout *btn;

	btn = _gui1.buttonLayout("background");
	if (btn) {
		layout->removeChild(btn);
		layout->addChild(btn);
	}

	btn = _gui2.buttonLayout("helpButton");
	if (btn) {
		layout->removeChild(btn);
		layout->addChild(btn);
	}
}

void Objectif::removeChildren() {
	TeLayout *tasks = _gui1.layoutChecked("tasks");
	while (tasks->childCount()) {
		Te3DObject2 *child = tasks->child(0);
		TeTextLayout *text = dynamic_cast<TeTextLayout*>(child);
		tasks->removeChild(child);
		if (text)
			delete text;
	}
	_layoutsDirty = true;
}

void Objectif::update() {
	Game *game = g_engine->getGame();
	game->luaScript().execute("UpdateHelp");
	if (_layoutsDirty) {
		TeLayout *tasks = _gui1.layoutChecked("tasks");
		removeChildren();

		int last_i = -1;
		for (uint i = 0; i < _tasks.size(); i++) {
			if (!_tasks[i]._taskFlag)
				continue;
			if (last_i != -1 && _tasks[i]._headTask == _tasks[last_i]._headTask)
				continue;
			last_i = i;
			createChildLayout(tasks, _tasks[i]._headTask, false);
			// Creating the subtasks for this head
			for (uint j = 0; j < _tasks.size(); j++) {
				if (_tasks[j]._taskFlag && _tasks[j]._headTask == _tasks[i]._headTask && _tasks[j]._subTask != "")
					createChildLayout(tasks, _tasks[j]._subTask, true);
			}
		}

		float z = 0.1f;
		for (Te3DObject2 *child : tasks->childList()) {
			TeTextLayout *text = dynamic_cast<TeTextLayout *>(child);
			/*TeVector3f32 size =*/
			text->size();
			TeVector3f32 userPos = text->userPosition();
			userPos.z() = z;
			text->setPosition(userPos);
			z += text->userSize().y();
		}
	}
	_layoutsDirty = false;
}

void Objectif::createChildLayout(TeLayout *layout, Common::String const &taskId, bool isSubTask) {
	TeTextLayout *text = new TeTextLayout();
	text->setName(taskId);
	text->setAnchor(TeVector3f32(0.0f, 0.0f, 0.0f));
	text->setPositionType(TeILayout::RELATIVE_TO_PARENT);
	text->setSizeType(TeILayout::RELATIVE_TO_PARENT);
	Application *app = g_engine->getApplication();
	// No help at difficulty 2.
	if (app->difficulty() != 2) {
		Common::String textVal;
		if (!isSubTask) {
			text->setSize(TeVector3f32(0.8f, 1.0f, 0.1f));
			text->setPosition(TeVector3f32(0.1f, 0.0f, 0.1f));
			textVal = "<section style=\"left\" /><color r=\"39\" g=\"85\" b=\"97\"/><font file=\"Common/Fonts/ComicRelief.ttf\" size=\"12\"/>";
		} else {
			text->setSize(TeVector3f32(0.75f, 1.0f, 0.1f));
			text->setPosition(TeVector3f32(0.15f, 0.0f, 0.1f));
			if (app->difficulty() == 0) {
				textVal = "<section style=\"left\" /><color r=\"0\" g=\"0\" b=\"0\"/><font file=\"Common/Fonts/ComicRelief.ttf\" size=\"12\"/>\t";
			} else {
				textVal = "<section style=\"left\" /><color r=\"0\" g=\"0\" b=\"0\"/><font file=\"Common/Fonts/arial.ttf\" size=\"16\"/>";
			}
		}
		textVal += app->getHelpText(taskId);
		text->setText(textVal);
	}

	layout->addChild(text);
}


void Objectif::unload() {
	removeChildren();
	leave();

	Application *app = g_engine->getApplication();
	TeButtonLayout *btn = _gui2.buttonLayoutChecked("helpButton");
	app->frontLayout().removeChild(btn);
	btn = _gui1.buttonLayoutChecked("background");
	app->frontLayout().removeChild(btn);

	_gui1.unload();
	_gui2.unload();
	_tasks.clear();
}

void Objectif::setVisibleButtonHelp(bool visible) {
	_gui2.buttonLayoutChecked("helpButton")->setVisible(visible);
	_helpButtonVisible = visible;
}

void Objectif::setVisibleObjectif(bool visible) {
	_gui1.buttonLayoutChecked("background")->setVisible(visible);
}

} // end namespace Tetraedge
