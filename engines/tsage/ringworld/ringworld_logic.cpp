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

#include "common/config-manager.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld/ringworld_demo.h"
#include "tsage/ringworld/ringworld_dialogs.h"
#include "tsage/ringworld/ringworld_scenes1.h"
#include "tsage/ringworld/ringworld_scenes2.h"
#include "tsage/ringworld/ringworld_scenes3.h"
#include "tsage/ringworld/ringworld_scenes4.h"
#include "tsage/ringworld/ringworld_scenes5.h"
#include "tsage/ringworld/ringworld_scenes6.h"
#include "tsage/ringworld/ringworld_scenes8.h"
#include "tsage/ringworld/ringworld_scenes10.h"

namespace TsAGE {

namespace Ringworld {

Scene *RingworldGame::createScene(int sceneNumber) {
	switch (sceneNumber) {
	/* Scene group 1 */
	// Kziniti Palace (Introduction)
	case 10: return new Scene10();
	// Outer Space (Introduction)
	case 15: return new Scene15();
	// Cut-scenes for Ch'mee house in distance
	case 20: return new Scene20();
	// Outside Ch'mee residence
	case 30: return new Scene30();
	// Chmeee Home
	case 40: return new Scene40();
	// By Flycycles
	case 50: return new Scene50();
	// Flycycle controls
	case 60: return new Scene60();
	// Shipyard Entrance
	case 90: return new Scene90();
	// Ship Close-up
	case 95: return new Scene95();
	// Sunflower navigation sequence
	case 6100: return new Scene6100();

	/* Scene group 2 */
	// Title screen
	case 1000: return new Scene1000();
	// Fleeing planet cutscene
	case 1001: return new Scene1001();
	// Unused
	case 1250: return new Scene1250();
	// Ringworld Wall
	case 1400: return new Scene1400();
	// Ringworld Space-port
	case 1500: return new Scene1500();

	/* Scene group 3 - Part #1 */
	// Cockpit cutscenes
	case 2000: return new Scene2000();
	// Starcraft - Cockpit
	case 2100: return new Scene2100();
	// Encyclopedia
	case 2120: return new Scene2120();
	// Starcraft - Level 2
	case 2150: return new Scene2150();
	// Starcraft - AutoDoc
	case 2200: return new Scene2200();
	// Stasis Field Map
	case 2222: return new Scene2222();
	// Starcraft - Quinn's Room
	case 2230: return new Scene2230();

	/* Scene group 3 - Part #2 */
	// Starcraft - Storage Room
	case 2280: return new Scene2280();
	// Starcraft - Hanger Bay
	case 2300: return new Scene2300();
	// Starcraft - Copy Protection Screen
	case 2310: return new Scene2310();
	// Starcraft - Lander Bay
	case 2320: return new Scene2320();
	// Scene 2400 - Descending in Lander
	case 2400: return new Scene2400();

	/* Scene group 4 */
	// Ringworld Scan
	case 3500: return new Scene3500();
	// Remote Viewer
	case 3700: return new Scene3700();

	/* Scene group 5 */
	// Village
	case 4000: return new Scene4000();
	// Village - Outside Lander
	case 4010: return new Scene4010();
	// Village - Puzzle Board
	case 4025: return new Scene4025();
	// Village - Temple Antechamber
	case 4045: return new Scene4045();
	// Village - Temple
	case 4050: return new Scene4050();
	// Village - Hut
	case 4100: return new Scene4100();
	// Village - Bedroom
	case 4150: return new Scene4150();
	// Village - Near Slaver Ship
	case 4250: return new Scene4250();
	// Village - Slaver Ship
	case 4300: return new Scene4300();
	// Village - Slaver Ship Keypad
	case 4301: return new Scene4301();

	/* Scene group 6 */
	// Caverns - Entrance
	case 5000: return new Scene5000();
	// Caverns
	case 5100: return new Scene5100();
	// Caverns - Throne-room
	case 5200: return new Scene5200();
	// Caverns - Pit
	case 5300: return new Scene5300();

	/* Scene group 8 */
	// Landing near beach
	case 7000: return new Scene7000();
	// Underwater: swimming
	case 7100: return new Scene7100();
	// Underwater: Entering the cave
	case 7200: return new Scene7200();
	// Underwater: Lord Poria
	case 7300: return new Scene7300();
	// Floating Buildings: Outside
	case 7600: return new Scene7600();
	// Floating Buildings: In the lab
	case 7700: return new Scene7700();

	/* Scene group 10 */
	// Near beach: Slave washing clothes
	case 9100: return new Scene9100();
	// Castle: Outside the bulwarks
	case 9150: return new Scene9150();
	// Castle: Near the fountain
	case 9200: return new Scene9200();
	// Castle: In front of a large guarded door
	case 9300: return new Scene9300();
	// Castle: In a hallway
	case 9350: return new Scene9350();
	// Castle: In a hallway
	case 9360: return new Scene9360();
	// Castle: Black-Smith room
	case 9400: return new Scene9400();
	// Castle: Dining room
	case 9450: return new Scene9450();
	// Castle: Bedroom
	case 9500: return new Scene9500();
	// Castle: Balcony
	case 9700: return new Scene9700();
	// Castle: In the garden
	case 9750: return new Scene9750();
	// Castle: Dressing room
	case 9850: return new Scene9850();
	// Ending
	case 9900: return new Scene9900();
	// Space travel
	case 9999: return new Scene9999();

	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool RingworldGame::canLoadGameStateCurrently() {
	// Don't allow a game to be loaded if a dialog is active
	return !g_globals->getFlag(50) && (g_globals->_gfxManagers.size() == 1);
}

/**
 * Returns true if it is currently okay to save the game
 */
bool RingworldGame::canSaveGameStateCurrently() {
	// Don't allow a game to be saved if a dialog is active, or the copy protection dialog
	return !g_globals->getFlag(50) && (g_globals->_gfxManagers.size() == 1) && g_globals->_sceneManager._sceneNumber != 2310;
}

/*--------------------------------------------------------------------------*/

DisplayHotspot::DisplayHotspot(int regionId, ...) {
	_sceneRegionId = regionId;

	// Load up the actions
	va_list va;
	va_start(va, regionId);

	int param = va_arg(va, int);
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayHotspot::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

DisplayObject::DisplayObject(int firstAction, ...) {
	// Load up the actions
	va_list va;
	va_start(va, firstAction);

	int param = firstAction;
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayObject::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------*/

SceneArea::SceneArea() {
	_savedArea = NULL;
	_pt.x = _pt.y = 0;

	_resNum = 0;
	_rlbNum = 0;
	_subNum = 0;
	_actionId = 0;
}

SceneArea::~SceneArea() {
	delete _savedArea;
}

void SceneArea::setup(int resNum, int rlbNum, int subNum, int actionId) {
	_resNum = resNum;
	_rlbNum = rlbNum;
	_subNum = subNum;
	_actionId = actionId;

	_surface = surfaceFromRes(resNum, rlbNum, subNum);
}

void SceneArea::draw2() {
	_surface.draw(Common::Point(_bounds.left, _bounds.top));
}

void SceneArea::display() {
	_bounds.left = _pt.x - (_surface.getBounds().width() / 2);
	_bounds.top = _pt.y + 1 - _surface.getBounds().height();
	_bounds.setWidth(_surface.getBounds().width());
	_bounds.setHeight(_surface.getBounds().height());

	_savedArea = surfaceGetArea(g_globals->_gfxManagerInstance.getSurface(), _bounds);
	draw2();
}

void SceneArea::restore() {
	assert(_savedArea);
	_savedArea->draw(Common::Point(_bounds.left, _bounds.top));
	delete _savedArea;
	_savedArea = NULL;
}

void SceneArea::draw(bool flag) {
	_surface = surfaceFromRes(_resNum, _rlbNum, flag ? _subNum + 1 : _subNum);
	_surface.draw(Common::Point(_bounds.left, _bounds.top));
}

void SceneArea::wait() {
	// Wait until a mouse or keypress
	Event event;
	while (!g_vm->shouldQuit() && !g_globals->_events.getEvent(event)) {
		GLOBALS._screen.update();
		g_system->delayMillis(10);
	}

	SynchronizedList<SceneItem *>::iterator ii;
	for (ii = g_globals->_sceneItems.begin(); ii != g_globals->_sceneItems.end(); ++ii) {
		SceneItem *sceneItem = *ii;
		if (sceneItem->contains(event.mousePos)) {
			sceneItem->doAction(_actionId);
			break;
		}
	}

	g_globals->_events.setCursor(CURSOR_ARROW);
}

void SceneArea::synchronize(Serializer &s) {
	if (s.getVersion() >= 2)
		SavedObject::synchronize(s);

	s.syncAsSint16LE(_pt.x);
	s.syncAsSint16LE(_pt.y);
	s.syncAsSint32LE(_resNum);
	s.syncAsSint32LE(_rlbNum);
	s.syncAsSint32LE(_subNum);
	s.syncAsSint32LE(_actionId);
	_bounds.synchronize(s);
}

/*--------------------------------------------------------------------------*/

RingworldInvObjectList::RingworldInvObjectList() :
		_ESP(g_vm->getLanguage() == Common::ES_ESP),
		_RUS(g_vm->getLanguage() == Common::RU_RUS),
		_stunner(2280, 1, 2, OBJECT_STUNNER, _ESP ? "Tu paralizador." : _RUS ? "\x82\xA0\xE8 \xE1\xE2\xA0\xAD\xAD\xA5\xE0." : "This is your stunner."),  // Ваш станнер.
		_scanner(1, 1, 3, OBJECT_SCANNER, _ESP ? "Una unidad combinada de esc\240ner y comunicaciones." : _RUS ? "\x91\xAA\xA0\xAD\xA5\xE0\x2D\xAA\xAE\xAC\xAC\xE3\xAD\xA8\xAA\xA0\xE2\xAE\xE0." : "A combination scanner comm unit."),  // Сканер-коммуникатор.
		_stasisBox(5200, 1, 4, OBJECT_STASIS_BOX, _ESP ? "Una caja est\240sica." : _RUS ? "\x8A\xAE\xAD\xE2\xA5\xA9\xAD\xA5\xE0." : "A stasis box."),  // Контейнер.
		_infoDisk(40, 1, 1, OBJECT_INFODISK, _ESP ? "El infodisk que le cogiste al asesino." : _RUS ? "\x88\xA7\xEA\xEF\xE2\xEB\xA9 \xE3 \xAA\xA8\xAB\xAB\xA5\xE0\xA0 \xA8\xAD\xE4\xAE\xA4\xA8\xE1\xAA." : "The infodisk you took from the assassin."),  // Изъятый у киллера инфодиск.
		_stasisNegator(0, 2, 2, OBJECT_STASIS_NEGATOR, _ESP ? "El negador de campos est\240sicos." : _RUS ? "\x88\xAD\xA2\xA5\xE0\xE2\xAE\xE0 \xE1\xE2\xA0\xA7\xA8\xE1\x2D\xAF\xAE\xAB\xEF." : "The stasis field negator."),  // Инвертор стазис-поля.
		_keyDevice(4250, 1, 6, OBJECT_KEY_DEVICE, _ESP ? "Una llave magn\202tica." : _RUS ? "\x8C\xA0\xA3\xAD\xA8\xE2\xAD\xEB\xA9 \xAA\xAB\xEE\xE7." : "A magnetic key device."),  // Магнитный ключ.
		_medkit(2280, 1, 7, OBJECT_MEDKIT, _ESP ? "Tu botiqu\241n." : _RUS ? "\x80\xAF\xE2\xA5\xE7\xAA\xA0." : "Your medkit."),  // Аптечка.
		_ladder(4100, 1, 8, OBJECT_LADDER, _ESP ? "La escalera del jefe." : _RUS ? "\x8B\xA5\xE1\xE2\xAD\xA8\xE6\xA0 \xA2\xAE\xA6\xA4\xEF." : "The chief's ladder."),  // Лестница вождя.
		_rope(4150, 1, 9, OBJECT_ROPE, _ESP ? "La cuerda del jefe." : _RUS ? "\x82\xA5\xE0\xF1\xA2\xAA\xA0 \xA2\xAE\xA6\xA4\xEF." : "The chief's rope."),  // Верёвка вождя.
		_key(7700, 1, 11, OBJECT_KEY, _ESP ? "Una llave." : _RUS ? "\x8A\xAB\xEE\xE7." : "A key."),  // Ключ.
		_translator(7700, 1, 13, OBJECT_TRANSLATOR,  _ESP ? "La caja traductora delfiniana." : _RUS ? "\x8F\xA5\xE0\xA5\xA2\xAE\xA4\xE7\xA8\xAA \xE1 \xA4\xA5\xAB\xEC\xE4\xA8\xAD\xEC\xA5\xA3\xAE." : "The dolphin translator box."),  // Переводчик с дельфиньего.
		_ale(2150, 1, 10, OBJECT_ALE, _ESP ? "Una botella de cerveza." : _RUS ? "\x81\xE3\xE2\xEB\xAB\xAA\xA0 \xED\xAB\xEF." : "A bottle of ale."),  // Бутылка эля.
		_paper(7700, 1, 12, OBJECT_PAPER, _ESP ? "Un trozo de papel con los n\243meros 2,4, y 3 escritos en \202l." : _RUS ? "\x8D\xA0 \xAA\xAB\xAE\xE7\xAA\xA5 \xA1\xE3\xAC\xA0\xA3\xA8 \xAD\xA0\xE7\xA5\xE0\xE2\xA0\xAD\xEB \xE6\xA8\xE4\xE0\xEB 2, 4, 3." : "A slip of paper with the numbers 2,4, and 3 written on it."),  // На клочке бумаги начертаны цифры 2, 4, 3.
		_waldos(0, 1, 14, OBJECT_WALDOS, _ESP ? "Un par de brazos de la sonda averiada." : _RUS ? "\x8C\xA0\xAD\xA8\xAF\xE3\xAB\xEF\xE2\xAE\xE0\xEB \xE1\xAE \xE1\xAB\xAE\xAC\xA0\xAD\xAD\xAE\xA3\xAE \xA7\xAE\xAD\xA4\xA0." : "A pair of waldos from the ruined probe."),  // Манипуляторы со сломанного зонда.
		_stasisBox2(8100, 1, 4, OBJECT_STASIS_BOX2, _ESP ? "Una caja est\240sica." : _RUS ? "\x8A\xAE\xAD\xE2\xA5\xA9\xAD\xA5\xE0." : "A stasis box."),  // Контейнер.
		_ring(8100, 2, 5, OBJECT_RING, _ESP ? "El anillo que te envi\242 Louis Wu." : _RUS ? "\x9D\xE2\xAE\xE2 \xAF\xA5\xE0\xE1\xE2\xA5\xAD\xEC \xA2\xEB \xAF\xAE\xAB\xE3\xE7\xA8\xAB\xA8 \xAE\xE2 \x8B\xE3\xA8\xE1\xA0 \x82\xE3." : "This is a signet ring sent to you by Louis Wu."),  // Этот перстень вы получили от Луиса Ву.
		_cloak(9850, 2, 6, OBJECT_CLOAK, _ESP ? "Una t\243nica de seda fina." : _RUS ? "\x98\xF1\xAB\xAA\xAE\xA2\xA0\xEF \xE2\xE3\xAD\xA8\xAA\xA0." : "A fine silk cloak."),  // Шёлковая туника.
		_tunic(9450, 2, 7, OBJECT_TUNIC, _ESP ? "La t\243nica manchada del patriarca." : _RUS ? "\x83\xE0\xEF\xA7\xAD\xA0\xEF \xE2\xE3\xAD\xA8\xAA\xA0 \x8F\xA0\xE2\xE0\xA8\xA0\xE0\xE5\xA0." : "The patriarch's soiled tunic."),  // Грязная туника Патриарха.
		_candle(9500, 2, 8, OBJECT_CANDLE, _ESP ? "Una vela de sebo." : _RUS ? "\x91\xA0\xAB\xEC\xAD\xA0\xEF \xE1\xA2\xA5\xE7\xA0." : "A tallow candle."),  // Сальная свеча.
		_straw(9400, 2, 9, OBJECT_STRAW, _ESP ? "Paja limpia y seca." : _RUS ? "\x91\xE3\xE5\xA0\xEF \xA8 \xE7\xA8\xE1\xE2\xA0\xEF." : "Clean, dry straw."),  // Сухая и чистая.
		_scimitar(9850, 1, 18, OBJECT_SCIMITAR, _ESP ? "La cimitarra del armario del Patriarca." : _RUS ? "\x91\xAA\xA8\xAC\xA8\xE2\xA0\xE0 \xA8\xA7 \xE8\xAA\xA0\xE4\xA0 \x8F\xA0\xE2\xE0\xA8\xA0\xE0\xE5\xA0." : "A scimitar from the Patriarch's closet."),  // Скимитар из шкафа Патриарха.
		_sword(9850, 1, 17, OBJECT_SWORD, _ESP ? "La espada corta del armario del Patriarca." : _RUS ? "\x8A\xAE\xE0\xAE\xE2\xAA\xA8\xA9 \xAC\xA5\xE7 \xA8\xA7 \xE8\xAA\xA0\xE4\xA0 \x8F\xA0\xE2\xE0\xA8\xA0\xE0\xE5\xA0." : "A short sword from the Patriarch's closet."),  // Короткий меч из шкафа Патриарха.
		_helmet(9500, 2, 4, OBJECT_HELMET, _ESP ? "Un extra\244o yelmo." : _RUS ? "\x91\xE2\xE0\xA0\xAD\xAD\xEB\xA9 \xE8\xAB\xA5\xAC." : "Some type of helmet."),  // Странный шлем.
		_items(4300, 2, 10, OBJECT_ITEMS, _ESP ? "Dos interesantes objetos de la nave Tnuctipun." : _RUS ? "\x8B\xEE\xA1\xAE\xAF\xEB\xE2\xAD\xEB\xA5 \xA2\xA5\xE9\xA8\xE6\xEB \xE1 \xAA\xAE\xE0\xA0\xA1\xAB\xEF \xE2\xAD\xE3\xAA\xE2\xA8\xAF\xE3\xAD\xAE\xA2." : "Two interesting items from the Tnuctipun vessel."),  // Любопытные вещицы с корабля тнуктипунов.
		_concentrator(4300, 2, 11, OBJECT_CONCENTRATOR, _ESP ? "El concentrador antimateria Tnuctipun contenido en un campo est\240sico." : _RUS ? "\x8A\xAE\xAD\xE6\xA5\xAD\xE2\xE0\xA0\xE2\xAE\xE0 \xA0\xAD\xE2\xA8\xA2\xA5\xE9\xA5\xE1\xE2\xA2\xA0 \xA2 \xE1\xE2\xA0\xA7\xA8\xE1\xAD\xAE\xAC \xAF\xAE\xAB\xA5." : "The Tnuctipun anti-matter concentrator contained in a stasis field."),  // Концентратор антивещества в стазисном поле.
		_nullifier(5200, 2, 12, OBJECT_NULLIFIER, _ESP ? "Un anulador de ondas neuronales." : _RUS ? "\x82\xE0\xAE\xA4\xA5 \xA1\xEB \xAD\xA5\xA2\xE0\xA0\xAB\xEC\xAD\xEB\xA9 \xAD\xA5\xA9\xE2\xE0\xA0\xAB\xA8\xA7\xA0\xE2\xAE\xE0." : "A purported neural wave nullifier."),  // Вроде бы невральный нейтрализатор.
		_peg(4045, 2, 16, OBJECT_PEG, _ESP ? "Una clavija con un s\241mbolo." : _RUS ? "\x8A\xAE\xAB\xEB\xE8\xA5\xAA \xE1 \xE1\xA8\xAC\xA2\xAE\xAB\xAE\xAC." : "A peg with a symbol."),  // Колышек с символом.
		_vial(5100, 2, 17, OBJECT_VIAL, _ESP ? "Un frasco con la droga antiferomonas de los murci\202lagos." : _RUS ? "\x8F\xE3\xA7\xEB\xE0\xF1\xAA \xE1 \xA0\xAD\xE2\xA8\xE4\xA5\xE0\xAE\xAC\xAE\xAD\xAE\xAC." : "A vial of the bat creatures anti-pheromone drug."),  // Пузырёк с антиферомоном.
		_jacket(9850, 3, 1, OBJECT_JACKET, _ESP ? "Una elegante chaqueta." : _RUS ? "\x93\xE2\xA5\xAF\xAB\xF1\xAD\xAD\xA0\xEF \xE2\xE3\xAD\xA8\xAA\xA0." : "A natty padded jacket."),  // Утеплённая туника.
		_tunic2(9850, 3, 2, OBJECT_TUNIC2, _ESP ? "Una t\243nica muy ligera." : _RUS ? "\x8F\xE3\xE8\xA8\xE1\xE2\xA0\xEF \xE2\xE3\xAD\xA8\xAA\xA0." : "A very hairy tunic."),  // Пушистая туника.
		_bone(5300, 3, 5, OBJECT_BONE, _ESP ? "Un hueso muy afilado." : _RUS ? "\x82\xAE\xE1\xE2\xE0\xA0\xEF \xAA\xAE\xE1\xE2\xEC." : "A very sharp bone."),  // Вострая кость.
		_jar(7700, 3, 4, OBJECT_JAR, _ESP ? "Un frasco lleno de una sustancia verde." : _RUS ? "\x91\xAE\xE1\xE3\xA4 \xE1 \xA7\xA5\xAB\xF1\xAD\xAE\xA9 \xE1\xE3\xA1\xE1\xE2\xA0\xAD\xE6\xA8\xA5\xA9." : "An jar filled with a green substance."),  // Сосуд с зелёной субстанцией.
		_emptyJar(7700, 3, 3, OBJECT_EMPTY_JAR, _ESP ? "Un frasco vac\241o." : _RUS ? "\x8F\xE3\xE1\xE2\xAE\xA9 \xE1\xAE\xE1\xE3\xA4." : "An empty jar.") {  // Пустой сосуд.

	// Add the items to the list
	_itemList.push_back(&_stunner);
	_itemList.push_back(&_scanner);
	_itemList.push_back(&_stasisBox);
	_itemList.push_back(&_infoDisk);
	_itemList.push_back(&_stasisNegator);
	_itemList.push_back(&_keyDevice);
	_itemList.push_back(&_medkit);
	_itemList.push_back(&_ladder);
	_itemList.push_back(&_rope);
	_itemList.push_back(&_key);
	_itemList.push_back(&_translator);
	_itemList.push_back(&_ale);
	_itemList.push_back(&_paper);
	_itemList.push_back(&_waldos);
	_itemList.push_back(&_stasisBox2);
	_itemList.push_back(&_ring);
	_itemList.push_back(&_cloak);
	_itemList.push_back(&_tunic);
	_itemList.push_back(&_candle);
	_itemList.push_back(&_straw);
	_itemList.push_back(&_scimitar);
	_itemList.push_back(&_sword);
	_itemList.push_back(&_helmet);
	_itemList.push_back(&_items);
	_itemList.push_back(&_concentrator);
	_itemList.push_back(&_nullifier);
	_itemList.push_back(&_peg);
	_itemList.push_back(&_vial);
	_itemList.push_back(&_jacket);
	_itemList.push_back(&_tunic2);
	_itemList.push_back(&_bone);
	_itemList.push_back(&_jar);
	_itemList.push_back(&_emptyJar);

	_selectedItem = NULL;
}

/*--------------------------------------------------------------------------*/

void RingworldGame::start() {
	// Set some default flags
	g_globals->setFlag(12);
	g_globals->setFlag(34);

	// Set the screen to scroll in response to the player moving off-screen
	g_globals->_scrollFollower = &g_globals->_player;

	// Set the object's that will be in the player's inventory by default
	RING_INVENTORY._stunner._sceneNumber = 1;
	RING_INVENTORY._scanner._sceneNumber = 1;
	RING_INVENTORY._ring._sceneNumber = 1;

	int slot = -1;

	if (ConfMan.hasKey("save_slot")) {
		slot = ConfMan.getInt("save_slot");
		Common::String file = g_vm->getSaveStateName(slot);
		Common::InSaveFile *in = g_vm->_system->getSavefileManager()->openForLoading(file);
		if (in)
			delete in;
		else
			slot = -1;
	}

	if (slot >= 0)
		g_globals->_sceneHandler->_loadGameSlot = slot;
	else
		// Switch to the title screen
		g_globals->_sceneManager.setNewScene(1000);

	g_globals->_events.showCursor();
}

void RingworldGame::restart() {
	g_globals->_scenePalette.clearListeners();
	g_globals->_soundHandler.stop();

	// Reset the flags
	g_globals->reset();
	g_globals->setFlag(34);

	// Clear save/load slots
	g_globals->_sceneHandler->_saveGameSlot = -1;
	g_globals->_sceneHandler->_loadGameSlot = -1;

	g_globals->_stripNum = 0;
	g_globals->_events.setCursor(CURSOR_WALK);

	// Reset item properties
	RING_INVENTORY._stunner._sceneNumber = 1;
	RING_INVENTORY._scanner._sceneNumber = 1;
	RING_INVENTORY._stasisBox._sceneNumber = 5200;
	RING_INVENTORY._infoDisk._sceneNumber = 40;
	RING_INVENTORY._stasisNegator._sceneNumber = 0;
	RING_INVENTORY._keyDevice._sceneNumber = 0;
	RING_INVENTORY._medkit._sceneNumber = 2280;
	RING_INVENTORY._ladder._sceneNumber = 4100;
	RING_INVENTORY._rope._sceneNumber = 4150;
	RING_INVENTORY._key._sceneNumber = 7700;
	RING_INVENTORY._translator._sceneNumber = 2150;
	RING_INVENTORY._paper._sceneNumber = 7700;
	RING_INVENTORY._waldos._sceneNumber = 0;
	RING_INVENTORY._ring._sceneNumber = 1;
	RING_INVENTORY._stasisBox2._sceneNumber = 8100;
	RING_INVENTORY._cloak._sceneNumber = 9850;
	RING_INVENTORY._tunic._sceneNumber = 9450;
	RING_INVENTORY._candle._sceneNumber = 9500;
	RING_INVENTORY._straw._sceneNumber = 9400;
	RING_INVENTORY._scimitar._sceneNumber = 9850;
	RING_INVENTORY._sword._sceneNumber = 9850;
	RING_INVENTORY._helmet._sceneNumber = 9500;
	RING_INVENTORY._items._sceneNumber = 4300;
	RING_INVENTORY._concentrator._sceneNumber = 4300;
	RING_INVENTORY._nullifier._sceneNumber = 4300;
	RING_INVENTORY._peg._sceneNumber = 4045;
	RING_INVENTORY._vial._sceneNumber = 5100;
	RING_INVENTORY._jacket._sceneNumber = 9850;
	RING_INVENTORY._tunic2._sceneNumber = 9850;
	RING_INVENTORY._bone._sceneNumber = 5300;
	RING_INVENTORY._jar._sceneNumber = 7700;
	RING_INVENTORY._emptyJar._sceneNumber = 7700;
	RING_INVENTORY._selectedItem = NULL;

	// Change to the first game scene
	g_globals->_sceneManager.changeScene(30);
}

void RingworldGame::endGame(int resNum, int lineNum) {
	g_globals->_events.setCursor(CURSOR_WALK);
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum);
	bool savesExist = g_saver->savegamesExist();

	if (!savesExist) {
		// No savegames exist, so prompt the user to restart or quit
		int rc;
		if (g_vm->getLanguage() == Common::ES_ESP) {
			rc = MessageDialog::show(msg, ESP_QUIT_BTN_STRING, ESP_RESTART_BTN_2_STRING);
		} else if (g_vm->getLanguage() == Common::RU_RUS) {
			rc = MessageDialog::show(msg, RUS_QUIT_BTN_STRING, RUS_RESTART_BTN_STRING);
		} else {
			rc = MessageDialog::show(msg, QUIT_BTN_STRING, RESTART_BTN_STRING);
		}
		if (rc == 0)
			g_vm->quitGame();
		else
			restart();
	} else {
		// Savegames exist, so prompt for Restore/Restart
		bool breakFlag;
		do {
			if (g_vm->shouldQuit()) {
				breakFlag = true;
			} else {
				int rc;
				if (g_vm->getLanguage() == Common::ES_ESP) {
					rc = MessageDialog::show(msg, ESP_RESTART_BTN_2_STRING, ESP_RESTORE_BTN_STRING);
				} else if (g_vm->getLanguage() == Common::RU_RUS) {
					rc = MessageDialog::show(msg, RUS_RESTART_BTN_STRING, RUS_RESTORE_BTN_STRING);
				} else {
					rc = MessageDialog::show(msg, RESTART_BTN_STRING, RESTORE_BTN_STRING);
				}
				if (rc== 0) {
					restart();
					breakFlag = true;
				} else {
					handleSaveLoad(false, g_globals->_sceneHandler->_loadGameSlot, g_globals->_sceneHandler->_saveName);
					breakFlag = g_globals->_sceneHandler->_loadGameSlot >= 0;
				}
			}
		} while (!breakFlag);
	}

	g_globals->_events.setCursorFromFlag();
}

void RingworldGame::processEvent(Event &event) {
	if (event.eventType == EVENT_CUSTOM_ACTIONSTART) {
		switch (event.customType) {
		case kActionHelp:
			// F1 - Help
			if (g_vm->getLanguage() == Common::ES_ESP) {
				MessageDialog::show(ESP_HELP_MSG, ESP_OK_BTN_STRING);
			} else if (g_vm->getLanguage() == Common::RU_RUS) {
				MessageDialog::show(RUS_HELP_MSG, RUS_OK_BTN_STRING);
			} else {
				MessageDialog::show(HELP_MSG, OK_BTN_STRING);
			}
			break;

		case kActionSoundOptions:
			// F2 - Sound Options
			SoundDialog::execute();
			break;

		case kActionQuitGame:
			// F3 - Quit
			quitGame();
			event.handled = false;
			break;

		case kActionRestartGame:
			// F4 - Restart
			restartGame();
			g_globals->_events.setCursorFromFlag();
			break;

		case kActionRestoreGame:
			// F7 - Restore
			restoreGame();
			g_globals->_events.setCursorFromFlag();
			break;

		case kActionPauseGame:
			// F10 - Pause
			GfxDialog::setPalette();
			if (g_vm->getLanguage() == Common::ES_ESP) {
				MessageDialog::show(ESP_GAME_PAUSED_MSG, ESP_CONTINUE_BTN_STRING);
			} else if (g_vm->getLanguage() == Common::RU_RUS) {
				MessageDialog::show(RUS_GAME_PAUSED_MSG, RUS_CONTINUE_BTN_STRING);
			} else {
				MessageDialog::show(GAME_PAUSED_MSG, CONTINUE_BTN_STRING);
			}
			g_globals->_events.setCursorFromFlag();
			break;

		default:
			break;
		}
	}
}

void RingworldGame::rightClick() {
	RightClickDialog *dlg = new RightClickDialog();
	dlg->execute();
	delete dlg;
}

/*--------------------------------------------------------------------------*/

NamedHotspot::NamedHotspot() : SceneHotspot() {
	_resNum = 0;
	_lookLineNum = _useLineNum = _talkLineNum = -1;
}

void NamedHotspot::doAction(int action) {
	switch (action) {
	case CURSOR_WALK:
		// Nothing
		return;
	case CURSOR_LOOK:
		if (_lookLineNum == -1)
			break;

		SceneItem::display(_resNum, _lookLineNum, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		return;
	case CURSOR_USE:
		if (_useLineNum == -1)
			break;

		SceneItem::display(_resNum, _useLineNum, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		return;
	case CURSOR_TALK:
		if (_talkLineNum == -1)
			break;

		SceneItem::display(_resNum, _lookLineNum, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOR, 7, LIST_END);
		return;
	default:
		break;
	}

	SceneHotspot::doAction(action);
}

void NamedHotspot::synchronize(Serializer &s) {
	SceneHotspot::synchronize(s);
	s.syncAsSint16LE(_resNum);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_useLineNum);

	if (g_vm->getGameID() == GType_BlueForce)
		s.syncAsSint16LE(_talkLineNum);
}


} // End of namespace Ringworld

} // End of namespace TsAGE
