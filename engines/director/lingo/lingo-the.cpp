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

#include "common/system.h"
#include "common/events.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macbutton.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/frame.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/score.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-the.h"

namespace Director {

class Sprite;

TheEntity entities[] = {
	{ kTheActorList,		"actorList",		false, 4 },	//				D4 property
	{ kTheBeepOn,			"beepOn",			false, 2 },	// D2 p
	{ kTheButtonStyle,		"buttonStyle",		false, 2 },	// D2 p
	{ kTheCast,				"cast",				true,  2 },	// D2
	{ kTheCastMembers,		"castmembers",		false, 3 },	//		 D3
	{ kTheCenterStage,		"centerStage",		false, 2 },	// D2 p
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
	{ kTheFrameTempo,		"frameTempo",		false, 4 },	//				D4 f
	{ kTheFreeBlock,		"freeBlock",		false, 2 },	// D2 f
	{ kTheFreeBytes,		"freeBytes",		false, 2 },	// D2 f
	{ kTheFullColorPermit,	"fullColorPermit",	false, 2 },	// D2 p
	{ kTheImageDirect,		"imageDirect",		false, 2 },	// D2 p
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
	{ kTheMachineType,		"machineType",		false, 2 },	// D2 f
	{ kTheMaxInteger,		"maxInteger",		false, 3 },	//		D3.1 f
	{ kTheMemorySize,		"memorySize",		false, 2 },	// D2 f
	{ kTheMenu,				"menu",				true,  3 },	//		 D3 p
	{ kTheMenuItem,			"menuitem",			true,  3 },	//		 D3 p
	{ kTheMenuItems,		"menuitems",		false, 3 },	//		 D3 f
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
	{ kTheMultiSound,		"multiSound",		false, 3 },	//		D3.1 f
	{ kTheOptionDown,		"optionDown",		false, 2 },	// D2 f
	{ kTheParamCount,		"paramCount",		false, 4 },	//				D4 f
	{ kThePathName,			"pathName",			false, 2 },	// D2 f
	{ kThePauseState,		"pauseState",		false, 2 },	// D2 f
	{ kThePi,				"pi",				false, 4 },	//				D4 f
	{ kThePerFrameHook,		"perFrameHook",		false, 2 },	// D2 p
	{ kThePreloadEventAbort,"preloadEventAbort",false, 4 },	//				D4 p
	{ kThePreLoadRAM,		"preLoadRAM",		false, 4 },	//				D4 p
	{ kTheQuickTimePresent,	"quickTimePresent",	false, 3 },	//		D3.1 f
	{ kTheRandomSeed,		"randomSeed",		false, 4 },	//				D4 p
	{ kTheResult,			"result",			false, 2 },	// D2 f
	{ kTheRightMouseDown,	"rightMouseDown",	false, 5 },	//						D5 f
	{ kTheRightMouseUp,		"rightMouseUp",		false, 5 },	//						D5 f
	{ kTheRomanLingo,		"romanLingo",		false, 3 },	//		D3.1 p
	{ kTheSearchCurrentFolder,"searchCurrentFolder",false,4 },//			D4 f
	{ kTheSearchPath,		"searchPath",		false, 4 },	//				D4 f
	{ kTheSelection,		"selection",		false, 2 },	// D2 f
	{ kTheSelEnd,			"selEnd",			false, 2 },	// D2 p
	{ kTheSelStart,			"selStart",			false, 2 },	// D2 p
	{ kTheShiftDown,		"shiftDown",		false, 2 },	// D2 f
	{ kTheSoundEnabled,		"soundEnabled",		false, 2 },	// D2 p
	{ kTheSoundLevel,		"soundLevel",		false, 2 },	// D2 p
	{ kTheSoundEntity,		"sound",			true,  4 },	// D4
	{ kTheSprite,			"sprite",			true,  2 },	// D4 p
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
	{ kTheNOEntity, NULL, false, 0 }
};

TheEntityField fields[] = {
	{ kTheSprite,	"backColor",	kTheBackColor,	2 },// D2 p
	{ kTheSprite,	"blend",		kTheBlend,		4 },//				D4 p
	{ kTheSprite,	"bottom",		kTheBottom,		2 },// D2 p
	{ kTheSprite,	"castNum",		kTheCastNum,	2 },// D2 p
	{ kTheSprite,	"constraint",	kTheConstraint, 2 },// D2 p
	{ kTheSprite,	"cursor",		kTheCursor,		2 },// D2 p
	{ kTheSprite,	"editableText", kTheEditableText,4 },//				D4 p
	{ kTheSprite,	"foreColor",	kTheForeColor,	2 },// D2 p
	{ kTheSprite,	"height",		kTheHeight,		2 },// D2 p
	{ kTheSprite,	"immediate",	kTheImmediate,	2 },// D2 p
	{ kTheSprite,	"ink",			kTheInk,		2 },// D2 p
	{ kTheSprite,	"left",			kTheLeft,		2 },// D2 p
	{ kTheSprite,	"lineSize",		kTheLineSize,	2 },// D2 p
	{ kTheSprite,	"loc",			kTheLoc,		4 },//				D4 p ???
	{ kTheSprite,	"locH",			kTheLocH,		2 },// D2 p
	{ kTheSprite,	"locV",			kTheLocV,		2 },// D2 p
	{ kTheSprite,	"moveableSprite",kTheMoveableSprite,4 },//			D4 p
	{ kTheSprite,	"pattern",		kThePattern,	2 },// D2 p
	{ kTheSprite,	"puppet",		kThePuppet,		2 },// D2 p
	{ kTheSprite,	"rect",			kTheRect,		4 },//				D4 p ???
	{ kTheSprite,	"right",		kTheRight,		2 },// D2 p
	{ kTheSprite,	"scoreColor",	kTheScoreColor,	4 },//				D4 p
	{ kTheSprite,	"scriptNum",	kTheScriptNum,	4 },//				D4 p
	{ kTheSprite,	"stretch",		kTheStretch,		2 },// D2 p
	{ kTheSprite,	"top",			kTheTop,		2 },// D2 p
	{ kTheSprite,	"trails",		kTheTrails,		3 },//		D3.1 p
	{ kTheSprite,	"type",			kTheType,		2 },// D2 p
	{ kTheSprite,	"visibility",	kTheVisibility,	3 },//		D3.1 p
	{ kTheSprite,	"visible",		kTheVisible,	4 },//				D4 p
	{ kTheSprite,	"width",		kTheWidth,		2 },// D2 p

	// Common cast fields
	{ kTheCast,		"castType",		kTheCastType,	4 },//				D4 p
	{ kTheCast,		"filename",		kTheFileName,	4 },//				D4 p
	{ kTheCast,		"height",		kTheHeight,		4 },//				D4 p
	{ kTheCast,		"loaded",		kTheLoaded,		4 },//				D4 p
	{ kTheCast,		"modified",		kTheModified,	4 },//				D4 p
	{ kTheCast,		"name",			kTheName,		3 },//		D3 p
	{ kTheCast,		"number",		kTheNumber,		3 },//		D3 p
	{ kTheCast,		"rect",			kTheRect,		4 },//				D4 p
	{ kTheCast,		"purgePriority",kThePurgePriority,4 },//			D4 p // 0 Never purge, 1 Purge Last, 2 Purge next, 2 Purge normal
	{ kTheCast,		"scriptText",	kTheScriptText,	4 },//				D4 p
	{ kTheCast,		"width",		kTheWidth,		4 },//				D4 p

	// Shape fields
	{ kTheCast,		"backColor",	kTheBackColor,	4 },//				D4 p
	{ kTheCast,		"foreColor",	kTheForeColor,	4 },//				D4 p

	// Digital video fields
	{ kTheCast,		"center",		kTheCenter,		4 },//				D4 p
	{ kTheCast,		"controller",	kTheController,	3 },//			D3.1 p
	{ kTheCast,		"crop",			kTheCrop,		4 },//				D4 p
	{ kTheCast,		"directToStage",kTheDirectToStage,3 },//		D3.1 p
	{ kTheCast,		"duration",		kTheDuration,	3 },//			D3.1 p
	{ kTheCast,		"frameRate",	kTheFrameRate,	4 },//				D4 p
	{ kTheCast,		"loop",			kTheLoop,		3 },//			D3.1 p
	{ kTheSprite,	"movieRate",	kTheMovieRate,	3 },//			D3.1 P
	{ kTheSprite,	"movieTime",	kTheMovieTime,	3 },//			D3.1 P
	{ kTheCast,		"pausedAtStart",kThePausedAtStart,4 },//			D4 p
	{ kTheCast,		"preLoad",		kThePreLoad,	3 },//			D3.1 p
	{ kTheCast,		"sound",		kTheSound,		3 },//			D3.1 p // 0-1 off-on
	{ kTheSprite,	"startTime",	kTheStartTime,	3 },//			D3.1 p
	{ kTheSprite,	"stopTime",		kTheStopTime,	3 },//			D3.1 p
	{ kTheCast,		"video",		kTheVideo,		4 },//				D4 p
	{ kTheSprite,	"volume",		kTheVolume,		3 },//		D3.1 p

	// Bitmap fields
	{ kTheCast,		"depth",		kTheDepth,		4 },//				D4 p
	{ kTheCast,		"regPoint",		kTheRegPoint,	4 },//				D4 p
	{ kTheCast,		"palette",		kThePalette,	4 },//				D4 p
	{ kTheCast,		"picture",		kThePicture,	3 },//		D3 p

	// TextCast fields
	{ kTheCast,		"hilite",		kTheHilite,		2 },// D2 p
	{ kTheCast,		"size",			kTheSize,		3 },//		D3.1 p
	{ kTheCast,		"text",			kTheText,		2 },// D2 p

	// Field fields
	{ kTheField,	"text",			kTheText,		3 },// 		D3 p undocumented
	{ kTheField,	"textAlign",	kTheTextAlign,	3 },//		D3 p
	{ kTheField,	"textFont",		kTheTextFont,	3 },//		D3 p
	{ kTheField,	"textHeight",	kTheTextHeight,	3 },//		D3 p
	{ kTheField,	"textSize",		kTheTextSize,	3 },//		D3 p
	{ kTheField,	"textStyle",	kTheTextStyle,	3 },//		D3 p

	{ kTheWindow,	"drawRect",		kTheDrawRect,	4 },//				D4 p
	{ kTheWindow,	"fileName",		kTheFileName,	4 },//				D4 p
	{ kTheWindow,	"modal",		kTheModal,		4 },//				D4 p
	{ kTheWindow,	"rect",			kTheRect,		4 },//				D4 p
	{ kTheWindow,	"title",		kTheTitle,		4 },//				D4 p
	{ kTheWindow,	"titleVisible",	kTheTitleVisible,4 },//				D4 p
	{ kTheWindow,	"sourceRect",	kTheSourceRect,	4 },//				D4 p
	{ kTheWindow,	"visible",		kTheVisible,	4 },//				D4 p
	{ kTheWindow,	"windowType",	kTheWindowType,	4 },//				D4 p

	{ kTheMenuItem,	"checkmark",	kTheCheckMark,	3 },//		D3 p
	{ kTheMenuItem, "enabled",		kTheEnabled,	3 },//		D3 p
	{ kTheMenuItem, "name",			kTheName,		3 },//		D3 p
	{ kTheMenuItem, "script",		kTheScript,		3 },//		D3 p
	{ kTheMenuItems,"number",		kTheNumber,		3 },//		D3 p 			// number of menuitems of menu <xx>

	{ kTheMenu,		"name",			kTheName,		3 },//		D3 p

	{ kTheCastMembers,	"number",	kTheNumber,		3 },// 		D3 p

	{ kTheDate,		"short",		kTheShort,		3 },//		D3 f
	{ kTheDate,		"long",			kTheLong,		3 },//		D3 f
	{ kTheDate,		"abbreviated",	kTheAbbr,		3 },//		D3 f
	{ kTheDate,		"abbrev",		kTheAbbr,		3 },//		D3 f
	{ kTheDate,		"abbr",			kTheAbbr,		3 },//		D3 f
	{ kTheTime,		"short",		kTheShort,		3 },//		D3 f
	{ kTheTime,		"long",			kTheLong,		3 },//		D3 f
	{ kTheTime,		"abbreviated",	kTheAbbr,		3 },//		D3 f
	{ kTheTime,		"abbrev",		kTheAbbr,		3 },//		D3 f
	{ kTheTime,		"abbr",			kTheAbbr,		3 },//		D3 f

	{ kTheSoundEntity,"volume",		kTheVolume,		4 },//				D4 p

	{ kTheNOEntity, NULL, kTheNOField, 0 }
};

void Lingo::initTheEntities() {
	_objectEntityId = kTheObject;

	TheEntity *e = entities;
	_entityNames.resize(kTheMaxTheEntityType);

	while (e->entity != kTheNOEntity) {
		if (e->version <= _vm->getVersion()) {
			_theEntities[e->name] = e;

			_entityNames[e->entity] = e->name;
		}

		e++;
	}

	TheEntityField *f = fields;
	_fieldNames.resize(kTheMaxTheFieldType);

	while (f->entity != kTheNOEntity) {
		if (f->version <= _vm->getVersion()) {
			_theEntityFields[Common::String::format("%d%s", f->entity, f->name)] = f;

			_fieldNames[f->field] = f->name;
		}

		// Store all fields for kTheObject
		_theEntityFields[Common::String::format("%d%s", _objectEntityId, f->name)] = f;

		f++;
	}
}

const char *Lingo::entity2str(int id) {
	static char buf[20];

	if (id && id < kTheMaxTheEntityType && !_entityNames[id].empty())
		return _entityNames[id].c_str();

	snprintf(buf, 19, "#%d", id);

	return (const char *)buf;
}
const char *Lingo::field2str(int id) {
	static char buf[20];

	if (id && id < kTheMaxTheFieldType && !_fieldNames[id].empty())
		return _fieldNames[id].c_str();

	snprintf(buf, 19, "#%d", id);

	return (const char *)buf;
}

Datum Lingo::getTheEntity(int entity, Datum &id, int field) {
	if (debugChannelSet(3, kDebugLingoExec)) {
		debugC(3, kDebugLingoExec, "Lingo::getTheEntity(%s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field));
	}

	Datum d;

	if (!_vm->getCurrentScore()) {
		warning("Lingo::getTheEntity(): Score is missing");
		d.type = VOID;

		return d;
	}

	switch (entity) {
	case kTheCast:
		d = getTheCast(id, field);
		break;
	case kTheClickOn:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_currentClickOnSpriteId;
		break;
	case kTheColorDepth:
		// bpp. 1, 2, 4, 8, 32
		d.type = INT;
		d.u.i = _vm->_colorDepth;
		break;
	case kTheColorQD:
		d.type = INT;
		d.u.i = 1;
		break;
	case kTheFloatPrecision:
		d.type = INT;
		d.u.i = _floatPrecision;
		break;
	case kTheFrame:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->getCurrentFrame();
		break;
	case kTheKey:
		d.type = STRING;
		d.u.s = new Common::String(_vm->_key);
		break;
	case kTheKeyCode:
		d.type = INT;
		d.u.i = _vm->_keyCode;
		break;
	case kTheKeyDownScript:
		d.type = STRING;
		if (_archives[kArchMain].primaryEventHandlers.contains(kEventKeyDown))
			d.u.s = new Common::String(_archives[kArchMain].primaryEventHandlers[kEventKeyDown]);
		else
			d.u.s = new Common::String();
		break;
	case kTheKeyUpScript:
		d.type = STRING;
		if (_archives[kArchMain].primaryEventHandlers.contains(kEventKeyUp))
			d.u.s = new Common::String(_archives[kArchMain].primaryEventHandlers[kEventKeyUp]);
		else
			d.u.s = new Common::String();
		break;
	case kTheLastClick:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - _vm->getCurrentScore()->_lastClickTime;
		break;
	case kTheLastEvent:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - _vm->getCurrentScore()->_lastEventTime;
		break;
	case kTheLastFrame:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_frames.size() - 1;
		break;
	case kTheLastKey:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - _vm->getCurrentScore()->_lastKeyTime;
		break;
	case kTheLastRoll:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - _vm->getCurrentScore()->_lastRollTime;
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
	case kTheMovie:
	case kTheMovieName:
		d.type = STRING;
		d.u.s = new Common::String(_vm->getCurrentScore()->getMacName());
		break;
	case kTheMoviePath:
	case kThePathName:
		d.type = STRING;
		d.u.s = new Common::String(_vm->getCurrentPath());
		break;
	case kTheMouseCast:
		{
			Common::Point pos = g_system->getEventManager()->getMousePos();
			Score *sc = _vm->getCurrentScore();
			uint16 spriteId = sc->getSpriteIDFromPos(pos);
			d.type = INT;
			d.u.i = sc->getSpriteById(spriteId)->_castId;
			if (d.u.i == 0)
				d.u.i = -1;
		}
		break;
	case kTheMouseDown:
		d.type = INT;
		d.u.i = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT | 1 << Common::MOUSE_BUTTON_RIGHT) ? 1 : 0;
		break;
	case kTheMouseDownScript:
		d.type = STRING;
		if (_archives[kArchMain].primaryEventHandlers.contains(kEventMouseDown))
			d.u.s = new Common::String(_archives[kArchMain].primaryEventHandlers[kEventMouseDown]);
		else
			d.u.s = new Common::String();
		break;
	case kTheMouseH:
		d.type = INT;
		d.u.i = g_system->getEventManager()->getMousePos().x;
		break;
	case kTheMouseV:
		d.type = INT;
		d.u.i = g_system->getEventManager()->getMousePos().y;
		break;
	case kTheMouseUp:
		d.type = INT;
		d.u.i = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT | 1 << Common::MOUSE_BUTTON_RIGHT) ? 0 : 1;
		break;
	case kTheMouseUpScript:
		d.type = STRING;
		if (_archives[kArchMain].primaryEventHandlers.contains(kEventMouseUp))
			d.u.s = new Common::String(_archives[kArchMain].primaryEventHandlers[kEventMouseUp]);
		else
			d.u.s = new Common::String();
		break;
	case kTheMultiSound:
		// We always support multiple sound channels!
		d.type = INT;
		d.u.i = 1;
		break;
	case kThePerFrameHook:
		d = _perFrameHook;
		break;
	case kThePi:
		d.type = FLOAT;
		d.u.f = M_PI;
		break;
	case kTheQuickTimePresent:
		// QuickTime is always present for scummvm
		d.type = INT;
		d.u.i = 1;
		break;
	case kTheSoundEntity:
		{
			switch (field) {
			case kTheVolume:
				{
					SoundChannel *chan = _vm->getSoundManager()->getChannel(id.asInt());
					if (chan) {
						d.type = INT;
						d.u.i = (int)chan->volume;
					}
				}
				break;
			default:
				warning("Lingo::getTheEntity(): Unprocessed getting field \"%s\" of entity %s", field2str(field), entity2str(entity));
				break;
			}
		}
		break;
	case kTheSprite:
		d = getTheSprite(id, field);
		break;
	case kTheStageBottom:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_movieRect.bottom;
		break;
	case kTheStageLeft:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_movieRect.left;
		break;
	case kTheStageRight:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_movieRect.right;
		break;
	case kTheStageTop:
		d.type = INT;
		d.u.i = _vm->getCurrentScore()->_movieRect.top;
		break;
	case kTheStillDown:
		d.type = INT;
		d.u.i = _vm->_wm->_mouseDown;
		break;
	case kTheTimer:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - _vm->getCurrentScore()->_lastTimerReset;
		break;
	case kTheTimeoutScript:
		d.type = STRING;
		if (_archives[kArchMain].primaryEventHandlers.contains(kEventTimeout))
			d.u.s = new Common::String(_archives[kArchMain].primaryEventHandlers[kEventTimeout]);
		else
			d.u.s = new Common::String();
		break;
	default:
		warning("Lingo::getTheEntity(): Unprocessed getting field \"%s\" of entity %s", field2str(field), entity2str(entity));
		break;
	}

	return d;
}

void Lingo::setTheEntity(int entity, Datum &id, int field, Datum &d) {
	if (debugChannelSet(3, kDebugLingoExec)) {
		debugC(3, kDebugLingoExec, "Lingo::setTheEntity(%s, %s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field), d.asString(true).c_str());
	}

	switch (entity) {
	case kTheButtonStyle:
		if (d.asInt())
			g_director->_wm->_mode = Director::wmMode | Graphics::kWMModeButtonDialogStyle;
		else
			g_director->_wm->_mode = Director::wmMode;
		break;
	case kTheCast:
		setTheCast(id, field, d);
		break;
	case kTheColorDepth:
		_vm->_colorDepth = d.asInt();

		// bpp. 1, 2, 4, 8, 32
		warning("STUB: Lingo::setTheEntity(): Set color depth to %d", _vm->_colorDepth);
		break;
	case kTheField:
		setTheField(id, field, d);
		break;
	case kTheFloatPrecision:
		_floatPrecision = d.asInt();
		_floatPrecision = MAX(0, MIN(_floatPrecision, 19)); // 0 to 19
		_floatPrecisionFormat = Common::String::format("%%.%df", _floatPrecision);
		break;
	case kTheKeyDownScript:
		setPrimaryEventHandler(kEventKeyDown, d.asString());
		break;
	case kTheKeyUpScript:
		setPrimaryEventHandler(kEventKeyUp, d.asString());
		break;
	case kTheMouseDownScript:
		setPrimaryEventHandler(kEventMouseDown, d.asString());
		break;
	case kTheMouseUpScript:
		setPrimaryEventHandler(kEventMouseUp, d.asString());
		break;
	case kThePerFrameHook:
		_perFrameHook = d;
		break;
	case kTheSoundEntity:
		{
			switch (field) {
			case kTheVolume:
				{
					SoundChannel *chan = _vm->getSoundManager()->getChannel(id.asInt());
					if (chan) {
						chan->volume = (byte)d.asInt();
					}
				}
				break;
			default:
				warning("Lingo::setTheEntity(): Unprocessed getting field \"%s\" of entity %s", field2str(field), entity2str(entity));
				break;
			}
		}
		break;
	case kTheStageColor: {
		Score *score = _vm->getCurrentScore();
		score->_stageColor = _vm->transformColor(d.asInt());
		if (score->_surface) {
			score->_surface->clear(score->_stageColor);
			score->renderFrame(score->getCurrentFrame(), true);
		} else {
			warning("setStageColor: score has no surface, skipping");
		}
	}
		break;
	case kTheSprite:
		setTheSprite(id, field, d);
		break;
	case kTheTimeoutScript:
		setPrimaryEventHandler(kEventTimeout, d.asString());
		break;
	default:
		warning("Lingo::setTheEntity(): Unprocessed setting field \"%s\" of entity %s", field2str(field), entity2str(entity));
	}
}

void Lingo::setTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId, Datum &d) {
	warning("STUB: setTheMenuItemEntity(%s, %s, %s, %s, %s)", entity2str(entity), menuId.asString(true).c_str(), field2str(field),
				menuItemId.asString(true).c_str(), d.asString(true).c_str());
}

Datum Lingo::getTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId) {
	warning("STUB: getTheMenuItemEntity(%s, %s, %s, %s)", entity2str(entity), menuId.asString(true).c_str(), field2str(field),
				menuItemId.asString(true).c_str());

	return Datum();
}

Datum Lingo::getTheSprite(Datum &id1, int field) {
	Datum d;
	int id = 0;
	Score *score = _vm->getCurrentScore();

	if (!score) {
		warning("Lingo::getTheSprite(): The sprite %d field \"%s\" setting over non-active score", id, field2str(field));
		return d;
	}

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::getTheSprite(): Unknown the sprite id type: %s", id1.type2str());
		return d;
	}

	Channel *channel = score->getChannelById(id);
	if (!channel)
		return d;

	Sprite *sprite = channel->_sprite;
	if (!sprite)
		return d;

	d.type = INT;

	switch (field) {
	case kTheBackColor:
		d.u.i = sprite->_backColor;
		break;
	case kTheBlend:
		d.u.i = sprite->_blend;
		break;
	case kTheBottom:
		d.u.i = channel->getBbox().bottom;
		break;
	case kTheCastNum:
		d.u.i = sprite->_castId;
		break;
	case kTheConstraint:
		d.u.i = channel->_constraint;
		break;
	case kTheEditableText:
		d.u.i = sprite->_cast ? sprite->_cast->isEditable() : 0;
		break;
	case kTheForeColor:
		d.u.i = sprite->_foreColor;
		break;
	case kTheHeight:
		d.u.i = sprite->_height;
		break;
	case kTheImmediate:
		d.u.i = sprite->_immediate;
		break;
	case kTheInk:
		d.u.i = sprite->_ink;
		break;
	case kTheLeft:
		d.u.i = channel->getBbox().left;
		break;
	case kTheLineSize:
		d.u.i = sprite->_thickness & 0x3;
		break;
	case kTheLocH:
		d.u.i = channel->_currentPoint.x;
		break;
	case kTheLocV:
		d.u.i = channel->_currentPoint.y;
		break;
	case kTheMoveableSprite:
		d.u.i = sprite->_moveable;
		break;
	case kTheMovieRate:
		d.u.i = sprite->_movieRate;
		break;
	case kTheMovieTime:
		d.u.i = sprite->_movieTime;
		break;
	case kThePattern:
		d.u.i = sprite->getPattern();
		break;
	case kThePuppet:
		d.u.i = sprite->_puppet;
		break;
	case kTheRight:
		d.u.i = channel->getBbox().right;
		break;
	case kTheStartTime:
		d.u.i = sprite->_startTime;
		break;
	case kTheStopTime:
		d.u.i = sprite->_stopTime;
		break;
	case kTheStretch:
		d.u.i = sprite->_stretch;
		break;
	case kTheTop:
		d.u.i = channel->getBbox().top;
		break;
	case kTheTrails:
		d.u.i = sprite->_trails;
		break;
	case kTheType:
		d.u.i = sprite->_spriteType;
		break;
	case kTheVisibility:
	case kTheVisible:
		d.u.i = (channel->_visible ? 1 : 0);
		break;
	case kTheVolume:
		d.u.i = sprite->_volume;
		break;
	case kTheWidth:
		d.u.i = sprite->_width;
		break;
	default:
		warning("Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
		d.type = VOID;
	}

	return d;
}

void Lingo::setTheSprite(Datum &id1, int field, Datum &d) {
	int id = 0;
	Score *score = _vm->getCurrentScore();

	if (!score) {
		warning("Lingo::setTheSprite(): The sprite %d field \"%s\" setting over non-active score", id, field2str(field));
		return;
	}

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::setTheSprite(): Unknown the sprite id type: %s", id1.type2str());
		return;
	}

	Channel *channel = score->getChannelById(id);
	if (!channel)
		return;

	Sprite *sprite = channel->_sprite;
	if (!sprite)
		return;

	if (!sprite->_enabled)
		sprite->_enabled = true;

	switch (field) {
	case kTheBackColor:
		sprite->_backColor = d.asInt();
		break;
	case kTheBlend:
		sprite->_blend = d.asInt();
		break;
	case kTheCastNum:
		// Don't cause unnecessary flashing
		if (d.asInt() != sprite->_castId) {
			score->markDirtyRect(channel->getBbox());
			sprite->setCast(d.asInt());
		}
		break;
	case kTheConstraint:
		if (d.type == CASTREF) {
			// Reference: Cast ID
			// Find the first channel that uses this cast.
			for (uint i = 0; i < score->_channels.size(); i++)
				if (score->_channels[i]->_sprite->_castId == d.u.i)
					d.u.i = i;
		}

		channel->_constraint = d.u.i;
		break;
	case kTheEditableText:
		sprite->_cast ? sprite->_cast->setEditable(d.asInt()) : false;
		break;
	case kTheForeColor:
		sprite->_foreColor = d.asInt();
		break;
	case kTheHeight:
		if (sprite->_puppet && sprite->_stretch) {
			score->markDirtyRect(channel->getBbox());
			sprite->_height = d.asInt();
		}
		break;
	case kTheImmediate:
		sprite->_immediate = d.asInt();
		break;
	case kTheInk:
		sprite->_ink = static_cast<InkType>(d.asInt());
		break;
	case kTheLineSize:
		sprite->_thickness = d.asInt();
		break;
	case kTheLocH:
		channel->addDelta(Common::Point(d.asInt() - channel->_currentPoint.x, 0));
		break;
	case kTheLocV:
		channel->addDelta(Common::Point(0, d.asInt() - channel->_currentPoint.y));
		break;
	case kTheMoveableSprite:
		sprite->_moveable = d.asInt();
		break;
	case kTheMovieRate:
		sprite->_movieRate = d.asInt();
		break;
	case kTheMovieTime:
		sprite->_movieTime = d.asInt();
		break;
	case kThePattern:
		sprite->setPattern(d.asInt());
		break;
	case kThePuppet:
		sprite->_puppet = d.asInt();
		if (!d.asInt()) {
			// TODO: Properly reset sprite properties after puppet disabled.
			sprite->_moveable = false;
		}
		break;
	case kTheStartTime:
		sprite->_startTime = d.asInt();
		break;
	case kTheStopTime:
		sprite->_stopTime = d.asInt();
		break;
	case kTheStretch:
		sprite->_stretch = d.asInt();
		break;
	case kTheTrails:
		sprite->_trails = d.asInt();
		break;
	case kTheType:
		sprite->_spriteType = static_cast<SpriteType>(d.asInt());
		break;
	case kTheVisibility:
	case kTheVisible:
		channel->_visible = (d.asInt() == 0 ? false : true);
		break;
	case kTheVolume:
		sprite->_volume = d.asInt();
		break;
	case kTheWidth:
		if (sprite->_puppet && sprite->_stretch) {
			score->markDirtyRect(channel->getBbox());
			sprite->_width = d.asInt();
		}
		break;
	default:
		warning("Lingo::setTheSprite(): Unprocessed setting field \"%s\" of sprite", field2str(field));
	}
	sprite->_dirty = true;
}

Datum Lingo::getTheCast(Datum &id1, int field) {
	Datum d;
	int id = g_lingo->castIdFetch(id1);

	Score *score = _vm->getCurrentScore();
	// Setting default type
	d.type = INT;

	if (!score) {
		warning("Lingo::getTheCast(): No score loaded");
		return d;
	}

	Cast *member = _vm->getCastMember(id);
	if (!member) {
		if (field == kTheLoaded) {
			d.u.i = 0;
		} else {
			warning("Lingo::getTheCast(): Cast %d not found", id);
		}
		return d;
	}

	if (field == kTheHilite) {
		d.u.i = member->_hilite;
		return d;
	}

	CastType castType = member->_type;
	CastInfo *castInfo = nullptr;
	if (score->_castsInfo.contains(id)) {
		castInfo = score->_castsInfo.getVal(id);
	} else {
		Score *shared = _vm->getSharedScore();
		if (shared && shared->_castsInfo.contains(id)) {
			castInfo = shared->_castsInfo.getVal(id);
		} else {
			warning("Lingo::getTheCast(): Cast info for %d not found", id);
			return d;
		}
	}

	switch (field) {
	case kTheBackColor:
		member->getColors(nullptr, &d.u.i);
		break;
	case kTheCastType:
		d.u.i = castType;
		break;
	case kTheFileName:
		d = Datum(castInfo->fileName);
		break;
	case kTheForeColor:
		member->getColors(&d.u.i, nullptr);
		break;
	case kTheHeight:
		d.u.i = score->getCastMemberInitialRect(id).height();
		break;
	case kTheLoaded:
		d.u.i = 1; //Not loaded handled above
		break;
	case kTheName:
		d = Datum(castInfo->name);
		break;
	case kTheScriptText:
		d = Datum(castInfo->script);
		break;
	case kTheText:
		{
			Common::String text;
			if (castType == kCastText) {
				if (member && member->_type == kCastText) {
					text = ((TextCast *)member)->getText();
				} else {
					warning("Lingo::getTheCast(): Unknown STXT cast id %d", id);
				}
			} else {
				warning("Lingo::getTheCast(): Unprocessed getting text of cast %d type %d", id, castType);
			}
			d = Datum(text);
		}
		break;
	case kTheWidth:
		d.u.i = score->getCastMemberInitialRect(id).width();
		break;
	case kTheNumber:
		d.u.i = id;
		break;
	default:
		warning("Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		d.type = VOID;
	//TODO find out about String fields
	}

	return d;
}

void Lingo::setTheCast(Datum &id1, int field, Datum &d) {
	int id = 0;
	Score *score = _vm->getCurrentScore();

	if (!score) {
		warning("Lingo::setTheCast(): The cast %d field \"%s\" setting over non-active score", id, field2str(field));
		return;
	}

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::setTheCast(): Unknown the cast id type: %s", id1.type2str());
		return;
	}

	Cast *member = _vm->getCastMember(id);
	if (!member) {
		warning("Lingo::setTheCast(): Cast id %d doesn't exist", id);
		return;
	}
	CastType castType = member->_type;
	CastInfo *castInfo = score->_castsInfo[id];

	switch (field) {
	case kTheBackColor: {
		int color = _vm->transformColor(d.asInt());
		member->setColors(nullptr, &color);
		break;
	}
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
		castInfo->fileName = d.asString();
		break;
	case kTheForeColor: {
		int color = _vm->transformColor(d.asInt());
		member->setColors(&color, nullptr);
		break;
	}
	case kTheHeight:
		score->getCastMemberInitialRect(id).setHeight(d.asInt());
		score->setCastMemberModified(id);
		break;
	case kTheHilite:
		// TODO: Understand how texts can be selected programmatically as well.
		if (member->_type == kCastButton) {
			TextCast *button = (TextCast *)member;
			if ((bool)d.asInt() !=  member->_hilite) {
				((Graphics::MacButton *) button->_widget)->invertInner();
				button->_hilite = !!d.asInt();
			}
		} else {
			warning("Lingo::setTheCast: Attempted to set hilite of unsupported cast type");
		}
		break;
	case kTheName:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		castInfo->name = d.asString();
		break;
	case kTheScriptText:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		// FIXME: What if the cast member is in a different archive than the current one?
		addCode(d.u.s->c_str(), _archiveIndex, kCastScript, id);

		castInfo->script = d.asString();
		break;
	case kTheText:
		if (castType == kCastText) {
			if (member->_type == kCastText) {
				((TextCast *)member)->setText(d.asString().c_str());
			} else {
				warning("Lingo::setTheCast(): Unknown STXT cast id %d", id);
				return;
			}
		} else {
			warning("Lingo::setTheCast(): Unprocessed setting text of cast %d type %d", id, castType);
		}
		break;
	case kTheWidth:
		score->getCastMemberInitialRect(id).setWidth(d.asInt());
		score->setCastMemberModified(id);
		break;
	default:
		warning("Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
	}
}

Datum Lingo::getTheField(Datum &id1, int field) {
	Datum d;
	int id = g_lingo->castIdFetch(id1);

	Cast *member = _vm->getCastMember(id);
	if (!member) {
		warning("Lingo::getTheField(): Cast id %d doesn't exist", id);
		return d;
	} else if (member->_type != kCastText) {
		warning("Lingo::getTheField(): Cast id %d is not a field", id);
		return d;
	}

	switch (field) {
	case kTheTextAlign:
		d.type = STRING;
		switch (((Graphics::MacText *)member->_widget)->getAlign()) {
		case Graphics::kTextAlignLeft:
			d.u.s = new Common::String("left");
			break;
		case Graphics::kTextAlignCenter:
			d.u.s = new Common::String("center");
			break;
		case Graphics::kTextAlignRight:
			d.u.s = new Common::String("right");
			break;
		case Graphics::kTextAlignInvalid:
		default:
			warning("Lingo::getTheField: Invalid text align spec");
			break;
		}
		break;
	default:
		warning("Lingo::getTheField(): Unprocessed getting field \"%s\" of field %d", field2str(field), id);
	}

	return d;
}

void Lingo::setTheField(Datum &id1, int field, Datum &d) {
	int id = 0;

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Lingo::setTheField(): Unknown the cast id type: %s", id1.type2str());
		return;
	}

	Cast *member = _vm->getCastMember(id);
	if (!member) {
		warning("Lingo::setTheField(): Cast id %d doesn't exist", id);
		return;
	} else if (member->_type != kCastText) {
		warning("Lingo::setTheField(): Cast id %d is not a field", id);
	}

	switch (field) {
	case kTheTextAlign: {
		Common::String select = d.asString(true);
		select.toLowercase();

		Graphics::TextAlign align;
		if (select == "\"left\"") {
			align = Graphics::kTextAlignLeft;
		} else if (select == "\"center\"") {
			align = Graphics::kTextAlignCenter;
		} else if (select == "\"right\"") {
			align = Graphics::kTextAlignRight;
		} else {
			warning("Lingo::setTheField: Unknown text align spec: %s", d.asString(true).c_str());
			break;
		}

		((Graphics::MacText *)member->_widget)->setAlignOffset(align);
		((Graphics::MacText *)member->_widget)->draw();
		member->_modified = true;
		break;
	}
	default:
		warning("Lingo::setTheField(): Unprocessed setting field \"%s\" of field %d", field2str(field), id);
	}
}

Datum Lingo::getObjectProp(Datum &obj, Common::String &propName) {
	Datum d;
	if (obj.type == OBJECT) {
		if (obj.u.obj->hasProp(propName)) {
			return obj.u.obj->getProp(propName);
		} else {
			warning("Lingo::getObjectProp: Object <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
	} else if (obj.type == PARRAY) {
		int index = LC::compareArrays(LC::eqData, obj, propName, true).u.i;
		if (index > 0) {
			d = obj.u.parr->operator[](index - 1).v;
		}
		return d;
	} else {
		warning("Lingo::getObjectProp: Invalid object: %s", obj.asString(true).c_str());
	}
	return d;
}

void Lingo::setObjectProp(Datum &obj, Common::String &propName, Datum &val) {
	if (obj.type == OBJECT) {
		if (obj.u.obj->hasProp(propName)) {
			obj.u.obj->getProp(propName) = val;
		} else {
			warning("Lingo::setObjectProp: Object <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
	} else if (obj.type == PARRAY) {
		int index = LC::compareArrays(LC::eqData, obj, propName, true).u.i;
		if (index > 0) {
			obj.u.parr->operator[](index - 1).v = val;
		} else {
			PCell cell = PCell(propName, val);
			obj.u.parr->push_back(cell);
		}
	} else {
		warning("Lingo::setObjectProp: Invalid object: %s", obj.asString(true).c_str());
	}
}

} // End of namespace Director
