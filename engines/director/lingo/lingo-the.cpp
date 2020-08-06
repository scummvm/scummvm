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

#include "graphics/macgui/macbutton.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/score.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-the.h"
#include "director/lingo/lingo-gr.h"
namespace Director {

class Sprite;

TheEntity entities[] = {
	{ kTheActorList,		"actorList",		false, 4 },	//			D4 property
	{ kTheBeepOn,			"beepOn",			false, 2 },	// D2 p
	{ kTheButtonStyle,		"buttonStyle",		false, 2 },	// D2 p
	{ kTheCast,				"cast",				true,  2 },	// D2
	{ kTheCastMembers,		"castmembers",		false, 3 },	//		 D3
	{ kTheCenterStage,		"centerStage",		false, 2 },	// D2 p
	{ kTheCheckBoxAccess,	"checkBoxAccess",	false, 2 },	// D2 p
	{ kTheCheckBoxType,		"checkBoxType",		false, 2 },	// D2 p
	{ kTheClickLoc,			"clickLoc",			false, 4 },	// 			D4 function
	{ kTheClickOn,			"clickOn",			false, 2 },	// D2 f
	{ kTheColorDepth,		"colorDepth",		false, 2 },	// D2 p
	{ kTheColorQD,			"colorQD",			false, 2 },	// D2 f
	{ kTheCommandDown,		"commandDown",		false, 2 },	// D2 f
	{ kTheControlDown,		"controlDown",		false, 2 },	// D2 f
	{ kTheDate,				"date",				false, 3 },	//		D3 f
	{ kTheDoubleClick,		"doubleClick",		false, 2 },	// D2 f
	{ kTheExitLock,			"exitLock",			false, 2 },	// D2 p
	{ kTheField,			"field",			true,  3 },	//		D3
	{ kTheFixStageSize,		"fixStageSize",		false, 2 },	// D2 p
	{ kTheFloatPrecision,	"floatPrecision",	false, 3 },	//		D3 p
	{ kTheFrame,			"frame",			false, 2 },	// D2 f
	{ kTheFrameLabel,		"frameLabel",		false, 4 },	//			D4 p
	{ kTheFrameScript,		"frameScript",		false, 4 },	//			D4 p
	{ kTheFramePalette,		"framePalette",		false, 4 },	//			D4 p
	{ kTheFrameTempo,		"frameTempo",		false, 4 },	//			D4 f
	{ kTheFreeBlock,		"freeBlock",		false, 2 },	// D2 f
	{ kTheFreeBytes,		"freeBytes",		false, 2 },	// D2 f
	{ kTheFullColorPermit,	"fullColorPermit",	false, 2 },	// D2 p
	{ kTheImageDirect,		"imageDirect",		false, 2 },	// D2 p
	{ kTheItemDelimiter,	"itemDelimiter",	false, 4 },	//			D4 p
	{ kTheKey,				"key",				false, 2 },	// D2 f
	{ kTheKeyCode,			"keyCode",			false, 2 },	// D2 f
	{ kTheKeyDownScript,	"keyDownScript",	false, 2 },	// D2 p
	{ kTheKeyUpScript,		"keyUpScript",		false, 4 },	//			D4 p
	{ kTheLabelList,		"labelList",		false, 3 },	//		D3 f
	{ kTheLastClick,		"lastClick",		false, 2 },	// D2 f
	{ kTheLastEvent,		"lastEvent",		false, 2 },	// D2 f
	{ kTheLastFrame,		"lastFrame",		false, 4 },	//			D4 p
	{ kTheLastKey,			"lastKey",			false, 2 },	// D2 f
	{ kTheLastRoll,			"lastRoll",			false, 2 },	// D2 f
	{ kTheMachineType,		"machineType",		false, 2 },	// D2 f
	{ kTheMaxInteger,		"maxInteger",		false, 3 },	//		D3.1 f
	{ kTheMemorySize,		"memorySize",		false, 2 },	// D2 f
	{ kTheMenu,				"menu",				true,  3 },	//		D3 p
	{ kTheMenuItem,			"menuitem",			true,  3 },	//		D3 p
	{ kTheMenuItems,		"menuitems",		false, 3 },	//		D3 f
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
	{ kTheMovieFileFreeSize,"movieFileFreeSize",false, 4 },	//			D4 f
	{ kTheMovieFileSize,	"movieFileSize",	false, 4 },	//			D4 f
	{ kTheMovieName,		"movieName",		false, 4 },	//			D4 f
	{ kTheMoviePath,		"moviePath",		false, 4 },	//			D4 f
	{ kTheMultiSound,		"multiSound",		false, 3 },	//		D3.1 f
	{ kTheOptionDown,		"optionDown",		false, 2 },	// D2 f
	{ kTheParamCount,		"paramCount",		false, 4 },	//			D4 f
	{ kThePathName,			"pathName",			false, 2 },	// D2 f
	{ kThePauseState,		"pauseState",		false, 2 },	// D2 f
	{ kThePerFrameHook,		"perFrameHook",		false, 2 },	// D2 p
	{ kThePreloadEventAbort,"preloadEventAbort",false, 4 },	//			D4 p
	{ kThePreLoadRAM,		"preLoadRAM",		false, 4 },	//			D4 p
	{ kThePi,				"pi",				false, 4 },	//			D4 f
	{ kTheQuickTimePresent,	"quickTimePresent",	false, 3 },	//		D3.1 f
	{ kTheRandomSeed,		"randomSeed",		false, 4 },	//			D4 p
	{ kTheResult,			"result",			false, 2 },	// D2 f
	{ kTheRightMouseDown,	"rightMouseDown",	false, 5 },	//					D5 f
	{ kTheRightMouseUp,		"rightMouseUp",		false, 5 },	//					D5 f
	{ kTheRomanLingo,		"romanLingo",		false, 3 },	//		D3.1 p
	{ kTheScummvmVersion,	"scummvmVersion",	false, 2 }, // 					ScummVM only
	{ kTheSearchCurrentFolder,"searchCurrentFolder",false,4 },//		D4 f
	{ kTheSearchPath,		"searchPath",		false, 4 },	//			D4 f
	{ kTheSelection,		"selection",		false, 2 },	// D2 f
	{ kTheSelEnd,			"selEnd",			false, 2 },	// D2 p
	{ kTheSelStart,			"selStart",			false, 2 },	// D2 p
	{ kTheShiftDown,		"shiftDown",		false, 2 },	// D2 f
	{ kTheSoundEnabled,		"soundEnabled",		false, 2 },	// D2 p
	{ kTheSoundEntity,		"sound",			true,  3 },	// 		D3 p
	{ kTheSoundLevel,		"soundLevel",		false, 2 },	// D2 p
	{ kTheSprite,			"sprite",			true,  2 },	// 			D4 p
	{ kTheStage,			"stage",			false, 4 },	//			D4 p
	{ kTheStageBottom,		"stageBottom",		false, 2 },	// D2 f
	{ kTheStageColor,		"stageColor",		false, 3 },	//		D3 p
	{ kTheStageLeft,		"stageLeft",		false, 2 },	// D2 f
	{ kTheStageRight,		"stageRight",		false, 2 },	// D2 f
	{ kTheStageTop,			"stageTop",			false, 2 },	// D2 f
	{ kTheStillDown,		"stillDown",		false, 2 },	// D2 f
	{ kTheSwitchColorDepth,	"switchColorDepth",	false, 2 },	// D2 p
	{ kTheTicks,			"ticks",			false, 2 },	// D2 f
	{ kTheTime,				"time",				false, 3 },	// 		D3 f
	{ kTheTimeoutKeyDown,	"timeoutKeyDown",	false, 2 },	// D2 p
	{ kTheTimeoutLapsed,	"timeoutLapsed",	false, 2 },	// D2 p
	{ kTheTimeoutLength,	"timeoutLength",	false, 2 },	// D2 p
	{ kTheTimeoutMouse,		"timeoutMouse",		false, 2 },	// D2 p
	{ kTheTimeoutPlay,		"timeoutPlay",		false, 2 },	// D2 p
	{ kTheTimeoutScript,	"timeoutScript",	false, 2 },	// D2 p
	{ kTheTimer,			"timer",			false, 2 },	// D2 p
	{ kTheTrace,			"trace",			false, 4 },	//			D4 p
	{ kTheTraceLoad,		"traceLoad",		false, 4 },	//			D4 p
	{ kTheTraceLogFile,		"traceLogFile",		false, 4 },	//			D4 p
	{ kTheUpdateMovieEnabled,"updateMovieEnabled",false,4 },//			D4 p
	{ kTheWindow,			"window",			true,  4 },	//			D4
	{ kTheWindowList,		"windowList",		false, 4 },	//			D4 p
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

	// TextCastMember fields
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

	{ kTheSoundEntity,"volume",		kTheVolume,		3 },//		D3 p

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

#define getTheEntitySTUB(entity) \
	warning("Lingo::getTheEntity(): Unprocessed getting entity %s", entity2str(entity));

Datum Lingo::getTheEntity(int entity, Datum &id, int field) {
	if (debugChannelSet(3, kDebugLingoExec)) {
		debugC(3, kDebugLingoExec, "Lingo::getTheEntity(%s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field));
	}

	Datum d;
	Movie *movie = _vm->getCurrentMovie();

	if (!movie) {
		warning("Lingo::getTheEntity(): Movie is missing");
		d.type = VOID;

		return d;
	}

	LingoArchive *mainArchive = movie->getMainLingoArch();
	Score *score = movie->getScore();

	switch (entity) {
	case kTheActorList:
		getTheEntitySTUB(kTheActorList);
		break;
	case kTheBeepOn:
		getTheEntitySTUB(kTheBeepOn);
		break;
	case kTheButtonStyle:
		getTheEntitySTUB(kTheButtonStyle);
		break;
	case kTheCast:
		d = getTheCast(id, field);
		break;
	case kTheCastMembers:
		warning("STUB: Lingo::getTheEntity(): Unprocessed getting field %s of entity %s", field2str(field), entity2str(entity));
		break;
	case kTheCenterStage:
		d.type = INT;
		d.u.i = g_director->_centerStage;
		break;
	case kTheCheckBoxAccess:
		getTheEntitySTUB(kTheCheckBoxAccess);
		break;
	case kTheCheckBoxType:
		getTheEntitySTUB(kTheCheckBoxType);
		break;
	case kTheClickLoc:
		d.u.farr = new DatumArray;

		d.u.farr->push_back(movie->_lastClickPos.x);
		d.u.farr->push_back(movie->_lastClickPos.y);
		d.type = POINT;
		break;
	case kTheClickOn:
		d.type = INT;
		d.u.i = movie->_currentClickOnSpriteId;
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
	case kTheCommandDown:
		d.type = INT;
		d.u.i = (movie->_keyFlags & Common::KBD_META) ? 1 : 0;
		break;
	case kTheControlDown:
		d.type = INT;
		d.u.i = (movie->_keyFlags & Common::KBD_CTRL) ? 1 : 0;
		break;
	case kTheDate:
		d = getTheDate(field);
		break;
	case kTheDoubleClick:
		getTheEntitySTUB(kTheDoubleClick);
		break;
	case kTheExitLock:
		getTheEntitySTUB(kTheExitLock);
		break;
	case kTheFixStageSize:
		getTheEntitySTUB(kTheFixStageSize);
		break;
	case kTheFloatPrecision:
		d.type = INT;
		d.u.i = _floatPrecision;
		break;
	case kTheFrame:
		d.type = INT;
		d.u.i = score->getCurrentFrame();
		break;
	case kTheFrameLabel:
		d.type = STRING;
		d.u.s = score->getFrameLabel(score->getCurrentFrame());
		break;
	case kTheFrameScript:
		getTheEntitySTUB(kTheFrameScript);
		break;
	case kTheFramePalette:
		d.type = INT;
		d.u.i = score->getCurrentPalette();
		break;
	case kTheFrameTempo:
		d.type = INT;
		d.u.i = score->_currentFrameRate;
		break;
	case kTheFreeBlock:
	case kTheFreeBytes:
		d.type = INT;
		d.u.i = 32 * 1024 * 1024;	// Let's have 32 Mbytes
		break;
	case kTheFullColorPermit:
		d.type = INT;
		d.u.i = 1;					// We always allow it in ScummVM
		break;
	case kTheImageDirect:
		d.type = INT;
		d.u.i = 1;					// We always allow it in ScummVM
		break;
	case kTheItemDelimiter:
		d.type = STRING;
		d.u.s = new Common::String(g_lingo->_itemDelimiter);
		break;
	case kTheKey:
		d.type = STRING;
		d.u.s = new Common::String(movie->_key);
		break;
	case kTheKeyCode:
		d.type = INT;
		d.u.i = movie->_keyCode;
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
		d.type = INT;
		d.u.i = _vm->getMacTicks() - movie->_lastClickTime;
		break;
	case kTheLastEvent:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - movie->_lastEventTime;
		break;
	case kTheLastFrame:
		d.type = INT;
		d.u.i = score->_frames.size() - 1;
		break;
	case kTheLastKey:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - movie->_lastKeyTime;
		break;
	case kTheLastRoll:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - movie->_lastRollTime;
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
	case kTheMaxInteger:
		getTheEntitySTUB(kTheMaxInteger);
		break;
	case kTheMemorySize:
		d.type = INT;
		d.u.i = 32 * 1024 * 1024;	// Let's have 32 Mbytes
		break;
	case kTheMenu:
		getTheEntitySTUB(kTheMenu);
		break;
	case kTheMenuItem:
		getTheEntitySTUB(kTheMenuItem);
		break;
	case kTheMenuItems:
		getTheEntitySTUB(kTheMenuItems);
		break;
	case kTheMouseCast:
		{
			Common::Point pos = g_director->getCurrentWindow()->getMousePos();
			uint16 spriteId = score->getSpriteIDFromPos(pos);
			d.type = INT;
			d.u.i = score->getSpriteById(spriteId)->_castId;
			if (d.u.i == 0)
				d.u.i = -1;
		}
		break;
	case kTheMouseChar:
		getTheEntitySTUB(kTheMouseChar);
		break;
	case kTheMouseDown:
		d.type = INT;
		d.u.i = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT | 1 << Common::MOUSE_BUTTON_RIGHT) ? 1 : 0;
		break;
	case kTheMouseDownScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventMouseDown))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventMouseDown]);
		else
			d.u.s = new Common::String();
		break;
	case kTheMouseH:
		d.type = INT;
		d.u.i = g_director->getCurrentWindow()->getMousePos().x;
		break;
	case kTheMouseItem:
		getTheEntitySTUB(kTheMouseItem);
		break;
	case kTheMouseLine:
		getTheEntitySTUB(kTheMouseLine);
		break;
	case kTheMouseUp:
		d.type = INT;
		d.u.i = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT | 1 << Common::MOUSE_BUTTON_RIGHT) ? 0 : 1;
		break;
	case kTheMouseUpScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventMouseUp))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventMouseUp]);
		else
			d.u.s = new Common::String();
		break;
	case kTheMouseV:
		d.type = INT;
		d.u.i = g_director->getCurrentWindow()->getMousePos().y;
		break;
	case kTheMouseWord:
		getTheEntitySTUB(kTheMouseWord);
		break;
	case kTheMovie:
	case kTheMovieName:
		d.type = STRING;
		d.u.s = new Common::String(movie->getMacName());
		break;
	case kTheMovieFileFreeSize:
		d.type = INT;
		d.u.i = 0;	// Let's pretend the movie is compactified
		break;
	case kTheMovieFileSize:
		d.type = INT;
		d.u.i = movie->getArchive()->getFileSize();
		break;
	case kTheMoviePath:
	case kThePathName:
		d.type = STRING;
		d.u.s = new Common::String(unixToMacPath(_vm->getCurrentPath()));
		break;
	case kTheMultiSound:
		// We always support multiple sound channels!
		d.type = INT;
		d.u.i = 1;
		break;
	case kTheOptionDown:
		d.type = INT;
		d.u.i = (movie->_keyFlags & Common::KBD_ALT) ? 1 : 0;
		break;
	case kThePauseState:
		getTheEntitySTUB(kThePauseState);
		break;
	case kThePerFrameHook:
		d = _perFrameHook;
		break;
	case kThePreloadEventAbort:
		getTheEntitySTUB(kThePreloadEventAbort);
		break;
	case kThePreLoadRAM:
		d.u.i = 0;		// We always have unlimited RAM
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
	case kTheRandomSeed:
		d.type = INT;
		d.u.i = g_director->_rnd.getSeed();
		break;
	case kTheResult:
		d = g_lingo->_theResult;
		break;
	case kTheRightMouseDown:
		getTheEntitySTUB(kTheRightMouseDown);
		break;
	case kTheRightMouseUp:
		getTheEntitySTUB(kTheRightMouseUp);
		break;
	case kTheRomanLingo:
		getTheEntitySTUB(kTheRomanLingo);
		break;
	case kTheScummvmVersion:
		d.type = INT;
		d.u.i = _vm->getVersion();
		break;
	case kTheSearchCurrentFolder:
		getTheEntitySTUB(kTheSearchCurrentFolder);
		break;
	case kTheSearchPath:
		getTheEntitySTUB(kTheSearchPath);
		break;
	case kTheSelection:
		if (movie->_currentEditableTextChannel) {
			Channel *channel = score->_channels[movie->_currentEditableTextChannel];

			if (channel->_widget) {
				d.type = STRING;
				d.u.s = new Common::String(Common::convertFromU32String(((Graphics::MacText *)channel->_widget)->getSelection()));
			}
		}
		break;
	case kTheSelEnd:
	case kTheSelStart:
		if (movie->_currentEditableTextChannel) {
			Channel *channel = score->_channels[movie->_currentEditableTextChannel];

			if (channel->_widget) {
				d.type = INT;
				d.u.i = ((Graphics::MacText *)channel->_widget)->getSelectionIndex(entity == kTheSelStart);
			}
		}
		break;
	case kTheShiftDown:
		d.type = INT;
		d.u.i = (movie->_keyFlags & Common::KBD_SHIFT) ? 1 : 0;
		break;
	case kTheSoundEnabled:
		getTheEntitySTUB(kTheSoundEnabled);
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
	case kTheSoundLevel:
		getTheEntitySTUB(kTheSoundLevel);
		break;
	case kTheSprite:
		d = getTheSprite(id, field);
		break;
	case kTheStage:
		d = _vm->getStage();
		break;
	case kTheStageBottom:
		d.type = INT;
		d.u.i = movie->_movieRect.bottom;
		break;
	case kTheStageColor:
		getTheEntitySTUB(kTheStageColor);
		break;
	case kTheStageLeft:
		d.type = INT;
		d.u.i = movie->_movieRect.left;
		break;
	case kTheStageRight:
		d.type = INT;
		d.u.i = movie->_movieRect.right;
		break;
	case kTheStageTop:
		d.type = INT;
		d.u.i = movie->_movieRect.top;
		break;
	case kTheStillDown:
		d.type = INT;
		d.u.i = _vm->_wm->_mouseDown;
		break;
	case kTheSwitchColorDepth:
		getTheEntitySTUB(kTheSwitchColorDepth);
		break;
	case kTheTicks:
		d.type = INT;
		d.u.i = _vm->getMacTicks();
		break;
	case kTheTime:
		d = getTheTime(field);
		break;
	case kTheTimeoutKeyDown:
		getTheEntitySTUB(kTheTimeoutKeyDown);
		break;
	case kTheTimeoutLapsed:
		getTheEntitySTUB(kTheTimeoutLapsed);
		break;
	case kTheTimeoutLength:
		getTheEntitySTUB(kTheTimeoutLength);
		break;
	case kTheTimeoutMouse:
		getTheEntitySTUB(kTheTimeoutMouse);
		break;
	case kTheTimeoutPlay:
		getTheEntitySTUB(kTheTimeoutPlay);
		break;
	case kTheTimeoutScript:
		d.type = STRING;
		if (mainArchive->primaryEventHandlers.contains(kEventTimeout))
			d.u.s = new Common::String(mainArchive->primaryEventHandlers[kEventTimeout]);
		else
			d.u.s = new Common::String();
		break;
	case kTheTimer:
		d.type = INT;
		d.u.i = _vm->getMacTicks() - movie->_lastTimerReset;
		break;
	case kTheTrace:
		getTheEntitySTUB(kTheTrace);
		break;
	case kTheTraceLoad:
		getTheEntitySTUB(kTheTraceLoad);
		break;
	case kTheTraceLogFile:
		getTheEntitySTUB(kTheTraceLogFile);
		break;
	case kTheUpdateMovieEnabled:
		getTheEntitySTUB(kTheUpdateMovieEnabled);
		break;
	case kTheWindow:
		g_lingo->push(id);
		LB::b_window(1);
		d = g_lingo->pop().u.obj->getField(field);
		break;
	case kTheWindowList:
		d = g_lingo->_windowList;
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
	if (debugChannelSet(3, kDebugLingoExec)) {
		debugC(3, kDebugLingoExec, "Lingo::setTheEntity(%s, %s, %s, %s)", entity2str(entity), id.asString(true).c_str(), field2str(field), d.asString(true).c_str());
	}

	Movie *movie = _vm->getCurrentMovie();
	Score *score = movie->getScore();

	switch (entity) {
	case kTheActorList:
		setTheEntitySTUB(kTheActorList);
		break;
	case kTheBeepOn:
		setTheEntitySTUB(kTheBeepOn);
		break;
	case kTheButtonStyle:
		if (d.asInt())
			g_director->_wm->_mode = Director::wmMode | Graphics::kWMModeButtonDialogStyle;
		else
			g_director->_wm->_mode = Director::wmMode;
		break;
	case kTheCast:
		setTheCast(id, field, d);
		break;
	case kTheCenterStage:
		g_director->_centerStage = d.asInt();
		break;
	case kTheCheckBoxAccess:
		setTheEntitySTUB(kTheCheckBoxAccess);
		break;
	case kTheCheckBoxType:
		setTheEntitySTUB(kTheCheckBoxType);
		break;
	case kTheColorDepth:
		_vm->_colorDepth = d.asInt();

		// bpp. 1, 2, 4, 8, 32
		warning("STUB: Lingo::setTheEntity(): Set color depth to %d", _vm->_colorDepth);
		break;
	case kTheExitLock:
		setTheEntitySTUB(kTheExitLock);
		break;
	case kTheFixStageSize:
		setTheEntitySTUB(kTheFixStageSize);
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
		setTheEntitySTUB(kTheFrameScript);
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
			g_lingo->_itemDelimiter = *"";
		else
			g_lingo->_itemDelimiter = d.asString()[0];
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
		setTheEntitySTUB(kTheMenuItem);
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
		setTheEntitySTUB(kThePreloadEventAbort);
		break;
	case kThePreLoadRAM:
		// We always have the unlimited RAM, ignore
		break;
	case kTheRandomSeed:
		g_director->_rnd.setSeed(d.asInt());
		break;
	case kTheRomanLingo:
		setTheEntitySTUB(kTheRomanLingo);
		break;
	case kTheScummvmVersion:
		// Allow director version change: used for testing version differences via the lingo tests.
		_vm->setVersion(d.asInt());
		break;
	case kTheSelEnd:
		if (movie->_currentEditableTextChannel != 0) {
			Channel *channel = score->getChannelById(movie->_currentEditableTextChannel);

			if (channel->_widget)
				(((Graphics::MacText *)channel->_widget)->setSelection(d.asInt(), false));
		}
		break;
	case kTheSelStart:
		if (movie->_currentEditableTextChannel != 0) {
			Channel *channel = score->getChannelById(movie->_currentEditableTextChannel);

			if (channel->_widget)
				(((Graphics::MacText *)channel->_widget)->setSelection(d.asInt(), true));
		}
		break;
	case kTheSoundEnabled:
		setTheEntitySTUB(kTheSoundEnabled);
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
	case kTheSoundLevel:
		setTheEntitySTUB(kTheSoundLevel);
		break;
	case kTheSprite:
		setTheSprite(id, field, d);
		break;
	case kTheStage:
		setTheEntitySTUB(kTheStage);
		break;
	case kTheStageColor:
		g_director->getCurrentWindow()->setStageColor(d.asInt());

		// Queue an immediate update of the stage
		if (!score->getNextFrame())
			score->setCurrentFrame(score->getCurrentFrame());
		break;
	case kTheSwitchColorDepth:
		setTheEntitySTUB(kTheSwitchColorDepth);
		break;
	case kTheTimeoutKeyDown:
		setTheEntitySTUB(kTheTimeoutKeyDown);
		break;
	case kTheTimeoutLapsed:
		setTheEntitySTUB(kTheTimeoutLapsed);
		break;
	case kTheTimeoutLength:
		setTheEntitySTUB(kTheTimeoutLength);
		break;
	case kTheTimeoutMouse:
		setTheEntitySTUB(kTheTimeoutMouse);
		break;
	case kTheTimeoutPlay:
		setTheEntitySTUB(kTheTimeoutPlay);
		break;
	case kTheTimeoutScript:
		movie->setPrimaryEventHandler(kEventTimeout, d.asString());
		break;
	case kTheTimer:
		setTheEntitySTUB(kTheTimer);
		break;
	case kTheTrace:
		setTheEntitySTUB(kTheTrace);
		break;
	case kTheTraceLoad:
		setTheEntitySTUB(kTheTraceLoad);
		break;
	case kTheTraceLogFile:
		setTheEntitySTUB(kTheTraceLogFile);
		break;
	case kTheUpdateMovieEnabled:
		setTheEntitySTUB(kTheUpdateMovieEnabled);
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
}

Datum Lingo::getTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId) {
	warning("STUB: getTheMenuItemEntity(%s, %s, %s, %s)", entity2str(entity), menuId.asString(true).c_str(), field2str(field),
				menuItemId.asString(true).c_str());

	return Datum();
}

void Lingo::setTheMenuItemEntity(int entity, Datum &menuId, int field, Datum &menuItemId, Datum &d) {
	warning("STUB: setTheMenuItemEntity(%s, %s, %s, %s, %s)", entity2str(entity), menuId.asString(true).c_str(), field2str(field),
				menuItemId.asString(true).c_str(), d.asString(true).c_str());
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
	case kTheCursor:
		if (channel->_cursor._cursorResId) {
			d.u.i = channel->_cursor._cursorResId;
		} else {
			d.type = ARRAY;
			d.u.farr = new DatumArray(2);

			d.u.farr->operator[](0) = (int)channel->_cursor._cursorCastId;
			d.u.farr->operator[](1) = (int)channel->_cursor._cursorMaskId;
		}
		break;
	case kTheEditableText:
		d.u.i = sprite->_cast ? sprite->_cast->isEditable() : 0;
		break;
	case kTheForeColor:
		d.u.i = sprite->_foreColor;
		break;
	case kTheHeight:
		d.u.i = channel->_height;
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
	case kTheLoc:
		warning("STUB: Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
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
		d.u.i = channel->_movieRate;
		break;
	case kTheMovieTime:
		d.u.i = channel->_movieTime;
		break;
	case kThePattern:
		d.u.i = sprite->getPattern();
		break;
	case kThePuppet:
		d.u.i = sprite->_puppet;
		break;
	case kTheRect:
		warning("STUB: Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
		break;
	case kTheRight:
		d.u.i = channel->getBbox().right;
		break;
	case kTheScoreColor:
		warning("STUB: Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
		break;
	case kTheScriptNum:
		warning("STUB: Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
		break;
	case kTheStartTime:
		d.u.i = channel->_startTime;
		break;
	case kTheStopTime:
		d.u.i = channel->_stopTime;
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
		d.u.i = channel->_width;
		break;
	default:
		warning("Lingo::getTheSprite(): Unprocessed getting field \"%s\" of sprite", field2str(field));
		d.type = VOID;
	}

	return d;
}

void Lingo::setTheSprite(Datum &id1, int field, Datum &d) {
	int id = 0;
	Score *score = _vm->getCurrentMovie()->getScore();

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
		if (d.asInt() != sprite->_backColor) {
			sprite->_backColor = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheBlend:
		if (d.asInt() != sprite->_blend) {
			sprite->_blend = (d.asInt() == 100 ? 0 : d.asInt());
			channel->_dirty = true;
		}
		break;
	case kTheCastNum:
		if (d.asInt() != sprite->_castId) {
			g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
			channel->setCast(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kTheConstraint:
		if (d.asInt() != (int)channel->_constraint) {
			if (d.type == CASTREF) {
				// Reference: CastMember ID
				// Find the first channel that uses this cast.
				for (uint i = 0; i < score->_channels.size(); i++)
					if (score->_channels[i]->_sprite->_castId == d.u.i)
						d.u.i = i;
			}

			channel->_constraint = d.u.i;
			channel->_dirty = true;
		}
		break;
	case kTheCursor:
		if (d.type == INT) {
			channel->_cursor.readFromResource(d.asInt());
		} else if (d.type == ARRAY && d.u.farr->size() == 2) {
			channel->_cursor.readFromCast(d.u.farr->operator[](0).asInt(), d.u.farr->operator[](1).asInt());
		}
		break;
	case kTheEditableText:
		if (sprite->_cast)
			sprite->_cast->setEditable(d.asInt());
		break;
	case kTheForeColor:
		if (d.asInt() != sprite->_foreColor) {
			sprite->_foreColor = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheHeight:
		if (d.asInt() != channel->_height) {
			g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
			channel->setHeight(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kTheImmediate:
		sprite->_immediate = d.asInt();
		break;
	case kTheInk:
		if (d.asInt() != sprite->_ink) {
			sprite->_ink = static_cast<InkType>(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kTheLineSize:
		if (d.asInt() != sprite->_thickness) {
			sprite->_thickness = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheLoc:
		warning("STUB: Lingo::setTheSprite(): Unprocessed setting field \"%s\" of sprite", field2str(field));
		break;
	case kTheLocH:
		if (d.asInt() != channel->_currentPoint.x) {
			g_director->getCurrentMovie()->getWindow()->addDirtyRect(channel->getBbox());
			channel->_currentPoint.x = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheLocV:
		if (d.asInt() != channel->_currentPoint.y) {
			g_director->getCurrentMovie()->getWindow()->addDirtyRect(channel->getBbox());
			channel->_currentPoint.y = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheMoveableSprite:
		sprite->_moveable = d.asInt();
		break;
	case kTheMovieRate:
		channel->_movieRate = d.asInt();
		break;
	case kTheMovieTime:
		channel->_movieTime = d.asInt();
		break;
	case kThePattern:
		if (d.asInt() != sprite->getPattern()){
			sprite->setPattern(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kThePuppet:
		sprite->_puppet = d.asInt();
		if (!d.asInt()) {
			// TODO: Properly reset sprite properties after puppet disabled.
			sprite->_moveable = false;
		}
		break;
	case kTheStartTime:
		channel->_startTime = d.asInt();
		break;
	case kTheStopTime:
		channel->_stopTime = d.asInt();
		break;
	case kTheStretch:
		if (d.asInt() != sprite->_stretch) {
			sprite->_stretch = d.asInt();
			channel->_dirty = true;

			if (sprite->_stretch) {
				g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());

				channel->_width = sprite->_width;
				channel->_height = sprite->_height;
			}
		}
		break;
	case kTheTrails:
		sprite->_trails = d.asInt();
		break;
	case kTheType:
		if (d.asInt() != sprite->_spriteType) {
			sprite->_spriteType = static_cast<SpriteType>(d.asInt());
			channel->_dirty = true;
		}
		break;
	case kTheVisibility:
	case kTheVisible:
		if (d.asInt() != channel->_visible) {
			channel->_visible = d.asInt();
			channel->_dirty = true;
		}
		break;
	case kTheVolume:
		// TODO: Should changing digital video flags mark as dirty?
		sprite->_volume = d.asInt();
		break;
	case kTheWidth:
		if (d.asInt() != channel->_width) {
			g_director->getCurrentWindow()->addDirtyRect(channel->getBbox());
			channel->setWidth(d.asInt());
			channel->_dirty = true;
		}
		break;
	default:
		warning("Lingo::setTheSprite(): Unprocessed setting field \"%s\" of sprite", field2str(field));
	}

	if (channel->_dirty && g_director->getCurrentMovie())
		g_director->getCurrentMovie()->getWindow()->addDirtyRect(channel->getBbox());
}

Datum Lingo::getTheCast(Datum &id1, int field) {
	Datum d;
	int id = g_lingo->castIdFetch(id1);

	Movie *movie = _vm->getCurrentMovie();
	// Setting default type
	d.type = INT;

	if (!movie) {
		warning("Lingo::getTheCast(): No movie loaded");
		return d;
	}

	CastMember *member = _vm->getCurrentMovie()->getCastMember(id);
	if (!member) {
		if (field == kTheLoaded) {
			d.u.i = 0;
		} else {
			warning("Lingo::getTheCast(): CastMember %d not found", id);
		}
		return d;
	}
	Cast *cast = member->getCast();

	CastType castType = member->_type;
	CastMemberInfo *castInfo = cast->getCastMemberInfo(id);
	if (!castInfo)
		warning("Lingo::getTheCast(): CastMember info for %d not found", id);

	switch (field) {
	case kTheBackColor:
		d.u.i = member->getBackColor();
		break;
	case kTheCastType:
		d.u.i = castType;
		break;
	case kTheCenter:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_center ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheController:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_showControls ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheCrop:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_crop ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheDepth:
		warning("STUB: Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheDirectToStage:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_directToStage ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheDuration:
		warning("STUB: Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheFileName:
		if (castInfo)
			d = Datum(castInfo->fileName);
		break;
	case kTheForeColor:
		d.u.i = member->getForeColor();
		break;
	case kTheFrameRate:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_frameRate;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheHeight:
		d.u.i = cast->getCastMemberInitialRect(id).height();
		break;
	case kTheHilite:
		d.u.i = member->_hilite;
		break;
	case kTheLoaded:
		d.u.i = 1; //Not loaded handled above
		break;
	case kTheLoop:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_looping ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheModified:
		warning("STUB: Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheName:
		if (castInfo)
			d = Datum(castInfo->name);
		break;
	case kTheNumber:
		d.u.i = id;
		break;
	case kThePausedAtStart:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_pausedAtStart ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kThePreLoad:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_preload ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheRect:
		warning("STUB: Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheRegPoint:
		warning("STUB: Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kThePalette:
		if (member->_type == kCastBitmap)
			d.u.i = ((BitmapCastMember *)member)->_clut;
		break;
	case kThePicture:
		warning("STUB: Lingo::getTheCast(): Unprocessed getting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kThePurgePriority:
		d.u.i = member->_purgePriority;
		break;
	case kTheScriptText:
		if (castInfo)
			d = Datum(castInfo->script);
		break;
	case kTheSize:
		d.u.i = member->_size;
		break;
	case kTheSound:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_enableSound ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheText:
		{
			Common::String text;
			if (castType == kCastText) {
				if (member && member->_type == kCastText) {
					text = ((TextCastMember *)member)->getText();
				} else {
					warning("Lingo::getTheCast(): Unknown STXT cast id %d", id);
				}
			} else {
				warning("Lingo::getTheCast(): Unprocessed getting text of cast %d type %d", id, castType);
			}
			d = Datum(text);
		}
		break;
	case kTheVideo:
		if (castType == kCastDigitalVideo) {
			d.u.i = ((DigitalVideoCastMember *)member)->_enableVideo ? 1 : 0;
		} else {
			warning("Lingo::getTheCast(): Unsupported getting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheWidth:
		d.u.i = cast->getCastMemberInitialRect(id).width();
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

	if (id1.type == INT) {
		id = id1.u.i;
	} else if (id1.type == STRING) {
		id = g_lingo->castIdFetch(id1);
	} else {
		warning("Lingo::setTheCast(): Unknown the cast id type: %s", id1.type2str());
		return;
	}

	CastMember *member = _vm->getCurrentMovie()->getCastMember(id);
	if (!member) {
		warning("Lingo::setTheCast(): CastMember id %d doesn't exist", id);
		return;
	}
	Cast *cast = member->getCast();
	CastType castType = member->_type;
	CastMemberInfo *castInfo = cast->getCastMemberInfo(id);

	switch (field) {
	case kTheBackColor:
		if (castType == kCastText) {
			int color = _vm->transformColor(d.asInt());
			member->setColors(nullptr, &color);
		}
		break;
	case kTheCastType:
		warning("Lingo::setTheCast(): Attempt to set read-only field %s of cast %d", entity2str(field), id);
		break;
	case kTheCenter:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_center = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheController:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_showControls = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheCrop:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_crop = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheDepth:
		warning("STUB: Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheDirectToStage:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_directToStage = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheDuration:
		warning("STUB: Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheFileName:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		castInfo->fileName = d.asString();
		break;
	case kTheForeColor:
		if (castType == kCastText) {
			int color = _vm->transformColor(d.asInt());
			member->setColors(&color, nullptr);
		}
		break;
	case kTheFrameRate:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_frameRate = d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheHeight:
		warning("Lingo::setTheCast(): Attempt to set read-only field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheHilite:
		// TODO: Understand how texts can be selected programmatically as well.
		if (member->_type == kCastButton) {
			TextCastMember *button = (TextCastMember *)member;
			if ((bool)d.asInt() !=  member->_hilite) {
				button->_hilite = !!d.asInt();
			}
		} else {
			warning("Lingo::setTheCast: Attempted to set hilite of unsupported cast type");
		}
		break;
	case kTheLoop:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_looping = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheName:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		castInfo->name = d.asString();
		break;
	case kThePausedAtStart:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_pausedAtStart = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kThePreLoad:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_preload = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheRect:
		warning("STUB: Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kTheRegPoint:
		warning("STUB: Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kThePalette:
		if (member->_type == kCastBitmap)
			((BitmapCastMember *)member)->_clut = d.asInt();
		break;
	case kThePicture:
		warning("STUB: Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
		break;
	case kThePurgePriority:
		member->_purgePriority = CLIP<int>(d.asInt(), 0, 3);
		break;
	case kTheScriptText:
		if (!castInfo) {
			warning("Lingo::setTheCast(): The cast %d not found. type: %d", id, castType);
			return;
		}
		cast->_lingoArchive->addCode(d.u.s->c_str(), kCastScript, id);
		castInfo->script = d.asString();
		break;
	case kTheSound:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_enableSound = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheText:
		if (castType == kCastText) {
			if (member->_type == kCastText) {
				((TextCastMember *)member)->setText(d.asString().c_str());
			} else {
				warning("Lingo::setTheCast(): Unknown STXT cast id %d", id);
				return;
			}
		} else {
			warning("Lingo::setTheCast(): Unprocessed setting text of cast %d type %d", id, castType);
		}
		break;
	case kTheVideo:
		if (castType == kCastDigitalVideo) {
			((DigitalVideoCastMember *)member)->_enableVideo = (bool)d.asInt();
		} else {
			warning("Lingo::setTheCast(): Unsupported setting kCastDigitalVideo field \"%s\" of cast %d", field2str(field), id);
		}
		break;
	case kTheWidth:
		warning("Lingo::setTheCast(): Attempt to set read-only field \"%s\" of cast %d", field2str(field), id);
		break;
	default:
		warning("Lingo::setTheCast(): Unprocessed setting field \"%s\" of cast %d", field2str(field), id);
	}
}

Datum Lingo::getTheField(Datum &id1, int field) {
	Datum d;
	int id = g_lingo->castIdFetch(id1);

	CastMember *member = _vm->getCurrentMovie()->getCastMember(id);
	if (!member) {
		warning("Lingo::getTheField(): CastMember id %d doesn't exist", id);
		return d;
	} else if (member->_type != kCastText) {
		warning("Lingo::getTheField(): CastMember id %d is not a field", id);
		return d;
	}

	switch (field) {
	case kTheText:
		warning("Lingo::getTheField(): Unprocessed getting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextAlign:
		d.type = STRING;
		switch (((TextCastMember *)member)->_textAlign) {
		case kTextAlignLeft:
			d.u.s = new Common::String("left");
			break;
		case kTextAlignCenter:
			d.u.s = new Common::String("center");
			break;
		case kTextAlignRight:
			d.u.s = new Common::String("right");
			break;
		default:
			warning("Lingo::getTheField: Invalid text align spec");
			break;
		}
		break;
	case kTheTextFont:
		warning("Lingo::getTheField(): Unprocessed getting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextHeight:
		warning("Lingo::getTheField(): Unprocessed getting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextSize:
		warning("Lingo::getTheField(): Unprocessed getting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextStyle:
		warning("Lingo::getTheField(): Unprocessed getting field \"%s\" of field %d", field2str(field), id);
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

	CastMember *member = _vm->getCurrentMovie()->getCastMember(id);
	if (!member) {
		warning("Lingo::setTheField(): CastMember id %d doesn't exist", id);
		return;
	} else if (member->_type != kCastText) {
		warning("Lingo::setTheField(): CastMember id %d is not a field", id);
	}

	switch (field) {
	case kTheText:
		warning("Lingo::setTheField(): Unprocessed setting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextAlign:
		{
			Common::String select = d.asString(true);
			select.toLowercase();

			TextAlignType align;
			if (select == "\"left\"") {
				align = kTextAlignLeft;
			} else if (select == "\"center\"") {
				align = kTextAlignCenter;
			} else if (select == "\"right\"") {
				align = kTextAlignRight;
			} else {
				warning("Lingo::setTheField: Unknown text align spec: %s", d.asString(true).c_str());
				break;
			}

			((TextCastMember *)member)->_textAlign = align;
			member->_modified = true;
			break;
		}
	case kTheTextFont:
		warning("Lingo::setTheField(): Unprocessed setting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextHeight:
		warning("Lingo::setTheField(): Unprocessed setting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextSize:
		warning("Lingo::setTheField(): Unprocessed setting field \"%s\" of field %d", field2str(field), id);
		break;
	case kTheTextStyle:
		warning("Lingo::setTheField(): Unprocessed setting field \"%s\" of field %d", field2str(field), id);
		break;
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
	} else if (obj.type == CASTREF) {
		// WORKAROUND: Until CastMembers are made Lingo objects
		if (propName.equalsIgnoreCase("palette")) {
			d.type = INT;
			CastMember *member = _vm->getCurrentMovie()->getCastMember(obj.u.i);

			if (member && member->_type == kCastBitmap)
				d.u.i = ((BitmapCastMember *)member)->_clut + 1;
		}
	} else {
		warning("Lingo::getObjectProp: Invalid object: %s", obj.asString(true).c_str());
	}
	return d;
}

void Lingo::setObjectProp(Datum &obj, Common::String &propName, Datum &val) {
	if (obj.type == OBJECT) {
		if (obj.u.obj->hasProp(propName)) {
			obj.u.obj->setProp(propName, val);
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
	} if (obj.type == CASTREF) {
		// WORKAROUND: Until CastMembers are made Lingo objects
		if (propName.equalsIgnoreCase("palette")) {
			CastMember *member = _vm->getCurrentMovie()->getCastMember(obj.u.i);

			if (member && member->_type == kCastBitmap)
				((BitmapCastMember *)member)->_clut = val.asInt();
		}
	} else {
		warning("Lingo::setObjectProp: Invalid object: %s", obj.asString(true).c_str());
	}
}

static const char *mfull[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August" "September", "October", "Novemver", "December"
};

static const char *wday[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

Datum Lingo::getTheDate(int field) {
	TimeDate t;
	g_system->getTimeAndDate(t);

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

} // End of namespace Director
