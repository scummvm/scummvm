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
#include "common/fs.h"
#include "common/platform.h"
#include "director/types.h"
#include "graphics/macgui/macbutton.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/debugger.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/score.h"
#include "director/window.h"
#include "director/castmember/castmember.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/text.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-the.h"

namespace Director {

class Sprite;

const TheEntity entities[] = {
	{ kTheActorList,		"actorList",		false, 400, false },	//			D4 property
	{ kTheBeepOn,			"beepOn",			false, 200, false },	// D2 p
	{ kTheButtonStyle,		"buttonStyle",		false, 200, false },	// D2 p
	{ kTheCast,				"cast",				true,  200, false },	// D2
	{ kTheCastMembers,		"castmembers",		false, 300, false },	//		 D3
	{ kTheCenterStage,		"centerStage",		false, 200, false },	// D2 p
	{ kTheCheckBoxAccess,	"checkBoxAccess",	false, 200, false },	// D2 p
	{ kTheCheckBoxType,		"checkBoxType",		false, 200, false },	// D2 p
	{ kTheChunk,			"chunk",			true,  300, false },	//		D3
	{ kTheClickLoc,			"clickLoc",			false, 400, true },	// 			D4 function
	{ kTheClickOn,			"clickOn",			false, 200, true },	// D2 f
	{ kTheColorDepth,		"colorDepth",		false, 200, false },	// D2 p
	{ kTheColorQD,			"colorQD",			false, 200, true },	// D2 f
	{ kTheCommandDown,		"commandDown",		false, 200, true },	// D2 f
	{ kTheControlDown,		"controlDown",		false, 200, true },	// D2 f
	{ kTheDate,				"date",				false, 300, true },	//		D3 f
	{ kTheDeskTopRectList,	"deskTopRectList",	false, 500, true },	// D5 p
	{ kTheDoubleClick,		"doubleClick",		false, 200, true },	// D2 f
	{ kTheExitLock,			"exitLock",			false, 200, false },	// D2 p
	{ kTheField,			"field",			true,  300, false },	//		D3
	{ kTheFixStageSize,		"fixStageSize",		false, 200, false },	// D2 p
	{ kTheFloatPrecision,	"floatPrecision",	false, 300, false },	//		D3 p
	{ kTheFrame,			"frame",			false, 200, true },	// D2 f
	{ kTheFrameLabel,		"frameLabel",		false, 400, false },	//			D4 p
	{ kTheFrameScript,		"frameScript",		false, 400, false },	//			D4 p
	{ kTheFramePalette,		"framePalette",		false, 400, false },	//			D4 p
	{ kTheFrameTempo,		"frameTempo",		false, 400, true },	//			D4 f
	{ kTheFreeBlock,		"freeBlock",		false, 200, true },	// D2 f
	{ kTheFreeBytes,		"freeBytes",		false, 200, true },	// D2 f
	{ kTheFullColorPermit,	"fullColorPermit",	false, 200, false },	// D2 p
	{ kTheImageDirect,		"imageDirect",		false, 200, false },	// D2 p
	{ kTheItemDelimiter,	"itemDelimiter",	false, 400, false },	//			D4 p
	{ kTheKey,				"key",				false, 200, true },	// D2 f
	{ kTheKeyCode,			"keyCode",			false, 200, true },	// D2 f
	{ kTheKeyDownScript,	"keyDownScript",	false, 200, false },	// D2 p
	{ kTheKeyUpScript,		"keyUpScript",		false, 400, false },	//			D4 p
	{ kTheLabelList,		"labelList",		false, 300, true },	//		D3 f
	{ kTheLastClick,		"lastClick",		false, 200, true },	// D2 f
	{ kTheLastEvent,		"lastEvent",		false, 200, true },	// D2 f
	{ kTheLastFrame,		"lastFrame",		false, 400, false },	//			D4 p
	{ kTheLastKey,			"lastKey",			false, 200, true },	// D2 f
	{ kTheLastRoll,			"lastRoll",			false, 200, true },	// D2 f
	{ kTheMachineType,		"machineType",		false, 200, true },	// D2 f
	{ kTheMaxInteger,		"maxInteger",		false, 300, true },	//		D3.1 f
	{ kTheMemorySize,		"memorySize",		false, 200, true },	// D2 f
	{ kTheMenu,				"menu",				true,  300, false },	//		D3 p
	{ kTheMenuItem,			"menuitem",			true,  300, false },	//		D3 p
	{ kTheMenuItems,		"menuitems",		false, 300, true },	//		D3 f
	{ kTheMouseCast,		"mouseCast",		false, 300, true },	//		D3 f
	{ kTheMouseChar,		"mouseChar",		false, 300, true },	//		D3 f
	{ kTheMouseDown,		"mouseDown",		false, 200, true },	// D2 f
	{ kTheMouseDownScript,  "mouseDownScript",	false, 200, false },	// D2 p
	{ kTheMouseH,			"mouseH",			false, 200, true },	// D2 f
	{ kTheMouseItem,		"mouseItem",		false, 300, true },	//		D3 f
	{ kTheMouseLine,		"mouseLine",		false, 300, true },	//		D3 f
	{ kTheMouseUp,			"mouseUp",			false, 200, true },	// D2 f
	{ kTheMouseUpScript,  	"mouseUpScript",	false, 200, false },	// D2 p
	{ kTheMouseV,			"mouseV",			false, 200, true },	// D2 f
	{ kTheMouseWord,		"mouseWord",		false, 300, true },	//		D3 f
	{ kTheMovie,			"movie",			false, 200, true },	// D2 f
	{ kTheMovieFileFreeSize,"movieFileFreeSize",false, 400, true },	//			D4 f
	{ kTheMovieFileSize,	"movieFileSize",	false, 400, true },	//			D4 f
	{ kTheMovieName,		"movieName",		false, 400, true },	//			D4 f
	{ kTheMoviePath,		"moviePath",		false, 400, true },	//			D4 f
	{ kTheMultiSound,		"multiSound",		false, 300, true },	//		D3.1 f
	{ kTheOptionDown,		"optionDown",		false, 200, true },	// D2 f
	{ kTheParamCount,		"paramCount",		false, 400, true },	//			D4 f
	{ kThePathName,			"pathName",			false, 200, true },	// D2 f
	{ kThePauseState,		"pauseState",		false, 200, true },	// D2 f
	{ kThePerFrameHook,		"perFrameHook",		false, 200, false },	// D2 p
	{ kThePreloadEventAbort,"preloadEventAbort",false, 400, false },	//			D4 p
	{ kThePreLoadRAM,		"preLoadRAM",		false, 400, false },	//			D4 p
	{ kThePi,				"pi",				false, 400, true },	//			D4 f
	{ kTheQuickTimePresent,	"quickTimePresent",	false, 300, true },	//		D3.1 f
	{ kTheRandomSeed,		"randomSeed",		false, 400, false },	//			D4 p
	{ kTheResult,			"result",			false, 200, true },	// D2 f
	{ kTheRightMouseDown,	"rightMouseDown",	false, 500, true },	//					D5 f
	{ kTheRightMouseUp,		"rightMouseUp",		false, 500, true },	//					D5 f
	{ kTheRollOver,			"rollOver",			false, 500, true },	//					D5 f, undocumented
	{ kTheRomanLingo,		"romanLingo",		false, 300, false },	//		D3.1 p
	{ kTheRunMode, 			"runMode",			false, 500, false },//					D5 f, documented in D6
	{ kTheScummvmVersion,	"scummvmVersion",	false, 200, true }, // 					ScummVM only
	{ kTheSearchCurrentFolder,"searchCurrentFolder",false,400, true },//			D4 f
	{ kTheSearchPath,		"searchPath",		false, 400, true },	//			D4 f
	{ kTheSelection,		"selection",		false, 200, true },	// D2 f
	{ kTheSelEnd,			"selEnd",			false, 200, false },	// D2 p
	{ kTheSelStart,			"selStart",			false, 200, false },	// D2 p
	{ kTheShiftDown,		"shiftDown",		false, 200, true },	// D2 f
	{ kTheSoundEnabled,		"soundEnabled",		false, 200, false },	// D2 p
	{ kTheSoundEntity,		"sound",			true,  300, false },	// 		D3 p
	{ kTheSoundLevel,		"soundLevel",		false, 200, false },	// D2 p
	{ kTheSprite,			"sprite",			true,  200, false },	// 			D4 p
	{ kTheStage,			"stage",			false, 400, false },	//			D4 p
	{ kTheStageBottom,		"stageBottom",		false, 200, true },	// D2 f
	{ kTheStageColor,		"stageColor",		false, 300, false },	//		D3 p
	{ kTheStageLeft,		"stageLeft",		false, 200, true },	// D2 f
	{ kTheStageRight,		"stageRight",		false, 200, true },	// D2 f
	{ kTheStageTop,			"stageTop",			false, 200, true },	// D2 f
	{ kTheStillDown,		"stillDown",		false, 200, true },	// D2 f
	{ kTheSwitchColorDepth,	"switchColorDepth",	false, 200, false },	// D2 p
	{ kTheTicks,			"ticks",			false, 200, true },	// D2 f
	{ kTheTime,				"time",				false, 300, true },	// 		D3 f
	{ kTheTimeoutKeyDown,	"timeoutKeyDown",	false, 200, false },	// D2 p
	{ kTheTimeoutLapsed,	"timeoutLapsed",	false, 200, false },	// D2 p
	{ kTheTimeoutLength,	"timeoutLength",	false, 200, false },	// D2 p
	{ kTheTimeoutMouse,		"timeoutMouse",		false, 200, false },	// D2 p
	{ kTheTimeoutPlay,		"timeoutPlay",		false, 200, false },	// D2 p
	{ kTheTimeoutScript,	"timeoutScript",	false, 200, false },	// D2 p
	{ kTheTimer,			"timer",			false, 200, false },	// D2 p
	{ kTheTrace,			"trace",			false, 400, false },	//			D4 p
	{ kTheTraceLoad,		"traceLoad",		false, 400, false },	//			D4 p
	{ kTheTraceLogFile,		"traceLogFile",		false, 400, false },	//			D4 p
	{ kTheUpdateMovieEnabled,"updateMovieEnabled",false,400, false },//			D4 p
	{ kTheVideoForWindowsPresent,	"videoForWindowsPresent",	false, 400, true },	//		D4 f
	{ kTheWindow,			"window",			true,  400, false },	//			D4
	{ kTheWindowList,		"windowList",		false, 400, false },	//			D4 p
	{ kTheXtras,			"xtras",			false, 500, false },	//			D4 p
	{ kTheNOEntity, nullptr, false, 0, false }
};

const TheEntityField fields[] = {
	{ kTheSprite,	"backColor",	kTheBackColor,	200 },// D2 p
	{ kTheSprite,	"blend",		kTheBlend,		400 },//				D4 p
	{ kTheSprite,	"bottom",		kTheBottom,		200 },// D2 p
	{ kTheSprite,	"castNum",		kTheCastNum,	200 },// D2 p
	{ kTheSprite,	"constraint",	kTheConstraint, 200 },// D2 p
	{ kTheSprite,	"cursor",		kTheCursor,		200 },// D2 p
	{ kTheSprite,	"editableText", kTheEditableText,400 },//				D4 p
	{ kTheSprite,	"foreColor",	kTheForeColor,	200 },// D2 p
	{ kTheSprite,	"height",		kTheHeight,		200 },// D2 p
	{ kTheSprite,	"immediate",	kTheImmediate,	200 },// D2 p
	{ kTheSprite,	"ink",			kTheInk,		200 },// D2 p
	{ kTheSprite,	"left",			kTheLeft,		200 },// D2 p
	{ kTheSprite,	"lineSize",		kTheLineSize,	200 },// D2 p
	{ kTheSprite,	"loc",			kTheLoc,		400 },//				D4 p ???
	{ kTheSprite,	"locH",			kTheLocH,		200 },// D2 p
	{ kTheSprite,	"locV",			kTheLocV,		200 },// D2 p
	{ kTheSprite,	"memberNum",	kTheMemberNum,	500 },//					D5 p
	{ kTheSprite,	"moveableSprite",kTheMoveableSprite,400 },//			D4 p
	{ kTheSprite,	"pattern",		kThePattern,	200 },// D2 p
	{ kTheSprite,	"puppet",		kThePuppet,		200 },// D2 p
	{ kTheSprite,	"rect",			kTheRect,		400 },//				D4 p ???
	{ kTheSprite,	"right",		kTheRight,		200 },// D2 p
	{ kTheSprite,	"scoreColor",	kTheScoreColor,	400 },//				D4 p
	{ kTheSprite,	"scriptNum",	kTheScriptNum,	400 },//				D4 p
	{ kTheSprite,	"stretch",		kTheStretch,		200 },// D2 p
	{ kTheSprite,	"top",			kTheTop,		200 },// D2 p
	{ kTheSprite,	"trails",		kTheTrails,		300 },//		D3.1 p
	{ kTheSprite,	"type",			kTheType,		200 },// D2 p
	{ kTheSprite,	"visibility",	kTheVisibility,	300 },//		D3.1 p
	{ kTheSprite,	"visible",		kTheVisible,	400 },//				D4 p
	{ kTheSprite,	"width",		kTheWidth,		200 },// D2 p

	// Common cast fields
	{ kTheCast,		"backColor",	kTheBackColor,	400 },//				D4 p
	{ kTheCast,		"castLibNum",	kTheCastLibNum,	500 },// 					D5 p
	{ kTheCast,		"castType",		kTheCastType,	400 },//				D4 p
	{ kTheCast,		"filename",		kTheFileName,	400 },//				D4 p
	{ kTheCast,		"foreColor",	kTheForeColor,	400 },//				D4 p
	{ kTheCast,		"height",		kTheHeight,		400 },//				D4 p
	{ kTheCast,		"loaded",		kTheLoaded,		400 },//				D4 p
	{ kTheCast,		"modified",		kTheModified,	400 },//				D4 p
	{ kTheCast,		"memberNum",	kTheMemberNum,	500 },//					D5 p
	{ kTheCast,		"name",			kTheName,		300 },//		D3 p
	{ kTheCast,		"number",		kTheNumber,		300 },//		D3 p
	{ kTheCast,		"rect",			kTheRect,		400 },//				D4 p
	{ kTheCast,		"purgePriority",kThePurgePriority,400 },//				D4 p // 0 Never purge, 1 Purge Last, 2 Purge next, 2 Purge normal
	{ kTheCast,		"scriptText",	kTheScriptText,	400 },//				D4 p
	{ kTheCast,		"size",			kTheSize,		300 },//		D3.1 p
	{ kTheCast,		"type",			kTheType,		500 },//					D5 p
	{ kTheCast,		"width",		kTheWidth,		400 },//				D4 p

	// Digital video fields
	{ kTheCast,		"center",		kTheCenter,		400 },//				D4 p
	{ kTheCast,		"controller",	kTheController,	300 },//		D3.1 p
	{ kTheCast,		"crop",			kTheCrop,		400 },//				D4 p
	{ kTheCast,		"directToStage",kTheDirectToStage,300 },//		D3.1 p
	{ kTheCast,		"duration",		kTheDuration,	300 },//		D3.1 p
	{ kTheCast,		"frameRate",	kTheFrameRate,	400 },//				D4 p
	{ kTheCast,		"loop",			kTheLoop,		300 },//		D3.1 p
	{ kTheSprite,	"movieRate",	kTheMovieRate,	300 },//		D3.1 P
	{ kTheSprite,	"movieTime",	kTheMovieTime,	300 },//		D3.1 P
	{ kTheCast,		"pausedAtStart",kThePausedAtStart,400 },//				D4 p
	{ kTheCast,		"preLoad",		kThePreLoad,	300 },//		D3.1 p
	{ kTheCast,		"sound",		kTheSound,		300 },//		D3.1 p // 0-1 off-on
	{ kTheSprite,	"startTime",	kTheStartTime,	300 },//		D3.1 p
	{ kTheSprite,	"stopTime",		kTheStopTime,	300 },//		D3.1 p
	{ kTheCast,		"video",		kTheVideo,		400 },//				D4 p
	{ kTheSprite,	"volume",		kTheVolume,		300 },//		D3.1 p

	// Bitmap fields
	{ kTheCast,		"depth",		kTheDepth,		400 },//				D4 p
	{ kTheCast,		"regPoint",		kTheRegPoint,	400 },//				D4 p
	{ kTheCast,		"palette",		kThePalette,	400 },//				D4 p
	{ kTheCast,		"picture",		kThePicture,	300 },//		D3 p

	// TextCastMember fields
	{ kTheCast,		"hilite",		kTheHilite,		200 },// D2 p
	{ kTheCast,		"text",			kTheText,		200 },// D2 p
	{ kTheCast,		"textAlign",	kTheTextAlign,	300 },//		D3 p
	{ kTheCast,		"textFont",		kTheTextFont,	300 },//		D3 p
	{ kTheCast,		"textHeight",	kTheTextHeight,	300 },//		D3 p
	{ kTheCast,		"textSize",		kTheTextSize,	300 },//		D3 p
	{ kTheCast,		"textStyle",	kTheTextStyle,	300 },//		D3 p
	{ kTheCast,		"scrollTop",	kTheScrollTop,  500 },//						D5 p


	// Field fields
	{ kTheField,	"foreColor",	kTheForeColor,	400 },//				D4 p
	{ kTheField,	"hilite",		kTheHilite,		200 },// D2 p
	{ kTheField,	"name",			kTheName,		300 },//		D3 p
	{ kTheField,	"text",			kTheText,		200 },// D2 p
	{ kTheField,	"textAlign",	kTheTextAlign,	300 },//		D3 p
	{ kTheField,	"textFont",		kTheTextFont,	300 },//		D3 p
	{ kTheField,	"textHeight",	kTheTextHeight,	300 },//		D3 p
	{ kTheField,	"textSize",		kTheTextSize,	300 },//		D3 p
	{ kTheField,	"textStyle",	kTheTextStyle,	300 },//		D3 p
	{ kTheField,	"scrollTop",	kTheScrollTop,  500 },//						D5 p

	// Chunk fields
	{ kTheChunk,	"foreColor",	kTheForeColor,	400 },//				D4 p
	{ kTheChunk,	"textFont",		kTheTextFont,	300 },//		D3 p
	{ kTheChunk,	"textHeight",	kTheTextHeight,	300 },//		D3 p
	{ kTheChunk,	"textSize",		kTheTextSize,	300 },//		D3 p
	{ kTheChunk,	"textStyle",	kTheTextStyle,	300 },//		D3 p

	{ kTheWindow,	"drawRect",		kTheDrawRect,	400 },//				D4 p
	{ kTheWindow,	"fileName",		kTheFileName,	400 },//				D4 p
	{ kTheWindow,	"modal",		kTheModal,		400 },//				D4 p
	{ kTheWindow,	"rect",			kTheRect,		400 },//				D4 p
	{ kTheWindow,	"title",		kTheTitle,		400 },//				D4 p
	{ kTheWindow,	"titleVisible",	kTheTitleVisible,400 },//				D4 p
	{ kTheWindow,	"sourceRect",	kTheSourceRect,	400 },//				D4 p
	{ kTheWindow,	"visible",		kTheVisible,	400 },//				D4 p
	{ kTheWindow,	"windowType",	kTheWindowType,	400 },//				D4 p

	{ kTheMenuItem,	"checkmark",	kTheCheckMark,	300 },//		D3 p
	{ kTheMenuItem, "enabled",		kTheEnabled,	300 },//		D3 p
	{ kTheMenuItem, "name",			kTheName,		300 },//		D3 p
	{ kTheMenuItem, "script",		kTheScript,		300 },//		D3 p
	{ kTheMenuItems,"number",		kTheNumber,		300 },//		D3 p 			// number of menuitems of menu <xx>

	{ kTheMenu,		"name",			kTheName,		300 },//		D3 p

	{ kTheCastMembers,	"number",	kTheNumber,		300 },// 		D3 p

	{ kTheDate,		"short",		kTheShort,		300 },//		D3 f
	{ kTheDate,		"long",			kTheLong,		300 },//		D3 f
	{ kTheDate,		"abbreviated",	kTheAbbr,		300 },//		D3 f
	{ kTheDate,		"abbrev",		kTheAbbr,		300 },//		D3 f
	{ kTheDate,		"abbr",			kTheAbbr,		300 },//		D3 f
	{ kTheTime,		"short",		kTheShort,		300 },//		D3 f
	{ kTheTime,		"long",			kTheLong,		300 },//		D3 f
	{ kTheTime,		"abbreviated",	kTheAbbr,		300 },//		D3 f
	{ kTheTime,		"abbrev",		kTheAbbr,		300 },//		D3 f
	{ kTheTime,		"abbr",			kTheAbbr,		300 },//		D3 f

	{ kTheSoundEntity,"volume",		kTheVolume,		300 },//		D3 p

	{ kTheNOEntity, nullptr, kTheNOField, 0 }
};

void Lingo::initTheEntities() {
	_objectEntityId = kTheObject;

	const TheEntity *e = entities;
	_entityNames.resize(kTheMaxTheEntityType);

	while (e->entity != kTheNOEntity) {
		if (e->version <= _vm->getVersion()) {
			_theEntities[e->name] = e;

			_entityNames[e->entity] = e->name;
		}

		e++;
	}

	const TheEntityField *f = fields;
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

void Lingo::cleanUpTheEntities() {
	_entityNames.clear();
	_fieldNames.clear();
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

#define getTheEntitySTUB(entity) \
	warning("Lingo::getTheEntity(): Unprocessed getting entity %s", entity2str(entity));

Datum Lingo::getTheEntity(int entity, Datum &id, int field) {
	debugC(3, kDebugLingoThe, "Lingo::getTheEntity(%s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field));
	debugC(3, kDebugLingoExec, "Lingo::getTheEntity(%s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field));

	Datum d;
	Movie *movie = _vm->getCurrentMovie();

	if (!movie) {
		warning("Lingo::getTheEntity(): Movie is missing");
		d.type = VOID;

		return d;
	}

	g_debugger->entityReadHook(entity, field);

	LingoArchive *mainArchive = movie->getMainLingoArch();
	Score *score = movie->getScore();

	switch (entity) {
	case kTheActorList:
		d = g_lingo->_actorList;
		break;
	case kTheBeepOn:
		d = (int)movie->_isBeepOn;
		break;
	case kTheButtonStyle:
		d = (int)(g_director->_wm->_mode & Graphics::kWMModeButtonDialogStyle);
		break;
	case kTheCast:
		d = getTheCast(id, field);
		break;
	case kTheCastLibs: // D5
		d = getCastLibsNum();
		break;
	case kTheCastMembers:
		d = getMembersNum();
		break;
	case kTheCenterStage:
		d = g_director->_centerStage;
		break;
	case kTheCheckBoxAccess:
		d = movie->_checkBoxAccess;
		break;
	case kTheCheckBoxType:
		d = movie->_checkBoxType;
		break;
	case kTheChunk:
		d = getTheChunk(id, field);
		break;
	case kTheClickLoc:
		d.u.farr = new FArray;

		d.u.farr->arr.push_back(movie->_lastClickPos.x);
		d.u.farr->arr.push_back(movie->_lastClickPos.y);
		d.type = POINT;
		break;
	case kTheClickOn:
		// Even in D4, `the clickOn` uses the old "active" sprite instead of mouse sprite.
		d = (int)movie->_currentActiveSpriteId;
		break;
	case kTheColorDepth:
		// bpp. 1, 2, 4, 8, 32
		d = _vm->_colorDepth;
		break;
	case kTheColorQD:
		d = 1;
		break;
	case kTheCommandDown:
		if (g_director->getPlatform() == Common::kPlatformWindows)
			d = (movie->_keyFlags & Common::KBD_CTRL) ? 1 : 0;
		else
			d = (movie->_keyFlags & Common::KBD_META) ? 1 : 0;
		break;
	case kTheControlDown:
		d = (movie->_keyFlags & Common::KBD_CTRL) ? 1 : 0;
		break;
	case kTheDate:
		d = getTheDate(field);
		break;
	case kTheDeskTopRectList:
		d = getTheDeskTopRectList();
		break;
	case kTheDoubleClick:
		// Always measured against the last two clicks.
		// 25 ticks seems to be the threshold for a double click.
		d = (movie->_lastClickTime - movie->_lastClickTime2) <= 25 ? 1 : 0;
		break;
	case kTheExitLock:
		d = g_lingo->_exitLock;
		break;
	case kTheField:
		d = getTheField(id, field);
		break;
	case kTheFixStageSize:
		d = (int)g_director->_fixStageSize;
		break;
	case kTheFloatPrecision:
		d = _floatPrecision;
		break;
	case kTheFrame:
		d = (int)score->getCurrentFrameNum();
		break;
	case kTheFrameLabel:
		d.type = STRING;
		d.u.s = score->getFrameLabel(score->getCurrentFrameNum());
		break;
	case kTheFrameScript:
		d = score->_currentFrame->_mainChannels.actionId.member;
		break;
	case kTheFramePalette:
		d = score->getCurrentPalette();
		break;
	case kTheFrameTempo:
		d = score->_currentFrameRate;
		break;
	case kTheFreeBlock:
	case kTheFreeBytes:
		d = 32 * 1024 * 1024;	// Let's have 32 Mbytes
		break;
	case kTheFullColorPermit:
		d = 1;					// We always allow it in ScummVM
		break;
	case kTheImageDirect:
		d = 1;					// We always allow it in ScummVM
		break;
	case kTheItemDelimiter:
		{
			Common::U32String ch(g_lingo->_itemDelimiter);
			d.type = STRING;
			d.u.s = new Common::String(ch, Common::kUtf8);
		}
		break;
	case kTheKey:
		d.type = STRING;
		d.u.s = new Common::String(movie->_key);
		break;
	case kTheKeyCode:
		d = movie->_keyCode;
		break;
	case kTheKeyDownScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventKeyDown))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventKeyDown]);
		else
			d.u.s = new Common::String();
		break;
	case kTheKeyUpScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventKeyUp))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventKeyUp]);
		else
			d.u.s = new Common::String();
		break;
	case kTheLabelList:
		d.type = STRING;
		d.u.s = score->getLabelList();
		break;
	case kTheLastClick:
		d = (int)(_vm->getMacTicks() - movie->_lastClickTime);
		break;
	case kTheLastEvent:
		d = (int)(_vm->getMacTicks() - movie->_lastEventTime);
		break;
	case kTheLastFrame:
		d = (int)score->getFramesNum();
		break;
	case kTheLastKey:
		d = (int)(_vm->getMacTicks() - movie->_lastKeyTime);
		break;
	case kTheLastRoll:
		d = (int)(_vm->getMacTicks() - movie->_lastRollTime);
		break;
	case kTheMachineType:
		// These are actually coming from Gestalt machineType
		//
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
		// 25 - PowerBook Duo 210		D4
		// 27 - Macintosh LCIII			D4
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
		// 70 - PowerBook 540C			D6 // "Director 6 Demystified" p.818
		// 73 - Power Macintosh 6100/60	D5
		// 76 - Macintosh Quadra 840av	D4
		// 256 - IBM PC-type machine	D3
		d = _vm->_machineType;
		break;
	case kTheMaxInteger:
		d = 2147483647; // (2^31)-1 [max 32bit signed integer]
		break;
	case kTheMemorySize:
		d = 32 * 1024 * 1024;	// Let's have 32 Mbytes
		break;
	case kTheMenu:
		if (!g_director->_wm->getMenu()) {
			warning("Lingo::getTheEntity(): Menu does not exist!");
			break;
		}

		d.type = STRING;
		Graphics::MacMenuItem *menuRef;
		menuRef = nullptr;

		if (id.u.menu->menuIdNum == -1) {
			menuRef = g_director->_wm->getMenu()->getMenuItem(*id.u.menu->menuIdStr);
		} else {
			menuRef = g_director->_wm->getMenu()->getMenuItem(id.u.menu->menuIdNum - 1);
		}
		d.u.s = new Common::String();
		*d.u.s = g_director->_wm->getMenu()->getName(menuRef);
		break;
	case kTheMenuItem:
		if (!g_director->_wm->getMenu()) {
			warning("Lingo::getTheEntity(): Menu does not exist!");
			break;
		}
		Graphics::MacMenuItem *menu, *menuItem;
		menu = nullptr, menuItem = nullptr;

		if (id.u.menu->menuIdNum == -1) {
			menu = g_director->_wm->getMenu()->getMenuItem(*id.u.menu->menuIdStr);
		} else {
			menu = g_director->_wm->getMenu()->getMenuItem(id.u.menu->menuIdNum - 1);
		}
		if (id.u.menu->menuItemIdNum == -1) {
			menuItem = g_director->_wm->getMenu()->getSubMenuItem(menu, *id.u.menu->menuItemIdStr);
		} else {
			menuItem = g_director->_wm->getMenu()->getSubMenuItem(menu, id.u.menu->menuItemIdNum - 1);
		}

		switch (field) {
		case kTheCheckMark:
			d = g_director->_wm->getMenuItemCheckMark(menuItem);
			break;
		case kTheEnabled:
			d = g_director->_wm->getMenuItemEnabled(menuItem);
			break;
		case kTheName:
			d.type = STRING;
			d.u.s = new Common::String;
			*(d.u.s) = g_director->_wm->getMenuItemName(menuItem);
			break;
		case kTheScript:
			d = g_director->_wm->getMenuItemAction(menuItem);
			break;
		default:
			warning("Lingo::getTheEntity(): Unprocessed setting field \"%s\" of entity %s", field2str(field), entity2str(entity));
			break;
		}
		break;
	case kTheMenuItems:
		d = getMenuItemsNum(id);
		break;
	case kTheMenus:
		d = getMenuNum();
		break;
	case kTheMouseCast:
		{
			// TODO: How is this handled with multiple casts in D5?
			Common::Point pos = g_director->getCurrentWindow()->getMousePos();
			uint16 spriteId = score->getSpriteIDFromPos(pos);
			d = score->getSpriteById(spriteId)->_castId.member;
			if (d.u.i == 0)
				d = -1;
		}
		break;
	case kTheMouseChar:
		{
			// maybe a better handling is iterate channels and check the text sprite that enclose the cursor
			Common::Point pos = g_director->getCurrentWindow()->getMousePos();
			uint16 spriteId = score->getSpriteIDFromPos(pos);
			Channel *ch = score->getChannelById(spriteId);
			d = ch->getMouseChar(pos.x, pos.y);
		}
		break;
	case kTheMouseDown:
		d = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT | 1 << Common::MOUSE_BUTTON_RIGHT) ? 1 : 0;
		break;
	case kTheMouseDownScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventMouseDown))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventMouseDown]);
		else
			d.u.s = new Common::String();
		break;
	case kTheMouseH:
		d = g_director->getCurrentWindow()->getMousePos().x;
		break;
	case kTheMouseItem:
		{
			Common::Point pos = g_director->getCurrentWindow()->getMousePos();
			uint16 spriteId = score->getSpriteIDFromPos(pos);
			Channel *ch = score->getChannelById(spriteId);
			d = ch->getMouseItem(pos.x, pos.y);
		}
		break;
	case kTheMouseLine:
		{
			Common::Point pos = g_director->getCurrentWindow()->getMousePos();
			uint16 spriteId = score->getSpriteIDFromPos(pos);
			Channel *ch = score->getChannelById(spriteId);
			d = ch->getMouseLine(pos.x, pos.y);
		}
		break;
	case kTheMouseUp:
		d = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT | 1 << Common::MOUSE_BUTTON_RIGHT) ? 0 : 1;
		break;
	case kTheMouseUpScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventMouseUp))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventMouseUp]);
		else
			d.u.s = new Common::String();
		break;
	case kTheMouseV:
		d = g_director->getCurrentWindow()->getMousePos().y;
		break;
	case kTheMouseWord:
		{
			// same issue as MouseChar, check MouseChar above
			Common::Point pos = g_director->getCurrentWindow()->getMousePos();
			uint16 spriteId = score->getSpriteIDFromPos(pos);
			Channel *ch = score->getChannelById(spriteId);
			d = ch->getMouseWord(pos.x, pos.y);
		}
		break;
	case kTheMovie:
	case kTheMovieName:
		d.type = STRING;
		d.u.s = new Common::String(movie->getMacName());
		break;
	case kTheMovieFileFreeSize:
		d = 0;	// Let's pretend the movie is compactified
		break;
	case kTheMovieFileSize:
		d = (int)movie->getArchive()->getFileSize();
		break;
	case kTheMoviePath:
	case kThePathName:
		d.type = STRING;
		d.u.s = new Common::String(_vm->getCurrentAbsolutePath());
		break;
	case kTheMultiSound:
		// We always support multiple sound channels!
		d = 1;
		break;
	case kTheOptionDown:
		d = (movie->_keyFlags & Common::KBD_ALT) ? 1 : 0;
		break;
	case kTheParamCount:
		d = g_lingo->_state->callstack[g_lingo->_state->callstack.size() - 1]->paramCount;
		break;
	case kThePauseState:
		d = (int) g_director->_playbackPaused;
		break;
	case kThePerFrameHook:
		d = _perFrameHook;
		break;
	case kThePreloadEventAbort:
		d = g_lingo->_preLoadEventAbort;
		break;
	case kThePreLoadRAM:
		d = 0;		// We always have unlimited RAM
		break;
	case kThePi:
		d = M_PI;
		break;
	case kTheQuickTimePresent:
		// QuickTime is always present for ScummVM
		d = 1;
		break;
	case kTheRandomSeed:
		d = (int)g_director->_rnd.getSeed();
		break;
	case kTheResult:
		d = g_lingo->_theResult;
		break;
	case kTheRightMouseDown:
		d = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_RIGHT) ? 1 : 0;
		break;
	case kTheRightMouseUp:
		d = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_RIGHT) ? 0 : 1;
		break;
	case kTheRollOver:
		d = score->getSpriteIDFromPos(g_director->getCurrentWindow()->getMousePos());
		break;
	case kTheRomanLingo:
		d = g_lingo->_romanLingo;
		warning("BUILDBOT: the romanLingo is get, value is %d", g_lingo->_romanLingo);
		break;
	case kTheRunMode:
		if (ConfMan.hasKey("director_runMode"))
			d = Datum(ConfMan.get("director_runMode"));
		else {
			d = Datum("Projector");
		}
		break;
	case kTheScummvmVersion:
		d = _vm->getVersion();
		break;
	case kTheSearchCurrentFolder:
		//We always allow searching in current folder
		warning("BUILDBOT: SearchCurrentFolder is queried");
		d = 1;
		break;
	case kTheSearchPath:
		d = g_lingo->_searchPath;
		break;
	case kTheSelection:
		if (movie->_currentEditableTextChannel) {
			Channel *channel = score->_channels[movie->_currentEditableTextChannel];

			if (channel->_widget) {
				d.type = STRING;
				d.u.s = new Common::String(Common::convertFromU32String(((Graphics::MacText *)channel->_widget)->getSelection(false, false)));
			}
		}
		break;
	case kTheSelEnd:
	case kTheSelStart:
		if (movie->_currentEditableTextChannel) {
			Channel *channel = score->_channels[movie->_currentEditableTextChannel];

			if (channel->_widget) {
				d = (int)((Graphics::MacText *)channel->_widget)->getSelectionIndex(entity == kTheSelStart);
			}
		}
		break;
	case kTheShiftDown:
		d = (movie->_keyFlags & Common::KBD_SHIFT) ? 1 : 0;
		break;
	case kTheSoundEnabled:
		d = _vm->getCurrentWindow()->getSoundManager()->getSoundEnabled();
		break;
	case kTheSoundEntity:
		{
			switch (field) {
			case kTheVolume:
				d = _vm->getCurrentWindow()->getSoundManager()->getChannelVolume(id.asInt());
				break;
			default:
				warning("Lingo::getTheEntity(): Unprocessed getting field \"%s\" of entity %s", field2str(field), entity2str(entity));
				break;
			}
		}
		break;
	case kTheSoundLevel:
		// getting sound level of channel 1, maybe need to be amended in higher version
		d = _vm->getCurrentWindow()->getSoundManager()->getChannelVolume(1) / 32;
		break;
	case kTheSprite:
		d = getTheSprite(id, field);
		break;
	case kTheStage:
		d = _vm->getStage();
		break;
	case kTheStageBottom:
		d = _vm->getCurrentWindow()->getSurface()->h;
		break;
	case kTheStageColor:
		// TODO: Provide proper reverse transform for non-indexed color
		d = (int)g_director->transformColor(g_director->getCurrentWindow()->getStageColor());
		break;
	case kTheStageLeft:
		d = 0;
		break;
	case kTheStageRight:
		d = _vm->getCurrentWindow()->getSurface()->w;
		break;
	case kTheStageTop:
		d = 0;
		break;
	case kTheStillDown:
		d = _vm->_wm->_mouseDown;
		break;
	case kTheSwitchColorDepth:
		getTheEntitySTUB(kTheSwitchColorDepth);
		break;
	case kTheTicks:
		d = (int)_vm->getMacTicks();
		break;
	case kTheTime:
		d = getTheTime(field);
		break;
	case kTheTimeoutKeyDown:
		d= movie->_timeOutKeyDown;
		break;
	case kTheTimeoutLapsed:
		d = (int)(_vm->getMacTicks() - movie->_lastTimeOut);
		break;
	case kTheTimeoutLength:
		d = (int)movie->_timeOutLength;
		break;
	case kTheTimeoutMouse:
		d = movie->_timeOutMouse;
		break;
	case kTheTimeoutPlay:
		d = movie->_timeOutPlay;
		break;
	case kTheTimeoutScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventTimeout))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventTimeout]);
		else
			d.u.s = new Common::String();
		break;
	case kTheTimer:
		d = (int)(_vm->getMacTicks() - movie->_lastTimerReset);
		break;
	case kTheTrace:
		d = (int) g_lingo->_trace;
		break;
	case kTheTraceLoad:
		d = g_lingo->_traceLoad;
		break;
	case kTheTraceLogFile:
		d.type = STRING;
		d.u.s = new Common::String(g_director->_traceLogFile.toString(Common::Path::kNativeSeparator));
		break;
	case kTheUpdateMovieEnabled:
		d = g_lingo->_updateMovieEnabled;
		break;
	case kTheVideoForWindowsPresent:
		// Video For Windows is always present for ScummVM
		d = 1;
		break;
	case kTheWindow:
		g_lingo->push(id);
		LB::b_window(1);
		d = g_lingo->pop().u.obj->getField(field);
		break;
	case kTheWindowList:
		d = g_lingo->_windowList;
		break;
	case kTheXtras: // D5
		d = getXtrasNum();
		break;
	default:
		warning("Lingo::getTheEntity(): Unprocessed getting field \"%s\" of entity %s", field2str(field), entity2str(entity));
		break;
	}

	return d;
}

#define setTheEntitySTUB(entity) \
	warning("Lingo::setTheEntity(): Unprocessed setting entity %s", entity2str(entity));

#define setTheEntityReadOnly(entity) \
	warning("Lingo::setTheEntity: Attempt to set read-only entity %s", entity2str(entity));

void Lingo::setTheEntity(int entity, Datum &id, int field, Datum &d) {
	debugC(3, kDebugLingoThe, "Lingo::setTheEntity(%s, %s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field), d.asString(true).c_str());
	debugC(3, kDebugLingoExec, "Lingo::setTheEntity(%s, %s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field), d.asString(true).c_str());

	Movie *movie = _vm->getCurrentMovie();
	Score *score = movie->getScore();

	switch (entity) {
	case kTheActorList:
		g_lingo->_actorList = d;
		break;
	case kTheBeepOn:
		movie->_isBeepOn = (bool)d.u.i;
		break;
	case kTheButtonStyle:
		if (d.asInt())
			g_director->_wm->_mode = g_director->_wmMode | Graphics::kWMModeButtonDialogStyle;
		else
			g_director->_wm->_mode = g_director->_wmMode;
		break;
	case kTheCast:
		setTheCast(id, field, d);
		break;
	case kTheCenterStage:
		g_director->_centerStage = d.asInt();
		break;
	case kTheCheckBoxAccess:
		movie->_checkBoxAccess = d.asInt();
		break;
	case kTheCheckBoxType:
		movie->_checkBoxType = d.asInt();
		break;
	case kTheChunk:
		setTheChunk(id, field, d);
		break;
	case kTheColorDepth:
		_vm->_colorDepth = d.asInt();

		// bpp. 1, 2, 4, 8, 32
		warning("STUB: Lingo::setTheEntity(): Set color depth to %d", _vm->_colorDepth);
		break;
	case kTheExitLock:
		g_lingo->_exitLock = bool(d.asInt());
		break;
	case kTheFixStageSize:
		g_director->_fixStageSize = (bool)d.u.i;
		if (d.u.i) {
			g_director->_fixStageRect = movie->_movieRect;
		}
		break;
	case kTheField:
		setTheField(id, field, d);
		break;
	case kTheFloatPrecision:
		_floatPrecision = d.asInt();
		_floatPrecision = MAX(0, MIN(_floatPrecision, 19)); // 0 to 19
		_floatPrecisionFormat = Common::String::format("%%.%df", _floatPrecision);
		break;
	case kTheFrameLabel:
		setTheEntityReadOnly(kTheFrameLabel);
		break;
	case kTheFrameScript:
		setTheEntityReadOnly(kTheFrameScript);
		break;
	case kTheFramePalette:
		setTheEntityReadOnly(kTheFramePalette);
		break;
	case kTheFrameTempo:
		setTheEntityReadOnly(kTheFramePalette);
		break;
	case kTheFullColorPermit:
		// No op in ScummVM. We always allow it
		break;
	case kTheImageDirect:
		// No op in ScummVM. We always allow it
		break;
	case kTheItemDelimiter:
		if (d.asString().size() == 0)
			g_lingo->_itemDelimiter = 0;
		else
			g_lingo->_itemDelimiter = d.asString().decode(Common::kUtf8)[0];
		break;
	case kTheKeyDownScript:
		movie->setPrimaryEventHandler(kEventKeyDown, d.asString());
		break;
	case kTheKeyUpScript:
		movie->setPrimaryEventHandler(kEventKeyUp, d.asString());
		break;
	case kTheMenu:
		setTheEntitySTUB(kTheMenu);
		break;
	case kTheMenuItem:
		Graphics::MacMenuItem *menu, *menuItem;
		menu = nullptr, menuItem = nullptr;

		if (!g_director->_wm->getMenu()) {
			warning("Lingo::setTheEntity(): Menu does not exist!");
			break;
		}

		if (id.u.menu->menuIdNum == -1) {
			menu = g_director->_wm->getMenu()->getMenuItem(*id.u.menu->menuIdStr);
		} else {
			menu = g_director->_wm->getMenu()->getMenuItem(id.u.menu->menuIdNum - 1);
		}

		if (id.u.menu->menuItemIdNum == -1) {
			menuItem = g_director->_wm->getMenu()->getSubMenuItem(menu, *id.u.menu->menuItemIdStr);
		} else {
			menuItem = g_director->_wm->getMenu()->getSubMenuItem(menu, id.u.menu->menuItemIdNum - 1);
		}

		if (!menuItem) {
			warning("Wrong menuItem!");
			break;
		}
		switch (field) {
		case kTheCheckMark:
			g_director->_wm->setMenuItemCheckMark(menuItem, d.asInt());
			break;
		case kTheEnabled:
			g_director->_wm->setMenuItemEnabled(menuItem, d.asInt());
			break;
		case kTheName:
			g_director->_wm->setMenuItemName(menuItem, d.asString());
			break;
		case kTheScript:
		{
			LingoArchive *mainArchive = movie->getMainLingoArch();
			int commandId = 100;
			while (mainArchive->getScriptContext(kEventScript, commandId))
				commandId++;
			mainArchive->replaceCode(d.asString(), kEventScript, commandId);

			g_director->_wm->setMenuItemAction(menuItem, commandId);
		}
		break;
		default:
			warning("Lingo::setTheEntity(): Unprocessed setting field \"%s\" of entity %s", field2str(field), entity2str(entity));
			break;
		}
		break;
	case kTheMouseDownScript:
		movie->setPrimaryEventHandler(kEventMouseDown, d.asString());
		break;
	case kTheMouseUpScript:
		movie->setPrimaryEventHandler(kEventMouseUp, d.asString());
		break;
	case kThePerFrameHook:
		_perFrameHook = d;
		break;
	case kThePreloadEventAbort:
		g_lingo->_preLoadEventAbort = bool(d.asInt());
		break;
	case kThePreLoadRAM:
		// We always have the unlimited RAM, ignore
		break;
	case kTheRandomSeed:
		g_director->_rnd.setSeed(d.asInt());
		break;
	case kTheRomanLingo:
		g_lingo->_romanLingo = bool(d.asInt());

		// Catching when we set to Japanese
		if (!g_lingo->_romanLingo) {
			warning("BUILDBOT: the romanLingo is set to %d", g_lingo->_romanLingo);
			setTheEntitySTUB(kTheRomanLingo);
		}
		break;
	case kTheScummvmVersion:
		// Allow director version change: used for testing version differences via the lingo tests.
		_vm->setVersion(d.asInt());
		break;
	case kTheSearchCurrentFolder:
		warning("BUILDBOT: Trying to set SearchCurrentFolder lingo property");
		break;
	case kTheSearchPath:
		g_lingo->_searchPath = d;
		break;
	case kTheSelEnd:
		movie->_selEnd = d.asInt();
		if (movie->_currentEditableTextChannel != 0) {
			Channel *channel = score->getChannelById(movie->_currentEditableTextChannel);

			if (channel->_widget)
				(((Graphics::MacText *)channel->_widget)->setSelection(d.asInt(), false));
		}
		break;
	case kTheSelStart:
		movie->_selStart = d.asInt();
		if (movie->_currentEditableTextChannel != 0) {
			Channel *channel = score->getChannelById(movie->_currentEditableTextChannel);

			if (channel->_widget)
				(((Graphics::MacText *)channel->_widget)->setSelection(d.asInt(), true));
		}
		break;
	case kTheSoundEnabled:
		_vm->getCurrentWindow()->getSoundManager()->setSoundEnabled((bool)d.asInt());
		break;
	case kTheSoundEntity:
		{
			switch (field) {
			case kTheVolume:
				{
					_vm->getCurrentWindow()->getSoundManager()->setChannelVolume(id.asInt(), MAX(0, MIN(d.asInt(), 255)));
				}
				break;
			default:
				warning("Lingo::setTheEntity(): Unprocessed getting field \"%s\" of entity %s", field2str(field), entity2str(entity));
				break;
			}
		}
		break;
	case kTheSoundLevel:
		// setting all of the channel for now
		_vm->getCurrentWindow()->getSoundManager()->setChannelVolume(-1, MAX(0, MIN(d.asInt() * 32, 255)));
		break;
	case kTheSprite:
		setTheSprite(id, field, d);
		break;
	case kTheStage:
		setTheEntitySTUB(kTheStage);
		break;
	case kTheStageColor:
		g_director->getCurrentWindow()->setStageColor(g_director->transformColor(d.asInt()));

		// Redraw the stage
		score->updateSprites(kRenderForceUpdate);
		g_director->getCurrentWindow()->render();
		break;
	case kTheSwitchColorDepth:
		setTheEntitySTUB(kTheSwitchColorDepth);
		break;
	case kTheTimeoutKeyDown:
		movie->_timeOutKeyDown = d.asInt();
		break;
	case kTheTimeoutLapsed:
		// timeOutLapsed can be set in D4, but can't in D3. see D3.1 interactivity manual p312 and D4 dictionary p296.
		if (g_director->getVersion() >= 400 && (d.type == INT || d.type == FLOAT)) {
			g_director->_tickBaseline = g_director->getMacTicks() - d.asInt();
		}
		if (d.type != INT) {
			warning("Lingo::setTheEntity() : Wrong DatumType %d for setting of Lingo Property timeOutLapsed", d.type);
		}
		break;
	case kTheTimeoutLength:
		movie->_timeOutLength = d.asInt();
		break;
	case kTheTimeoutMouse:
		movie->_timeOutMouse = d.asInt();
		break;
	case kTheTimeoutPlay:
		movie->_timeOutPlay = d.asInt();
		break;
	case kTheTimeoutScript:
		movie->setPrimaryEventHandler(kEventTimeout, d.asString());
		break;
	case kTheTimer:
		// so value of the timer would be d.asInt()
		movie->_lastTimerReset = _vm->getMacTicks() - d.asInt();
		break;
	case kTheTrace:
		g_lingo->_trace = (bool) d.asInt();
		break;
	case kTheTraceLoad:
		g_lingo->_traceLoad = d.asInt();
		break;
	case kTheTraceLogFile:
	{
		if (d.asString().size()) {
			Common::Path logPath = ConfMan.getPath("path").appendComponent(d.asString());
			Common::FSNode out(logPath);
			if (!out.exists())
				out.createWriteStream(false);
			if (out.isWritable())
				g_director->_traceLogFile = logPath;
			else
				warning("traceLogFile '%s' is not writeable", logPath.toString(Common::Path::kNativeSeparator).c_str());
		} else {
			g_director->_traceLogFile.clear();
		}
	}
		break;
	case kTheUpdateMovieEnabled:
		g_lingo->_updateMovieEnabled = bool(d.asInt());
		break;
	case kTheWindow:
		g_lingo->push(id);
		LB::b_window(1);
		g_lingo->pop().u.obj->setField(field, d);
		break;
	case kTheWindowList:
		if (d.type == ARRAY) {
			g_lingo->_windowList = d;
		} else {
			warning("Lingo::setTheEntity(): kTheWindowList must be a list");
		}
		break;
	default:
		warning("Lingo::setTheEntity(): Unprocessed setting field \"%s\" of entity %s", field2str(field), entity2str(entity));
	}
	g_debugger->entityWriteHook(entity, field);
}

int Lingo::getMenuNum() {
	if (!g_director->_wm->getMenu()) {
		warning("Lingo::getMenuNum(): Menu does not exist!");
		return 0;
	}

	return g_director->_wm->getMenu()->numberOfMenus();
}

int Lingo::getCastLibsNum() {
	return _vm->getCurrentMovie()->getCasts()->size();
}

int Lingo::getMembersNum() {
	// FIXME: deal with D5 castlibs
	Movie *movie = _vm->getCurrentMovie();
	return (MAX(movie->getCast()->getCastMaxID(), (movie->_sharedCast ? movie->_sharedCast->getCastMaxID() : 0)));
}

int Lingo::getXtrasNum() {
	return _openXtras.size();
}

int Lingo::getMenuItemsNum(Datum &d) {
	if (d.type != MENUREF) {
		warning("Datum of wrong type: Expected MENUREF, got '%d'", d.type);
		return 0;
	}

	Graphics::MacMenuItem *menu = nullptr;

	if (!g_director->_wm->getMenu()) {
		warning("Lingo::getMenuItemsNum(): Menu does not exist!");
		return 0;
	}

	if (d.u.menu->menuIdNum == -1) {
		menu = g_director->_wm->getMenu()->getMenuItem(*d.u.menu->menuIdStr);
	} else {
		menu = g_director->_wm->getMenu()->getMenuItem(d.u.menu->menuIdNum);
	}
	return g_director->_wm->getMenu()->numberOfMenuItems(menu);
}

Datum Lingo::getTheSprite(Datum &id1, int field) {
	Datum d;
	int id = 0;
	Score *score = _vm->getCurrentMovie()->getScore();

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


	switch (field) {
	case kTheBackColor:
		// TODO: Provide proper reverse transform for non-indexed color
		d = (int)g_director->transformColor(sprite->_backColor);
		break;
	case kTheBlend:
		d = (255 - sprite->_blendAmount) * 255 / 100;
		break;
	case kTheBottom:
		d = channel->getBbox().bottom;
		break;
	case kTheMember:
		d = sprite->_castId;
		break;
	case kTheCastNum:
	case kTheMemberNum:
		d = sprite->_castId.member;
		break;
	case kTheCastLibNum:
		d = sprite->_castId.castLib;
		break;
	case kTheConstraint:
		d = (int)channel->_constraint;
		break;
	case kTheCursor:
		d = channel->_cursor._cursorResId;
		break;
	case kTheEditableText:
		d = sprite->_editable;
		break;
	case kTheForeColor:
		// TODO: Provide proper reverse transform for non-indexed color
		d = (int)g_director->transformColor(sprite->_foreColor);
		break;
	case kTheHeight:
		d = channel->getHeight();
		break;
	case kTheImmediate:
		d = sprite->_immediate;
		break;
	case kTheInk:
		d = sprite->_ink;
		break;
	case kTheLeft:
		d = channel->getBbox().left;
		break;
	case kTheLineSize:
		d = sprite->_thickness & 0x3;
		break;
	case kTheLoc:
		{
			Common::Point position = channel->getPosition();
			d.type = POINT;
			d.u.farr = new FArray;
			d.u.farr->arr.push_back(position.x);
			d.u.farr->arr.push_back(position.y);
		}
		break;
	case kTheLocH:
		d = channel->getPosition().x;
		break;
	case kTheLocV:
		d = channel->getPosition().y;
		break;
	case kTheMoveableSprite:
		d = sprite->_moveable;
		break;
	case kTheMovieRate:
		d = channel->_movieRate;
		if (debugChannelSet(-1, kDebugEndVideo))
			d.u.f = 0.0;
		break;
	case kTheMovieTime:
		d = channel->_movieTime;
		break;
	case kThePattern:
		d = sprite->getPattern();
		break;
	case kThePuppet:
		d = sprite->_puppet;
		break;
	case kTheRect:
		// let compiler to optimize this
		{
			Common::Rect bbox = channel->getBbox();
			d.type = RECT;
			d.u.farr = new FArray;
			d.u.farr->arr.push_back(bbox.left);
			d.u.farr->arr.push_back(bbox.top);
			d.u.farr->arr.push_back(bbox.right);
			d.u.farr->arr.push_back(bbox.bottom);
		}
		break;
	case kTheRight:
		d = channel->getBbox().right;
		break;
	case kTheScoreColor:
		//Check the last 3 bits of the _colorcode byte as value lies in 0 to 5
		d = (int)(sprite->_colorcode & 0x7);
		break;
	case kTheScriptNum:
		d = sprite->_scriptId.member;
		break;
	case kTheStartTime:
		d = channel->_startTime;
		break;
	case kTheStopTime:
		d = channel->_stopTime;
		break;
	case kTheStretch:
		d = (sprite->_stretch ? 1 : 0);
		break;
	case kTheTop:
		d = channel->getBbox().top;
		break;
	case kTheTrails:
		d = (sprite->_trails ? 1 : 0);
		break;
	case kTheType:
		d = sprite->_spriteType;
		break;
	case kTheVisibility:
	case kTheVisible:
		d = (channel->_visible ? 1 : 0);
		break;
	case kTheVolume:
		d = sprite->_volume;
		break;
	case kTheWidth:
		d = channel->getWidth();
		break;
	default:
		warning("Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
		d.type = VOID;
	}

	return d;
}

void Lingo::setTheSprite(Datum &id1, int field, Datum &d) {
	int id = id1.asInt();
	Movie *movie = _vm->getCurrentMovie();
	Score *score = movie->getScore();

	if (!score) {
		warning("Lingo::setTheSprite(): The sprite %d field \"%s\" setting over non-active score", id, field2str(field));
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
		{
			uint32 newColor = g_director->transformColor(d.asInt());
			if (newColor != sprite->_backColor) {
				sprite->_backColor = newColor;
				channel->_dirty = true;

				// Based on Director in a Nutshell, page 15
				sprite->setAutoPuppet(kAPBackColor, true);
			}
		}
		break;
	case kTheBlend:
		{
			// Convert from (0, 100) range to (0xff, 0x00)
			int blend = (100 - CLIP(d.asInt(), 0, 100)) * 255 / 100;
			if (blend != sprite->_blendAmount) {
				sprite->_blendAmount = blend;
				channel->_dirty = true;
			}

			// Based on Director in a Nutshell, page 15
			sprite->setAutoPuppet(kAPBlend, true);
		}
		break;
	case kTheMember:
		{
			CastMemberID targetMember = d.asMemberID();

			if (targetMember != sprite->_castId) {
				movie->getWindow()->addDirtyRect(channel->getBbox());
				movie->duplicateCastMember(targetMember, sprite->_castId);
				channel->_sprite->setCast(sprite->_castId);
				// Ensure the new sprite, whether larger or smaller, appears correctly on the screen
				movie->getWindow()->addDirtyRect(channel->getBbox());
				channel->_dirty = true;
			}
		}
		break;
	case kTheCastNum:
	case kTheMemberNum:
		{
			CastMemberID castId = d.asMemberID();
			// Setting the cast ID as a number will preserve whatever is in castLib
			if (d.isNumeric() && (sprite->_castId.castLib != 0)) {
				castId = CastMemberID(d.asInt(), sprite->_castId.castLib);
			}
			CastMember *castMember = movie->getCastMember(castId);

			if (castMember && castMember->_type == kCastDigitalVideo) {
				if (((DigitalVideoCastMember *)castMember)->loadVideoFromCast()) {
					((DigitalVideoCastMember *)castMember)->setChannel(channel);
					((DigitalVideoCastMember *)castMember)->startVideo();
					// b_updateStage needs to have _videoPlayback set to render video
					// in the regular case Score::updateSprites sets it.
					// However Score::updateSprites is not in the current code path.
					movie->_videoPlayback = true;
				}
			}

			if (castId != sprite->_castId) {
				if (!sprite->_trails) {
					movie->getWindow()->addDirtyRect(channel->getBbox());
					channel->_dirty = true;
				}
				channel->setCast(castId);
				channel->_dirty = true;
			}
		}
		break;
	case kTheConstraint:
		{
			int channelId = -1;
			if (d.type == CASTREF) {
				// Reference: CastMember ID
				// Find the first channel that uses this cast.
				CastMemberID memberID = *d.u.cast;
				for (uint i = 0; i < score->_channels.size(); i++) {
					if (score->_channels[i]->_sprite->_castId == memberID) {
						channelId = i;
						break;
					}
				}
			} else {
				channelId = d.asInt();
			}
			if (channelId != -1 && channelId != (int)channel->_constraint) {
				channel->_constraint = d.u.i;
				channel->_dirty = true;
			}
		}
		break;
	case kTheCursor:
		if (d.type == INT) {
			channel->_cursor.readFromResource(d);
		} else {
			channel->_cursor.readFromCast(d);
		}
		score->_cursorDirty = true;
		break;
	case kTheEditableText:
		channel->_sprite->_editable = d.asInt();
		break;
	case kTheForeColor:
		{
			uint32 newColor = g_director->transformColor(d.asInt());
			if (newColor != sprite->_foreColor) {
				sprite->_foreColor = newColor;
				channel->_dirty = true;
			}

			// Based on Director in a Nutshell, page 15
			sprite->setAutoPuppet(kAPForeColor, true);
		}
		break;
	case kTheHeight:
		if (d.asInt() != channel->getHeight()) {
			g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
			channel->setHeight(d.asInt());
			channel->_dirty = true;
		}

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPHeight, true);

		break;
	case kTheImmediate:
		sprite->_immediate = (bool)d.asInt();
		break;
	case kTheInk:
		if (d.asInt() != sprite->_ink) {
			sprite->_ink = static_cast<InkType>(d.asInt());
			channel->_dirty = true;
		}

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPInk, true);

		break;
	case kTheLineSize:
		if (d.asInt() != sprite->_thickness) {
			sprite->_thickness = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheLoc:
		if (channel->getPosition() != d.asPoint()) {
			movie->getWindow()->addDirtyRect(channel->getBbox());
			channel->_dirty = true;
		}
		channel->setPosition(d.asPoint().x, d.asPoint().y);
		break;
	case kTheLocH:
		if (d.asInt() != channel->getPosition().x) {
			// adding the dirtyRect only when the trails is false. Other changes which will add dirtyRect may also apply this patch
			// this is for fixing the bug in jman-win. Currently, i've only patched the LocH, LocV and castNum since those are the only ones used in jman
			if (!channel->_sprite->_trails) {
				movie->getWindow()->addDirtyRect(channel->getBbox());
				channel->_dirty = true;
			}
			channel->setPosition(d.asInt(), channel->getPosition().y);
		}

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPLocH, true);

		break;
	case kTheLocV:
		if (d.asInt() != channel->getPosition().y) {
			if (!channel->_sprite->_trails) {
				movie->getWindow()->addDirtyRect(channel->getBbox());
				channel->_dirty = true;
			}
			channel->setPosition(channel->getPosition().x, d.asInt());
		}

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPLocV, true);

		break;
	case kTheMoveableSprite:
		sprite->_moveable = (bool)d.asInt();

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPMoveable, true);

		break;
	case kTheMovieRate:
		channel->_movieRate = d.asFloat();
		if (sprite->_cast && sprite->_cast->_type == kCastDigitalVideo)
			((DigitalVideoCastMember *)sprite->_cast)->setMovieRate(channel->_movieRate);
		else
			warning("Setting movieTime for non-digital video");
		break;
	case kTheMovieTime:
		channel->_movieTime = d.asInt();
		if (sprite->_cast->_type == kCastDigitalVideo)
			((DigitalVideoCastMember *)sprite->_cast)->seekMovie(channel->_movieTime);
		else
			warning("Setting movieTime for non-digital video");
		break;
	case kThePattern:
		if (d.asInt() != sprite->getPattern()) {
			sprite->setPattern(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kThePuppet:
		sprite->_puppet = (bool)d.asInt();
		if (!d.asInt()) {
			// TODO: Properly reset sprite properties after puppet disabled.
			sprite->_moveable = false;
		}
		break;
	case kTheRect:
		if (d.type == RECT || (d.type == ARRAY && d.u.farr->arr.size() >= 4)) {
			score->updateSprites(kRenderForceUpdate);
			channel->setBbox(
				d.u.farr->arr[0].u.i, d.u.farr->arr[1].u.i,
				d.u.farr->arr[2].u.i, d.u.farr->arr[3].u.i
			);
			channel->_dirty = true;
		}

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPRect, true);

		break;
	case kTheStartTime:
		channel->_startTime = d.asInt();
		if (sprite->_cast->_type == kCastDigitalVideo)
			((DigitalVideoCastMember *)sprite->_cast)->seekMovie(channel->_startTime);
		else
			warning("Setting startTime for non-digital video");
		break;
	case kTheStopTime:
		channel->_stopTime = d.asInt();
		if (sprite->_cast->_type == kCastDigitalVideo)
			((DigitalVideoCastMember *)sprite->_cast)->setStopTime(channel->_stopTime);
		else
			warning("Setting stopTime for non-digital video");
		break;
	case kTheStretch:
		channel->setStretch(d.asInt() != 0);
		break;
	case kTheTrails:
		sprite->_trails = (d.asInt() ? true : false);
		break;
	case kTheType:
		if (d.asInt() != sprite->_spriteType) {
			sprite->_spriteType = static_cast<SpriteType>(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kTheVisibility:
	case kTheVisible:
		if ((bool)d.asInt() != channel->_visible) {
			channel->_visible = (bool)d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheVolume:
		// TODO: Should changing digital video flags mark as dirty?
		sprite->_volume = d.asInt();
		break;
	case kTheWidth:
		if (d.asInt() != channel->getWidth()) {
			g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
			channel->setWidth(d.asInt());
			channel->_dirty = true;
		}

		// Based on Director in a Nutshell, page 15
		sprite->setAutoPuppet(kAPWidth, true);

		break;
	default:
		warning("Lingo::setTheSprite(): Unprocessed setting field \"%s\" of sprite", field2str(field));
	}

	if (channel->_dirty)
		movie->getWindow()->addDirtyRect(channel->getBbox());
}

Datum Lingo::getTheCast(Datum &id1, int field) {
	Datum d;

	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Lingo::getTheCast(): No movie loaded");
		return d;
	}

	CastMemberID id = id1.asMemberID();

	CastMember *member = movie->getCastMember(id);
	if (!member) {
		if (field == kTheLoaded) {
			d = 0;
		} else if (field == kTheNumber) {
			d = -1;
		} else if (id.member <= getMembersNum()) {
			// If a cast member with the ID doesn't exist,
			// but the ID isn't greater than the biggest cast member ID,
			// Lingo will not crash, and instead return a VOID.
			return d;
		} else {
			g_lingo->lingoError("Lingo::getTheCast(): CastMember %s not found", id1.asString().c_str());
		}
		return d;
	}

	if (!member->hasField(field)) {
		warning("Lingo::getTheCast(): %s has no property '%s'", id.asString().c_str(), field2str(field));
		return d;
	}

	d = member->getField(field);

	return d;
}

void Lingo::setTheCast(Datum &id1, int field, Datum &d) {
	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Lingo::setTheCast(): No movie loaded");
		return;
	}

	CastMemberID id = id1.asMemberID();

	CastMember *member = movie->getCastMember(id);
	if (!member) {
		g_lingo->lingoError("Lingo::setTheCast(): %s not found", id.asString().c_str());
		return;
	}

	if (!member->hasField(field)) {
		warning("Lingo::setTheCast(): %s has no property '%s'", id.asString().c_str(), field2str(field));
		return;
	}

	member->setField(field, d);
}

Datum Lingo::getTheField(Datum &id1, int field) {
	Datum d;

	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Lingo::getTheField(): No movie loaded");
		return d;
	}

	CastMemberID id = id1.asMemberID();

	CastMember *member = movie->getCastMember(id);
	if (!member) {
		if (field == kTheLoaded) {
			d = 0;
		} else {
			g_lingo->lingoError("Lingo::getTheField(): %s not found", id.asString().c_str());
		}
		return d;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("Lingo::getTheField(): %s is not a field", id.asString().c_str());
		return d;
	}

	if (!member->hasField(field)) {
		warning("Lingo::getTheField(): %s has no property '%s'", id.asString().c_str(), field2str(field));
		return d;
	}

	d = member->getField(field);

	return d;
}

void Lingo::setTheField(Datum &id1, int field, Datum &d) {
	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Lingo::setTheField(): No movie loaded");
		return;
	}

	CastMemberID id = id1.asMemberID();

	CastMember *member = movie->getCastMember(id);
	if (!member) {
		g_lingo->lingoError("Lingo::setTheField(): %s not found", id.asString().c_str());
		return;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("Lingo::setTheField(): %s is not a field", id.asString().c_str());
		return;
	}

	if (!member->hasField(field)) {
		warning("Lingo::setTheField(): %s has no property '%s'", id.asString().c_str(), field2str(field));
		return;
	}

	member->setField(field, d);
}

Datum Lingo::getTheChunk(Datum &chunk, int field) {
	Datum d;

	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Lingo::getTheChunk(): No movie loaded");
		return d;
	}

	if (chunk.type != CHUNKREF) {
		warning("BUILDBOT: Lingo::getTheChunk(): bad chunk ref type: %s", chunk.type2str());
		return d;
	}

	int start, end;
	start = chunk.u.cref->start;
	end = chunk.u.cref->end;
	Datum src = chunk.u.cref->source;
	while (src.type == CHUNKREF) {
		start += src.u.cref->start;
		end += src.u.cref->start;
		src = src.u.cref->source;
	}
	if (!src.isCastRef()) {
		warning("BUILDBOT: Lingo::getTheChunk(): bad chunk ref field type: %s", src.type2str());
		return d;
	}

	CastMemberID memberID = *src.u.cast;
	CastMember *member = movie->getCastMember(memberID);
	if (!member) {
		g_lingo->lingoError("Lingo::getTheChunk(): %s not found", memberID.asString().c_str());
		return d;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("Lingo::getTheChunk(): %s is not a field", memberID.asString().c_str());
		return d;
	}

	if (!((TextCastMember *)member)->hasChunkField(field)) {
		warning("Lingo::getTheChunk(): %s has no chunk property '%s'", memberID.asString().c_str(), field2str(field));
		return d;
	}

	d = ((TextCastMember *)member)->getChunkField(field, start, end);

	return d;
}

void Lingo::setTheChunk(Datum &chunk, int field, Datum &d) {
	Movie *movie = _vm->getCurrentMovie();
	if (!movie) {
		warning("Lingo::setTheChunk(): No movie loaded");
		return;
	}

	if (chunk.type != CHUNKREF) {
		warning("BUILDBOT: Lingo::setTheChunk(): bad chunk ref type: %s", chunk.type2str());
		return;
	}

	int start, end;
	start = chunk.u.cref->start;
	end = chunk.u.cref->end;
	Datum src = chunk.u.cref->source;
	while (src.type == CHUNKREF) {
		start += src.u.cref->start;
		end += src.u.cref->start;
		src = src.u.cref->source;
	}
	if (!src.isCastRef()) {
		warning("BUILDBOT: Lingo::setTheChunk(): bad chunk ref field type: %s", src.type2str());
		return;
	}

	CastMemberID memberID = *src.u.cast;
	CastMember *member = movie->getCastMember(memberID);
	if (!member) {
		g_lingo->lingoError("Lingo::setTheChunk(): %s not found", memberID.asString().c_str());
		return;
	}
	if (member->_type != kCastText) {
		g_lingo->lingoError("Lingo::setTheChunk(): %s is not a field", memberID.asString().c_str());
		return;
	}

	if (!((TextCastMember *)member)->hasChunkField(field)) {
		warning("Lingo::setTheChunk(): %s has no chunk property '%s'", memberID.asString().c_str(), field2str(field));
		return;
	}

	((TextCastMember *)member)->setChunkField(field, start, end, d);
}

void Lingo::getObjectProp(Datum &obj, Common::String &propName) {
	Datum d;
	if (obj.type == OBJECT) {
		if (obj.u.obj->hasProp(propName)) {
			d = obj.u.obj->getProp(propName);
		} else {
			g_lingo->lingoError("Lingo::getObjectProp: Object <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
		g_lingo->push(d);
		g_debugger->propReadHook(propName);
		return;
	}
	if (obj.type == PARRAY) {
		int index = LC::compareArrays(LC::eqData, obj, propName, true).u.i;
		if (index > 0) {
			d = obj.u.parr->arr[index - 1].v;
		}
		g_lingo->push(d);
		g_debugger->propReadHook(propName);
		return;
	}
	if (obj.type == POINT) {
		if (propName.equalsIgnoreCase("locH")) {
			d = obj.u.farr->arr[0];
		} else if (propName.equalsIgnoreCase("locV")) {
			d = obj.u.farr->arr[1];
		} else {
			g_lingo->lingoError("Lingo::getObjectProp: Point <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
		g_lingo->push(d);
		g_debugger->propReadHook(propName);
		return;
	}
	if (obj.type == RECT) {
		if (propName.equalsIgnoreCase("left")) {
			d = obj.u.farr->arr[0];
		} else if (propName.equalsIgnoreCase("top")) {
			d = obj.u.farr->arr[1];
		} else if (propName.equalsIgnoreCase("right")) {
			d = obj.u.farr->arr[2];
		} else if (propName.equalsIgnoreCase("bottom")) {
			d = obj.u.farr->arr[3];
		} else {
			g_lingo->lingoError("Lingo::getObjectProp: Rect <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
		g_lingo->push(d);
		g_debugger->propReadHook(propName);
		return;
	}
	if (obj.type == CASTREF) {
		Movie *movie = _vm->getCurrentMovie();
		if (!movie) {
			g_lingo->lingoError("Lingo::getObjectProp(): No movie loaded");
			g_lingo->push(d);
			return;
		}

		CastMemberID id = *obj.u.cast;
		CastMember *member = movie->getCastMember(id);
		if (!member) {
			// No matching cast member. Many of the fields are accessible
			// to indicate the cast member is empty, however the
			// rest will throw a Lingo error.
			Common::String key = Common::String::format("%d%s", kTheCast, propName.c_str());
			bool emptyAllowed = false;
			if (_theEntityFields.contains(key)) {
				emptyAllowed = true;
				switch (_theEntityFields[key]->field) {
				case kTheCastType:
				case kTheType:
					d = Datum("empty");
					d.type = SYMBOL;
					break;
				case kTheFileName:
				case kTheScriptText:
					d = Datum("");
					break;
				case kTheHeight:
				case kTheLoaded:
				case kTheModified:
				case kThePurgePriority:
				case kTheWidth:
				case kTheCenter:
				case kTheFrameRate:
				case kThePausedAtStart:
				case kThePreLoad:
				case kTheDepth:
				case kThePalette:
					d = Datum(0);
					break;
				case kTheCrop:
				case kTheVideo:
					d = Datum(1);
					break;
				case kTheRect:
					d = Datum(Common::Rect(0, 0, 0, 0));
					break;
				case kTheRegPoint:
					d = Datum(Common::Point(0, 0));
					break;
				case kTheNumber:
					d = Datum(id.member);
					break;
				default:
					emptyAllowed = false;
					break;
				}
			}

			if (id.member <= getMembersNum()) {
				// Cast member ID is within range (i.e. less than max)
				// In real Director, accessing -any- of the properties will
				// be allowed, but return garbage.
				warning("Lingo::getObjectProp(): %s not found, but within cast ID range", id.asString().c_str());
			} else if (!emptyAllowed) {
				// Cast member ID is out of range, throw a Lingo error
				g_lingo->lingoError("Lingo::getObjectProp(): %s not found and out of cast ID range", id.asString().c_str());
			}

			g_lingo->push(d);
			return;
		}

		if (member->hasProp(propName)) {
			d = member->getProp(propName);
		} else {
			g_lingo->lingoError("Lingo::getObjectProp(): %s has no property '%s'", id.asString().c_str(), propName.c_str());
		}
		g_lingo->push(d);
		return;
	}
	if (_builtinFuncs.contains(propName) && _builtinFuncs[propName].nargs == 1) {
		push(obj);
		LC::call(_builtinFuncs[propName], 1, true);
		return;
	}
	g_lingo->lingoError("Lingo::getObjectProp: Invalid object: %s", obj.asString(true).c_str());
	g_lingo->push(d);
}

void Lingo::setObjectProp(Datum &obj, Common::String &propName, Datum &val) {
	if (obj.type == OBJECT) {
		if (obj.u.obj->hasProp(propName)) {
			obj.u.obj->setProp(propName, val);
			g_debugger->propWriteHook(propName);
		} else {
			g_lingo->lingoError("Lingo::setObjectProp: Object <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
	} else if (obj.type == PARRAY) {
		int index = LC::compareArrays(LC::eqData, obj, propName, true).u.i;
		if (index > 0) {
			obj.u.parr->arr[index - 1].v = val;
		} else {
			PCell cell = PCell(propName, val);
			obj.u.parr->arr.push_back(cell);
		}
		g_debugger->propWriteHook(propName);
	} else if (obj.type == POINT) {
		if (propName.equalsIgnoreCase("locH")) {
			obj.u.farr->arr[0] = val.asInt();
		} else if (propName.equalsIgnoreCase("locV")) {
			obj.u.farr->arr[1] = val.asInt();
		} else {
			g_lingo->lingoError("Lingo::setObjectProp: Point <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
		g_debugger->propWriteHook(propName);
		return;
	} else if (obj.type == RECT) {
		if (propName.equalsIgnoreCase("left")) {
			obj.u.farr->arr[0] = val.asInt();
		} else if (propName.equalsIgnoreCase("top")) {
			obj.u.farr->arr[1] = val.asInt();
		} else if (propName.equalsIgnoreCase("right")) {
			obj.u.farr->arr[2] = val.asInt();
		} else if (propName.equalsIgnoreCase("bottom")) {
			obj.u.farr->arr[3] = val.asInt();
		} else {
			g_lingo->lingoError("Lingo::setObjectProp: Rect <%s> has no property '%s'", obj.asString(true).c_str(), propName.c_str());
		}
		g_debugger->propWriteHook(propName);
		return;
	} else if (obj.type == CASTREF) {
		Movie *movie = _vm->getCurrentMovie();
		if (!movie) {
			g_lingo->lingoError("Lingo::setObjectProp(): No movie loaded");
			return;
		}

		CastMemberID id = *obj.u.cast;
		CastMember *member = movie->getCastMember(id);
		if (!member) {
			g_lingo->lingoError("Lingo::setObjectProp(): %s not found", id.asString().c_str());
			return;
		}

		if (member->hasProp(propName)) {
			member->setProp(propName, val);
		} else {
			g_lingo->lingoError("Lingo::setObjectProp(): %s has no property '%s'", id.asString().c_str(), propName.c_str());
		}
	} else {
		g_lingo->lingoError("Lingo::setObjectProp: Invalid object: %s", obj.asString(true).c_str());
	}
}

static const char *mfull[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};

static const char *wday[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

Datum Lingo::getTheDate(int field) {
	TimeDate t;
	g_system->getTimeAndDate(t);

	if (g_director->_forceDate.tm_year != -1) {
		// Override date portion
		t.tm_year = g_director->_forceDate.tm_year;
		t.tm_mon = g_director->_forceDate.tm_mon;
		t.tm_wday = g_director->_forceDate.tm_wday;
		t.tm_mday = g_director->_forceDate.tm_mday;
	}

	Common::String s;

	Datum d;
	d.type = STRING;

	const char *m = mfull[t.tm_mon];
	const char *w = wday[t.tm_wday];

	switch (field) {
	case kTheAbbr:	// "Sat, Sep 7, 1991"
		s = Common::String::format("%c%c%c, %c%c%c %d, %d", w[0], w[1], w[2], m[0], m[1], m[2], t.tm_mday, t.tm_year + 1900);
		break;

	case kTheLong:	// "Saturday, September 7, 1991"
		s = Common::String::format("%s, %s %d, %d", w, m, t.tm_mday, t.tm_year + 1900);
		break;

	default:		// "9/7/91"
		s = Common::String::format("%d/%d/%02d", t.tm_mday, t.tm_mon, t.tm_year % 100);
		break;
	}

	d.u.s = new Common::String(s);

	return d;
}

Datum Lingo::getTheTime(int field) {
	TimeDate t;
	g_system->getTimeAndDate(t);

	Common::String s;

	Datum d;
	d.type = STRING;

	switch (field) {
	case kTheLong:
		s = Common::String::format("%d:%02d:%02d %s", t.tm_hour % 12, t.tm_min, t.tm_sec, t.tm_hour < 12 ? "AM" : "PM");
		break;

	default:
		s = Common::String::format("%d:%02d %s", t.tm_hour % 12, t.tm_min, t.tm_hour < 12 ? "AM" : "PM");
		break;
	}

	d.u.s = new Common::String(s);

	return d;
}

Datum Lingo::getTheDeskTopRectList() {
	// Returns dimensions of each monitor
	Datum monitorSize;
	monitorSize.type = RECT;
	monitorSize.u.farr = new FArray;
	monitorSize.u.farr->arr.push_back(0);
	monitorSize.u.farr->arr.push_back(0);
	monitorSize.u.farr->arr.push_back(g_director->getMacWindowManager()->getWidth());
	monitorSize.u.farr->arr.push_back(g_director->getMacWindowManager()->getHeight());

	Datum d;
	d.type = ARRAY;
	d.u.farr = new FArray;
	d.u.farr->arr.push_back(monitorSize);

	return d;
}

} // End of namespace Director
