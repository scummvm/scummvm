/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_fgetc

#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/localize.h"
#include "engines/grim/actor.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/savegame.h"
#include "engines/grim/colormap.h"

#include "engines/grim/imuse/imuse.h"

#include "engines/grim/lua/lauxlib.h"

namespace Grim {

/* Destroy a text object since we don't need it anymore
 * note that the menu creates more objects than it needs,
 * so it deletes some objects right after creating them
 */
void L1_KillTextObject() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		textObject->setDisabled(true);
	}
}

/* Make changes to a text object based on the parameters passed
 * in the table in the LUA parameter 2.
 */
void L1_ChangeTextObject() {
	const char *line;
	lua_Object textObj = lua_getparam(1);
	int paramId = 2;
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		do {
			lua_Object paramObj = lua_getparam(paramId++);
			if (!paramObj)
				break;
			if (!lua_isstring(paramObj)) {
				if (!lua_istable(paramObj))
					break;
				setTextObjectParams(textObject, paramObj);
			} else {
				line = lua_getstring(paramObj);
				textObject->setText(line);
				lua_getstring(paramObj);

			}
			textObject->destroyBitmap();
			textObject->createBitmap();

			lua_pushnumber(textObject->getBitmapWidth());
			lua_pushnumber(textObject->getBitmapHeight());
		} while (false);
	}
}

/* Return the "text speed", this option must be handled
 * to prevent errors in the "Options" menu even though
 * we're not currently using the value
 */
void L1_GetTextSpeed() {
	lua_pushnumber(g_grim->getTextSpeed());
}

void L1_SetTextSpeed() {
	lua_Object speedObj = lua_getparam(1);
	if (!lua_isnumber(speedObj))
		return;

	int speed = (int)lua_getnumber(speedObj);
	g_grim->setTextSpeed(speed);
}

void L1_MakeTextObject() {
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj)) {
		return;
	}

	TextObject *textObject = new TextObject(false);
	const char *line = lua_getstring(textObj);
	Common::String text = line;

	textObject->setDefaults(&g_grim->_blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText(text.c_str());
	if (!(g_grim->getGameFlags() & ADGF_DEMO))
		textObject->createBitmap();
	g_grim->registerTextObject(textObject);

	lua_pushusertag(textObject->getId(), MKTAG('T', 'E', 'X', 'T'));
	if (!(g_grim->getGameFlags() & ADGF_DEMO)) {
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

void L1_GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		lua_pushnumber(textObject->getBitmapWidth());
		lua_pushnumber(textObject->getBitmapHeight());
	}
}

void L1_ExpireText() {
	// Expire all the text objects
	for (GrimEngine::TextListType::const_iterator i = g_grim->textsBegin(); i != g_grim->textsEnd(); ++i)
		i->_value->setDisabled(true);

	// Cleanup actor references to deleted text objects
	for (GrimEngine::ActorListType::const_iterator i = g_grim->actorsBegin(); i != g_grim->actorsEnd(); ++i)
		i->_value->lineCleanup();
}

void L1_GetTextCharPosition() {
	lua_Object textObj = lua_getparam(1);
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		int pos = (int)lua_getnumber(lua_getparam(2));
		lua_pushnumber(textObject->getTextCharPosition(pos));
	}
}

void L1_BlastText() {
	lua_Object textObj = lua_getparam(1);
	if (!lua_isstring(textObj)) {
		return;
	}

	const char *line = lua_getstring(textObj);
	Common::String text = line;

	TextObject *textObject = new TextObject(true);
	textObject->setDefaults(&g_grim->_blastTextDefaults);
	lua_Object tableObj = lua_getparam(2);
	if (lua_istable(tableObj))
		setTextObjectParams(textObject, tableObj);

	textObject->setText(text.c_str());
	textObject->createBitmap();
	textObject->draw();
	delete textObject;
}

void L1_SetOffscreenTextPos() {
	warning("L1_SetOffscreenTextPos: implement opcode");
	// this sets where we shouldn't put dialog maybe?
}

void setTextObjectParams(TextObjectCommon *textObject, lua_Object tableObj) {
	lua_Object keyObj;

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectX));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setX((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectY));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setY((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFont));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('F','O','N','T')) {
			textObject->setFont(getfont(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectWidth));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setWidth((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectHeight));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setHeight((int)lua_getnumber(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			textObject->setFGColor(getcolor(keyObj));
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectBGColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//textObject->setBGColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy BGColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectFXColor));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isuserdata(keyObj) && lua_tag(keyObj) == MKTAG('C','O','L','R')) {
			//textObject->setFXColor(static_cast<Color *>(lua_getuserdata(keyObj)));
			warning("setTextObjectParams: dummy FXColor");
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectCenter));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(1); //5
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectLJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(2); //4
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectRJustify));
	keyObj = lua_gettable();
	if (keyObj) {
		if (!lua_isnil(keyObj)) {
			textObject->setJustify(3); //6
		}
	}

	lua_pushobject(tableObj);
	lua_pushobject(lua_getref(refTextObjectDuration));
	keyObj = lua_gettable();
	if (keyObj) {
		if (lua_isnumber(keyObj)) {
			textObject->setDuration(lua_getnumber(keyObj));
		}
	}
}

// 0 - translate from '/msgId/'
// 1 - don't translate - message after '/msgId'
// 2 - return '/msgId/'
int translationMode = 0;

Common::String parseMsgText(const char *msg, char *msgId) {
	Common::String translation = g_localizer->localize(msg);
	const char *secondSlash = NULL;

	if (msg[0] == '/' && msgId) {
		secondSlash = strchr(msg + 1, '/');
		if (secondSlash) {
			strncpy(msgId, msg + 1, secondSlash - msg - 1);
			msgId[secondSlash - msg - 1] = 0;
		} else {
			msgId[0] = 0;
		}
	}

	if (translationMode == 1)
		return secondSlash;

	if (translationMode == 2)
		return msg;

	return translation;
}

void L1_TextFileGetLine() {
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

void L1_TextFileGetLineCount() {
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

void L1_LocalizeString() {
	char msgId[50], buf[1000];
	lua_Object strObj = lua_getparam(1);

	if (lua_isstring(strObj)) {
		const char *str = lua_getstring(strObj);
		// If the string that we're passed isn't localized yet then
		// construct the localized string, otherwise spit back what
		// we've been given
		if (str[0] == '/' && str[strlen(str) - 1] == '/') {
			Common::String msg = parseMsgText(str, msgId);
			sprintf(buf, "/%s/%s", msgId, msg.c_str());
			str = buf;
		}
		lua_pushstring(str);
	}
}

void parseSayLineTable(lua_Object paramObj, bool *background, int *vol, int *pan, int *x, int *y) {
	lua_Object tableObj;

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectX));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*x)
			*x = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectY));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*y)
			*y = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectBackground));
	tableObj = lua_gettable();
	if (!lua_isnil(tableObj)) {
		if (*background)
			*background = 0;
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectVolume));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*vol)
			*vol = (int)lua_getnumber(tableObj);
	}

	lua_pushobject(paramObj);
	lua_pushobject(lua_getref(refTextObjectPan));
	tableObj = lua_gettable();
	if (lua_isnumber(tableObj)) {
		if (*pan)
			*pan = (int)lua_getnumber(tableObj);
	}
}

void L1_SetSayLineDefaults() {
	lua_Object tableObj = lua_getparam(1);
	if (tableObj && lua_istable(tableObj))
		setTextObjectParams(&g_grim->_sayLineDefaults, tableObj);
}

void L1_SayLine() {
	int vol = 127, buffer = 64, paramId = 1, x = -1, y = -1;
	bool background = true;
	char msgId[50];
	Common::String msg;
	lua_Object paramObj = lua_getparam(paramId++);

	if ((lua_isuserdata(paramObj) && lua_tag(paramObj) == MKTAG('A','C','T','R'))
			|| lua_isstring(paramObj) || lua_istable(paramObj)) {
		Actor *actor = NULL;//some_Actor, maybe some current actor
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
						const char *tmpstr = lua_getstring(paramObj);
						msg = parseMsgText(tmpstr, msgId);
					}
				}
				paramObj = lua_getparam(paramId++);
			}
			if (!msg.empty()) {
				actor->sayLine(msg.c_str(), msgId); //background, vol, pan, x, y
			}
		}
	}
}

void L1_ShutUpActor() {
	lua_Object actorObj = lua_getparam(1);

	if (!lua_isuserdata(actorObj) || lua_tag(actorObj) != MKTAG('A','C','T','R'))
		return;
	Actor *actor = getactor(actorObj);
	if (actor)
		actor->shutUp();
}

void L1_PrintLine() {
	int vol = 127, buffer = 64, /*paramId = 1, */x = -1, y = -1;
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

void L1_InputDialog() {
	lua_Object str1Obj = lua_getparam(1);
	lua_Object str2Obj = lua_getparam(2);
	int c, i = 0;
	char buf[512];

	if (!lua_isstring(str1Obj) || !lua_isstring(str2Obj)) {
		lua_pushnil();
		return;
	}
	fprintf(stderr, "%s %s: ", lua_getstring(str1Obj), lua_getstring(str2Obj));
	while (i < 512 && (c = fgetc(stdin)) != EOF && c != '\n')
		buf[i++] = c;
	buf[i] = '\0';

	lua_pushstring(buf);
}

void L1_IsMessageGoing() {
	lua_Object actorObj = lua_getparam(1);

	if (!actorObj || (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) || lua_isnil(actorObj)) {
		if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) {
			Actor *actor = getactor(actorObj);
			if (actor) {
				pushbool(actor->isTalking());
			}
		} else {
			// NOTE: i'm not sure this currentTextObject stuff is totally right.
			// if you do changes test them against the crying angelitos in the fo set.
			// the dialog menu should appear few secods after they start crying.
			pushbool(g_grim->getTalkingActor() != NULL);
		}
	} else
		lua_pushnil();
}

} // end of namespace Grim
