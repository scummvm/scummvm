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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/callbackregistry.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"
#include "sword25/script/luacallback.h"
#include "sword25/math/vertex.h"

#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/renderobject.h"
#include "sword25/gfx/bitmap.h"
#include "sword25/gfx/animation.h"
#include "sword25/gfx/panel.h"
#include "sword25/gfx/text.h"
#include "sword25/gfx/animationtemplate.h"
#include "sword25/gfx/animationtemplateregistry.h"

namespace Sword25 {

#define BS_LOG_PREFIX "GRAPHICENGINE"

// -----------------------------------------------------------------------------
// Callback-Objekte
// -----------------------------------------------------------------------------

static bool AnimationDeleteCallback(uint Data);
static bool AnimationActionCallback(uint Data);
static bool AnimationLoopPointCallback(uint Data);

namespace {
// -------------------------------------------------------------------------

class ActionCallback : public LuaCallback {
public:
	ActionCallback(lua_State *L) : LuaCallback(L) {};

	Common::String Action;

protected:
	virtual int PreFunctionInvokation(lua_State *L) {
		lua_pushstring(L, Action.c_str());
		return 1;
	}
};

Common::ScopedPtr<LuaCallback> LoopPointCallbackPtr;
Common::ScopedPtr<ActionCallback> ActionCallbackPtr;

// -------------------------------------------------------------------------

struct CallbackfunctionRegisterer {
	CallbackfunctionRegisterer() {
		CallbackRegistry::getInstance().registerCallbackFunction("LuaLoopPointCB", (void ( *)(int))AnimationLoopPointCallback);
		CallbackRegistry::getInstance().registerCallbackFunction("LuaActionCB", (void ( *)(int))AnimationActionCallback);
		CallbackRegistry::getInstance().registerCallbackFunction("LuaDeleteCB", (void ( *)(int))AnimationDeleteCallback);
	}
};
static CallbackfunctionRegisterer Instance;
}

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// Die Strings werden als #defines definiert um Stringkomposition zur Compilezeit zu ermöglichen.
#define RENDEROBJECT_CLASS_NAME "Gfx.RenderObject"
#define BITMAP_CLASS_NAME "Gfx.Bitmap"
#define PANEL_CLASS_NAME "Gfx.Panel"
#define TEXT_CLASS_NAME "Gfx.Text"
#define ANIMATION_CLASS_NAME "Gfx.Animation"
#define ANIMATION_TEMPLATE_CLASS_NAME "Gfx.AnimationTemplate"
static const char *GFX_LIBRARY_NAME = "Gfx";

// -----------------------------------------------------------------------------

// Wie luaL_checkudata, nur ohne dass kein Fehler erzeugt wird.
static void *my_checkudata(lua_State *L, int ud, const char *tname) {
	int top = lua_gettop(L);

	void *p = lua_touserdata(L, ud);
	if (p != NULL) { /* value is a userdata? */
		if (lua_getmetatable(L, ud)) { /* does it have a metatable? */
			// lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			LuaBindhelper::getMetatable(L, tname);
			if (lua_rawequal(L, -1, -2)) { /* does it have the correct mt? */
				lua_settop(L, top);
				return p;
			}
		}
	}

	lua_settop(L, top);
	return NULL;
}

// -----------------------------------------------------------------------------

static void NewUintUserData(lua_State *L, uint Value) {
	void *UserData = lua_newuserdata(L, sizeof(Value));
	memcpy(UserData, &Value, sizeof(Value));
}

// -----------------------------------------------------------------------------

static AnimationTemplate *CheckAnimationTemplate(lua_State *L, int idx = 1) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.AnimationTemplate
	uint AnimationTemplateHandle;
	if ((AnimationTemplateHandle = *reinterpret_cast<uint *>(my_checkudata(L, idx, ANIMATION_TEMPLATE_CLASS_NAME))) != 0) {
		AnimationTemplate *AnimationTemplatePtr = AnimationTemplateRegistry::getInstance().resolveHandle(AnimationTemplateHandle);
		if (!AnimationTemplatePtr)
			luaL_error(L, "The animation template with the handle %d does no longer exist.", AnimationTemplateHandle);
		return AnimationTemplatePtr;
	} else {
		luaL_argcheck(L, 0, idx, "'" ANIMATION_TEMPLATE_CLASS_NAME "' expected");
		return 0;
	}
}


// -----------------------------------------------------------------------------

static int NewAnimationTemplate(lua_State *L) {
	uint AnimationTemplateHandle = AnimationTemplate::create(luaL_checkstring(L, 1));
	AnimationTemplate *AnimationTemplatePtr = AnimationTemplateRegistry::getInstance().resolveHandle(AnimationTemplateHandle);
	if (AnimationTemplatePtr && AnimationTemplatePtr->isValid()) {
		NewUintUserData(L, AnimationTemplateHandle);
		//luaL_getmetatable(L, ANIMATION_TEMPLATE_CLASS_NAME);
		LuaBindhelper::getMetatable(L, ANIMATION_TEMPLATE_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

// -----------------------------------------------------------------------------

static int AT_AddFrame(lua_State *L) {
	AnimationTemplate *pAT = CheckAnimationTemplate(L);
	pAT->addFrame(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int AT_SetFrame(lua_State *L) {
	AnimationTemplate *pAT = CheckAnimationTemplate(L);
	pAT->setFrame(static_cast<int>(luaL_checknumber(L, 2)), static_cast<int>(luaL_checknumber(L, 3)));
	return 0;
}

// -----------------------------------------------------------------------------

static bool AnimationTypeStringToNumber(const char *TypeString, Animation::ANIMATION_TYPES &Result) {
	if (strcmp(TypeString, "jojo") == 0) {
		Result = Animation::AT_JOJO;
		return true;
	} else if (strcmp(TypeString, "loop") == 0) {
		Result = Animation::AT_LOOP;
		return true;
	} else if (strcmp(TypeString, "oneshot") == 0) {
		Result = Animation::AT_ONESHOT;
		return true;
	} else
		return false;
}

// -----------------------------------------------------------------------------

static int AT_SetAnimationType(lua_State *L) {
	AnimationTemplate *pAT = CheckAnimationTemplate(L);
	Animation::ANIMATION_TYPES AnimationType;
	if (AnimationTypeStringToNumber(luaL_checkstring(L, 2), AnimationType)) {
		pAT->setAnimationType(AnimationType);
	} else {
		luaL_argcheck(L, 0, 2, "Invalid animation type");
	}

	return 0;
}

// -----------------------------------------------------------------------------

static int AT_SetFPS(lua_State *L) {
	AnimationTemplate *pAT = CheckAnimationTemplate(L);
	pAT->setFPS(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int AT_Finalize(lua_State *L) {
	AnimationTemplate *pAT = CheckAnimationTemplate(L);
	delete pAT;
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg ANIMATION_TEMPLATE_METHODS[] = {
	{"AddFrame", AT_AddFrame},
	{"SetFrame", AT_SetFrame},
	{"SetAnimationType", AT_SetAnimationType},
	{"SetFPS", AT_SetFPS},
	{"__gc", AT_Finalize},
	{0, 0}
};

// -----------------------------------------------------------------------------

static GraphicEngine *GetGE() {
	Kernel *pKernel = Kernel::GetInstance();
	BS_ASSERT(pKernel);
	GraphicEngine *pGE = static_cast<GraphicEngine *>(pKernel->GetService("gfx"));
	BS_ASSERT(pGE);
	return pGE;
}

// -----------------------------------------------------------------------------

static int Init(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	switch (lua_gettop(L)) {
	case 0:
		lua_pushbooleancpp(L, pGE->Init());
		break;
	case 1:
		lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L, 1))));
		break;
	case 2:
		lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2))));
		break;
	case 3:
		lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2)),
		                                static_cast<int>(luaL_checknumber(L, 3))));
		break;
	case 4:
		lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2)),
		                                static_cast<int>(luaL_checknumber(L, 3)), static_cast<int>(luaL_checknumber(L, 4))));
		break;
	default:
		lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L, 1)), static_cast<int>(luaL_checknumber(L, 2)),
		                                static_cast<int>(luaL_checknumber(L, 3)), static_cast<int>(luaL_checknumber(L, 4)),
		                                lua_tobooleancpp(L, 5)));
	}


#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Main-Panel zum Gfx-Modul hinzufügen
	RenderObjectPtr<Panel> MainPanelPtr(GetGE()->GetMainPanel());
	BS_ASSERT(MainPanelPtr.isValid());

	lua_pushstring(L, GFX_LIBRARY_NAME);
	lua_gettable(L, LUA_GLOBALSINDEX);
	BS_ASSERT(!lua_isnil(L, -1));

	NewUintUserData(L, MainPanelPtr->getHandle());
	BS_ASSERT(!lua_isnil(L, -1));
	// luaL_getmetatable(L, PANEL_CLASS_NAME);
	LuaBindhelper::getMetatable(L, PANEL_CLASS_NAME);
	BS_ASSERT(!lua_isnil(L, -1));
	lua_setmetatable(L, -2);

	lua_pushstring(L, "MainPanel");
	lua_insert(L, -2);
	lua_settable(L, -3);

	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return 1;
}

// -----------------------------------------------------------------------------

static int StartFrame(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	if (lua_gettop(L) == 0)
		lua_pushbooleancpp(L, pGE->StartFrame());
	else
		lua_pushbooleancpp(L, pGE->StartFrame(lua_tobooleancpp(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int EndFrame(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushbooleancpp(L, pGE->EndFrame());

	return 1;
}

// -----------------------------------------------------------------------------

static int DrawDebugLine(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	Vertex Start;
	Vertex End;
	Vertex::luaVertexToVertex(L, 1, Start);
	Vertex::luaVertexToVertex(L, 2, End);
	pGE->DrawDebugLine(Start, End, GraphicEngine::LuaColorToARGBColor(L, 3));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetDisplayWidth(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushnumber(L, pGE->GetDisplayWidth());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetDisplayHeight(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushnumber(L, pGE->GetDisplayHeight());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetBitDepth(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushnumber(L, pGE->GetBitDepth());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetVsync(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	pGE->SetVsync(lua_tobooleancpp(L, 1));

	return 0;
}

// -----------------------------------------------------------------------------

static int IsVsync(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushbooleancpp(L, pGE->GetVsync());

	return 1;
}

// -----------------------------------------------------------------------------

static int IsWindowed(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushbooleancpp(L, pGE->IsWindowed());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetFPSCount(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushnumber(L, pGE->GetFPSCount());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetLastFrameDuration(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushnumber(L, pGE->GetLastFrameDuration());

	return 1;
}

// -----------------------------------------------------------------------------

static int StopMainTimer(lua_State *L) {
	GraphicEngine *pGE = GetGE();
	pGE->StopMainTimer();
	return 0;
}

// -----------------------------------------------------------------------------

static int ResumeMainTimer(lua_State *L) {
	GraphicEngine *pGE = GetGE();
	pGE->ResumeMainTimer();
	return 0;
}

// -----------------------------------------------------------------------------

static int GetSecondaryFrameDuration(lua_State *L) {
	GraphicEngine *pGE = GetGE();

	lua_pushnumber(L, pGE->GetSecondaryFrameDuration());

	return 1;
}

// -----------------------------------------------------------------------------

static int SaveScreenshot(lua_State *L) {
	GraphicEngine *pGE = GetGE();
	lua_pushbooleancpp(L, pGE->SaveScreenshot(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int SaveThumbnailScreenshot(lua_State *L) {
	GraphicEngine *pGE = GetGE();
	lua_pushbooleancpp(L, pGE->SaveThumbnailScreenshot(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int GetRepaintedPixels(lua_State *L) {
	GraphicEngine *pGE = GetGE();
	lua_pushnumber(L, static_cast<lua_Number>(pGE->GetRepaintedPixels()));
	return 1;
}

// -----------------------------------------------------------------------------

static const luaL_reg GFX_FUNCTIONS[] = {
	{"Init", Init},
	{"StartFrame", StartFrame},
	{"EndFrame", EndFrame},
	{"DrawDebugLine", DrawDebugLine},
	{"SetVsync", SetVsync},
	{"GetDisplayWidth", GetDisplayWidth},
	{"GetDisplayHeight", GetDisplayHeight},
	{"GetBitDepth", GetBitDepth},
	{"IsVsync", IsVsync},
	{"IsWindowed", IsWindowed},
	{"GetFPSCount", GetFPSCount},
	{"GetLastFrameDuration", GetLastFrameDuration},
	{"StopMainTimer", StopMainTimer},
	{"ResumeMainTimer", ResumeMainTimer},
	{"GetSecondaryFrameDuration", GetSecondaryFrameDuration},
	{"SaveScreenshot", SaveScreenshot},
	{"NewAnimationTemplate", NewAnimationTemplate},
	{"GetRepaintedPixels", GetRepaintedPixels},
	{"SaveThumbnailScreenshot", SaveThumbnailScreenshot},
	{0, 0}
};

// -----------------------------------------------------------------------------

static RenderObjectPtr<RenderObject> CheckRenderObject(lua_State *L, bool ErrorIfRemoved = true) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable einer Klasse haben, die von Gfx.RenderObject "erbt".
	uint *UserDataPtr;
	if ((UserDataPtr = (uint *) my_checkudata(L, 1, BITMAP_CLASS_NAME)) != 0 ||
	        (UserDataPtr = (uint *) my_checkudata(L, 1, ANIMATION_CLASS_NAME)) != 0 ||
	        (UserDataPtr = (uint *) my_checkudata(L, 1, PANEL_CLASS_NAME)) != 0 ||
	        (UserDataPtr = (uint *) my_checkudata(L, 1, TEXT_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> ROPtr(* UserDataPtr);
		if (ROPtr.isValid())
			return ROPtr;
		else {
			if (ErrorIfRemoved)
				luaL_error(L, "The renderobject with the handle %d does no longer exist.", * UserDataPtr);
		}
	} else {
		luaL_argcheck(L, 0, 1, "'" RENDEROBJECT_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<RenderObject>();
}

// -----------------------------------------------------------------------------

static int RO_SetPos(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	Vertex Pos;
	Vertex::luaVertexToVertex(L, 2, Pos);
	ROPtr->setPos(Pos.x, Pos.y);
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetX(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	ROPtr->setX(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetY(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	ROPtr->setY(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetZ(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	ROPtr->setZ(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetVisible(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	ROPtr->setVisible(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_GetX(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getX());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetY(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getY());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetZ(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getZ());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetAbsoluteX(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getAbsoluteX());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetAbsoluteY(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getAbsoluteY());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetWidth(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getWidth());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetHeight(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushnumber(L, ROPtr->getHeight());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_IsVisible(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	lua_pushbooleancpp(L, ROPtr->isVisible());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddPanel(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	RenderObjectPtr<Panel> PanelPtr = ROPtr->addPanel(static_cast<int>(luaL_checknumber(L, 2)),
	                                        static_cast<int>(luaL_checknumber(L, 3)),
	                                        GraphicEngine::LuaColorToARGBColor(L, 4));
	if (PanelPtr.isValid()) {
		NewUintUserData(L, PanelPtr->getHandle());
		// luaL_getmetatable(L, PANEL_CLASS_NAME);
		LuaBindhelper::getMetatable(L, PANEL_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddBitmap(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	RenderObjectPtr<Bitmap> BitmaPtr = ROPtr->addBitmap(luaL_checkstring(L, 2));
	if (BitmaPtr.isValid()) {
		NewUintUserData(L, BitmaPtr->getHandle());
		// luaL_getmetatable(L, BITMAP_CLASS_NAME);
		LuaBindhelper::getMetatable(L, BITMAP_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddText(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());

	RenderObjectPtr<Text> TextPtr;
	if (lua_gettop(L) >= 3) TextPtr = ROPtr->addText(luaL_checkstring(L, 2), luaL_checkstring(L, 3));
	else TextPtr = ROPtr->addText(luaL_checkstring(L, 2));

	if (TextPtr.isValid()) {
		NewUintUserData(L, TextPtr->getHandle());
		// luaL_getmetatable(L, TEXT_CLASS_NAME);
		LuaBindhelper::getMetatable(L, TEXT_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	} else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddAnimation(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());

	RenderObjectPtr<Animation> AnimationPtr;
	if (lua_type(L, 2) == LUA_TUSERDATA)
		AnimationPtr = ROPtr->addAnimation(*CheckAnimationTemplate(L, 2));
	else
		AnimationPtr = ROPtr->addAnimation(luaL_checkstring(L, 2));

	if (AnimationPtr.isValid()) {
		NewUintUserData(L, AnimationPtr->getHandle());
		// luaL_getmetatable(L, ANIMATION_CLASS_NAME);
		LuaBindhelper::getMetatable(L, ANIMATION_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);

		// Alle Animationscallbacks registrieren.
		AnimationPtr->registerDeleteCallback(AnimationDeleteCallback, AnimationPtr->getHandle());
		AnimationPtr->registerLoopPointCallback(AnimationLoopPointCallback, AnimationPtr->getHandle());
		AnimationPtr->registerActionCallback(AnimationActionCallback, AnimationPtr->getHandle());
	} else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static const luaL_reg RENDEROBJECT_METHODS[] = {
	{"AddAnimation", RO_AddAnimation},
	{"AddText", RO_AddText},
	{"AddBitmap", RO_AddBitmap},
	{"AddPanel", RO_AddPanel},
	{"SetPos", RO_SetPos},
	{"SetX", RO_SetX},
	{"SetY", RO_SetY},
	{"SetZ", RO_SetZ},
	{"SetVisible", RO_SetVisible},
	{"GetX", RO_GetX},
	{"GetY", RO_GetY},
	{"GetZ", RO_GetZ},
	{"GetAbsoluteX", RO_GetAbsoluteX},
	{"GetAbsoluteY", RO_GetAbsoluteY},
	{"GetWidth", RO_GetWidth},
	{"GetHeight", RO_GetHeight},
	{"IsVisible", RO_IsVisible},
	{0, 0}
};

// -----------------------------------------------------------------------------

static RenderObjectPtr<Panel> CheckPanel(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Panel
	uint *UserDataPtr;
	if ((UserDataPtr = (uint *) my_checkudata(L, 1, PANEL_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.isValid()) {
			return ROPtr->toPanel();
		} else
			luaL_error(L, "The panel with the handle %d does no longer exist.", *UserDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" PANEL_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Panel>();
}

// -----------------------------------------------------------------------------

static int P_GetColor(lua_State *L) {
	RenderObjectPtr<Panel> PanelPtr = CheckPanel(L);
	BS_ASSERT(PanelPtr.isValid());
	GraphicEngine::ARGBColorToLuaColor(L, PanelPtr->getColor());

	return 1;
}

// -----------------------------------------------------------------------------

static int P_SetColor(lua_State *L) {
	RenderObjectPtr<Panel> PanelPtr = CheckPanel(L);
	BS_ASSERT(PanelPtr.isValid());
	PanelPtr->setColor(GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int P_Remove(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	ROPtr.erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg PANEL_METHODS[] = {
	{"GetColor", P_GetColor},
	{"SetColor", P_SetColor},
	{"Remove", P_Remove},
	{0, 0}
};

// -----------------------------------------------------------------------------

static RenderObjectPtr<Bitmap> CheckBitmap(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Bitmap
	uint *UserDataPtr;
	if ((UserDataPtr = (uint *) my_checkudata(L, 1, BITMAP_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.isValid()) {
			return ROPtr->toBitmap();
		} else
			luaL_error(L, "The bitmap with the handle %d does no longer exist.", *UserDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" BITMAP_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Bitmap>();
}

// -----------------------------------------------------------------------------

static int B_SetAlpha(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setAlpha(static_cast<uint>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetTintColor(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setModulationColor(GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetScaleFactor(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setScaleFactor(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetScaleFactorX(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setScaleFactorX(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetScaleFactorY(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setScaleFactorY(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetFlipH(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setFlipH(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetFlipV(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	BitmapPtr->setFlipV(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_GetAlpha(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushnumber(L, BitmapPtr->getAlpha());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetTintColor(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	GraphicEngine::ARGBColorToLuaColor(L, BitmapPtr->getModulationColor());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetScaleFactorX(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushnumber(L, BitmapPtr->getScaleFactorX());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetScaleFactorY(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushnumber(L, BitmapPtr->getScaleFactorY());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsFlipH(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushbooleancpp(L, BitmapPtr->isFlipH());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsFlipV(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushbooleancpp(L, BitmapPtr->isFlipV());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetPixel(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	Vertex Pos;
	Vertex::luaVertexToVertex(L, 2, Pos);
	GraphicEngine::ARGBColorToLuaColor(L, BitmapPtr->getPixel(Pos.x, Pos.y));
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsScalingAllowed(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushbooleancpp(L, BitmapPtr->isScalingAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsAlphaAllowed(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushbooleancpp(L, BitmapPtr->isAlphaAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsTintingAllowed(lua_State *L) {
	RenderObjectPtr<Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.isValid());
	lua_pushbooleancpp(L, BitmapPtr->isColorModulationAllowed());
	return 1;
}
// -----------------------------------------------------------------------------

static int B_Remove(lua_State *L) {
	RenderObjectPtr<RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.isValid());
	ROPtr.erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg BITMAP_METHODS[] = {
	{"SetAlpha", B_SetAlpha},
	{"SetTintColor", B_SetTintColor},
	{"SetScaleFactor", B_SetScaleFactor},
	{"SetScaleFactorX", B_SetScaleFactorX},
	{"SetScaleFactorY", B_SetScaleFactorY},
	{"SetFlipH", B_SetFlipH},
	{"SetFlipV", B_SetFlipV},
	{"GetAlpha", B_GetAlpha},
	{"GetTintColor", B_GetTintColor},
	{"GetScaleFactorX", B_GetScaleFactorX},
	{"GetScaleFactorY", B_GetScaleFactorY},
	{"IsFlipH", B_IsFlipH},
	{"IsFlipV", B_IsFlipV},
	{"GetPixel", B_GetPixel},
	{"IsScalingAllowed", B_IsScalingAllowed},
	{"IsAlphaAllowed", B_IsAlphaAllowed},
	{"IsTintingAllowed", B_IsTintingAllowed},
	{"Remove", B_Remove},
	{0, 0}
};

// -----------------------------------------------------------------------------

static RenderObjectPtr<Animation> CheckAnimation(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Animation
	uint *UserDataPtr;
	if ((UserDataPtr = (uint *) my_checkudata(L, 1, ANIMATION_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.isValid())
			return ROPtr->toAnimation();
		else {
			luaL_error(L, "The animation with the handle %d does no longer exist.", *UserDataPtr);
		}
	} else {
		luaL_argcheck(L, 0, 1, "'" ANIMATION_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Animation>();
}

// -----------------------------------------------------------------------------

static int A_Play(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->play();
	return 0;
}

// -----------------------------------------------------------------------------

static int A_Pause(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->pause();
	return 0;
}

// -----------------------------------------------------------------------------

static int A_Stop(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->stop();
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetFrame(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->setFrame(static_cast<uint>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetAlpha(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->setAlpha(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}
// -----------------------------------------------------------------------------

static int A_SetTintColor(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->setModulationColor(GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetScaleFactor(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->setScaleFactor(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetScaleFactorX(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->setScaleFactorX(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetScaleFactorY(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr->setScaleFactorY(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_GetScaleFactorX(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushnumber(L, AnimationPtr->getScaleFactorX());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetScaleFactorY(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushnumber(L, AnimationPtr->getScaleFactorY());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetAnimationType(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	switch (AnimationPtr->getAnimationType()) {
	case Animation::AT_JOJO:
		lua_pushstring(L, "jojo");
		break;
	case Animation::AT_LOOP:
		lua_pushstring(L, "loop");
		break;
	case Animation::AT_ONESHOT:
		lua_pushstring(L, "oneshot");
		break;
	default:
		BS_ASSERT(false);
	}
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetFPS(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushnumber(L, AnimationPtr->getFPS());
	return 1;
}


// -----------------------------------------------------------------------------

static int A_GetFrameCount(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushnumber(L, AnimationPtr->getFrameCount());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsScalingAllowed(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushbooleancpp(L, AnimationPtr->isScalingAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsAlphaAllowed(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushbooleancpp(L, AnimationPtr->isAlphaAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsTintingAllowed(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushbooleancpp(L, AnimationPtr->isColorModulationAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetCurrentFrame(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushnumber(L, AnimationPtr->getCurrentFrame());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetCurrentAction(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushstring(L, AnimationPtr->getCurrentAction().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsPlaying(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	lua_pushbooleancpp(L, AnimationPtr->isRunning());
	return 1;
}

// -----------------------------------------------------------------------------

static bool AnimationLoopPointCallback(uint Handle) {
	lua_State *L = static_cast<lua_State *>(Kernel::GetInstance()->GetScript()->getScriptObject());
	LoopPointCallbackPtr->invokeCallbackFunctions(L, Handle);

	return true;
}

// -----------------------------------------------------------------------------

static int A_RegisterLoopPointCallback(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	LoopPointCallbackPtr->registerCallbackFunction(L, AnimationPtr->getHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static int A_UnregisterLoopPointCallback(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	LoopPointCallbackPtr->unregisterCallbackFunction(L, AnimationPtr->getHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static bool AnimationActionCallback(uint Handle) {
	RenderObjectPtr<Animation> AnimationPtr(Handle);
	if (AnimationPtr.isValid()) {
		ActionCallbackPtr->Action = AnimationPtr->getCurrentAction();
		lua_State *L = static_cast<lua_State *>(Kernel::GetInstance()->GetScript()->getScriptObject());
		ActionCallbackPtr->invokeCallbackFunctions(L, AnimationPtr->getHandle());
	}

	return true;
}

// -----------------------------------------------------------------------------

static int A_RegisterActionCallback(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	ActionCallbackPtr->registerCallbackFunction(L, AnimationPtr->getHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static int A_UnregisterActionCallback(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	ActionCallbackPtr->unregisterCallbackFunction(L, AnimationPtr->getHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static bool AnimationDeleteCallback(uint Handle) {
	lua_State *L = static_cast<lua_State *>(Kernel::GetInstance()->GetScript()->getScriptObject());
	LoopPointCallbackPtr->removeAllObjectCallbacks(L, Handle);

	return true;
}

// -----------------------------------------------------------------------------

static int A_Remove(lua_State *L) {
	RenderObjectPtr<Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.isValid());
	AnimationPtr.erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg ANIMATION_METHODS[] = {
	{"Play", A_Play},
	{"Pause", A_Pause},
	{"Stop", A_Stop},
	{"SetFrame", A_SetFrame},
	{"SetAlpha", A_SetAlpha},
	{"SetTintColor", A_SetTintColor},
	{"SetScaleFactor", A_SetScaleFactor},
	{"SetScaleFactorX", A_SetScaleFactorX},
	{"SetScaleFactorY", A_SetScaleFactorY},
	{"GetScaleFactorX", A_GetScaleFactorX},
	{"GetScaleFactorY", A_GetScaleFactorY},
	{"GetAnimationType", A_GetAnimationType},
	{"GetFPS", A_GetFPS},
	{"GetFrameCount", A_GetFrameCount},
	{"IsScalingAllowed", A_IsScalingAllowed},
	{"IsAlphaAllowed", A_IsAlphaAllowed},
	{"IsTintingAllowed", A_IsTintingAllowed},
	{"GetCurrentFrame", A_GetCurrentFrame},
	{"GetCurrentAction", A_GetCurrentAction},
	{"IsPlaying", A_IsPlaying},
	{"RegisterLoopPointCallback", A_RegisterLoopPointCallback},
	{"UnregisterLoopPointCallback", A_UnregisterLoopPointCallback},
	{"RegisterActionCallback", A_RegisterActionCallback},
	{"UnregisterActionCallback", A_UnregisterActionCallback},
	{"Remove", A_Remove},
	{0, 0}
};

// -----------------------------------------------------------------------------

static RenderObjectPtr<Text> CheckText(lua_State *L) {
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Text
	uint *UserDataPtr;
	if ((UserDataPtr = (uint *) my_checkudata(L, 1, TEXT_CLASS_NAME)) != 0) {
		RenderObjectPtr<RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.isValid())
			return ROPtr->toText();
		else
			luaL_error(L, "The text with the handle %d does no longer exist.", *UserDataPtr);
	} else {
		luaL_argcheck(L, 0, 1, "'" TEXT_CLASS_NAME "' expected");
	}

	return RenderObjectPtr<Text>();
}

// -----------------------------------------------------------------------------

static int T_SetFont(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr->SetFont(luaL_checkstring(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetText(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr->SetText(luaL_checkstring(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetAlpha(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr->setAlpha(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetColor(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr->setColor(GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetAutoWrap(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr->SetAutoWrap(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetAutoWrapThreshold(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr->SetAutoWrapThreshold(static_cast<uint>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_GetText(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	lua_pushstring(L, TextPtr->GetText().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetFont(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	lua_pushstring(L, TextPtr->GetFont().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetAlpha(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	lua_pushnumber(L, TextPtr->getAlpha());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetColor(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	lua_pushnumber(L, TextPtr->getColor());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_IsAutoWrap(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	lua_pushbooleancpp(L, TextPtr->IsAutoWrapActive());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetAutoWrapThreshold(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	lua_pushnumber(L, TextPtr->GetAutoWrapThreshold());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_Remove(lua_State *L) {
	RenderObjectPtr<Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.isValid());
	TextPtr.erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg TEXT_METHODS[] = {
	{"SetFont", T_SetFont},
	{"SetText", T_SetText},
	{"SetAlpha", T_SetAlpha},
	{"SetColor", T_SetColor},
	{"SetAutoWrap", T_SetAutoWrap},
	{"SetAutoWrapThreshold", T_SetAutoWrapThreshold},
	{"GetText", T_GetText},
	{"GetFont", T_GetFont},
	{"GetAlpha", T_GetAlpha},
	{"GetColor", T_GetColor},
	{"IsAutoWrap", T_IsAutoWrap},
	{"GetAutoWrapThreshold", T_GetAutoWrapThreshold},
	{"Remove", T_Remove},
	{0, 0}
};

// -----------------------------------------------------------------------------

bool GraphicEngine::RegisterScriptBindings() {
	Kernel *pKernel = Kernel::GetInstance();
	BS_ASSERT(pKernel);
	ScriptEngine *pScript = static_cast<ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	BS_ASSERT(L);

	if (!LuaBindhelper::addMethodsToClass(L, BITMAP_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, ANIMATION_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, PANEL_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, TEXT_CLASS_NAME, RENDEROBJECT_METHODS)) return false;

	if (!LuaBindhelper::addMethodsToClass(L, PANEL_CLASS_NAME, PANEL_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, BITMAP_CLASS_NAME, BITMAP_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, TEXT_CLASS_NAME, TEXT_METHODS)) return false;
	if (!LuaBindhelper::addMethodsToClass(L, ANIMATION_CLASS_NAME, ANIMATION_METHODS)) return false;

	if (!LuaBindhelper::addMethodsToClass(L, ANIMATION_TEMPLATE_CLASS_NAME, ANIMATION_TEMPLATE_METHODS)) return false;

	if (!LuaBindhelper::addFunctionsToLib(L, GFX_LIBRARY_NAME, GFX_FUNCTIONS)) return false;

	LoopPointCallbackPtr.reset(new LuaCallback(L));
	ActionCallbackPtr.reset(new ActionCallback(L));

	return true;
}

} // End of namespace Sword25
