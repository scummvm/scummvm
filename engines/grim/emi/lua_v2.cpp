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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/endian.h"
#include "common/foreach.h"
#include "common/savefile.h"

#include "engines/grim/emi/lua_v2.h"
#include "engines/grim/lua/lauxlib.h"

#include "engines/grim/resource.h"
#include "engines/grim/set.h"
#include "engines/grim/grim.h"
#include "engines/grim/gfx_base.h"
#include "engines/grim/font.h"

#include "engines/grim/emi/emi.h"

#include "engines/grim/movie/movie.h"

namespace Grim {

void Lua_V2::UndimAll() {
	g_driver->setDimLevel(0);
	warning("Lua_V2::UndimAll: stub");
}

void Lua_V2::UndimRegion() {
	lua_Object regionObj = lua_getparam(1);

	if (lua_isnumber(regionObj)) {
		int region = (int)lua_getnumber(regionObj);
		// FIXME func(region);
		warning("Lua_V2::UndimRegion: region: %d", region);
	} else {
		lua_pushnil();
		// HACK: The demo uses this to undim the intro-screen.
		// thus UndimRegion(nil) might mean UndimScreen.
		g_driver->setDimLevel(0);
	}
}

void Lua_V2::DimScreen() {
	lua_Object dimObj = lua_getparam(1);
	float dim = 0.6999f;

	if (lua_isnumber(dimObj))
		dim = lua_getnumber(dimObj);

	g_driver->setDimLevel(dim);
	// FIXME func(dim);
	warning("Lua_V2::DimScreen: dim: %f", dim);
}

void Lua_V2::MakeCurrentSetup() {
	lua_Object setupObj = lua_getparam(1);
	if (lua_isnumber(setupObj)) {
		int num = (int)lua_getnumber(setupObj);
		g_grim->makeCurrentSetup(num);
	} else if (lua_isstring(setupObj)) {
		const char *setupName = lua_getstring(setupObj);
		error("Lua_V2::MakeCurrentSetup: Not implemented case: setup: %s", setupName);
	}
}

void Lua_V2::LockBackground() {
	lua_Object filenameObj = lua_getparam(1);

	if (!lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}
	const char *filename = lua_getstring(filenameObj);
	warning("Lua_V2::LockBackground, filename: %s", filename);
	// FIXME: implement missing rest part of code
}

void Lua_V2::UnLockBackground() {
	lua_Object filenameObj = lua_getparam(1);

	if (!lua_isstring(filenameObj)) {
		lua_pushnil();
		return;
	}

	const char *filename = lua_getstring(filenameObj);
	// FIXME: implement missin code
	warning("Lua_V2::UnLockBackground: stub, filename: %s", filename);
}

void Lua_V2::MakeScreenTextures() {
	lua_Object indexObj = lua_getparam(1);

	if (!lua_isnil(indexObj) && lua_isnumber(indexObj)) {
		/*int index = (int)lua_getnumber(indexObj);*/
		// The index does not seem to matter

		g_driver->createSpecialtyTextures();
		lua_pushnumber(1.0);
	} else {
		lua_pushnil();
	}
}

void Lua_V2::ClearSpecialtyTexture() {
	//This seems to be used in the save/load menu
	//Not sure why the specialty textures need to be cleared.
	warning("Lua_V2::ClearSpecialtyTexture: stub");
}

void Lua_V2::LoadBundle() {
	lua_Object paramObj = lua_getparam(1);
	if (lua_isstring(paramObj) || lua_isnil(paramObj)) {
		const char *name = lua_getstring(paramObj);
		// FIXME: implement missing function
/*		if (!func(name))
			lua_pushnil();
		else*/
			lua_pushnumber(1.0);
		warning("Lua_V2::LoadBundle: stub, name: %s", name);
	}
}

void Lua_V2::AreWeInternational() {
	if (g_grim->getGameLanguage() != Common::EN_ANY)
		lua_pushnumber(1.0);
}

void Lua_V2::GetCPUSpeed() {
	lua_pushnumber(500); // anything above 333 make best configuration
}

// This should be correct, judging by the Demo
// the only real difference from L1 is the lack of looping
void Lua_V2::StartMovie() {
	lua_Object name = lua_getparam(1);
	if (!lua_isstring(name)) {
		lua_pushnil();
		return;
	}
	Lua_V1::CleanBuffer();

	GrimEngine::EngineMode prevEngineMode = g_grim->getMode();
	g_grim->setMode(GrimEngine::SmushMode);
	g_grim->setMovieSubtitle(NULL);
	bool result = g_movie->play(lua_getstring(name), false, 0, 0);
	if (!result)
		g_grim->setMode(prevEngineMode);
	pushbool(result);

	// The following line causes issues after 9547a9b61674546077301bf09f89a2d120046d8e
	//g_grim->setMode(GrimEngine::SmushMode);
}

void Lua_V2::EscapeMovie() {
	g_movie->stop();
}

void Lua_V2::IsMoviePlaying() {
	pushbool(g_movie->isPlaying());
}

void Lua_V2::SetActiveCD() {
	lua_Object cdObj = lua_getparam(1);
	int cd = (int)lua_getnumber(cdObj);

	if (cd == 1 || cd == 2) {
		warning("Lua_V2::GetActiveCD: set to CD: %d", cd);
		// FIXME
		lua_pushnumber(1.0);
	}
}

void Lua_V2::GetActiveCD() {
	// FIXME: return current CD number 1 or 2, original can also avoid push any numer
	warning("Lua_V2::GetActiveCD: return const CD 1");
	lua_pushnumber(1);
}

void Lua_V2::PurgeText() {
	TextObject::getPool().deleteObjects();
}

void Lua_V2::GetFontDimensions() {
	lua_Object fontObj = lua_getparam(1);
	if (!lua_isstring(fontObj))
		return;

	const char *fontName = lua_getstring(fontObj);

	Font *font = NULL;
	foreach (Font *f, Font::getPool()) {
		if (f->getFilename() == fontName) {
			font = f;
		}
	}
	if (!font) {
		font = g_resourceloader->loadFont(fontName);
	}
	if (font) {
		int32 h = font->getBaseOffsetY();
		int32 w = font->getCharWidth('w');
		warning("Lua_V2::GetFontDimensions for font '%s': returns %d,%d", fontName, h, w);
		lua_pushnumber(w);
		lua_pushnumber(h);
	} else {
		warning("Lua_V2::GetFontDimensions for font '%s': returns 0,0", fontName);
		lua_pushnumber(0.f);
		lua_pushnumber(0.f);
	}
}

void Lua_V2::GetTextCharPosition() {
	lua_Object textObj = lua_getparam(1);
	lua_Object posObj = lua_getparam(2);
	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		int pos = (int)lua_getnumber(posObj);
		float textPos = textObject->getTextCharPosition(pos);
		lua_pushnumber(textPos / 320.f);
	}
}

void Lua_V2::GetTextObjectDimensions() {
	lua_Object textObj = lua_getparam(1);

	if (lua_isuserdata(textObj) && lua_tag(textObj) == MKTAG('T', 'E', 'X', 'T')) {
		TextObject *textObject = gettextobject(textObj);
		lua_pushnumber(textObject->getBitmapWidth() / 320.f);
		lua_pushnumber(textObject->getBitmapHeight() / 240.f);
	}
}

void Lua_V2::ToggleOverworld() {
	lua_Object boolObj = lua_getparam(1);
	bool backToNormal = (lua_isnil(boolObj) == 0);
	if (backToNormal) {
		GrimEngine::EngineMode previous = g_grim->getPreviousMode();
		g_grim->setPreviousMode(GrimEngine::OverworldMode);
		// HACK: ToggleOverworld is only called after we load a save game.
		//       However, the engine saved PreviousMode as OverworldMode.
		//       Reset it to normal here.
		if (previous == GrimEngine::OverworldMode)
			previous = GrimEngine::NormalMode;
		g_grim->setMode(previous);
	} else {
		GrimEngine::EngineMode previous = g_grim->getMode();
		g_grim->setPreviousMode(previous);
		g_grim->setMode(GrimEngine::OverworldMode);
	}
}

void Lua_V2::ClearOverworld() {
	warning("Lua_V2::ClearOverworld: implement opcode");
}

void Lua_V2::ScreenshotForSavegame() {
	warning("Lua_V2::ScreenshotForSavegame: implement opcode");
}

void Lua_V2::EngineDisplay() {
	// dummy
}

void Lua_V2::Display() {
	// dummy
}

void Lua_V2::RenderModeUser() {
	lua_Object param1 = lua_getparam(1);
	g_movie->pause(!lua_isnil(param1));
}

void Lua_V2::GetCameraPosition() {
	Set::Setup *setup = g_grim->getCurrSet()->getCurrSetup();
	lua_pushnumber(setup->_pos.x());
	lua_pushnumber(setup->_pos.y());
	lua_pushnumber(setup->_pos.z());
}

void Lua_V2::GetCameraYaw() {
	warning("Lua_V2::GetCameraYaw: implement opcode, just returns 0");
	lua_pushnumber(0);
}

void Lua_V2::GetCameraRoll() {
	warning("Lua_V2::GetCameraRoll: implement opcode, just returns 0");
	lua_pushnumber(0);
}

void Lua_V2::PushText() {
	Common::List<TextObject *> *textobjects = new Common::List<TextObject *>;
	TextObject::Pool::iterator it = TextObject::getPool().begin();
	for (; it != TextObject::getPool().end(); ++it) {
		textobjects->push_back(*it);
		TextObject::getPool().removeObject((*it)->getId());
	}
	g_emi->pushText(textobjects);
}

void Lua_V2::PopText() {
	Common::List<TextObject *> *textobjects = g_emi->popText();
	Common::List<TextObject *>::iterator it = textobjects->begin();
	for (; it != textobjects->end(); ++it) {
		TextObject::getPool().addObject(*it);
	}
	delete textobjects;
}

void Lua_V2::GetSectorName() {
	lua_Object xObj = lua_getparam(1);
	lua_Object yObj = lua_getparam(2);
	lua_Object zObj = lua_getparam(3);

	if (!lua_isnumber(xObj) || !lua_isnumber(yObj) || !lua_isnumber(zObj)) {
		lua_pushnil();
		return;
	}
	float x, y, z;
	x = lua_getnumber(xObj);
	y = lua_getnumber(yObj);
	z = lua_getnumber(zObj);

	Math::Vector3d pos(x, y, z);
	Set *set = g_grim->getCurrSet();
	Sector *sector = set->findPointSector(pos, Sector::NoneType);
	if (sector) {
		lua_pushstring(sector->getName().c_str());
	}
}

void Lua_V2::GammaEnabled() {
	warning("Lua_V2::GammaEnabled: implement opcode, pushing nil");
	lua_pushnil();
}

void Lua_V2::FileFindFirst() {
	lua_Object extObj = lua_getparam(1);
	if (!lua_isstring(extObj)) {
		lua_pushnil();
		return;
	}

	FileFindDispose();

	const char *extension = lua_getstring(extObj);
	if (0 == strncmp(extension, "Saves/", 6))
		extension += 6;

	// _menus.lua: saveload_menu.get_file_list searches for *.gsv.
	// This avoids conflicts with grim saves.
	if (0 == strcmp(extension, "*.gsv"))
		extension = "efmi???.gsv";

	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	g_grim->_listFiles = saveFileMan->listSavefiles(extension);
	Common::sort(g_grim->_listFiles.begin(), g_grim->_listFiles.end());
	g_grim->_listFilesIter = g_grim->_listFiles.begin();

	if (g_grim->_listFilesIter == g_grim->_listFiles.end())
		lua_pushnil();
	else
		FileFindNext();
}

void Lua_V2::ThumbnailFromFile() {
	lua_Object texIdObj = lua_getparam(1);
	lua_Object filenameObj = lua_getparam(2);

	if (!lua_isnumber(texIdObj) || !lua_isstring(filenameObj))
		return;

	warning("Lua_V2::ThumbnailFromFile: implement opcode, pushing true");
	pushbool(true);
}

void Lua_V2::GetMemoryCardId() {
	// 0 - No mem card
	lua_pushnumber(0);
	warning("GetMemoryCardId: Currently just returning 0");
}

void Lua_V2::LocalizeString() {
	char msgId[50], buf[1000];
	lua_Object strObj = lua_getparam(1);

	if (lua_isstring(strObj)) {
		const char *str = lua_getstring(strObj);
		Common::String msg = parseMsgText(str, msgId);
		sprintf(buf, "/%s/%s", msgId, msg.c_str());
		str = buf;

		lua_pushstring(str);
	}
}

void Lua_V2::OverWorldToScreen() { // TODO
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	lua_Object param3 = lua_getparam(3);

	float x = 0, y = 0, z = 0;
	if (!lua_isnumber(param1) || !lua_isnumber(param2) || !lua_isnumber(param3)) {
		error("Param not a number for OverWorldToScreen");
	} else {
		x = lua_getnumber(param1);
		y = lua_getnumber(param2);
		z = lua_getnumber(param3);
	}

	warning("Stub function: OverWorldToScreen(%f, %f, %f) - returning 0,0", x, y, z);

	lua_pushnumber(0);
	lua_pushnumber(0);
}

void Lua_V2::NewLayer() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	lua_Object param3 = lua_getparam(3);

	const char *til = NULL;
	int layer = 0, zero = 0;
	if (lua_isstring(param1) && lua_isnumber(param2) && lua_isnumber(param3)) {
		til = lua_getstring(param1);
		layer = (int)lua_getnumber(param2);

		//This one is always specified, but also always 0...
		zero = (int)lua_getnumber(param2);

		warning("Stub function: NewLayer(%s, %d, %d) - returning 0", til, layer, zero);

		// Need to return something that can be looked up later
		lua_pushusertag(0, MKTAG('L','A','Y','R'));
	}
}

void Lua_V2::FreeLayer() {
	lua_Object param1 = lua_getparam(1);
	if (lua_isuserdata(param1) && lua_tag(param1) == MKTAG('L','A','Y','R')) {
		int layer = (int)lua_getuserdata(param1);
		warning("Stub function: FreeLayer(%d)", layer);
	}
}

void Lua_V2::AdvanceLayerFrame() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	if (lua_isuserdata(param1) && lua_tag(param1) == MKTAG('L','A','Y','R') && lua_isnumber(param2)) {
		int layer = (int)lua_getuserdata(param1);
		int one = (int)lua_getnumber(param2);
		warning("Stub function: AdvanceLayerFrame(%d, %d)", layer, one);
	}
}

void Lua_V2::SetLayerFrame() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	if (lua_isuserdata(param1) && lua_tag(param1) == MKTAG('L','A','Y','R') && lua_isnumber(param2)) {
		int layer = (int)lua_getuserdata(param1);
		int one = (int)lua_getnumber(param2);
		warning("Stub function: SetLayerFrame(%d, %d)", layer, one);
	}
}

void Lua_V2::SetLayerSortOrder() {
	lua_Object param1 = lua_getparam(1);
	lua_Object param2 = lua_getparam(2);
	if (lua_isuserdata(param1) && lua_tag(param1) != MKTAG('L','A','Y','R') && lua_isnumber(param2)) {
		int layer = (int)lua_getuserdata(param1);
		int sortorder = (int)lua_getnumber(param2);
		warning("Stub function: SetLayerSortOrder(%d, %d)", layer, sortorder);
	}
}

// Stub function for builtin functions not yet implemented
/*static void stubWarning(const char *funcName) {
    warning("Stub function: %s", funcName);
}*/

static void stubError(const char *funcName) {
	error("Stub function: %s", funcName);
}

#define STUB_FUNC(name) void name() { stubWarning(#name); }
#define STUB_FUNC2(name) void name() { stubError(#name); }

// Opcodes more or less differ to Grim Lua_V1::* LUA_OPCODEs
// STUB_FUNC2(Lua_V2::SetActorWalkChore)
// STUB_FUNC2(Lua_V2::SetActorTurnChores)
// STUB_FUNC2(Lua_V2::SetActorRestChore)
// STUB_FUNC2(Lua_V2::SetActorMumblechore)
// STUB_FUNC2(Lua_V2::SetActorTalkChore)
// STUB_FUNC2(Lua_V2::WalkActorVector)
// STUB_FUNC2(Lua_V2::SetActorLookRate)
// STUB_FUNC2(Lua_V2::GetActorLookRate)
// STUB_FUNC2(Lua_V2::GetVisibleThings)
// STUB_FUNC2(Lua_V2::GetActorRot)
// STUB_FUNC2(Lua_V2::LockSet)
// STUB_FUNC2(Lua_V2::UnLockSet)
// STUB_FUNC2(Lua_V2::PlaySound)
// STUB_FUNC2(Lua_V2::IsSoundPlaying)
// STUB_FUNC2(Lua_V2::MakeSectorActive)
// STUB_FUNC2(Lua_V2::TurnActorTo)
// STUB_FUNC2(Lua_V2::GetAngleBetweenActors)
// STUB_FUNC2(Lua_V2::ImStartSound)
// STUB_FUNC2(Lua_V2::ImGetSfxVol)
// STUB_FUNC2(Lua_V2::ImGetVoiceVol)
// STUB_FUNC2(Lua_V2::ImGetMusicVol)
// STUB_FUNC2(Lua_V2::ImSetSequence)
// STUB_FUNC2(Lua_V2::ChangeTextObject)
// STUB_FUNC2(Lua_V2::GetTextCharPosition)
// STUB_FUNC2(Lua_V2::SetOffscreenTextPos)
// STUB_FUNC2(Lua_V2::FadeInChore)
// STUB_FUNC2(Lua_V2::FadeOutChore)
// STUB_FUNC2(Lua_V2::SetLightPosition)
// STUB_FUNC2(Lua_V2::GetAngleBetweenVectors)
// STUB_FUNC2(Lua_V2::IsPointInSector)
// STUB_FUNC2(Lua_V2::ThumbnailFromFile)

// Stubbed functions with semi-known arguments:
// TODO: Verify and implement these: (And add type-checking), also rename params
void Lua_V2::GetCameraPitch() {
	error("Lua_V2::GetCameraPitch() - TODO: Implement opcode");
}

// No idea about parameter types for these three, presumably float
void Lua_V2::PitchCamera() {
	lua_Object param1 = lua_getparam(1);

	if (!lua_isnumber(param1))
		error("Lua_V2::PitchCamera - Unknown parameters");

	float floatValue = lua_getnumber(param1);
	error("Lua_V2::PitchCamera(%f) - TODO: Implement opcode", floatValue);
}

void Lua_V2::RollCamera() {
	lua_Object param1 = lua_getparam(1);

	if (!lua_isnumber(param1))
		error("Lua_V2::RollCamera - Unknown parameters");

	float floatValue = lua_getnumber(param1);
	error("Lua_V2::RollCamera(%f) - TODO: Implement opcode", floatValue);
}

void Lua_V2::YawCamera() {
	lua_Object param1 = lua_getparam(1);

	if (!lua_isnumber(param1))
		error("Lua_V2::YawCamera - Unknown parameters");

	float floatValue = lua_getnumber(param1);
	error("Lua_V2::YawCamera(%f) - TODO: Implement opcode", floatValue);
}

void Lua_V2::NukeAllScriptLocks() {
	error("Lua_V2::NukeAllScriptLocks() - TODO: Implement opcode");
}

void Lua_V2::FRUTEY_Begin() {
	lua_Object param1 = lua_getparam(1);

	if (!lua_isstring(param1))
		error("Lua_V2::FRUTEY_Begin - Unknown parameters");

	const char *paramText = lua_getstring(param1);
	error("Lua_V2::FRUTEY_Begin(%s) - TODO: Implement opcode", paramText);
}

void Lua_V2::FRUTEY_End() {
	error("Lua_V2::FRUTEY_End() - TODO: Implement opcode");
}

// Monkey specific LUA_OPCODEs
STUB_FUNC2(Lua_V2::ToggleDebugDraw)
STUB_FUNC2(Lua_V2::ToggleDrawCameras)
STUB_FUNC2(Lua_V2::ToggleDrawLights)
STUB_FUNC2(Lua_V2::ToggleDrawSectors)
STUB_FUNC2(Lua_V2::ToggleDrawBBoxes)
STUB_FUNC2(Lua_V2::ToggleDrawFPS)
STUB_FUNC2(Lua_V2::ToggleDrawPerformance)
STUB_FUNC2(Lua_V2::ToggleDrawActorStats)
STUB_FUNC2(Lua_V2::SectEditSelect)
STUB_FUNC2(Lua_V2::SectEditPlace)
STUB_FUNC2(Lua_V2::SectEditDelete)
STUB_FUNC2(Lua_V2::SectEditInsert)
STUB_FUNC2(Lua_V2::SectEditSortAdd)
STUB_FUNC2(Lua_V2::SectEditForgetIt)

struct luaL_reg monkeyMainOpcodes[] = {
	// Monkey specific LUA_OPCODEs:
	{ "ScreenshotForSavegame", LUA_OPCODE(Lua_V2, ScreenshotForSavegame) },
	{ "GetActorWorldPos", LUA_OPCODE(Lua_V2, GetActorWorldPos) },
	{ "SetActiveCD", LUA_OPCODE(Lua_V2, SetActiveCD) },
	{ "GetActiveCD", LUA_OPCODE(Lua_V2, GetActiveCD) },
	{ "AreWeInternational", LUA_OPCODE(Lua_V2, AreWeInternational) },
	{ "MakeScreenTextures", LUA_OPCODE(Lua_V2, MakeScreenTextures) },
	{ "ThumbnailFromFile", LUA_OPCODE(Lua_V2, ThumbnailFromFile) },
	{ "ClearSpecialtyTexture", LUA_OPCODE(Lua_V2, ClearSpecialtyTexture) },
	{ "UnloadActor", LUA_OPCODE(Lua_V2, UnloadActor) },
	{ "PutActorInOverworld", LUA_OPCODE(Lua_V2, PutActorInOverworld) },
	{ "RemoveActorFromOverworld", LUA_OPCODE(Lua_V2, RemoveActorFromOverworld) },
	{ "ClearOverworld", LUA_OPCODE(Lua_V2, ClearOverworld) },
	{ "ToggleOverworld", LUA_OPCODE(Lua_V2, ToggleOverworld) },
	{ "ActorStopMoving", LUA_OPCODE(Lua_V2, ActorStopMoving) },
	{ "SetActorFOV", LUA_OPCODE(Lua_V2, SetActorFOV) },
	{ "SetActorLighting", LUA_OPCODE(Lua_V2, SetActorLighting) },
	{ "SetActorHeadLimits", LUA_OPCODE(Lua_V2, SetActorHeadLimits) },
	{ "ActorActivateShadow", LUA_OPCODE(Lua_V2, ActorActivateShadow) },
	{ "EnableActorPuck", LUA_OPCODE(Lua_V2, EnableActorPuck) },
	{ "SetActorGlobalAlpha", LUA_OPCODE(Lua_V2, SetActorGlobalAlpha) },
	{ "SetActorLocalAlpha", LUA_OPCODE(Lua_V2, SetActorLocalAlpha) },
	{ "SetActorSortOrder", LUA_OPCODE(Lua_V2, SetActorSortOrder) },
	{ "GetActorSortOrder", LUA_OPCODE(Lua_V2, GetActorSortOrder) },
	{ "AttachActor", LUA_OPCODE(Lua_V2, AttachActor) },
	{ "DetachActor", LUA_OPCODE(Lua_V2, DetachActor) },
	{ "IsChoreValid", LUA_OPCODE(Lua_V2, IsChoreValid) },
	{ "IsChorePlaying", LUA_OPCODE(Lua_V2, IsChorePlaying) },
	{ "IsChoreLooping", LUA_OPCODE(Lua_V2, IsChoreLooping) },
	{ "SetChoreLooping", LUA_OPCODE(Lua_V2, SetChoreLooping) },
	{ "StopActorChores", LUA_OPCODE(Lua_V2, StopActorChores) },
	{ "PlayChore", LUA_OPCODE(Lua_V2, PlayChore) },
	{ "StopChore", LUA_OPCODE(Lua_V2, StopChore) },
	{ "PauseChore", LUA_OPCODE(Lua_V2, PauseChore) },
	{ "AdvanceChore", LUA_OPCODE(Lua_V2, AdvanceChore) },
	{ "CompleteChore", LUA_OPCODE(Lua_V2, CompleteChore) },
	{ "LockChore", LUA_OPCODE(Lua_V2, LockChore) },
	{ "UnlockChore", LUA_OPCODE(Lua_V2, UnlockChore) },
	{ "LockChoreSet", LUA_OPCODE(Lua_V2, LockChoreSet) },
	{ "UnlockChoreSet", LUA_OPCODE(Lua_V2, UnlockChoreSet) },
	{ "LockBackground", LUA_OPCODE(Lua_V2, LockBackground) },
	{ "UnLockBackground", LUA_OPCODE(Lua_V2, UnLockBackground) },
	{ "EscapeMovie", LUA_OPCODE(Lua_V2, EscapeMovie) },
	{ "StopAllSounds", LUA_OPCODE(Lua_V2, StopAllSounds) },
	{ "LoadSound", LUA_OPCODE(Lua_V2, LoadSound) },
	{ "FreeSound", LUA_OPCODE(Lua_V2, FreeSound) },
	{ "PlayLoadedSound", LUA_OPCODE(Lua_V2, PlayLoadedSound) },
	{ "StopSound", LUA_OPCODE(Lua_V2, StopSound) },
	{ "SetGroupVolume", LUA_OPCODE(Lua_V2, SetGroupVolume) },
	{ "GetSoundVolume", LUA_OPCODE(Lua_V2, GetSoundVolume) },
	{ "SetSoundVolume", LUA_OPCODE(Lua_V2, SetSoundVolume) },
	{ "EnableAudioGroup", LUA_OPCODE(Lua_V2, EnableAudioGroup) },
	{ "EnableVoiceFX", LUA_OPCODE(Lua_V2, EnableVoiceFX) },
	{ "PlaySoundFrom", LUA_OPCODE(Lua_V2, PlaySoundFrom) },
	{ "PlayLoadedSoundFrom", LUA_OPCODE(Lua_V2, PlayLoadedSoundFrom) },
	{ "SetReverb", LUA_OPCODE(Lua_V2, SetReverb) },
	{ "UpdateSoundPosition", LUA_OPCODE(Lua_V2, UpdateSoundPosition) },
	{ "ImSelectSet", LUA_OPCODE(Lua_V2, ImSelectSet) },
	{ "ImStateHasLooped", LUA_OPCODE(Lua_V2, ImStateHasLooped) },
	{ "ImStateHasEnded", LUA_OPCODE(Lua_V2, ImStateHasEnded) },
	{ "ImPushState", LUA_OPCODE(Lua_V2, ImPushState) },
	{ "ImPopState", LUA_OPCODE(Lua_V2, ImPopState) },
	{ "ImFlushStack", LUA_OPCODE(Lua_V2, ImFlushStack) },
	{ "ImGetMillisecondPosition", LUA_OPCODE(Lua_V2, ImGetMillisecondPosition) },
	{ "GetSectorName", LUA_OPCODE(Lua_V2, GetSectorName) },
	{ "GetCameraYaw", LUA_OPCODE(Lua_V2, GetCameraYaw) },
	{ "YawCamera", LUA_OPCODE(Lua_V2, YawCamera) },
	{ "GetCameraPitch", LUA_OPCODE(Lua_V2, GetCameraPitch) },
	{ "PitchCamera", LUA_OPCODE(Lua_V2, PitchCamera) },
	{ "RollCamera", LUA_OPCODE(Lua_V2, RollCamera) },
	{ "UndimAll", LUA_OPCODE(Lua_V2, UndimAll) },
	{ "UndimRegion", LUA_OPCODE(Lua_V2, UndimRegion) },
	{ "GetCPUSpeed", LUA_OPCODE(Lua_V2, GetCPUSpeed) },
	{ "NewLayer", LUA_OPCODE(Lua_V2, NewLayer) },
	{ "FreeLayer", LUA_OPCODE(Lua_V2, FreeLayer) },
	{ "SetLayerSortOrder", LUA_OPCODE(Lua_V2, SetLayerSortOrder) },
	{ "SetLayerFrame", LUA_OPCODE(Lua_V2, SetLayerFrame) },
	{ "AdvanceLayerFrame", LUA_OPCODE(Lua_V2, AdvanceLayerFrame) },
	{ "PushText", LUA_OPCODE(Lua_V2, PushText) },
	{ "PopText", LUA_OPCODE(Lua_V2, PopText) },
	{ "NukeAllScriptLocks", LUA_OPCODE(Lua_V2, NukeAllScriptLocks) },
	{ "ToggleDebugDraw", LUA_OPCODE(Lua_V2, ToggleDebugDraw) },
	{ "ToggleDrawCameras", LUA_OPCODE(Lua_V2, ToggleDrawCameras) },
	{ "ToggleDrawLights", LUA_OPCODE(Lua_V2, ToggleDrawLights) },
	{ "ToggleDrawSectors", LUA_OPCODE(Lua_V2, ToggleDrawSectors) },
	{ "ToggleDrawBBoxes", LUA_OPCODE(Lua_V2, ToggleDrawBBoxes) },
	{ "ToggleDrawFPS", LUA_OPCODE(Lua_V2, ToggleDrawFPS) },
	{ "ToggleDrawPerformance", LUA_OPCODE(Lua_V2, ToggleDrawPerformance) },
	{ "ToggleDrawActorStats", LUA_OPCODE(Lua_V2, ToggleDrawActorStats) },
	{ "SectEditSelect", LUA_OPCODE(Lua_V2, SectEditSelect) },
	{ "SectEditPlace", LUA_OPCODE(Lua_V2, SectEditPlace) },
	{ "SectEditDelete", LUA_OPCODE(Lua_V2, SectEditDelete) },
	{ "SectEditInsert", LUA_OPCODE(Lua_V2, SectEditInsert) },
	{ "SectEditSortAdd", LUA_OPCODE(Lua_V2, SectEditSortAdd) },
	{ "SectEditForgetIt", LUA_OPCODE(Lua_V2, SectEditForgetIt) },
	{ "GammaEnabled", LUA_OPCODE(Lua_V2, GammaEnabled) },
	{ "FRUTEY_Begin", LUA_OPCODE(Lua_V2, FRUTEY_Begin) },
	{ "FRUTEY_End", LUA_OPCODE(Lua_V2, FRUTEY_End) },
	{ "LocalizeString", LUA_OPCODE(Lua_V2, LocalizeString) },
// PS2:
	{ "GetMemoryCardId", LUA_OPCODE(Lua_V2, GetMemoryCardId) },
	{ "OverWorldToScreen", LUA_OPCODE(Lua_V2, OverWorldToScreen) }
};

void Lua_V2::registerOpcodes() {
	Lua_V1::registerOpcodes();

	// Register main opcodes functions
	luaL_openlib(monkeyMainOpcodes, ARRAYSIZE(monkeyMainOpcodes));
}

} // end of namespace Grim
