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

#include "engines/director/lingo/lingo.h"

namespace Director {

class Sprite;

TheEntity entities[] = {
	{ kTheCast,				"cast",				true  },
	{ kTheClickOn,			"clickOn",			false },
	{ kTheColorDepth,		"colorDepth",		false },
	{ kTheColorQD,			"colorQD",			false },
	{ kTheCommandDown,		"commandDown",		false },
	{ kTheControlDown,		"controlDown",		false },
	{ kTheDoubleClick,		"doubleClick",		false },
	{ kTheExitLock,			"exitlock",			false },
	{ kTheFloatPrecision,	"floatPrecision",	false },
	{ kTheFrame,			"frame",			false },
	{ kTheFreeBlock,		"freeBlock",		false },
	{ kTheFreeBytes,		"freeBytes",		false },
	{ kTheItemDelimiter,	"itemDelimiter",	false },
	{ kTheKey,				"key",				false },
	{ kTheKeyCode,			"keyCode",			false },
	{ kTheLastClick,		"lastClick",		false },
	{ kTheLastEvent,		"lastEvent",		false },
	{ kTheLastFrame,		"lastFrame",		false },
	{ kTheMenu,				"menu",				true  },
	{ kTheMenus,			"menus",			false },
	{ kTheMenuItem,			"menuitem",			true  },
	{ kTheMenuItems,		"menuitems",		false },
	{ kTheMouseDown,		"mouseDown",		false },
	{ kTheMouseDownScript,  "mouseDownScript",	false },
	{ kTheMouseH,			"mouseh",			false },
	{ kTheMouseUp,			"mouseUp",			false },
	{ kTheMouseUpScript,  	"mouseUpScript",	false },
	{ kTheMouseV,			"mousev",			false },
	{ kTheMovie,			"movie",			false },
	{ kTheMultiSound,		"multiSound",		false },
	{ kTheOptionDown,		"optionDown",		false },
	{ kThePathName,			"pathname",			false },
	{ kThePerFrameHook,		"perframehook",		false },
	{ kThePreloadEventAbort,"preloadEventAbort",false },
	{ kTheRightMouseDown,	"rightMouseDown",	false },
	{ kTheRightMouseUp,		"rightMouseUp",		false },
	{ kTheRomanLingo,		"romanLingo",		false },
	{ kTheShiftDown,		"shiftDown",		false },
	{ kTheSprite,			"sprite",			true  },
	{ kTheStage,			"stage",			false },
	{ kTheStillDown,		"stillDown",		false },
	{ kTheTicks,			"ticks",			false },
	{ kTheTimeoutLength,	"timeoutlength",	false },
	{ kTheTimer,			"timer",			false },
	{ kTheWindow,			"window",			false },
	{ kTheNOEntity, NULL, false }
};

TheEntityField fields[] = {
	{ kTheSprite,	"backColor",	kTheBackColor },
	{ kTheSprite,	"blend",		kTheBlend },
	{ kTheSprite,	"bottom",		kTheBottom },
	{ kTheSprite,	"castnum",		kTheCastNum },
	{ kTheSprite,	"constraint",	kTheConstraint },
	{ kTheSprite,	"cursor",		kTheCursor },
	{ kTheSprite,	"editableText", kTheEditableText },
	{ kTheSprite,	"foreColor",	kTheForeColor },
	{ kTheSprite,	"height",		kTheHeight },
	{ kTheSprite,	"ink",			kTheInk },
	{ kTheSprite,	"left",			kTheLeft },
	{ kTheSprite,	"lineSize",		kTheLineSize },
	{ kTheSprite,	"loch",			kTheLocH },
	{ kTheSprite,	"locv",			kTheLocV },
	{ kTheSprite,	"moveable",		kTheMoveable },
	{ kTheSprite,	"movieRate",	kTheMovieRate },
	{ kTheSprite,	"movieTime",	kTheMovieTime },
	{ kTheSprite,	"right",		kTheRight },
	{ kTheSprite,	"scriptNum",	kTheScriptNum },
	{ kTheSprite,	"startTime",	kTheStartTime },
	{ kTheSprite,	"stretch",		kTheStrech },
	{ kTheSprite,	"stopTime",		kTheStopTime },
	{ kTheSprite,	"top",			kTheTop },
	{ kTheSprite,	"trails",		kTheTrails },
	{ kTheSprite,	"type",			kTheType },
	{ kTheSprite,	"visible",		kTheVisible },
	{ kTheSprite,	"volume",		kTheVolume },
	{ kTheSprite,	"width",		kTheWidth },

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
	{ kTheCast,		"hilite",		kTheHilite },
	{ kTheCast,		"size",			kTheSize },
	{ kTheCast,		"text",			kTheText },

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
	case kThePerFrameHook:
		warning("STUB: getting the perframehook");
		break;
	case kTheFloatPrecision:
		d.type = INT;
		d.u.i = _floatPrecision;
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
		return d;
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
