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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "director/director.h"
#include "director/cast.h"
#include "director/sprite.h"
#include "director/score.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-the.h"

namespace Director {

class Sprite;

TheEntity entities[] = {
	{ kTheActorList,		"actorList",		false, 4 },	//				D4 property
	{ kTheBeepOn,			"beepOn",			false, 2 },	// D2 p
	{ kTheButtonStyle,		"buttonStyle",		false, 2 },	// D2 p
	{ kTheCast,				"cast",				true,  3 },	// D3
	{ kTheCastMembers,		"castmembers",		false, 3 },	//		 D3
	{ kTheCenterStage,		"centerStage",		false, 2 },	// D2 p
	{ kTheChars,			"chars",			false, 3 },	//		 D3
	{ kTheCheckBoxAccess,	"checkBoxAccess",	false, 2 },	// D2 p
	{ kTheCheckBoxType,		"checkBoxType",		false, 2 },	// D2 p
	{ kTheClickLoc,			"clickLoc",			false, 4 },	// 				D4 function
	{ kTheClickOn,			"clickOn",			false, 2 },	// D2 f
	{ kTheColorDepth,		"colorDepth",		false, 2 },	// D2 p
	{ kTheColorQD,			"colorQD",			false, 2 },	// D2 f
	{ kTheCommandDown,		"commandDown",		false, 2 },	// D2 f
	{ kTheControlDown,		"controlDown",		false, 2 },	// D2 f
	{ kTheDate,				"date",				false, 3 },	//		 D3
	{ kTheDoubleClick,		"doubleClick",		false, 2 },	// D2 f
	{ kTheExitLock,			"exitLock",			false, 2 },	// D2 p
	{ kTheField,			"field",			true,  3 },	//		 D3
	{ kTheFixStageSize,		"fixStageSize",		false, 2 },	// D2 p
	{ kTheFloatPrecision,	"floatPrecision",	false, 3 },	//		D3 p
	{ kTheFrame,			"frame",			false, 2 },	// D2 f
	{ kTheFrameLabel,		"frameLabel",		false, 4 },	//				D4 p
	{ kTheFrameScript,		"frameScript",		false, 4 },	//				D4 p
	{ kTheFramePalette,		"framePalette",		false, 4 },	//				D4 p
	{ kTheFreeBlock,		"freeBlock",		false, 2 },	// D2 f
	{ kTheFreeBytes,		"freeBytes",		false, 2 },	// D2 f
	{ kTheFullColorPermit,	"fullColorPermit",	false, 2 },	// D2 p
	{ kTheImageDirect,		"imageDirect",		false, 2 },	// D2 p
	{ kTheItems,			"items",			false, 3 },	//		 D3
	{ kTheItemDelimiter,	"itemDelimiter",	false, 4 },	//				D4 p
	{ kTheKey,				"key",				false, 2 },	// D2 f
	{ kTheKeyCode,			"keyCode",			false, 2 },	// D2 f
	{ kTheKeyDownScript,	"keyDownScript",	false, 2 },	// D2 p
	{ kTheKeyUpScript,		"keyUpScript",		false, 4 },	//				D4 p
	{ kTheLabelList,		"labelList",		false, 3 },	//		D3 f
	{ kTheLastClick,		"lastClick",		false, 2 },	// D2 f
	{ kTheLastEvent,		"lastEvent",		false, 2 },	// D2 f
	{ kTheLastFrame,		"lastFrame",		false, 4 },	//				D4 p
	{ kTheLastKey,			"lastKey",			false, 2 },	// D2 f
	{ kTheLastRoll,			"lastRoll",			false, 2 },	// D2 f
	{ kTheLines,			"lines",			false, 3 },	//		 D3
	{ kTheMachineType,		"machineType",		false, 2 },	// D2 f
	{ kTheMaxInteger,		"maxInteger",		false, 4 },	//				D4 f
	{ kTheMemorySize,		"memorySize",		false, 2 },	// D2 f
	{ kTheMenu,				"menu",				true,  0 },
	{ kTheMenus,			"menus",			false, 0 },
	{ kTheMenuItem,			"menuitem",			true,  0 },
	{ kTheMenuItems,		"menuitems",		false, 0 },
	{ kTheMouseCast,		"mouseCast",		false, 3 },	//		D3 f
	{ kTheMouseChar,		"mouseChar",		false, 3 },	//		D3 f
	{ kTheMouseDown,		"mouseDown",		false, 2 },	// D2 f
	{ kTheMouseDownScript,  "mouseDownScript",	false, 2 },	// D2 p
	{ kTheMouseH,			"mouseH",			false, 2 },	// D2 f
	{ kTheMouseItem,		"mouseItem",		false, 3 },	//		D3 f
	{ kTheMouseLine,		"mouseLine",		false, 3 },	//		D3 f
	{ kTheMouseUp,			"mouseUp",			false, 2 },	// D2 f
	{ kTheMouseUpScript,  	"mouseUpScript",	false, 2 },	// D2 p
	{ kTheMouseV,			"mouseV",			false, 2 },	// D2 f
	{ kTheMouseWord,		"mouseWord",		false, 3 },	//		D3 f
	{ kTheMovie,			"movie",			false, 2 },	// D2 f
	{ kTheMovieFileFreeSize,"movieFileFreeSize",false, 4 },	//				D4 f
	{ kTheMovieFileSize,	"movieFileSize",	false, 4 },	//				D4 f
	{ kTheMovieName,		"movieName",		false, 4 },	//				D4 f
	{ kTheMoviePath,		"moviePath",		false, 4 },	//				D4 f
	{ kTheMultiSound,		"multiSound",		false, 4 },	//				D4 p
	{ kTheOptionDown,		"optionDown",		false, 2 },	// D2 f
	{ kTheParamCount,		"paramCount",		false, 4 },	//				D4 f
	{ kThePathName,			"pathName",			false, 2 },	// D2 f
	{ kThePauseState,		"pauseState",		false, 2 },	// D2 f
	{ kThePerFrameHook,		"perFrameHook",		false, 2 },	// D2 p
	{ kThePreloadEventAbort,"preloadEventAbort",false, 4 },	//				D4 p
	{ kThePreLoadRAM,		"preLoadRAM",		false, 4 },	//				D4 p
	{ kTheQuickTimePresent,	"quickTimePresent",	false, 4 },	//				D4 f
	{ kTheRandomSeed,		"randomSeed",		false, 4 },	//				D4 p
	{ kTheResult,			"result",			false, 2 },	// D2 f
	{ kTheRightMouseDown,	"rightMouseDown",	false, 0 },
	{ kTheRightMouseUp,		"rightMouseUp",		false, 0 },
	{ kTheRomanLingo,		"romanLingo",		false, 4 },	//				D4 p
	{ kTheSearchCurrentFolder,"searchCurrentFolder",false,4 },//			D4 f
	{ kTheSearchPath,		"searchPath",		false, 4 },	//				D4 f
	{ kTheSelection,		"selection",		false, 2 },	// D2 f
	{ kTheSelEnd,			"selEnd",			false, 2 },	// D2 p
	{ kTheSelStart,			"selStart",			false, 2 },	// D2 p
	{ kTheShiftDown,		"shiftDown",		false, 2 },	// D2 f
	{ kTheSoundEnabled,		"soundEnabled",		false, 2 },	// D2 p
	{ kTheSoundLevel,		"soundLevel",		false, 2 },	// D2 p
	{ kTheSprite,			"sprite",			true,  4 },	//				D4
	{ kTheSqrt,				"sqrt",				false, 2 },	// D2 f
	{ kTheStage,			"stage",			false, 4 },	//				D4 p
	{ kTheStageBottom,		"stageBottom",		false, 2 },	// D2 f
	{ kTheStageColor,		"stageColor",		false, 3 },	//		D3 p
	{ kTheStageLeft,		"stageLeft",		false, 2 },	// D2 f
	{ kTheStageRight,		"stageRight",		false, 2 },	// D2 f
	{ kTheStageTop,			"stageTop",			false, 2 },	// D2 f
	{ kTheStillDown,		"stillDown",		false, 2 },	// D2 f
	{ kTheSwitchColorDepth,	"switchColorDepth",	false, 2 },	// D2 p
	{ kTheTicks,			"ticks",			false, 2 },	// D2 f
	{ kTheTimeoutKeyDown,	"timeoutKeyDown",	false, 2 },	// D2 p
	{ kTheTimeoutLapsed,	"timeoutLapsed",	false, 2 },	// D2 p
	{ kTheTimeoutLength,	"timeoutLength",	false, 2 },	// D2 p
	{ kTheTimeoutMouse,		"timeoutMouse",		false, 2 },	// D2 p
	{ kTheTimeoutPlay,		"timeoutPlay",		false, 2 },	// D2 p
	{ kTheTimeoutScript,	"timeoutScript",	false, 2 },	// D2 p
	{ kTheTime,				"time",				false, 3 },	// 		D3 f
	{ kTheTimer,			"timer",			false, 2 },	// D2 p
	{ kTheTrace,			"trace",			false, 4 },	//				D4 p
	{ kTheTraceLoad,		"traceLoad",		false, 4 },	//				D4 p
	{ kTheTraceLogFile,		"traceLogFile",		false, 4 },	//				D4 p
	{ kTheUpdateMovieEnabled,"updateMovieEnabled",false,4 },//				D4 p
	{ kTheWindow,			"window",			true,  4 },	//				D4
	{ kTheWindowList,		"windowList",		false, 4 },	//				D4 p
	{ kTheWords,			"words",			false, 3 },	//		 D3
	{ kTheNOEntity, NULL, false, 0 }
};

TheEntityField fields[] = {
	{ kTheSprite,	"backColor",	kTheBackColor },	// D2 p
	{ kTheSprite,	"blend",		kTheBlend },		//				D4 p
	{ kTheSprite,	"bottom",		kTheBottom },		// D2 p
	{ kTheSprite,	"castNum",		kTheCastNum },		// D2 p
	{ kTheSprite,	"constraint",	kTheConstraint },	// D2 p
	{ kTheSprite,	"cursor",		kTheCursor },		// D2 p
	{ kTheSprite,	"editableText", kTheEditableText },	//				D4 p
	{ kTheSprite,	"foreColor",	kTheForeColor },	// D2 p
	{ kTheSprite,	"height",		kTheHeight },		// D2 p
	{ kTheSprite,	"immediate",	kTheImmediate },	// D2 p
	{ kTheSprite,	"ink",			kTheInk },			// D2 p
	{ kTheSprite,	"left",			kTheLeft },			// D2 p
	{ kTheSprite,	"lineSize",		kTheLineSize },		// D2 p
	{ kTheSprite,	"locH",			kTheLocH },			// D2 p
	{ kTheSprite,	"locV",			kTheLocV },			// D2 p
	{ kTheSprite,	"moveableSprite",kTheMoveableSprite },//			D4 p
	{ kTheSprite,	"movieRate",	kTheMovieRate },	//				D4 P
	{ kTheSprite,	"movieTime",	kTheMovieTime },	//				D4 P
	{ kTheSprite,	"pattern",		kThePattern },		// D2 p
	{ kTheSprite,	"puppet",		kThePuppet },		// D2 p
	{ kTheSprite,	"right",		kTheRight },		// D2 p
	{ kTheSprite,	"scoreColor",	kTheScoreColor },	//				D4 p
	{ kTheSprite,	"scriptNum",	kTheScriptNum },	//				D4 p
	{ kTheSprite,	"startTime",	kTheStartTime },	//				D4 p
	{ kTheSprite,	"stretch",		kTheStrech },		// D2 p
	{ kTheSprite,	"stopTime",		kTheStopTime },		//				D4 p
	{ kTheSprite,	"top",			kTheTop },			// D2 p
	{ kTheSprite,	"trails",		kTheTrails },		//				D4 p
	{ kTheSprite,	"type",			kTheType },			// D2 p
	{ kTheSprite,	"visible",		kTheVisible },		//				D4 p
	{ kTheSprite,	"volume",		kTheVolume },		//				D4 p
	{ kTheSprite,	"width",		kTheWidth },		// D2 p

	// Common cast fields
	{ kTheCast,		"castType",		kTheCastType },		//				D4 p
	{ kTheCast,		"filename",		kTheFileName },		//				D4 p
	{ kTheCast,		"height",		kTheHeight },		//				D4 p
	{ kTheCast,		"loaded",		kTheLoaded },		//				D4 p
	{ kTheCast,		"modified",		kTheModified },		//				D4 p
	{ kTheCast,		"name",			kTheName },			//		D3 p
	{ kTheCast,		"number",		kTheNumber },		//		D3 p
	{ kTheCast,		"rect",			kTheRect },			//				D4 p
	{ kTheCast,		"purgePriority",kThePurgePriority },//	 			D4 p // 0 Never purge, 1 Purge Last, 2 Purge next, 2 Purge normal
	{ kTheCast,		"scriptText",	kTheScriptText },	//				D4 p
	{ kTheCast,		"width",		kTheWidth },		//				D4 p

	// Shape fields
	{ kTheCast,		"backColor",	kTheBackColor },	//				D4 p
	{ kTheCast,		"foreColor",	kTheForeColor },	//				D4 p

	// Digital video fields
	{ kTheCast,		"center",		kTheCenter },		//				D4 p
	{ kTheCast,		"controller",	kTheController },	//				D4 p
	{ kTheCast,		"crop",			kTheCrop },			//				D4 p
	{ kTheCast,		"directToStage",kTheDirectToStage },//				D4 p
	{ kTheCast,		"duration",		kTheDuration },		//				D4 p
	{ kTheCast,		"frameRate",	kTheFrameRate },	//				D4 p
	{ kTheCast,		"loop",			kTheLoop },			//				D4 p
	{ kTheCast,		"pausedAtStart",kThePausedAtStart },//				D4 p
	{ kTheCast,		"preLoad",		kThePreLoad },		//				D4 p
	{ kTheCast,		"sound",		kTheSound },		//				D4 p // 0-1 off-on
	{ kTheCast,		"video",		kTheVideo },		//				D4 p

	// Bitmap fields
	{ kTheCast,		"depth",		kTheDepth },
	{ kTheCast,		"regPoint",		kTheRegPoint },		//				D4 p
	{ kTheCast,		"palette",		kThePalette },		//				D4 p
	{ kTheCast,		"picture",		kThePicture },		//		D3 p

	// TextCast fields
	{ kTheCast,		"hilite",		kTheHilite },		// D2 p
	{ kTheCast,		"size",			kTheSize },			//				D4 p
	{ kTheCast,		"text",			kTheText },			// D2 p

	// Field fields
	{ kTheField,	"textAlign",	kTheTextAlign },	//		D3 p
	{ kTheField,	"textFont",		kTheTextFont },		//		D3 p
	{ kTheField,	"textHeight",	kTheTextHeight },	//		D3 p
	{ kTheField,	"textSize",		kTheTextSize },		//		D3 p
	{ kTheField,	"textStyle",	kTheTextStyle },	//		D3 p

	{ kTheWindow,	"drawRect",		kTheDrawRect },		//				D4 p
	{ kTheWindow,	"fileName",		kTheFileName },		//				D4 p
	{ kTheWindow,	"modal",		kTheModal },		//				D4 p
	{ kTheWindow,	"rect",			kTheRect },			//				D4 p
	{ kTheWindow,	"title",		kTheTitle },		//				D4 p
	{ kTheWindow,	"titleVisible",	kTheTitleVisible },	//				D4 p
	{ kTheWindow,	"sourceRect",	kTheSourceRect },	//				D4 p
	{ kTheWindow,	"visible",		kTheVisible },		//				D4 p
	{ kTheWindow,	"windowType",	kTheWindowType },	//				D4 p

	{ kTheMenuItem,	"checkmark",	kTheCheckMark },	//		D3 p
	{ kTheMenuItem, "enabled",		kTheEnabled },		//		D3 p
	{ kTheMenuItem, "name",			kTheName },			//		D3 p
	{ kTheMenuItem, "script",		kTheScript },		//		D3 p

	{ kTheMenu,		"name",			kTheName },			//		D3 p

	{ kTheCastMembers,	"number",	kTheNumber },		// 		D3 p
	{ kTheChars,	"number",		kTheNumber },		//		D3 p
	{ kTheChars,	"last",			kTheLast },			//				D4 f
	{ kTheItems,	"number",		kTheNumber },		//		D3 p
	{ kTheItems,	"last",			kTheLast },			//				D4 f
	{ kTheLines,	"number",		kTheNumber },		//		D3 p
	{ kTheLines,	"last",			kTheLast },			//				D4 f
	{ kTheMenuItems,"number",		kTheNumber },		//		D3 p
	{ kTheMenus,	"number",		kTheNumber },		//		D3 p
	{ kTheWords,	"number",		kTheNumber },		//		D3 p
	{ kTheWords,	"last",			kTheLast },			//				D4 f

	{ kTheDate,		"short",		kTheShort },		//		D3 f
	{ kTheDate,		"long",			kTheLong },			//		D3 f
	{ kTheDate,		"abbreviated",	kTheAbbr },			//		D3 f
	{ kTheDate,		"abbrev",		kTheAbbr },			//		D3 f
	{ kTheDate,		"abbr",			kTheAbbr },			//		D3 f
	{ kTheTime,		"short",		kTheShort },		//		D3 f
	{ kTheTime,		"long",			kTheLong },			//		D3 f
	{ kTheTime,		"abbreviated",	kTheAbbr },			//		D3 f
	{ kTheTime,		"abbrev",		kTheAbbr },			//		D3 f
	{ kTheTime,		"abbr",			kTheAbbr },			//		D3 f

	{ kTheNOEntity, NULL, kTheNOField }
};

void Lingo::initTheEntities() {
	TheEntity *e = entities;

	while (e->entity != kTheNOEntity) {
		_theEntities[e->name] = e;
		e++;
	}

	TheEntityField *f = fields;

	while (f->entity != kTheNOEntity) {
		_theEntityFields[Common::String::format("%d%s", f->entity, f->name)] = f;
		f++;
	}
}

void Lingo::setTheEntity(int entity, Datum &id, int field, Datum &d) {
	switch (entity) {
	case kTheSprite:
		setTheSprite(id, field, d);
		break;
	case kThePerFrameHook:
		warning("STUB: Lingo::setTheEntity(): setting the perframehook");
		break;
	case kTheFloatPrecision:
		_floatPrecision = d.toInt();
		_floatPrecision = MAX(0, MIN(_floatPrecision, 19)); // 0 to 19
		_floatPrecisionFormat = Common::String::format("%%.%df", _floatPrecision);
		break;
	case kTheColorDepth:
		_vm->_colorDepth = d.toInt();

		// bpp. 1, 2, 4, 8, 32
		warning("STUB: Lingo::setTheEntity(): Set color depth to %d", _vm->_colorDepth);
		break;
	case kTheCast:
		setTheCast(id, field, d);
		break;
	default:
		warning("Lingo::setTheEntity(): Unprocessed setting field %d of entity %d", field, entity);
	}
}

void Lingo::setTheSprite(Datum &id1, int field, Datum &d) {
	int id = 0;

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::setTheSprite(): Unknown the sprite id type: %s", id1.type2str());
		return;
	}

	d.toInt(); // Enforce Integer

	if (!_vm->getCurrentScore()) {
		warning("Lingo::setTheSprite(): The sprite %d field %d setting over non-active score", id, field);
		return;
	}

	Sprite *sprite = _vm->getCurrentScore()->getSpriteById(id);

	if (!sprite)
		return;

	switch (field) {
	case kTheCastNum:
		if (_vm->getCurrentScore()->_loadedCast->contains(d.u.i)) {
			_vm->getCurrentScore()->loadCastInto(sprite, d.u.i);
			sprite->_castId = d.u.i;
		}
		break;
	case kTheWidth:
		sprite->_width = d.u.i;
		break;
	case kTheHeight:
		sprite->_height = d.u.i;
		break;
	case kTheTrails:
		sprite->_trails = d.u.i;
		break;
	case kTheInk:
		sprite->_ink = static_cast<InkType>(d.u.i);
		break;
	case kTheLocH:
		sprite->_startPoint.x = d.u.i;
		break;
	case kTheLocV:
		sprite->_startPoint.y = d.u.i;
		break;
	case kTheConstraint:
		sprite->_constraint = d.u.i;
		break;
	case kTheMoveableSprite:
		sprite->_moveable = d.u.i;
		break;
	case kTheBackColor:
		sprite->_backColor = d.u.i;
		break;
	case kTheForeColor:
		sprite->_foreColor = d.u.i;
		break;
	case kTheLeft:
		sprite->_left = d.u.i;
		break;
	case kTheRight:
		sprite->_right = d.u.i;
		break;
	case kTheTop:
		sprite->_top = d.u.i;
		break;
	case kTheBottom:
		sprite->_bottom = d.u.i;
		break;
	case kTheBlend:
		sprite->_blend = d.u.i;
		break;
	case kTheVisible:
		sprite->_visible = (d.u.i == 0 ? false : true);
		break;
	case kTheType:
		sprite->_type = static_cast<SpriteType>(d.u.i);
		break;
	case kTheMovieRate:
		sprite->_movieRate = d.u.i;
		break;
	case kTheMovieTime:
		sprite->_movieTime = d.u.i;
		break;
	case kTheStopTime:
		sprite->_stopTime = d.u.i;
		break;
	case kTheStartTime:
		sprite->_startTime = d.u.i;
		break;
	case kTheStretch:
		sprite->_stretch = d.u.i;
		break;
	case kTheVolume:
		sprite->_volume = d.u.i;
		break;
	case kTheLineSize:
		sprite->_lineSize = d.u.i;
		break;
	case kTheEditableText:
		sprite->_editableText = *d.toString();
		break;
	case kThePattern:
		sprite->setPattern(d.u.i);
		break;
	default:
		warning("Lingo::setTheSprite(): Unprocessed setting field %d of sprite", field);
	}
}

Datum Lingo::getTheEntity(int entity, Datum &id, int field) {
	Datum d;

	switch (entity) {
	case kTheSprite:
		d = getTheSprite(id, field);
		break;
	case kTheCast:
		d = getTheCast(id, field);
		break;
	case kThePerFrameHook:
		warning("STUB: Lingo::getTheEntity(): getting the perframehook");
		break;
	case kTheFloatPrecision:
		d.type = INT;
		d.u.i = _floatPrecision;
		break;
	case kTheSqrt:
		id.toFloat();
		d.type = FLOAT;
		d.u.f = sqrt(id.u.f);
		break;
	case kTheKey:
		d.type = STRING;
		d.u.s = new Common::String(_vm->_key);
		break;
	case kTheKeyCode:
		d.type = INT;
		d.u.i = _vm->_keyCode;
		break;
	case kTheColorQD:
		d.type = INT;
		d.u.i = 1;
		break;
	case kTheColorDepth:
		// bpp. 1, 2, 4, 8, 32
		d.type = INT;
		d.u.i = _vm->_colorDepth;
		break;
	case kTheMachineType:
		// 1 - Macintosh 512Ke			D2
		// 2 - Macintosh Plus			D2
		// 3 - Macintosh SE				D2
		// 4 - Macintosh II				D2
		// 5 - Macintosh IIx			D2
		// 6 - Macintosh IIcx			D2
		// 7 - Macintosh SE/30			D2
		// 8 - Macintosh Portable		D2
		// 9 - Macintosh IIci			D2
		// 11 - Macintosh IIfx			D3
		// 15 - Macintosh Classic		D3
		// 16 - Macintosh IIsi			D3
		// 17 - Macintosh LC			D3
		// 18 - Macintosh Quadra 900	D3
		// 19 - PowerBook 170			D3
		// 20 - Macintosh Quadra 700	D3
		// 21 - Classic II				D3
		// 22 - PowerBook 100			D3
		// 23 - PowerBook 140			D3
		// 24 - Macintosh Quadra 950	D4
		// 25 - Macintosh LCIII			D4
		// 27 - PowerBook Duo 210		D4
		// 28 - Macintosh Centris 650	D4
		// 30 - PowerBook Duo 230		D4
		// 31 - PowerBook 180			D4
		// 32 - PowerBook 160			D4
		// 33 - Macintosh Quadra 800	D4
		// 35 - Macintosh LC II			D4
		// 42 - Macintosh IIvi			D4
		// 45 - Power Macintosh 7100/70	D5
		// 46 - Macintosh IIvx			D4
		// 47 - Macintosh Color Classic	D4
		// 48 - PowerBook 165c			D4
		// 50 - Macintosh Centris 610	D4
		// 52 - PowerBook 145			D4
		// 53 - PowerComputing 8100/100	D5
		// 73 - Power Macintosh 6100/60	D5
		// 76 - Macintosh Quadra 840av	D4
		// 256 - IBM PC-type machine	D3
		d.type = INT;
		d.u.i = _vm->_machineType;
		break;
	case kTheFrame:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->getCurrentFrame();
		break;
	case kTheStillDown:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_mouseIsDown;
		break;
	default:
		warning("Lingo::getTheEntity(): Unprocessed getting field %d of entity %d", field, entity);
		d.type = VOID;
	}

	return d;
}

Datum Lingo::getTheSprite(Datum &id1, int field) {
	Datum d;
	int id = 0;

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::getTheSprite(): Unknown the sprite id type: %s", id1.type2str());
		return d;
	}

	if (!_vm->getCurrentScore()) {
		warning("Lingo::getTheSprite(): The sprite %d field %d setting over non-active score", id, field);
		return d;
	}

	Sprite *sprite = _vm->getCurrentScore()->getSpriteById(id);

	if (!sprite)
		return d;

	d.type = INT;

	switch (field) {
	case kTheCastNum:
		d.u.i = sprite->_castId;
		break;
	case kTheWidth:
		d.u.i = sprite->_width;
		break;
	case kTheHeight:
		d.u.i = sprite->_height;
		break;
	case kTheTrails:
		d.u.i = sprite->_trails;
		break;
	case kTheInk:
		d.u.i = sprite->_ink;
		break;
	case kTheLocH:
		d.u.i = sprite->_startPoint.x;
		break;
	case kTheLocV:
		d.u.i = sprite->_startPoint.y;
		break;
	case kTheConstraint:
		d.u.i = sprite->_constraint;
		break;
	case kTheMoveableSprite:
		d.u.i = sprite->_moveable;
		break;
	case kTheBackColor:
		d.u.i = sprite->_backColor;
		break;
	case kTheForeColor:
		d.u.i = sprite->_foreColor;
		break;
	case kTheLeft:
		d.u.i = sprite->_left;
		break;
	case kTheRight:
		d.u.i = sprite->_right;
		break;
	case kTheBottom:
		d.u.i = sprite->_bottom;
		break;
	case kTheTop:
		d.u.i = sprite->_top;
		break;
	case kTheBlend:
		d.u.i = sprite->_blend;
		break;
	case kTheVisible:
		d.u.i = (sprite->_visible ? 1 : 0);
		break;
	case kTheType:
		d.u.i = sprite->_type;
		break;
	case kTheMovieRate:
		d.u.i = sprite->_movieRate;
		break;
	case kTheMovieTime:
		d.u.i = sprite->_movieTime;
		break;
	case kTheStopTime:
		d.u.i = sprite->_stopTime;
		break;
	case kTheStartTime:
		d.u.i = sprite->_startTime;
		break;
	case kTheVolume:
		d.u.i = sprite->_volume;
		break;
	case kTheStretch:
		d.u.i = sprite->_stretch;
		break;
	case kTheLineSize:
		d.u.i = sprite->_lineSize;
		break;
	case kTheEditableText:
		d.toString();
		d.u.s = &sprite->_editableText;
		break;
	case kThePattern:
		d.u.i = sprite->getPattern();
		break;
	default:
		warning("Lingo::getTheSprite(): Unprocessed getting field %d of sprite", field);
		d.type = VOID;
	}

	return d;
}

Datum Lingo::getTheCast(Datum &id1, int field) {
	Datum d;
	int id = 0;

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::getTheCast(): Unknown the cast id type: %s", id1.type2str());
		return d;
	}

	if (!_vm->getCurrentScore()) {
		warning("Lingo::getTheCast(): The cast %d field %d setting over non-active score", id, field);
		return d;
	}

	CastType castType;
	CastInfo *castInfo;
	if (!_vm->getCurrentScore()->_loadedCast->contains(id)) {
		if (field == kTheLoaded) {
			d.type = INT;
			d.u.i = 0;
		}

		return d;
	} else {
		warning("Lingo::getTheCast(): The cast %d found", id);
	}

	castType = _vm->getCurrentScore()->_loadedCast->getVal(id)->_type;
	castInfo = _vm->getCurrentScore()->_castsInfo[id];

	d.type = INT;

	switch (field) {
	case kTheCastType:
		d.u.i = castType;
		break;
	case kTheFileName:
		d.toString();
		d.u.s = &castInfo->fileName;
		break;
	case kTheName:
		d.toString();
		d.u.s = &castInfo->name;
		break;
	case kTheScriptText:
		d.toString();
		d.u.s = &castInfo->script;
		break;
	case kTheWidth:
		d.u.i = _vm->getCurrentScore()->getCastMemberInitialRect(id).width();
		break;
	case kTheHeight:
		d.u.i = _vm->getCurrentScore()->getCastMemberInitialRect(id).height();
		break;
	case kTheBackColor:
		{
			if (castType != kCastShape) {
				warning("Lingo::getTheCast(): Field %d of cast %d not found", field, id);
				d.type = VOID;
				return d;
			}

			ShapeCast *shape = (ShapeCast *)_vm->getCurrentScore()->_loadedCast->getVal(id);
			d.u.i = shape->_bgCol;
		}
		break;
	case kTheForeColor:
		{
			if (castType != kCastShape) {
				warning("Lingo::getTheCast(): Field %d of cast %d not found", field, id);
				d.type = VOID;
				return d;
			}

			ShapeCast *shape = (ShapeCast *)_vm->getCurrentScore()->_loadedCast->getVal(id);
			d.u.i = shape->_fgCol;
		}
		break;
	case kTheLoaded:
		d.u.i = 1; //Not loaded handled above
		break;
	default:
		warning("Lingo::getTheCast(): Unprocessed getting field %d of cast %d", field, id);
		d.type = VOID;
	//TODO find out about String fields
	}

	return d;
}

void Lingo::setTheCast(Datum &id1, int field, Datum &d) {
	int id = 0;
	Score *score = _vm->getCurrentScore();

	if (!score) {
		warning("Lingo::setTheCast(): The cast %d field %d setting over non-active score", id, field);
		return;
	}

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::setTheCast(): Unknown the cast id type: %s", id1.type2str());
		return;
	}

	CastType castType = score->_loadedCast->getVal(id)->_type;
	CastInfo *castInfo = score->_castsInfo[id + score->_castIDoffset];

	switch (field) {
	case kTheCastType:
		// TODO: You can actually switch the cast type!?
		warning("Lingo::setTheCast(): Tried to switch cast type of %d", id);
		//cast->type = static_cast<CastType>(d.u.i);
		//cast->modified = 1;
		break;
	case kTheFileName:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		d.toString();
		castInfo->fileName = *d.u.s;
		break;
	case kTheName:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		d.toString();
		castInfo->name = *d.u.s;
		break;
	case kTheScriptText:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		d.toString();
		castInfo->script = *d.u.s;
		break;
	case kTheWidth:
		d.toInt();
		score->getCastMemberInitialRect(id).setWidth(d.u.i);
		score->setCastMemberModified(id);
		break;
	case kTheHeight:
		d.toInt();
		score->getCastMemberInitialRect(id).setHeight(d.u.i);
		score->setCastMemberModified(id);
		break;
	case kTheBackColor:
		{
			if (castType != kCastShape) {
				warning("Lingo::setTheCast(): Field %d of cast %d not found", field, id);
			}
			ShapeCast *shape = (ShapeCast *)score->_loadedCast->getVal(id);

			d.toInt();
			shape->_bgCol = d.u.i;
			shape->_modified = 1;
		}
		break;
	case kTheForeColor:
		{
			if (castType != kCastShape) {
				warning("Lingo::setTheCast(): Field %d of cast %d not found", field, id);
				return;
			}
			ShapeCast *shape = (ShapeCast *)score->_loadedCast->getVal(id);
			shape->_fgCol = d.u.i;
			shape->_modified = 1;
		}
		break;
	case kTheText:
		if (castType == kCastText) {
			if (score->_loadedCast->contains(id) && score->_loadedCast->getVal(id)->_type == kCastText) {
				d.toString();
				((TextCast *)score->_loadedCast->getVal(id))->setText(d.u.s->c_str());
			} else {
				warning("Lingo::setTheCast(): Unknown STXT cast id %d", id);
				return;
			}
		} else {
			warning("Lingo::setTheCast(): Unprocessed setting text of cast %d type %d", id, castType);
		}
		break;
	default:
		warning("Lingo::setTheCast(): Unprocessed setting field %d of cast %d", field, id);
	}
}

} // End of namespace Director
