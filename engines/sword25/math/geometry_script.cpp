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

#include "sword25/kernel/memlog_off.h"
#include <memory>
#include <vector>
#include "sword25/kernel/memlog_on.h"

#include "sword25/gfx/graphicengine.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/math/geometry.h"
#include "sword25/math/region.h"
#include "sword25/math/regionregistry.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/vertex.h"

// -----------------------------------------------------------------------------

using namespace std;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// Die Strings werden als #defines definiert um Stringkomposition zur Compilezeit zu ermöglichen.
#define REGION_CLASS_NAME "Geo.Region"
#define WALKREGION_CLASS_NAME "Geo.WalkRegion"

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

static bool IsValidPolygonDefinition(lua_State * L)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Sicherstellen, dass wir wirklich eine Tabelle betrachten
	if (!lua_istable(L, -1))
	{
		luaL_error(L, "Invalid polygon definition. Unexpected type, \"table\" needed.");
		return false;
	}

	int TableSize = luaL_getn(L, -1);

	// Sicherstellen, dass mindestens 3 Vertecies existieren.
	if (TableSize < 6)
	{
		luaL_error(L, "Invalid polygon definition. At least three vertecies needed.");
		return false;
	}

	// Sicherstellen, dass die Anzahl der Tabellenelemente durch zwei teilbar ist.
	// Da je zwei Elemente ein Vertex definieren, ist eine ungerade Anzahl an Elementen nicht zulässig.
	if ((TableSize % 2) != 0)
	{
		luaL_error(L, "Invalid polygon definition. Even number of table elements needed.");
		return false;
	}

	// Sicherstellen, dass alle Elemente der Tabelle vom Typ Number sind.
	for (int i = 1; i <= TableSize; i += 1)
	{
		lua_rawgeti(L, -1, i);
		if (!lua_isnumber(L, -1))
		{
			luaL_error(L, "Invalid polygon definition. All table elements have to be numbers.");
			return false;
		}
		lua_pop(L, 1);
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

static void TablePolygonToPolygon(lua_State * L, BS_Polygon & Polygon)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Sicherstellen, dass eine gültige Polygon-Definition auf dem Stack liegt.
	// Es ist nicht notwendig den Rückgabewert abzufangen, da alle Fehler über luaL_error ausgegeben werden und somit die Ausführung des
	// Skriptes beenden.
	IsValidPolygonDefinition(L);

	int VertexCount = luaL_getn(L, -1) / 2;

	// Speicher für Vertecies reservieren
	vector<BS_Vertex> Vertecies;
	Vertecies.reserve(VertexCount);

	// Vertecies erstellen
	for (int i = 0; i < VertexCount; i++)
	{
		// X-Wert
		lua_rawgeti(L, -1, (i * 2) + 1);
		int X = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		// Y-Wert
		lua_rawgeti(L, -1, (i * 2) + 2);
		int Y = static_cast<int>(lua_tonumber(L, -1));
		lua_pop(L, 1);

		// Vertex
		Vertecies.push_back(BS_Vertex(X, Y));
	}
	BS_ASSERT(Vertecies.size() == VertexCount);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	// Polygon erstellen
	Polygon.Init(VertexCount, &Vertecies[0]);
}

// -----------------------------------------------------------------------------

static unsigned int TableRegionToRegion(lua_State * L, const char * ClassName)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Man kann eine Region in Lua auf zwei Arten definieren:
	// 1. Eine Tabelle, die ein Polygon definiert (Polygon = Tabelle mit Zahlen, wobei je zwei aufeinander folgende Zahlen ein Vertex definieren)
	//    Das eine Polygon definiert die Region vollständig (=> keine Löcher möglich)
	// 2. Eine Tabelle, die mehrere Polygondefinitionen (wiederum Tabellen (siehe 1.)) enthält.
	//    Dann definiert das erste Polygon den Umriss der Region und die folgenden Löcher im ersten Polygon.

	// Es darf nur ein Parameter übergeben werden und dieser muss eine Tabelle sein.
	if (lua_gettop(L) != 1 || !lua_istable(L, -1))
	{
		luaL_error(L, "First and only parameter has to be of type \"table\".");
		return 0;
	}

	unsigned int RegionHandle;
	if (ClassName == REGION_CLASS_NAME)
	{
		RegionHandle = BS_Region::Create(BS_Region::RT_REGION);
	}
	else if (ClassName == WALKREGION_CLASS_NAME)
	{
		RegionHandle = BS_WalkRegion::Create(BS_Region::RT_WALKREGION);
	}
	else
	{
		BS_ASSERT(false);
	}

	BS_ASSERT(RegionHandle);

	// Wenn das erste Element des Parameters eine Zahl ist, wird der 1. Fall angenommen.
	// Wenn das erste Element des Parameters eine Tabelle ist, wird der 2. Fall angenommen.
	// Wenn das erste Element des Parameters einen anderen Typ hat, liegt ein Fehler vor.
	lua_rawgeti(L, -1, 1);
	int FirstElementType = lua_type(L, -1);
	lua_pop(L, 1);

	switch(FirstElementType)
	{
		case LUA_TNUMBER:
			{
				BS_Polygon Polygon;
				TablePolygonToPolygon(L, Polygon);
				BS_RegionRegistry::GetInstance().ResolveHandle(RegionHandle)->Init(Polygon);
			}
			break;
		
		case LUA_TTABLE:
			{
				lua_rawgeti(L, -1, 1);
				BS_Polygon Polygon;
				TablePolygonToPolygon(L, Polygon);
				lua_pop(L, 1);

				int PolygonCount = luaL_getn(L, -1);
				if (PolygonCount == 1)
					BS_RegionRegistry::GetInstance().ResolveHandle(RegionHandle)->Init(Polygon);
				else
				{
					vector<BS_Polygon> Holes;
					Holes.reserve(PolygonCount - 1);

					for (int i = 2; i <= PolygonCount; i++)
					{
						lua_rawgeti(L, -1, i);
						Holes.resize(Holes.size() + 1);
						TablePolygonToPolygon(L, Holes.back());
						lua_pop(L, 1);
					}
					BS_ASSERT(Holes.size() == PolygonCount - 1);

					BS_RegionRegistry::GetInstance().ResolveHandle(RegionHandle)->Init(Polygon, &Holes);
				}
			}
			break;

		default:
			luaL_error(L, "Illegal region definition.");
			return 0;
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return RegionHandle;
}

// -----------------------------------------------------------------------------

static void NewUserdataRegion(lua_State * L, const char * ClassName)
{
	// Region aufgrund des Lua-Codes erstellen.
	// Fehler treten nicht auf, sondern werden von der Funktion über luaL_error abgefangen.
	unsigned int RegionHandle = TableRegionToRegion(L, ClassName);
	BS_ASSERT(RegionHandle);

	NewUintUserData(L, RegionHandle);
	// luaL_getmetatable(L, ClassName);
	BS_LuaBindhelper::GetMetatable(L, ClassName);
	BS_ASSERT(!lua_isnil(L, -1));
	lua_setmetatable(L, -2);
}

// -----------------------------------------------------------------------------

static int NewRegion(lua_State * L)
{
	NewUserdataRegion(L, REGION_CLASS_NAME);
	return 1;
}

// -----------------------------------------------------------------------------

static int NewWalkRegion(lua_State * L)
{
	NewUserdataRegion(L, WALKREGION_CLASS_NAME);
	return 1;
}

// -----------------------------------------------------------------------------

static const char * GEO_LIBRARY_NAME = "Geo";

static const luaL_reg GEO_FUNCTIONS[] =
{
	"NewRegion", NewRegion,
	"NewWalkRegion", NewWalkRegion,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_Region * CheckRegion(lua_State * L)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Geo.Region oder Geo.WalkRegion
	unsigned int * RegionHandlePtr;
	if ((RegionHandlePtr = reinterpret_cast<unsigned int *>(my_checkudata(L, 1, REGION_CLASS_NAME))) != 0 ||
		(RegionHandlePtr = reinterpret_cast<unsigned int *>(my_checkudata(L, 1, WALKREGION_CLASS_NAME))) != 0)
	{
		return BS_RegionRegistry::GetInstance().ResolveHandle(*RegionHandlePtr);
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" REGION_CLASS_NAME "' expected");
	}

	// Compiler ruhigstellen. Ausführung kommt nie an diesem Punkt an.
	return 0;
}

// -----------------------------------------------------------------------------

static int R_IsValid(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	lua_pushbooleancpp(L, pR->IsValid());
	return 1;
}

// -----------------------------------------------------------------------------

static int R_GetX(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	lua_pushnumber(L, pR->GetPosX());
	return 1;
}

// -----------------------------------------------------------------------------

static int R_GetY(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	lua_pushnumber(L, pR->GetPosY());
	return 1;
}

// -----------------------------------------------------------------------------

static int R_GetPos(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	BS_Vertex::VertexToLuaVertex(L, pR->GetPosition());
	return 1;
}

// -----------------------------------------------------------------------------

static int R_IsPointInRegion(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	BS_Vertex Vertex;
	BS_Vertex::LuaVertexToVertex(L, 2, Vertex);
	lua_pushbooleancpp(L, pR->IsPointInRegion(Vertex));
	return 1;
}

// -----------------------------------------------------------------------------

static int R_SetPos(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	BS_Vertex Vertex;
	BS_Vertex::LuaVertexToVertex(L, 2, Vertex);
	pR->SetPos(Vertex.X, Vertex.Y);

	return 0;
}

// -----------------------------------------------------------------------------

static int R_SetX(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	pR->SetPosX(static_cast<int>(luaL_checknumber(L, 2)));

	return 0;
}

// -----------------------------------------------------------------------------

static int R_SetY(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	pR->SetPosY(static_cast<int>(luaL_checknumber(L, 2)));

	return 0;
}

// -----------------------------------------------------------------------------

static void DrawPolygon(const BS_Polygon & Polygon, unsigned int Color, const BS_Vertex & Offset)
{
	BS_GraphicEngine * pGE = static_cast<BS_GraphicEngine *>(BS_Kernel::GetInstance()->GetService("gfx"));
	BS_ASSERT(pGE);

	for (int i = 0; i < Polygon.VertexCount - 1; i++)
		pGE->DrawDebugLine(Polygon.Vertecies[i] + Offset, Polygon.Vertecies[i + 1] + Offset, Color);

	pGE->DrawDebugLine(Polygon.Vertecies[Polygon.VertexCount - 1] + Offset, Polygon.Vertecies[0] + Offset, Color);
}

// -----------------------------------------------------------------------------

static void DrawRegion(const BS_Region & Region, unsigned int Color, const BS_Vertex & Offset)
{
	DrawPolygon(Region.GetContour(), Color, Offset);
	for (int i = 0; i < Region.GetHoleCount(); i++)
		DrawPolygon(Region.GetHole(i), Color, Offset);
}

// -----------------------------------------------------------------------------

static int R_Draw(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);

	switch (lua_gettop(L))
	{
		case 3:
			{
				BS_Vertex Offset;
				BS_Vertex::LuaVertexToVertex(L, 3, Offset);
				DrawRegion(*pR, BS_GraphicEngine::LuaColorToARGBColor(L, 2), Offset);
			}
			break;

		case 2:
			DrawRegion(*pR, BS_GraphicEngine::LuaColorToARGBColor(L, 2), BS_Vertex(0, 0));
			break;

		default:
			DrawRegion(*pR, BS_RGB(255, 255, 255), BS_Vertex(0, 0));
	}	

	return 0;
}

// -----------------------------------------------------------------------------

static int R_GetCentroid(lua_State * L)
{
	BS_Region * RPtr = CheckRegion(L);
	BS_ASSERT(RPtr);

	BS_Vertex::VertexToLuaVertex(L, RPtr->GetCentroid());

	return 1;
}

// -----------------------------------------------------------------------------

static int R_Delete(lua_State * L)
{
	BS_Region * pR = CheckRegion(L);
	BS_ASSERT(pR);
	delete pR;
	return 0;
}

// -----------------------------------------------------------------------------

static const luaL_reg REGION_METHODS[] =
{
	"SetPos", R_SetPos,
	"SetX", R_SetX,
	"SetY", R_SetY,
	"GetPos", R_GetPos,
	"IsPointInRegion", R_IsPointInRegion,
	"GetX", R_GetX,
	"GetY", R_GetY,
	"IsValid", R_IsValid,
	"Draw", R_Draw,
	"GetCentroid", R_GetCentroid,
	0, 0,
};

// -----------------------------------------------------------------------------

static BS_WalkRegion * CheckWalkRegion(lua_State * L)
{
	// Der erste Parameter muss vom Typ userdata sein und die Metatable der Klasse Geo.WalkRegion
	unsigned int RegionHandle;
	if ((RegionHandle = *reinterpret_cast<unsigned int *>(my_checkudata(L, 1, WALKREGION_CLASS_NAME))) != 0)
	{
		return reinterpret_cast<BS_WalkRegion *>(BS_RegionRegistry::GetInstance().ResolveHandle(RegionHandle));
	}
	else
	{
		luaL_argcheck(L, 0, 1, "'" WALKREGION_CLASS_NAME "' expected");
	}

	// Compiler ruhigstellen. Ausführung kommt nie an diesem Punkt an.
	return 0;
}

// -----------------------------------------------------------------------------

static int WR_GetPath(lua_State * L)
{
	BS_WalkRegion * pWR = CheckWalkRegion(L);
	BS_ASSERT(pWR);

	BS_Vertex Start;
	BS_Vertex::LuaVertexToVertex(L, 2, Start);
	BS_Vertex End;
	BS_Vertex::LuaVertexToVertex(L, 3, End);
	BS_Path Path;
	if (pWR->QueryPath(Start, End, Path))
	{
		lua_newtable(L);
		BS_Path::const_iterator it = Path.begin();
		for (; it != Path.end(); it++)
		{
			lua_pushnumber(L, (it - Path.begin()) + 1);
			BS_Vertex::VertexToLuaVertex(L, *it);
			lua_settable(L, -3);
		}
	}
	else
		lua_pushnil(L);

	return 1;
}

// -----------------------------------------------------------------------------

static const luaL_reg WALKREGION_METHODS[] =
{
	"GetPath", WR_GetPath,
	0, 0,
};

// -----------------------------------------------------------------------------

bool BS_Geometry::_RegisterScriptBindings()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ScriptEngine * pScript = static_cast<BS_ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State * L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!BS_LuaBindhelper::AddMethodsToClass(L, REGION_CLASS_NAME, REGION_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, WALKREGION_CLASS_NAME, REGION_METHODS)) return false;
	if (!BS_LuaBindhelper::AddMethodsToClass(L, WALKREGION_CLASS_NAME, WALKREGION_METHODS)) return false;

	if (!BS_LuaBindhelper::SetClassGCHandler(L, REGION_CLASS_NAME, R_Delete)) return false;
	if (!BS_LuaBindhelper::SetClassGCHandler(L, WALKREGION_CLASS_NAME, R_Delete)) return false;

	if (!BS_LuaBindhelper::AddFunctionsToLib(L, GEO_LIBRARY_NAME, GEO_FUNCTIONS)) return false;

	return true;
}
