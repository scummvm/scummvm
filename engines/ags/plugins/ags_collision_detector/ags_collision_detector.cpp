/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_collision_detector/ags_collision_detector.h"
#include "ags/shared/util/geometry.h"
#include "common/config-manager.h"

namespace AGS3 {
namespace Plugins {
namespace AGSCollisionDetector {

#define CD_NORMAL 0
#define CD_DEBUG 1

#define CD_OBJECT 0
#define CD_CHARACTER 1

#define CD_SYS_ERROR -2
#define CD_PARAM_ERROR -1
#define CD_NO_COLLISION 0
#define CD_COLLISION 1
#define CD_OK 0
#define CD_IGNORED 1

bool find_overlapping_area(Rect r1, Rect r2, Rect *overlap);

const char *AGSCollisionDetector::AGS_GetPluginName() {
	return "AGS Collision Detector";
}

void AGSCollisionDetector::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(boundingBoxColDetect, AGSCollisionDetector::boundingBoxColDetect);
	SCRIPT_METHOD(dumpInfo, AGSCollisionDetector::dumpInfo);
	SCRIPT_METHOD(resetTransparencyThreshold, AGSCollisionDetector::resetTransparencyThreshold);
	SCRIPT_METHOD(resetTransparentColor, AGSCollisionDetector::resetTransparentColor);
	SCRIPT_METHOD(setDebugMode, AGSCollisionDetector::setDebugMode);
	SCRIPT_METHOD(setTransparencyThreshold, AGSCollisionDetector::setTransparencyThreshold);
	SCRIPT_METHOD(setTransparentColor, AGSCollisionDetector::setTransparentColor);
	SCRIPT_METHOD(spritePixelColDetect, AGSCollisionDetector::spritePixelColDetect);
	SCRIPT_METHOD(spriteSpriteColDetect, AGSCollisionDetector::spriteSpriteColDetect);
}

void AGSCollisionDetector::boundingBoxColDetect(ScriptMethodParams &params) {
	//	PARAMS8(int, id1, int, type1, int, slot1, int, scale1, int, id2, int, type2, int, slot2, int, scale2);

	// TODO
	warning("STUB: AGSCollisionDetector::boundingBoxColDetect is not implemented");
	params._result = CD_NO_COLLISION;
}

void AGSCollisionDetector::dumpInfo(ScriptMethodParams &params) {

	// TODO
	warning("STUB: AGSCollisionDetector::dumpInfo is not implemented");
	params._result = CD_IGNORED;
}

void AGSCollisionDetector::resetTransparencyThreshold(ScriptMethodParams &params) {
	transparencyTreshold = 100;
	params._result = CD_OK;
}

void AGSCollisionDetector::resetTransparentColor(ScriptMethodParams &params) {
	transparentColor.r = 255;
	transparentColor.g = 0;
	transparentColor.b = 255;
	params._result = CD_OK;
}

void AGSCollisionDetector::setDebugMode(ScriptMethodParams &params) {
	//	PARAMS1(int, status);

	// TODO
	warning("STUB: AGSCollisionDetector::setDebugMode is not implemented");
	params._result = CD_OK;
}

void AGSCollisionDetector::setTransparencyThreshold(ScriptMethodParams &params) {
	PARAMS1(int, trans_thresh);

	if ((trans_thresh >= 0) && (trans_thresh <= 100)) {
		transparencyTreshold = trans_thresh;
		params._result = CD_OK;
	} else
		params._result = CD_PARAM_ERROR;
}

void AGSCollisionDetector::setTransparentColor(ScriptMethodParams &params) {
	PARAMS3(int, transparentRed, int, transparentGreen, int, transparentBlue);

	if ((transparentRed < 0) || (transparentRed > 255) ||
		(transparentGreen < 0) || (transparentGreen > 255) ||
		(transparentBlue < 0) || (transparentBlue > 255)) {
		params._result = CD_PARAM_ERROR;
		return;
	} else {
		transparentColor.r = transparentRed;
		transparentColor.g = transparentGreen;
		transparentColor.b = transparentBlue;
		params._result = CD_OK;
	}
}

void AGSCollisionDetector::spritePixelColDetect(ScriptMethodParams &params) {
	// PARAMS5(int, id, int, type, int, slot, int, pixelX, int, pixelY);

	// TODO
	warning("STUB: AGSCollisionDetector::spritePixelColDetect is not implemented");
	params._result = CD_NO_COLLISION;
}

void AGSCollisionDetector::spriteSpriteColDetect(ScriptMethodParams &params) {
	PARAMS6(int, id1, int, type1, int, slot1, int, id2, int, type2, int, slot2);

	int32 spr1_w, spr1_h; // sprite sizes
	int32 spr2_w, spr2_h;

	Rect r1, r2;
	Rect overlap(0, 0, 0, 0);

	BITMAP *sprite1 = _engine->GetSpriteGraphic(slot1);
	BITMAP *sprite2 = _engine->GetSpriteGraphic(slot2);
	_engine->GetBitmapDimensions(sprite1, &spr1_w, &spr1_h, nullptr);
	_engine->GetBitmapDimensions(sprite2, &spr2_w, &spr2_h, nullptr);

	if (type1 == CD_OBJECT) { // object
		if (id1 < _engine->GetNumObjects()) {
			AGSObject *obj1 = _engine->GetObject(id1);
			r1.Left = obj1->x;
			r1.Top = obj1->y - spr1_h;
			r1.Right = obj1->x + spr1_w;
			r1.Bottom = obj1->y;
		}
	} else { // character
		if (id1 < _engine->GetNumCharacters()) {
			AGSCharacter *ch1 = _engine->GetCharacter(id1);
			r1.Left = ch1->x - (spr1_w / 2);
			r1.Top = ch1->y - spr1_h;
			r1.Right = ch1->x + (spr1_w / 2);
			r1.Bottom = ch1->y;
		}
	}

	if (type2 == CD_OBJECT) { // object
		if (id2 < _engine->GetNumObjects()) {
			AGSObject *obj2 = _engine->GetObject(id2);
			r2.Left = obj2->x;
			r2.Top = obj2->y - spr2_h;
			r2.Right = obj2->x + spr2_w;
			r2.Bottom = obj2->y;
		}
	} else { // character
		if (id2 < _engine->GetNumCharacters()) {
			AGSCharacter *ch2 = _engine->GetCharacter(id2);
			r2.Left = ch2->x - (spr2_w / 2);
			r2.Top = ch2->y - spr2_h;
			r2.Right = ch2->x + (spr2_w / 2);
			r2.Bottom = ch2->y;
		}
	}

	// WORKAROUND for bee sprite in Bear Story
	if ((ConfMan.get("gameid") == "bear") && (slot1 == 87)) {
		r1.Bottom -= 9;
	}

	if (find_overlapping_area(r1, r2, &overlap)) {
		params._result = CD_COLLISION;
		return;
	}

	params._result = CD_NO_COLLISION;
}

bool find_overlapping_area(Rect r1, Rect r2, Rect *overlap) {
	overlap->Left = MAX(r1.Left, r2.Left);
	overlap->Top = MAX(r1.Top, r2.Top);
	overlap->Right = MIN(r1.Right, r2.Right);
	overlap->Bottom = MIN(r1.Bottom, r2.Bottom);

	if (overlap->Left >= overlap->Right || overlap->Top >= overlap->Bottom) // degenerate rect means no overlap
		return false;

	return true;
}

} // namespace AGSCollisionDetector
} // namespace Plugins
} // namespace AGS3
