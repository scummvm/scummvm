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
	//
	case 1400: return new Scene1400();
	// Ringworld Space-port
	case 1500: return new Scene1500();

	/* Scene group 3 */
	// Cockpit cutscenes
	case 2000: return new Scene2000();
	// Cockpit
	case 2100: return new Scene2100();

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

} // End of namespace tSage
