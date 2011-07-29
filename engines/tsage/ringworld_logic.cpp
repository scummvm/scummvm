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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/translation.h"
#include "gui/saveload.h"
#include "tsage/ringworld_logic.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld_demo.h"
#include "tsage/ringworld_scenes1.h"
#include "tsage/ringworld_scenes2.h"
#include "tsage/ringworld_scenes3.h"
#include "tsage/ringworld_scenes4.h"
#include "tsage/ringworld_scenes5.h"
#include "tsage/ringworld_scenes6.h"
#include "tsage/ringworld_scenes8.h"
#include "tsage/ringworld_scenes10.h"

namespace tSage {

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

	_savedArea = Surface_getArea(_globals->_gfxManagerInstance.getSurface(), _bounds);
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
	while (!_vm->shouldQuit() && !_globals->_events.getEvent(event)) {
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	SynchronizedList<SceneItem *>::iterator ii;
	for (ii = _globals->_sceneItems.begin(); ii != _globals->_sceneItems.end(); ++ii) {
		SceneItem *sceneItem = *ii;
		if (sceneItem->contains(event.mousePos)) {
			sceneItem->doAction(_actionId);
			break;
		}
	}

	_globals->_events.setCursor(CURSOR_ARROW);
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

SpeakerGText::SpeakerGText() {
	_speakerName = "GTEXT";
	_textWidth = 160;
	_textPos = Common::Point(130, 10);
	_color1 = 42;
	_hideObjects = false;
}

void SpeakerGText::setText(const Common::String &msg) {
	// Set the animation properties
	_sceneObject.postInit();
	_sceneObject.setVisage(9405);
	_sceneObject.setStrip2(3);
	_sceneObject.fixPriority(255);
	_sceneObject.changeZoom(100);
	_sceneObject._frame = 1;
	_sceneObject.setPosition(Common::Point(183, 71));
	_sceneObject.animate(ANIM_MODE_7, 0, NULL);

	// Set the text
	Rect textRect;
	_globals->gfxManager()._font.getStringBounds(msg.c_str(), textRect, _textWidth);
	textRect.center(_sceneObject._position.x, _sceneObject._position.y);
	_textPos.x = textRect.left;
	Speaker::setText(msg);
}

void SpeakerGText::removeText() {
	_sceneObject.remove();
	Speaker::removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerPOR::SpeakerPOR() {
	_speakerName = "POR";
	_newSceneNumber = 7221;
	_textPos = Common::Point(10, 30);
	_color1 = 41;
}

void SpeakerPOR::SpeakerAction1::signal(){
	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(60) + 60);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_5, this, NULL);
		break;
	case 2:
		setDelay(_globals->_randomSource.getRandomNumber(10));
		_actionIndex = 0;
		break;
	default:
		break;
	}
}

void SpeakerPOR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(7223);
	_object1.setStrip2(2);
	_object1.setPosition(Common::Point(191, 166));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(7223);
	_object2.setPosition(Common::Point(159, 86));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(7223);
	_object3.setStrip(3);
	_object3.setPosition(Common::Point(119, 107));
	_object3.fixPriority(199);
	_object3.setAction(&_action2);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerOR::SpeakerOR() {
	_speakerName = "OR";
	_newSceneNumber = 9430;
	_textPos = Common::Point(8, 36);
	_color1 = 42;
	_textWidth = 136;
}

void SpeakerOR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9431);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(202, 147));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(9431);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(199, 85));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerOText::SpeakerOText() : SpeakerGText() {
	_speakerName = "OTEXT";
	_textWidth = 240;
	_textPos = Common::Point(130, 10);
	_color1 = 42;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerQText::SpeakerQText() : ScreenSpeaker() {
	_speakerName = "QTEXT";
	_textPos = Common::Point(160, 40);
	_color1 = 35;
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSText::SpeakerSText() : ScreenSpeaker() {
	_speakerName = "STEXT";
	_color1 = 13;
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerPOText::SpeakerPOText() : ScreenSpeaker() {
	_speakerName = "POTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 41;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerMText::SpeakerMText() {
	_speakerName = "MTEXT";
	_color1 = 22;
	_textWidth = 230;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCText::SpeakerCText() {
	_speakerName = "CTEXT";
	_color1 = 4;
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerEText::SpeakerEText() {
	_speakerName = "ETEXT";
	_textPos = Common::Point(20, 20);
	_color1 = 22;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerGR::SpeakerGR() : AnimatedSpeaker() {
	_speakerName = "GR";
	_newSceneNumber = 9220;
	_textWidth = 136;
	_textPos = Common::Point(168, 36);
	_color1 = 14;
}

void SpeakerGR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9221);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(101, 70));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerHText::SpeakerHText() {
	_speakerName = "HTEXT";
	_textPos = Common::Point(160, 40);
	_color1 = 52;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSKText::SpeakerSKText() : ScreenSpeaker() {
	_speakerName = "SKTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 9;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerPText::SpeakerPText() {
	_speakerName = "PTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 5;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCHFText::SpeakerCHFText() {
	_speakerName = "CHFTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 56;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCDRText::SpeakerCDRText() {
	_speakerName = "CDRTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 52;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerFLText::SpeakerFLText() {
	_speakerName = "FLTEXT";
	_textPos = Common::Point(10, 40);
	_color1 = 17;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerBatText::SpeakerBatText() {
	_speakerName = "BATTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 3;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSKL::SpeakerSKL() : AnimatedSpeaker() {
	_speakerName = "SKL";
	_newSceneNumber = 7011;
	_textPos = Common::Point(10, 30);
	_color1 = 9;
}

void SpeakerSKL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(7013);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(203, 120));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(7013);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(197, 80));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerQL::SpeakerQL() : AnimatedSpeaker() {
	_speakerName = "QL";
	_newSceneNumber = 2610;
	_textPos = Common::Point(160, 30);
	_color1 = 35;
	_textMode = ALIGN_CENTER;
}

void SpeakerQL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2612);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(128, 146));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2612);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(122, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerSR::SpeakerSR() {
	_speakerName = "SR";
	_newSceneNumber = 2811;
	_textPos = Common::Point(10, 30);
	_color1 = 13;
	_textMode = ALIGN_CENTER;
}

void SpeakerSR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2813);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(224, 198));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2813);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(203, 96));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(2813);
	_object3.setStrip(3);
	_object3.setPosition(Common::Point(204, 91));
	_object3.fixPriority(199);
	_object3._numFrames = 3;
	_object3.animate(ANIM_MODE_7, 0, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerSL::SpeakerSL() {
	_speakerName = "SL";
	_newSceneNumber = 2810;
	_textPos = Common::Point(140, 30);
	_textWidth = 160;
	_color1 = 13;
	_textMode = ALIGN_CENTER;
}

void SpeakerSL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2812);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(95, 198));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2812);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(116, 96));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerQR::SpeakerQR() {
	_speakerName = "QR";
	_newSceneNumber = 2611;
	_textPos = Common::Point(10, 30);
	_color1 = 35;
	_textMode = ALIGN_CENTER;
}

void SpeakerQR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2613);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(191, 146));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2613);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(197, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerQU::SpeakerQU() {
	_speakerName = "QU";
	_newSceneNumber = 7020;
	_textPos = Common::Point(160, 30);
	_color1 = 35;
	_textMode = ALIGN_CENTER;
}

void SpeakerQU::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(7021);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(116, 120));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(7021);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(111, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCR::SpeakerCR() {
	_speakerName = "CR";
	_newSceneNumber = 9010;
	_textPos = Common::Point(20, 40);
	_color1 = 4;
}

void SpeakerCR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9011);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.setPosition(Common::Point(219, 168));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(9011);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(232, 81));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerMR::SpeakerMR() {
	_speakerName = "MR";
	_newSceneNumber = 2711;
	_textPos = Common::Point(10, 40);
	_color1 = 22;
}

void SpeakerMR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2713);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(220, 143));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2713);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(215, 99));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerSAL::SpeakerSAL() {
	_speakerName = "SAL";
	_newSceneNumber = 2851;
	_textPos = Common::Point(10, 30);
	_color1 = 13;
	_textMode = ALIGN_CENTER;
}

void SpeakerSAL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2853);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(185, 200));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2853);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(170, 92));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerML::SpeakerML() {
	_speakerName = "ML";
	_newSceneNumber = 2710;
	_textPos = Common::Point(160, 40);
	_color1 = 22;
}

void SpeakerML::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2712);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(99, 143));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2712);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(105, 99));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCHFL::SpeakerCHFL() {
	_speakerName = "CHFL";
	_newSceneNumber = 4111;
	_textPos = Common::Point(10, 40);
	_color1 = 56;
}

void SpeakerCHFL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4113);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(205, 116));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4113);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(202, 71));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCHFR::SpeakerCHFR() {
	_speakerName = "CHFR";
	_newSceneNumber = 4110;
	_textPos = Common::Point(160, 40);
	_color1 = 56;
}

void SpeakerCHFR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4112);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(103, 116));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4112);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(106, 71));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerPL::SpeakerPL() {
	_speakerName = "PL";
	_newSceneNumber = 4060;
	_textPos = Common::Point(160, 40);
	_color1 = 5;
}

void SpeakerPL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4062);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(107, 117));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4062);
	_object2.setStrip2(1);
	_object2.fixPriority(200);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(105, 62));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(4062);
	_object3.setStrip2(3);
	_object3.fixPriority(255);
	_object3.changeZoom(100);
	_object3._frame = 1;
	_object3.setPosition(Common::Point(105, 59));
	_object3.setAction(&_speakerAction2, NULL);

	Speaker::setText(msg);
}

void SpeakerPL::removeText() {
	_object3.remove();
	AnimatedSpeaker::removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerPR::SpeakerPR() {
	_speakerName = "PR";
	_newSceneNumber = 4061;
	_textPos = Common::Point(10, 40);
	_color1 = 5;
}

void SpeakerPR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4063);
	_object1.setStrip2(1);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(212, 117));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4063);
	_object2.setStrip2(2);
	_object2.fixPriority(200);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(214, 62));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(4063);
	_object3.setStrip2(3);
	_object3.fixPriority(255);
	_object3.changeZoom(100);
	_object3._frame = 1;
	_object3.setPosition(Common::Point(214, 59));
	_object3.setAction(&_speakerAction2, NULL);

	Speaker::setText(msg);
}

void SpeakerPR::removeText() {
	_object3.remove();
	AnimatedSpeaker::removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerCDR::SpeakerCDR() {
	_speakerName = "CDR";
	_newSceneNumber = 4161;
	_textPos = Common::Point(10, 40);
	_color1 = 52;
}

void SpeakerCDR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4163);
	_object1.setStrip2(1);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(208, 97));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4163);
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(200, 57));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCDL::SpeakerCDL() {
	_speakerName = "CDL";
	_newSceneNumber = 4160;
	_textPos = Common::Point(160, 40);
	_color1 = 52;
}

void SpeakerCDL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4162);
	_object1.setStrip2(1);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(112, 97));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4162);
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(115, 57));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerFLL::SpeakerFLL() {
	_speakerName = "FLL";
	_newSceneNumber = 5221;
	_textPos = Common::Point(10, 40);
	_color1 = 17;
}

void SpeakerFLL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(5223);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(216, 129));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(5223);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(210, 67));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerBatR::SpeakerBatR() {
	_speakerName = "BATR";
	_newSceneNumber = 5360;
	_textPos = Common::Point(140, 40);
	_color1 = 3;
}

void SpeakerBatR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(5361);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(137, 122));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(5361);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(137, 104));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

RingworldInvObjectList::RingworldInvObjectList() :
		_stunner(2280, 1, 2, OBJECT_STUNNER, "This is your stunner."),
		_scanner(1, 1, 3, OBJECT_SCANNER, "A combination scanner comm unit."),
		_stasisBox(5200, 1, 4, OBJECT_STASIS_BOX, "A stasis box."),
		_infoDisk(40, 1, 1, OBJECT_INFODISK, "The infodisk you took from the assassin."),
		_stasisNegator(0, 2, 2, OBJECT_STASIS_NEGATOR, "The stasis field negator."),
		_keyDevice(4250, 1, 6, OBJECT_KEY_DEVICE, "A magnetic key device."),
		_medkit(2280, 1, 7, OBJECT_MEDKIT,  "Your medkit."),
		_ladder(4100, 1, 8, OBJECT_LADDER, "The chief's ladder."),
		_rope(4150, 1, 9, OBJECT_ROPE, "The chief's rope."),
		_key(7700, 1, 11, OBJECT_KEY, "A key."),
		_translator(7700, 1, 13, OBJECT_TRANSLATOR,  "The dolphin translator box."),
		_ale(2150, 1, 10, OBJECT_ALE, "A bottle of ale."),
		_paper(7700, 1, 12, OBJECT_PAPER, "A slip of paper with the numbers 2,4, and 3 written on it."),
		_waldos(0, 1, 14, OBJECT_WALDOS, "A pair of waldos from the ruined probe."),
		_stasisBox2(8100, 1, 4, OBJECT_STASIS_BOX2, "A stasis box."),
		_ring(8100, 2, 5, OBJECT_RING, "This is a signet ring sent to you by Louis Wu."),
		_cloak(9850, 2, 6, OBJECT_CLOAK, "A fine silk cloak."),
		_tunic(9450, 2, 7, OBJECT_TUNIC, "The patriarch's soiled tunic."),
		_candle(9500, 2, 8, OBJECT_CANDLE, "A tallow candle."),
		_straw(9400, 2, 9, OBJECT_STRAW, "Clean, dry straw."),
		_scimitar(9850, 1, 18, OBJECT_SCIMITAR, "A scimitar from the Patriarch's closet."),
		_sword(9850, 1, 17, OBJECT_SWORD, "A short sword from the Patriarch's closet."),
		_helmet(9500, 2, 4, OBJECT_HELMET, "Some type of helmet."),
		_items(4300, 2, 10, OBJECT_ITEMS, "Two interesting items from the Tnuctipun vessel."),
		_concentrator(4300, 2, 11, OBJECT_CONCENTRATOR, "The Tnuctipun anti-matter concentrator contained in a stasis field."),
		_nullifier(5200, 2, 12, OBJECT_NULLIFIER, "A purported neural wave nullifier."),
		_peg(4045, 2, 16, OBJECT_PEG, "A peg with a symbol."),
		_vial(5100, 2, 17, OBJECT_VIAL, "A vial of the bat creatures anti-pheromone drug."),
		_jacket(9850, 3, 1, OBJECT_JACKET, "A natty padded jacket."),
		_tunic2(9850, 3, 2, OBJECT_TUNIC2, "A very hairy tunic."),
		_bone(5300, 3, 5, OBJECT_BONE, "A very sharp bone."),
		_jar(7700, 3, 4, OBJECT_JAR, "An jar filled with a green substance."),
		_emptyJar(7700, 3, 3, OBJECT_EMPTY_JAR, "An empty jar.") {

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

void RingworldGame::restartGame() {
	if (MessageDialog::show(RESTART_MSG, CANCEL_BTN_STRING, RESTART_BTN_STRING) == 1)
		_globals->_game->restart();
}

void RingworldGame::saveGame() {
	if (!_vm->canSaveGameStateCurrently())
		MessageDialog::show(SAVING_NOT_ALLOWED_MSG, OK_BTN_STRING);
	else {
		// Show the save dialog
		handleSaveLoad(true, _globals->_sceneHandler._saveGameSlot, _globals->_sceneHandler._saveName);
	}
}

void RingworldGame::restoreGame() {
	if (!_vm->canLoadGameStateCurrently())
		MessageDialog::show(RESTORING_NOT_ALLOWED_MSG, OK_BTN_STRING);
	else {
		// Show the load dialog
		handleSaveLoad(false, _globals->_sceneHandler._loadGameSlot, _globals->_sceneHandler._saveName);
	}
}

void RingworldGame::quitGame() {
	if (MessageDialog::show(QUIT_CONFIRM_MSG, CANCEL_BTN_STRING, QUIT_BTN_STRING) == 1)
		_vm->quitGame();
}

void RingworldGame::handleSaveLoad(bool saveFlag, int &saveSlot, Common::String &saveName) {
	const EnginePlugin *plugin = 0;
	EngineMan.findGame(_vm->getGameId(), &plugin);
	GUI::SaveLoadChooser *dialog;
	if (saveFlag)
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"));
	else
		dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"));

	dialog->setSaveMode(saveFlag);

	saveSlot = dialog->runModalWithPluginAndTarget(plugin, ConfMan.getActiveDomainName());
	saveName = dialog->getResultString();

	delete dialog;
}

void RingworldGame::start() {
	// Set some default flags
	_globals->setFlag(12);
	_globals->setFlag(34);

	// Set the screen to scroll in response to the player moving off-screen
	_globals->_scrollFollower = &_globals->_player;

	// Set the object's that will be in the player's inventory by default
	RING_INVENTORY._stunner._sceneNumber = 1;
	RING_INVENTORY._scanner._sceneNumber = 1;
	RING_INVENTORY._ring._sceneNumber = 1;

	int slot = -1;

	if (ConfMan.hasKey("save_slot")) {
		slot = ConfMan.getInt("save_slot");
		Common::String file = _vm->generateSaveName(slot);
		Common::InSaveFile *in = _vm->_system->getSavefileManager()->openForLoading(file);
		if (in)
			delete in;
		else
			slot = -1;
	}

	if (slot >= 0)
		_globals->_sceneHandler._loadGameSlot = slot;
	else
		// Switch to the title screen
		_globals->_sceneManager.setNewScene(1000);

	_globals->_events.showCursor();
}

void RingworldGame::restart() {
	_globals->_scenePalette.clearListeners();
	_globals->_soundHandler.stop();

	// Reset the flags
	_globals->reset();
	_globals->setFlag(34);

	// Clear save/load slots
	_globals->_sceneHandler._saveGameSlot = -1;
	_globals->_sceneHandler._loadGameSlot = -1;

	_globals->_stripNum = 0;
	_globals->_events.setCursor(CURSOR_WALK);

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
	_globals->_sceneManager.changeScene(30);
}

void RingworldGame::endGame(int resNum, int lineNum) {
	_globals->_events.setCursor(CURSOR_WALK);
	Common::String msg = _resourceManager->getMessage(resNum, lineNum);
	bool savesExist = _saver->savegamesExist();

	if (!savesExist) {
		// No savegames exist, so prompt the user to restart or quit
		if (MessageDialog::show(msg, QUIT_BTN_STRING, RESTART_BTN_STRING) == 0)
			_vm->quitGame();
		else
			restart();
	} else {
		// Savegames exist, so prompt for Restore/Restart
		bool breakFlag;
		do {
			if (_vm->shouldQuit()) {
				breakFlag = true;
			} else if (MessageDialog::show(msg, RESTART_BTN_STRING, RESTORE_BTN_STRING) == 0) {
				restart();
				breakFlag = true;
			} else {
				handleSaveLoad(false, _globals->_sceneHandler._loadGameSlot, _globals->_sceneHandler._saveName);
				breakFlag = _globals->_sceneHandler._loadGameSlot >= 0;
			}
		} while (!breakFlag);
	}

	_globals->_events.setCursorFromFlag();
}

void RingworldGame::processEvent(Event &event) {
	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			// F1 - Help
			MessageDialog::show(HELP_MSG, OK_BTN_STRING);
			break;

		case Common::KEYCODE_F2: {
			// F2 - Sound Options
			ConfigDialog *dlg = new ConfigDialog();
			dlg->runModal();
			delete dlg;
			_globals->_soundManager.syncSounds();
			_globals->_events.setCursorFromFlag();
			break;
		}

		case Common::KEYCODE_F3:
			// F3 - Quit
			quitGame();
			event.handled = false;
			break;

		case Common::KEYCODE_F4:
			// F4 - Restart
			restartGame();
			_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F7:
			// F7 - Restore
			restoreGame();
			_globals->_events.setCursorFromFlag();
			break;

		case Common::KEYCODE_F10:
			// F10 - Pause
			GfxDialog::setPalette();
			MessageDialog::show(GAME_PAUSED_MSG, OK_BTN_STRING);
			_globals->_events.setCursorFromFlag();
			break;

		default:
			break;
		}
	}
}

} // End of namespace tSage
