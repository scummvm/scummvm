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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.cpp $
 * $Id: scene_logic.cpp 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#include "tsage/ringworld_logic.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld_scenes1.h"
#include "tsage/ringworld_scenes2.h"
#include "tsage/ringworld_scenes3.h"
#include "tsage/ringworld_scenes4.h"
#include "tsage/ringworld_scenes5.h"

namespace tSage {

Scene *SceneFactory::createScene(int sceneNumber) {
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

	/* Scene group 6 */

	/* Scene group 8 */

	/* Scene group 10 */

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
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
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
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
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
	while (!_vm->getEventManager()->shouldQuit() && !_globals->_events.getEvent(event)) {
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	List<SceneItem *>::iterator ii;
	for (ii = _globals->_sceneItems.begin(); ii != _globals->_sceneItems.end(); ++ii) {
		SceneItem *sceneItem = *ii;
		if (sceneItem->contains(event.mousePos)) {
			sceneItem->doAction(_actionId);
			break;
		}
	}

	_globals->_events.setCursor(CURSOR_ARROW);
}

void SceneArea::synchronise(Serialiser &s) {
	s.syncAsSint16LE(_pt.x);
	s.syncAsSint16LE(_pt.y);
	s.syncAsSint32LE(_resNum);
	s.syncAsSint32LE(_rlbNum);
	s.syncAsSint32LE(_subNum);
	s.syncAsSint32LE(_actionId);
	_bounds.synchronise(s);
}

/*--------------------------------------------------------------------------*/

SpeakerGText::SpeakerGText() {
	_speakerName = "GTEXT";
	_textWidth = 160;
	_textPos = Common::Point(130, 10);
	_colour1 = 42;
	_hideObjects = false;
}

void SpeakerGText::setText(const Common::String &msg) {
	// Set the animation properties
	_sceneObject.postInit();
	_sceneObject.setVisage(9405);
	_sceneObject.setStrip2(3);
	_sceneObject.setPriority2(255);
	_sceneObject.changeZoom(100);
	_sceneObject._frame = 1;
	_sceneObject.setPosition(Common::Point(183, 71));
	_sceneObject.animate(ANIM_MODE_7, 0, NULL);

	// Set the text
	Rect textRect;
	_globals->gfxManager()._font.getStringBounds(msg.c_str(), textRect, _textWidth);
	textRect.centre(_sceneObject._position.x, _sceneObject._position.y);
	_textPos.x = textRect.left;
	setText(msg);
}

void SpeakerGText::removeText() {
	_sceneObject.remove();
	removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerOText::SpeakerOText(): SpeakerGText() {
	_speakerName = "OTEXT";
	_textWidth = 240;
	_textPos = Common::Point(130, 10);
	_colour1 = 42;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerQText::SpeakerQText(): ScreenSpeaker() {
	_speakerName = "QTEXT";
	_textPos = Common::Point(160, 40);
	_colour1 = 35;
	_textWidth = 240;
	_textMode = ALIGN_CENTRE;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSText::SpeakerSText(): ScreenSpeaker() {
	_speakerName = "STEXT";
	_colour1 = 13;
	_textWidth = 240;
	_textMode = ALIGN_CENTRE;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerMText::SpeakerMText() {
	_speakerName = "MTEXT";
	_colour1 = 11;
	_textWidth = 160;
	_textMode = ALIGN_CENTRE;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCText::SpeakerCText() {
	_speakerName = "CTEXT";
	_colour1 = 4;
	_textWidth = 240;
	_textMode = ALIGN_CENTRE;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerHText::SpeakerHText() {
	_speakerName = "HTEXT";
	_textPos = Common::Point(160, 40);
	_colour1 = 52;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerPText::SpeakerPText() {
	_speakerName = "PTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTRE;
	_colour1 = 5;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCHFText::SpeakerCHFText() {
	_speakerName = "CHFTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTRE;
	_colour1 = 56;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCDRText::SpeakerCDRText() {
	_speakerName = "CDRTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTRE;
	_colour1 = 52;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerQL::SpeakerQL(): AnimatedSpeaker() {
	_speakerName = "QL";
	_newSceneNumber = 2610;
	_textPos = Common::Point(160, 30);
	_colour1 = 35;
	_textMode = ALIGN_CENTRE;
}

void SpeakerQL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2612);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(128, 146));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2612);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 13;
	_textMode = ALIGN_CENTRE;
}

void SpeakerSR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2813);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(224, 198));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2813);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(203, 96));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(2813);
	_object3.setStrip(3);
	_object3.setPosition(Common::Point(204, 91));
	_object3.setPriority2(199);
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
	_colour1 = 13;
	_textMode = ALIGN_CENTRE;
}

void SpeakerSL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2812);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(95, 198));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2812);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 13;
	_textMode = ALIGN_CENTRE;
}

void SpeakerQR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2613);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(191, 146));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2613);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(197, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCR::SpeakerCR() {
	_speakerName = "CR";
	_newSceneNumber = 9010;
	_textPos = Common::Point(20, 40);
	_colour1 = 4;
}

void SpeakerCR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9011);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.setPosition(Common::Point(219, 168));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(9011);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
	_object2.setPosition(Common::Point(232, 81));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerMR::SpeakerMR() {
	_speakerName = "MR";
	_newSceneNumber = 2711;
	_textPos = Common::Point(40, 10);
	_colour1 = 22;
}

void SpeakerMR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2713);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(220, 143));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2713);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 13;
	_textMode = ALIGN_CENTRE;
}

void SpeakerSAL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2853);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(185, 200));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2853);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 22;
}

void SpeakerML::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2712);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(99, 143));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(2712);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 56;
}

void SpeakerCHFL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4113);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(205, 116));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(4113);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 56;
}

void SpeakerCHFR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4112);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(103, 116));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(4112);
	_object2.setStrip2(1);
	_object2.setPriority2(255);
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
	_colour1 = 5;
}

void SpeakerPL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4062);
	_object1.setStrip2(2);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(107, 117));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(4062);
	_object2.setStrip2(1);
	_object2.setPriority2(200);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(105, 62));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(4062);
	_object3.setStrip2(3);
	_object3.setPriority2(255);
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
	_colour1 = 5;
}

void SpeakerPR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4063);
	_object1.setStrip2(1);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(212, 117));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(4063);
	_object2.setStrip2(2);
	_object2.setPriority2(200);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(214, 62));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(4063);
	_object3.setStrip2(3);
	_object3.setPriority2(255);
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
	_colour1 = 52;
}

void SpeakerCDR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4163);
	_object1.setStrip2(1);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(208, 97));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(4163);
	_object2.setStrip2(2);
	_object2.setPriority2(255);
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
	_colour1 = 52;
}

void SpeakerCDL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4162);
	_object1.setStrip2(1);
	_object1.setPriority2(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(112, 97));
	_object1.animate(ANIM_MODE_7, 0, NULL);
	
	_object2.postInit(&_objectList);
	_object2.setVisage(4162);
	_object2.setStrip2(2);
	_object2.setPriority2(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(115, 57));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

} // End of namespace tSage
