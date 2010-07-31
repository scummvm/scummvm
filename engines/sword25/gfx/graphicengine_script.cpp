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

#include <algorithm>
#include <string>

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

#define BS_LOG_PREFIX "GRAPHICENGINE"

// -----------------------------------------------------------------------------
// Callback-Objekte
// -----------------------------------------------------------------------------

static bool AnimationDeleteCallback(unsigned int Data);
static bool AnimationActionCallback(unsigned int Data);
static bool AnimationLoopPointCallback(unsigned int Data);

namespace
{
	// -------------------------------------------------------------------------

	class ActionCallback : public BS_LuaCallback
	{
	public:
		ActionCallback(lua_State * L) : BS_LuaCallback(L) {};

		std::string Action;

	protected:
		virtual int PreFunctionInvokation(lua_State * L)
		{
			lua_pushstring(L, Action.c_str());
			return 1;
		}
	};

	std::auto_ptr<BS_LuaCallback> LoopPointCallbackPtr;
	std::auto_ptr<ActionCallback> ActionCallbackPtr;

	// -------------------------------------------------------------------------

	struct CallbackfunctionRegisterer
	{
		CallbackfunctionRegisterer()
		{
			BS_CallbackRegistry::GetInstance().RegisterCallbackFunction("LuaLoopPointCB", AnimationLoopPointCallback);
			BS_CallbackRegistry::GetInstance().RegisterCallbackFunction("LuaActionCB", AnimationActionCallback);
			BS_CallbackRegistry::GetInstance().RegisterCallbackFunction("LuaDeleteCB", AnimationDeleteCallback);
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
static const char * GFX_LIBRARY_NAME = "Gfx";

// -----------------------------------------------------------------------------

// Wie luaL_checkudata, nur ohne dass kein Fehler erzeugt wird.
static void * my_checkudata (lua_State *L, int ud, const char *tname)
{
	int top = lua_gettop(L);

	void * p = lua_touserdata(L, ud);
	if (p != NULL) /* value is a userdata? */
	{
		if (lua_getmetatable(L, ud)) /* does it have a metatable? */
		{  
			// lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			BS_LuaBindhelper::GetMetatable(L, tname);
			if (lua_rawequal(L, -1, -2)) /* does it have the correct mt? */
			{
				lua_settop(L, top);
				return p;
			}
		}
	}

	lua_settop(L, top);
	return NULL;
}

// -----------------------------------------------------------------------------

static void NewUintUserData(lua_State * L, unsigned int Value)
{
	void * UserData = lua_newuserdata(L, sizeof(Value));
	memcpy(UserData, &Value, sizeof(Value));
}

// -----------------------------------------------------------------------------

static BS_AnimationTemplate * CheckAnimationTemplate(lua_State * L, int idx = 1)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.AnimationTemplate
	unsigned int AnimationTemplateHandle;
	if ((AnimationTemplateHandle = *reinterpret_cast<unsigned int *>(my_checkudata(L, idx, ANIMATION_TEMPLATE_CLASS_NAME))) != 0)
	{
		BS_AnimationTemplate * AnimationTemplatePtr = BS_AnimationTemplateRegistry::GetInstance().ResolveHandle(AnimationTemplateHandle);
		if (!AnimationTemplatePtr)
			luaL_error(L, "The animation template with the handle %d does no longer exist.", AnimationTemplateHandle);
		return AnimationTemplatePtr;
	}
	else
	{
		luaL_argcheck(L, 0, idx, "'" ANIMATION_TEMPLATE_CLASS_NAME "' expected");
		return 0;
	}
}


// -----------------------------------------------------------------------------

static int NewAnimationTemplate(lua_State * L)
{
	unsigned int AnimationTemplateHandle = BS_AnimationTemplate::Create(luaL_checkstring(L, 1));
	BS_AnimationTemplate * AnimationTemplatePtr = BS_AnimationTemplateRegistry::GetInstance().ResolveHandle(AnimationTemplateHandle);
	if (AnimationTemplatePtr && AnimationTemplatePtr->IsValid())
	{
		NewUintUserData(L, AnimationTemplateHandle);
		//luaL_getmetatable(L, ANIMATION_TEMPLATE_CLASS_NAME);
		BS_LuaBindhelper::GetMetatable(L, ANIMATION_TEMPLATE_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

// -----------------------------------------------------------------------------

static int AT_AddFrame(lua_State * L)
{
	BS_AnimationTemplate * pAT = CheckAnimationTemplate(L);
	pAT->AddFrame(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int AT_SetFrame(lua_State * L)
{
	BS_AnimationTemplate * pAT = CheckAnimationTemplate(L);
	pAT->SetFrame(static_cast<int>(luaL_checknumber(L, 2)), static_cast<int>(luaL_checknumber(L, 3)));
	return 0;
}

// -----------------------------------------------------------------------------

static bool AnimationTypeStringToNumber(const char * TypeString, BS_Animation::ANIMATION_TYPES & Result)
{
	if (strcmp(TypeString, "jojo") == 0)
	{
		Result = BS_Animation::AT_JOJO;
		return true;
	}
	else if (strcmp(TypeString, "loop") == 0)
	{
		Result = BS_Animation::AT_LOOP;
		return true;
	}
	else if (strcmp(TypeString, "oneshot") == 0)
	{
		Result = BS_Animation::AT_ONESHOT;
		return true;
	}
	else
		return false;
}

// -----------------------------------------------------------------------------

static int AT_SetAnimationType(lua_State * L)
{
	BS_AnimationTemplate * pAT = CheckAnimationTemplate(L);
	BS_Animation::ANIMATION_TYPES AnimationType;
	if (AnimationTypeStringToNumber(luaL_checkstring(L, 2), AnimationType))
	{
		pAT->SetAnimationType(AnimationType);
	}
	else
	{
		luaL_argcheck(L, 0, 2, "Invalid animation type");
	}
	
	return 0;
}

// -----------------------------------------------------------------------------

static int AT_SetFPS(lua_State * L)
{
	BS_AnimationTemplate * pAT = CheckAnimationTemplate(L);
	pAT->SetFPS(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int AT_Finalize(lua_State * L)
{
	BS_AnimationTemplate * pAT = CheckAnimationTemplate(L);
	delete pAT;
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg ANIMATION_TEMPLATE_METHODS[] =
{
	"AddFrame", AT_AddFrame,
	"SetFrame", AT_SetFrame,
	"SetAnimationType", AT_SetAnimationType,
	"SetFPS", AT_SetFPS,
	"__gc", AT_Finalize,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_GraphicEngine * GetGE()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_GraphicEngine * pGE = static_cast<BS_GraphicEngine *>(pKernel->GetService("gfx"));
	BS_ASSERT(pGE);
	return pGE;
}

// -----------------------------------------------------------------------------

static int Init(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	switch (lua_gettop(L))
	{
		case 0:
			lua_pushbooleancpp(L, pGE->Init());
			break;
		case 1:
			lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L, 1))));
			break;
		case 2:
			lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L,1)), static_cast<int>(luaL_checknumber(L, 2))));
			break;
		case 3:
			lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L,1)), static_cast<int>(luaL_checknumber(L, 2)),
											static_cast<int>(luaL_checknumber(L, 3))));
			break;
		case 4:
			lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L,1)), static_cast<int>(luaL_checknumber(L, 2)),
											static_cast<int>(luaL_checknumber(L, 3)), static_cast<int>(luaL_checknumber(L, 4))));
			break;
		default:
			lua_pushbooleancpp(L, pGE->Init(static_cast<int>(luaL_checknumber(L,1)), static_cast<int>(luaL_checknumber(L, 2)),
											static_cast<int>(luaL_checknumber(L, 3)), static_cast<int>(luaL_checknumber(L, 4)),
											lua_tobooleancpp(L, 5)));
	}

	
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Main-Panel zum Gfx-Modul hinzufügen
	BS_RenderObjectPtr<BS_Panel> MainPanelPtr(GetGE()->GetMainPanel());
	BS_ASSERT(MainPanelPtr.IsValid());

	lua_pushstring(L, GFX_LIBRARY_NAME);
	lua_gettable(L, LUA_GLOBALSINDEX);
	BS_ASSERT(!lua_isnil(L, -1));

	NewUintUserData(L, MainPanelPtr->GetHandle());
	BS_ASSERT(!lua_isnil(L, -1));
	// luaL_getmetatable(L, PANEL_CLASS_NAME);
	BS_LuaBindhelper::GetMetatable(L, PANEL_CLASS_NAME);
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

static int StartFrame(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	if (lua_gettop(L) == 0)
		lua_pushbooleancpp(L, pGE->StartFrame());
	else
		lua_pushbooleancpp(L, pGE->StartFrame(lua_tobooleancpp(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int EndFrame(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushbooleancpp(L, pGE->EndFrame());

	return 1;
}

// -----------------------------------------------------------------------------

static int DrawDebugLine(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	BS_Vertex Start;
	BS_Vertex End;
	BS_Vertex::LuaVertexToVertex(L, 1, Start);
	BS_Vertex::LuaVertexToVertex(L, 2, End);
	pGE->DrawDebugLine(Start, End, BS_GraphicEngine::LuaColorToARGBColor(L, 3));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetDisplayWidth(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushnumber(L, pGE->GetDisplayWidth());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetDisplayHeight(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushnumber(L, pGE->GetDisplayHeight());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetBitDepth(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushnumber(L, pGE->GetBitDepth());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetVsync(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	pGE->SetVsync(lua_tobooleancpp(L, 1));

	return 0;
}

// -----------------------------------------------------------------------------

static int IsVsync(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushbooleancpp(L, pGE->GetVsync());

	return 1;
}

// -----------------------------------------------------------------------------

static int IsWindowed(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushbooleancpp(L, pGE->IsWindowed());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetFPSCount(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushnumber(L, pGE->GetFPSCount());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetLastFrameDuration(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushnumber(L, pGE->GetLastFrameDuration());

	return 1;
}

// -----------------------------------------------------------------------------

static int StopMainTimer(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();
	pGE->StopMainTimer();
	return 0;
}

// -----------------------------------------------------------------------------

static int ResumeMainTimer(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();
	pGE->ResumeMainTimer();
	return 0;
}

// -----------------------------------------------------------------------------

static int GetSecondaryFrameDuration(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();

	lua_pushnumber(L, pGE->GetSecondaryFrameDuration());

	return 1;
}

// -----------------------------------------------------------------------------

static int SaveScreenshot(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();
	lua_pushbooleancpp(L, pGE->SaveScreenshot(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int SaveThumbnailScreenshot(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();
	lua_pushbooleancpp(L, pGE->SaveThumbnailScreenshot(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int GetRepaintedPixels(lua_State * L)
{
	BS_GraphicEngine * pGE = GetGE();
	lua_pushnumber(L, static_cast<lua_Number>(pGE->GetRepaintedPixels()));
	return 1;
}

// -----------------------------------------------------------------------------

static const luaL_reg GFX_FUNCTIONS[] =
{
	"Init", Init,
	"StartFrame", StartFrame,
	"EndFrame", EndFrame,
	"DrawDebugLine", DrawDebugLine,
	"SetVsync", SetVsync,
	"GetDisplayWidth", GetDisplayWidth,
	"GetDisplayHeight", GetDisplayHeight,
	"GetBitDepth", GetBitDepth,
	"IsVsync", IsVsync,
	"IsWindowed", IsWindowed,
	"GetFPSCount", GetFPSCount,
	"GetLastFrameDuration", GetLastFrameDuration,
	"StopMainTimer", StopMainTimer,
	"ResumeMainTimer", ResumeMainTimer,
	"GetSecondaryFrameDuration", GetSecondaryFrameDuration,
	"SaveScreenshot", SaveScreenshot,
	"NewAnimationTemplate", NewAnimationTemplate,
	"GetRepaintedPixels", GetRepaintedPixels,
	"SaveThumbnailScreenshot", SaveThumbnailScreenshot,
	0, 0,	
};

// -----------------------------------------------------------------------------

static BS_RenderObjectPtr<BS_RenderObject> CheckRenderObject(lua_State * L, bool ErrorIfRemoved = true)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable einer Klasse haben, die von Gfx.RenderObject "erbt".
	unsigned int * UserDataPtr;
	if ((UserDataPtr = (unsigned int *) my_checkudata(L, 1, BITMAP_CLASS_NAME)) != 0 ||
		(UserDataPtr = (unsigned int *) my_checkudata(L, 1, ANIMATION_CLASS_NAME)) != 0 ||
		(UserDataPtr = (unsigned int *) my_checkudata(L, 1, PANEL_CLASS_NAME)) != 0 ||
		(UserDataPtr = (unsigned int *) my_checkudata(L, 1, TEXT_CLASS_NAME)) != 0)
	{
		BS_RenderObjectPtr<BS_RenderObject> ROPtr(* UserDataPtr);
		if (ROPtr.IsValid())
			return ROPtr;
		else
		{
			if (ErrorIfRemoved)
				luaL_error(L, "The renderobject with the handle %d does no longer exist.", * UserDataPtr);
		}
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" RENDEROBJECT_CLASS_NAME "' expected");
	}

	return BS_RenderObjectPtr<BS_RenderObject>();
}

// -----------------------------------------------------------------------------

static int RO_SetPos(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	BS_Vertex Pos;
	BS_Vertex::LuaVertexToVertex(L, 2, Pos);
	ROPtr->SetPos(Pos.X, Pos.Y);
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetX(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	ROPtr->SetX(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetY(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	ROPtr->SetY(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetZ(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	ROPtr->SetZ(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_SetVisible(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	ROPtr->SetVisible(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int RO_GetX(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetX());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetY(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetY());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetZ(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetZ());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetAbsoluteX(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetAbsoluteX());

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetAbsoluteY(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetAbsoluteY());
	
	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetWidth(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetWidth());
	
	return 1;
}

// -----------------------------------------------------------------------------

static int RO_GetHeight(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushnumber(L, ROPtr->GetHeight());
	
	return 1;
}

// -----------------------------------------------------------------------------

static int RO_IsVisible(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	lua_pushbooleancpp(L, ROPtr->IsVisible());
	
	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddPanel(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	BS_RenderObjectPtr<BS_Panel> PanelPtr = ROPtr->AddPanel(static_cast<int>(luaL_checknumber(L, 2)),
															static_cast<int>(luaL_checknumber(L, 3)),
															BS_GraphicEngine::LuaColorToARGBColor(L, 4));
	if (PanelPtr.IsValid())
	{
		NewUintUserData(L, PanelPtr->GetHandle());
		// luaL_getmetatable(L, PANEL_CLASS_NAME);
		BS_LuaBindhelper::GetMetatable(L, PANEL_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddBitmap(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	BS_RenderObjectPtr<BS_Bitmap> BitmaPtr = ROPtr->AddBitmap(luaL_checkstring(L, 2));
	if (BitmaPtr.IsValid())
	{
		NewUintUserData(L, BitmaPtr->GetHandle());
		// luaL_getmetatable(L, BITMAP_CLASS_NAME);
		BS_LuaBindhelper::GetMetatable(L, BITMAP_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddText(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());

	BS_RenderObjectPtr<BS_Text> TextPtr;
	if (lua_gettop(L) >= 3) TextPtr = ROPtr->AddText(luaL_checkstring(L, 2), luaL_checkstring(L, 3));
	else TextPtr = ROPtr->AddText(luaL_checkstring(L, 2));

	if (TextPtr.IsValid())
	{
		NewUintUserData(L, TextPtr->GetHandle());
		// luaL_getmetatable(L, TEXT_CLASS_NAME);
		BS_LuaBindhelper::GetMetatable(L, TEXT_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_AddAnimation(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());

	BS_RenderObjectPtr<BS_Animation> AnimationPtr;
	if (lua_type(L, 2) == LUA_TUSERDATA)
		AnimationPtr = ROPtr->AddAnimation(*CheckAnimationTemplate(L, 2));
	else
		AnimationPtr = ROPtr->AddAnimation(luaL_checkstring(L, 2));

	if (AnimationPtr.IsValid())
	{
		NewUintUserData(L, AnimationPtr->GetHandle());
		// luaL_getmetatable(L, ANIMATION_CLASS_NAME);
		BS_LuaBindhelper::GetMetatable(L, ANIMATION_CLASS_NAME);
		BS_ASSERT(!lua_isnil(L, -1));
		lua_setmetatable(L, -2);

		// Alle Animationscallbacks registrieren.
		AnimationPtr->RegisterDeleteCallback(AnimationDeleteCallback, AnimationPtr->GetHandle());
		AnimationPtr->RegisterLoopPointCallback(AnimationLoopPointCallback, AnimationPtr->GetHandle());
		AnimationPtr->RegisterActionCallback(AnimationActionCallback, AnimationPtr->GetHandle());
	}
	else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static int RO_Remove(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	ROPtr.Erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg RENDEROBJECT_METHODS[] =
{
	"AddAnimation", RO_AddAnimation,
	"AddText", RO_AddText,
	"AddBitmap", RO_AddBitmap,
	"AddPanel", RO_AddPanel,
	"SetPos", RO_SetPos,
	"SetX", RO_SetX,
	"SetY", RO_SetY,
	"SetZ", RO_SetZ,
	"SetVisible", RO_SetVisible,
	"GetX", RO_GetX,
	"GetY", RO_GetY,
	"GetZ", RO_GetZ,
	"GetAbsoluteX", RO_GetAbsoluteX,
	"GetAbsoluteY", RO_GetAbsoluteY,
	"GetWidth", RO_GetWidth,
	"GetHeight", RO_GetHeight,
	"IsVisible", RO_IsVisible,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_RenderObjectPtr<BS_Panel> CheckPanel(lua_State * L)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Panel
	unsigned int * UserDataPtr;
	if ((UserDataPtr = (unsigned int *) my_checkudata(L, 1, PANEL_CLASS_NAME)) != 0)
	{
		BS_RenderObjectPtr<BS_RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.IsValid())
		{
			return ROPtr->ToPanel();
		}
		else
			luaL_error(L, "The panel with the handle %d does no longer exist.", *UserDataPtr);
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" PANEL_CLASS_NAME "' expected");
	}

	return BS_RenderObjectPtr<BS_Panel>();
}

// -----------------------------------------------------------------------------

static int P_GetColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Panel> PanelPtr = CheckPanel(L);
	BS_ASSERT(PanelPtr.IsValid());
	BS_GraphicEngine::ARGBColorToLuaColor(L, PanelPtr->GetColor());
	
	return 1;
}

// -----------------------------------------------------------------------------

static int P_SetColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Panel> PanelPtr = CheckPanel(L);
	BS_ASSERT(PanelPtr.IsValid());
	PanelPtr->SetColor(BS_GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int P_Remove(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	ROPtr.Erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg PANEL_METHODS[] =
{
	"GetColor", P_GetColor,
	"SetColor", P_SetColor,
	"Remove", P_Remove,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_RenderObjectPtr<BS_Bitmap> CheckBitmap(lua_State * L)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Bitmap
	unsigned int * UserDataPtr;
	if ((UserDataPtr = (unsigned int *) my_checkudata(L, 1, BITMAP_CLASS_NAME)) != 0)
	{
		BS_RenderObjectPtr<BS_RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.IsValid())
		{
			return ROPtr->ToBitmap();
		}
		else
			luaL_error(L, "The bitmap with the handle %d does no longer exist.", *UserDataPtr);
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" BITMAP_CLASS_NAME "' expected");
	}

	return BS_RenderObjectPtr<BS_Bitmap>();
}

// -----------------------------------------------------------------------------

static int B_SetAlpha(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetAlpha(static_cast<unsigned int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetTintColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetModulationColor(BS_GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetScaleFactor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetScaleFactor(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetScaleFactorX(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetScaleFactorX(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetScaleFactorY(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetScaleFactorY(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetFlipH(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetFlipH(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_SetFlipV(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BitmapPtr->SetFlipV(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int B_GetAlpha(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushnumber(L, BitmapPtr->GetAlpha());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetTintColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BS_GraphicEngine::ARGBColorToLuaColor(L, BitmapPtr->GetModulationColor());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetScaleFactorX(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushnumber(L, BitmapPtr->GetScaleFactorX());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetScaleFactorY(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushnumber(L, BitmapPtr->GetScaleFactorY());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsFlipH(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushbooleancpp(L, BitmapPtr->IsFlipH());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsFlipV(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushbooleancpp(L, BitmapPtr->IsFlipV());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_GetPixel(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	BS_Vertex Pos;
	BS_Vertex::LuaVertexToVertex(L, 2, Pos);
	BS_GraphicEngine::ARGBColorToLuaColor(L, BitmapPtr->GetPixel(Pos.X, Pos.Y));
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsScalingAllowed(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushbooleancpp(L, BitmapPtr->IsScalingAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsAlphaAllowed(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushbooleancpp(L, BitmapPtr->IsAlphaAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int B_IsTintingAllowed(lua_State * L)
{
	BS_RenderObjectPtr<BS_Bitmap> BitmapPtr = CheckBitmap(L);
	BS_ASSERT(BitmapPtr.IsValid());
	lua_pushbooleancpp(L, BitmapPtr->IsColorModulationAllowed());
	return 1;
}
// -----------------------------------------------------------------------------

static int B_Remove(lua_State * L)
{
	BS_RenderObjectPtr<BS_RenderObject> ROPtr = CheckRenderObject(L);
	BS_ASSERT(ROPtr.IsValid());
	ROPtr.Erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg BITMAP_METHODS[] =
{
	"SetAlpha", B_SetAlpha,
	"SetTintColor", B_SetTintColor,
	"SetScaleFactor", B_SetScaleFactor,
	"SetScaleFactorX", B_SetScaleFactorX,
	"SetScaleFactorY", B_SetScaleFactorY,
	"SetFlipH", B_SetFlipH,
	"SetFlipV", B_SetFlipV,
	"GetAlpha", B_GetAlpha,
	"GetTintColor", B_GetTintColor,
	"GetScaleFactorX", B_GetScaleFactorX,
	"GetScaleFactorY", B_GetScaleFactorY,
	"IsFlipH", B_IsFlipH,
	"IsFlipV", B_IsFlipV,
	"GetPixel", B_GetPixel,
	"IsScalingAllowed", B_IsScalingAllowed,
	"IsAlphaAllowed", B_IsAlphaAllowed,
	"IsTintingAllowed", B_IsTintingAllowed,
	"Remove", B_Remove,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_RenderObjectPtr<BS_Animation> CheckAnimation(lua_State * L)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Animation
	unsigned int * UserDataPtr;
	if ((UserDataPtr = (unsigned int *) my_checkudata(L, 1, ANIMATION_CLASS_NAME)) != 0)
	{
		BS_RenderObjectPtr<BS_RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.IsValid())
			return ROPtr->ToAnimation();
		else
		{
			luaL_error(L, "The animation with the handle %d does no longer exist.", *UserDataPtr);
		}
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" ANIMATION_CLASS_NAME "' expected");
	}

	return BS_RenderObjectPtr<BS_Animation>();
}

// -----------------------------------------------------------------------------

static int A_Play(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->Play();
	return 0;
}

// -----------------------------------------------------------------------------

static int A_Pause(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->Pause();
	return 0;
}

// -----------------------------------------------------------------------------

static int A_Stop(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->Stop();
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetFrame(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->SetFrame(static_cast<unsigned int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetAlpha(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->SetAlpha(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}
// -----------------------------------------------------------------------------

static int A_SetTintColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->SetModulationColor(BS_GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetScaleFactor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->SetScaleFactor(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetScaleFactorX(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->SetScaleFactorX(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_SetScaleFactorY(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr->SetScaleFactorY(static_cast<float>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int A_GetScaleFactorX(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushnumber(L, AnimationPtr->GetScaleFactorX());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetScaleFactorY(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushnumber(L, AnimationPtr->GetScaleFactorY());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetAnimationType(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	switch (AnimationPtr->GetAnimationType())
	{
		case BS_Animation::AT_JOJO:
			lua_pushstring(L, "jojo");
			break;
		case BS_Animation::AT_LOOP:
			lua_pushstring(L, "loop");
			break;
		case BS_Animation::AT_ONESHOT:
			lua_pushstring(L, "oneshot");
			break;
		default:
			BS_ASSERT(false);
	}
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetFPS(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushnumber(L, AnimationPtr->GetFPS());
	return 1;
}


// -----------------------------------------------------------------------------

static int A_GetFrameCount(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushnumber(L, AnimationPtr->GetFrameCount());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsScalingAllowed(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushbooleancpp(L, AnimationPtr->IsScalingAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsAlphaAllowed(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushbooleancpp(L, AnimationPtr->IsAlphaAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsTintingAllowed(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushbooleancpp(L, AnimationPtr->IsColorModulationAllowed());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetCurrentFrame(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushnumber(L, AnimationPtr->GetCurrentFrame());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_GetCurrentAction(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushstring(L, AnimationPtr->GetCurrentAction().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int A_IsPlaying(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	lua_pushbooleancpp(L, AnimationPtr->IsRunning());
	return 1;
}

// -----------------------------------------------------------------------------

static bool AnimationLoopPointCallback(unsigned int Handle)
{
	lua_State * L = static_cast<lua_State *>(BS_Kernel::GetInstance()->GetScript()->GetScriptObject());
	LoopPointCallbackPtr->InvokeCallbackFunctions(L, Handle);

	return true;
}

// -----------------------------------------------------------------------------

static int A_RegisterLoopPointCallback(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	LoopPointCallbackPtr->RegisterCallbackFunction(L, AnimationPtr->GetHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static int A_UnregisterLoopPointCallback(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	LoopPointCallbackPtr->UnregisterCallbackFunction(L, AnimationPtr->GetHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static bool AnimationActionCallback(unsigned int Handle)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr(Handle);
	if (AnimationPtr.IsValid())
	{
		ActionCallbackPtr->Action = AnimationPtr->GetCurrentAction();
		lua_State * L = static_cast<lua_State *>(BS_Kernel::GetInstance()->GetScript()->GetScriptObject());
		ActionCallbackPtr->InvokeCallbackFunctions(L, AnimationPtr->GetHandle());
	}

	return true;
}

// -----------------------------------------------------------------------------

static int A_RegisterActionCallback(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	ActionCallbackPtr->RegisterCallbackFunction(L, AnimationPtr->GetHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static int A_UnregisterActionCallback(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	ActionCallbackPtr->UnregisterCallbackFunction(L, AnimationPtr->GetHandle());

	return 0;
}

// -----------------------------------------------------------------------------

static bool AnimationDeleteCallback(unsigned int Handle)
{
	lua_State * L = static_cast<lua_State *>(BS_Kernel::GetInstance()->GetScript()->GetScriptObject());
	LoopPointCallbackPtr->RemoveAllObjectCallbacks(L, Handle);

	return true;
}

// -----------------------------------------------------------------------------

static int A_Remove(lua_State * L)
{
	BS_RenderObjectPtr<BS_Animation> AnimationPtr = CheckAnimation(L);
	BS_ASSERT(AnimationPtr.IsValid());
	AnimationPtr.Erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg ANIMATION_METHODS[] =
{
	"Play", A_Play,
	"Pause", A_Pause,
	"Stop", A_Stop,
	"SetFrame", A_SetFrame,
	"SetAlpha", A_SetAlpha,
	"SetTintColor", A_SetTintColor,
	"SetScaleFactor", A_SetScaleFactor,
	"SetScaleFactorX", A_SetScaleFactorX,
	"SetScaleFactorY", A_SetScaleFactorY,
	"GetScaleFactorX", A_GetScaleFactorX,
	"GetScaleFactorY", A_GetScaleFactorY,
	"GetAnimationType", A_GetAnimationType,
	"GetFPS", A_GetFPS,
	"GetFrameCount", A_GetFrameCount,
	"IsScalingAllowed", A_IsScalingAllowed,
	"IsAlphaAllowed", A_IsAlphaAllowed,
	"IsTintingAllowed", A_IsTintingAllowed,
	"GetCurrentFrame", A_GetCurrentFrame,
	"GetCurrentAction", A_GetCurrentAction,
	"IsPlaying", A_IsPlaying,
	"RegisterLoopPointCallback", A_RegisterLoopPointCallback,
	"UnregisterLoopPointCallback", A_UnregisterLoopPointCallback,
	"RegisterActionCallback", A_RegisterActionCallback,
	"UnregisterActionCallback", A_UnregisterActionCallback,
	"Remove", A_Remove,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_RenderObjectPtr<BS_Text> CheckText(lua_State * L)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Gfx.Text
	unsigned int * UserDataPtr;
	if ((UserDataPtr = (unsigned int *) my_checkudata(L, 1, TEXT_CLASS_NAME)) != 0)
	{
		BS_RenderObjectPtr<BS_RenderObject> ROPtr(*UserDataPtr);
		if (ROPtr.IsValid())
			return ROPtr->ToText();
		else
			luaL_error(L, "The text with the handle %d does no longer exist.", *UserDataPtr);
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" TEXT_CLASS_NAME "' expected");
	}

	return BS_RenderObjectPtr<BS_Text>();
}

// -----------------------------------------------------------------------------

static int T_SetFont(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr->SetFont(luaL_checkstring(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetText(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr->SetText(luaL_checkstring(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetAlpha(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr->SetAlpha(static_cast<int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr->SetColor(BS_GraphicEngine::LuaColorToARGBColor(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetAutoWrap(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr->SetAutoWrap(lua_tobooleancpp(L, 2));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_SetAutoWrapThreshold(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr->SetAutoWrapThreshold(static_cast<unsigned int>(luaL_checknumber(L, 2)));
	return 0;
}

// -----------------------------------------------------------------------------

static int T_GetText(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	lua_pushstring(L, TextPtr->GetText().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetFont(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	lua_pushstring(L, TextPtr->GetFont().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetAlpha(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	lua_pushnumber(L, TextPtr->GetAlpha());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetColor(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	lua_pushnumber(L, TextPtr->GetColor());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_IsAutoWrap(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	lua_pushbooleancpp(L, TextPtr->IsAutoWrapActive());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_GetAutoWrapThreshold(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	lua_pushnumber(L, TextPtr->GetAutoWrapThreshold());
	return 1;
}

// -----------------------------------------------------------------------------

static int T_Remove(lua_State * L)
{
	BS_RenderObjectPtr<BS_Text> TextPtr = CheckText(L);
	BS_ASSERT(TextPtr.IsValid());
	TextPtr.Erase();
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg TEXT_METHODS[] =
{
	"SetFont", T_SetFont,
	"SetText", T_SetText,
	"SetAlpha", T_SetAlpha,
	"SetColor", T_SetColor,
	"SetAutoWrap", T_SetAutoWrap,
	"SetAutoWrapThreshold", T_SetAutoWrapThreshold,
	"GetText", T_GetText,
	"GetFont", T_GetFont,
	"GetAlpha", T_GetAlpha,
	"GetColor", T_GetColor,
	"IsAutoWrap", T_IsAutoWrap,
	"GetAutoWrapThreshold", T_GetAutoWrapThreshold,
	"Remove", T_Remove,
	0, 0,
};

// -----------------------------------------------------------------------------

bool BS_GraphicEngine::RegisterScriptBindings()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ScriptEngine * pScript = static_cast<BS_ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State * L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!BS_LuaBindhelper::AddMethodsToClass(L, BITMAP_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, ANIMATION_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, PANEL_CLASS_NAME, RENDEROBJECT_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, TEXT_CLASS_NAME, RENDEROBJECT_METHODS)) return false;

	if (!BS_LuaBindhelper::AddMethodsToClass(L, PANEL_CLASS_NAME, PANEL_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, BITMAP_CLASS_NAME, BITMAP_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, TEXT_CLASS_NAME, TEXT_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, ANIMATION_CLASS_NAME, ANIMATION_METHODS)) return false;

	if (!BS_LuaBindhelper::AddMethodsToClass(L, ANIMATION_TEMPLATE_CLASS_NAME, ANIMATION_TEMPLATE_METHODS)) return false;

	if (!BS_LuaBindhelper::AddFunctionsToLib(L, GFX_LIBRARY_NAME, GFX_FUNCTIONS)) return false;

	LoopPointCallbackPtr.reset(new BS_LuaCallback(L));
	ActionCallbackPtr.reset(new ActionCallback(L));

	return true;
}
