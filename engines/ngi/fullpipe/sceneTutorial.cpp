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

#include "ngi/ngi.h"
#include "ngi/constants.h"
#include "ngi/font.h"
#include "ngi/gameloader.h"
#include "ngi/modal.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"
#include "common/formats/xmlparser.h"

namespace NGI {

namespace {

enum TutorialStep { kFirst, kMoveMan, kSeeTask, kUseTask, kUseTransition, kLast, kDone };
const char *const tutorialIds[] = { "TUTORIAL_FIRST", "TUTORIAL_MOVE_MAN", "TUTORIAL_SEE_TASK", "TUTORIAL_USE_TASK", "TUTORIAL_USE_TRANSITION", "TUTORIAL_LAST" };

int resourceId(const Common::String &name) {
	uint32 id;
	return g_nmi->_gameProject->_objectIds.findId(name, id) ? id : 0;
}

class Tutorial : public Common::XMLParser {
public:
	explicit Tutorial(const char *id) : _id(id) {}
	Common::String _text;
	Common::Point _bearded, _arrow;
	int _angle = 0, _cloud = 0, _button = 0, _left = 0, _top = 0, _right = 0, _bottom = 0;
	bool _mirror = false, _hasArrow = false;

private:
	Common::String _id;

	CUSTOM_XML_PARSER(Tutorial) {
		XML_KEY(TUTORIALS)
		KEY_END()
	} PARSER_END()

	bool parserCallback_TUTORIALS(ParserNode *) { return true; }
	bool handleUnknownKey(ParserNode *node) override {
		node->layout = _XMLkeys;
		const auto &attrs = node->values;
		if (node->name == "TUTORIAL") {
			node->ignore = attrs.getValOrDefault("id") != _id;
			if (!node->ignore)
				_text = attrs.getValOrDefault("text_id");
		} else if (!getParentNode(node) || getParentNode(node)->values.getValOrDefault("id") != _id) {
			node->ignore = true;
		} else if (node->name == "BEARDED") {
			_bearded = Common::Point(number(node, "posX"), number(node, "posY"));
			_mirror = attrs.getValOrDefault("isMirror") == "true";
		} else if (node->name == "CLOUD") {
			_cloud = number(node, "view");
			_button = attrs.getValOrDefault("isButton") == "true" ? number(node, "viewButton") : 0;
		} else if (node->name == "TEXT") {
			_left = number(node, "left_offset");
			_right = number(node, "right_offset");
			_top = number(node, "top_offset");
			_bottom = number(node, "bottom_offset");
		} else if (node->name == "ARROW") {
			_angle = number(node, "angle");
			_hasArrow = attrs.getValOrDefault("is") == "true";
			_arrow = Common::Point(number(node, "posX"), number(node, "posY"));
		} else {
			node->ignore = true;
		}
		return true;
	}

	static int number(ParserNode *node, const char *name) {
		return atoi(node->values.getValOrDefault(name).c_str());
	}
};

class ModalTutorial : public BaseModalObject {
public:
	ModalTutorial(Scene &scene, TutorialStep step) : _scene(scene), _step(step), _layout(tutorialIds[step]) {}

	bool open() {
		const Common::Path directory(Common::String("data/res_") + g_nmi->getResourceLocale());
		TextContainer strings;
		if (!_layout.loadFile("data/tutorials.xml") || !_layout.parse() || !strings.load(directory) ||
			!_font.load(directory.join("Fonts/task_dlg_19.fnt")))
			return false;
		_text = strings.getText(_layout._text);
		_cloud = _scene.findPictureObjectById(resourceId(Common::String::format("PIC_SC_TUTORIAL_CLOUD_%d", _layout._cloud)), 0);
		_man = _scene.findAniObjectById(resourceId("ANI_SC_TUTORIAL_BEARDED"), 0);
		if (!_cloud || !_man || !_scene._guiManager || _text.empty())
			return false;
		for (const char *id : {"BTN_SC_TUTORIAL_CONTINUE", "BTN_SC_TUTORIAL_OK"}) {
			if (GuiElement *element = _scene._guiManager->getElement(id))
				element->hideAndDeactivate();
		}
		_button = _layout._button ? _scene._guiManager->getElement(_layout._button == 2 ? "BTN_SC_TUTORIAL_OK" : "BTN_SC_TUTORIAL_CONTINUE") : nullptr;
		_man->setPos(_layout._bearded.x, _layout._bearded.y);
		_man->_statics = _man->getStatic(resourceId("ST_SC_TUTORIAL_BEARDED_UP"));
		_man->_flags = 4 | (_layout._mirror ? 8 : 0);
		_man->startMovement(resourceId("MOV_SC_TUTORIAL_BEARDED_FALL"), 0, -1, _layout._mirror);
		debugC(1, kDebugSceneLogic, "Started %s", tutorialIds[_step]);
		return true;
	}

	bool init(int elapsed) override {
		if (_finished)
			return false;
		_man->update(elapsed);
		if (_arrow) {
			if (!_arrow->_movement)
				_arrow->startMovement(resourceId("MV_SC_TUTORIAL_ARROW"), 0, -1);
			_arrow->update(elapsed);
		}
		if (!_man->_movement && !_shown) {
			const Dims man = _man->getFullSize();
			const Dims cloud = _cloud->getFullSize();
			_pos = Common::Point(_man->_x + (_layout._mirror ? man.x - 50 : 50 - cloud.x),
				_man->_y + man.y - cloud.y - 50);
			const int buttonY = _pos.y + (cloud.y - (_button ? _button->_size.y : 0)) / 2 + (_layout._cloud == 2 ? 40 : 50);
			if (_button) {
				_button->setPos(Common::Point(_pos.x + (cloud.x - _button->_size.x) / 2, buttonY));
				_button->showAndActivate();
			}
			_bounds = Common::Rect(_pos.x + _layout._left, _pos.y + _layout._top,
				_pos.x + cloud.x + _layout._right, buttonY + _layout._bottom);
			if (_layout._hasArrow) {
				_arrow = _scene.findAniObjectById(resourceId("ANI_SC_TUTORIAL_ARROW"), 0);
				if (_arrow) {
					_arrow->setPos(_layout._arrow.x, _layout._arrow.y);
					_arrow->_encodedRotation = _layout._angle;
					_arrow->_flags |= 4;
				}
			}
			_shown = true;
			debugC(2, kDebugSceneLogic, "Showing %s cloud %d at %d,%d", tutorialIds[_step], _layout._cloud, _pos.x, _pos.y);
		}
		return true;
	}

	void update() override {
		g_nmi->_currentScene->render();
		if (g_nmi->_inventoryScene && g_nmi->_inventoryScene->_guiManager)
			g_nmi->_inventoryScene->_guiManager->render(g_nmi->_backgroundSurface);
		if (_step == kUseTask && _parentObj)
			_parentObj->update();
		_man->render();
		if (!_shown)
			return;
		_cloud->_picture->blitTo(g_nmi->_backgroundSurface, _pos);
		_font.drawCentered(g_nmi->_backgroundSurface, _bounds, _text);
		if (_arrow)
			_arrow->render();
		_scene._guiManager->render(g_nmi->_backgroundSurface);
	}

	bool handleMessage(Command *cmd) override {
		if (cmd->_cmd != 17 || cmd->_messageNum != 30)
			return false;
		const Common::Point pos(cmd->_x, cmd->_y);
		GameVar *root = g_nmi->getGameLoaderGameVar();
		if (g_nmi->_inventoryScene) {
			GuiManager *hud = g_nmi->_inventoryScene->_guiManager.get();
			GuiElement *skip = hud ? hud->getElement("BTN_SKIP") : nullptr;
			if (skip && skip->hit(pos)) {
				hud->onMouseMove(pos);
				hud->onMouseLKeyUp(pos);
				_finished = true;
				return true;
			}
		}
		if (!_shown)
			return false;
		if (_button) {
			if (!_button->hit(pos))
				return false;
			_scene._guiManager->onMouseMove(pos);
			_scene._guiManager->onMouseLKeyUp(pos);
		} else if (_step == kMoveMan) {
			Scene &room = *g_nmi->_currentScene;
			if (!room._motionController || !room._motionController->startMove(g_nmi->_aniMan, pos.x + room._x, pos.y + room._y, 0, 0))
				return false;
		} else if (_step == kSeeTask) {
			GuiElement *task = g_nmi->_inventoryScene->_guiManager->getElement("PIC_INV_TASK");
			if (!task || !task->hit(pos))
				return false;
		} else if (_step == kUseTransition) {
			const int pipe = resourceId("PIC_SC1_RTRUBA");
			PictureObject *picture = g_nmi->_currentScene->findPictureObjectAtPos(pos.x, pos.y);
			if (!picture || picture->_id != pipe)
				return false;
			root->setSubVarAsInt("Scene1ExitsDisabled", 0);
			if (!scene01_click(pos)) {
				root->setSubVarAsInt("Scene1ExitsDisabled", 1);
				return false;
			}
		}
		root->setSubVarAsInt("TutorialStep", _step + 1);
		_finished = true;
		debugC(1, kDebugSceneLogic, "Finished %s", tutorialIds[_step]);
		return true;
	}
	bool pollEvent() override { return true; }
	void saveload() override {}

private:
	Scene &_scene;
	TutorialStep _step;
	Tutorial _layout;
	BitmapFont _font;
	Common::String _text;
	AniObject *_man = nullptr, *_arrow = nullptr;
	PictureObject *_cloud = nullptr;
	GuiElement *_button = nullptr;
	Common::Point _pos;
	Common::Rect _bounds;
	bool _finished = false, _shown = false;
};

class ModalNotebook : public BaseModalObject {
public:
	explicit ModalNotebook(Scene &scene) : _scene(scene) {}
	bool open() {
		const Common::Path directory(Common::String("data/res_") + g_nmi->getResourceLocale());
		TextContainer strings;
		if (!strings.load(directory) || !_font.load(directory.join("Fonts/task_dlg_19.fnt")))
			return false;
		_title = strings.getText("STR_FIND_BOOT_SC4_TITLE");
		return !_title.empty() && _scene._guiManager;
	}

	bool pollEvent() override { return true; }
	void saveload() override {}
	bool init(int) override { return !_closed; }
	void update() override {
		g_nmi->_currentScene->render();
		_scene.render(false);
		_font.drawCentered(g_nmi->_backgroundSurface, Common::Rect(130, 240, 380, 360), _title);
	}
	bool handleMessage(Command *cmd) override {
		if (cmd->_cmd != 17)
			return false;
		GuiElement *close = _scene._guiManager->getElement("BTN_SC_NOTEBOOK_CLOSE");
		if ((cmd->_messageNum == 30 && close && close->hit(Common::Point(cmd->_x, cmd->_y))) ||
				cmd->_messageNum == 37) {
			_closed = true;
			return true;
		}
		return false;
	}
private:
	Scene &_scene;
	BitmapFont _font;
	Common::String _title;
	bool _closed = false;
};

void showTutorial(TutorialStep step) {
	Scene *scene = g_nmi->findScene(resourceId("SC_TUTORIAL"));
	if (scene) {
		Common::ScopedPtr<ModalTutorial> dialog(new ModalTutorial(*scene, step));
		if (dialog->open()) {
			dialog->_parentObj = g_nmi->_modalObject;
			g_nmi->_modalObject = dialog.release();
			return;
		}
	}
	warning("Could not open tutorial %s", tutorialIds[step]);
	g_nmi->getGameLoaderGameVar()->setSubVarAsInt("TutorialDisabled", 1);
	g_nmi->getGameLoaderGameVar()->setSubVarAsInt("Scene1ExitsDisabled", 0);
}

} // End of anonymous namespace

void sceneNotebook_show() {
	Scene *scene = g_nmi->findScene(resourceId("SC_NOTEBOOK"));
	if (!scene)
		return;
	Common::ScopedPtr<ModalNotebook> notebook(new ModalNotebook(*scene));
	if (notebook->open()) {
		notebook->_parentObj = g_nmi->_modalObject;
		g_nmi->_modalObject = notebook.release();
		if (g_nmi->getGameLoaderGameVar()->getSubVarAsInt("TutorialStep") == kUseTask)
			showTutorial(kUseTask);
	}
}

void sceneTutorial_check() {
	const int scene = g_nmi->_currentScene->_sceneId;
	if (scene != SC_1 && scene != SC_3)
		return;
	GameVar *root = g_nmi->getGameLoaderGameVar();
	if (root->getSubVarAsInt("TutorialDisabled"))
		return;
	const int step = root->getSubVarAsInt("TutorialStep");
	if (step >= kDone || ((scene == SC_3) != (step == kLast)))
		return;
	if (!g_nmi->_aniMan || g_nmi->_aniMan->_messageQueueId || g_nmi->_aniMan->_movement)
		return;
	for (const MessageQueue *queue : *g_nmi->_globalMessageQueueList)
		if (queue->_queueId == QU_SC1_ENTER && !queue->_isFinished)
			return;
	if (step == kUseTask)
		sceneNotebook_show();
	else
		showTutorial(static_cast<TutorialStep>(step));
}

} // End of namespace NGI
