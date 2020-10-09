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

#include "common/endian.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "graphics/pixelbuffer.h"
#include "graphics/renderer.h"
#include "graphics/colormasks.h"

#include "math/matrix3.h"

#include "engines/grim/debug.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/registry.h"
#include "engines/grim/grim.h"
#include "engines/grim/savegame.h"
#include "engines/grim/resource.h"
#include "engines/grim/bitmap.h"
#include "engines/grim/font.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/localize.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/luadebug.h"

namespace Grim {

byte clamp_color(int c) {
	if (c < 0)
		return 0;
	else if (c > 255)
		return 255;
	else
		return c;
}

int luaA_passresults();

void Lua_V1::new_dofile() {
	const char *fname_str = luaL_check_string(1);
	if (dofile(fname_str) == 0)
		if (luaA_passresults() == 0)
			lua_pushuserdata(0);
}

// Debugging message functions

void Lua_V1::PrintDebug() {
	if (Debug::isChannelEnabled(Debug::Scripts | Debug::Info)) {
		Common::String msg("Debug: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj));
		debugN("%s", msg.c_str());
	}
}

void Lua_V1::PrintError() {
	if (Debug::isChannelEnabled(Debug::Scripts | Debug::Error)) {
		Common::String msg("Error: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj));
		debugN("%s", msg.c_str());
	}
}

void Lua_V1::PrintWarning() {
	if (Debug::isChannelEnabled(Debug::Scripts | Debug::Warning)) {
		Common::String msg("Warning: ");
		lua_Object strObj = lua_getparam(1);
		if (lua_isnil(strObj))
			msg += "(nil)";
		if (!lua_isstring(strObj))
			return;
		msg += Common::String(lua_getstring(strObj));
		debugN("%s", msg.c_str());
	}
}

void Lua_V1::FunctionName() {
	const char *name;
	char buf[256];
	const char *filename = nullptr;
	int32 line;
	lua_Object param1 = lua_getparam(1);

	if (!lua_isfunction(param1)) {
		sprintf(buf, "function InvalidArgsToFunctionName");
		lua_pushstring(buf);
		return;
	}

	lua_funcinfo(param1, &filename, &line);
	switch (*lua_getobjname(param1, &name)) {
	case 'g':
		sprintf(buf, "function %.100s", name);
		break;
	case 't':
		sprintf(buf, "`%.100s' tag method", name);
		break;
	default:
		{
			if (line == 0)
				sprintf(buf, "main of %.100s", filename);
			else if (line < 0)
				sprintf(buf, "%.100s", filename);
			else {
				sprintf(buf, "function (%.100s:%d)", filename, (int)line);
				filename = nullptr;
			}
		}
	}
	int curr_line = lua_currentline(param1);
	if (curr_line > 0)
		sprintf(buf + strlen(buf), " at line %d", curr_line);
	if (filename)
		sprintf(buf + strlen(buf), " [in file %.100s]", filename);
	lua_pushstring(buf);
}

void Lua_V1::CheckForFile() {
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *filename = lua_getstring(strObj);
	pushbool(SearchMan.hasFile(filename));
}

void Lua_V1::MakeColor() {
	lua_Object rObj = lua_getparam(1);
	lua_Object gObj = lua_getparam(2);
	lua_Object bObj = lua_getparam(3);
	int r, g, b;

	if (!lua_isnumber(rObj))
		r = 0;
	else
		r = clamp_color((int)lua_getnumber(rObj));

	if (!lua_isnumber(gObj))
		g = 0;
	else
		g = clamp_color((int)lua_getnumber(gObj));

	if (!lua_isnumber(bObj))
		b = 0;
	else
		b = clamp_color((int)lua_getnumber(bObj));

	Color c(r, g, b);
	int32 cTag = c.toEncodedValue();
	if (g_grim->getGameType() == GType_MONKEY4)
		cTag |= (0xFF << 24);
	lua_pushusertag(cTag, MKTAG('C','O','L','R'));
}

void Lua_V1::GetColorComponents() {
	lua_Object colorObj = lua_getparam(1);
	Color c(getcolor(colorObj));
	lua_pushnumber(c.getRed());
	lua_pushnumber(c.getGreen());
	lua_pushnumber(c.getBlue());
}

void Lua_V1::ReadRegistryValue() {
	lua_Object keyObj = lua_getparam(1);

	if (!lua_isstring(keyObj)) {
		lua_pushnil();
		return;
	}
	const char *key = lua_getstring(keyObj);

	Registry::ValueType type = g_registry->getValueType(key);
	switch (type) {
	case Registry::String:
		lua_pushstring(g_registry->getString(key).c_str());
		break;
	case Registry::Integer:
		lua_pushnumber(g_registry->getInt(key));
		break;
	case Registry::Boolean:
		pushbool(g_registry->getBool(key));
		break;
	}
}

void Lua_V1::WriteRegistryValue() {
	lua_Object keyObj = lua_getparam(1);
	lua_Object valObj = lua_getparam(2);

	if (!lua_isstring(keyObj))
		return;

	const char *key = lua_getstring(keyObj);
	if (strcmp(key, "GrimMannyState") == 0) //This isn't used. it's probably a left over from testing phase.
		return;

	// Check isnumber first, because isstring returns true for numbers
	if (lua_isnumber(valObj)) {
		int val = (int)lua_getnumber(valObj);
		g_registry->setInt(key, val);
	} else if (lua_isstring(valObj)) {
		const char *val = lua_getstring(valObj);
		g_registry->setString(key, val);
	}
}

void Lua_V1::GetAngleBetweenVectors() {
	lua_Object vec1Obj = lua_getparam(1);
	lua_Object vec2Obj = lua_getparam(2);

	if (!lua_istable(vec1Obj) || !lua_istable(vec2Obj)) {
		lua_pushnil();
		return;
	}

	lua_pushobject(vec1Obj);
	lua_pushstring("x");
	lua_Object table = lua_gettable();
	float x1 = lua_getnumber(table);
	lua_pushobject(vec1Obj);
	lua_pushstring("y");
	table = lua_gettable();
	float y1 = lua_getnumber(table);
	lua_pushobject(vec1Obj);
	lua_pushstring("z");
	table = lua_gettable();
	float z1 = lua_getnumber(table);
	lua_pushobject(vec2Obj);
	lua_pushstring("x");
	table = lua_gettable();
	float x2 = lua_getnumber(table);
	lua_pushobject(vec2Obj);
	lua_pushstring("y");
	table = lua_gettable();
	float y2 = lua_getnumber(table);
	lua_pushobject(vec2Obj);
	lua_pushstring("z");
	table = lua_gettable();
	float z2 = lua_getnumber(table);

	Math::Vector3d vec1(x1, y1, z1);
	Math::Vector3d vec2(x2, y2, z2);
	vec1.normalize();
	vec2.normalize();

	float dot = vec1.dotProduct(vec2);
	float angle = 90.0f - (180.0f * asin(dot)) / (float)M_PI;
	if (angle < 0)
		angle = -angle;
	lua_pushnumber(angle);
}

void Lua_V1::Is3DHardwareEnabled() {
	pushbool(g_driver->isHardwareAccelerated());
}

void Lua_V1::SetHardwareState() {
	// changing only in config setup (software/hardware rendering)
	bool accel = getbool(1);

	Graphics::RendererType renderer = accel ? Graphics::kRendererTypeOpenGL : Graphics::kRendererTypeTinyGL;
	renderer = Graphics::getBestMatchingAvailableRendererType(renderer);
	ConfMan.set("renderer", Graphics::getRendererTypeCode(renderer));

	g_grim->changeHardwareState();
}

void Lua_V1::SetVideoDevices() {
	/*int devId = (int)*/lua_getnumber(lua_getparam(1));
	/*int modeId = (int)*/lua_getnumber(lua_getparam(2));
	// ignore setting video devices
}

void Lua_V1::GetVideoDevices() {
	lua_pushnumber(0.0);
	lua_pushnumber(-1.0);
}

void Lua_V1::EnumerateVideoDevices() {
	lua_Object result = lua_createtable();
	lua_pushobject(result);
	lua_pushnumber(0.0); // id of device
	lua_pushstring(g_driver->getVideoDeviceName()); // name of device
	lua_settable();
	lua_pushobject(result);
}

void Lua_V1::Enumerate3DDevices() {
	lua_Object result = lua_createtable();
	lua_Object numObj = lua_getparam(1);
	if (!lua_isnumber(numObj))
		return;
/*	int num = (int)lua_getnumber(numObj);*/
	lua_pushobject(result);
	lua_pushnumber(-1.0);
	if (g_driver->isHardwareAccelerated()) {
		lua_pushstring("OpenGL"); // type of 3d renderer
	} else {
		lua_pushstring("/engn003/Software"); // type of 3d renderer
	}
	lua_settable();
	lua_pushobject(result);
}

/* RotateVector takes a vector and rotates it around
 * the point (0,0,0) by the requested number of degrees.
 * This function is used to calculate the locations for
 * getting on and off of the Bone Wagon and for going up
 * and down the slide with the chain at the end of the world.
 */
void Lua_V1::RotateVector() {
	lua_Object vecObj = lua_getparam(1);
	lua_Object rotObj = lua_getparam(2);

	if (!lua_istable(vecObj) || !lua_istable(rotObj)) {
		lua_pushnil();
		return;
	}

	lua_pushobject(vecObj);
	lua_pushstring("x");
	float x = lua_getnumber(lua_gettable());
	lua_pushobject(vecObj);
	lua_pushstring("y");
	float y = lua_getnumber(lua_gettable());
	lua_pushobject(vecObj);
	lua_pushstring("z");
	float z = lua_getnumber(lua_gettable());
	Math::Vector3d vec(x, y, z);

	lua_pushobject(rotObj);
	lua_pushstring("x");
	Math::Angle pitch = lua_getnumber(lua_gettable());
	lua_pushobject(rotObj);
	lua_pushstring("y");
	Math::Angle yaw = lua_getnumber(lua_gettable());
	lua_pushobject(rotObj);
	lua_pushstring("z");
	Math::Angle roll = lua_getnumber(lua_gettable());

	Math::Matrix3 mat;
	mat.buildFromEuler(yaw, pitch, roll, Math::EO_ZXY);
	mat.transformVector(&vec);

	lua_Object resObj = lua_createtable();
	lua_pushobject(resObj);
	lua_pushstring("x");
	lua_pushnumber(vec.x());
	lua_settable();
	lua_pushobject(resObj);
	lua_pushstring("y");
	lua_pushnumber(vec.y());
	lua_settable();
	lua_pushobject(resObj);
	lua_pushstring("z");
	lua_pushnumber(vec.z());
	lua_settable();

	lua_pushobject(resObj);
}

void Lua_V1::FileFindDispose() {
	g_grim->_listFiles.clear();
	g_grim->_listFilesIter = nullptr;
}

void Lua_V1::FileFindNext() {
	if (g_grim->_listFilesIter == g_grim->_listFiles.end()) {
		lua_pushnil();
		Lua_V1::FileFindDispose();
	} else {
		lua_pushstring(g_grim->_listFilesIter->c_str());
		g_grim->_listFilesIter++;
	}
}

void Lua_V1::FileFindFirst() {
	lua_Object extObj = lua_getparam(1);
	if (!lua_isstring(extObj)) {
		lua_pushnil();
		return;
	}

	Lua_V1::FileFindDispose();

	const char *extension = lua_getstring(extObj);
	Common::String searchString = extension;
	if (searchString == "*.gsv") {
		searchString = "grim##.gsv";
	}
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	g_grim->_listFiles = saveFileMan->listSavefiles(searchString);
	Common::sort(g_grim->_listFiles.begin(), g_grim->_listFiles.end());
	g_grim->_listFilesIter = g_grim->_listFiles.begin();

	if (g_grim->_listFilesIter == g_grim->_listFiles.end())
		lua_pushnil();
	else
		Lua_V1::FileFindNext();
}

void Lua_V1::PerSecond() {
	lua_Object rateObj = lua_getparam(1);

	if (!lua_isnumber(rateObj)) {
		lua_pushnil();
		return;
	}
	float rate = lua_getnumber(rateObj);
	lua_pushnumber(g_grim->getPerSecond(rate));
}

void Lua_V1::EnableControl() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj)) {
		lua_pushnil();
		return;
	}
	int num = (int)lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");

	g_grim->enableControl(num);
}

void Lua_V1::DisableControl() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj)) {
		lua_pushnil();
		return;
	}
	int num = (int)lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");

	g_grim->disableControl(num);
}

void Lua_V1::GetControlState() {
	lua_Object numObj = lua_getparam(1);

	if (!lua_isnumber(numObj))
		return;

	int num = (int)lua_getnumber(numObj);
	if (num < 0 || num >= KEYCODE_EXTRA_LAST)
		error("control identifier out of range");
	if (num >= KEYCODE_AXIS_JOY1_X && num <= KEYCODE_AXIS_MOUSE_Z)
		lua_pushnumber(g_grim->getControlAxis(num));
	else {
		pushbool(g_grim->getControlState(num)); // key down, originaly it push number if key down
		//pushnil or number, what is is ?
	}
}

void Lua_V1::Exit() {
	g_grim->quitGame();
}

void Lua_V1::SetSpeechMode() {
	GrimEngine::SpeechMode mode = (GrimEngine::SpeechMode)((int)lua_getnumber(lua_getparam(1)));
	if (mode >= 1 && mode <= 3)
		g_grim->setSpeechMode(mode);
}

void Lua_V1::GetSpeechMode() {
	lua_pushnumber(g_grim->getSpeechMode());
}

void Lua_V1::GetDiskFreeSpace() {
	//the ps2 version of emi wants more than 600 KB
	//grim: amount of free space in MB, used for creating saves
	lua_pushnumber(700);
}

void Lua_V1::GetCurrentScript() {
	current_script();
}

void Lua_V1::GetSaveGameImage() {
	int width = 250, height = 188;
	Bitmap *screenshot;
	int dataSize;

	lua_Object param = lua_getparam(1);
	if (!lua_isstring(param)) {
		lua_pushnil();
		return;
	}
	const char *filename = lua_getstring(param);
	SaveGame *savedState = SaveGame::openForLoading(filename);
	if (!savedState || !savedState->isCompatible()) {
		delete savedState;
		lua_pushnil();
		return;
	}
	dataSize = savedState->beginSection('SIMG');
	uint16 *data = new uint16[dataSize / 2];
	for (int l = 0; l < dataSize / 2; l++) {
		data[l] = savedState->readLEUint16();
	}
	Graphics::PixelBuffer buf(Graphics::createPixelFormat<565>(), (byte *)data);
	screenshot = new Bitmap(buf, width, height, "screenshot");
	delete[] data;
	if (screenshot) {
		lua_pushusertag(screenshot->getId(), MKTAG('V','B','U','F'));
	} else {
		lua_pushnil();
		warning("Could not restore screenshot from file");
		delete savedState;
		return;
	}
	savedState->endSection();
	delete savedState;
}

void Lua_V1::SubmitSaveGameData() {
	lua_Object table, table2;
	SaveGame *savedState;
	const char *str;
	table = lua_getparam(1);

	savedState = g_grim->savedState();
	if (!savedState)
		error("Cannot obtain saved game");
	savedState->beginSection('SUBS');
	int count = 0;
	Common::String localized;
	for (;;) {
		lua_pushobject(table);
		lua_pushnumber(count);
		count++;
		table2 = lua_gettable();
		if (lua_isnil(table2))
			break;
		str = lua_getstring(table2);
		if (g_grim->getGameType() == GType_MONKEY4 &&
			g_grim->getGamePlatform() == Common::kPlatformPS2) {
			if (count == 1) {
				localized = g_localizer->localize(str);
			}
		}
		int32 len = strlen(str) + 1;
		savedState->writeLESint32(len);
		savedState->write(str, len);
	}
	savedState->endSection();
	
	//give ps2 saves a human-readable name
	if (g_grim->getGameType() == GType_MONKEY4 &&
		g_grim->getGamePlatform() == Common::kPlatformPS2) {
		savedState->beginSection('PS2S');
		savedState->writeLESint32(localized.size() + 1);
		savedState->write(localized.c_str(), localized.size() + 1);
		savedState->endSection();
	}
}

void Lua_V1::GetSaveGameData() {
	lua_Object param = lua_getparam(1);
	if (!lua_isstring(param))
		return;
	Common::String filename(lua_getstring(param));
	if (g_grim->getGameType() == GType_MONKEY4 &&
		g_grim->getGamePlatform() == Common::kPlatformPS2) {
		filename += ".ps2";
	}
	SaveGame *savedState = SaveGame::openForLoading(filename);
	lua_Object result = lua_createtable();

	if (!savedState || !savedState->isCompatible()) {
		lua_pushobject(result);
		lua_pushnumber(2);
		lua_pushstring("mo.set"); // Just a placeholder to not make it throw a lua error
		lua_settable();
		lua_pushobject(result);

		if (!savedState) {
			warning("Savegame %s is invalid", filename.c_str());
		} else {
			warning("Savegame %s is incompatible with this ScummVM build. Save version: %d.%d; current version: %d.%d",
					filename.c_str(), savedState->saveMajorVersion(), savedState->saveMinorVersion(),
					SaveGame::SAVEGAME_MAJOR_VERSION, SaveGame::SAVEGAME_MINOR_VERSION);
		}
		delete savedState;
		return;
	}
	int32 dataSize = savedState->beginSection('SUBS');

	char str[200];
	int32 strSize;
	int count = 0;

	for (;;) {
		if (dataSize <= 0)
			break;
		strSize = savedState->readLESint32();
		savedState->read(str, strSize);
		lua_pushobject(result);
		lua_pushnumber(count);
		lua_pushstring(str);
		lua_settable();
		dataSize -= strSize;
		dataSize -= 4;
		count++;
	}
	lua_pushobject(result);

	savedState->endSection();
	delete savedState;
}

void Lua_V1::Load() {
	lua_Object fileNameObj = lua_getparam(1);
	if (lua_isnil(fileNameObj)) {
		g_grim->loadGame("");
	} else if (lua_isstring(fileNameObj)) {
		Common::String fileName(lua_getstring(fileNameObj));
		if (g_grim->getGameType() == GType_MONKEY4 &&
			g_grim->getGamePlatform() == Common::kPlatformPS2) {
			fileName += ".ps2";
		}
		g_grim->loadGame(fileName);
	} else {
		warning("Load() fileName is wrong");
		return;
	}
}

void Lua_V1::Save() {
	lua_Object fileNameObj = lua_getparam(1);
	if (lua_isnil(fileNameObj)) {
		g_grim->saveGame("");
	} else if (lua_isstring(fileNameObj)) {
		Common::String fileName(lua_getstring(fileNameObj));
		if (g_grim->getGameType() == GType_MONKEY4 &&
			g_grim->getGamePlatform() == Common::kPlatformPS2) {
			fileName += ".ps2";
		}
		g_grim->saveGame(fileName);
	} else {
		warning("Save() fileName is wrong");
		return;
	}
}

void Lua_V1::Remove() {
	if (g_system->getSavefileManager()->removeSavefile(luaL_check_string(1)))
		lua_pushuserdata(0);
	else {
		lua_pushnil();
		lua_pushstring(g_system->getSavefileManager()->getErrorDesc().c_str());
	}
}

void Lua_V1::LockFont() {
	lua_Object param1 = lua_getparam(1);
	if (lua_isstring(param1)) {
		const char *fontName = lua_getstring(param1);
		Font *result = g_resourceloader->loadFont(fontName);
		if (result) {
			lua_pushusertag(result->getId(), MKTAG('F','O','N','T'));
			return;
		}
	}

	lua_pushnil();
}

void Lua_V1::EnableDebugKeys() {
}

void Lua_V1::LightMgrSetChange() {
	// that seems only used when some control panel is opened
}

void Lua_V1::LightMgrStartup() {
	// we will not implement this opcode
}

void Lua_V1::JustLoaded() {
	Debug::error("OPCODE USAGE VERIFICATION: JustLoaded");
}

void Lua_V1::SetEmergencyFont() {
	Debug::error("OPCODE USAGE VERIFICATION: SetEmergencyFont");
}


// Stub function for builtin functions not yet implemented
static void stubWarning(const char *funcName) {
	warning("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }

STUB_FUNC(Lua_V1::SetActorInvClipNode)
STUB_FUNC(Lua_V1::NukeResources)
STUB_FUNC(Lua_V1::ResetTextures)
STUB_FUNC(Lua_V1::AttachToResources)
STUB_FUNC(Lua_V1::DetachFromResources)
STUB_FUNC(Lua_V1::SetActorClipPlane)
STUB_FUNC(Lua_V1::SetActorClipActive)
STUB_FUNC(Lua_V1::FlushControls)
STUB_FUNC(Lua_V1::GetCameraLookVector)
STUB_FUNC(Lua_V1::SetCameraRoll)
STUB_FUNC(Lua_V1::SetCameraInterest)
STUB_FUNC(Lua_V1::GetCameraPosition)
STUB_FUNC(Lua_V1::SpewStartup)
STUB_FUNC(Lua_V1::WorldToScreen)
STUB_FUNC(Lua_V1::SetActorRoll)
STUB_FUNC(Lua_V1::SetActorFrustrumCull)
STUB_FUNC(Lua_V1::DriveActorTo)
STUB_FUNC(Lua_V1::GetTranslationMode)
STUB_FUNC(Lua_V1::SetTranslationMode)
STUB_FUNC(Lua_V1::WalkActorToAvoiding)
STUB_FUNC(Lua_V1::GetActorChores)
STUB_FUNC(Lua_V1::SetCameraPosition)
STUB_FUNC(Lua_V1::GetCameraFOV)
STUB_FUNC(Lua_V1::SetCameraFOV)
STUB_FUNC(Lua_V1::GetCameraRoll)
STUB_FUNC(Lua_V1::GetMemoryUsage)
STUB_FUNC(Lua_V1::GetFontDimensions)
STUB_FUNC(Lua_V1::PurgeText)

struct luaL_reg grimMainOpcodes[] = {
	{ "EngineDisplay", LUA_OPCODE(Lua_V1, EngineDisplay) },
	{ "CheckForFile", LUA_OPCODE(Lua_V1, CheckForFile) },
	{ "Load", LUA_OPCODE(Lua_V1, Load) },
	{ "Save", LUA_OPCODE(Lua_V1, Save) },
	{ "remove", LUA_OPCODE(Lua_V1, Remove) },
	{ "SetActorColormap", LUA_OPCODE(Lua_V1, SetActorColormap) },
	{ "GetActorCostume", LUA_OPCODE(Lua_V1, GetActorCostume) },
	{ "SetActorCostume", LUA_OPCODE(Lua_V1, SetActorCostume) },
	{ "SetActorScale", LUA_OPCODE(Lua_V1, SetActorScale) },
	{ "GetActorTimeScale", LUA_OPCODE(Lua_V1, GetActorTimeScale) },
	{ "SetActorTimeScale", LUA_OPCODE(Lua_V1, SetActorTimeScale) },
	{ "GetActorNodeLocation", LUA_OPCODE(Lua_V1, GetActorNodeLocation) },
	{ "SetActorWalkChore", LUA_OPCODE(Lua_V1, SetActorWalkChore) },
	{ "SetActorTurnChores", LUA_OPCODE(Lua_V1, SetActorTurnChores) },
	{ "SetActorRestChore", LUA_OPCODE(Lua_V1, SetActorRestChore) },
	{ "SetActorMumblechore", LUA_OPCODE(Lua_V1, SetActorMumblechore) },
	{ "SetActorTalkChore", LUA_OPCODE(Lua_V1, SetActorTalkChore) },
	{ "SetActorWalkRate", LUA_OPCODE(Lua_V1, SetActorWalkRate) },
	{ "GetActorWalkRate", LUA_OPCODE(Lua_V1, GetActorWalkRate) },
	{ "SetActorTurnRate", LUA_OPCODE(Lua_V1, SetActorTurnRate) },
	{ "SetSelectedActor", LUA_OPCODE(Lua_V1, SetSelectedActor) },
	{ "LoadActor", LUA_OPCODE(Lua_V1, LoadActor) },
	{ "GetActorPos", LUA_OPCODE(Lua_V1, GetActorPos) },
	{ "GetActorRect", LUA_OPCODE(Lua_V1, GetActorRect) },
	{ "GetActorPuckVector", LUA_OPCODE(Lua_V1, GetActorPuckVector) },
	{ "GetActorYawToPoint", LUA_OPCODE(Lua_V1, GetActorYawToPoint) },
	{ "SetActorReflection", LUA_OPCODE(Lua_V1, SetActorReflection) },
	{ "PutActorAtInterest", LUA_OPCODE(Lua_V1, PutActorAtInterest) },
	{ "PutActorAt", LUA_OPCODE(Lua_V1, PutActorAt) },
	{ "PutActorInSet", LUA_OPCODE(Lua_V1, PutActorInSet) },
	{ "WalkActorVector", LUA_OPCODE(Lua_V1, WalkActorVector) },
	{ "WalkActorForward", LUA_OPCODE(Lua_V1, WalkActorForward) },
	{ "DriveActorTo", LUA_OPCODE(Lua_V1, DriveActorTo) },
	{ "WalkActorTo", LUA_OPCODE(Lua_V1, WalkActorTo) },
	{ "WalkActorToAvoiding", LUA_OPCODE(Lua_V1, WalkActorToAvoiding) },
	{ "ActorLookAt", LUA_OPCODE(Lua_V1, ActorLookAt) },
	{ "SetActorLookRate", LUA_OPCODE(Lua_V1, SetActorLookRate) },
	{ "GetActorLookRate", LUA_OPCODE(Lua_V1, GetActorLookRate) },
	{ "GetVisibleThings", LUA_OPCODE(Lua_V1, GetVisibleThings) },
	{ "GetCameraActor", LUA_OPCODE(Lua_V1, GetCameraActor) },
	{ "SetActorHead", LUA_OPCODE(Lua_V1, SetActorHead) },
	{ "SetActorVisibility", LUA_OPCODE(Lua_V1, SetActorVisibility) },
	{ "SetActorFollowBoxes", LUA_OPCODE(Lua_V1, SetActorFollowBoxes) },
	{ "ShutUpActor", LUA_OPCODE(Lua_V1, ShutUpActor) },
	{ "SetActorFrustrumCull", LUA_OPCODE(Lua_V1, SetActorFrustrumCull) },
	{ "IsActorInSector", LUA_OPCODE(Lua_V1, IsActorInSector) },
	{ "GetActorSector", LUA_OPCODE(Lua_V1, GetActorSector) },
	{ "IsPointInSector", LUA_OPCODE(Lua_V1, IsPointInSector) },
	{ "GetPointSector", LUA_OPCODE(Lua_V1, GetPointSector) },
	{ "TurnActor", LUA_OPCODE(Lua_V1, TurnActor) },
	{ "GetActorRot", LUA_OPCODE(Lua_V1, GetActorRot) },
	{ "SetActorRot", LUA_OPCODE(Lua_V1, SetActorRot) },
	{ "SetActorPitch", LUA_OPCODE(Lua_V1, SetActorPitch) },
	{ "SetActorRoll", LUA_OPCODE(Lua_V1, SetActorRoll) },
	{ "IsActorTurning", LUA_OPCODE(Lua_V1, IsActorTurning) },
	{ "PlayActorChore", LUA_OPCODE(Lua_V1, PlayActorChore) },
	{ "PlayActorChoreLooping", LUA_OPCODE(Lua_V1, PlayActorChoreLooping) },
	{ "StopActorChore", LUA_OPCODE(Lua_V1, StopActorChore) },
	{ "CompleteActorChore", LUA_OPCODE(Lua_V1, CompleteActorChore) },
	{ "IsActorMoving", LUA_OPCODE(Lua_V1, IsActorMoving) },
	{ "IsActorChoring", LUA_OPCODE(Lua_V1, IsActorChoring) },
	{ "IsActorResting", LUA_OPCODE(Lua_V1, IsActorResting) },
	{ "SetActorChoreLooping", LUA_OPCODE(Lua_V1, SetActorChoreLooping) },
	{ "GetActorChores", LUA_OPCODE(Lua_V1, GetActorChores) },
	{ "GetActorCostumeDepth", LUA_OPCODE(Lua_V1, GetActorCostumeDepth) },
	{ "WorldToScreen", LUA_OPCODE(Lua_V1, WorldToScreen) },
	{ "exit", LUA_OPCODE(Lua_V1, Exit) },
	{ "FunctionName", LUA_OPCODE(Lua_V1, FunctionName) },
	{ "EnableDebugKeys", LUA_OPCODE(Lua_V1, EnableDebugKeys) },
	{ "LockFont", LUA_OPCODE(Lua_V1, LockFont) },
	{ "EnableControl", LUA_OPCODE(Lua_V1, EnableControl) },
	{ "DisableControl", LUA_OPCODE(Lua_V1, DisableControl) },
	{ "GetControlState", LUA_OPCODE(Lua_V1, GetControlState) },
	{ "PrintError", LUA_OPCODE(Lua_V1, PrintError) },
	{ "PrintWarning", LUA_OPCODE(Lua_V1, PrintWarning) },
	{ "PrintDebug", LUA_OPCODE(Lua_V1, PrintDebug) },
	{ "MakeCurrentSet", LUA_OPCODE(Lua_V1, MakeCurrentSet) },
	{ "LockSet", LUA_OPCODE(Lua_V1, LockSet) },
	{ "UnLockSet", LUA_OPCODE(Lua_V1, UnLockSet) },
	{ "MakeCurrentSetup", LUA_OPCODE(Lua_V1, MakeCurrentSetup) },
	{ "GetCurrentSetup", LUA_OPCODE(Lua_V1, GetCurrentSetup) },
	{ "NextSetup", LUA_OPCODE(Lua_V1, NextSetup) },
	{ "PreviousSetup", LUA_OPCODE(Lua_V1, PreviousSetup) },
	{ "StartFullscreenMovie", LUA_OPCODE(Lua_V1, StartFullscreenMovie) },
	{ "IsFullscreenMoviePlaying", LUA_OPCODE(Lua_V1, IsFullscreenMoviePlaying) },
	{ "StartMovie", LUA_OPCODE(Lua_V1, StartMovie) },
	{ "StopMovie", LUA_OPCODE(Lua_V1, StopMovie) },
	{ "PauseMovie", LUA_OPCODE(Lua_V1, PauseMovie) },
	{ "IsMoviePlaying", LUA_OPCODE(Lua_V1, IsMoviePlaying) },
	{ "PlaySound", LUA_OPCODE(Lua_V1, PlaySound) },
	{ "PlaySoundAt", LUA_OPCODE(Lua_V1, PlaySoundAt) },
	{ "IsSoundPlaying", LUA_OPCODE(Lua_V1, IsSoundPlaying) },
	{ "SetSoundPosition", LUA_OPCODE(Lua_V1, SetSoundPosition) },
	{ "FileFindFirst", LUA_OPCODE(Lua_V1, FileFindFirst) },
	{ "FileFindNext", LUA_OPCODE(Lua_V1, FileFindNext) },
	{ "FileFindDispose", LUA_OPCODE(Lua_V1, FileFindDispose) },
	{ "InputDialog", LUA_OPCODE(Lua_V1, InputDialog) },
	{ "WriteRegistryValue", LUA_OPCODE(Lua_V1, WriteRegistryValue) },
	{ "ReadRegistryValue", LUA_OPCODE(Lua_V1, ReadRegistryValue) },
	{ "GetSectorOppositeEdge", LUA_OPCODE(Lua_V1, GetSectorOppositeEdge) },
	{ "MakeSectorActive", LUA_OPCODE(Lua_V1, MakeSectorActive) },
	{ "PreRender", LUA_OPCODE(Lua_V1, PreRender) },
	{ "SpewStartup", LUA_OPCODE(Lua_V1, SpewStartup) },
	{ "GetCurrentScript", LUA_OPCODE(Lua_V1, GetCurrentScript) },
	{ "PrintActorCostumes", LUA_OPCODE(Lua_V1, PrintActorCostumes) },
	{ "PushActorCostume", LUA_OPCODE(Lua_V1, PushActorCostume) },
	{ "PopActorCostume", LUA_OPCODE(Lua_V1, PopActorCostume) },
	{ "LoadCostume", LUA_OPCODE(Lua_V1, LoadCostume) },
	{ "RotateVector", LUA_OPCODE(Lua_V1, RotateVector) },
	{ "GetCameraPosition", LUA_OPCODE(Lua_V1, GetCameraPosition) },
	{ "SetCameraPosition", LUA_OPCODE(Lua_V1, SetCameraPosition) },
	{ "SetCameraInterest", LUA_OPCODE(Lua_V1, SetCameraInterest) },
	{ "GetCameraFOV", LUA_OPCODE(Lua_V1, GetCameraFOV) },
	{ "SetCameraFOV", LUA_OPCODE(Lua_V1, SetCameraFOV) },
	{ "GetCameraRoll", LUA_OPCODE(Lua_V1, GetCameraRoll) },
	{ "SetCameraRoll", LUA_OPCODE(Lua_V1, SetCameraRoll) },
	{ "GetCameraLookVector", LUA_OPCODE(Lua_V1, GetCameraLookVector) },
	{ "PointActorAt", LUA_OPCODE(Lua_V1, PointActorAt) },
	{ "TurnActorTo", LUA_OPCODE(Lua_V1, TurnActorTo) },
	{ "PerSecond", LUA_OPCODE(Lua_V1, PerSecond) },
	{ "GetAngleBetweenVectors", LUA_OPCODE(Lua_V1, GetAngleBetweenVectors) },
	{ "GetAngleBetweenActors", LUA_OPCODE(Lua_V1, GetAngleBetweenActors) },
	{ "SetAmbientLight", LUA_OPCODE(Lua_V1, SetAmbientLight) },
	{ "TurnLightOn", LUA_OPCODE(Lua_V1, TurnLightOn) },
	{ "SetLightPosition", LUA_OPCODE(Lua_V1, SetLightPosition) },
	{ "SetLightIntensity", LUA_OPCODE(Lua_V1, SetLightIntensity) },
	{ "LightMgrSetChange", LUA_OPCODE(Lua_V1, LightMgrSetChange) },
	{ "LightMgrStartup", LUA_OPCODE(Lua_V1, LightMgrStartup) },
	{ "ImStartSound", LUA_OPCODE(Lua_V1, ImStartSound) },
	{ "ImStopSound", LUA_OPCODE(Lua_V1, ImStopSound) },
	{ "ImStopAllSounds", LUA_OPCODE(Lua_V1, ImStopAllSounds) },
	{ "ImGetParam", LUA_OPCODE(Lua_V1, ImGetParam) },
	{ "ImSetParam", LUA_OPCODE(Lua_V1, ImSetParam) },
	{ "ImFadeParam", LUA_OPCODE(Lua_V1, ImFadeParam) },
	{ "ImGetSfxVol", LUA_OPCODE(Lua_V1, ImGetSfxVol) },
	{ "ImSetSfxVol", LUA_OPCODE(Lua_V1, ImSetSfxVol) },
	{ "ImGetVoiceVol", LUA_OPCODE(Lua_V1, ImGetVoiceVol) },
	{ "ImSetVoiceVol", LUA_OPCODE(Lua_V1, ImSetVoiceVol) },
	{ "ImGetMusicVol", LUA_OPCODE(Lua_V1, ImGetMusicVol) },
	{ "ImSetMusicVol", LUA_OPCODE(Lua_V1, ImSetMusicVol) },
	{ "ImSetState", LUA_OPCODE(Lua_V1, ImSetState) },
	{ "ImSetSequence", LUA_OPCODE(Lua_V1, ImSetSequence) },
	{ "ImPause", LUA_OPCODE(Lua_V1, ImPause) },
	{ "ImResume", LUA_OPCODE(Lua_V1, ImResume) },
	{ "ImSetVoiceEffect", LUA_OPCODE(Lua_V1, ImSetVoiceEffect) },
	{ "LoadBundle", LUA_OPCODE(Lua_V1, LoadBundle) },
	{ "SetGamma", LUA_OPCODE(Lua_V1, SetGamma) },
	{ "SetActorWalkDominate", LUA_OPCODE(Lua_V1, SetActorWalkDominate) },
	{ "SetActorConstrain", LUA_OPCODE(Lua_V1, SetActorConstrain) },
	{ "RenderModeUser", LUA_OPCODE(Lua_V1, RenderModeUser) },
	{ "ForceRefresh", LUA_OPCODE(Lua_V1, ForceRefresh) },
	{ "DimScreen", LUA_OPCODE(Lua_V1, DimScreen) },
	{ "DimRegion", LUA_OPCODE(Lua_V1, DimRegion) },
	{ "CleanBuffer", LUA_OPCODE(Lua_V1, CleanBuffer) },
	{ "Display", LUA_OPCODE(Lua_V1, Display) },
	{ "SetSpeechMode", LUA_OPCODE(Lua_V1, SetSpeechMode) },
	{ "GetSpeechMode", LUA_OPCODE(Lua_V1, GetSpeechMode) },
	{ "SetShadowColor", LUA_OPCODE(Lua_V1, SetShadowColor) },
	{ "ActivateActorShadow", LUA_OPCODE(Lua_V1, ActivateActorShadow) },
	{ "SetActorShadowPlane", LUA_OPCODE(Lua_V1, SetActorShadowPlane) },
	{ "SetActorShadowPoint", LUA_OPCODE(Lua_V1, SetActorShadowPoint) },
	{ "SetActiveShadow", LUA_OPCODE(Lua_V1, SetActiveShadow) },
	{ "KillActorShadows", LUA_OPCODE(Lua_V1, KillActorShadows) },
	{ "AddShadowPlane", LUA_OPCODE(Lua_V1, AddShadowPlane) },
	{ "SetActorShadowValid", LUA_OPCODE(Lua_V1, SetActorShadowValid) },
	{ "FreeObjectState", LUA_OPCODE(Lua_V1, FreeObjectState) },
	{ "NewObjectState", LUA_OPCODE(Lua_V1, NewObjectState) },
	{ "SetObjectType", LUA_OPCODE(Lua_V1, SetObjectType) },
	{ "SendObjectToBack", LUA_OPCODE(Lua_V1, SendObjectToBack) },
	{ "SendObjectToFront", LUA_OPCODE(Lua_V1, SendObjectToFront) },
	{ "ActorToClean", LUA_OPCODE(Lua_V1, ActorToClean) },
	{ "FlushControls", LUA_OPCODE(Lua_V1, FlushControls) },
	{ "SetActorCollisionMode", LUA_OPCODE(Lua_V1, SetActorCollisionMode) },
	{ "SetActorCollisionScale", LUA_OPCODE(Lua_V1, SetActorCollisionScale) },
	{ "SetActorClipActive", LUA_OPCODE(Lua_V1, SetActorClipActive) },
	{ "SetActorClipPlane", LUA_OPCODE(Lua_V1, SetActorClipPlane) },
	{ "FadeOutChore", LUA_OPCODE(Lua_V1, FadeOutChore) },
	{ "FadeInChore", LUA_OPCODE(Lua_V1, FadeInChore) },
	{ "IrisDown", LUA_OPCODE(Lua_V1, IrisDown) },
	{ "IrisUp", LUA_OPCODE(Lua_V1, IrisUp) },
	{ "TextFileGetLineCount", LUA_OPCODE(Lua_V1, TextFileGetLineCount) },
	{ "TextFileGetLine", LUA_OPCODE(Lua_V1, TextFileGetLine) },
	{ "ScreenShot", LUA_OPCODE(Lua_V1, ScreenShot) },
	{ "GetSaveGameImage", LUA_OPCODE(Lua_V1, GetSaveGameImage) },
	{ "GetImage", LUA_OPCODE(Lua_V1, GetImage) },
	{ "FreeImage", LUA_OPCODE(Lua_V1, FreeImage) },
	{ "BlastImage", LUA_OPCODE(Lua_V1, BlastImage) },
	{ "BlastRect", LUA_OPCODE(Lua_V1, BlastRect) },
	{ "SubmitSaveGameData", LUA_OPCODE(Lua_V1, SubmitSaveGameData) },
	{ "GetSaveGameData", LUA_OPCODE(Lua_V1, GetSaveGameData) },
	{ "SetTextSpeed", LUA_OPCODE(Lua_V1, SetTextSpeed) },
	{ "GetTextSpeed", LUA_OPCODE(Lua_V1, GetTextSpeed) },
	{ "DetachFromResources", LUA_OPCODE(Lua_V1, DetachFromResources) },
	{ "AttachToResources", LUA_OPCODE(Lua_V1, AttachToResources) },
	{ "ActorPuckOrient", LUA_OPCODE(Lua_V1, ActorPuckOrient) },
	{ "JustLoaded", LUA_OPCODE(Lua_V1, JustLoaded) },
	{ "ResetTextures", LUA_OPCODE(Lua_V1, ResetTextures) },
	{ "ShrinkBoxes", LUA_OPCODE(Lua_V1, ShrinkBoxes) },
	{ "UnShrinkBoxes", LUA_OPCODE(Lua_V1, UnShrinkBoxes) },
	{ "GetShrinkPos", LUA_OPCODE(Lua_V1, GetShrinkPos) },
	{ "NukeResources", LUA_OPCODE(Lua_V1, NukeResources) },
	{ "SetActorInvClipNode", LUA_OPCODE(Lua_V1, SetActorInvClipNode) },
	{ "GetDiskFreeSpace", LUA_OPCODE(Lua_V1, GetDiskFreeSpace) },
	{ "SaveIMuse", LUA_OPCODE(Lua_V1, SaveIMuse) },
	{ "RestoreIMuse", LUA_OPCODE(Lua_V1, RestoreIMuse) },
	{ "GetMemoryUsage", LUA_OPCODE(Lua_V1, GetMemoryUsage) },
	{ "dofile", LUA_OPCODE(Lua_V1, new_dofile) },
};

static struct luaL_reg grimTextOpcodes[] = {
	{ "IsMessageGoing", LUA_OPCODE(Lua_V1, IsMessageGoing) },
	{ "SetSayLineDefaults", LUA_OPCODE(Lua_V1, SetSayLineDefaults) },
	{ "SetActorTalkColor", LUA_OPCODE(Lua_V1, SetActorTalkColor) },
	{ "GetActorTalkColor", LUA_OPCODE(Lua_V1, GetActorTalkColor) },
	{ "SayLine", LUA_OPCODE(Lua_V1, SayLine) },
	{ "PrintLine", LUA_OPCODE(Lua_V1, PrintLine) },
	{ "MakeTextObject", LUA_OPCODE(Lua_V1, MakeTextObject) },
	{ "GetTextObjectDimensions", LUA_OPCODE(Lua_V1, GetTextObjectDimensions) },
	{ "GetFontDimensions", LUA_OPCODE(Lua_V1, GetFontDimensions) },
	{ "ChangeTextObject", LUA_OPCODE(Lua_V1, ChangeTextObject) },
	{ "KillTextObject", LUA_OPCODE(Lua_V1, KillTextObject) },
	{ "BlastText", LUA_OPCODE(Lua_V1, BlastText) },
	{ "ExpireText", LUA_OPCODE(Lua_V1, ExpireText) },
	{ "PurgeText", LUA_OPCODE(Lua_V1, PurgeText) },
	{ "MakeColor", LUA_OPCODE(Lua_V1, MakeColor) },
	{ "GetColorComponents", LUA_OPCODE(Lua_V1, GetColorComponents) },
	{ "SetTranslationMode", LUA_OPCODE(Lua_V1, SetTranslationMode) },
	{ "GetTranslationMode", LUA_OPCODE(Lua_V1, GetTranslationMode) },
	{ "GetTextCharPosition", LUA_OPCODE(Lua_V1, GetTextCharPosition) },
	{ "LocalizeString", LUA_OPCODE(Lua_V1, LocalizeString) },
	{ "SetEmergencyFont", LUA_OPCODE(Lua_V1, SetEmergencyFont) },
	{ "SetOffscreenTextPos", LUA_OPCODE(Lua_V1, SetOffscreenTextPos) }
};

struct luaL_reg grimPrimitivesOpcodes[] = {
	{ "DrawLine", LUA_OPCODE(Lua_V1, DrawLine) },
	{ "DrawPolygon", LUA_OPCODE(Lua_V1, DrawPolygon) },
	{ "DrawRectangle", LUA_OPCODE(Lua_V1, DrawRectangle) },
	{ "ChangePrimitive", LUA_OPCODE(Lua_V1, ChangePrimitive) },
	{ "KillPrimitive", LUA_OPCODE(Lua_V1, KillPrimitive) },
	{ "PurgePrimitiveQueue", LUA_OPCODE(Lua_V1, PurgePrimitiveQueue) }
};

struct luaL_reg grimHardwareOpcodes[] = {
	{ "Is3DHardwareEnabled", LUA_OPCODE(Lua_V1, Is3DHardwareEnabled) },
	{ "GetVideoDevices", LUA_OPCODE(Lua_V1, GetVideoDevices) },
	{ "SetVideoDevices", LUA_OPCODE(Lua_V1, SetVideoDevices) },
	{ "SetHardwareState", LUA_OPCODE(Lua_V1, SetHardwareState) },
	{ "Enumerate3DDevices", LUA_OPCODE(Lua_V1, Enumerate3DDevices) },
	{ "EnumerateVideoDevices", LUA_OPCODE(Lua_V1, EnumerateVideoDevices) }
};

void Lua_V1::registerOpcodes() {
	// Register main opcodes functions
	luaL_openlib(grimMainOpcodes, ARRAYSIZE(grimMainOpcodes));

	// Register text opcodes functions
	luaL_openlib(grimTextOpcodes, ARRAYSIZE(grimTextOpcodes));

	// Register primitives opcodeEs functions
	luaL_openlib(grimPrimitivesOpcodes, ARRAYSIZE(grimPrimitivesOpcodes));

	// Register hardware opcode functions
	luaL_openlib(grimHardwareOpcodes, ARRAYSIZE(grimHardwareOpcodes));

	LuaBase::registerOpcodes();
}

void Lua_V1::boot() {
	// The default value of these globals, defined in _controls.lua, is 256, 257, 258, 259.
	// These values clash with the numpad 0, 1, 2 and 3 keycodes, so we set them here.
	lua_pushnumber(KEYCODE_JOY1_HLEFT);
	lua_setglobal("JOYSTICK_X_LEFT");

	lua_pushnumber(KEYCODE_JOY1_HRIGHT);
	lua_setglobal("JOYSTICK_X_RIGHT");

	lua_pushnumber(KEYCODE_JOY1_HUP);
	lua_setglobal("JOYSTICK_Y_UP");

	lua_pushnumber(KEYCODE_JOY1_HDOWN);
	lua_setglobal("JOYSTICK_Y_DOWN");

	LuaBase::boot();
}

void Lua_V1::postRestoreHandle() {

	if (g_grim->getGameType() == GType_GRIM) {
		lua_beginblock();
		// Set the developerMode, since the save contains the value of
		// the installation it was made with.
		lua_pushobject(lua_getglobal("developerMode"));
		bool devMode = g_registry->getBool("good_times");
		pushbool(devMode);
		lua_setglobal("developerMode");
		lua_endblock();
	}

	// Starting a movie calls the function 'music_state.pause()', which saves the current sfx volume to a temp
	// variable and sets it to 0. When the movie finishes 'music_state.unpause()' will be called, which reads
	// the volume from the temp variable and sets it. But if we have modified the sfx volume in the options
	// and than load a savegame made during a movie, at the end of the movie the temp variable will have the
	// old value. So here we call 'music_state.pause()' again, so that it can set the right value to the
	// temp variable.
	lua_beginblock();
	lua_Object o = lua_getglobal("music_state");
	if (!lua_isnil(o)) {
		lua_pushobject(o);
		lua_pushstring("paused");
		if (!lua_isnil(lua_gettable())) {
			lua_pushobject(o);
			lua_pushstring("paused");
			pushbool(false);
			lua_settable();

			lua_pushobject(o);
			lua_pushstring("pause");
			lua_Object func = lua_gettable();
			lua_pushobject(o);
			lua_callfunction(func);
		}
	}
	lua_endblock();
}

} // end of namespace Grim
