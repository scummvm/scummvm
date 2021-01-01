/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/foreach.h"
#include "common/savefile.h"
#include "common/system.h"

#include "engines/grim/grim.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/localize.h"
#include "engines/grim/actor.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/inputdialog.h"
#include "engines/grim/textobject.h"

#include "engines/grim/lua/lauxlib.h"

namespace Grim {

/* Destroy a text object since we don't need it anymore
 * note that the menu creates more objects than it needs,
 * so it deletes some objects right after creating them
 */
void Lua_V1::KillTextObject() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		delete gettextobject(textObj);
	}
}

/* Make changes to a text object based on the parameters passed
 * in the table in the LUA parameter 2.
 */
void Lua_V1::ChangeTextObject() {
	lua_Object textObj = lua_getparam(1);
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		int paramId = 2;
		for (;;) {
			lua_Object paramObj = lua_getparam(paramId++);
			if (!paramObj)
				break;

			// If the text object is invalid (deleted) ignore the request
			// We do the check here so that all of the params are used
			if (!textObject)
				continue;

			if (!lua_isstring(paramObj)) {
				if (!lua_istable(paramObj))
					break;
				setTextObjectParams(textObject, paramObj);
				textObject->destroy();
			} else {
				const char *line = lua_getstring(paramObj);
				textObject->setText(line, false);
				lua_getstring(paramObj);

			}

			lua_pushnumber(textObject->getBitmapWidth());
			lua_pushnumber(textObject->getBitmapHeight());
		}
	}
}

/* Return the "text speed", this option must be handled
 * to prevent errors in the "Options" menu even though
 * we're not currently using the value
 */
void Lua_V1::GetTextSpeed() {
	lua_pushnumber(g_grim->getTextSpeed());
}

void Lua_V1::SetTextSpeed() {
	lua_Object speedObj = lua_getparam(1);
	if (!lua_isnumber(speedObj))
		return;

	int speed = (int)lua_getnumber(speedObj);
	g_grim->setTextSpeed(speed);
}

void Lua_V1::MakeTextObject() {
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj)) {
		return;
	}

	TextObject *textObject = new TextObject();
	const char *line = lua_getstring(textObj);

	textObject->setDefaults(&g_grim->_blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText(line, false);

	lua_pushusertag(textObject->getId(), MKTAG('T', 'E', 'X', 'T'));
	if (!(g_grim->getGameFlags() & ADGF_DEMO)) {
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

void Lua_V1::GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

void Lua_V1::ExpireText() {
	// Cleanup actor references to deleted text objects
	foreach (Actor *a, Actor::getPool()) {
		a->lineCleanup();
	}
}

void Lua_V1::GetTextCharPosition() {
	lua_Object textObj = lua_getparam(1);
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		int pos = (int)lua_getnumber(lua_getparam(2));
		lua_pushnumber(textObject->getTextCharPosition(pos));
	}
}

void Lua_V1::BlastText() {
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj)) {
		return;
	}

	const char *line = lua_getstring(textObj);
	if (!line || line[0] == 0)
		return;

	TextObject *textObject = new TextObject();
	textObject->setBlastDraw();
	textObject->setDefaults(&g_grim->_blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText(line, false);
	textObject->draw();
	delete textObject;
}

void Lua_V1::SetOffscreenTextPos() {
	// called with (0,0) on dialog entry, (nil, nil) on dialog exit
	warning("Lua_V1::SetOffscreenTextPos: implement opcode");
}

void Lua_V1::TextFileGetLine() {
	char textBuf[1000];
	lua_Object nameObj = lua_getparam(1);
	lua_Object posObj = lua_getparam(2);
	Common::SeekableReadStream *file;

	if (lua_isnil(nameObj) || lua_isnil(posObj)) {
		lua_pushnil();
		return;
	}

	const char *filename = lua_getstring(nameObj);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	file = saveFileMan->openForLoading(filename);
	if (!file) {
		lua_pushnil();
		return;
	}

	int pos = (int)lua_getnumber(posObj);
	file->seek(pos, SEEK_SET);
	memset(textBuf, 0, 1000);
	file->readLine(textBuf, 1000);
	delete file;

	lua_pushstring(textBuf);
}

void Lua_V1::TextFileGetLineCount() {
	char textBuf[1000];
	lua_Object nameObj = lua_getparam(1);

	if (lua_isnil(nameObj)) {
		lua_pushnil();
		return;
	}

	const char *filename = luaL_check_string(1);
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::SeekableReadStream *file = saveFileMan->openForLoading(filename);
	if (!file) {
		lua_pushnil();
		return;
	}

	lua_Object result = lua_createtable();

	int line = 0;
	for (;;) {
		if (file->eos())
			break;
		lua_pushobject(result);
		lua_pushnumber(line);
		int pos = file->pos();
		lua_pushnumber(pos);
		lua_settable();
		file->readLine(textBuf, 1000);
		line++;
	}
	delete file;

	lua_pushobject(result);
	lua_pushstring("count");
	lua_pushnumber(line);
	lua_settable();
	lua_pushobject(result);
}

// Localization function

void Lua_V1::LocalizeString() {
	char msgId[50], buf[1000];
	lua_Object strObj = lua_getparam(1);

	if (lua_isstring(strObj)) {
		const char *str = lua_getstring(strObj);
		// If the string that we're passed isn't localized yet then
		// construct the localized string, otherwise spit back what
		// we've been given
		if (str[0] == '/') {
			Common::String msg = parseMsgText(str, msgId);
			sprintf(buf, "/%s/%s", msgId, msg.c_str());
			str = buf;
		}
		lua_pushstring(str);
	}
}

void Lua_V1::SetSayLineDefaults() {
	lua_Object tableObj = lua_getparam(1);
	if (tableObj && lua_istable(tableObj))
		setTextObjectParams(&g_grim->_sayLineDefaults, tableObj);
}

void Lua_V1::SayLine() {
	int vol = 127, buffer = 64, paramId = 1;
	float x = -1, y = -1;
	bool background = true;
	const char *msgId = nullptr;
	Common::String msg;
	lua_Object paramObj = lua_getparam(paramId++);

	if ((lua_isuserdata(paramObj) && lua_tag(paramObj) == MKTAG('A','C','T','R'))
			|| lua_isstring(paramObj) || lua_istable(paramObj)) {
		Actor *actor = nullptr;//some_Actor, maybe some current actor
		if (lua_isuserdata(paramObj) && lua_tag(paramObj) == MKTAG('A','C','T','R')) {
			actor = getactor(paramObj);
			paramObj = lua_getparam(paramId++);
		}
		if (actor) {
			while (!lua_isnil(paramObj)) {
				if (!lua_isstring(paramObj) && !lua_isnumber(paramObj) && !lua_istable(paramObj))
					break;
				if (lua_istable(paramObj))
					parseSayLineTable(paramObj, &background, &vol, &buffer, &x, &y);
				else {
					if (lua_isnumber(paramObj))
						background = false;
					else {
						msgId = lua_getstring(paramObj);
					}
				}
				paramObj = lua_getparam(paramId++);
			}

			actor->sayLine(msgId, background, x, y); //background, vol, pan, x, y
		}
	}
}

void Lua_V1::ShutUpActor() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	if (actor)
		actor->shutUp();
}

void Lua_V1::PrintLine() {
	int vol = 127, buffer = 64; /*paramId = 1, */
	float x = -1, y = -1;
	bool background = true;
	char msgId[50];
	Common::String msg;
	lua_Object param1Obj = lua_getparam(0);
	lua_Object param2Obj = lua_getparam(1);

	if ((lua_isstring(param1Obj) || lua_isnil(param1Obj)) && (lua_istable(param2Obj) || lua_isnil(param2Obj))) {
		if (lua_istable(param2Obj)) {
			setTextObjectParams(&g_grim->_printLineDefaults, param2Obj);
			parseSayLineTable(param2Obj, &background, &vol, &buffer, &x, &y);
		}
		if (lua_isstring(param1Obj)) {
			const char *tmpstr = lua_getstring(param1Obj);
			msg = parseMsgText(tmpstr, msgId);
		}
		if (!msg.empty()) {
//			actor->sayLine(msg.c_str(), msgId); //background, vol, pan, x, y
		}
	}
}

void Lua_V1::InputDialog() {
	lua_Object titleObj = lua_getparam(1);
	lua_Object messageObj = lua_getparam(2);
	lua_Object defaultObj = lua_getparam(3);

	if (!lua_isstring(titleObj) || !lua_isstring(messageObj)) {
		lua_pushnil();
		return;
	}

	Common::String str = lua_getstring(titleObj);
	str += ": ";
	str += lua_getstring(messageObj);
	Grim::InputDialog d(str, lua_getstring(defaultObj));
	int res = d.runModal();
	// The KeyUp event for CTRL has been eat by the gui loop, so we
	// need to reset it manually.
	g_grim->clearEventQueue();
	if (res) {
		lua_pushstring(d.getString().c_str());
	} else {
		lua_pushnil();
	}
}

void Lua_V1::IsMessageGoing() {
	lua_Object actorObj = lua_getparam(1);

	if (!actorObj || (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) || lua_isnil(actorObj)) {
		if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) {
			Actor *actor = getactor(actorObj);
			if (actor) {
				pushbool(actor->isTalking());
			}
		} else {
			// NOTE: this part is quite delicate. If you do changes test the crying angelitos in set fo.
			pushbool(g_grim->areActorsTalking());
		}
	} else
		lua_pushnil();
}

} // end of namespace Grim
