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

#include "director/lingo/lingo.h"
#include "director/cast.h"
#include "director/sprite.h"

namespace Director {

class Sprite;

TheEntity entities[] = {
	{ kTheBeepOn,			"beepOn",			false },	// D2 property
	{ kTheButtonStyle,		"buttonStyle",		false },	// D2 p
	{ kTheCast,				"cast",				true  },
	{ kTheCenterStage,		"centerStage",		false },	// D2 p
	{ kTheCheckBoxAccess,	"checkBoxAccess",	false },	// D2 p
	{ kTheCheckBoxType,		"checkBoxType",		false },	// D2 p
	{ kTheClickOn,			"clickOn",			false },	// D2 function
	{ kTheColorDepth,		"colorDepth",		false },	// D2 p
	{ kTheColorQD,			"colorQD",			false },	// D2 f
	{ kTheCommandDown,		"commandDown",		false },	// D2 f
	{ kTheControlDown,		"controlDown",		false },	// D2 f
	{ kTheDoubleClick,		"doubleClick",		false },	// D2 f
	{ kTheExitLock,			"exitLock",			false },	// D2 p
	{ kTheFixStageSize,		"fixStageSize",		false },	// D2 p
	{ kTheFloatPrecision,	"floatPrecision",	false },
	{ kTheFrame,			"frame",			false },	// D2 f
	{ kTheFreeBlock,		"freeBlock",		false },	// D2 f
	{ kTheFreeBytes,		"freeBytes",		false },	// D2 f
	{ kTheFullColorPermit,	"fullColorPermit",	false },	// D2 p
	{ kTheImageDirect,		"imageDirect",		false },	// D2 p
	{ kTheItemDelimiter,	"itemDelimiter",	false },
	{ kTheKey,				"key",				false },	// D2 f
	{ kTheKeyCode,			"keyCode",			false },	// D2 f
	{ kTheKeyDownScript,	"keyDownScript",	false },	// D2 p
	{ kTheLastClick,		"lastClick",		false },	// D2 f
	{ kTheLastEvent,		"lastEvent",		false },	// D2 f
	{ kTheLastFrame,		"lastFrame",		false },
	{ kTheLastKey,			"lastKey",			false },	// D2 f
	{ kTheLastRoll,			"lastRoll",			false },	// D2 f
	{ kTheMachineType,		"machineType",		false },	// D2 f
	{ kTheMemorySize,		"memorySize",		false },	// D2 f
	{ kTheMenu,				"menu",				true  },
	{ kTheMenus,			"menus",			false },
	{ kTheMenuItem,			"menuitem",			true  },
	{ kTheMenuItems,		"menuitems",		false },
	{ kTheMouseDown,		"mouseDown",		false },	// D2 f
	{ kTheMouseDownScript,  "mouseDownScript",	false },	// D2 p
	{ kTheMouseH,			"mouseH",			false },	// D2 f
	{ kTheMouseUp,			"mouseUp",			false },	// D2 f
	{ kTheMouseUpScript,  	"mouseUpScript",	false },	// D2 p
	{ kTheMouseV,			"mouseV",			false },	// D2 f
	{ kTheMovie,			"movie",			false },	// D2 f
	{ kTheMultiSound,		"multiSound",		false },
	{ kTheOptionDown,		"optionDown",		false },	// D2 f
	{ kThePathName,			"pathName",			false },	// D2 f
	{ kThePauseState,		"pauseState",		false },	// D2 f
	{ kThePerFrameHook,		"perFrameHook",		false },	// D2 p
	{ kThePreloadEventAbort,"preloadEventAbort",false },
	{ kTheResult,			"result",			false },	// D2 f
	{ kTheRightMouseDown,	"rightMouseDown",	false },
	{ kTheRightMouseUp,		"rightMouseUp",		false },
	{ kTheRomanLingo,		"romanLingo",		false },
	{ kTheSelection,		"selection",		false },	// D2 f
	{ kTheShiftDown,		"shiftDown",		false },	// D2 f
	{ kTheSoundEnabled,		"soundEnabled",		false },	// D2 p
	{ kTheSoundLevel,		"soundLevel",		false },	// D2 p
	{ kTheSprite,			"sprite",			true  },
	{ kTheSqrt,				"sqrt",				false },	// D2 f
	{ kTheStage,			"stage",			false },
	{ kTheStageBottom,		"stageBottom",		false },	// D2 f
	{ kTheStageLeft,		"stageLeft",		false },	// D2 f
	{ kTheStageRight,		"stageRight",		false },	// D2 f
	{ kTheStageTop,			"stageTop",			false },	// D2 f
	{ kTheStillDown,		"stillDown",		false },	// D2 f
	{ kTheSwitchColorDepth,	"switchColorDepth",	false },	// D2 p
	{ kTheTicks,			"ticks",			false },	// D2 f
	{ kTheTimeoutKeydown,	"timeoutKeydown",	false },	// D2 p
	{ kTheTimeoutLapsed,	"timeoutLapsed",	false },	// D2 p
	{ kTheTimeoutLength,	"timeoutLength",	false },	// D2 p
	{ kTheTimeoutMouse,		"timeoutMouse",		false },	// D2 p
	{ kTheTimeoutPlay,		"timeoutPlay",		false },	// D2 p
	{ kTheTimeoutScript,	"timeoutScript",	false },	// D2 p
	{ kTheTimer,			"timer",			false },	// D2 p
	{ kTheWindow,			"window",			false },
	{ kTheNOEntity, NULL, false }
};

TheEntityField fields[] = {
	{ kTheSprite,	"backColor",	kTheBackColor },	// D2 p
	{ kTheSprite,	"blend",		kTheBlend },
	{ kTheSprite,	"bottom",		kTheBottom },		// D2 p
	{ kTheSprite,	"castNum",		kTheCastNum },		// D2 p
	{ kTheSprite,	"constraint",	kTheConstraint },	// D2 p
	{ kTheSprite,	"cursor",		kTheCursor },		// D2 p
	{ kTheSprite,	"editableText", kTheEditableText },
	{ kTheSprite,	"foreColor",	kTheForeColor },	// D2 p
	{ kTheSprite,	"height",		kTheHeight },		// D2 p
	{ kTheSprite,	"immediate",	kTheImmediate },	// D2 p
	{ kTheSprite,	"ink",			kTheInk },			// D2 p
	{ kTheSprite,	"left",			kTheLeft },			// D2 p
	{ kTheSprite,	"lineSize",		kTheLineSize },		// D2 p
	{ kTheSprite,	"locH",			kTheLocH },			// D2 p
	{ kTheSprite,	"locV",			kTheLocV },			// D2 p
	{ kTheSprite,	"moveable",		kTheMoveable },
	{ kTheSprite,	"movieRate",	kTheMovieRate },
	{ kTheSprite,	"movieTime",	kTheMovieTime },
	{ kTheSprite,	"pattern",		kThePattern },		// D2 p
	{ kTheSprite,	"puppet",		kThePuppet },		// D2 p
	{ kTheSprite,	"right",		kTheRight },		// D2 p
	{ kTheSprite,	"scriptNum",	kTheScriptNum },
	{ kTheSprite,	"startTime",	kTheStartTime },
	{ kTheSprite,	"stretch",		kTheStrech },		// D2 p
	{ kTheSprite,	"stopTime",		kTheStopTime },
	{ kTheSprite,	"top",			kTheTop },			// D2 p
	{ kTheSprite,	"trails",		kTheTrails },
	{ kTheSprite,	"type",			kTheType },			// D2 p
	{ kTheSprite,	"visible",		kTheVisible },
	{ kTheSprite,	"volume",		kTheVolume },
	{ kTheSprite,	"width",		kTheWidth },		// D2 p

	// Common cast fields
	{ kTheCast,		"castType",		kTheCastType },
	{ kTheCast,		"filename",		kTheFilename },
	{ kTheCast,		"height",		kTheHeight },
	{ kTheCast,		"loaded",		kTheLoaded },
	{ kTheCast,		"modified",		kTheModified },
	{ kTheCast,		"name",			kTheName },
	{ kTheCast,		"number",		kTheNumber },
	{ kTheCast,		"rect",			kTheRect },
	{ kTheCast,		"purgePriority",kThePurgePriority }, // 0 Never purge, 1 Purge Last, 2 Purge next, 2 Purge normal
	{ kTheCast,		"scriptText",	kTheScriptText },
	{ kTheCast,		"width",		kTheWidth },

	// Shape fields
	{ kTheCast,		"backColor",	kTheBackColor },
	{ kTheCast,		"foreColor",	kTheForeColor },

	// Digital video fields
	{ kTheCast,		"controller",	kTheController },
	{ kTheCast,		"directToStage",kTheDirectToStage },
	{ kTheCast,		"frameRate",	kTheFrameRate },
	{ kTheCast,		"loop",			kTheLoop },
	{ kTheCast,		"pausedAtStart",kThePausedAtStart },
	{ kTheCast,		"preload",		kThePreload },
	{ kTheCast,		"sound",		kTheSound }, // 0-1 off-on

	// Bitmap fields
	{ kTheCast,		"depth",		kTheDepth },
	{ kTheCast,		"regPoint",		kTheRegPoint },
	{ kTheCast,		"palette",		kThePalette },
	{ kTheCast,		"picture",		kThePicture },

	// TextCast fields
	{ kTheCast,		"hilite",		kTheHilite },		// D2 p
	{ kTheCast,		"selEnd",		kTheSelEnd },		// D2 p
	{ kTheCast,		"selStart",		kTheSelStart },		// D2 p
	{ kTheCast,		"size",			kTheSize },
	{ kTheCast,		"text",			kTheText },			// D2 p

	{ kTheWindow,	"drawRect",		kTheDrawRect },
	{ kTheWindow,	"filename",		kTheFilename },
	{ kTheWindow,	"sourceRect",	kTheSourceRect },
	{ kTheWindow,	"visible",		kTheVisible },

	{ kTheMenuItem,	"checkmark",	kTheCheckMark },
	{ kTheMenuItem, "enabled",		kTheEnabled },
	{ kTheMenuItem, "name",			kTheName },
	{ kTheMenuItem, "script",		kTheScript },

	{ kTheMenu,		"name",			kTheName },

	{ kTheMenuItems,"number",		kTheNumber },
	{ kTheMenus,	"number",		kTheNumber },

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
		warning("STUB: setting the perframehook");
		break;
	case kTheFloatPrecision:
		_floatPrecision = d.toInt();
		_floatPrecision = MAX(0, MIN(_floatPrecision, 19)); // 0 to 19
		_floatPrecisionFormat = Common::String::format("%%.%df", _floatPrecision);
		warning("set to %d: %s", _floatPrecision, _floatPrecisionFormat.c_str());
		break;
	case kTheColorDepth:
		_vm->_colorDepth = d.toInt();

		// bpp. 1, 2, 4, 8, 32
		warning("STUB: Set color depth to %d", _vm->_colorDepth);
		break;
	default:
		warning("Unprocessed setting field %d of entity %d", field, entity);
	}
}

void Lingo::setTheSprite(Datum &id1, int field, Datum &d) {
	int id = 0;

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Unknown the sprite id type: %s", id1.type2str());
		return;
	}

	d.toInt(); // Enforce Integer

	if (!_vm->_currentScore) {
		warning("The sprite %d field %d setting over non-active score", id, field);
		return;
	}

	Sprite *sprite = _vm->_currentScore->getSpriteById(id);

	if (!sprite)
		return;

	switch (field) {
	case kTheCastNum:
		if (_vm->_currentScore->_casts.contains(d.u.i)) {
			sprite->_cast = _vm->_currentScore->_casts[d.u.i];
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
	case kTheMoveable:
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
	default:
		warning("Unprocessed setting field %d of sprite", field);
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
		warning("STUB: getting the perframehook");
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
	default:
		warning("Unprocessed getting field %d of entity %d", field, entity);
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
		warning("Unknown the sprite id type: %s", id1.type2str());
		return d;
	}

	if (!_vm->_currentScore) {
		warning("The sprite %d field %d setting over non-active score", id, field);
		return d;
	}

	Sprite *sprite = _vm->_currentScore->getSpriteById(id);

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
	case kTheMoveable:
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
	default:
		warning("Unprocessed getting field %d of sprite", field);
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
		warning("Unknown the cast id type: %s", id1.type2str());
		return d;
	}

	if (!_vm->_currentScore) {
		warning("The cast %d field %d setting over non-active score", id, field);
		return d;
	}

	Cast *cast;
	CastInfo *castInfo;
	if (!_vm->_currentScore->_casts.contains(id)) {
		if (field == kTheLoaded) {
			d.type = INT;
			d.u.i = 0;
		}

		return d;
	} else {
		warning("The cast %d found", id);
	}

	cast = _vm->_currentScore->_casts[id];
	castInfo = _vm->_currentScore->_castsInfo[id];

	d.type = INT;

	switch (field) {
	case kTheCastType:
		d.u.i = cast->type;
		break;
	case kTheFilename:
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
		d.u.i = cast->initialRect.width();
		break;
	case kTheHeight:
		d.u.i = cast->initialRect.height();
		break;
	case kTheBackColor:
		{
			if (cast->type != kCastShape) {
				warning("Field %d of cast %d not found", field, id);
				d.type = VOID;
				return d;
			}

			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			d.u.i = shape->bgCol;
		}
		break;
	case kTheForeColor:
		{
			if (cast->type != kCastShape) {
				warning("Field %d of cast %d not found", field, id);
				d.type = VOID;
				return d;
			}

			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			d.u.i = shape->fgCol;
		}
		break;
	case kTheLoaded:
		d.u.i = 1; //Not loaded handled above
		break;
	default:
		warning("Unprocessed getting field %d of cast %d", field, id);
		d.type = VOID;
	//TODO find out about String fields
	}

	return d;
}

void Lingo::setTheCast(Datum &id1, int field, Datum &d) {
	int id = 0;

	if (id1.type == INT) {
		id = id1.u.i;
	} else {
		warning("Unknown the cast id type: %s", id1.type2str());
		return;
	}

	if (!_vm->_currentScore) {
		warning("The cast %d field %d setting over non-active score", id, field);
		return;
	}

	Cast *cast = _vm->_currentScore->_casts[id];
	CastInfo *castInfo = _vm->_currentScore->_castsInfo[id];

	if (!cast) {
		warning("The cast %d found", id);
		return;
	}

	switch (field) {
	case kTheCastType:
		cast->type = static_cast<CastType>(d.u.i);
		cast->modified = 1;
		break;
	case kTheFilename:
		castInfo->fileName = *d.u.s;
		break;
	case kTheName:
		castInfo->name = *d.u.s;
		break;
	case kTheScriptText:
		castInfo->script = *d.u.s;
		break;
	case kTheWidth:
		cast->initialRect.setWidth(d.u.i);
		cast->modified = 1;
		break;
	case kTheHeight:
		cast->initialRect.setHeight(d.u.i);
		cast->modified = 1;
		break;
	case kTheBackColor:
		{
			if (cast->type != kCastShape) {
				warning("Field %d of cast %d not found", field, id);
			}
			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			shape->bgCol = d.u.i;
			shape->modified = 1;
		}
		break;
	case kTheForeColor:
		{
			if (cast->type != kCastShape) {
				warning("Field %d of cast %d not found", field, id);
				return;
			}
			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			shape->fgCol = d.u.i;
			shape->modified = 1;
		}
		break;
	default:
		warning("Unprocessed getting field %d of cast %d", field, id);
	}
}

} // End of namespace Director
