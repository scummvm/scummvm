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
	bool hasID;
} entities[] = {
	{ kTheFrame,			"frame",			false },
	{ kThePathName,			"pathname",			false },
	{ kTheMovie,			"movie",			false },
	{ kTheMouseH,			"mouseh",			false },
	{ kTheMouseV,			"mousev",			false },
	{ kTheMouseDownScript,  "mouseDownScript",	false },
	{ kTheMouseUpScript,  	"mouseUpScript",	false },
	{ kTheClickOn,			"clickOn",			false },
	{ kTheDoubleClick,		"doubleClick",		false },
	{ kTheLastClick,		"lastClick",		false },
	{ kTheLastFrame,		"lastFrame",		false },
	{ kTheLastEvent,		"lastEvent",		false },
	{ kTheMouseUp,			"mouseUp",			false },
	{ kTheMouseDown,		"mouseDown",		false },
	{ kTheRightMouseUp,		"rightMouseUp",		false },
	{ kTheRightMouseDown,	"rightMouseDown",	false },
	{ kTheStillDown,		"stillDown",		false },
	{ kTheKey,				"key",				false },
	{ kTheControlDown,		"controlDown",		false },
	{ kTheCommandDown,		"commandDown",		false },
	{ kTheShiftDown,		"shiftDown",		false },
	{ kTheOptionDown,		"optionDown",		false },
	{ kTheMenu,				"menu",				true  },
	{ kTheMenus,			"menus",			false },
	{ kTheMenuItem,			"menuitem",			true  },
	{ kTheMenuItems,		"menuitems",		true  },
	{ kTheSprite,			"sprite",			true  },
	{ kTheCast,				"cast",				true  },
	{ kThePerFrameHook,		"perframehook",		false },
	{ kTheTicks,			"ticks",			false },
	{ kTheTimer,			"timer",			false },
	{ kTheTimeoutLength,	"timeoutlength",	false },
	{ kTheWindow,			"window",			false },
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

	{ kTheCast,		"backColor",	kTheBackColor },
	{ kTheCast,		"castType",		kTheCastType },
	{ kTheCast,		"controller",	kTheController },
	{ kTheCast,		"depth",		kTheDepth },
	{ kTheCast,		"directToStage",kTheDirectToStage },
	{ kTheCast,		"filename",		kTheFilename },
	{ kTheCast,		"foreColor",	kTheForeColor },
	{ kTheCast,		"frameRate",	kTheFrameRate },
	{ kTheCast,		"hilite",		kTheHilite },
	{ kTheCast,		"height",		kTheHeight },
	{ kTheCast,		"loop",			kTheLoop },
	{ kTheCast,		"loaded",		kTheLoaded },
	{ kTheCast,		"modified",		kTheModified },
	{ kTheCast,		"number",		kTheNumber },
	{ kTheCast,		"name",			kTheName },
	{ kTheCast,		"palette",		kThePalette },
	{ kTheCast,		"pausedAtStart",kThePausedAtStart },
	{ kTheCast,		"picture",		kThePicture },
	{ kTheCast,		"preload",		kThePreload },
	{ kTheCast,		"purgePriority",kThePurgePriority },
	{ kTheCast,		"rect",			kTheRect },
	{ kTheCast,		"regPoint",		kTheRegPoint },
	{ kTheCast,		"scriptText",	kTheScriptText },
	{ kTheCast,		"size",			kTheSize },
	{ kTheCast,		"sound",		kTheSound },
	{ kTheCast,		"text",			kTheText },
	{ kTheCast,		"width",		kTheWidth },

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
	case kTheInk:
		sprite->_ink = static_cast<InkType>(d.u.i);
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


} // End of namespace Director
