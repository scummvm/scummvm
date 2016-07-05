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

static struct TheEntityProto {
	TheEntity entity;
	const char *name;
	int hasID;
} entities[] = {
	{ kTheFrame,			"frame",			VOID },
	{ kThePathName,			"pathname",			VOID },
	{ kTheMovie,			"movie",			VOID },
	{ kTheMouseH,			"mouseh",			VOID },
	{ kTheMouseV,			"mousev",			VOID },
	{ kTheMouseDownScript,  "mouseDownScript",	VOID },
	{ kTheMouseUpScript,  	"mouseUpScript",	VOID },
	{ kTheClickOn,			"clickOn",			VOID },
	{ kTheDoubleClick,		"doubleClick",		VOID },
	{ kTheLastClick,		"lastClick",		VOID },
	{ kTheLastFrame,		"lastFrame",		VOID },
	{ kTheLastEvent,		"lastEvent",		VOID },
	{ kTheMouseUp,			"mouseUp",			VOID },
	{ kTheMouseDown,		"mouseDown",		VOID },
	{ kTheRightMouseUp,		"rightMouseUp",		VOID },
	{ kTheRightMouseDown,	"rightMouseDown",	VOID },
	{ kTheStillDown,		"stillDown",		VOID },
	{ kTheKey,				"key",				VOID },
	{ kTheControlDown,		"controlDown",		VOID },
	{ kTheCommandDown,		"commandDown",		VOID },
	{ kTheShiftDown,		"shiftDown",		VOID },
	{ kTheOptionDown,		"optionDown",		VOID },
	{ kTheMenu,				"menu",				INT  },
	{ kTheMenus,			"menus",			VOID },
	{ kTheMenuItem,			"menuitem",			STRING  }, // int?
	{ kTheMenuItems,		"menuitems",		VOID  },
	{ kTheSprite,			"sprite",			INT  },
	{ kTheCast,				"cast",				INT  }, // string?
	{ kThePerFrameHook,		"perframehook",		VOID },
	{ kTheTicks,			"ticks",			VOID },
	{ kTheTimer,			"timer",			VOID },
	{ kTheTimeoutLength,	"timeoutlength",	VOID },
	{ kTheWindow,			"window",			VOID },
	{ kTheNOEntity, NULL, false }
};

static struct TheEntityFieldProto {
	TheEntity entity;
	const char *name;
	TheField field;
} fields[] = {
	{ kTheSprite,		"castnum",		kTheCastNum },
	{ kTheSprite,		"cursor",		kTheCursor },
	{ kTheSprite,		"loch",			kTheLocH },
	{ kTheSprite,		"locv",			kTheLocV },
	{ kTheSprite,		"backColor",	kTheBackColor },
	{ kTheSprite,		"blend",		kTheBlend },
	{ kTheSprite,		"bottom",		kTheBottom },
	{ kTheSprite,		"constraint",	kTheConstraint },
	{ kTheSprite,		"editableText", kTheEditableText },
	{ kTheSprite,		"foreColor",	kTheForeColor },
	{ kTheSprite,		"height",		kTheHeight },
	{ kTheSprite,		"ink",			kTheInk },
	{ kTheSprite,		"left",			kTheLeft },
	{ kTheSprite,		"lineSize",		kTheLineSize },
	{ kTheSprite,		"moveable",		kTheMoveable },
	{ kTheSprite,		"movieRate",	kTheMovieRate },
	{ kTheSprite,		"movieTime",	kTheMovieTime },
	{ kTheSprite,		"right",		kTheRight },
	{ kTheSprite,		"stopTime",		kTheStopTime },
	{ kTheSprite,		"stretch",		kTheStrech },
	{ kTheSprite,		"startTime",	kTheStartTime },
	{ kTheSprite,		"scriptNum",	kTheScriptNum },
	{ kTheSprite,		"top",			kTheTop },
	{ kTheSprite,		"trails",		kTheTrails },
	{ kTheSprite,		"type",			kTheType },
	{ kTheSprite,		"visible",		kTheVisible },
	{ kTheSprite,		"volume",		kTheVolume },
	{ kTheSprite,		"width",		kTheWidth },

	//Common cast fields
	{ kTheCast,		"width",		kTheWidth },
	{ kTheCast,		"height",		kTheHeight },
	{ kTheCast,		"filename",		kTheFilename },
	{ kTheCast,		"scriptText",	kTheScriptText },
	{ kTheCast,		"castType",		kTheCastType },
	{ kTheCast,		"name",			kTheName },
	{ kTheCast,		"rect",			kTheRect },
	{ kTheCast,		"number",		kTheNumber },
	{ kTheCast,		"modified",		kTheModified },
	{ kTheCast,		"loaded",		kTheLoaded },
	{ kTheCast,		"purgePriority",kThePurgePriority }, //0 Never purge, 1 Purge Last, 2 Purge next, 2 Purge normal

	//Shape fields
	{ kTheCast,		"backColor",	kTheBackColor },
	{ kTheCast,		"foreColor",	kTheForeColor },

	//Digital video fields
	{ kTheCast,		"controller",	kTheController },
	{ kTheCast,		"directToStage",kTheDirectToStage },
	{ kTheCast,		"frameRate",	kTheFrameRate },
	{ kTheCast,		"loop",			kTheLoop },
	{ kTheCast,		"pausedAtStart",kThePausedAtStart },
	{ kTheCast,		"preload",		kThePreload },
	{ kTheCast,		"sound",		kTheSound }, // 0-1 off-on

	//Bitmap fields
	{ kTheCast,		"depth",		kTheDepth },
	{ kTheCast,		"regPoint",		kTheRegPoint },
	{ kTheCast,		"palette",		kThePalette },
	{ kTheCast,		"picture",		kThePicture },

	//TextCast fields
	{ kTheCast,		"size",			kTheSize },
	{ kTheCast,		"hilite",		kTheHilite },
	{ kTheCast,		"text",			kTheText },

	{ kTheWindow,	"drawRect",		kTheDrawRect },
	{ kTheWindow,	"filename",		kTheFilename },
	{ kTheWindow,	"sourceRect",	kTheSourceRect },
	{ kTheWindow,	"visible",		kTheVisible },

	{ kTheMenuItem,	"checkmark",	kTheCheckMark },
	{ kTheMenuItem, "enabled",		kTheEnabled },
	{ kTheMenuItem, "name",			kTheName },
	{ kTheMenuItem, "name",			kTheScript },

	{ kTheMenu,		"name",			kTheName },

	{ kTheMenuItems,"number",		kTheNumber },
	{ kTheMenus,	"number",		kTheNumber },

	{ kTheNOEntity, NULL, kTheNOField }
};

void Lingo::setTheEntity(TheEntity entity, int id, TheField field, Datum &d) {
	switch (entity) {
	case kTheSprite:
		setTheSprite(id, field, d);
		break;
	case kThePerFrameHook:
		warning("STUB: setting the perframehook");
		break;
	default:
		error("Unprocessed setting field %d of entity %d", field, entity);
	}
}

void Lingo::setTheSprite(int id, TheField field, Datum &d) {
	Sprite *sprite = _vm->_currentScore->getSpriteById(id);

	d.toInt(); // Enforce Integer

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

	default:
		error("Unprocessed setting field %d of sprite", field);
	}
}

Datum Lingo::getTheEntity(TheEntity entity, int id, TheField field) {
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
	default:
		error("Unprocessed getting field %d of entity %d", field, entity);
	}

	return d;
}

Datum Lingo::getTheSprite(int id, TheField field) {
	Datum d;
	Sprite *sprite = _vm->_currentScore->getSpriteById(id);

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
	default:
		error("Unprocessed getting field %d of sprite", field);
	}

	return d;
}

Datum Lingo::getTheCast(int id, TheField field) {
	Datum d;
	d.type = INT;

	Cast *cast;
	if (!_vm->_currentScore->_casts.contains(id)) {

		if (field == kTheLoaded) {
			d.u.i = 0;
		}

		return d;
	} else {
		error ("Not cast %d found", id);
	}
	cast = _vm->_currentScore->_casts[id];

	switch (field) {
	case kTheCastType:
		d.u.i = cast->type;
		break;
	case kTheWidth:
		d.u.i = cast->initialRect.width();
		break;
	case kTheHeight:
		d.u.i = cast->initialRect.height();
		break;
	case kTheBackColor:
		{
			if (cast->type != kCastShape)
				error("Field %d of cast %d not found", field, id);
			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			d.u.i = shape->bgCol;
		}
		break;
	case kTheForeColor:
		{
			if (cast->type != kCastShape)
				error("Field %d of cast %d not found", field, id);
			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			d.u.i = shape->fgCol;
		}
		break;
	case kTheLoaded:
		d.u.i = 1; //Not loaded handled above
		break;
	default:
		error("Unprocessed getting field %d of cast %d", field, id);
	//TODO find out about String fields
	}
}

void Lingo::setTheCast(int id, TheField field, Datum &d) {
	Cast *cast = _vm->_currentScore->_casts[id];
	switch (field) {
	case kTheCastType:
		cast->type = static_cast<CastType>(d.u.i);
		cast->modified = 1;
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
			if (cast->type != kCastShape)
				error("Field %d of cast %d not found", field, id);
			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			shape->bgCol = d.u.i;
			shape->modified = 1;
		}
		break;
	case kTheForeColor:
		{
			if (cast->type != kCastShape)
				error("Field %d of cast %d not found", field, id);
			ShapeCast *shape = static_cast<ShapeCast *>(_vm->_currentScore->_casts[id]);
			shape->fgCol = d.u.i;
			shape->modified = 1;
		}
		break;
	default:
		error("Unprocessed getting field %d of cast %d", field, id);
	}
}

} // End of namespace Director
