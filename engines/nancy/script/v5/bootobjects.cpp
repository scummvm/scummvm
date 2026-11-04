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

#include "engines/nancy/nancy.h"

#include "engines/nancy/script/api.h"
#include "engines/nancy/script/bootapi.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/types.h"

#include "engines/nancy/script/v5/bootobjects.h"

#include "engines/nancy/ui/ndui/button.h"

namespace Nancy {
namespace Script {

// Nancy26 gives every item the scene that shows it, or nothing. The pictures
// of the items are cursors.
int createNancy26Inventory(lua_State *L, INVD &inventory) {
	Common::Array<Common::String> names;
	getSortedKeys(L, 2, names);

	for (const Common::String &name : names) {
		INVD::Item item;
		item.id = (int16)inventory.items.size();
		item.name = name;

		Common::String sceneName;
		readField(L, 2, name.c_str(), sceneName);
		item.sceneName = Common::Path(sceneName);
		item.kind = sceneName.empty() ? UI::NDUI::InvButton::kItemPickUp : UI::NDUI::InvButton::kItemView;

		inventory.items.push_back(item);
	}

	return 0;
}

// Nancy26: every cursor is a single picture, with its own image. What used to be
// the variant for hovering something is a cursor of its own ("MagGlassHot"), and
// the items are cursors named like them.
static void readNancy26Cursor(lua_State *L, int cursorTable, ScriptBootData::Cursor &cursor) {
	Common::String imageName;
	if (!readField(L, cursorTable, "ovl", imageName) || !pushField(L, cursorTable, "source")) {
		luaL_error(L, "Cursor %s needs the fields ovl, source and activePt", cursor.name.c_str());
	}

	cursor.imageName = Common::Path(imageName);
	cursor.sources[0] = cursor.sources[1] = checkRect(L, -1).toRect();
	lua_pop(L, 1);

	ScriptPoint hotspot;
	if (pushField(L, cursorTable, "activePt")) {
		readPoint(L, -1, hotspot);
		lua_pop(L, 1);
	}

	cursor.hotspots[0] = cursor.hotspots[1] = hotspot.toPoint();
}

int createNancy26Cursors(lua_State *L, ScriptBootData &boot) {
	requireCursorField(L, "default", boot.defaultCursorName);
	readField(L, 2, "rightNode", boot.rightNodeCursorName);
	readField(L, 2, "leftNode", boot.leftNodeCursorName);
	readField(L, 2, "uturnNode", boot.uturnNodeCursorName);

	ScriptPoint offset;
	if (pushField(L, 2, "invOffset")) {
		readPoint(L, -1, offset);
		lua_pop(L, 1);
	}

	boot.itemCursorOffset = offset.toPoint();

	Common::Array<Common::String> names;
	getSortedKeys(L, 2, names);

	for (const Common::String &name : names) {
		lua_getfield(L, 2, name.c_str());
		if (lua_istable(L, -1)) {
			ScriptBootData::Cursor cursor;
			cursor.name = name;
			readNancy26Cursor(L, lua_gettop(L), cursor);
			boot.cursors.push_back(cursor);
		}

		lua_pop(L, 1);
	}

	if (!hasCursor(boot, boot.defaultCursorName)) {
		luaL_error(L, "CursorInit:Create specified default as %s but there's no cursor by that name", boot.defaultCursorName.c_str());
	}

	return 0;
}

// One style of a font made of pictures, as a BMFont file describes it:
// { ovl = picture, height = line height, [char] = { x, y, width, height, xoffset,
// yoffset, xadvance }, kerning = { { first, second, amount }, ... } }
static void readFontFace(lua_State *L, int faceTable, ScriptBootData::BitmapFont::Face &face) {
	Common::String imageName;
	if (!readField(L, faceTable, "ovl", imageName)) {
		luaL_error(L, "A style of a font has no 'ovl' field");
	}

	face.imageName = Common::Path(imageName);
	readField(L, faceTable, "height", face.height);

	lua_pushnil(L);
	while (lua_next(L, faceTable)) {
		if (lua_type(L, -2) == LUA_TNUMBER && lua_istable(L, -1)) {
			const int glyphTable = lua_gettop(L);
			int x = 0, y = 0, width = 0, height = 0, xOffset = 0, yOffset = 0, xAdvance = 0;
			readField(L, glyphTable, "x", x);
			readField(L, glyphTable, "y", y);
			readField(L, glyphTable, "width", width);
			readField(L, glyphTable, "height", height);
			readField(L, glyphTable, "xoffset", xOffset);
			readField(L, glyphTable, "yoffset", yOffset);
			readField(L, glyphTable, "xadvance", xAdvance);

			ScriptBootData::BitmapFont::Glyph glyph;
			glyph.source = Common::Rect(x, y, x + width, y + height);
			glyph.xOffset = (int16)xOffset;
			glyph.yOffset = (int16)yOffset;
			glyph.xAdvance = (int16)xAdvance;
			face.glyphs.setVal((uint32)lua_tointeger(L, -2), glyph);
		}

		lua_pop(L, 1);
	}

	if (pushField(L, faceTable, "kerning")) {
		const int kerningTable = lua_gettop(L);
		for (int i = 1; i <= (int)lua_objlen(L, kerningTable); ++i) {
			lua_rawgeti(L, kerningTable, i);
			if (lua_istable(L, -1)) {
				const int pairTable = lua_gettop(L);
				int first = 0, second = 0, amount = 0;
				readField(L, pairTable, "first", first);
				readField(L, pairTable, "second", second);
				readField(L, pairTable, "amount", amount);
				face.kerning.setVal(((uint32)first << 16) | (uint32)(second & 0xFFFF), amount);
			}

			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	}
}

// FontsInit:Create in the file of a font: its four styles
int createBitmapFont(lua_State *L, ScriptBootData::BitmapFont &font) {
	checkCreateArguments(L, "ND.FontsInit", true);

	static const char *const kStyleNames[ScriptBootData::BitmapFont::kNumStyles] = {
		"regular", "bold", "italic", "boldItalic"
	};

	for (int i = 0; i < ScriptBootData::BitmapFont::kNumStyles; ++i) {
		if (pushField(L, 2, kStyleNames[i])) {
			luaL_checktype(L, -1, LUA_TTABLE);
			readFontFace(L, lua_gettop(L), font.faces[i]);
			lua_pop(L, 1);
		}
	}

	if (font.faces[ScriptBootData::BitmapFont::kRegular].imageName.empty()) {
		luaL_error(L, "Font %s has no regular style", font.name.c_str());
	}

	return 0;
}

// FontsBoot:Init({ { name, number, file }, ... }): the fonts of the
// game, each described by a script file of its own
static int fontsBootInit(lua_State *L) {
	checkCreateArguments(L, "ND.FontsBoot", getBootTables() && getBootTables()->boot);
	ScriptBootData &boot = *getBootTables()->boot;

	for (int i = 1; i <= (int)lua_objlen(L, 2); ++i) {
		lua_rawgeti(L, 2, i);
		luaL_checktype(L, -1, LUA_TTABLE);
		const int entry = lua_gettop(L);

		ScriptBootData::BitmapFont font;
		Common::String fileName;
		readField(L, entry, "name", font.name);
		readField(L, entry, "number", font.id);
		if (!readField(L, entry, "file", fileName)) {
			luaL_error(L, "Font %s names no file", font.name.c_str());
		}

		lua_pop(L, 1);

		BootTables fontTables;
		fontTables.font = &font;
		if (!runBootScript(fileName.c_str(), fontTables)) {
			luaL_error(L, "Could not load the font file %s", fileName.c_str());
		}

		boot.bitmapFonts.push_back(font);
	}

	return 0;
}

// FontColorsInit:Create({ [number] = Color, ... })
static int fontColorsCreate(lua_State *L) {
	checkCreateArguments(L, "ND.FontColorsInit", getBootTables() && getBootTables()->boot);
	ScriptBootData &boot = *getBootTables()->boot;

	lua_pushnil(L);
	while (lua_next(L, 2)) {
		ScriptBootData::FontColor color;
		color.id = luaL_checkint(L, -2);
		color.normal = color.disabled = color.mouseOver = color.pressed = checkColor(L, -1);
		boot.fontColors.push_back(color);
		lua_pop(L, 1);
	}

	return 0;
}

// AutotextBoot:Load({ file, ... }): the files of texts
static int autotextBootLoad(lua_State *L) {
	checkCreateArguments(L, "ND.AutotextBoot", getBootTables() && getBootTables()->boot);
	readStringList(L, 2, getBootTables()->boot->textFileNames);
	return 0;
}

// BuildNumberInit:Define(text)
static int buildNumberDefine(lua_State *L) {
	checkUserData(L, 1, "ND.BuildNumberBoot");
	if (!getBootTables() || !getBootTables()->boot) {
		luaL_error(L, "ND.BuildNumberBoot cannot be used by this script");
	}

	getBootTables()->boot->buildNumber = luaL_checkstring(L, 2);
	return 0;
}

void registerNancy26BootAPI(lua_State *L) {
	struct NamedFunction {
		const char *globalName;
		const char *typeName;
		const char *functionName;
		lua_CFunction function;
	};

	static const NamedFunction kObjects[] = {
		{ "FontsBoot", "ND.FontsBoot", "Init", fontsBootInit },
		{ "FontColorsInit", "ND.FontColorsInit", "Create", fontColorsCreate },
		{ "AutotextBoot", "ND.AutotextBoot", "Load", autotextBootLoad },
		{ "BuildNumberInit", "ND.BuildNumberBoot", "Define", buildNumberDefine }
	};

	for (const NamedFunction &object : kObjects) {
		const luaL_Reg functions[] = {
			{ "__index", indexFunctions },
			{ object.functionName, object.function },
			{ nullptr, nullptr }
		};

		registerGlobal(L, object.globalName, object.typeName, functions);
	}
}

} // End of namespace Script

static void runRequiredScript(const char *name, const Script::BootTables &tables) {
	const Common::String scriptName = Common::String(name) + "_SC";
	if (!Script::runBootScript(scriptName.c_str(), tables)) {
		error("Failed to load boot script %s", scriptName.c_str());
	}
}

// Nancy26 reads a different set of boot scripts, in this order. Its BOOT_SC
// is left over from the earlier games; the exe never runs it. What it set
// is fixed in the exe instead.
void NancyEngine::bootNancy26Data(ScriptBootData *boot, EVNT *flags, INVD *inventory, ENVS *environments, VIEW *viewport, CVTX *autotext) {
	Script::BootTables flagTables;
	flagTables.flags = flags;
	runRequiredScript("Flags", flagTables);

	Script::BootTables bootTables;
	bootTables.boot = boot;
	runRequiredScript("Vars", bootTables);
	runRequiredScript("Strs", bootTables);
	runRequiredScript("Fonts", bootTables);

	Script::BootTables inventoryTables;
	inventoryTables.inventory = inventory;
	inventoryTables.boot = boot;
	runRequiredScript("InventoryInit", inventoryTables);
	runRequiredScript("Cursors", inventoryTables);

	Script::BootTables envTables;
	envTables.environments = environments;
	runRequiredScript("Envs", envTables);

	runRequiredScript("AutotextBoot", bootTables);
	runRequiredScript("BuildNumber", bootTables);

	Script::BootTables textTables;
	textTables.texts = autotext;
	for (const Common::String &name : boot->textFileNames) {
		if (!Script::runBootScript(name.c_str(), textTables)) {
			error("Failed to load text file %s", name.c_str());
		}
	}

	// The window is 1024x768, and the viewport the 1024x690 at the top of it
	viewport->screenPosition = Common::Rect(0, 0, 1024, 690);
	viewport->bounds = Common::Rect(0, 0, 1024, 768);

	// The sides of a node turn it, and the strip along the bottom turns it around
	const int width = viewport->screenPosition.width();
	const int height = viewport->screenPosition.height();
	boot->leftNodeHotspot = Common::Rect(0, 0, 47, height - 80);
	boot->rightNodeHotspot = Common::Rect(width - 47, 0, width, height - 80);
	boot->uturnNodeHotspot = Common::Rect(0, height - 80, width, height);

	boot->defaultSaveName = "Tomb";

	// The scripts every scene starts with
	static const char *const kIncludeFiles[] = { "Math_SC", "Fader_SC", "string_SC", "ScriptingShortcuts_SC" };
	for (const char *name : kIncludeFiles) {
		boot->includeFileNames.push_back(name);
	}
}

} // End of namespace Nancy
