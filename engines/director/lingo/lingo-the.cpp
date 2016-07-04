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

static struct TheEntityProto {
	TheEntity entity;
	const char *name;
	bool hasID;
} entities[] = {
	{ kTheFrame,		"frame",		false },
	{ kThePathName,		"pathname",		false },
	{ kTheMovie,		"movie",		false },
	{ kTheMouseH,		"mouseh",		false },
	{ kTheMouseV,		"mousev",		false },
	{ kTheSprite,		"sprite",		true  },
	{ kThePerFrameHook,	"perframehook",	false },
	{ kTheTicks,		"ticks",		false },
	{ kTheTimer,		"timer",		false },
	{ kTheTimeoutLength,"timeoutlength",false },

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
	switch (field) {
	case kTheCastNum:
		warning("STUB: setting thecastnum of sprite %d", id);
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

	switch (field) {
	case kTheCastNum:
		warning("STUB: getting thecastnum of sprite %d", id);
		break;
	default:
		error("Unprocessed getting field %d of sprite", field);
	}

	return d;
}

} // End of namespace Director
